//-
//**************************************************************************/
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//**************************************************************************/
//+

///////////////////////////////////////////////////////////////////////////////
//
// gpuCache MEL command
//
// Creates one or more cache files on disk to store attribute data for
// a span of frames.
//
////////////////////////////////////////////////////////////////////////////////


#include "gpuCacheCmd.h"
#include "gpuCacheShapeNode.h"
#include "gpuCacheStrings.h"
#include "gpuCacheUtil.h"
#include "gpuCacheConfig.h"
#include "gpuCacheVBOProxy.h"
#include "gpuCacheVramQuery.h"
#include "gpuCacheMaterialBakers.h"
#include "gpuCacheSubSceneOverride.h"
#include "gpuCacheUnitBoundingBox.h"

#include "CacheWriter.h"
#include "CacheReader.h"
#include "gpuCacheGeometry.h"

#include <maya/MArgList.h>
#include <maya/MAnimControl.h>
#include <maya/MDagPathArray.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MFnSet.h>
#include <maya/MFnSubd.h>
#include <maya/MGlobal.h>
#include <maya/MPlugArray.h>
#include <maya/MSyntax.h>
#include <maya/MBoundingBox.h>
#include <maya/MFileObject.h>
#include <maya/MItDag.h>
#include <maya/MVector.h>
#include <maya/MFnTransform.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnComponentListData.h>
#include <maya/MFnLambertShader.h>
#include <maya/MViewport2Renderer.h>

#include <maya/MDagPath.h>
#include <maya/MPointArray.h>
#include <maya/MUintArray.h>

#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/unordered_set.hpp>

#include <cfloat>
#include <limits>
#include <list>
#include <iomanip>
#include <map>
#include <sstream>
#include <fstream>


#define MStatError(status,msg)                              \
    if ( MS::kSuccess != (status) ) {                       \
        MPxCommand::displayError(                           \
            (msg) + MString(":") + (status).errorString()); \
        return (status);                                    \
    }

#define MStatErrorNullObj(status,msg)                       \
    if ( MS::kSuccess != (status) ) {                       \
        MPxCommand::displayError(                           \
            (msg) + MString(":") + (status).errorString()); \
        return MObject::kNullObj;                           \
    }

#define MCheckReturn(expression)                \
    {                                           \
        MStatus status = (expression);          \
        if ( MS::kSuccess != (status) ) {       \
            return (status);                    \
        }                                       \
    }

#define MUpdateProgressAndCheckInterruption(progressBar)    \
    {                                                       \
        (progressBar).stepProgress();                       \
        if ((progressBar).isCancelled()) {                  \
            return MS::kFailure;                            \
        }                                                   \
    }                                                       \

namespace {

using namespace GPUCache;

    //==============================================================================
    // LOCAL FUNCTIONS
    //==============================================================================

    // Create a cache writer object that will write to the specified file path.
    // If the directory does not exist, a new one will be created.
    // If the file already exists, the existing file will be deleted. (Overwrite)
    // Writing to a read-only file will return an error.
    // Args:
    //    targetFile    The target file path that the new writer will write files to.
    //    compressLevel Hint the compress level: -1 (Store), 0~9 (Fastest~Best).
    //    dataFormat    Hint the file format: ogawa or hdf.
    boost::shared_ptr<CacheWriter> createWriter(const MFileObject& targetFile,
                                                const char         compressLevel,
                                                const MString&     dataFormat)
    {
        // Get the directory of the target file.
        MFileObject cacheDirectory;
        cacheDirectory.setRawFullName(targetFile.resolvedPath());

        // Make sure the cache folder exists.
        if (!cacheDirectory.exists()) {
            // Create the cache folder.
            MString createFolderCmd;
            createFolderCmd.format("sysFile -md \"^1s\"", EncodeString(targetFile.resolvedPath()));
            MGlobal::executeCommand(createFolderCmd);
        }

        // Delete the existing file.
        // We have already confirmed that the file is going to be overwritten.
        if (MFileObject(targetFile).exists()) {
            // The file already exists!
            MString resolvedFullName = targetFile.resolvedFullName();

            // Check if the file is writeable.
            bool writeable;
            {
                std::ofstream ofs(resolvedFullName.asChar());
                writeable = ofs.is_open();
            }

            // We can't overwrite a read-only file.
            if (!writeable) {
                MStatus stat;
                MString fmt = MStringResource::getString(kCouldNotSaveFileMsg, stat);
                MString msg;
                msg.format(fmt, resolvedFullName);
                MPxCommand::displayError(msg);
                return boost::shared_ptr<CacheWriter>();
            }

            // We are going to overwrite the file. Delete it!!
            if (remove(resolvedFullName.asChar()) != 0) {
                MStatus stat;
                MString fmt = MStringResource::getString(kCouldNotSaveFileMsg, stat);
                MString msg;
                msg.format(fmt, resolvedFullName);
                MPxCommand::displayError(msg);
                return boost::shared_ptr<CacheWriter>();
            }
        }

        // first parameter is the file to write
        // second parameter is gzip compress level, -1 or 0~9
        // third parameter is data format, hdf or ogawa        
        boost::shared_ptr<CacheWriter> cacheWriter = 
            CacheWriter::create("Alembic", targetFile, compressLevel, dataFormat);

        if (!cacheWriter) {
            MStatus stat;
            MString msg = MStringResource::getString(kCreateCacheWriterErrorMsg, stat);
            MPxCommand::displayError(msg);
            return boost::shared_ptr<CacheWriter>();
        }

        if (!cacheWriter->valid()) {
            // release the file handle
            cacheWriter.reset();

            MString errorMsg;
            errorMsg.format("Couldn't open cache file: ^1s", targetFile.resolvedFullName());
            MPxCommand::displayError(errorMsg);
            return boost::shared_ptr<CacheWriter>();
        }

        return cacheWriter;
    }

    bool isPlugConnectedToTexture2d(const MPlug& plug)
    {
        MPlugArray connections;
        if (plug.connectedTo(connections, true, false)) {
            assert(connections.length() == 1);
			//return false immediately if connections is empty, in order to fix the crash MAYA-41542
			if(connections.length() == 0)
				return false;

            MObject srcNode = connections[0].node();

            return srcNode.hasFn(MFn::kTexture2d);
        }

        return false;
    }

    MColor getTexture2dDefaultColor(const MPlug& plug)
    {
        MPlugArray connections;
        if (plug.connectedTo(connections, true, false)) {
            assert(connections.length() == 1);
			//return immediately if connections is empty
			if(connections.length() == 0)
				return MColor(0.5, 0.5, 0.5);

            MFnDependencyNode srcNode(connections[0].node());

            MPlug diffusePlugR = srcNode.findPlug("defaultColorR");
            MPlug diffusePlugG = srcNode.findPlug("defaultColorG");
            MPlug diffusePlugB = srcNode.findPlug("defaultColorB");

            assert(!diffusePlugR.isNull());
            assert(!diffusePlugG.isNull());
            assert(!diffusePlugB.isNull());

            MStatus statusR, statusG, statusB;
            float r = diffusePlugR.asFloat(MDGContext::fsNormal, &statusR);
            float g = diffusePlugG.asFloat(MDGContext::fsNormal, &statusG);
            float b = diffusePlugB.asFloat(MDGContext::fsNormal, &statusB);

            assert(statusR == MS::kSuccess);
            assert(statusG == MS::kSuccess);
            assert(statusB == MS::kSuccess);

            return MColor(r, g, b);
        }

        return MColor(0.5, 0.5, 0.5);
    }

    bool isPlugConnectedToTextureNode(const MPlug& plug)
    {
        MPlugArray connections;
        if (plug.connectedTo(connections, true, false)) {
            assert(connections.length() == 1);
			//return false immediately if connections is empty
			if(connections.length() == 0)
				return false;

            MObject srcNode = connections[0].node();

            if (srcNode.hasFn(MFn::kTexture2d) ||
                srcNode.hasFn(MFn::kTexture3d) ||
                srcNode.hasFn(MFn::kTextureEnv) ||
                srcNode.hasFn(MFn::kLayeredTexture) ||
                srcNode.hasFn(MFn::kImageSource)) {
                return true;
            }
        }

        return false;
    }

    MStatus getShapeDiffuseColors(const std::vector<MDagPath>& paths,
                                  std::vector<MColor>& diffuseColors)
    {
        MStatus status;
        diffuseColors.resize(paths.size(), Config::kDefaultGrayColor);

        // Get the diffuse color for each instance
        for (size_t pathIndex = 0; pathIndex < paths.size(); pathIndex++) {
            MFnDagNode shape(paths[pathIndex], &status);
            assert(status == MS::kSuccess);

            MObject shadingGroup;
            MObject shaderObj;

            // Find the instObjGroups plug
            MPlug instObjectGroupsParent = shape.findPlug("instObjGroups");
            assert(!instObjectGroupsParent.isNull());

            MPlug instObjectGroups = instObjectGroupsParent.elementByLogicalIndex(
                paths[pathIndex].instanceNumber());
            assert(!instObjectGroups.isNull());

            // instObjGroups is connected, the whole shape is assigned a material
            if (instObjectGroups.isConnected()) {
                // instObjGroups[instanceNumber] -> shadingGroup
                MPlugArray dstPlugs;
                instObjectGroups.connectedTo(dstPlugs, false, true, &status);
                if (status && dstPlugs.length() > 0) {
                    // Found shadingGroup assigned to the whole shape
                    shadingGroup = dstPlugs[0].node();
                }
            }

            // For per-component shader assignment, we use the first shading group.
            // Find the objectGroups plug
            MPlug objectGroupsParent = instObjectGroups.child(0);
            assert(!objectGroupsParent.isNull());

            for (unsigned int parts = 0;
                parts < objectGroupsParent.numElements() && shadingGroup.isNull();
                parts++) {
                    MPlug objectGroups = objectGroupsParent[parts];

                    // objectGroups is connected, there is per-component material
                    if (objectGroups.isConnected()) {
                        // objectGroups[i] -> shadingGroup
                        MPlugArray dstPlugs;
                        objectGroups.connectedTo(dstPlugs, false, true, &status);
                        if (status && dstPlugs.length() > 0) {
                            // Found shadingGroup assigned to components
                            shadingGroup = dstPlugs[0].node();
                        }
                    }
            } // for each objectGroup plug

            if (!shadingGroup.isNull()) {
                // Found a shading group, find its surface shader
                MFnDependencyNode shadingEngine(shadingGroup, &status);
                assert(status == MS::kSuccess);

                // Find surfaceShader plug
                MPlug surfaceShaderPlug = shadingEngine.findPlug("surfaceShader");
                assert(!surfaceShaderPlug.isNull());

                // outColor -> surfaceShader
                if (surfaceShaderPlug.isConnected()) {
                    MPlugArray srcPlugs;
                    surfaceShaderPlug.connectedTo(srcPlugs, true, false, &status);
                    if (status && srcPlugs.length() > 0) {
                        // Found the material node
                        shaderObj = srcPlugs[0].node();
                    }
                }
            }

            if (!shaderObj.isNull()) {
                MColor diffuseColor = Config::kDefaultGrayColor;
                MColor transparency = Config::kDefaultTransparency;

                // Found a material node, get its color
                if (shaderObj.hasFn(MFn::kLambert)) {
                    MFnLambertShader lambert(shaderObj, &status);
                    assert(status == MS::kSuccess);

                    MPlug colorPlug = lambert.findPlug("color");
                    assert(!colorPlug.isNull());
                    MPlug diffusePlug = lambert.findPlug("diffuse");
                    assert(!diffusePlug.isNull());
                    MPlug transparencyPlug = lambert.findPlug("transparency");
                    assert(!transparencyPlug.isNull());

                    if (isPlugConnectedToTexture2d(colorPlug)) {
                        diffuseColor = getTexture2dDefaultColor(colorPlug);
                    }
                    else if (!isPlugConnectedToTextureNode(colorPlug)) {
                        diffuseColor = lambert.color();
                    }

                    if (!isPlugConnectedToTextureNode(diffusePlug)) {
                        diffuseColor *= lambert.diffuseCoeff();
                    }

                    if (!isPlugConnectedToTextureNode(transparencyPlug)) {
                        transparency = lambert.transparency();
                    }
                }

                // Transparency RGB Luminance as alpha
                diffuseColor.a = 1.0f - (transparency.r * 0.3f +
                    transparency.g * 0.59f + transparency.b * 0.11f);
                diffuseColors[pathIndex] = diffuseColor;
            }
        }

        return MS::kSuccess;
    }

    MString getSceneName()
    {
        MString sceneName = MGlobal::executeCommandStringResult(
            "basenameEx(`file -q -sceneName`)");
        if (sceneName.length() == 0) {
            sceneName = MGlobal::executeCommandStringResult("untitledFileName");
        }
        return sceneName;
    }

    MString getSceneNameAsValidObjectName()
    {
        return MGlobal::executeCommandStringResult("formValidObjectName \"" + EncodeString(getSceneName()) + "\"");
    }

    size_t maxNumVerts(const ShapeData::Ptr& geom)
    {
        size_t maxNumVerts = 0;
        BOOST_FOREACH(
            const ShapeData::SampleMap::value_type& smv, geom->getSamples()
        ) {
            maxNumVerts = std::max(maxNumVerts, smv.second->numVerts());
        }
        return maxNumVerts;
    }

    double toHumanUnits(MUint64 bytes, MString& units)
    {
        const MUint64 KB = 1024;
        const MUint64 MB = 1024 * KB;
        const MUint64 GB = 1024 * MB;
        const MUint64 TB = 1024 * GB;

        double  value;
        if (bytes >= TB) {
            units = "TB";
            value =  double(bytes)/TB;
        }
        else if (bytes >= GB) {
            units = "GB";
            value =  double(bytes)/GB;
        }
        else if (bytes >= MB) {
            units = "MB";
            value =  double(bytes)/MB;
        }
        else if (bytes >= KB) {
            units = "KB";
            value =  double(bytes)/KB;
        }
        else {
            units = "bytes";
            value =  double(bytes);
        }

        return value;
    }


    //==============================================================================
    // CLASS Baker
    //==============================================================================

    class Baker
    {
    public:
        static boost::shared_ptr<Baker> create(const MObject&               object,
                                               const std::vector<MDagPath>& paths);
        static bool isBakeable(const MObject& dagNode);

        Baker(const MObject& object, const std::vector<MDagPath>& paths)
            : fNode(object), fPaths(paths)
        {}
        virtual ~Baker() {}

        virtual MStatus sample(const MTime& time) = 0;
        virtual const SubNode::MPtr getNode(size_t instIndex) const = 0;

        virtual void setWriteMaterials() {}
        virtual void setUseBaseTessellation() {}

    protected:
        MFnDagNode            fNode;
        std::vector<MDagPath> fPaths;
    };


    //==============================================================================
    // CLASS ShapeBaker
    //==============================================================================

    // Base class for wrappers that encapsulate the logic necessary to
    // bake particular types of shapes (meshes, nurbs, subds, etc.).
    class ShapeBaker : public Baker
    {
    public:
        virtual ~ShapeBaker() {}

		void enableUVs() 
		{
			fCacheMeshSampler->enableUVs();
		}

        // The function is called to sample the geometry at the specified time
        virtual MStatus sample(const MTime& time)
        {
            // Sample the shape
            MCheckReturn( sampleTopologyAndAttributes() );

            // Sample the diffuse color
            std::vector<MColor> diffuseColors;
            MCheckReturn( getShapeDiffuseColors(fPaths, diffuseColors) );

            bool diffuseColorsAnimated = (fPrevDiffuseColors != diffuseColors);

            // add sample to geometry
            if (fCacheMeshSampler->isAnimated() || diffuseColorsAnimated) {
                for (size_t i = 0; i < fGeometryInstances.size(); i++) {
                    fGeometryInstances[i]->addSample(
                        fCacheMeshSampler->getSample(
                            time.as(MTime::kSeconds),
                            diffuseColors[i]));
                }
            }

            fPrevDiffuseColors.swap(diffuseColors);
            return MS::kSuccess;
        }

        // The function is called at the end of baking process to get the baked geometry
        virtual const SubNode::MPtr getNode(size_t instIndex) const
        {
            return SubNode::create(
                fNode.name(),
                fGeometryInstances[instIndex]
            );
        }

    protected:
        ShapeBaker(const MObject& node, const std::vector<MDagPath>& paths)
            : Baker(node, paths),
              fCacheMeshSampler(
				  // note: the UVs can always get enabled later by calling our method enableUVs()
                  CacheMeshSampler::create(!Config::isIgnoringUVs())),
              fGeometryInstances(paths.size())
        {
            // Create one geometry for each instance
            for (size_t i = 0; i < paths.size(); i++) {
                fGeometryInstances[i] = ShapeData::create();
            }
        }

        virtual void setWriteMaterials()
        {
            // Create one geometry for each instance
            for (size_t i = 0; i < fPaths.size(); i++) {
                // Set material to the shape data.
                MString surfaceMaterial;

                InstanceMaterialLookup lookup(fPaths[i]);
                if (lookup.hasWholeObjectMaterial()) {
                    // Whole object material assignment.
                    MObject material = lookup.findWholeObjectSurfaceMaterial();
                    if (!material.isNull()) {
                        MFnDependencyNode dgMaterial(material);
                        surfaceMaterial = dgMaterial.name();
                    }
                }
                else if (lookup.hasComponentMaterials()) {
                    // Per-component material assignment.
                    std::vector<MObject> materials;
                    lookup.findSurfaceMaterials(materials);

                    // Use the first surface material
                    // TODO: Support per-component material assignment.
                    BOOST_FOREACH (const MObject& material, materials) {
                        if (!material.isNull()) {
                            MFnDependencyNode dgMaterial(material);
                            surfaceMaterial = dgMaterial.name();
                            break;
                        }
                    }
                }

                if (surfaceMaterial.length() > 0) {
                    fGeometryInstances[i]->setMaterial(surfaceMaterial);
                }
            }
        }

        virtual void setUseBaseTessellation()
        {
            fCacheMeshSampler->setUseBaseTessellation();
        }

        virtual MStatus sampleTopologyAndAttributes() = 0;

    private:
        // Forbidden and not implemented.
        ShapeBaker(const ShapeBaker&);
        const ShapeBaker& operator=(const ShapeBaker&);

    protected:
        const boost::shared_ptr<CacheMeshSampler> fCacheMeshSampler;
        std::vector<MColor>                       fPrevDiffuseColors;
        std::vector<ShapeData::MPtr>              fGeometryInstances;
    };

    //==============================================================================
    // CLASS XformBaker
    //==============================================================================

    // Class for baking a transform MObject
    class XformBaker : public Baker
    {
    public:
        virtual ~XformBaker()
        {}

        XformBaker(const MObject& xformNode, const std::vector<MDagPath>& xformPaths)
            : Baker(xformNode, xformPaths),
              fCacheXformSamplers(CacheXformSampler::create(xformNode)),
              fXformInstances(xformPaths.size())
        {
            for (size_t i = 0; i < fXformInstances.size(); i++) {
                fXformInstances[i] = XformData::create();
            }
        }

        virtual MStatus sample(const MTime& currentTime)
        {
            fCacheXformSamplers->addSample();
            if (fCacheXformSamplers->isAnimated()) {
                for (size_t i = 0; i < fXformInstances.size(); i++) {
                    fXformInstances[i]->addSample(
                        fCacheXformSamplers->getSample(currentTime.as(MTime::kSeconds)));
                }
            }
            return MS::kSuccess;
        }

        virtual const SubNode::MPtr getNode(size_t instIndex) const
        {
            return SubNode::create(
                fNode.name(),
                fXformInstances[instIndex]
            );
        }

    private:
        boost::shared_ptr<CacheXformSampler> fCacheXformSamplers;
        std::vector<XformData::MPtr>         fXformInstances;
    };

    //==============================================================================
    // CLASS MeshDataBaker
    //==============================================================================

    // Base class for baking a mesh MObject
    class MeshDataBaker : public ShapeBaker
    {
    public:
        virtual ~MeshDataBaker() {}

    protected:
        virtual MStatus sampleTopologyAndAttributes()
        {
            MStatus status;

            MObject meshData = getMeshData(&status);
            MStatError(status, "getMeshData()");

            bool shapeVisibility = ShapeVisibilityChecker(fNode.object()).isVisible();

            // Snapshot the topology and vertex attributes.
            return fCacheMeshSampler->addSample(meshData, shapeVisibility) ?
                MS::kSuccess : MS::kFailure;
        }

        virtual MObject getMeshData(MStatus* status) = 0;

        MeshDataBaker(const MObject& shapeNode, const std::vector<MDagPath>& shapePaths)
            : ShapeBaker(shapeNode, shapePaths)
        {}

    private:
        // Forbidden and not implemented.
        MeshDataBaker(const MeshDataBaker&);
        const MeshDataBaker& operator=(const MeshDataBaker&);
    };


    //==============================================================================
    // CLASS MeshBaker
    //==============================================================================

    class MeshBaker : public ShapeBaker
    {
    public:
        MeshBaker(const MObject& meshNode, const std::vector<MDagPath>& meshPaths)
            : ShapeBaker(meshNode, meshPaths), fMeshNode(meshNode)
        {}

        virtual ~MeshBaker() {}

    protected:
        virtual MStatus sampleTopologyAndAttributes()
        {
            return fCacheMeshSampler->addSampleFromMesh(fMeshNode) ?
                        MS::kSuccess : MS::kFailure;
        }

    private:
        // Forbidden and not implemented.
        MeshBaker(const MeshBaker&);
        const MeshBaker& operator=(const MeshBaker&);

        MFnMesh fMeshNode;
    };


    //==============================================================================
    // CLASS NurbsBaker
    //==============================================================================

    class NurbsBaker : public MeshDataBaker
    {
    public:
        NurbsBaker(const MObject& nurbsNode, const std::vector<MDagPath>& nurbsPaths)
            : MeshDataBaker(nurbsNode, nurbsPaths)
        {
            // Disable Viewport 2.0 updates while baking NURBS surfaces.
            MHWRender::MRenderer::disableChangeManagementUntilNextRefresh();
        }

    protected:
        virtual MObject getMeshData(MStatus* status)
        {
            MObject mesh;
            MDGModifier modifier;

            MFnNurbsSurface nurbsNode(fNode.object());

            MObject tessellator = modifier.createNode("nurbsTessellate");
            MFnDependencyNode tessellatorNode(tessellator);
            modifier.connect(nurbsNode.findPlug("explicitTessellationAttributes"),
                             tessellatorNode.findPlug("explicitTessellationAttributes"));
            modifier.connect(nurbsNode.findPlug("curvatureTolerance"),
                             tessellatorNode.findPlug("curvatureTolerance"));
            modifier.connect(nurbsNode.findPlug("uDivisionsFactor"),
                             tessellatorNode.findPlug("uDivisionsFactor"));
            modifier.connect(nurbsNode.findPlug("vDivisionsFactor"),
                             tessellatorNode.findPlug("vDivisionsFactor"));
            modifier.connect(nurbsNode.findPlug("modeU"),
                             tessellatorNode.findPlug("uType"));
            modifier.connect(nurbsNode.findPlug("modeV"),
                             tessellatorNode.findPlug("vType"));
            modifier.connect(nurbsNode.findPlug("numberU"),
                             tessellatorNode.findPlug("uNumber"));
            modifier.connect(nurbsNode.findPlug("numberV"),
                             tessellatorNode.findPlug("vNumber"));
            modifier.connect(nurbsNode.findPlug("useChordHeight"),
                             tessellatorNode.findPlug("useChordHeight"));
            modifier.connect(nurbsNode.findPlug("useChordHeightRatio"),
                             tessellatorNode.findPlug("useChordHeightRatio"));
            modifier.connect(nurbsNode.findPlug("chordHeight"),
                             tessellatorNode.findPlug("chordHeight"));
            modifier.connect(nurbsNode.findPlug("chordHeightRatio"),
                             tessellatorNode.findPlug("chordHeightRatio"));
            modifier.connect(nurbsNode.findPlug("smoothEdge"),
                             tessellatorNode.findPlug("smoothEdge"));
            modifier.connect(nurbsNode.findPlug("smoothEdgeRatio"),
                             tessellatorNode.findPlug("smoothEdgeRatio"));
            modifier.connect(nurbsNode.findPlug("edgeSwap"),
                             tessellatorNode.findPlug("edgeSwap"));
            modifier.connect(nurbsNode.findPlug("local"),
                             tessellatorNode.findPlug("inputSurface"));

            // poly type - 0 means triangles
            modifier.newPlugValueInt(tessellatorNode.findPlug("polygonType"),0);
            // format - 2 means general fit
            modifier.newPlugValueInt(tessellatorNode.findPlug("format"),2);

            modifier.doIt();
            tessellatorNode.findPlug("outputPolygon").getValue(mesh);
            modifier.undoIt();

            return mesh;
        }
    };


    //==============================================================================
    // CLASS SubdBaker
    //==============================================================================

    class SubdBaker : public MeshDataBaker
    {
    public:
        SubdBaker(const MObject& subdNode, const std::vector<MDagPath>& subdPaths)
            : MeshDataBaker(subdNode, subdPaths)
        {}

    protected:
        virtual MObject getMeshData(MStatus* status)
        {
            MFnSubd subdNode(fNode.object());

            MFnMeshData meshData;
            meshData.create(status);
            if (!*status) return meshData.object();

            int format = -1;
            int depth = -1;
            int sampleCount = -1;
            MPlug formatPlug = subdNode.findPlug("format");
            MPlug depthPlug = subdNode.findPlug("depth");
            MPlug sampleCountPlug = subdNode.findPlug("sampleCount");
            formatPlug.getValue(format);
            depthPlug.getValue(depth);
            sampleCountPlug.getValue(sampleCount);

            subdNode.tesselate(
                format==0, depth, sampleCount, meshData.object(), status);

            return meshData.object();
        }
    };


    //==============================================================================
    // CLASS RecursiveBaker
    //==============================================================================

    // This class simply extracts the hierarchy from gpuCache node.
    class RecursiveBaker : public Baker
    {
    public:
        RecursiveBaker(const MObject& shapeNode, const std::vector<MDagPath>& shapePaths)
            : Baker(shapeNode, shapePaths)
        {
            // Find the user node
            MPxNode* userNode = fNode.userNode();
            assert(userNode);

            ShapeNode* bakedNode = userNode ?
                dynamic_cast<ShapeNode*>(userNode) : NULL;
            assert(bakedNode);

            // Extract the baked geometry
            if (bakedNode) {
                GlobalReaderCache::theCache().waitForRead(bakedNode->getCacheFileEntry().get());
                fSrcTopNode = bakedNode->getCachedGeometry();
                if (fSrcTopNode) {
                    fSampleReplicator.reset(new SampleReplicator);
                    fSrcTopNode->accept(*fSampleReplicator);
                }
            }
        }

        virtual ~RecursiveBaker()
        {}

        virtual MStatus sample(const MTime& time)
        {
            if (!fSrcTopNode) {
                return MS::kFailure;
            }

            return fSampleReplicator->sample(time);
        }

        virtual const SubNode::MPtr getNode(size_t instIndex) const
        {
            // We ignore the material assigned to the gpuCache node.
            if (fSrcTopNode && !fDstTopNode) {
                // We replicate the hierarchy after all xform/shape data are
                // filled with samples.
                HierarchyReplicator hierarchyReplicator(fSampleReplicator);
                fSrcTopNode->accept(hierarchyReplicator);

                RecursiveBaker* nonConstThis = const_cast<RecursiveBaker*>(this);
                nonConstThis->fDstTopNode = hierarchyReplicator.dstSubNode();
            }
            return fDstTopNode;
        }

    private:
        // Forbidden and not implemented.
        RecursiveBaker(const RecursiveBaker&);
        const RecursiveBaker& operator=(const RecursiveBaker&);

        class SampleReplicator : public SubNodeVisitor
        {
        public:
            typedef boost::shared_ptr<SampleReplicator> MPtr;

            SampleReplicator()
            {}

            virtual void visit(const XformData& srcXform,
                               const SubNode&   srcSubNode)
            {
                // Create a new xform data, it will be filled later in sample()
                XformData::MPtr dstXform = XformData::create();
                fXforms[&srcXform] = std::make_pair(
                        dstXform, boost::shared_ptr<const XformSample>());

                // Recursively replicate xform/shape data in the child hierarchy
                BOOST_FOREACH(const SubNode::Ptr& srcChild, srcSubNode.getChildren()) {
                    srcChild->accept(*this);
                }
            }

            virtual void visit(const ShapeData& srcShape,
                               const SubNode&   srcSubNode)
            {
                // Create a new shape data, it will be filled later in sample()
                ShapeData::MPtr dstShape = ShapeData::create();
                dstShape->setMaterials(srcShape.getMaterials());
                fShapes[&srcShape] = std::make_pair(
                        dstShape, boost::shared_ptr<const ShapeSample>());
            }

            MStatus sample(const MTime& time)
            {
                BOOST_FOREACH(XformMapping::value_type& xform, fXforms) {
                    // Get the already baked sample
                    boost::shared_ptr<const XformSample> srcXformSample =
                        xform.first->getSample(time);

                    // Only add the sample if it's different than prev sample
                    if (xform.second.second != srcXformSample) {
                        // Create a new sample with the same content but different time
                        boost::shared_ptr<XformSample> dstXformSample =
                            XformSample::create(
                                time.as(MTime::kSeconds),
                                srcXformSample->xform(),
                                srcXformSample->boundingBox(),
                                srcXformSample->visibility());

                        xform.second.first->addSample(dstXformSample);
                        xform.second.second = srcXformSample;
                    }
                }


                BOOST_FOREACH(ShapeMapping::value_type& shape, fShapes) {
                    // Get the already baked sample
                    boost::shared_ptr<const ShapeSample> srcShapeSample =
                        shape.first->getSample(time);

                    // Only add the sample if it's different than prev sample
                    if (shape.second.second != srcShapeSample) {
                        // Create a new sample with the same content but different time
                        boost::shared_ptr<ShapeSample> dstShapeSample =
                            ShapeSample::create(
                                time.as(MTime::kSeconds),
                                srcShapeSample->numWires(),
                                srcShapeSample->numVerts(),
                                srcShapeSample->wireVertIndices(),
                                srcShapeSample->triangleVertexIndexGroups(),
                                srcShapeSample->positions(),
                                srcShapeSample->boundingBox(),
                                srcShapeSample->diffuseColor(),
                                srcShapeSample->visibility());

                        if (srcShapeSample->normals()) {
                            dstShapeSample->setNormals(srcShapeSample->normals());
                        }

                        if (srcShapeSample->uvs()) {
                            dstShapeSample->setUVs(srcShapeSample->uvs());
                        }

                        shape.second.first->addSample(dstShapeSample);
                        shape.second.second = srcShapeSample;
                    }
                }

                return MS::kSuccess;
            }

            XformData::MPtr xform(const XformData& xform)
            {
                XformMapping::iterator iter = fXforms.find(&xform);
                assert(iter != fXforms.end());
                return (*iter).second.first;
            }

            ShapeData::MPtr shape(const ShapeData& shape)
            {
                ShapeMapping::iterator iter = fShapes.find(&shape);
                assert(iter != fShapes.end());
                return (*iter).second.first;
            }

        private:
            // Forbidden and not implemented.
            SampleReplicator(const SampleReplicator&);
            const SampleReplicator& operator=(const SampleReplicator&);

            typedef std::pair<XformData::MPtr,boost::shared_ptr<const XformSample> > XformWithPrev;
            typedef std::pair<ShapeData::MPtr,boost::shared_ptr<const ShapeSample> > ShapeWithPrev;
            typedef std::map<const XformData*,XformWithPrev> XformMapping;
            typedef std::map<const ShapeData*,ShapeWithPrev> ShapeMapping;
            XformMapping fXforms;
            ShapeMapping fShapes;
        };

        class HierarchyReplicator : public SubNodeVisitor
        {
        public:
            HierarchyReplicator(SampleReplicator::MPtr sampleReplicator)
                : fSampleReplicator(sampleReplicator)
            {}

            virtual void visit(const XformData& srcXform,
                               const SubNode&   srcSubNode)
            {
                // Create a new sub node for the xform
                // We rename "|" to "top" as we don't want "|" to appear in hierarchy.
                XformData::MPtr dstXform = fSampleReplicator->xform(srcXform);
                fDstSubNode = SubNode::create(srcSubNode.getName() != "|" ?
                    srcSubNode.getName() : "top", dstXform);

                // Recursively replicate the child hierarchy
                BOOST_FOREACH(const SubNode::Ptr& srcChild, srcSubNode.getChildren())
                {
                    HierarchyReplicator replicator(fSampleReplicator);
                    srcChild->accept(replicator);

                    SubNode::connect(fDstSubNode, replicator.dstSubNode());
                }
            }

            virtual void visit(const ShapeData& srcShape,
                               const SubNode&   srcSubNode)
            {
                // Create a new sub node for the shape
                ShapeData::MPtr dstShape = fSampleReplicator->shape(srcShape);
                fDstSubNode = SubNode::create(srcSubNode.getName(), dstShape);
            }

            SubNode::MPtr dstSubNode() const
            { return fDstSubNode; }

        private:
            // Forbidden and not implemented.
            HierarchyReplicator(const HierarchyReplicator&);
            const HierarchyReplicator& operator=(const HierarchyReplicator&);

            SampleReplicator::MPtr fSampleReplicator;
            SubNode::MPtr fDstSubNode;
        };

        SubNode::Ptr            fSrcTopNode;
        SubNode::MPtr           fDstTopNode;
        SampleReplicator::MPtr  fSampleReplicator;
    };


    //==============================================================================
    // CLASS Baker
    //==============================================================================

    bool Baker::isBakeable(const MObject& dagNode)
    {
        if (dagNode.hasFn(MFn::kTransform)
            || dagNode.hasFn(MFn::kMesh)
            || dagNode.hasFn(MFn::kNurbsSurface)
            || dagNode.hasFn(MFn::kSubdiv))
        {
            return true;
        }

        return false;
    }

    boost::shared_ptr<Baker> Baker::create(const MObject&               shapeNode,
                                           const std::vector<MDagPath>& shapePaths)
    {
        if (shapeNode.hasFn(MFn::kTransform)) {
            return boost::make_shared<XformBaker>(shapeNode, shapePaths);
        }
        else if (shapeNode.hasFn(MFn::kMesh)) {
            return boost::make_shared<MeshBaker>(shapeNode, shapePaths);
        }
        else if (shapeNode.hasFn(MFn::kNurbsSurface)) {
            return boost::make_shared<NurbsBaker>(shapeNode, shapePaths);
        }
        else if (shapeNode.hasFn(MFn::kSubdiv)) {
            return boost::make_shared<SubdBaker>(shapeNode, shapePaths);
        }

        MStatus status;
        MFnDagNode shape(shapeNode, &status);
        assert(status == MS::kSuccess);

        if (shape.typeId() == ShapeNode::id) {
            return boost::make_shared<RecursiveBaker>(shapeNode, shapePaths);
        }

        assert(false);
        return boost::shared_ptr<Baker>();
    }

    //==============================================================================
    // CLASS Writer
    //==============================================================================

    class Writer : boost::noncopyable
    {
    public:
        Writer(const char       compressLevel,
               const MString&   dataFormat,
               const MTime&     timePerCycle,
               const MTime&     startTime)
          : fCompressLevel(compressLevel),
            fDataFormat(dataFormat),
            fTimePerCycleInSeconds(timePerCycle.as(MTime::kSeconds)),
            fStartTimeInSeconds(startTime.as(MTime::kSeconds))
        {}

        // Write a sub-node hierarchy to the specified file.
        MStatus writeNode(const SubNode::Ptr&           subNode,
                          const MaterialGraphMap::Ptr&  materials,
                          const MFileObject&            targetFile)
        {
            boost::shared_ptr<CacheWriter> writer = createWriter(
                targetFile,
                fCompressLevel,
                fDataFormat
            );

            if (!writer) {
                return MS::kFailure;
            }

            writer->writeSubNodeHierarchy(
                subNode,
                fTimePerCycleInSeconds, fStartTimeInSeconds);
            if (materials) {
                writer->writeMaterials(
                    materials,
                    fTimePerCycleInSeconds, fStartTimeInSeconds);
            }

            return MS::kSuccess;
        }

        // Write a list of sub-node hierarchies to the specified file.
        MStatus writeNodes(const std::vector<SubNode::Ptr>& subNodes,
                           const MaterialGraphMap::Ptr&     materials,
                           const MFileObject&               targetFile)
        {
            boost::shared_ptr<CacheWriter> writer = createWriter(
                targetFile,
                fCompressLevel,
                fDataFormat
            );

            if (!writer) {
                return MS::kFailure;
            }

            BOOST_FOREACH(const SubNode::Ptr& subNode, subNodes) {
                writer->writeSubNodeHierarchy(
                    subNode,
                    fTimePerCycleInSeconds,
                    fStartTimeInSeconds);
            }
            if (materials) {
                writer->writeMaterials(materials,
                    fTimePerCycleInSeconds, fStartTimeInSeconds);
            }

            return MS::kSuccess;
        }

    private:
        const char     fCompressLevel;
        const MString  fDataFormat;
        const double   fTimePerCycleInSeconds;
        const double   fStartTimeInSeconds;
    };


    //==========================================================================
    // CLASS Stat
    //==========================================================================

    class Stat
    {
    public:
        Stat(MUint64 bytesPerUnit)
            : fMin(std::numeric_limits<MUint64>::max()),
              fMax(0),
              fTotal(0),
              fInstancedTotal(0),
              fBytesPerUnit(bytesPerUnit)
        {}

        void addSample(
            const boost::shared_ptr<const IndexBuffer> buffer, const int indicesPerElem)
        {
            addSample(buffer->numIndices() / indicesPerElem, buffer.get());
        };

        void addSample(
            const boost::shared_ptr<const VertexBuffer> buffer)
        {
            addSample(buffer->numVerts(), buffer.get());
        };

        void addSample(
            const MHWRender::MIndexBuffer* buffer, size_t numIndices)
        {
            addSample(numIndices, (void*)buffer);
        }

        void addSample(
            const MHWRender::MVertexBuffer* buffer, size_t numVertices)
        {
            addSample(numVertices, (void*)buffer);
        }

        void addSample(
            const boost::shared_ptr<const VBOBuffer> buffer, size_t numPrimitives)
        {
            addSample(numPrimitives, buffer.get());
        }

        MUint64 getNbSamples() const        { return fUniqueEntries.size(); }
        MUint64 getMin() const              { return fMin;}
        MUint64 getMax() const              { return fMin;}
        MUint64 getTotal() const            { return fTotal;}
        MUint64 getInstancedTotal() const   { return fInstancedTotal;}

        double getAverage() const {
            return double(getTotal())/double(getNbSamples());
        }

        MUint64 getSize() const {
            return fTotal * fBytesPerUnit;
        }

        MString print(MString name) const
        {
            MStatus status;
            MString result;

            if (getNbSamples() == 0) {
                MString msg;
                msg.format(
                    MStringResource::getString(kStatsZeroBuffersMsg, status),
                    name);
                result = msg;
            }
            else {
                MString memUnit;
                double  memSize = toHumanUnits(getSize(), memUnit);

                MString msg;
                MString msg_buffers; msg_buffers += (double)getNbSamples();
                MString msg_avrg;    msg_avrg    += (double)getAverage();
                MString msg_min;     msg_min     += (double)fMin;
                MString msg_max;     msg_max     += (double)fMax;
                MString msg_total;   msg_total   += (double)fTotal;
                MString msg_memSize; msg_memSize += memSize;
                msg.format(
                    MStringResource::getString(kStatsBuffersMsg, status),
                    name, msg_buffers, msg_avrg,
                    msg_min, msg_max, msg_total, msg_memSize, memUnit);
                result = msg;
            }

            return result;
        }

    private:
        void addSample(MUint64 value, const void* buffer)
        {
            if (fUniqueEntries.insert(buffer).second) {
                fMin = std::min(fMin, value);
                fMax = std::max(fMax, value);
                fTotal += value;
            }

            fInstancedTotal += value;
        };


        boost::unordered_set<const void*> fUniqueEntries;

        MUint64         fMin;
        MUint64         fMax;
        MUint64         fTotal;
        const MUint64   fBytesPerUnit;

        // Total number of instanced geometry.
        MUint64         fInstancedTotal;
    };


    //==========================================================================
    // CLASS Stats
    //==========================================================================

    class Stats
    {
    public:
        Stats()
            : fNbNodes(0),
              fNbSubNodes(0),
              fWires(     2 * sizeof(IndexBuffer::index_t)),
              fTriangles( 3 * sizeof(IndexBuffer::index_t)),
              fVerts(     3 * sizeof(float)),
              fNormals(   3 * sizeof(float)),
              fUVs(       2 * sizeof(float)),
              fVP2Index(  sizeof(IndexBuffer::index_t)),
              fVP2Vertex( sizeof(float)),
              fVBOIndex(  sizeof(IndexBuffer::index_t)),
              fVBOVertex( sizeof(float)),
              fNbMaterialGraphs(0),
              fNbMaterialNodes(0)
        {}

        void accumulateNode()
        {
            ++fNbNodes;
        }

        void accumulateMaterialGraph(const MaterialGraph::Ptr&)
        {
            ++fNbMaterialGraphs;
        }

        void accumulateMaterialNode(const MaterialNode::Ptr&)
        {
            ++fNbMaterialNodes;
        }

        void accumulate(const ShapeData& shape)
        {
            ++fNbSubNodes;

            BOOST_FOREACH(const ShapeData::SampleMap::value_type& v,
                          shape.getSamples()) {
                accumSample(v.second);
            }
        }

        void accumulate(const ShapeData& shape,
                        MTime time)
        {
            ++fNbSubNodes;
            accumSample(shape.getSample(time));
        }

        void print(MStringArray& result, bool printInstancedInfo) const
        {
            MStatus status;

            {
                MString msg;
                MString msg_nbGeom;     msg_nbGeom     += fNbNodes;
                MString msg_nbSubNodes; msg_nbSubNodes += fNbSubNodes;
                msg.format(
                    MStringResource::getString(kStatsNbGeomMsg, status),
                    msg_nbGeom, msg_nbSubNodes);
                result.append(msg);
            }

            result.append(fWires.print(    MStringResource::getString(kStatsWiresMsg,     status)));
            result.append(fTriangles.print(MStringResource::getString(kStatsTrianglesMsg, status)));
            result.append(fVerts.print(    MStringResource::getString(kStatsVerticesMsg,  status)));
            result.append(fNormals.print(  MStringResource::getString(kStatsNormalsMsg,   status)));
            result.append(fUVs.print(      MStringResource::getString(kStatsUVsMsg,       status)));

            if (printInstancedInfo) {
                MString msg;
                MString msgInstWires; msgInstWires += (double)fWires.getInstancedTotal();
                MString msgInstTris;  msgInstTris  += (double)fTriangles.getInstancedTotal();
                msg.format(
                    MStringResource::getString(kStatsTotalInstancedMsg, status),
                    msgInstWires, msgInstTris);
                result.append(msg);
            }

            {
                MUint64 totalMem = (fWires.getSize() +
                                    fTriangles.getSize() +
                                    fVerts.getSize() +
                                    fNormals.getSize() +
                                    fUVs.getSize());

                MString memUnit;
                double  memSize = toHumanUnits(totalMem, memUnit);

                MString msg;
                MString msg_memSize; msg_memSize += memSize;
                msg.format(
                    MStringResource::getString(kStatsSystemTotalMsg, status),
                    msg_memSize, memUnit);
                result.append(msg);
            }
            {
                MUint64 totalMem = (fVBOIndex.getSize() + fVBOVertex.getSize());
                result.append(fVBOIndex.print( MStringResource::getString(kStatsVBOIndexMsg,  status)));
                result.append(fVBOVertex.print(MStringResource::getString(kStatsVBOVertexMsg, status)));

                if (Config::vp2OverrideAPI() != Config::kMPxDrawOverride) {
                    result.append(fVP2Index.print( MStringResource::getString(kStatsVP2IndexMsg,  status)));
                    result.append(fVP2Vertex.print(MStringResource::getString(kStatsVP2VertexMsg, status)));
                    totalMem += (fVP2Index.getSize() + fVP2Vertex.getSize());
                }

                MString memUnit;
                double  memSize = toHumanUnits(totalMem, memUnit);

                MString msg;
                MString msg_memSize; msg_memSize += memSize;
                msg.format(
                    MStringResource::getString(kStatsVideoTotalMsg, status),
                    msg_memSize, memUnit);
                result.append(msg);
            }
            {
                MString msg_nbGraphs;
                msg_nbGraphs += fNbMaterialGraphs;

                MString msg_nbNodes;
                msg_nbNodes  += fNbMaterialNodes;

                MString msg;
                msg.format(
                    MStringResource::getString(kStatsMaterialsMsg, status),
                    msg_nbGraphs, msg_nbNodes);
                result.append(msg);
            }
        }

    private:
        /*----- member functions -----*/

        void accumSample(const boost::shared_ptr<const ShapeSample>& sample) {
            accumIndexBuffer(fWires, sample->wireVertIndices(), 2);
            for(size_t i=0; i<sample->numIndexGroups(); ++i) {
                accumIndexBuffer(fTriangles, sample->triangleVertIndices(i), 3);
            }
            accumVertexBuffer(fVerts, sample->positions());
            accumVertexBuffer(fNormals, sample->normals());
            accumVertexBuffer(fUVs, sample->uvs());
        }

        void accumIndexBuffer(
            Stat& stat,
            const boost::shared_ptr<const IndexBuffer> indexBuffer,
            const int indicesPerElem
        )
        {
            if (indexBuffer && indexBuffer != UnitBoundingBox::indices()) {
                stat.addSample(indexBuffer, indicesPerElem);

                {
                    MHWRender::MIndexBuffer* vp2Buffer = 
                        SubSceneOverride::lookup(indexBuffer);
                    if (vp2Buffer)
                        fVP2Index.addSample(vp2Buffer, indexBuffer->numIndices());
                }

                {
                    boost::shared_ptr<const VBOBuffer> vboBuffer =
                        VBOBuffer::lookup(indexBuffer);
                    if (vboBuffer)
                        fVBOIndex.addSample(vboBuffer, indexBuffer->numIndices());
                }
            }
        }

        void accumVertexBuffer(
            Stat& stat,
            const boost::shared_ptr<const VertexBuffer> vertexBuffer
        )
        {
            if (vertexBuffer && vertexBuffer != UnitBoundingBox::positions()) {
                stat.addSample(vertexBuffer);

                {
                    MHWRender::MVertexBuffer* vp2Buffer = 
                        SubSceneOverride::lookup(vertexBuffer);
                    if (vp2Buffer)
                        fVP2Vertex.addSample(
                            vp2Buffer, 3 * vertexBuffer->numVerts());
                }

                {
                    boost::shared_ptr<const VBOBuffer> vboBuffer =
                        VBOBuffer::lookup(vertexBuffer);
                    if (vboBuffer)
                        fVBOVertex.addSample(
                            vboBuffer, 3 * vertexBuffer->numVerts());
                }

                {
                    boost::shared_ptr<const VBOBuffer> vboBuffer =
                        VBOBuffer::lookupFlippedNormals(vertexBuffer);
                    if (vboBuffer)
                        fVBOVertex.addSample(
                            vboBuffer, 3 * vertexBuffer->numVerts());
                }
            }
        }


        /*----- data members -----*/

        int  fNbNodes;
        int  fNbSubNodes;

        Stat fWires;
        Stat fTriangles;
        Stat fVerts;
        Stat fNormals;
        Stat fUVs;

        Stat fVP2Index;
        Stat fVP2Vertex;
        Stat fVBOIndex;
        Stat fVBOVertex;

        int  fNbMaterialGraphs;
        int  fNbMaterialNodes;
    };


    //==========================================================================
    // CLASS StatsVisitor
    //==========================================================================

    class StatsVisitor : public SubNodeVisitor
    {
    public:

        StatsVisitor() : fAtGivenTime(false) {}
        StatsVisitor(MTime time) : fAtGivenTime(true), fTime(time) {}

        void accumulateNode(const SubNode::Ptr& topNode)
        {
            fStats.accumulateNode();

            if (topNode) {
                topNode->accept(*this);
            }
        }

        void accumulateMaterialGraph(const MaterialGraphMap::Ptr& materials)
        {
            if (materials) {
                BOOST_FOREACH (
                        const MaterialGraphMap::NamedMap::value_type& val,
                        materials->getGraphs()) {
                    fStats.accumulateMaterialGraph(val.second);
                    accumulateMaterialNode(val.second);
                }
            }
        }

        void accumulateMaterialNode(const MaterialGraph::Ptr& material)
        {
            if (material) {
                BOOST_FOREACH (
                        const MaterialGraph::NamedMap::value_type& val,
                        material->getNodes()) {
                    fStats.accumulateMaterialNode(val.second);
                }
            }
        }


        void print(MStringArray& result, bool printInstancedInfo) const
        {
            fStats.print(result, printInstancedInfo);
        }


    private:

        virtual void visit(const XformData&   /*xform*/,
                           const SubNode&     subNode)
        {
            // Recurse into children sub nodes. Expand all instances.
            BOOST_FOREACH(const SubNode::Ptr& child,
                          subNode.getChildren() ) {
                child->accept(*this);
            }
        }

        virtual void visit(const ShapeData&   shape,
                           const SubNode&     /*subNode*/)
        {
            if (fAtGivenTime) {
                fStats.accumulate(shape, fTime);

            }
            else {
                fStats.accumulate(shape);
            }
        }

        const bool  fAtGivenTime;
        const MTime fTime;

        Stats fStats;
    };


    //==========================================================================
    // CLASS DumpHierarchyVisitor
    //==========================================================================

    class DumpHierarchyVisitor : public SubNodeVisitor
    {
    public:

        DumpHierarchyVisitor(MStringArray& result)
            : fResult(result),
              fLevel(0)
        {}

        virtual void visit(const XformData&   xform,
                           const SubNode&     subNode)
        {
            using namespace std;

            {
                ostringstream tmp;
                tmp << setw(kIndent*fLevel) << ' '
                    << "xform name = " << subNode.getName().asChar()
                    << ", tt = "  << subNode.transparentType()
                    << ", ptr = " << (void*)&subNode
                    << " {" << ends;
                fResult.append(MString(tmp.str().c_str()));
            }

            ++fLevel;
            {
                std::string indent(kIndent*fLevel, ' ');

                BOOST_FOREACH(const XformData::SampleMap::value_type& sample,
                              xform.getSamples()) {
                    ostringstream tmp;
                    tmp << setw(kIndent*fLevel) << ' '
                        << "time = " << setw(10) << sample.first
                        << ", ptr = " << (void*)sample.second.get()
                        << ", vis = " << sample.second->visibility()
                        << ", bbox = ("
                        << setw(8) << sample.second->boundingBox().min().x << ","
                        << setw(8) << sample.second->boundingBox().min().y << ","
                        << setw(8) << sample.second->boundingBox().min().z << ") - ("
                        << setw(8) << sample.second->boundingBox().max().x << ","
                        << setw(8) << sample.second->boundingBox().max().y << ","
                        << setw(8) << sample.second->boundingBox().max().z << ")"
                        << ends;
                    fResult.append(MString(tmp.str().c_str()));
                }

                // Recurse into children sub nodes. Expand all instances.
                BOOST_FOREACH(const SubNode::Ptr& child,
                              subNode.getChildren() ) {
                    child->accept(*this);
                }
            }
            --fLevel;

            {
                ostringstream tmp;
                tmp << setw(kIndent*fLevel) << ' '
                    << "}" << ends;
                fResult.append(MString(tmp.str().c_str()));
            }
        }

        virtual void visit(const ShapeData&   shape,
                           const SubNode&     subNode)
        {
            using namespace std;

            {
                ostringstream tmp;
                tmp << setw(kIndent*fLevel) << ' '
                    << "shape name = " << subNode.getName().asChar()
                    << ", tt = "  << subNode.transparentType()
                    << ", ptr = " << (void*)&subNode
                    << " {" << ends;
                fResult.append(MString(tmp.str().c_str()));
            }

            ++fLevel;
            {
                std::string indent(kIndent*fLevel, ' ');

                BOOST_FOREACH(const ShapeData::SampleMap::value_type& sample,
                              shape.getSamples()) {
                    {
                        ostringstream tmp;
                        tmp << setw(kIndent*fLevel) << ' '
                            << "time = " << setw(10) << sample.first
                            << ", ptr = " << (void*)sample.second.get()
                            << ", vis = " << sample.second->visibility()
                            << ", nT = " << sample.second->numTriangles()
                            << ", nW = " << sample.second->numWires()
                            << ", nV = " << sample.second->numVerts()
                            << "," << ends;
                        fResult.append(MString(tmp.str().c_str()));
                    }
                    {
                        ostringstream tmp;
                        tmp << setw(kIndent*fLevel) << ' '
                            << "P = " << (void*)sample.second->positions().get()
                            << ", N = " << (void*)sample.second->normals().get()
                            << "," << ends;
                        fResult.append(MString(tmp.str().c_str()));
                    }
                    {
                        ostringstream tmp;
                        tmp << setw(kIndent*fLevel) << ' '
                            << "C = ("
                            << setw(8) << sample.second->diffuseColor()[0] << ","
                            << setw(8) << sample.second->diffuseColor()[1] << ","
                            << setw(8) << sample.second->diffuseColor()[2] << ","
                            << setw(8) << sample.second->diffuseColor()[3] << ","
                            << "), bbox = ("
                            << setw(8) << sample.second->boundingBox().min().x << ","
                            << setw(8) << sample.second->boundingBox().min().y << ","
                            << setw(8) << sample.second->boundingBox().min().z << ") - ("
                            << setw(8) << sample.second->boundingBox().max().x << ","
                            << setw(8) << sample.second->boundingBox().max().y << ","
                            << setw(8) << sample.second->boundingBox().max().z << ")"
                            << ends;
                        fResult.append(MString(tmp.str().c_str()));
                    }
                    {
                        ostringstream tmp;
                        tmp << setw(kIndent*fLevel) << ' '
                            << "bbox place holder = " << (sample.second->isBoundingBoxPlaceHolder() ? "yes" : "no")
                            << ends;
                        fResult.append(MString(tmp.str().c_str()));
                    }
                }
            }
            if (!shape.getMaterials().empty()) {
                ostringstream tmp;
                tmp << setw(kIndent*fLevel) << ' '
                    << "materials = ";
                BOOST_FOREACH (const MString& material, shape.getMaterials()) {
                    tmp << material.asChar() << ' ';
                }
                tmp << ends;
                fResult.append(MString(tmp.str().c_str()));
            }
            --fLevel;

            {
                ostringstream tmp;
                tmp << setw(kIndent*fLevel) << ' '
                    << "}" << ends;
                fResult.append(MString(tmp.str().c_str()));
            }
        }

    private:

        static const int kIndent = 2;

        MStringArray& fResult;
        int           fLevel;
    };


    //==========================================================================
    // CLASS DumpMaterialVisitor
    //==========================================================================

    class DumpMaterialVisitor
    {
    public:
        DumpMaterialVisitor(MStringArray& result)
            : fResult(result),
              fLevel(0)
        {}

        void dumpMaterials(const MaterialGraphMap::Ptr& materials)
        {
            using namespace std;

            BOOST_FOREACH (const MaterialGraphMap::NamedMap::value_type& val, materials->getGraphs())
            {
                const MaterialGraph::Ptr& graph = val.second;

                {
                    ostringstream tmp;
                    tmp << setw(kIndent*fLevel) << ' '
                        << "material graph name = " << graph->name().asChar()
                        << ", nNodes = " << graph->getNodes().size()
                        << ", ptr = " << (void*)graph.get()
                        << " {" << ends;
                    fResult.append(MString(tmp.str().c_str()));
                }

                ++fLevel;
                BOOST_FOREACH (const MaterialGraph::NamedMap::value_type& val, graph->getNodes())
                {
                    dumpMaterialNode(val.second);
                }
                --fLevel;

                {
                    ostringstream tmp;
                    tmp << setw(kIndent*fLevel) << ' '
                        << "}" << ends;
                    fResult.append(MString(tmp.str().c_str()));
                }
            }
        }

        void dumpMaterialNode(const MaterialNode::Ptr& node)
        {
            using namespace std;

            {
                ostringstream tmp;
                tmp << setw(kIndent*fLevel) << ' '
                    << "material node name = " << node->name().asChar()
                    << ", type = " << node->type()
                    << ", ptr = " << (void*)node.get()
                    << " {" << ends;
                fResult.append(MString(tmp.str().c_str()));
            }

            ++fLevel;
            BOOST_FOREACH (const MaterialNode::PropertyMap::value_type& val, node->properties())
            {
                dumpMaterialProperty(val.second);
            }
            --fLevel;

            {
                ostringstream tmp;
                tmp << setw(kIndent*fLevel) << ' '
                    << "}" << ends;
                fResult.append(MString(tmp.str().c_str()));
            }
        }

        void dumpMaterialProperty(const MaterialProperty::Ptr& prop)
        {
            using namespace std;

            {
                ostringstream tmp;
                tmp << setw(kIndent*fLevel) << ' '
                    << "prop name = " << prop->name().asChar()
                    << ", type = " << propertyTypeString(prop)
                    << ", ptr = " << (void*)prop.get()
                    << " {" << ends;
                fResult.append(MString(tmp.str().c_str()));
            }

            ++fLevel;
            BOOST_FOREACH (const MaterialProperty::SampleMap::value_type& val, prop->getSamples())
            {
                ostringstream tmp;
                tmp << setw(kIndent*fLevel) << ' '
                    << "time = " << setw(10) << val.first
                    << ", value = " << propertyValueString(val.first, prop)
                    << ", ptr = " << (void*)val.second.get()
                    << ends;
                fResult.append(MString(tmp.str().c_str()));
            }

            const MaterialNode::Ptr     srcNode = prop->srcNode();
            const MaterialProperty::Ptr srcProp = prop->srcProp();
            if (srcNode && srcProp) {
                ostringstream tmp;
                tmp << setw(kIndent*fLevel) << ' '
                    << "src node = " << srcNode->name().asChar()
                    << ", src prop = " << srcProp->name().asChar()
                    << ends;
                fResult.append(MString(tmp.str().c_str()));
            }
            --fLevel;

            {
                ostringstream tmp;
                tmp << setw(kIndent*fLevel) << ' '
                    << "}" << ends;
                fResult.append(MString(tmp.str().c_str()));
            }
        }

        std::string propertyTypeString(const MaterialProperty::Ptr& prop)
        {
            switch (prop->type()) {
            case MaterialProperty::kBool:   return "bool";
            case MaterialProperty::kInt32:  return "int32";
            case MaterialProperty::kFloat:  return "float";
            case MaterialProperty::kFloat2: return "float2";
            case MaterialProperty::kFloat3: return "float3";
            case MaterialProperty::kRGB:    return "rgb";
            case MaterialProperty::kString: return "string";
            default:             assert(0); return "unknown";
            }
        }

        std::string propertyValueString(double seconds, const MaterialProperty::Ptr& prop)
        {
            std::ostringstream tmp;
            switch (prop->type()) {
            case MaterialProperty::kBool:
                tmp << (prop->asBool(seconds) ? "true" : "false");
                break;
            case MaterialProperty::kInt32:
                tmp << prop->asInt32(seconds);
                break;
            case MaterialProperty::kFloat:
                tmp << prop->asFloat(seconds);
                break;
            case MaterialProperty::kFloat2:
                {
                    float x, y;
                    prop->asFloat2(seconds, x, y);
                    tmp << "(" << x << "," << y << ")";
                    break;
                }
            case MaterialProperty::kFloat3:
                {
                    float x, y, z;
                    prop->asFloat3(seconds, x, y, z);
                    tmp << "(" << x << "," << y << "," << z << ")";
                    break;
                }
            case MaterialProperty::kRGB:
                {
                    MColor c = prop->asColor(seconds);
                    tmp << "rgb(" << c.r << "," << c.g << "," << c.b << ")";
                    break;
                }
            case MaterialProperty::kString:
                tmp << prop->asString(seconds).asChar();
                break;
            default:
                assert(0);
                tmp << "unknown type";
                break;
            }
            return tmp.str();
        }

    private:
        static const int kIndent = 2;

        MStringArray& fResult;
        int           fLevel;
    };


    //==============================================================================
    // CLASS ProgressBar
    //==============================================================================

    class ProgressBar
    {
    public:
        ProgressBar(const MStringResourceId& msg, unsigned int max)
        {
            // Display a progress bar if Maya is running in UI mode
            fShowProgress = (MGlobal::mayaState() == MGlobal::kInteractive);
            reset(msg, max);
        }

        void reset(const MStringResourceId& msg, unsigned int max)
        {
            MStatus status;
            beginProgress(MStringResource::getString(msg, status), max);
        }

        ~ProgressBar()
        {
            endProgress();
        }

        void stepProgress() const
        {
            if (fShowProgress) {
                MGlobal::executeCommand("progressBar -e -s 1 $gMainProgressBar");
            }
        }

        bool isCancelled() const
        {
            int isCancelled = 0;
            if (fShowProgress) {
                MGlobal::executeCommand("progressBar -q -ic $gMainProgressBar", isCancelled);
            }

            if (isCancelled) {
                MStatus status;
                const MString interruptMsg = MStringResource::getString(kInterruptedMsg, status);
                MGlobal::displayInfo(interruptMsg);
                return true;
            }
            return false;
        }

    private:
        // Forbidden and not implemented.
        ProgressBar(const ProgressBar&);
        const ProgressBar& operator=(const ProgressBar&);

        ProgressBar(const MString& msg, unsigned int max)
        {
            beginProgress(msg, max);
        }

        void beginProgress(const MString& msg, unsigned int max) const
        {
            if (fShowProgress) {
                MString maxValue, progressBarCmd;

                // Progress from 0 to max
                if (max <= 0) {
                    max = 1;
                }
                maxValue += max;

                // Clear previous isCancelled flag
                MGlobal::executeCommand("progressBar -e -bp -ii 1 $gMainProgressBar");
                MGlobal::executeCommand("progressBar -e -ep $gMainProgressBar");

                // Initialize the progress bar
                progressBarCmd.format("progressBar -e -bp -ii 1 -st \"^1s\" -max ^2s $gMainProgressBar",
                    msg, maxValue);
                MGlobal::executeCommand(progressBarCmd);
            }
        }

        void endProgress() const
        {
            if (fShowProgress) {
                MGlobal::executeCommand("progressBar -e -ep $gMainProgressBar");
            }
        }

        bool fShowProgress;  // whether to show the progress bar
    };


    //==============================================================================
    // CLASS GroupCreator
    //==============================================================================

    class GroupCreator
    {
    public:
        GroupCreator() {}
        ~GroupCreator() {}

        void addChild(const SubNode::MPtr& childNode)
        {
            XformData::Ptr childXform = boost::dynamic_pointer_cast<const XformData>(
                    childNode->getData());
            assert(childXform);

            if (childXform) {
                fChildNodes.push_back(childNode);
                fChildXforms.push_back(childXform);
            }
        }

        void group()
        {
            assert(!fGroup);
            fGroup = XformData::create();

            // Collect time samples
            std::set<double> times;
            BOOST_FOREACH(const XformData::Ptr child, fChildXforms) {
                BOOST_FOREACH(const XformData::SampleMap::value_type& val, child->getSamples()) {
                    times.insert(val.first);
                }
            }

            std::set<double>::const_iterator timeIt  = times.begin();
            std::set<double>::const_iterator timeEnd = times.end();

            if (timeIt != timeEnd) {
                fGroup->addSample(XformSample::create(
                    *timeIt,
                    MMatrix::identity,
                    MBoundingBox(),
                    true));
            }
        }

        SubNode::MPtr getSubNode(const MString& name)
        {
            SubNode::MPtr subNode = SubNode::create(name, fGroup);
            BOOST_FOREACH(const SubNode::MPtr& childNode, fChildNodes) {
                SubNode::connect(subNode, childNode);
            }
            return subNode;
        }

    private:
        // Prohibited and not implemented.
        GroupCreator(const GroupCreator&);
        const GroupCreator& operator= (const GroupCreator&);

        std::vector<SubNode::MPtr>   fChildNodes;
        std::vector<XformData::Ptr>  fChildXforms;
        XformData::MPtr              fGroup;
    };


    //==============================================================================
    // CLASS XformFreezer
    //==============================================================================

    class XformFreezer : public SubNodeVisitor
    {
    public:
        typedef std::vector<ShapeData::Ptr>                            FrozenGeometries;
        typedef std::vector<std::pair<XformData::Ptr,ShapeData::Ptr> > AnimatedGeometries;
        typedef std::set<double>                                       TimeSet;

        XformFreezer(const XformData::Ptr& parentXform,
                     FrozenGeometries&     frozenGeometries,
                     bool                  dontFreezeAnimatedObjects,
                     AnimatedGeometries&   animatedGeometries)
            : fParentXform(parentXform),
              fFrozenGeometries(frozenGeometries),
              fDontFreezeAnimatedObjects(dontFreezeAnimatedObjects),
              fAnimatedGeometries(animatedGeometries)
        {}

        virtual void visit(const XformData& xform,
                           const SubNode&   subNode)
        {
            // Aggregate the list of sample times.
            TimeSet times;

            BOOST_FOREACH(const XformData::SampleMap::value_type& val,
                    fParentXform->getSamples() ) {
                times.insert(val.first);
            }

            BOOST_FOREACH(const XformData::SampleMap::value_type& val,
                    xform.getSamples()) {
                times.insert(val.first);
            }

            // Freeze xform sample
            XformData::MPtr frozenXform = XformData::create();
            BOOST_FOREACH(const double& time, times) {
                // Parent xform sample
                boost::shared_ptr<const XformSample> parentSample =
                        fParentXform->getSample(time);
                // Child xform sample
                boost::shared_ptr<const XformSample> sample =
                        xform.getSample(time);

                frozenXform->addSample(XformSample::create(
                    time,
                    sample->xform() * parentSample->xform(),
                    MBoundingBox(),  // not used
                    sample->visibility() && parentSample->visibility()));
            }

            // Recursive into children
            BOOST_FOREACH(const SubNode::Ptr& child, subNode.getChildren()) {
                XformFreezer xformFreezer(frozenXform, fFrozenGeometries,
                    fDontFreezeAnimatedObjects, fAnimatedGeometries);
                child->accept(xformFreezer);
            }
        }

        virtual void visit(const ShapeData& shape,
                           const SubNode&   subNode)
        {
            // Don't freeze animated objects for motion blur.
            if (fDontFreezeAnimatedObjects) {
                // If the shape matches all the following conditions, we don't freeze/consolidate it.
                // 1) Any of the parents (direct,indirect) is animated.
                // 2) Shape is not animated.
                if (fParentXform->getSamples().size() > 1 && shape.getSamples().size() <= 1) {
                    // Duplicate the xform data.
                    XformData::MPtr animatedXform = XformData::create();
                    BOOST_FOREACH (const XformData::SampleMap::value_type& val,
                            fParentXform->getSamples()) {
                        animatedXform->addSample(val.second);
                    }

                    // Duplicate the shape data.
                    ShapeData::MPtr animatedShape = ShapeData::create();
                    BOOST_FOREACH (const ShapeData::SampleMap::value_type& val,
                            shape.getSamples()) {
                        animatedShape->addSample(val.second);
                    }
                    animatedShape->setMaterials(shape.getMaterials());

                    // Give up. We don't freeze and consolidate shapes with
                    // animated xforms.
                    fAnimatedGeometries.push_back(
                        std::make_pair(animatedXform, animatedShape));
                    return;
                }
            }

            // Aggregate the list of sample times.
            TimeSet times;

            BOOST_FOREACH(const XformData::SampleMap::value_type& val,
                    fParentXform->getSamples()) {
                times.insert(val.first);
            }

            BOOST_FOREACH(const ShapeData::SampleMap::value_type& val,
                    shape.getSamples()) {
                times.insert(val.first);
            }

            // Freeze shape sample
            ShapeData::MPtr frozenShape = ShapeData::create();

            TimeSet::const_iterator it  = times.begin();
            TimeSet::const_iterator end = times.end();

            if (it != end) {
                // The first xform and shape sample
                boost::shared_ptr<const XformSample> xformSample =
                        fParentXform->getSample(*it);
                boost::shared_ptr<const ShapeSample> shapeSample =
                        shape.getSample(*it);

                // Freeze the shape sample
                boost::shared_ptr<const ShapeSample> frozenSample;
                if (xformSample->visibility() && shapeSample->visibility()) {
                    frozenSample = freezeSample(*it, xformSample, shapeSample);
                }
                else {
                    frozenSample = ShapeSample::createEmptySample(*it);
                }

                // Add the frozen shape sample
                frozenShape->addSample(frozenSample);
                ++it;

                for (; it != end; ++it) {
                    // Save the previous sample
                    boost::shared_ptr<const XformSample> prevXformSample = xformSample;
                    boost::shared_ptr<const ShapeSample> prevShapeSample = shapeSample;

                    // The next xform and shape sample
                    xformSample = fParentXform->getSample(*it);
                    shapeSample = shape.getSample(*it);

                    if (xformSample->visibility() && shapeSample->visibility()) {
                        if (!xformSample->xform().isEquivalent(prevXformSample->xform()) ||
                            xformSample->visibility() != prevXformSample->visibility() ||
                            shapeSample->wireVertIndices() != prevShapeSample->wireVertIndices() ||
                            shapeSample->triangleVertexIndexGroups() != prevShapeSample->triangleVertexIndexGroups() ||
                            shapeSample->positions() != prevShapeSample->positions() ||
                            shapeSample->normals()   != prevShapeSample->normals() ||
                            shapeSample->diffuseColor() != prevShapeSample->diffuseColor() ||
                            shapeSample->visibility() != prevShapeSample->visibility())
                        {
                            // Something changed, need to re-freeze the shape sample
                            frozenSample = freezeSample(*it, xformSample, shapeSample);
                        }
                        else {
                            // Reuse the last freezeSample() result.
                            boost::shared_ptr<ShapeSample> newFrozenSample =
                                ShapeSample::create(
                                *it,
                                shapeSample->numWires(),
                                shapeSample->numVerts(),
                                shapeSample->wireVertIndices(),
                                shapeSample->triangleVertexIndexGroups(),
                                frozenSample->positions(),
                                frozenSample->boundingBox(),
                                shapeSample->diffuseColor(),
                                xformSample->visibility() && shapeSample->visibility());
                            newFrozenSample->setNormals(frozenSample->normals());
                            newFrozenSample->setUVs(shapeSample->uvs());
                            frozenSample = newFrozenSample;
                        }
                    }
                    else {
                        frozenSample = ShapeSample::createEmptySample(*it);
                    }

                    // Add the frozen shape sample
                    frozenShape->addSample(frozenSample);
                }
            }

            frozenShape->setMaterials(shape.getMaterials());
            fFrozenGeometries.push_back(frozenShape);
        }

    private:
        boost::shared_ptr<const ShapeSample> freezeSample(
            const double time,
            const boost::shared_ptr<const XformSample>& xform,
            const boost::shared_ptr<const ShapeSample>& shape)
        {
            const size_t numWires = shape->numWires();
            const size_t numVerts = shape->numVerts();

            boost::shared_ptr<IndexBuffer> wireVertIndices =
                    shape->wireVertIndices();
            std::vector<boost::shared_ptr<IndexBuffer> > triangleVertexIndexGroups =
                    shape->triangleVertexIndexGroups();
            boost::shared_ptr<VertexBuffer> uvs =
                    shape->uvs();
            MColor diffuseColor = shape->diffuseColor();
            bool visibility = shape->visibility() && xform->visibility();

            // Check bad polys
            if (numWires == 0 || numVerts == 0 || !wireVertIndices || 
                    triangleVertexIndexGroups.empty()) {
                return ShapeSample::createEmptySample(time);
            }

            boost::shared_ptr<VertexBuffer> positions;
            boost::shared_ptr<VertexBuffer> normals;
            MBoundingBox boundingBox;

            MMatrix xformMatrix = xform->xform();
            if (xformMatrix.isEquivalent(MMatrix::identity)) {
                // Nothing to bake for an identity transform.
                positions    = shape->positions();
                normals      = shape->normals();
                boundingBox  = shape->boundingBox();
            }
            else {
                float xform[4][4];
                float xformIT[4][4];
                xformMatrix.get(xform);
                xformMatrix.inverse().transpose().get(xformIT);

                const bool isReflection = xformMatrix.det3x3() < 0.0;
                if (isReflection) {
                    // Change the winding order of the triangles if
                    // the matrix contains a reflection along one the
                    // axis to preserve front facing.

                    std::vector<boost::shared_ptr<IndexBuffer> > newTriangleVertexIndexGroups;
                    BOOST_FOREACH(const boost::shared_ptr<IndexBuffer>& srcIdxBuf,
                                  triangleVertexIndexGroups) {
                        typedef IndexBuffer::index_t index_t;
                        const size_t numIndices = srcIdxBuf->numIndices();
                        IndexBuffer::ReadInterfacePtr readable = srcIdxBuf->readableInterface();
                        const index_t* srcIndices = readable->get();
                        const boost::shared_array<IndexBuffer::index_t> dstIndices(
                            new index_t[numIndices]);
                        for (size_t i=0; i<numIndices; i+=3) {
                            dstIndices[i + 0] = srcIndices[i + 2];
                            dstIndices[i + 1] = srcIndices[i + 1];
                            dstIndices[i + 2] = srcIndices[i + 0];
                        }

                        boost::shared_ptr<IndexBuffer> dstIdxBuf(
                            IndexBuffer::create(
                                SharedArray<index_t>::create(dstIndices, numIndices)));
                        newTriangleVertexIndexGroups.push_back(dstIdxBuf);
                    }

                    triangleVertexIndexGroups.swap(newTriangleVertexIndexGroups);
                }
                
                VertexBuffer::ReadInterfacePtr srcPosRead = shape->positions()->readableInterface();
                const float* srcPositions = srcPosRead->get();
                VertexBuffer::ReadInterfacePtr srcNormRead = shape->normals()->readableInterface();
                const float* srcNormals   = srcNormRead->get();

                boost::shared_array<float> dstPositions(new float[3 * numVerts]);
                boost::shared_array<float> dstNormals(new float[3 * numVerts]);

                float minX = +std::numeric_limits<float>::max();
                float minY = +std::numeric_limits<float>::max();
                float minZ = +std::numeric_limits<float>::max();

                float maxX = -std::numeric_limits<float>::max();
                float maxY = -std::numeric_limits<float>::max();
                float maxZ = -std::numeric_limits<float>::max();


                for (size_t i=0; i<numVerts; ++i) {
                    const float x = srcPositions[3*i + 0];
                    const float y = srcPositions[3*i + 1];
                    const float z = srcPositions[3*i + 2];

                    const float xp =
                        xform[0][0] * x + xform[1][0] * y + xform[2][0] * z + xform[3][0];
                    const float yp =
                        xform[0][1] * x + xform[1][1] * y + xform[2][1] * z + xform[3][1];
                    const float zp =
                        xform[0][2] * x + xform[1][2] * y + xform[2][2] * z + xform[3][2];

                    minX = std::min(xp, minX);
                    minY = std::min(yp, minY);
                    minZ = std::min(zp, minZ);

                    maxX = std::max(xp, maxX);
                    maxY = std::max(yp, maxY);
                    maxZ = std::max(zp, maxZ);

                    dstPositions[3*i + 0] = xp;
                    dstPositions[3*i + 1] = yp;
                    dstPositions[3*i + 2] = zp;

                    const float nx = srcNormals[3*i + 0];
                    const float ny = srcNormals[3*i + 1];
                    const float nz = srcNormals[3*i + 2];

                    dstNormals[3*i + 0] =
                        xformIT[0][0] * nx + xformIT[1][0] * ny + xformIT[2][0] * nz + xformIT[3][0];
                    dstNormals[3*i + 1] =
                        xformIT[0][1] * nx + xformIT[1][1] * ny + xformIT[2][1] * nz + xformIT[3][1];
                    dstNormals[3*i + 2] =
                        xformIT[0][2] * nx + xformIT[1][2] * ny + xformIT[2][2] * nz + xformIT[3][2];
                }

                positions = VertexBuffer::createPositions(
                    SharedArray<float>::create(dstPositions, 3 * numVerts));
                normals   = VertexBuffer::createNormals(
                    SharedArray<float>::create(dstNormals, 3 * numVerts));
                boundingBox = MBoundingBox(MPoint(minX, minY, minZ),
                    MPoint(maxX, maxY, maxZ));
            }

            boost::shared_ptr<ShapeSample> frozenSample =
                ShapeSample::create(
                    time,
                    numWires,
                    numVerts,
                    wireVertIndices,
                    triangleVertexIndexGroups,
                    positions,
                    boundingBox,
                    diffuseColor,
                    visibility);
            frozenSample->setNormals(normals);
            frozenSample->setUVs(uvs);
            return frozenSample;
        }

        // Prohibited and not implemented.
        XformFreezer(const XformFreezer&);
        const XformFreezer& operator= (const XformFreezer&);

        XformData::Ptr      fParentXform;
        FrozenGeometries&   fFrozenGeometries;
        AnimatedGeometries& fAnimatedGeometries;
        bool                fDontFreezeAnimatedObjects;
    };


    //==============================================================================
    // CLASS ConsolidateBuckets
    //==============================================================================

    class ConsolidateBuckets
    {
    public:
        struct BucketKey
        {
            typedef std::map<double,MColor> DiffuseColorMap;
            typedef std::map<double,bool>   VisibilityMap;
            typedef std::map<double,size_t> IndexGroupMap;
            typedef std::vector<MString>    MaterialsAssignment;

            BucketKey(const ShapeData::Ptr& shape)
            {
                ShapeData::SampleMap::const_iterator it  = shape->getSamples().begin();
                ShapeData::SampleMap::const_iterator end = shape->getSamples().end();

                if (it != end) {
                    MColor diffuseColor = (*it).second->diffuseColor();
                    bool   visibility   = (*it).second->visibility();
                    size_t indexGroups  = (*it).second->numIndexGroups();

                    fDiffuseColor[(*it).first] = diffuseColor;
                    fVisibility[(*it).first]    = visibility;
                    fIndexGroup[(*it).first]   = indexGroups;
                    ++it;

                    for (; it != end; ++it) {
                        MColor prevDiffuseColor = diffuseColor;
                        bool   prevVisibility   = visibility;
                        size_t prevIndexGroups  = indexGroups;

                        MColor diffuseColor = (*it).second->diffuseColor();
                        bool   visibility   = (*it).second->visibility();
                        size_t indexGroups  = (*it).second->numIndexGroups();

                        if (prevDiffuseColor != diffuseColor) {
                            fDiffuseColor[(*it).first] = diffuseColor;
                        }
                        if (prevVisibility != visibility) {
                            fVisibility[(*it).first]    = visibility;
                        }
                        if (prevIndexGroups != indexGroups) {
                            fIndexGroup[(*it).first]   = indexGroups;
                        }
                    }
                }

                fMaterials = shape->getMaterials();
            }

            struct Hash : std::unary_function<BucketKey, std::size_t>
            {
                std::size_t operator()(const BucketKey& key) const
                {
                    std::size_t seed = 0;
                    BOOST_FOREACH(const DiffuseColorMap::value_type& val, key.fDiffuseColor) {
                        boost::hash_combine(seed, val.first);
                        boost::hash_combine(seed, val.second.r);
                        boost::hash_combine(seed, val.second.g);
                        boost::hash_combine(seed, val.second.b);
                        boost::hash_combine(seed, val.second.a);
                    }
                    BOOST_FOREACH(const VisibilityMap::value_type& val, key.fVisibility) {
                        boost::hash_combine(seed, val.first);
                        boost::hash_combine(seed, val.second);
                    }
                    BOOST_FOREACH(const IndexGroupMap::value_type& val, key.fIndexGroup) {
                        boost::hash_combine(seed, val.first);
                        boost::hash_combine(seed, val.second);
                    }
                    BOOST_FOREACH(const MString& material, key.fMaterials) {
                        unsigned int length = material.length();
                        const char* begin = material.asChar();
                        size_t hash = boost::hash_range(begin, begin + length);
                        boost::hash_combine(seed, hash);
                    }
                    return seed;
                }
            };

            struct EqualTo : std::binary_function<BucketKey, BucketKey, bool>
            {
                bool operator()(const BucketKey& x, const BucketKey& y) const
                {
                    return x.fDiffuseColor == y.fDiffuseColor &&
                            x.fVisibility == y.fVisibility &&
                            x.fIndexGroup == y.fIndexGroup &&
                            x.fMaterials == y.fMaterials;
                }
            };

            DiffuseColorMap     fDiffuseColor;
            VisibilityMap       fVisibility;
            IndexGroupMap       fIndexGroup;
            MaterialsAssignment fMaterials;
        };

        typedef std::multimap<size_t,ShapeData::Ptr>  Bucket;
        typedef boost::unordered_map<BucketKey,Bucket,BucketKey::Hash,BucketKey::EqualTo> BucketMap;
        typedef std::list<Bucket> BucketList;

        ConsolidateBuckets(const XformFreezer::FrozenGeometries& shapes)
            : fShapes(shapes)
        {}

        void divide()
        {
            BOOST_FOREACH(const ShapeData::Ptr& shape , fShapes) {
                BucketKey key(shape);
                std::pair<BucketMap::iterator,bool> ret =
                        fBucketMap.insert(std::make_pair(key, Bucket()));
                ret.first->second.insert(std::make_pair(maxNumVerts(shape), shape));
            }
        }

        void getBucketList(BucketList& bucketList)
        {
            bucketList.clear();
            BOOST_FOREACH(const BucketMap::value_type& val, fBucketMap) {
                bucketList.push_back(val.second);
            }
        }

    private:
        // Prohibited and not implemented.
        ConsolidateBuckets(const ConsolidateBuckets&);
        const ConsolidateBuckets& operator= (const ConsolidateBuckets&);

        const XformFreezer::FrozenGeometries& fShapes;
        BucketMap                             fBucketMap;
    };


    //==============================================================================
    // CLASS FirstSampleTime
    //==============================================================================

    class FirstSampleTime : public SubNodeVisitor
    {
    public:
        FirstSampleTime()
            : fTime(0)
        {}

        virtual void visit(const XformData& xform,
                           const SubNode&   subNode)
        {
            fTime = xform.getSamples().begin()->first;
        }

        virtual void visit(const ShapeData& shape,
                           const SubNode&   subNode)
        {
            fTime = shape.getSamples().begin()->first;
        }

        double get()
        { return fTime; }

    private:
        // Prohibited and not implemented.
        FirstSampleTime(const FirstSampleTime&);
        const FirstSampleTime& operator= (const FirstSampleTime&);

        double fTime;
    };


    //==============================================================================
    // CLASS Consolidator
    //==============================================================================

    class Consolidator
    {
    public:
        Consolidator(SubNode::MPtr rootNode, const int threshold, const bool motionBlur)
            : fRootNode(rootNode), fThreshold(threshold), fMotionBlur(motionBlur)
        {}

        ~Consolidator()
        {}

        MStatus consolidate()
        {
            // We currently unconditionally expand all instances. This is kind
            // of brute force as it assumes that the instances have a low poly
            // count so that consolidating them is worthwhile and that also
            // the instance count is low so that the data expansion is
            // reasonable.
            //
            // FIXME: Obviously, a more intelligent heuristic would be needed
            // at one point.

            // Get the time of the first sample, useful when creating new xform
            // samples.
            double firstSampleTime = 0;
            {
                FirstSampleTime firstSampleTimeVisitor;
                fRootNode->accept(firstSampleTimeVisitor);

                firstSampleTime = firstSampleTimeVisitor.get();
            }

            // Freeze transforms.
            XformFreezer::FrozenGeometries   frozenGeometries;
            XformFreezer::AnimatedGeometries animatedGeometries;
            {
                // Create an dummy identity xform data as the root of traversal
                XformData::MPtr identityXformData = XformData::create();
                identityXformData->addSample(XformSample::create(
                    firstSampleTime,
                    MMatrix::identity,
                    MBoundingBox(),  // not used when freeze transform
                    true));

                // Traversal the hierarchy to freeze transforms
                XformFreezer xformFreezer(identityXformData, frozenGeometries,
                    fMotionBlur, animatedGeometries);
                fRootNode->accept(xformFreezer);
            }

            // Divide shapes into buckets
            ConsolidateBuckets::BucketList bucketList;
            {
                ConsolidateBuckets buckets(frozenGeometries);
                buckets.divide();
                buckets.getBucketList(bucketList);
            }

            // Set up consolidation progress bar
            ProgressBar progressBar(kOptimizingMsg, (unsigned int)frozenGeometries.size());

            // Consolidate each bucket
            std::vector<ShapeData::Ptr> newShapes;
            std::vector<ShapeData::Ptr> consolidatedShapes;

            BOOST_FOREACH(ConsolidateBuckets::Bucket& bucket, bucketList) {

                // Consolidate shapes until the bucket becomes empty
                while (!bucket.empty()) {
                    const ConsolidateBuckets::Bucket::iterator largestNode = --bucket.end();
                    MInt64 numRemainingVerts = fThreshold - largestNode->first;

                    if (numRemainingVerts < 0) {
                        // Already too large to be consolidated.
                        newShapes.push_back(largestNode->second);
                        bucket.erase(largestNode);

                        MUpdateProgressAndCheckInterruption(progressBar);
                    }
                    else {
                        // Find nodes that could make up a consolidation group.
                        consolidatedShapes.push_back(largestNode->second);
                        bucket.erase(largestNode);

                        MUpdateProgressAndCheckInterruption(progressBar);

                        while (numRemainingVerts > 0 && !bucket.empty()) {
                            ConsolidateBuckets::Bucket::iterator node =
                                    bucket.upper_bound((size_t)numRemainingVerts);
                            if (node == bucket.begin()) break;
                            --node;
                            numRemainingVerts -= (MInt64)node->first;
                            consolidatedShapes.push_back(node->second);
                            bucket.erase(node);

                            MUpdateProgressAndCheckInterruption(progressBar);
                        }

                        // Consolidate the consolidation group
                        consolidateGeometry(newShapes, consolidatedShapes);
                    }
                }
            }

            // Attach a xform data to each new shape data
            std::vector<XformData::Ptr> newXforms;

            BOOST_FOREACH(const ShapeData::Ptr& newShape, newShapes) {
                XformData::MPtr newXform = XformData::create();

                ShapeData::SampleMap::const_iterator it  = newShape->getSamples().begin();
                ShapeData::SampleMap::const_iterator end = newShape->getSamples().end();

                if (it != end) {
                    newXform->addSample(XformSample::create(
                        (*it).first,
                        MMatrix::identity,
                        MBoundingBox(),
                        true));
                }

                newXforms.push_back(newXform);
            }

            // Build a vector of all nodes (consolidated + animated nodes).
            std::vector<std::pair<XformData::Ptr,ShapeData::Ptr> > finalXformsAndShapes;
            for (size_t i = 0; i < newXforms.size(); i++) {
                finalXformsAndShapes.push_back(std::make_pair(newXforms[i], newShapes[i]));
            }
            finalXformsAndShapes.insert(finalXformsAndShapes.end(),
                animatedGeometries.begin(), animatedGeometries.end());

            // Done
            if (finalXformsAndShapes.size() == 1) {
                // Only one shape, use its xform node as the consolidation root
                SubNode::MPtr xformNode = SubNode::create(
                    fRootNode->getName(),
                    finalXformsAndShapes[0].first);
                SubNode::MPtr shapeNode = SubNode::create(
                    fRootNode->getName() + "Shape",
                    finalXformsAndShapes[0].second);
                SubNode::connect(xformNode, shapeNode);

                fConsolidatedRootNode = xformNode;
            }
            else if (finalXformsAndShapes.size() > 1) {
                // There are more than one shape
                // We create one more xform node as the consolidation root
                XformData::MPtr topXform = XformData::create();

                std::set<double> times;
                for (size_t i = 0; i < finalXformsAndShapes.size(); i++) {
                    BOOST_FOREACH(const XformData::SampleMap::value_type& val,
                            finalXformsAndShapes[i].first->getSamples()) {
                        times.insert(val.first);
                    }
                    BOOST_FOREACH(const ShapeData::SampleMap::value_type& val,
                            finalXformsAndShapes[i].second->getSamples()) {
                        times.insert(val.first);
                    }
                }

                std::set<double>::const_iterator timeIt  = times.begin();
                std::set<double>::const_iterator timeEnd = times.end();

                if (timeIt != timeEnd) {
                    topXform->addSample(XformSample::create(
                        *timeIt,
                        MMatrix::identity,
                        MBoundingBox(),
                        true));
                }

                SubNode::MPtr topXformNode = SubNode::create(
                    fRootNode->getName(),
                    topXform);

                // Create shapes' parent xform sub nodes.
                // They are children of the consolidation root.
                for (size_t i = 0; i < finalXformsAndShapes.size(); i++) {
                    SubNode::MPtr xformNode = SubNode::create(
                        fRootNode->getName() + (i + 1),
                        finalXformsAndShapes[i].first);
                    SubNode::MPtr shapeNode = SubNode::create(
                        fRootNode->getName() + "Shape" + (i + 1),
                        finalXformsAndShapes[i].second);
                    SubNode::connect(xformNode, shapeNode);
                    SubNode::connect(topXformNode, xformNode);
                }

                fConsolidatedRootNode = topXformNode;
            }

            return MS::kSuccess;
        }

        SubNode::MPtr consolidatedRootNode()
        { return fConsolidatedRootNode; }

    private:
        // Prohibited and not implemented.
        Consolidator(const Consolidator&);
        const Consolidator& operator= (const Consolidator&);

        void consolidateGeometry(std::vector<ShapeData::Ptr>& newShapes,
                                 std::vector<ShapeData::Ptr>& consolidatedShapes)
        {

            // Aggregate the list of sample times.
            std::set<double> times;
            BOOST_FOREACH(const ShapeData::Ptr& shape, consolidatedShapes) {
                BOOST_FOREACH(const ShapeData::SampleMap::value_type& smv,
                              shape->getSamples()) {
                    times.insert(smv.first);
                }
            }

            // Consolidated geometry.
            ShapeData::MPtr newShape = ShapeData::create();

            const size_t nbShapes = consolidatedShapes.size();

            std::set<double>::const_iterator timeIt  = times.begin();
            std::set<double>::const_iterator timeEnd = times.end();

            // Consolidate the first sample.
            typedef IndexBuffer::index_t index_t;

            boost::shared_array<index_t>               wireVertIndices;
            std::vector<boost::shared_array<index_t> > triangleVertIndices;

            boost::shared_array<float>   positions;
            boost::shared_array<float>   normals;
            boost::shared_array<float>   uvs;
            MBoundingBox boundingBox;

            MColor diffuseColor;
            bool   visibility = true;

            {
                size_t totalWires = 0;
                size_t totalVerts = 0;
                std::vector<size_t> totalTriangles;
                size_t numIndexGroups = 0;

                bool uvExists     = false;

                for (size_t i = 0; i < nbShapes; i++) {
                    ShapeData::Ptr& shape = consolidatedShapes[i];

                    const boost::shared_ptr<const ShapeSample>& sample =
                        shape->getSample(*timeIt);

                    totalWires += sample->numWires();
                    totalVerts += sample->numVerts();

                    if (numIndexGroups == 0) {
                        // Initialize totalTriangles, assume that
                        // all shapes has the same number of index groups
                        numIndexGroups = sample->numIndexGroups();
                        totalTriangles.resize(numIndexGroups, 0);

                        diffuseColor = sample->diffuseColor();
                        visibility   = sample->visibility();
                    }
                    // Shapes with different number of index groups, diffuseColor and visibility
                    // should be divided into separate buckets.
                    assert(numIndexGroups == sample->numIndexGroups());
                    assert(fabs(diffuseColor.r - sample->diffuseColor().r) < 1e-5);
                    assert(fabs(diffuseColor.g - sample->diffuseColor().g) < 1e-5);
                    assert(fabs(diffuseColor.b - sample->diffuseColor().b) < 1e-5);
                    assert(fabs(diffuseColor.a - sample->diffuseColor().a) < 1e-5);
                    assert(visibility == sample->visibility());

                    for (size_t j = 0; j < totalTriangles.size(); j++) {
                        totalTriangles[j] += sample->numTriangles(j);
                    }

                    // Check whether UV exists
                    if (!uvExists && sample->uvs()) {
                        uvExists = true;
                    }
                }

                wireVertIndices = boost::shared_array<index_t>(
                    new index_t[2 * totalWires]);

                triangleVertIndices.resize(totalTriangles.size());
                for (size_t i = 0; i < totalTriangles.size(); i++) {
                    triangleVertIndices[i] = boost::shared_array<index_t>(
                        new index_t[3 * totalTriangles[i]]);
                }

                positions = boost::shared_array<float>(
                    new float[3 * totalVerts]);
                normals = boost::shared_array<float>(
                    new float[3 * totalVerts]);
                if (uvExists) {
                    uvs = boost::shared_array<float>(
                        new float[2 * totalVerts]);
                }

                {
                    size_t wireIdx = 0;
                    size_t vertIdx = 0;
                    std::vector<size_t> triangleIdx(numIndexGroups, 0);

                    for (size_t i = 0; i < nbShapes; i++) {
                        const boost::shared_ptr<const ShapeSample>& sample =
                            consolidatedShapes[i]->getSample(*timeIt);

                        const size_t numWires     = sample->numWires();
                        const size_t numVerts     = sample->numVerts();

                        // Wires
                        if (sample->wireVertIndices()) {
                            IndexBuffer::ReadInterfacePtr readable = sample->wireVertIndices()->readableInterface();
                            const index_t* srcWireVertIndices = readable->get();
                            for (size_t j = 0; j < numWires; j++) {
                                wireVertIndices[2*(j + wireIdx) + 0] = index_t(srcWireVertIndices[2*j + 0] + vertIdx);
                                wireVertIndices[2*(j + wireIdx) + 1] = index_t(srcWireVertIndices[2*j + 1] + vertIdx);
                            }
                        }

                        // Triangles
                        for (size_t group = 0; group < numIndexGroups; group++) {
                            const size_t numTriangles = sample->numTriangles(group);
                            if (sample->triangleVertIndices(group)) {
                                IndexBuffer::ReadInterfacePtr readable = sample->triangleVertIndices(group)->readableInterface();
                                const index_t* srcTriangleVertIndices = readable->get();
                                for (size_t j = 0; j < numTriangles; j++) {
                                    triangleVertIndices[group][3*(j + triangleIdx[group]) + 0] = index_t(srcTriangleVertIndices[3*j + 0] + vertIdx);
                                    triangleVertIndices[group][3*(j + triangleIdx[group]) + 1] = index_t(srcTriangleVertIndices[3*j + 1] + vertIdx);
                                    triangleVertIndices[group][3*(j + triangleIdx[group]) + 2] = index_t(srcTriangleVertIndices[3*j + 2] + vertIdx);
                                }
                            }
                        }

                        // Positions
                        if (sample->positions()) {
                            VertexBuffer::ReadInterfacePtr readable = sample->positions()->readableInterface();
                            memcpy(&positions[3*vertIdx], readable->get(), 3*numVerts*sizeof(float));
                        }

                        // Normals
                        if (sample->normals()) {
                            VertexBuffer::ReadInterfacePtr readable = sample->normals()->readableInterface();
                            memcpy(&normals[3*vertIdx], readable->get(), 3*numVerts*sizeof(float));
                        }

                        // UVs
                        if (sample->uvs()) {
                            VertexBuffer::ReadInterfacePtr readable = sample->uvs()->readableInterface();
                            memcpy(&uvs[2*vertIdx], readable->get(), 2*numVerts*sizeof(float));
                        }
                        else if (uvExists) {
                            memset(&uvs[2*vertIdx], 0, 2*numVerts*sizeof(float));
                        }

                        wireIdx += numWires;
                        vertIdx += numVerts;
                        for (size_t i = 0; i < numIndexGroups; i++) {
                            triangleIdx[i] += sample->numTriangles(i);
                        }

                        boundingBox.expand(sample->boundingBox());
                    }
                }

                std::vector<boost::shared_ptr<IndexBuffer> > newTriangleVertIndices(numIndexGroups);
                for (size_t i = 0; i < numIndexGroups; i++) {
                    newTriangleVertIndices[i] = IndexBuffer::create(
                        SharedArray<index_t>::create(
                            triangleVertIndices[i], 3 * totalTriangles[i]));
                }

                boost::shared_ptr<ShapeSample> newSample = ShapeSample::create(
                        *timeIt,
                        totalWires,
                        totalVerts,
                        IndexBuffer::create(
                            SharedArray<index_t>::create(
                                wireVertIndices, 2 * totalWires)),
                        newTriangleVertIndices,
                        VertexBuffer::createPositions(
                            SharedArray<float>::create(
                                positions, 3 * totalVerts)),
                        boundingBox,
                        diffuseColor,
                        visibility);

                if (normals) {
                    newSample->setNormals(
                        VertexBuffer::createNormals(
                            SharedArray<float>::create(
                                normals, 3 * totalVerts)));
                }

                if (uvs) {
                    newSample->setUVs(
                        VertexBuffer::createUVs(
                            SharedArray<float>::create(
                                uvs, 2 * totalVerts)));
                }

                newShape->addSample(newSample);
            }

            // Consolidate the remaining samples.
            std::set<double>::const_iterator timePrev  = timeIt;
            ++timeIt;

            for (; timeIt != timeEnd; ++timeIt) {
                size_t totalWires     = 0;
                size_t totalVerts     = 0;
                std::vector<size_t> totalTriangles;
                size_t numIndexGroups = 0;

                bool uvExists       = false;

                bool wiresDirty     = false;
                bool trianglesDirty = false;
                bool positionsDirty = false;
                bool normalsDirty   = false;
                bool uvsDirty       = false;

                for (size_t i = 0; i < nbShapes; i++) {
                    ShapeData::Ptr& shape = consolidatedShapes[i];

                    const boost::shared_ptr<const ShapeSample>& sample =
                        shape->getSample(*timeIt);
                    const boost::shared_ptr<const ShapeSample>& prevSample =
                        shape->getSample(*timePrev);

                    totalWires += sample->numWires();
                    totalVerts += sample->numVerts();

                    if (numIndexGroups == 0) {
                        // Initialize totalTriangles, assume that
                        // all shapes has the same number of index groups
                        numIndexGroups = sample->numIndexGroups();
                        totalTriangles.resize(numIndexGroups, 0);

                        diffuseColor = sample->diffuseColor();
                        visibility   = sample->visibility();
                    }
                    // Shapes with different number of index groups, diffuseColor and visibility
                    // should be divided into separate buckets.
                    assert(numIndexGroups == sample->numIndexGroups());
                    assert(fabs(diffuseColor.r - sample->diffuseColor().r) < 1e-5);
                    assert(fabs(diffuseColor.g - sample->diffuseColor().g) < 1e-5);
                    assert(fabs(diffuseColor.b - sample->diffuseColor().b) < 1e-5);
                    assert(fabs(diffuseColor.a - sample->diffuseColor().a) < 1e-5);
                    assert(visibility == sample->visibility());

                    for (size_t j = 0; j < totalTriangles.size(); j++) {
                        totalTriangles[j] += sample->numTriangles(j);
                    }

                    // Check whether UV exists
                    if (!uvExists && sample->uvs()) {
                        uvExists = true;
                    }

                    for (size_t j = 0; j < numIndexGroups; j++) {
                        trianglesDirty |= sample->triangleVertIndices(j) != prevSample->triangleVertIndices(j);
                    }
                    wiresDirty     |= sample->wireVertIndices()     != prevSample->wireVertIndices();
                    positionsDirty |= sample->positions()           != prevSample->positions();
                    normalsDirty   |= sample->normals()             != prevSample->normals();
                    uvsDirty       |= sample->uvs()                 != prevSample->uvs();
                }

                if (wiresDirty || trianglesDirty ||
                    positionsDirty || normalsDirty || uvsDirty) {

                        if (wiresDirty) {
                            wireVertIndices = boost::shared_array<index_t>(
                                new index_t[2 * totalWires]);
                        }

                        if (trianglesDirty) {
                            triangleVertIndices.resize(totalTriangles.size());
                            for (size_t i = 0; i < totalTriangles.size(); i++) {
                                triangleVertIndices[i] = boost::shared_array<index_t>(
                                    new index_t[3 * totalTriangles[i]]);
                            }
                        }

                        if (positionsDirty) {
                            positions = boost::shared_array<float>(
                                new float[3 * totalVerts]);
                        }
                        if (normalsDirty) {
                            normals = boost::shared_array<float>(
                                new float[3 * totalVerts]);
                        }
                        if (uvsDirty) {
                            uvs.reset();
                            if (uvExists) {
                                uvs = boost::shared_array<float>(
                                    new float[2 * totalVerts]);
                            }
                        }

                        boundingBox.clear();

                        {
                            size_t wireIdx = 0;
                            size_t vertIdx = 0;
                            std::vector<size_t> triangleIdx(numIndexGroups, 0);

                            for (size_t i = 0; i < nbShapes; i++) {
                                const boost::shared_ptr<const ShapeSample>& sample =
                                    consolidatedShapes[i]->getSample(*timeIt);

                                const size_t numWires = sample->numWires();
                                const size_t numVerts = sample->numVerts();

                                // Wires
                                if (wiresDirty && sample->wireVertIndices()) {
                                    IndexBuffer::ReadInterfacePtr readable = sample->wireVertIndices()->readableInterface();
                                    const index_t* srcWireVertIndices = readable->get();
                                    for (size_t j = 0; j < numWires; j++) {
                                        wireVertIndices[2*(j + wireIdx) + 0] = index_t(srcWireVertIndices[2*j + 0] + vertIdx);
                                        wireVertIndices[2*(j + wireIdx) + 1] = index_t(srcWireVertIndices[2*j + 1] + vertIdx);
                                    }
                                }

                                // Triangles
                                if (trianglesDirty) {
                                    for (size_t group = 0; group < numIndexGroups; group++) {
                                        const size_t numTriangles = sample->numTriangles(group);
                                        if (sample->triangleVertIndices(group)) {
                                            IndexBuffer::ReadInterfacePtr readable = sample->triangleVertIndices(group)->readableInterface();
                                            const index_t* srcTriangleVertIndices = readable->get();
                                            for (size_t j = 0; j < numTriangles; j++) {
                                                triangleVertIndices[group][3*(j + triangleIdx[group]) + 0] = index_t(srcTriangleVertIndices[3*j + 0] + vertIdx);
                                                triangleVertIndices[group][3*(j + triangleIdx[group]) + 1] = index_t(srcTriangleVertIndices[3*j + 1] + vertIdx);
                                                triangleVertIndices[group][3*(j + triangleIdx[group]) + 2] = index_t(srcTriangleVertIndices[3*j + 2] + vertIdx);
                                            }
                                        }
                                    }
                                }

                                // Positions
                                if (positionsDirty && sample->positions()) {
                                    VertexBuffer::ReadInterfacePtr readable = sample->positions()->readableInterface();
                                    memcpy(&positions[3*vertIdx], readable->get(),
                                        3*numVerts*sizeof(float));
                                }

                                // Normals
                                if (normalsDirty && sample->normals()) {
                                    VertexBuffer::ReadInterfacePtr readable = sample->normals()->readableInterface();
                                    memcpy(&normals[3*vertIdx], readable->get(),
                                        3*numVerts*sizeof(float));
                                }

                                // UVs
                                if (uvsDirty) {
                                    if (sample->uvs()) {
                                        VertexBuffer::ReadInterfacePtr readable = sample->uvs()->readableInterface();
                                        memcpy(&uvs[2*vertIdx], readable->get(),
                                            2*numVerts*sizeof(float));
                                    } else if (uvExists) {
                                        memset(&uvs[2*vertIdx], 0, 2*numVerts*sizeof(float));
                                    }
                                }

                                wireIdx += numWires;
                                vertIdx += numVerts;
                                for (size_t i = 0; i < numIndexGroups; i++) {
                                    triangleIdx[i] += sample->numTriangles(i);
                                }

                                boundingBox.expand(sample->boundingBox());
                            }    // for each nodes
                        }
                }    // if anything dirty

                std::vector<boost::shared_ptr<IndexBuffer> > newTriangleVertIndices(numIndexGroups);
                for (size_t i = 0; i < numIndexGroups; i++) {
                    newTriangleVertIndices[i] = IndexBuffer::create(
                        SharedArray<index_t>::create(
                            triangleVertIndices[i], 3 * totalTriangles[i]));
                }

                boost::shared_ptr<ShapeSample> newSample = ShapeSample::create(
                    *timeIt,
                    totalWires,
                    totalVerts,
                    IndexBuffer::create(
                        SharedArray<index_t>::create(
                            wireVertIndices, 2 * totalWires)),
                    newTriangleVertIndices,
                    VertexBuffer::createPositions(
                        SharedArray<float>::create(
                            positions, 3 * totalVerts)),
                    boundingBox,
                    diffuseColor,
                    visibility);

                if (normals) {
                    newSample->setNormals(
                        VertexBuffer::createNormals(
                            SharedArray<float>::create(
                                normals, 3 * totalVerts)));
                }

                if (uvs) {
                    newSample->setUVs(
                        VertexBuffer::createUVs(
                            SharedArray<float>::create(
                                uvs, 2 * totalVerts)));
                }

                newShape->addSample(newSample);
                timePrev = timeIt;
            }

            // All consolidated shapes should have the same materials.
            newShape->setMaterials(consolidatedShapes[0]->getMaterials());

            // Re-use the largest node infos.
            newShapes.push_back(newShape);
            consolidatedShapes.clear();
        }

        SubNode::MPtr fRootNode;
        const int     fThreshold;
        const bool    fMotionBlur;

        SubNode::MPtr fConsolidatedRootNode;
    };


    //==============================================================================
    // CLASS SelectionChecker
    //==============================================================================

    class SelectionChecker
    {
    public:
        SelectionChecker(const MSelectionList& selection)
        {
            MStatus status;
            MDagPath dagPath;

            // A selected node should be ignored
            // if its parent/grandparent is selected.
            for (unsigned int i = 0; i < selection.length(); i++) {
                status = selection.getDagPath(i, dagPath);
                if (status == MS::kSuccess) {
                    std::string fullDagPath = dagPath.fullPathName().asChar();
                    fSelectionPaths.insert(fullDagPath);
                }
            }

            // Check each selected DAG Path
            for (unsigned int i = 0; i < selection.length(); i++) {
                status = selection.getDagPath(i, dagPath);

                if (status == MS::kSuccess && check(dagPath)) {
                    fSelection.add(dagPath);
                }
            }
        }

        const MSelectionList& selection()
        { return fSelection; }

    private:
        // Prohibited and not implemented.
        SelectionChecker(const SelectionChecker&);
        const SelectionChecker& operator= (const SelectionChecker&);

        bool check(const MDagPath& dagPath)
        {
            // This node should not have its parent/grandparent selected
            MDagPath parent = dagPath;
            parent.pop();
            for (; parent.length() > 0; parent.pop()) {
                std::string fullDagPath = parent.fullPathName().asChar();
                if (fSelectionPaths.find(fullDagPath) != fSelectionPaths.end()) {
                    return false;
                }
            }

            return checkGeometry(dagPath);
        }

        bool checkGeometry(const MDagPath& dagPath)
        {
            // Check we have bakeable geometry
            MFnDagNode dagNode(dagPath);
            MObject object = dagPath.node();
            if ((Baker::isBakeable(object) ||
                dagNode.typeId() == ShapeNode::id) &&
                !object.hasFn(MFn::kTransform)) {
                    return true;
            }

            // At least one descendant must be bakeable geometry
            bool hasGeometry = false;
            for (unsigned int i = 0; i < dagPath.childCount(); i++) {
                MDagPath child = dagPath;
                child.push(dagPath.child(i));

                MFnDagNode childNode(child);
                if (childNode.isIntermediateObject()) {
                    continue;
                }

                if (checkGeometry(child)) {
                    hasGeometry= true;
                    break;
                }
            }

            return hasGeometry;
        }

        MSelectionList        fSelection;
        std::set<std::string> fSelectionPaths;
    };


    //==============================================================================
    // CLASS ScopedPauseWorkerThread
    //==============================================================================

    class ScopedPauseWorkerThread : boost::noncopyable
    {
    public:
        ScopedPauseWorkerThread()
        {
            GlobalReaderCache::theCache().pauseRead();
        }

        ~ScopedPauseWorkerThread()
        {
            GlobalReaderCache::theCache().resumeRead();
        }
    };


    //==============================================================================
    // CLASS FileAndSubNodeList
    //==============================================================================

    // This is a list of files and corresponding hierarchy roots.
    // The dummy flag means that the top-level transform is a dummy sub-node. The
    // dummy sub-node should be ignored and its children should be written instead.
    class FileAndSubNode
    {
    public:
        MFileObject   targetFile;
        SubNode::MPtr subNode;
        bool          isDummy;

        FileAndSubNode()
            : isDummy(false)
        {}

        FileAndSubNode(const MFileObject& f, const SubNode::MPtr& n, bool d)
            : targetFile(f), subNode(n), isDummy(d)
        {}
    };
    typedef std::vector<FileAndSubNode> FileAndSubNodeList;


    //==============================================================================
    // CLASS NodePathRegistry
    //==============================================================================

    // This class is responsible for re-constructing the sub-node hierarchy 
    // according to the original dag paths.
    // This class should encapsulate all the logic to determine the file paths.
    //
    // If we are going to write a single hierarchy, 
    //  targetFile = [directory] / [filePrefix] [fileName] [extension]
    //
    // Assuming we are going to bake the following hierarchies:
    // |-A
    //   |-B
    // |-C
    //   |-D
    // We have 4 dag paths: |A, |A|B, |C and |C|D.
    // In this case, we have two hierarchy roots: |A and |C
    //
    // 1) Either -allDagObjects or -saveMultipleFiles false is specified.
    //  We are going to write two hierarchies to a single file.
    //  targetFile = [directory] / [filePrefix] [fileName] [extension]
    //  e.g. ... / filename_specified_by_fileName_arg.abc
    //          (containing |A, |A|B, |C and |C|D)
    //
    // 2) -saveMultipleFiles true is specified. (default)
    //  We are going to write two hierarchies to two files.
    //   2.1) -clashOption numericalIncrement
    //     targetFile = [directory] / [filePrefix] [numericalIncrement] [extension]
    //     e.g. ... / scene1_0.abc  (containing |A and |A|B)
    //          ... / scene1_1.abc  (containing |C and |C|D)
    //
    //   2.2) -clashOption nodeName  (default)
    //     targetFile = [directory] / [filePrefix] [nodeName] [extension]
    //     [nodeName] is the name of the dag node. If there is any conflicts, the
    //     full dag path is used as [nodeName].
    //     e.g. ... / scene1_A.abc  (containing |A and |A|B)
    //          ... / scene1_C.abc  (containing |C and |C|D)
    //      
    class NodePathRegistry : boost::noncopyable
    {
    public:
        enum Stages {
            kResolveStage,      // Adding MDagPath objects.
            kConstructStage,    // Adding sub-node objects.
            kComplete           // Get a list of files and hierarchy roots.
        };

        NodePathRegistry(const bool     allDagObjects,
                         const bool     saveMultipleFiles,
                         const MString& directory,
                         const MString& filePrefix,
                         const MString& fileName,
                         const MString& clashOption)
            : fStage(kResolveStage),
              fAllDagObjects(allDagObjects),
              fSaveMultipleFiles(saveMultipleFiles),
              fDirectory(directory),
              fFilePrefix(filePrefix),
              fFileName(fileName),
              fClashOption(clashOption),
              fExtension(".abc")
        {}

        ~NodePathRegistry()
        {}

        // Add a MDagPath object to this class. When all MDagPath objects are added,
        // call resolve() to move to the next stage.
        void add(const MDagPath& dagPath)
        {
            // We can only add dag paths at resolve stage.
            assert(fStage == kResolveStage);
            assert(dagPath.isValid());

            // The full path name of this dag path.
            std::string fullPath = fullPathName(dagPath);
            assert(fPathMap.find(fullPath) == fPathMap.end());

            // The full path name of the parent dag path.
            MDagPath parentPath = dagPath;
            parentPath.pop();
            std::string parentFullPath = fullPathName(parentPath);

            // Insert into path map.
            PathEntry pathEntry;
            pathEntry.dagPath    = dagPath;
            pathEntry.parentPath = parentFullPath;
            fPathMap[fullPath] = pathEntry;
        }

        // Determine the hierarchy roots and file names based on all input dag paths.
        // Resolve the hierarchy root node name conflict.
        void resolve()
        {
            // We should be at resolve stage.
            assert(fStage == kResolveStage);
            fStage = kConstructStage;

            // Find all hierarchy root nodes.
            BOOST_FOREACH (const PathMapType::value_type& v, fPathMap) {
                // If the path doesn't have a parent, it's a hierarchy root.
                PathMapType::const_iterator it = fPathMap.find(v.second.parentPath);
                if (it == fPathMap.end()) {
                    assert(fRootMap.find(v.first) == fRootMap.end());

                    // Get the node name.
                    MFnDagNode dagNode(v.second.dagPath);
                    std::string nodeName = dagNode.name().asChar();

                    // Strip namespace ":" character.
                    std::replace(nodeName.begin(), nodeName.end(), ':', '_');

                    // Insert into root map.
                    RootEntry rootEntry;
                    rootEntry.nodeName   = nodeName;
                    rootEntry.uniqueName = "Not_Specified";
                    rootEntry.sequence   = std::numeric_limits<size_t>::max();
                    rootEntry.overwrite  = true;
                    fRootMap[v.first] = rootEntry;
                }
            }

            // Count the occurance of base names of the hierarchy roots.
            std::map<std::string,size_t> nameTable;     // name -> occurance
            BOOST_FOREACH (const RootMapType::value_type& v, fRootMap) {
                nameTable.insert(std::make_pair(v.second.nodeName, 0)).first->second++;
            }

            // Resolve root node name conflicts and compute sequences.
            size_t counter = 0;
            BOOST_FOREACH (RootMapType::value_type& v, fRootMap) {
                std::string uniqueName = v.second.nodeName;

                // The name conflicts with other names.
                // We use full path instead of its name.
                if (nameTable.find(uniqueName)->second > 1) {
                    uniqueName = v.first.substr(1);  // remove leading |
                    std::replace(uniqueName.begin(), uniqueName.end(), '|', '_');
                    std::replace(uniqueName.begin(), uniqueName.end(), ':', '_');
                }

                v.second.uniqueName = uniqueName;
                v.second.sequence   = counter++;
            }

            // Determine the directory we are going to export.
            const bool singleFile = fAllDagObjects || !fSaveMultipleFiles || fRootMap.size() == 1;
            const MString directory = validatedDirectory();
            const MString fileName  = validatedFileName();

            // Determine the absolute file path for hierarchy roots.
            BOOST_FOREACH (RootMapType::value_type& v, fRootMap) {
                MString targetFileName = fFilePrefix;

                if (singleFile) {
                    // We are going to save to a single file.
                    targetFileName += fileName;
                }
                else {
                    // We are going to save each hierarchy root to a separate file.
                    if (fClashOption == "numericalIncrement") {
                        // Clash Option: Numerical Increment
                        targetFileName += (unsigned int)v.second.sequence;
                    }
                    else {
                        // Clash Option: Node Name
                        targetFileName += v.second.uniqueName.c_str();
                    }
                }

                MString targetFullName = directory + "/" + targetFileName + fExtension;
                v.second.targetFile.setRawFullName(targetFullName);
            }
        }

        // Pop up a dialog to prompt the user we are going to overwrite the file.
        void promptOverwrite()
        {
            // We have resolved hierarchy roots and their target files.
            assert(fStage == kConstructStage);

            // Make sure we have the dialog proc.
            MGlobal::executeCommand(
                "if (!(`exists showGpuCacheExportConfirmDialog`))\n"
                "{\n"
                "    eval(\"source \\\"doGpuCacheExportArgList.mel\\\"\");\n"
                "}\n"
            );

            // Prompt every file or we remember the choice.
            const bool singleFile = fAllDagObjects || !fSaveMultipleFiles || fRootMap.size() == 1;
            enum OverwriteChoice { kUnknownOverwrite, kAlwaysOverwrite, kNeverOverwrite };
            OverwriteChoice choice = kUnknownOverwrite;

            BOOST_FOREACH (RootMapType::value_type& v, fRootMap) {
                // Skip non-exist files.
                if (!v.second.targetFile.exists()) continue;

                if (choice == kUnknownOverwrite) {
                    // Show the dialog.
                    MString result = MGlobal::executeCommandStringResult(
                        "showGpuCacheExportConfirmDialog \"" + 
                        EncodeString(v.second.targetFile.resolvedFullName()) + 
                        "\""
                    );

                    if (result == "yes") {
                        // Overwrite this file.
                        v.second.overwrite = true;

                        // All hierarchy roots are going to be written to a single file.
                        // Pop up the dialog only once for all hierarchy roots.
                        if (singleFile) choice = kAlwaysOverwrite;
                    }
                    else if (result == "no") {
                        // Skip this file.
                        v.second.overwrite = false;

                        // All hierarchy roots are going to be written to a single file.
                        // Pop up the dialog only once for all hierarchy roots.
                        if (singleFile) choice = kNeverOverwrite;
                    }
                    else if (result == "yesAll") {
                        // Overwrite this file and all following files.
                        v.second.overwrite = true;
                        choice = kAlwaysOverwrite;
                    }
                    else if (result == "noAll" || result == "dismiss") {
                        // Skip this file and all following files.
                        v.second.overwrite = false;
                        choice = kNeverOverwrite;
                    }
                    else {
                        // Something goes wrong with the dialog proc.
                        // We assume overwrite.
                        assert(0);
                    }
                }
                else if (choice == kAlwaysOverwrite) {
                    v.second.overwrite = true;
                }
                else if (choice == kNeverOverwrite) {
                    v.second.overwrite = false;
                }
                else {
                    assert(0);
                }
            }
        }

        // Associate the sub-node with the MDagPath object.
        void associateSubNode(const MDagPath& dagPath, const SubNode::MPtr& subNode)
        {
            assert(fStage == kConstructStage);
            assert(dagPath.isValid());
            assert(subNode);

            // Set the sub node member.
            std::string fullPath = fullPathName(dagPath);

            PathMapType::iterator it = fPathMap.find(fullPath);
            assert(it != fPathMap.end());

            if (it != fPathMap.end()) {
                it->second.subNode = subNode;
            }
        }

        // Construct sub-node hierarchy according to the MDagPath hierarchy.
        void constructHierarchy()
        {
            assert(fStage == kConstructStage);
            fStage = kComplete;

            // Connect child with its parent.
            // Instances are already expanded.
            BOOST_FOREACH (const PathMapType::value_type& v, fPathMap) {
                // Find this sub node.
                const SubNode::MPtr& thisSubNode = v.second.subNode;

                // Find parent sub node.
                PathMapType::iterator parentIt = fPathMap.find(v.second.parentPath);
                if (parentIt != fPathMap.end()) {
                    // Find a parent, connect them.
                    const SubNode::MPtr& parentSubNode = parentIt->second.subNode;
                    SubNode::connect(parentSubNode, thisSubNode);
                }
                else {
                    // This must be a hierarchy root.
                    RootMapType::iterator rootIt = fRootMap.find(v.first);
                    assert(rootIt != fRootMap.end());
                    if (rootIt != fRootMap.end()) {
                        rootIt->second.subNode = thisSubNode;
                    }
                }
            }
        }

        // Get a list of file paths and corresponding hierarchy root sub-node.
        void generateFileAndSubNodes(FileAndSubNodeList& list)
        {
            // Sub-node hierarchies are ready.
            assert(fStage == kComplete);

            // When -allDagObjects is set, we are going to save all dag objects in
            // the scene to a single file.
            // When -saveMultipleFiles is false, we are going to save all selected 
            // dag objects to a single file.
            // In both cases, we need to create a dummy root node ("|") for root nodes.
            if (fAllDagObjects || !fSaveMultipleFiles) {
                if (!fRootMap.empty() && fRootMap.begin()->second.overwrite) {
                    // Create the "|" node
                    GroupCreator groupCreator;
                    BOOST_FOREACH (const RootMapType::value_type& v, fRootMap) {
                        // We are going to write root nodes to a single file.
                        // To prevent root node name clash, we bake the unique name to the root node.
                        v.second.subNode->setName(v.second.uniqueName.c_str());

                        // Add the root node to the dummy group.
                        groupCreator.addChild(v.second.subNode);
                    }
                    groupCreator.group();

                    // Replace all nodes with a single "|" node.
                    // If there is one node, we use the node name as the "|" node name.
                    // If there are more than one node, we use the scene name as the "|" node name.
                    const RootEntry& rootEntry = fRootMap.begin()->second;
                    const MString rootNodeName = (fRootMap.size() == 1) ?
                        rootEntry.subNode->getName() : getSceneNameAsValidObjectName();

                    // Only one file.
                    list.push_back(FileAndSubNode(
                        rootEntry.targetFile,
                        groupCreator.getSubNode(rootNodeName),
                        true /* isDummy */
                    ));
                }
            }
            else {
                // One hierarchy root to one file.
                BOOST_FOREACH (const RootMapType::value_type& v, fRootMap) {
                    // Skip files that we are not going to overwrite.
                    if (!v.second.overwrite) continue;

                    list.push_back(FileAndSubNode(
                        v.second.targetFile,
                        v.second.subNode,
                        false /* isDummy */
                    ));
                }
            }
        }

        void dump()
        {
            using namespace std;
            stringstream out;

            out << "Current Stage: ";
            switch (fStage) {
            case kResolveStage:     out << "ResolveStage";      break;
            case kConstructStage:   out << "ConstructStage";    break;
            case kComplete:         out << "Complete";          break;
            }
            out << endl;

            out << "Path Map: " << endl;
            BOOST_FOREACH (const PathMapType::value_type& v, fPathMap) {
                out << "Path: "
                    << v.first
                    << ", Parent: "
                    << v.second.parentPath
                    << ", SubNode: "
                    << (void*)v.second.subNode.get()
                    << endl;
            }

            out << "Root Map: " << endl;
            BOOST_FOREACH (const RootMapType::value_type& v, fRootMap) {
                out << "Path: "
                    << v.first
                    << ", Node: "
                    << v.second.nodeName
                    << ", Unique: "
                    << v.second.uniqueName
                    << ", Sequence: "
                    << v.second.sequence
                    << ", Overwrite: "
                    << v.second.overwrite
                    << ", Target: "
                    << v.second.targetFile.resolvedFullName().asChar()
                    << ", SubNode: "
                    << (void*)v.second.subNode.get()
                    << endl;
            }

            cout << out.str() << endl;
        }

    private:
        static std::string fullPathName(const MDagPath& dagPath)
        {
            MString buffer = dagPath.fullPathName();
            return std::string(buffer.asChar());
        }

        MString validatedDirectory()
        {
            MFileObject directory;

            if (fDirectory.length() > 0) {
                // If there is a directory specified, we use that directory.
                directory.setRawFullName(fDirectory);
            }
            else {
                MString fileRule, expandName;
                const MString alembicFileRule = "alembicCache";
                const MString alembicFilePath = "cache/alembic";

                MString queryFileRuleCmd;
                queryFileRuleCmd.format("workspace -q -fre \"^1s\"", alembicFileRule);
                MString queryFolderCmd;
                queryFolderCmd.format("workspace -en `workspace -q -fre \"^1s\"`", alembicFileRule);

                // Query the file rule for alembic cache
                MGlobal::executeCommand(queryFileRuleCmd, fileRule);
                if (fileRule.length() > 0) {
                    // We have alembic file rule, query the folder
                    MGlobal::executeCommand(queryFolderCmd, expandName);
                }
                else {
                    // Alembic file rule does not exist, create it
                    MString addFileRuleCmd;
                    addFileRuleCmd.format("workspace -fr \"^1s\" \"^2s\"", alembicFileRule, alembicFilePath);
                    MGlobal::executeCommand(addFileRuleCmd);

                    // Save the workspace. maya may discard file rules on exit
                    MGlobal::executeCommand("workspace -s");

                    // Query the folder
                    MGlobal::executeCommand(queryFolderCmd, expandName);
                }

                // Resolve the expanded file rule
                if (expandName.length() == 0) {
                    expandName = alembicFilePath;
                }

                directory.setRawFullName(expandName);
            }

            return directory.resolvedFullName();
        }

        MString validatedFileName()
        {
            MString fileName;

            if (fFileName.length() > 0) {
                // If there is a file name specified, we use that file name.
                fileName = fFileName;
            }
            else {
                // Generate a default file name.
                if (fRootMap.size() == 1) {
                    fileName = fRootMap.begin()->second.uniqueName.c_str();
                }
                else {
                    fileName = getSceneName();
                }
            }

            return fileName;
        }

        // This map contains name/path info for all dag paths.
        struct PathEntry {
            MDagPath      dagPath;
            std::string   parentPath;
            SubNode::MPtr subNode;
        };
        typedef boost::unordered_map<std::string,PathEntry> PathMapType;

        // This map contains name/path info for all hierarchy roots.
        struct RootEntry {
            std::string   nodeName;
            std::string   uniqueName;
            size_t        sequence;
            bool          overwrite;
            MFileObject   targetFile;
            SubNode::MPtr subNode;
        };
        typedef boost::unordered_map<std::string,RootEntry> RootMapType;

        Stages      fStage;
        PathMapType fPathMap;
        RootMapType fRootMap;

        const bool    fAllDagObjects;
        const bool    fSaveMultipleFiles;
        const MString fDirectory;
        const MString fFilePrefix;
        const MString fFileName;
        const MString fClashOption;
        const MString fExtension;
    };

}

namespace GPUCache {

//==============================================================================
// CLASS Command
//==============================================================================

void* Command::creator()
{
    return new Command();
}

MSyntax Command::cmdSyntax()
{
    MSyntax syntax;

    syntax.addFlag("-dir", "-directory",              MSyntax::kString   );
    syntax.addFlag("-f",   "-fileName",               MSyntax::kString   );
    syntax.addFlag("-smf", "-saveMultipleFiles",      MSyntax::kBoolean  );
    syntax.addFlag("-fp",  "-filePrefix",             MSyntax::kString   );
    syntax.addFlag("-clo", "-clashOption",            MSyntax::kString   );
    syntax.addFlag("-o",   "-optimize"                                   );
    syntax.addFlag("-ot",  "-optimizationThreshold",  MSyntax::kUnsigned );
    syntax.addFlag("-st",  "-startTime",              MSyntax::kTime     );
    syntax.addFlag("-et",  "-endTime",                MSyntax::kTime     );
    syntax.addFlag("-smr", "-simulationRate",         MSyntax::kTime     );
    syntax.addFlag("-spm", "-sampleMultiplier",       MSyntax::kLong     );
    syntax.addFlag("-cl",  "-compressLevel",          MSyntax::kLong     );
    syntax.addFlag("-df",  "-dataFormat",             MSyntax::kString   );
    syntax.addFlag("-sf",  "-showFailed"                                 );
    syntax.addFlag("-ss",  "-showStats"                                  );
    syntax.addFlag("-sgs", "-showGlobalStats"                            );
    syntax.addFlag("-dh",  "-dumpHierarchy",          MSyntax::kString   );
    syntax.addFlag("-atr", "-animTimeRange"                              );
    syntax.addFlag("-gma", "-gpuManufacturer"                            );
    syntax.addFlag("-gmo", "-gpuModel"                                   );
    syntax.addFlag("-gdv", "-gpuDriverVersion"                           );
    syntax.addFlag("-gms", "-gpuMemorySize"                              );
    syntax.addFlag("-ado", "-allDagObjects"                              );
    syntax.addFlag("-r",   "-refresh"                                    );
	syntax.addFlag("-ra",  "-refreshAll"                                 );
    syntax.addFlag("-rs",  "-refreshSettings"                            );
    syntax.addFlag("-wbr", "-waitForBackgroundReading"                   );
    syntax.addFlag("-wm",  "-writeMaterials"                             );
	syntax.addFlag("-wuv", "-writeUVs"                                   );
    syntax.addFlag("-omb", "-optimizeAnimationsForMotionBlur"            );
    syntax.addFlag("-ubt", "-useBaseTessellation"                        );
    syntax.addFlag("-p",   "-prompt"                                     );
	syntax.addFlag("-lfe", "-listFileEntries"                            );
	syntax.addFlag("-lse", "-listShapeEntries"                           );

    syntax.makeFlagQueryWithFullArgs("-dumpHierarchy", true);

    syntax.useSelectionAsDefault(true);
    syntax.setObjectType(MSyntax::kSelectionList, 0);

    syntax.enableQuery(true);
    syntax.enableEdit(true);

    return syntax;
}

Command::Command()
    : fMode(kCreate)
{}

Command::~Command()
{}

bool Command::isUndoable() const
{
    return false;
}

bool Command::hasSyntax() const
{
    return true;
}

void Command::AddHierarchy(
    const MDagPath&                      dagPath,
    std::map<std::string, int>*          idMap,
    std::vector<MObject>*                sourceNodes,
    std::vector<std::vector<MDagPath> >* sourcePaths,
    std::vector<MObject>*                gpuCacheNodes)
{
    MFnDagNode dagNode(dagPath.node());

    MDagPath firstDagPath;
    MStatus status = dagNode.getPath(firstDagPath);
    if (status != MS::kSuccess) return;
    std::string firstPath(firstDagPath.partialPathName().asChar());

    std::map<std::string, int>::iterator pos = idMap->find(firstPath);
    if (pos != idMap->end()){
        // Already traversed. Only store its DAG Path.
        (*sourcePaths)[pos->second].push_back(dagPath);
    }
    else {
        MObject object(dagNode.object());
        MString msgFmt;
        bool isWarning = true;
        if (dagNode.typeId() == ShapeNode::id) {
            if (fMode == kCreate) {
                // Recursive bake a gpuCache node
                (*idMap)[firstPath] = (int)sourceNodes->size();
                sourceNodes->push_back(object);
                sourcePaths->push_back(std::vector<MDagPath>(1, dagPath));
            }
            else {
                // Query flag is set
                gpuCacheNodes->push_back(object);
            }
        }
        else if (Baker::isBakeable(object)) {
            (*idMap)[firstPath] = (int)sourceNodes->size();
            sourceNodes->push_back(object);
            sourcePaths->push_back(std::vector<MDagPath>(1, dagPath));

            if (fMode != kCreate && fShowFailedFlag.isSet()) {
                MStatus status;
                msgFmt = MStringResource::getString(kNodeWontBakeErrorMsg, status);
            }
        }
        else if (fShowFailedFlag.isSet()) {
            MStatus status;
            msgFmt = MStringResource::getString(kNodeBakedFailedErrorMsg, status);
        }

        if (msgFmt.length() > 0) {
            MString nodeName = firstDagPath.fullPathName();
            MString msg;
            msg.format(msgFmt, nodeName);
            if (isWarning) {
                MGlobal::displayWarning(msg);
            }
            else {
                MGlobal::displayInfo(msg);
            }
        }
    }


    unsigned int numChild = dagPath.childCount();
    for(unsigned int i = 0; i < numChild; ++i) {
        MDagPath childPath = dagPath;
        childPath.push(dagPath.child(i));

        MFnDagNode childNode(childPath);
        if (!childNode.isIntermediateObject())
            AddHierarchy(childPath, idMap, sourceNodes, sourcePaths, gpuCacheNodes);
    }
}

bool Command::AddSelected(
    const MSelectionList&                objects,
    std::vector<MObject>*                sourceNodes,
    std::vector<std::vector<MDagPath> >* sourcePaths,
    std::vector<MObject>*                gpuCacheNodes)
{
    MStatus status;

    // map first DAG path to node index
    std::map<std::string, int> idMap;
    for (unsigned int i = 0; i<objects.length(); ++i) {
        MDagPath sourceDagPath;
        status = objects.getDagPath(i, sourceDagPath);
        if (status == MS::kSuccess) {
            AddHierarchy(sourceDagPath, &idMap, sourceNodes, sourcePaths, gpuCacheNodes);
        }
    }

    if (fMode == kCreate) {
        if (sourceNodes->empty()) {
            MStatus stat;
            MString msg;
            if (gpuCacheNodes->empty()) {
                msg = MStringResource::getString(kNoObjBakable2ErrorMsg, stat);
            }
            else {
                msg = MStringResource::getString(kNoObjBakable1ErrorMsg, stat);
            }
            displayWarning(msg);
            return false;
        }

        return true;
    }
    else {
        if (!fRefreshSettingsFlag.isSet() && gpuCacheNodes->empty()) {
            MStatus stat;
            MString msg;
            if (sourceNodes->empty()) {
                msg = MStringResource::getString(kNoObjBaked2ErrorMsg, stat);
            }
            else {
                msg = MStringResource::getString(kNoObjBaked1ErrorMsg, stat);
            }
            displayWarning(msg);
            return false;
        }

        return true;
    }
}

MStatus Command::doIt(const MArgList& args)
{
    MStatus status;

    MArgDatabase argsDb(syntax(), args, &status);
    if (!status) return status;

	unsigned int numFlags = 0;

    // Save the command arguments for undo/redo purposes.
    if (argsDb.isEdit()) {
        if (argsDb.isQuery()) {
            MStatus stat;
            MString msg = MStringResource::getString(kEditQueryFlagErrorMsg, stat);
            displayError(msg);
            return MS::kFailure;
        }
        fMode = kEdit;
		numFlags++;
    }
    else if (argsDb.isQuery()) {
        fMode = kQuery;
		numFlags++;
    }

    numFlags += fDirectoryFlag.parse(argsDb, "-directory");
    if (!fDirectoryFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kDirectoryWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fFileNameFlag.parse(argsDb, "-fileName");
    if (!fFileNameFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kFileNameWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fSaveMultipleFilesFlag.parse(argsDb, "-saveMultipleFiles");
    if (!fSaveMultipleFilesFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kSaveMultipleFilesWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fFilePrefixFlag.parse(argsDb, "-filePrefix");
    if (!fFilePrefixFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kFilePrefixWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fClashOptionFlag.parse(argsDb, "-clashOption");
    if (!fClashOptionFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kClashOptionWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fOptimizeFlag.parse(argsDb, "-optimize");
    if (!fOptimizeFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kOptimizeWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fOptimizationThresholdFlag.parse(argsDb, "-optimizationThreshold");
    if (!fOptimizationThresholdFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kOptimizationThresholdWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fStartTimeFlag.parse(argsDb, "-startTime");
    if (!fStartTimeFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kStartTimeWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fEndTimeFlag.parse(argsDb, "-endTime");
    if (!fEndTimeFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kEndTimeWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fSimulationRateFlag.parse(argsDb, "-simulationRate");
    if (!fSimulationRateFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kSimulationRateWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }
    if (fSimulationRateFlag.isSet()) {
        MTime minRate(0.004, MTime::kFilm);
        if (fSimulationRateFlag.arg() < minRate) {
            // Simulation rate was below 1 tick, issue an appropriate error message.
            MStatus stat;
            MString msg, fmt = MStringResource::getString(kSimulationRateWrongValueMsg, stat);
            msg.format(fmt, MString() + minRate.as(MTime::uiUnit()));
            displayError(msg);
            return MS::kFailure;
        }
    }

    numFlags += fSampleMultiplierFlag.parse(argsDb, "-sampleMultiplier");
    if (!fSampleMultiplierFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kSampleMultiplierWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }
    if (fSampleMultiplierFlag.isSet() && fSampleMultiplierFlag.arg() <= 0) {
        MStatus stat;
        MString msg = MStringResource::getString(kSampleMultiplierWrongValueMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fCompressLevelFlag.parse(argsDb, "-compressLevel");
    if (!fCompressLevelFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kCompressLevelWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fDataFormatFlag.parse(argsDb, "-dataFormat");
    if (!fDataFormatFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kDataFormatWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fShowFailedFlag.parse(argsDb, "-showFailed");
    assert(fShowFailedFlag.isModeValid(fMode));

    numFlags += fShowStats.parse(argsDb, "-showStats");
    assert(fShowStats.isModeValid(fMode));

    numFlags += fShowGlobalStats.parse(argsDb, "-showGlobalStats");
    assert(fShowGlobalStats.isModeValid(fMode));

    numFlags += fDumpHierarchy.parse(argsDb, "-dumpHierarchy");
    assert(fDumpHierarchy.isModeValid(fMode));

    numFlags += fAnimTimeRangeFlag.parse(argsDb, "-animTimeRange");
    if (!fAnimTimeRangeFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kAnimTimeRangeWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fAllDagObjectsFlag.parse(argsDb, "-allDagObjects");
    if (!fAllDagObjectsFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kAllDagObjectsWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fRefreshFlag.parse(argsDb, "-refresh");
    if (!fRefreshFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kRefreshWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

	numFlags += fRefreshAllFlag.parse(argsDb, "-refreshAll");
	if (!fRefreshAllFlag.isModeValid(fMode)) {
		MStatus stat;
		MString msg = MStringResource::getString(kRefreshAllWrongModeMsg, stat);
		displayError(msg);
		return MS::kFailure;
	}

	numFlags += fListFileEntriesFlag.parse(argsDb, "-listFileEntries");
	if (!fListFileEntriesFlag.isModeValid(fMode)) {
		MStatus stat;
		MString msg = MStringResource::getString(kListFileEntriesWrongModeMsg, stat);
		displayError(msg);
		return MS::kFailure;
	}

	numFlags += fListShapeEntriesFlag.parse(argsDb, "-listShapeEntries");
	if (!fListShapeEntriesFlag.isModeValid(fMode)) {
		MStatus stat;
		MString msg = MStringResource::getString(kListShapeEntriesWrongModeMsg, stat);
		displayError(msg);
		return MS::kFailure;
	}

    numFlags += fRefreshSettingsFlag.parse(argsDb, "-refreshSettings");
    if (!fRefreshSettingsFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kRefreshSettingsWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fGpuManufacturerFlag.parse(argsDb, "-gpuManufacturer");
    if (!fGpuManufacturerFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kGpuManufacturerWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fGpuModelFlag.parse(argsDb, "-gpuModel");
    if (!fGpuModelFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kGpuModelWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fGpuDriverVersion.parse(argsDb, "-gpuDriverVersion");
    if (!fGpuDriverVersion.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kGpuDriverVersionWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fGpuMemorySize.parse(argsDb, "-gpuMemorySize");
    if (!fGpuMemorySize.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kGpuMemorySizeWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fWaitForBackgroundReadingFlag.parse(argsDb, "-waitForBackgroundReading");
    if (!fWaitForBackgroundReadingFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kWaitForBackgroundReadingWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fWriteMaterials.parse(argsDb, "-writeMaterials");
    if (!fWriteMaterials.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kWriteMaterialsWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

	numFlags += fUVsFlag.parse(argsDb, "-writeUVs");
	if( !fUVsFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kWriteUVsWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
	}

    numFlags += fOptimizeAnimationsForMotionBlurFlag.parse(argsDb, "-optimizeAnimationsForMotionBlur");
    if (!fOptimizeAnimationsForMotionBlurFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kOptimizeAnimationsForMotionBlurWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fUseBaseTessellationFlag.parse(argsDb, "-useBaseTessellation");
    if (!fUseBaseTessellationFlag.isModeValid(fMode)) {
        MStatus stat;
        MString msg = MStringResource::getString(kUseBaseTessellationWrongModeMsg, stat);
        displayError(msg);
        return MS::kFailure;
    }

    numFlags += fPromptFlag.parse(argsDb, "-prompt");

	if (fRefreshAllFlag.isSet())
	{
		// Ideally, we would use MArgParser to determine number of other flags used.
		// However, MArgParser returns an error when usesSelectionAsDefault() == true
		// and no objects are found.
		//
		// Instead, we manually test for the presence of other flags.
		if( numFlags > 1 )
		{
			MStatus stat;
			MString msg = MStringResource::getString(kRefreshAllOtherFlagsMsg, stat);
			displayError(msg);
			return MS::kFailure;
		}

		refreshAll();
		return MS::kSuccess;
	}

	if (fListFileEntriesFlag.isSet())
	{
		if( numFlags > 1 )
		{
			MStatus stat;
			MString msg = MStringResource::getString(kListFileEntriesOtherFlagsMsg, stat);
			displayError(msg);
			return MS::kFailure;
		}

		listFileEntries();
		return MS::kSuccess;
	}

	if (fListShapeEntriesFlag.isSet())
	{
		if( numFlags > 1 )
		{
			MStatus stat;
			MString msg = MStringResource::getString(kListShapeEntriesOtherFlagsMsg, stat);
			displayError(msg);
			return MS::kFailure;
		}

		listShapeEntries();
		return MS::kSuccess;
	}

    // Backup the current selection
    MSelectionList selectionBackup;
    MGlobal::getActiveSelectionList(selectionBackup);

    MSelectionList objects;
    if (fAllDagObjectsFlag.isSet()) {
        // -allDagObjects flag is set, export all the top-level DAG Nodes
        MStringArray result;
        MGlobal::executeCommand("ls -assemblies -long", result);

        for (unsigned int i = 0; i < result.length(); i++) {
            objects.add(result[i]);
        }
    }
    else {
        // -allDagObjects flag is not set, export the selection or gpuCache arguments
        // Duplicates are removed by merge().
        MSelectionList selectedObjectArgs;
        status = argsDb.getObjects(selectedObjectArgs);
        MStatError(status, "argsDb.getObjects()");

        if (!selectedObjectArgs.isEmpty()) {
            status = objects.merge(selectedObjectArgs, MSelectionList::kMergeNormal);
            MStatError(status, "objects.merge()");
        }
    }

    if (objects.length() == 0 &&
            !(fMode == kQuery && fShowGlobalStats.isSet()) &&
            !(fMode == kEdit && fRefreshSettingsFlag.isSet()) &&
            !(fMode == kQuery && fGpuManufacturerFlag.isSet()) &&
            !(fMode == kQuery && fGpuModelFlag.isSet()) &&
            !(fMode == kQuery && fGpuDriverVersion.isSet()) &&
            !(fMode == kQuery && fGpuMemorySize.isSet())
            ) {
        MString msg = MStringResource::getString(kNoObjectsMsg,status);
        MPxCommand::displayError(msg);
        return MS::kFailure;
    }

    {
        SelectionChecker selectionChecker(objects);
        objects = selectionChecker.selection();
    }

    std::vector<MObject>                sourceNodes;
    std::vector<std::vector<MDagPath> > sourcePaths;
    std::vector<MObject>                gpuCacheNodes;
    if (fMode == kCreate || fMode == kEdit || fShowStats.isSet() ||
            fDumpHierarchy.isSet() || fAnimTimeRangeFlag.isSet() ||
            fWaitForBackgroundReadingFlag.isSet()) {
        if (!AddSelected(objects, &sourceNodes, &sourcePaths, &gpuCacheNodes))
            return MS::kFailure;
    }

    // We flush the selection list before executing any MEL command
    // through MDGModifier::commandToExecute. This saves a LOT of
    // memory!!! This is due to the fact that each executed MEL
    // command might take a copy of the selection list to restore it
    // on undo. But, this is totally unnecessary since we invoking
    // them from another MEL command that already takes care of
    // restoring the selection list on undo!!!
    MGlobal::setActiveSelectionList(MSelectionList(), MGlobal::kReplaceList);

    switch (fMode) {
        case kCreate:   status = doCreate(sourceNodes, sourcePaths, objects); break;
        case kEdit:     status = doEdit(gpuCacheNodes);     break;
        case kQuery:    status = doQuery(gpuCacheNodes);    break;
    }

    // Restore the selection.
    MGlobal::setActiveSelectionList(selectionBackup, MGlobal::kReplaceList);

    return status;
}


MStatus Command::doCreate(const std::vector<MObject>&                sourceNodes,
                               const std::vector<std::vector<MDagPath> >& sourcePaths,
                               const MSelectionList&                      objects)
{
    MStatus status;
    // Compute the baked mesh before committing the Dag modifier so
    // that the Dag modifier includes the baking.
    MCheckReturn(
        Command::doBaking(
            sourceNodes,
            sourcePaths,
            fStartTimeFlag.arg(MAnimControl::animationStartTime()),
            fEndTimeFlag.arg(MAnimControl::animationEndTime()),
            fSimulationRateFlag.arg(MTime(1, MTime::uiUnit())),
            fSampleMultiplierFlag.arg(1)));

    return MS::kSuccess;
}

MStatus Command::doQuery(const std::vector<MObject>& gpuCacheNodes) const
{
    // set the result of gpuCache command
    if (fShowStats.isSet() ||
        fShowGlobalStats.isSet() ||
        fDumpHierarchy.isSet()
    ) {
        // String array result is incompatible with double[2]
        if (fAnimTimeRangeFlag.isSet()) {
            MStatus stat;
            MString msg = MStringResource::getString(kIncompatibleQueryMsg,stat);
            MPxCommand::displayError(msg);
            return MS::kFailure;
        }

        MStringArray result;
        if (fShowStats.isSet()) {
            showStats(gpuCacheNodes, result);
        }
        if (fShowGlobalStats.isSet()) {
            showGlobalStats(result);
        }
        if (fDumpHierarchy.isSet()) {
            if (fDumpHierarchy.isArgValid()) {
                // Dump to a text file
                MFileObject file;
                file.setRawFullName(fDumpHierarchy.arg());
                MCheckReturn( dumpHierarchyToFile(gpuCacheNodes, file) );

                result.append("Dumping hierarchy to: " + file.resolvedFullName());
            }
            else {
                // Dump to script editor
                dumpHierarchy(gpuCacheNodes, result);
            }
        }

        {
            MString output;
            for (unsigned int i = 0; i < result.length(); i++) {
                if (i > 0) output += "\n";
                output += result[i];
            }
            MPxCommand::setResult(output);
        }
    }
    else if (fAnimTimeRangeFlag.isSet()) {
        // -animTimeRange will return double[2] in current time unit
        MDoubleArray animTimeRange;
        showAnimTimeRange(gpuCacheNodes, animTimeRange);
        MPxCommand::setResult(animTimeRange);
    }
    else if (fGpuManufacturerFlag.isSet()) {
        MPxCommand::setResult(VramQuery::manufacturer());
    }
    else if (fGpuModelFlag.isSet()) {
        MPxCommand::setResult(VramQuery::model());
    }
    else if (fGpuDriverVersion.isSet()) {
        int driverVersion[3];
        VramQuery::driverVersion(driverVersion);

        MString verionStr;
        verionStr += driverVersion[0];
        verionStr += ".";
        verionStr += driverVersion[1];
        verionStr += ".";
        verionStr += driverVersion[2];
        MPxCommand::setResult(verionStr);
    }
    else if (fGpuMemorySize.isSet()) {
        MPxCommand::setResult((int)(VramQuery::queryVram() / 1024 / 1024));
    }
    else if (fWaitForBackgroundReadingFlag.isSet()) {
        // Wait until the background reading is finished.
        BOOST_FOREACH (const MObject& node, gpuCacheNodes) {
            // Request the geometry to begin reading
            MFnDagNode dagNode(node);
            ShapeNode* shapeNode = (ShapeNode*)dagNode.userNode();
            if (shapeNode) {
                shapeNode->getCachedGeometry();
            }

            // Wait for the reading
            GlobalReaderCache::theCache().waitForRead(shapeNode->getCacheFileEntry().get());

            // Pull the data
            if (shapeNode) {
                shapeNode->getCachedGeometry();
            }
        }
    }

    return MS::kSuccess;
}

MStatus Command::doEdit(const std::vector<MObject>& gpuCacheNodes)
{
    if (fRefreshSettingsFlag.isSet()) {
        Config::refresh();
    }

    if (fRefreshFlag.isSet()) {
        refresh(gpuCacheNodes);
    }

    return MS::kSuccess;
}

MStatus Command::doBaking(
    const std::vector<MObject>&                sourceNodes,
    const std::vector<std::vector<MDagPath> >& sourcePaths,
    MTime                       startTime,
    MTime                       endTime,
    MTime                       simulationRate,// The time interval to do the simulation.
    int                         samplingRate   // How many time intervals to sample once.
)
{
    // Check the start time and end time.
    if (startTime > endTime) {
        MStatus stat;
        MString msg = MStringResource::getString(kStartEndTimeErrorMsg, stat);
        MPxCommand::displayError(msg);
        return MS::kFailure;
    }

    // Find out the file names that we are going to write to.
    NodePathRegistry pathRegistry(
        fAllDagObjectsFlag.isSet(),
        fSaveMultipleFilesFlag.arg(true),
        fDirectoryFlag.arg(),
        fFilePrefixFlag.arg(),
        fFileNameFlag.arg(),
        fClashOptionFlag.arg()
    );
    BOOST_FOREACH (const std::vector<MDagPath>& dagPaths, sourcePaths) {
        BOOST_FOREACH (const MDagPath& path, dagPaths) {
            pathRegistry.add(path);
        }
    }
    pathRegistry.resolve();

    // Prompt for overwriting files. (Default is overwrite)
    if (MGlobal::mayaState() == MGlobal::kInteractive && fPromptFlag.isSet()) {
        pathRegistry.promptOverwrite();
    }

    // Set up the progress bar for baking
    ProgressBar progressBar(kExportingMsg,
        (unsigned int)(sourceNodes.size() * (int)(
        (endTime - startTime + simulationRate).as(MTime::kSeconds) /
        simulationRate.as(MTime::kSeconds)) / samplingRate));

    // First save the current time, so we can restore it later.
    const MTime previousTime = MAnimControl::currentTime();

    // For go to start time.
    MTime currentTime = startTime;
    MAnimControl::setCurrentTime(currentTime);

    // The DAG object bakers.
    typedef std::vector< boost::shared_ptr<Baker> > Bakers;
    Bakers bakers;

    // The top-level baker for materials.
    boost::shared_ptr<MaterialBaker> materialBaker;
    if (fWriteMaterials.isSet()) {
        materialBaker = boost::make_shared<MaterialBaker>();
    }

    for (size_t i = 0; i < sourceNodes.size(); i++) {
        // Create a new DAG object baker.
        const boost::shared_ptr<Baker> baker(
            Baker::create(sourceNodes[i], sourcePaths[i]));
        if (!baker) {
            MStatus stat;
            MString msg = MStringResource::getString(kCreateBakerErrorMsg, stat);
            MPxCommand::displayError(msg);
            return MS::kFailure;
        }

		const boost::shared_ptr<ShapeBaker> shapeBaker = boost::dynamic_pointer_cast<ShapeBaker,Baker>( baker );

		if( shapeBaker && fUVsFlag.isSet() ) 
		{
			shapeBaker->enableUVs();
		}

        if (materialBaker) {
            baker->setWriteMaterials();
        }
        if (fUseBaseTessellationFlag.isSet()) {
            baker->setUseBaseTessellation();
        }
        bakers.push_back(baker);

        // sample all shapes at start time
        MCheckReturn(baker->sample(currentTime));

        // Add the connected shaders to the material baker.
        if (materialBaker) {
            BOOST_FOREACH (const MDagPath& path, sourcePaths[i]) {
                if (path.node().hasFn(MFn::kShape)) {
                    MCheckReturn( materialBaker->addShapePath(path) );
                }
            }
        }

        MUpdateProgressAndCheckInterruption(progressBar);
    }

    // Sample all materials at start time.
    if (materialBaker) {
        MCheckReturn( materialBaker->sample(currentTime) );
    }

    // Sample the vertex attributes over time.
    currentTime += simulationRate;
    for (int sampleIdx = 1; currentTime<=endTime;
         currentTime += simulationRate, ++sampleIdx) {
        // Advance time.
        MAnimControl::setCurrentTime(currentTime);

        if (sampleIdx % samplingRate == 0) {
            BOOST_FOREACH(const boost::shared_ptr<Baker>& baker, bakers) {
                MCheckReturn(baker->sample(currentTime));

                MUpdateProgressAndCheckInterruption(progressBar);
            }    // for each baker

            if (materialBaker) {
                MCheckReturn( materialBaker->sample(currentTime) );
            }
        }
    }    // for each time sample

    // Construct the material graphs
    MaterialGraphMap::Ptr materials;
    if (materialBaker) {
        materialBaker->buildGraph();
        materials = materialBaker->get();
    }

    // Construct SubNode hierarchy.
    {
        assert(bakers.size() == sourceNodes.size());
        assert(bakers.size() == sourcePaths.size());

        // Create a SubNode for each instance.
        for (size_t i = 0; i < sourcePaths.size(); i++) {
            for (size_t j = 0; j < sourcePaths[i].size(); j++) {
                const MDagPath& path  = sourcePaths[i][j];
                SubNode::MPtr subNode = bakers[i]->getNode(j);

                pathRegistry.associateSubNode(path, subNode);
            }
        }

        pathRegistry.constructHierarchy();
    }

    // We are done with the bakers now.
    bakers.clear();
    materialBaker.reset();

    // Restore current time.
    MAnimControl::setCurrentTime(previousTime);

    // Preparing the root nodes and files to write.
    FileAndSubNodeList fileList;
    pathRegistry.generateFileAndSubNodes(fileList);

    // Do consolidation
    if (fOptimizeFlag.isSet()) {
        const int  threshold  = fOptimizationThresholdFlag.arg(40000);
        const bool motionBlur = fOptimizeAnimationsForMotionBlurFlag.isSet();

        BOOST_FOREACH (FileAndSubNode& v, fileList) {
            Consolidator consolidator(v.subNode, threshold, motionBlur);
            MCheckReturn( consolidator.consolidate() );

            SubNode::MPtr consolidatedRootNode = consolidator.consolidatedRootNode();
            if (consolidatedRootNode) {
                v.subNode = consolidatedRootNode;
                v.isDummy = false;
            }
        }
    }

    // Set up progress bar for writing
    //
    // FIXME: The cache writer should provide more granularity for
    // updating the progress bar.
    progressBar.reset(kWritingMsg, (unsigned int)fileList.size());

    // Write the baked geometry to the cache file.
    const MTime timePerCycle = simulationRate * samplingRate;

    Writer gpuCacheWriter(
        (char)fCompressLevelFlag.arg(-1),
        fDataFormatFlag.arg("hdf"),
        timePerCycle,
        startTime
    );

    BOOST_FOREACH (const FileAndSubNode& v, fileList) {
        if (v.isDummy) {
            // This is a dummy root node. We are going to write its children.
            MCheckReturn(
                gpuCacheWriter.writeNodes(
                    v.subNode->getChildren(),
                    materials,
                    v.targetFile
                )
            );
        }
        else {
            // We write the node to its taget file.
            MCheckReturn(
                gpuCacheWriter.writeNode(
                    v.subNode,
                    materials,
                    v.targetFile
                )
            );
        }

        appendToResult(v.targetFile.resolvedFullName());
        MUpdateProgressAndCheckInterruption(progressBar);
    }

    return MS::kSuccess;
}


void Command::showStats(
    const std::vector<MObject>& gpuCacheNodes,
    MStringArray& result
) const
{
    MStatus status;
    {
        result.append(MStringResource::getString(kStatsAllFramesMsg, status));

        StatsVisitor stats;
        BOOST_FOREACH(const MObject& gpuCacheObject, gpuCacheNodes) {
            MFnDagNode gpuCacheFn(gpuCacheObject);
            MPxNode* node = gpuCacheFn.userNode();
            assert(node);
            assert(dynamic_cast<ShapeNode*>(node));
            ShapeNode* gpuCacheNode =
                static_cast<ShapeNode*>(node);

            stats.accumulateNode(gpuCacheNode->getCachedGeometry());
            stats.accumulateMaterialGraph(gpuCacheNode->getCachedMaterial());
        }
        stats.print(result, false);
    }

    {
        result.append(MStringResource::getString(kStatsCurrentFrameMsg, status));

        StatsVisitor stats(MAnimControl::currentTime());
        BOOST_FOREACH(const MObject& gpuCacheObject, gpuCacheNodes) {
            MFnDagNode gpuCacheFn(gpuCacheObject);
            MPxNode* node = gpuCacheFn.userNode();
            assert(node);
            assert(dynamic_cast<ShapeNode*>(node));
            ShapeNode* gpuCacheNode =
                static_cast<ShapeNode*>(node);

            stats.accumulateNode(gpuCacheNode->getCachedGeometry());
            stats.accumulateMaterialGraph(gpuCacheNode->getCachedMaterial());
        }
        stats.print(result, true);
    }
}

void Command::showGlobalStats(
    MStringArray& result
) const
{
    MStatus status;

	// Exclude internal unit bounding box
	const size_t unitBoundingBoxIndicesBytes = UnitBoundingBox::indices()->bytes();
	const size_t unitBoundingBoxPositionsBytes = UnitBoundingBox::positions()->bytes();
	const size_t unitBoundingBoxBytes = unitBoundingBoxIndicesBytes + unitBoundingBoxPositionsBytes;
	const size_t unitBoundingBoxNbIndices = 1;
	const size_t unitBoundingBoxNbVertices = 1;
	const size_t unitBoundingBoxNbBuffers = unitBoundingBoxNbIndices + unitBoundingBoxNbVertices;

    // System memory buffers
    {
        MString memUnit;
        double  memSize =
            toHumanUnits(IndexBuffer::nbAllocatedBytes() +
                         VertexBuffer::nbAllocatedBytes() -
						 unitBoundingBoxBytes,
                         memUnit);

        MString msg;
        MString msg_buffers; msg_buffers += (double)(IndexBuffer::nbAllocated() +
                                                     VertexBuffer::nbAllocated() -
													 unitBoundingBoxNbBuffers);
        MString msg_memSize; msg_memSize += memSize;
        msg.format(
            MStringResource::getString(kGlobalSystemStatsMsg, status),
            msg_buffers, msg_memSize, memUnit);
        result.append(msg);
    }
    {
        MString memUnit;
        double  memSize =
            toHumanUnits(IndexBuffer::nbAllocatedBytes() - unitBoundingBoxIndicesBytes, memUnit);

        MString msg;
        MString msg_buffers; msg_buffers += (double)(IndexBuffer::nbAllocated() - unitBoundingBoxNbIndices);
        MString msg_memSize; msg_memSize += memSize;
        msg.format(
            MStringResource::getString(kGlobalSystemStatsIndexMsg, status),
            msg_buffers, msg_memSize, memUnit);
        result.append(msg);
    }
    {
        MString memUnit;
        double  memSize =
            toHumanUnits(VertexBuffer::nbAllocatedBytes() - unitBoundingBoxPositionsBytes, memUnit);

        MString msg;
        MString msg_buffers; msg_buffers +=
                                 (double)(VertexBuffer::nbAllocated() - unitBoundingBoxNbVertices);
        MString msg_memSize; msg_memSize += memSize;
        msg.format(
            MStringResource::getString(kGlobalSystemStatsVertexMsg, status),
            msg_buffers, msg_memSize, memUnit);
        result.append(msg);
    }

    // Video memory buffers
    {
        MString memUnit;
        double  memSize = toHumanUnits(VBOBuffer::nbAllocatedBytes(),
                                       memUnit);

        MString msg;
        MString msg_buffers; msg_buffers += (double)(VBOBuffer::nbAllocated());
        MString msg_memSize; msg_memSize += memSize;
        msg.format(
            MStringResource::getString(kGlobalVideoStatsMsg, status),
            msg_buffers, msg_memSize, memUnit);
        result.append(msg);
    }
    {
        MString memUnit;
        double  memSize = toHumanUnits(VBOBuffer::nbIndexAllocatedBytes(),
                                       memUnit);

        MString msg;
        MString msg_buffers; msg_buffers +=
                                 (double)VBOBuffer::nbIndexAllocated();
        MString msg_memSize; msg_memSize += memSize;
        msg.format(
            MStringResource::getString(kGlobalVideoStatsIndexMsg, status),
            msg_buffers, msg_memSize, memUnit);
        result.append(msg);
    }
    {
        MString memUnit;
        double  memSize = toHumanUnits(VBOBuffer::nbVertexAllocatedBytes(),
                                       memUnit);

        MString msg;
        MString msg_buffers; msg_buffers +=
                                 (double)VBOBuffer::nbVertexAllocated();
        MString msg_memSize; msg_memSize += memSize;
        msg.format(
            MStringResource::getString(kGlobalVideoStatsVertexMsg, status),
            msg_buffers, msg_memSize, memUnit);
        result.append(msg);
    }

    // Last refresh statistics
    {
        MString msg;
        msg.format(MStringResource::getString(kGlobalRefreshStatsMsg, status));
        result.append(msg);
    }
    {
        MString memUnit;
        double  memSize = toHumanUnits(VBOBuffer::nbUploadedBytes(),
                                       memUnit);

        MString msg;
        MString msg_buffers; msg_buffers +=
                                 (double)VBOBuffer::nbUploaded();
        MString msg_memSize; msg_memSize += memSize;
        msg.format(
            MStringResource::getString(kGlobalRefreshStatsUploadMsg, status),
            msg_buffers, msg_memSize, memUnit);
        result.append(msg);
    }
    {
        MString memUnit;
        double  memSize = toHumanUnits(VBOBuffer::nbEvictedBytes(),
                                       memUnit);

        MString msg;
        MString msg_buffers; msg_buffers +=
                                 (double)VBOBuffer::nbEvicted();
        MString msg_memSize; msg_memSize += memSize;
        msg.format(
            MStringResource::getString(kGlobalRefreshStatsEvictionMsg, status),
            msg_buffers, msg_memSize, memUnit);
        result.append(msg);
    }
}

void Command::dumpHierarchy(
    const std::vector<MObject>& gpuCacheNodes,
    MStringArray& result
) const
{
    BOOST_FOREACH(const MObject& gpuCacheObject, gpuCacheNodes) {
        MFnDagNode gpuCacheFn(gpuCacheObject);
        MPxNode* node = gpuCacheFn.userNode();
        assert(node);
        assert(dynamic_cast<ShapeNode*>(node));
        ShapeNode* gpuCacheNode =
            static_cast<ShapeNode*>(node);

        SubNode::Ptr rootNode = gpuCacheNode->getCachedGeometry();

        if (rootNode) {
            DumpHierarchyVisitor visitor(result);
            rootNode->accept(visitor);
        }

        MaterialGraphMap::Ptr materials = gpuCacheNode->getCachedMaterial();

        if (materials) {
            DumpMaterialVisitor visitor(result);
            visitor.dumpMaterials(materials);
        }
    }
}

MStatus Command::dumpHierarchyToFile(
    const std::vector<MObject>& gpuCacheNodes,
    const MFileObject& file
) const
{
    MStringArray result;
    dumpHierarchy(gpuCacheNodes, result);

    std::ofstream output(file.resolvedFullName().asChar());
    if (!output.is_open()) {
        MStatus stat;
        MString fmt = MStringResource::getString(kCouldNotSaveFileMsg, stat);
        MString msg;
        msg.format(fmt, file.resolvedFullName());
        MPxCommand::displayError(msg);
        return MS::kFailure;
    }

    for (unsigned int i = 0; i < result.length(); i++) {
        output << result[i].asChar() << std::endl;
    }

    output.close();
    return MS::kSuccess;
}

void Command::showAnimTimeRange(
    const std::vector<MObject>& gpuCacheNodes,
    MDoubleArray& result
) const
{
    TimeInterval animTimeRange(TimeInterval::kInvalid);

    BOOST_FOREACH(const MObject& node, gpuCacheNodes) {
        MFnDagNode dagNode(node);
        if (dagNode.typeId() != ShapeNode::id) {
            continue;
        }

        ShapeNode* userNode = dynamic_cast<ShapeNode*>(dagNode.userNode());
        if (userNode == NULL) {
            continue;
        }

        const SubNode::Ptr topNode = userNode->getCachedGeometry();
        if (userNode->backgroundReadingState() != CacheFileEntry::kReadingDone) {
            // Background reading in progress but we need the animation time
            // range information immediately.
            MString cacheFileName = MPlug(node, ShapeNode::aCacheFileName).asString();

            MFileObject cacheFile;
            cacheFile.setRawFullName(cacheFileName);
            cacheFile.setResolveMethod(MFileObject::kInputFile);
            if (cacheFileName.length() > 0 && cacheFile.exists()) {
                // Temporarily pause the worker thread and read the time range.
                ScopedPauseWorkerThread pause;

                GlobalReaderCache::CacheReaderProxy::Ptr proxy =
                    GlobalReaderCache::theCache().getCacheReaderProxy(cacheFile);
                GlobalReaderCache::CacheReaderHolder holder(proxy);

                boost::shared_ptr<CacheReader> reader = holder.getCacheReader();
                if (reader && reader->valid()) {
                    TimeInterval interval(TimeInterval::kInvalid);
                    if (reader->readAnimTimeRange(interval)) {
                        animTimeRange |= interval;
                    }
                }
            }

        }
        else if (topNode) {
            const SubNodeData::Ptr data = topNode->getData();
            if (data) {
                animTimeRange |= data->animTimeRange();
            }
        }
    }

    result.setLength(2);
    result[0] = MTime(animTimeRange.startTime(), MTime::kSeconds).as(MTime::uiUnit());
    result[1] = MTime(animTimeRange.endTime(),   MTime::kSeconds).as(MTime::uiUnit());
}

void Command::refresh(const std::vector<MObject>& gpuCacheNodes)
{
    BOOST_FOREACH(const MObject& node, gpuCacheNodes) {
        MFnDagNode dagNode(node);
        if (dagNode.typeId() != ShapeNode::id) {
            continue;
        }

        ShapeNode* userNode = dynamic_cast<ShapeNode*>(dagNode.userNode());
        if (userNode == NULL) {
            continue;
        }

        userNode->refreshCachedGeometry( true );
    }

	// Schedule an idle refresh. A normal refresh will cause the Alembic file to be
	// loaded immediately. We want this load operation to happen later.
	if (MGlobal::mayaState() == MGlobal::kInteractive) {
		MGlobal::executeCommandOnIdle("refresh");
	}
}

void Command::refreshAll()
{
	// Clear the CacheFileRegistry
	CacheFileRegistry::theCache().clear();

	// Force a refresh on all ShapeNodes
	MFnDependencyNode nodeFn;
	std::vector<MObjectHandle> shapes;
	CacheShapeRegistry::theCache().getAll( shapes );
	for( size_t i = 0; i < shapes.size(); i++ )
	{
		if( !shapes[i].isValid() )
		{
			continue;
		}

		nodeFn.setObject( shapes[i].object() );
		assert( nodeFn.typeId() == ShapeNode::id );
		ShapeNode* shape = (ShapeNode*) nodeFn.userNode();
		assert( shape );

		// File cache has already been cleared, do not request clearFileCache
		shape->refreshCachedGeometry( false );
	}

    // Schedule an idle refresh. A normal refresh will cause the Alembic file to be
    // loaded immediately. We want this load operation to happen later.
    if (MGlobal::mayaState() == MGlobal::kInteractive) {
        MGlobal::executeCommandOnIdle("refresh");
    }
}

void Command::listFileEntries()
{
	MStringArray output;

	std::vector<CacheFileEntry::MPtr> entries;
	CacheFileRegistry::theCache().getAll(entries);

	size_t nEntries = entries.size();
	for( size_t i = 0; i < nEntries; i++ )
	{
		output.append( entries[i]->fResolvedCacheFileName );
	}

	MPxCommand::setResult(output);
}

void Command::listShapeEntries()
{
	MStringArray output;

	std::vector<MObjectHandle> shapes;
	CacheShapeRegistry::theCache().getAll(shapes);

	MFnDependencyNode nodeFn;
	size_t nShapes = shapes.size();
	for( size_t i = 0; i < nShapes; i++ )
	{
		MString str;
		MObject obj = shapes[i].object();
		MStatus stat = nodeFn.setObject(obj);
		if( stat )
		{
			ShapeNode* shapeNode = (ShapeNode*) nodeFn.userNode();
			CacheFileEntry::MPtr entry = shapeNode->getCacheFileEntry();

			str += nodeFn.name();
			str += ":";
			if( entry )
			{
				str += entry->fResolvedCacheFileName;
			}
		}
		else
		{
			str += "kNullObj:";
		}
		output.append( str );
	}

	MPxCommand::setResult(output);
}

}

