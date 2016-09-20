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

#include "CacheWriter.h"
#include "gpuCacheUtil.h"
#include "gpuCacheStrings.h"

#include <maya/MFnDagNode.h>
#include <maya/MAnimControl.h>
#include <maya/MFnMeshData.h>
#include <maya/MIntArray.h>
#include <maya/MFloatArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFnMesh.h>
#include <maya/MUintArray.h>
#include <maya/MHWGeometry.h>
#include <maya/MGeometryExtractor.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MDagPathArray.h>
#include <maya/MGlobal.h>
#include <maya/MPlug.h>
#include <maya/MFnTransform.h>

#include <string.h>			// for memcmp
#include <string>
#include <cassert>

using namespace MHWRender;
using namespace GPUCache;

//==============================================================================
// CLASS CacheWriter
//==============================================================================

std::map<std::string,void*> CacheWriter::fsRegistry;

boost::shared_ptr<CacheWriter> CacheWriter::create(const MString& impl,
    const MFileObject& file, char compressLevel, const MString& dataFormat)
{
    std::string key = impl.asChar();
    std::map<std::string,void*>::iterator iter = fsRegistry.find(key);
    if (iter != fsRegistry.end()) {
        return ((CreateFunction*)(*iter).second)(file, compressLevel, dataFormat);
    }

    assert("not implemented");
    return boost::shared_ptr<CacheWriter>();
}

void CacheWriter::registerWriter(const MString& impl, CreateFunction func)
{
    std::string key = impl.asChar();
    fsRegistry[key] = (void*)func;
}

//==============================================================================
// CLASS CacheXformSampler
//==============================================================================

boost::shared_ptr<CacheXformSampler>
CacheXformSampler::create(const MObject& xformObject)
{
    return boost::make_shared<CacheXformSampler>(xformObject);
}

CacheXformSampler::CacheXformSampler(const MObject& xformObject)
    : fXform(xformObject),
      fIsFirstSample(true),
      // The first sample is always considered animated as we have to
      // capture its values.
      fXformAnimated(true),
      fVisibilitySample(false),
      fVisibilityAnimated(true)
{}

CacheXformSampler::~CacheXformSampler()
{}

void CacheXformSampler::addSample()
{
    MMatrix      prevXformSample      = fXformSample;
    bool         prevVisibilitySample = fVisibilitySample;

    fXformSample       = fXform.transformationMatrix();
    fVisibilitySample  = ShapeVisibilityChecker(fXform.object()).isVisible();

    if (fIsFirstSample) {
        // The first sample is always considered animated as we have
        // to capture its values.
        fIsFirstSample = false;
    }
    else {
        fXformAnimated = (prevXformSample != fXformSample);
        fVisibilityAnimated = (prevVisibilitySample != fVisibilitySample);
    }
}

boost::shared_ptr<const XformSample>
CacheXformSampler::getSample(double timeInSeconds)
{
	boost::shared_ptr<XformSample> sample =
		XformSample::create(
            timeInSeconds,fXformSample, MBoundingBox(), fVisibilitySample);
	return sample;
}

//==============================================================================
// CLASS CacheMeshSampler::AttributeSet
//==============================================================================

CacheMeshSampler::AttributeSet::AttributeSet(
    MObject meshObject,
    const bool visibility,
    const bool needUVs
)
{
    typedef IndexBuffer::index_t index_t;

    MayaMeshExtractor<index_t> extractor(meshObject);
    extractor.setWantUVs(needUVs);
    extractor.compute();

    boost::shared_ptr<Array<index_t> > wireIndices     = extractor.wireIndices();
    boost::shared_ptr<Array<index_t> > triangleIndices = extractor.triangleIndices();
    boost::shared_ptr<Array<float> > positions = extractor.positions();
    boost::shared_ptr<Array<float> > normals   = extractor.normals();
    boost::shared_ptr<Array<float> > uvs;
    if (needUVs) {
        uvs = extractor.uvs();
    }

    fNumWires     = wireIndices->size() / 2;
    fNumTriangles = triangleIndices->size() / 3;
    fNumVerts     = positions->size() / 3;

    fWireVertIndices = IndexBuffer::create(wireIndices);
    fTriangleVertIndices.push_back(IndexBuffer::create(triangleIndices));

    fPositions = VertexBuffer::createPositions(positions);
    fNormals   = VertexBuffer::createNormals(normals);
    if (needUVs) {
        fUVs       = VertexBuffer::createUVs(uvs);
    }

    {
        float minX = +std::numeric_limits<float>::max();
        float minY = +std::numeric_limits<float>::max();
        float minZ = +std::numeric_limits<float>::max();

        float maxX = -std::numeric_limits<float>::max();
        float maxY = -std::numeric_limits<float>::max();
        float maxZ = -std::numeric_limits<float>::max();

        VertexBuffer::ReadInterfacePtr readable = fPositions->readableInterface();
        const float* srcPositions = readable->get();

        for (size_t i=0; i<fNumVerts; ++i) {
            const float x = srcPositions[3*i + 0];
            const float y = srcPositions[3*i + 1];
            const float z = srcPositions[3*i + 2];

            minX = std::min(x, minX);
            minY = std::min(y, minY);
            minZ = std::min(z, minZ);

            maxX = std::max(x, maxX);
            maxY = std::max(y, maxY);
            maxZ = std::max(z, maxZ);
        }

        fBoundingBox = MBoundingBox(MPoint(minX, minY, minZ),
                                    MPoint(maxX, maxY, maxZ));
    }
    
    fVisibility  = visibility;
}

CacheMeshSampler::AttributeSet::AttributeSet(
    MFnMesh& mesh,
    const bool needUVs,
    const bool useBaseTessellation
)
    :fNumWires(0),
    fNumTriangles(0),
    fNumVerts(0)
{
    // Refresh the internal shape, otherwise topo changes make mesh.numPolygons() crash.
    // Note that buildShaderAssignmentGroups() also call mesh.numPolygons().
    mesh.syncObject();

    MDagPath dagPath;
    mesh.getPath(dagPath);

    // build a geometry request and add requirements to it.
    MHWRender::MGeometryRequirements geomRequirements;

    // Build descriptors to request the positions, normals and UVs	
    MVertexBufferDescriptor posDesc("", MGeometry::kPosition, MGeometry::kFloat, 3);
    MVertexBufferDescriptor normalDesc("", MGeometry::kNormal, MGeometry::kFloat, 3);
    MVertexBufferDescriptor uvDesc(mesh.currentUVSetName(), MGeometry::kTexture, MGeometry::kFloat, 2);

    // add the descriptors to the geometry requirements
    geomRequirements.addVertexRequirement(posDesc);
    geomRequirements.addVertexRequirement(normalDesc);
    if (needUVs)
        geomRequirements.addVertexRequirement(uvDesc);

    MString noName; // we do not need custom named index buffers here.

    // create a component to include all elements.
    MFnSingleIndexedComponent comp;
    MObject compObj = comp.create(MFn::kMeshPolygonComponent);
    comp.setCompleteData(mesh.numPolygons());

	// create edge component
	MFnSingleIndexedComponent edgeComp;
	MObject edgeCompObj = edgeComp.create(MFn::kMeshEdgeComponent);
	edgeComp.setCompleteData(mesh.numEdges());

    // Add the edge line index buffer to the requirements
    MIndexBufferDescriptor edgeDesc(MIndexBufferDescriptor::kEdgeLine, noName, MGeometry::kLines, 2, edgeCompObj);
    geomRequirements.addIndexingRequirement(edgeDesc);

    // add a triangle buffer to the requirements
    MIndexBufferDescriptor triangleDesc(MIndexBufferDescriptor::kTriangle, noName, MGeometry::kTriangles, 3, compObj);
    geomRequirements.addIndexingRequirement(triangleDesc);

    typedef IndexBuffer::index_t index_t;

    // We ignore the Smooth Preview option on the mesh shape node when
    // using base tessellation.
    int extractorOptions = MHWRender::kPolyGeom_Normal;
    if (useBaseTessellation) {
        extractorOptions |= MHWRender::kPolyGeom_BaseMesh;
    }

    // create an extractor to get the geometry
	MStatus status;
    MHWRender::MGeometryExtractor extractor(geomRequirements,dagPath, extractorOptions, &status);
	if (MS::kFailure==status)
		return;

    // get the number of vertices from the extractor
    unsigned int numVertices   = extractor.vertexCount();
    // get the number of primitives (triangles, lines, etc.)
    unsigned int numWires = extractor.primitiveCount(edgeDesc);

    // create the arrays that the generator will fill
    boost::shared_array<float> vertices(new float[numVertices*posDesc.stride()]);
    boost::shared_array<float> normals(new float[numVertices*normalDesc.stride()]);
    boost::shared_array<float> uvs(new float[numVertices*uvDesc.stride()]);

    unsigned int minBufferSize = extractor.minimumBufferSize(numWires, edgeDesc.primitive());
    boost::shared_array<index_t> wireframeIdx(new index_t[minBufferSize]);

    // populate the index buffer for the edges
    if (MS::kFailure==extractor.populateIndexBuffer(wireframeIdx.get(), numWires, edgeDesc))
        return;
    // populate the vertex buffers you are interested in. (pos, normal, and uv)
    if (MS::kFailure==extractor.populateVertexBuffer(vertices.get(), numVertices, posDesc))
        return;
    if (MS::kFailure==extractor.populateVertexBuffer(normals.get(), numVertices, normalDesc))
        return; 
    if (needUVs && MS::kFailure==extractor.populateVertexBuffer(uvs.get(), numVertices, uvDesc))
        return; 

    // populate the index buffers for all the triangle components
    {
        std::vector<boost::shared_ptr<Array<index_t> > > trgIdxGrps;

        // get the index buffer count from the extractor
        unsigned int numTriangles = extractor.primitiveCount(triangleDesc);
        minBufferSize = extractor.minimumBufferSize(numTriangles, triangleDesc.primitive());
        boost::shared_array<index_t> triangleIdx(new index_t[minBufferSize]);

        if (numTriangles != 0) 
        {
            if (MS::kFailure==extractor.populateIndexBuffer(triangleIdx.get(), numTriangles, triangleDesc))
                return;
            fNumTriangles += (size_t)numTriangles;
        }

        trgIdxGrps.push_back(SharedArray<index_t>::create(
            triangleIdx, 3 * numTriangles));

        for(size_t i=0, offset=0; i<trgIdxGrps.size(); ++i) {
            fTriangleVertIndices.push_back(IndexBuffer::create(
                trgIdxGrps[0], offset, offset + trgIdxGrps[i]->size()));
            offset += trgIdxGrps[i]->size();
        }
    }

    fNumWires     = (size_t)numWires;
    fNumVerts     = (size_t)numVertices;

    fWireVertIndices = IndexBuffer::create(
        SharedArray<index_t>::create( wireframeIdx, 2 * fNumWires));   

    fPositions = VertexBuffer::createPositions(
        SharedArray<float>::create( vertices, 3 * fNumVerts));

    fNormals = VertexBuffer::createNormals(
        SharedArray<float>::create( normals, 3 * fNumVerts));

    if (needUVs) {
        fUVs = VertexBuffer::createUVs(
            SharedArray<float>::create(uvs, 2 * fNumVerts));
    }

    fBoundingBox = mesh.boundingBox();

    // Check visibility 
    fVisibility = ShapeVisibilityChecker(mesh.object()).isVisible();
}


bool CacheMeshSampler::AttributeSet::updateAnimatedChannels(
    bool& animated, const AttributeSet& newer, const MString& path
)
{
    const bool numWiresAnimated     = fNumWires     != newer.fNumWires;
    const bool numTrianglesAnimated = fNumTriangles != newer.fNumTriangles;
    const bool numVertsAnimated     = fNumVerts     != newer.fNumVerts;

    const bool wiresAnimated = fWireVertIndices != newer.fWireVertIndices;

    // We reuse the triangulation from the previous sample
    // if the topology of the wire mesh is not changing. This is
    // done to avoid performance issue due to position dependent
    // triangulation of animated meshes. 
    const bool trianglesAnimated =
        (!numWiresAnimated && !numTrianglesAnimated && !numVertsAnimated &&
         !wiresAnimated) ?
        false :
        (fTriangleVertIndices != newer.fTriangleVertIndices);
    
    const bool positionsAnimated = fPositions != newer.fPositions;
    const bool normalsAnimated   = fNormals   != newer.fNormals;
    const bool uvsAnimated       = fUVs       != newer.fUVs;

    const bool boundingBoxAnimated =
        (!fBoundingBox.min().isEquivalent(newer.fBoundingBox.min()) ||
         !fBoundingBox.max().isEquivalent(newer.fBoundingBox.max()));

    const bool visibilityAnimated = fVisibility != newer.fVisibility;

    fNumWires     = newer.fNumWires;
    fNumTriangles = newer.fNumTriangles;
    fNumVerts     = newer.fNumVerts;
    
    fWireVertIndices = wiresAnimated ? newer.fWireVertIndices : fWireVertIndices;

	fTriangleVertIndices = trianglesAnimated ? newer.fTriangleVertIndices : fTriangleVertIndices;

    fPositions = positionsAnimated ? newer.fPositions : fPositions;
    fNormals   = normalsAnimated   ? newer.fNormals   : fNormals;
    fUVs       = uvsAnimated       ? newer.fUVs       : fUVs;

    fBoundingBox = newer.fBoundingBox;

    fVisibility  = newer.fVisibility;
    
    animated =
        numWiresAnimated || numTrianglesAnimated || numVertsAnimated ||
        wiresAnimated || trianglesAnimated ||
        positionsAnimated || normalsAnimated || uvsAnimated ||
        boundingBoxAnimated || visibilityAnimated;
	return true;
}


//==============================================================================
// CLASS CacheMeshSampler
//==============================================================================

boost::shared_ptr<CacheMeshSampler> CacheMeshSampler::create(
    const bool needUVs)
{
    return boost::make_shared<CacheMeshSampler>(needUVs);
}

CacheMeshSampler::CacheMeshSampler(const bool needUVs)
    : fNeedUVs(needUVs), fUseBaseTessellation(false), fIsAnimated(true)
{}

CacheMeshSampler::~CacheMeshSampler()
{}

bool CacheMeshSampler::addSample(MObject meshObject, bool visibility)
{
	return fAttributeSet.updateAnimatedChannels(
        fIsAnimated, AttributeSet(meshObject, visibility, fNeedUVs));
}

bool CacheMeshSampler::addSampleFromMesh(MFnMesh& mesh)
{
	MDagPath dagPath;
	mesh.getPath(dagPath);
	MString path = dagPath.fullPathName();
    return fAttributeSet.updateAnimatedChannels(
        fIsAnimated, AttributeSet(mesh, fNeedUVs, fUseBaseTessellation), path);
}

boost::shared_ptr<const ShapeSample>
CacheMeshSampler::getSample(double timeInSeconds, const MColor& diffuseColor)
{
    if (!fAttributeSet.fVisibility) {
        // return an empty sample if the shape is invisible
        boost::shared_ptr<ShapeSample> sample = 
        ShapeSample::createEmptySample(timeInSeconds);
        return sample;
    }

    boost::shared_ptr<ShapeSample> sample =
        ShapeSample::create(
            timeInSeconds,
            fAttributeSet.fNumWires,
            fAttributeSet.fNumVerts,
            fAttributeSet.fWireVertIndices,
            fAttributeSet.fTriangleVertIndices,
            fAttributeSet.fPositions,
            fAttributeSet.fBoundingBox,
            diffuseColor,
            fAttributeSet.fVisibility
        );
    sample->setNormals(fAttributeSet.fNormals);
    sample->setUVs(fAttributeSet.fUVs);
    return sample;
}



