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

#include "gpuCacheShapeNode.h"
#include "gpuCacheStrings.h"
#include "gpuCacheConfig.h"
#include "gpuCacheRasterSelect.h"
#include "gpuCacheGLPickingSelect.h"
#include "gpuCacheUtil.h"
#include "gpuCacheSubSceneOverride.h"

#include "gpuCacheDrawTraversal.h"
#include "gpuCacheGLFT.h"

#include "gpuCacheUtil.h"

#include <maya/M3dView.h>
#include <maya/MAnimControl.h>
#include <maya/MDagPath.h>
#include <maya/MFnDagNode.h>
#include <maya/MDrawData.h>
#include <maya/MFileIO.h>
#include <maya/MFileObject.h>
#include <maya/MFnDagNode.h>
#include <maya/MMaterial.h>
#include <maya/MMatrix.h>
#include <maya/MSelectionList.h>
#include <maya/MSelectionMask.h>
#include <maya/MHWGeometryUtilities.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MGlobal.h>
#include <maya/MHardwareRenderer.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MDagPathArray.h>
#include <maya/MEventMessage.h>
#include <maya/MModelMessage.h>
#include <maya/MUiMessage.h>
#include <maya/MItDag.h>
#include <maya/MFnCamera.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnPluginData.h>
#include <maya/MMaterialArray.h>
#include <maya/MObjectArray.h>
#include <maya/MAttributeSpec.h>
#include <maya/MAttributeSpecArray.h>
#include <maya/MAttributeIndex.h>
#include <maya/MPxSurfaceShapeUI.h>
#include <maya/MPointArray.h>
#include <maya/MVectorArray.h>
#include <maya/MExternalContentInfoTable.h>
#include <maya/MExternalContentLocationTable.h>

#include <cassert>
#include <climits>

#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>

//==============================================================================
// Error checking
//==============================================================================

#define MCHECKERROR(STAT,MSG)                   \
    if (!STAT) {                                \
        perror(MSG);                            \
        return MS::kFailure;                    \
    }

#define MREPORTERROR(STAT,MSG)                  \
    if (!STAT) {                                \
        perror(MSG);                            \
    }

#define MCHECKERRORVOID(STAT,MSG)               \
    if (!STAT) {                                \
        perror(MSG);                            \
        return;                                 \
    }



namespace {

using namespace GPUCache;
using namespace GPUCache::ShapeNodePrivate;

//==============================================================================
// LOCAL CLASSES
//==============================================================================

    //==========================================================================
    // CLASS DrawWireframeTraversal
    //==========================================================================

    class DrawWireframeState : public DrawTraversalState
    {
    public:
        DrawWireframeState(
            const Frustum&  frustrum,
            const double    seconds)
            : DrawTraversalState(frustrum, seconds, kPruneNone)
        {}
    };
        
    class DrawWireframeTraversal
        : public DrawTraversal<DrawWireframeTraversal, DrawWireframeState>
    {
    public:

        typedef DrawTraversal<DrawWireframeTraversal, DrawWireframeState> BaseClass;

        DrawWireframeTraversal(
            DrawWireframeState&     state,
            const MMatrix&          xform,
            bool                    isReflection,
            Frustum::ClippingResult parentClippingResult)
            : BaseClass(state, xform, isReflection, parentClippingResult)
        {}
        
        void draw(const boost::shared_ptr<const ShapeSample>& sample)
        {
            if (!sample->visibility()) return;
            gGLFT->glLoadMatrixd(xform().matrix[0]);

            if (sample->isBoundingBoxPlaceHolder()) {
                state().vboProxy().drawBoundingBox(sample);
                GlobalReaderCache::theCache().hintShapeReadOrder(subNode());
                return;
            }

            assert(sample->positions());
            assert(sample->normals());

            state().vboProxy().drawWireframe(sample);
        }
    };


    //==========================================================================
    // CLASS DrawShadedTraversal
    //==========================================================================

    class DrawShadedTypes 
    {
    public:
        enum ColorType {
            kSubNodeColor,
            kDefaultColor,
            kBlackColor,
            kXrayColor
        };
    
        enum NormalsType {
            kFrontNormals,
            kBackNormals
        };
    };

    class DrawShadedState : public DrawTraversalState, public DrawShadedTypes
    {
    public:
        DrawShadedState(
            const Frustum&             frustrum,
            const double               seconds,
            const TransparentPruneType transparentPrune,
            const ColorType            colorType,
            const MColor&              defaultDiffuseColor,
            const NormalsType          normalsType)
            : DrawTraversalState(frustrum, seconds, transparentPrune),
              fColorType(colorType),
              fDefaultDiffuseColor(defaultDiffuseColor),
              fNormalsType(normalsType)
        {}

        ColorType      colorType() const           { return fColorType; }
        const MColor&  defaultDiffuseColor() const { return fDefaultDiffuseColor; } 
        NormalsType    normalsType() const         { return fNormalsType; }
        
    private:
        const ColorType      fColorType;
        const MColor         fDefaultDiffuseColor;
        const NormalsType    fNormalsType;
    };
        
    class DrawShadedTraversal
        : public DrawTraversal<DrawShadedTraversal, DrawShadedState>,
          public DrawShadedTypes
    {
    public:

        typedef DrawTraversal<DrawShadedTraversal, DrawShadedState> BaseClass;

        DrawShadedTraversal(
            DrawShadedState&        state,
            const MMatrix&          xform,
            bool                    isReflection,
            Frustum::ClippingResult parentClippingResult)
            : BaseClass(state, xform, isReflection, parentClippingResult)
        {}
        
        void draw(const boost::shared_ptr<const ShapeSample>& sample)
        {
            if (!sample->visibility()) return;
            gGLFT->glLoadMatrixd(xform().matrix[0]);

            if (sample->isBoundingBoxPlaceHolder()) {
                state().vboProxy().drawBoundingBox(sample, true);
                GlobalReaderCache::theCache().hintShapeReadOrder(subNode());
                return;
            }

            assert(sample->positions());
            assert(sample->normals());
            
            MColor diffuseColor;
            switch (state().colorType()) {
                case kSubNodeColor:
                    diffuseColor = sample->diffuseColor();
                    break;
                case kDefaultColor:
                    diffuseColor = state().defaultDiffuseColor();
                    break;
                case kBlackColor:
                    diffuseColor =
                        MColor(0.0f, 0.0f, 0.0f, sample->diffuseColor()[3]);
                    break;
                case kXrayColor:
                    diffuseColor = MColor(sample->diffuseColor()[0],
                                          sample->diffuseColor()[1],
                                          sample->diffuseColor()[2],
                                          0.3f);
                    break;
                default:
                    assert(0);
                    break;
            }

            if (diffuseColor[3] <= 0.0 ||
                (diffuseColor[3] >= 1.0 &&
                    state().transparentPrune() == DrawShadedState::kPruneOpaque) ||
                (diffuseColor[3] <  1.0 &&
                    state().transparentPrune() == DrawShadedState::kPruneTransparent)) {
                return;
            }

            gGLFT->glColor4f(diffuseColor[0]*diffuseColor[3],
                             diffuseColor[1]*diffuseColor[3],
                             diffuseColor[2]*diffuseColor[3],
                             diffuseColor[3]);

            // The meaning of front faces changes depending whether
            // the transformation has a reflection or not.
            gGLFT->glFrontFace(isReflection() ? MGL_CW : MGL_CCW);

            for (size_t groupId = 0; groupId < sample->numIndexGroups(); ++groupId ) {
                state().vboProxy().drawTriangles(
                    sample, groupId,
                    state().normalsType() == kFrontNormals ?
                    VBOProxy::kFrontNormals : VBOProxy::kBackNormals,
                    VBOProxy::kNoUVs);
            }
        }
    };

	//==========================================================================
	// CLASS ReadBufferVisitor
	//==========================================================================

	class ReadBufferVisitor : public SubNodeVisitor
	{
	public:
		ReadBufferVisitor(double seconds, BufferCache* buffer, MMatrix xformMatrix) 
			: fSeconds(seconds), fMyBufferCache(buffer), fthisXForm(xformMatrix) {}

		virtual void visit(const XformData&   xform,
			const SubNode&     subNode)
		{
			const boost::shared_ptr<const XformSample>& sample =
				xform.getSample(fSeconds);
			
			ReadBufferVisitor newVisitor(fSeconds, fMyBufferCache, sample->xform() * fthisXForm);
			// Recurse into children sub nodes. Expand all instances.
			BOOST_FOREACH(const SubNode::Ptr& child,
				subNode.getChildren() ) {
					child->accept(newVisitor);
			}
		}

		virtual void visit(const ShapeData&   shape,
			const SubNode&     subNode)
		{
			const boost::shared_ptr<const ShapeSample>& sample =
				shape.getSample(fSeconds);
			if (!sample) return;
			
			fMyBufferCache->fNumTriangles.push_back(sample->numTriangles());
			fMyBufferCache->fNumEdges.push_back(sample->numWires());
			fMyBufferCache->fTotalNumVerts+=sample->numVerts();
			fMyBufferCache->fTotalNumTris+=sample->numTriangles();
			VertexBuffer::ReadInterfacePtr vertexPositionRead= sample->positions()->readableInterface();
			if (sample->triangleVertIndices(0) && sample->wireVertIndices()) {
					fMyBufferCache->fPositions.push_back(vertexPositionRead);
					IndexBuffer::ReadInterfacePtr triangleIndexRead = sample->triangleVertIndices(0)->readableInterface();
					IndexBuffer::ReadInterfacePtr edgeIndexRead = sample->wireVertIndices()->readableInterface();
					fMyBufferCache->fTriangleVertIndices.push_back(triangleIndexRead);
					fMyBufferCache->fEdgeVertIndices.push_back(edgeIndexRead);
					fMyBufferCache->fBoundingBoxes.push_back(sample->boundingBox());
					fMyBufferCache->fXFormMatrix.push_back(fthisXForm);
					fMyBufferCache->fXFormMatrixInverse.push_back(fthisXForm.inverse());
					fMyBufferCache->fUseCachedBuffers = true;
					fMyBufferCache->fNumShapes ++;
			}
		}

	private:
		BufferCache* fMyBufferCache;
		MMatrix fthisXForm;
		double fSeconds;
	};

    //==========================================================================
    // CLASS NbPrimitivesVisitor
    //==========================================================================

    class NbPrimitivesVisitor : public SubNodeVisitor
    {
    public:

        NbPrimitivesVisitor(double seconds) 
            : fSeconds(seconds),
              fNumWires(0),
              fNumTriangles(0)
        {}

        size_t numWires()       { return fNumWires; }
        size_t numTriangles()   { return fNumTriangles; }
        
        virtual void visit(const XformData&   xform,
                           const SubNode&     subNode)
        {
            // Recurse into children sub nodes. Expand all instances.
            BOOST_FOREACH(const SubNode::Ptr& child,
                          subNode.getChildren() ) {
                child->accept(*this);
            }
        }
        
        virtual void visit(const ShapeData&   shape,
                           const SubNode&     subNode)
        {
            const boost::shared_ptr<const ShapeSample>& sample =
                shape.getSample(fSeconds);
            if (!sample) return;

            fNumWires       += sample->numWires();
            fNumTriangles   += sample->numTriangles();
        }
        
    private:
    
        const double    fSeconds;
        size_t          fNumWires;
        size_t          fNumTriangles;
    };


    //==========================================================================
    // CLASS SnapTraversal
    //==========================================================================

    class SnapTraversalState : public DrawTraversalState 
    {
    public:

        SnapTraversalState(const Frustum&  frustrum,
                           const double    seconds,
                           const MMatrix&  localToPort,
                           const MMatrix&  inclusiveMatrix,
                           MSelectInfo&    snapInfo)
            : DrawTraversalState(frustrum, seconds, kPruneNone),
              fLocalToPort(localToPort),
              fInclusiveMatrix(inclusiveMatrix),
              fSnapInfo(snapInfo),
              fSelected(false)
        {}

        const MMatrix& localToPort() const      { return fLocalToPort; }
        const MMatrix& inclusiveMatrix() const  { return fInclusiveMatrix; }
        MSelectInfo& snapInfo()                 { return fSnapInfo; }
        
        bool selected() const { return fSelected; }
        void setSelected()    { fSelected  = true; }
        
    private:
        const MMatrix   fLocalToPort;
        const MMatrix   fInclusiveMatrix;
        MSelectInfo&    fSnapInfo;
        bool            fSelected;
    };


    class SnapTraversal
        : public DrawTraversal<SnapTraversal, SnapTraversalState>
    {
    public:

        typedef DrawTraversal<SnapTraversal, SnapTraversalState> BaseClass;

        SnapTraversal(
            SnapTraversalState&     state,
            const MMatrix&          xform,
            bool                    isReflection,
            Frustum::ClippingResult parentClippingResult)
            : BaseClass(state, xform, false, parentClippingResult)
        {}

        void draw(const boost::shared_ptr<const ShapeSample>& sample)
        {
            if (!sample->visibility()) return;
            if (sample->isBoundingBoxPlaceHolder()) return;

            assert(sample->positions());
            VertexBuffer::ReadInterfacePtr readable = sample->positions()->readableInterface();
            const float* const positions = readable->get();
            
            unsigned int srx, sry, srw, srh;
            state().snapInfo().selectRect(srx, sry, srw, srh);
            double srxl = srx;
            double sryl = sry;
            double srxh = srx + srw;
            double sryh = sry + srh;

            const MMatrix localToPort     = xform() * state().localToPort();
            const MMatrix inclusiveMatrix = xform() * state().inclusiveMatrix();
            
            // Loop through all vertices of the mesh.
            // See if they lie withing the view frustum,
            // then send them to snapping check.
            size_t numVertices = sample->numVerts();
            for (size_t vertexIndex=0; vertexIndex<numVertices; vertexIndex++)
            {
                const float* const currentPoint = &positions[vertexIndex*3];

                // Find the closest snapping point using the CPU. This is
                // faster than trying to use OpenGL picking.
                MPoint loPt(currentPoint[0], currentPoint[1], currentPoint[2]);
                MPoint pt = loPt * localToPort;
                pt.rationalize();

                if (pt.x >= srxl && pt.x <= srxh &&
                    pt.y >= sryl && pt.y <= sryh &&
                    pt.z >= 0.0  && pt.z <= 1.0) {
                    MPoint wsPt = loPt * inclusiveMatrix;
                    wsPt.rationalize();
                    state().snapInfo().setSnapPoint(wsPt);
                    state().setSelected();
                }
            }
        }
    };


    //==========================================================================
    // CLASS WaitCursor
    //==========================================================================

    class WaitCursor
    {
    public:
        WaitCursor()
        {
            MGlobal::executeCommand("waitCursor -state 1");
        }

        ~WaitCursor()
        {
            MGlobal::executeCommand("waitCursor -state 0");
        }

    private:
        // Forbidden and not implemented.
        WaitCursor(const WaitCursor&);
        const WaitCursor& operator=(const WaitCursor&);
    };
}


namespace GPUCache {

//==============================================================================
// CLASS ShapeNode
//==============================================================================

const MTypeId ShapeNode::id(0x580000C4);

const MString ShapeNode::drawDbClassificationGeometry(
    "drawdb/geometry/gpuCache" );

const MString ShapeNode::drawDbClassificationSubScene(
    "drawdb/subscene/gpuCache" );

const MString ShapeNode::drawRegistrantId("gpuCache" );

MObject ShapeNode::aCacheFileName;
MObject ShapeNode::aCacheGeomPath;
MCallbackId ShapeNode::fsModelEditorChangedCallbackId;

const char* ShapeNode::nodeTypeName = "gpuCache";
const char* ShapeNode::selectionMaskName = "gpuCache";

static std::vector<MCallbackId> s3dViewPostRenderCallbackIds;
static std::vector<MCallbackId> s3dViewDeletedCallbackIds;
static int sNb3dViewPostRenderCallbacks = 0;

enum ModelEditorState {
    kDefaultViewportOnly,
    kViewport2Only,
    kDefaultViewportAndViewport2
};
static ModelEditorState sModelEditorState = kDefaultViewportAndViewport2;


static void viewPostRender(const MString &str, void* /*clientData*/)
{
    VBOBuffer::nextRefresh();
}


static void clearPostRenderCallbacks()
{
    {
        std::vector<MCallbackId>::iterator it  = s3dViewPostRenderCallbackIds.begin();
        std::vector<MCallbackId>::iterator end = s3dViewPostRenderCallbackIds.end();
        for (; it != end; ++it) {
            MMessage::removeCallback(*it);
        }
        s3dViewPostRenderCallbackIds.clear();
    }

    {
        std::vector<MCallbackId>::iterator it  = s3dViewDeletedCallbackIds.begin();
        std::vector<MCallbackId>::iterator end = s3dViewDeletedCallbackIds.end();
        for (; it != end; ++it) {
            MMessage::removeCallback(*it);
        }
        s3dViewDeletedCallbackIds.clear();
    }

    sNb3dViewPostRenderCallbacks = 0;
}

static void uiDeleted(void* clientData)
{
    MUintPtrSz idx = reinterpret_cast<MUintPtrSz>(clientData);

    MMessage::removeCallback(s3dViewPostRenderCallbackIds[idx]);
    s3dViewPostRenderCallbackIds[idx] = MCallbackId();

    MMessage::removeCallback(s3dViewDeletedCallbackIds[idx]);
    s3dViewDeletedCallbackIds[idx] = MCallbackId();

    --sNb3dViewPostRenderCallbacks;
    assert(sNb3dViewPostRenderCallbacks >= 0);
}

static void modelEditorChanged(void* /*clientData*/)
{
    // When using the MPxSubSceneOverride, we have to free-up
    // the VBO used by a given renderer (default vs VP2.0) when it
    // is no longer in use!
        
    static bool sVBOsClean             = false;
    static bool sViewport2BuffersClean = false;

    // Loop through all the viewports to see if we have any
    // visible Viewport 1.0 or Viewport 2.0
    bool hasDefaultViewport = false;
    bool hasViewport2       = false;
    unsigned int viewCount = M3dView::numberOf3dViews();
    for (unsigned int i = 0; i < viewCount; i++) {
        M3dView view;
        M3dView::get3dView(i, view);

        // the i-th viewport's renderer and visibility
        M3dView::RendererName renderer = view.getRendererName(NULL);
        bool visible                   = view.isVisible();

        if (visible && (renderer == M3dView::kDefaultQualityRenderer
                || renderer == M3dView::kHighQualityRenderer
                || renderer == M3dView::kExternalRenderer)) {
            hasDefaultViewport = true;
        }

        if (visible && renderer == M3dView::kViewport2Renderer) {
            hasViewport2 = true;
        }
    }

    // if we have Default/High Quality viewports, we may want to clean VBOs
    if (hasDefaultViewport) {
        sVBOsClean = false;
    }

    // if we have Viewport 2.0, we may want to clean VP2 buffers
    if (hasViewport2) {
        sViewport2BuffersClean = false;
    }

    // free VBOs if we have no Default/High Quality viewports
    if (!hasDefaultViewport && !sVBOsClean) {
        VBOBuffer::clear();
        
        // we have cleaned all VBOs
        sVBOsClean = true;
    }

    // free Viewport 2.0 buffers if we have no Viewport 2.0
    if (!hasViewport2 && !sViewport2BuffersClean)  {
        SubSceneOverride::clear();
        sViewport2BuffersClean = true;
    }

    // Set the current model editor state.
    if (hasDefaultViewport && hasViewport2) {
        sModelEditorState = kDefaultViewportAndViewport2;
    }
    else if (hasDefaultViewport) {
        sModelEditorState = kDefaultViewportOnly;
    }
    else if (hasViewport2) {
        sModelEditorState = kViewport2Only;
    }
    else {
        sModelEditorState = kDefaultViewportAndViewport2;
    }
}

static void nodeAddedToModel(MObject& node, void* clientData)
{
	MFnDagNode dagNode(node);
	ShapeNode* shapeNode = (ShapeNode*)dagNode.userNode();
	assert(shapeNode);
	if (!shapeNode) return;

	shapeNode->addedToModelCB();
}

static void nodeRemovedFromModel(MObject& node, void* clientData)
{
    MFnDagNode dagNode(node);
    ShapeNode* shapeNode = (ShapeNode*)dagNode.userNode();
    assert(shapeNode);
    if (!shapeNode) return;

    shapeNode->removedFromModelCB();
}

void* ShapeNode::creator()
{
    return new ShapeNode;
}

MStatus ShapeNode::initialize()
{
    MStatus stat;
    MFnTypedAttribute typedAttrFn;

    // file name
    aCacheFileName = typedAttrFn.create("cacheFileName", "cfn",
        MFnData::kString, MObject::kNullObj, &stat);
    typedAttrFn.setInternal(true);
    typedAttrFn.setUsedAsFilename(true);
    stat = MPxNode::addAttribute(aCacheFileName);
    MCHECKERROR(stat, "MPxNode::addAttribute(aCacheFileName)");

    // geometry path used to find the geometry within the cache file
    aCacheGeomPath = typedAttrFn.create("cacheGeomPath", "cmp",
        MFnData::kString, MObject::kNullObj, &stat);
    typedAttrFn.setInternal(true);
    stat = MPxNode::addAttribute(aCacheGeomPath);
    MCHECKERROR(stat, "MPxNode::addAttribute(aCacheFileName)");

    if (Config::vp2OverrideAPI() != Config::kMPxDrawOverride) {
        fsModelEditorChangedCallbackId = MEventMessage::addEventCallback(
            "modelEditorChanged", modelEditorChanged, NULL, &stat);
        MCHECKERROR(stat, "MEventMessage::addEventCallback(modelEditorChanged)");
    }

    // Find the correct initial state for the type of viewport that we have.
    modelEditorChanged(NULL);

    stat = DisplayPref::initCallback();
    MCHECKERROR(stat, "DisplayPref::initCallbacks()");
    
    return stat;
}

MStatus ShapeNode::uninitialize()
{
    if (Config::vp2OverrideAPI() != Config::kMPxDrawOverride) {
        MEventMessage::removeCallback(fsModelEditorChangedCallbackId);
    }

    DisplayPref::removeCallback();

    clearPostRenderCallbacks();

	// The CacheFileRegistry and GlobalReaderCache both contain static maps. The
	// CacheFileRegistry contains references to the GlobalReaderCache (via CacheReaderProxy)
	// We cannot rely on the order of static destruction. Ensure that the CacheFileRegistry
	// is cleared before we unload the plug-in.
	//
	// While the CacheFileRegistry should ideally already be emptied at this point,
	// (hence the assert) this provides an added guarantee.
	//
	assert( CacheFileRegistry::theCache().size() == 0 );
	CacheFileRegistry::theCache().clear();
    
    return MStatus::kSuccess;
}

MStatus ShapeNode::init3dViewPostRenderCallbacks()
{
    MStatus exitStatus;

    if (int(M3dView::numberOf3dViews()) != sNb3dViewPostRenderCallbacks) {
        clearPostRenderCallbacks();

        static MString listEditorPanelsCmd = "gpuCacheListModelEditorPanels";
        MStringArray editorPanels;
        exitStatus = MGlobal::executeCommand(listEditorPanelsCmd, editorPanels);
        MCHECKERROR(exitStatus, "gpuCacheListModelEditorPanels");
    
        if (exitStatus == MStatus::kSuccess) {
            sNb3dViewPostRenderCallbacks = editorPanels.length();
            for (int i=0; i<sNb3dViewPostRenderCallbacks; ++i) {
                MStatus status;
                MCallbackId callbackId = MUiMessage::add3dViewPostRenderMsgCallback(
                    editorPanels[i], viewPostRender, NULL, &status);
                MREPORTERROR(status, "MUiMessage::add3dViewPostRenderMsgCallback()");
                if (status != MStatus::kSuccess) {
                    s3dViewDeletedCallbackIds.push_back(MCallbackId());
                    s3dViewPostRenderCallbackIds.push_back(MCallbackId());
                    exitStatus = MStatus::kFailure;
                    continue;
                }
                s3dViewPostRenderCallbackIds.push_back(callbackId);

                callbackId = MUiMessage::addUiDeletedCallback(
                    editorPanels[i], uiDeleted, reinterpret_cast<void*>(MUintPtrSz(i)), &status);
                MREPORTERROR(status, "MUiMessage::addUiDeletedCallback()");
                if (status != MStatus::kSuccess) {
                    s3dViewDeletedCallbackIds.push_back(MCallbackId());
                    exitStatus = MStatus::kFailure;
                    continue;
                }
                s3dViewDeletedCallbackIds.push_back(callbackId);
            }

            assert(M3dView::numberOf3dViews() == s3dViewPostRenderCallbackIds.size());
            assert(M3dView::numberOf3dViews() == s3dViewDeletedCallbackIds.size());
            assert(int(M3dView::numberOf3dViews()) == sNb3dViewPostRenderCallbacks);
        }
    }

    return exitStatus;
}

ShapeNode::ShapeNode()
:	fCachedGeometry()
,	fCacheReadingState(kCacheReadingDone)
{
	fBufferCache = NULL;
}

ShapeNode::~ShapeNode()
{
	while(!fSpatialSub.empty()){
		delete fSpatialSub.back();
		fSpatialSub.pop_back();
	}
	delete fBufferCache;
}

void ShapeNode::postConstructor()
{
    setRenderable(true);

    // Explicitly initialize config when the first gpuCache node is created.
    //   When initializing Config, it will access video adapters via WMI 
    //   and Windows will sometimes send OnPaint message to Maya and thus cause a refresh.
    //   The wired OnPaint message will crash VP2 and gpuCache.
    Config::initialize();

	MModelMessage::addNodeAddedToModelCallback(thisMObject(), nodeAddedToModel);
    MModelMessage::addNodeRemovedFromModelCallback(thisMObject(), nodeRemovedFromModel);
}

bool ShapeNode::isBounded() const
{
    return true;
}

unsigned int ShapeNode::getIntersectionAccelerator(
	const 		gpuCacheIsectAccelParams& accelParams,
				double		seconds
) const
	//
	//	Description:
	//
	//		Creates a gpuCacheSpatialSubdivision intersection acceleration structure
	//		for this ShapeNode.  If cacheForReuse is true, then the structure will
	//		be stored, and subsequent requests for an identically-configured
	//		accelerator will return the cached one.  If cacheForReuse is false,
	//		then the method simply returns an accelerator that the client is
	//		responsible for deleting when they are done with it.
	//
	//		The supplied gpucacheIsectAccelParams object defines the configuration
	//		of the accelerator (subdivision algorithm, number of voxels).  These
	//		objects can be obtained from the static "create*" methods on 
	//		gpuCacheIsectAccelParams.
	//
{
	if (!fCacheFileEntry || fCacheFileEntry->fReadState != CacheFileEntry::kReadingDone) {
		return 0;
	}

	if( fBufferCache !=NULL && (fBufferCache->fUseCachedBuffers && fBufferCache->fBufferReadTime==seconds) && (!fSpatialSub.empty()) && (fSpatialSub[0]->matchesParams(accelParams)) )
	{
		return fSpatialSub.size();
	}
	else
	{
		while(!fSpatialSub.empty()){
			delete fSpatialSub.back(); fSpatialSub.pop_back();
		}
		const SubNode::Ptr subNode = getCachedGeometry();
		if(readBuffers(subNode,seconds)){
			for(unsigned int s=0; s < fBufferCache->fNumShapes; s++){
				const index_t* srcTriangleVertIndices = fBufferCache->fTriangleVertIndices[s]->get();
				const float* srcPositions = fBufferCache->fPositions[s]->get();
				fSpatialSub.push_back (new gpuCacheSpatialSubdivision(fBufferCache->fNumTriangles[s], srcTriangleVertIndices, srcPositions, fBufferCache->fBoundingBoxes[s], accelParams));
			}
			return fSpatialSub.size();
		}
	}
	return 0;
}

bool ShapeNode::getEdgeSnapPoint(const MPoint &rayPointSrc, const MVector &rayDirectionSrc, MPoint &theClosestPoint) {
	const double seconds = MAnimControl::currentTime().as(MTime::kSeconds);
	gpuCacheIsectAccelParams accelParams = gpuCacheIsectAccelParams::autoUniformGridParams(); 
	unsigned int numAccels = getIntersectionAccelerator(accelParams, seconds);
	bool foundPoint = false;

	if(numAccels > 0 && numAccels == fBufferCache->fNumShapes) {
		unsigned int closestShape=0;
		double minDist = std::numeric_limits<double>::max();
		bool *checkedBox = new bool[fBufferCache->fNumShapes];
		double *allDists = new double[fBufferCache->fNumShapes];
		for(unsigned int s=0; s<fBufferCache->fNumShapes; s++){
			checkedBox[s] = false;
			if(fBufferCache->fNumTriangles[s]>0){
				MBoundingBox xformBBox = fBufferCache->fBoundingBoxes[s];
				xformBBox.transformUsing(fBufferCache->fXFormMatrix[s]);
				MPoint closestPointOnBox;
				allDists[s] = gpuCacheIsectUtil::getEdgeSnapPointOnBox(rayPointSrc, rayDirectionSrc, xformBBox, closestPointOnBox);
				if(allDists[s] < minDist){
					minDist = allDists[s];
					closestShape = s;
				}
			} else {
				allDists[s] = std::numeric_limits<double>::max();
			}
		}
		std::vector<int> potentialShapes;

		for(unsigned int s=0; s<fBufferCache->fNumShapes; s++){
			if(allDists[s]==minDist){
				potentialShapes.push_back(s);
				checkedBox[s]=true;
			}
		}

		double coef_plane = rayDirectionSrc * rayPointSrc;
		minDist = std::numeric_limits<double>::max();
		while(!potentialShapes.empty()){	
			closestShape = potentialShapes.back();
			potentialShapes.pop_back();
			
			if(allDists[closestShape]<=minDist){
				const index_t* srcTriangleVertIndices = fBufferCache->fTriangleVertIndices[closestShape]->get();
				const float* srcPositions = fBufferCache->fPositions[closestShape]->get();
				MPoint clsPoint;
				double dist = fSpatialSub[closestShape]->getEdgeSnapPoint(fBufferCache->fNumTriangles[closestShape], srcTriangleVertIndices, srcPositions,
					rayPointSrc*fBufferCache->fXFormMatrixInverse[closestShape], rayDirectionSrc*fBufferCache->fXFormMatrixInverse[closestShape], clsPoint);
				clsPoint *= fBufferCache->fXFormMatrix[closestShape];
				// project onto coef_plane to find closest
				double d = coef_plane - rayDirectionSrc * clsPoint;
				MPoint projectedClsPoint = clsPoint + rayDirectionSrc * d;
				dist = rayPointSrc.distanceTo(projectedClsPoint);
				if(dist < minDist){
					minDist = dist;
					theClosestPoint = clsPoint;
					foundPoint = true;
					for(unsigned int s=0; s<fBufferCache->fNumShapes; s++){
						if(!checkedBox[s] && allDists[s]<=minDist){
							std::vector<int>::iterator it = potentialShapes.begin();
							while (it != potentialShapes.end() && allDists[s]<allDists[*it])
							{
								it++;
							}
							potentialShapes.insert(it,s);
							checkedBox[s]=true;
						}
					}
				}
			}
		}
		delete[] checkedBox;
		delete[] allDists;
	}
	return foundPoint;
}

bool ShapeNode::closestPoint( const MPoint &raySource, const MVector &rayDirection, MPoint &theClosestPoint, MVector &theClosestNormal, bool findClosestOnMiss, double tolerance) 
{
	if(closestIntersectWithNorm(raySource,rayDirection,theClosestPoint,theClosestNormal)) {
		return true;
	} else if(findClosestOnMiss) {
		if(getEdgeSnapPoint(raySource,rayDirection,theClosestPoint)) {
			return true;
		}
	}
	return false;
}

bool ShapeNode::canMakeLive() const { 
	return true; 
}

/*
This function is used to create a cache 
with everything a live gpuCache will require.
The cache is re-created every time the frame
changes.
*/
bool ShapeNode::readBuffers(const SubNode::Ptr subNode, double seconds)const{
	if(subNode == NULL) return false;
	if(fBufferCache!=NULL && fBufferCache->fUseCachedBuffers && fBufferCache->fBufferReadTime==seconds) return true;
	if(fBufferCache!=NULL){
		delete fBufferCache;
	}
	seconds = MAnimControl::currentTime().as(MTime::kSeconds);
	fBufferCache = new BufferCache(seconds);
	if(fBufferCache == NULL){
		return false;
	} 
	MMatrix identMat;
	identMat.setToIdentity();
	ReadBufferVisitor visitor(seconds, fBufferCache, identMat);
	subNode->accept(visitor);
	if(fBufferCache->fUseCachedBuffers && (fBufferCache->fNumShapes)>1 && (fBufferCache->fTotalNumTris) > 1000000){
		MGlobal::executeCommandOnIdle( MString("gpuCacheManyShapesDialog(") + fBufferCache->fTotalNumVerts + MString(")") );
	}
	return fBufferCache->fUseCachedBuffers;
}

void ShapeNode::closestPoint(const MPoint &toThisPoint, MPoint &theClosestPoint, double tolerance) {
	const double seconds = MAnimControl::currentTime().as(MTime::kSeconds);
	gpuCacheIsectAccelParams accelParams = gpuCacheIsectAccelParams::autoUniformGridParams(); 
	unsigned int numAccels = getIntersectionAccelerator(accelParams, seconds);
	
	if(numAccels > 0 && numAccels == fBufferCache->fNumShapes) {
		unsigned int closestShape=0;
		double minDist = std::numeric_limits<double>::max();
		bool *checkedBox = new bool[fBufferCache->fNumShapes];
		double *allDists = new double[fBufferCache->fNumShapes];
		for(unsigned int s=0; s<fBufferCache->fNumShapes; s++){
			checkedBox[s] = false;
			if(fBufferCache->fNumTriangles[s]>0){
				MBoundingBox xformBBox = fBufferCache->fBoundingBoxes[s];
				xformBBox.transformUsing(fBufferCache->fXFormMatrix[s]);
				MPoint closestPointOnBox;
				allDists[s] = gpuCacheIsectUtil::getClosestPointOnBox(toThisPoint,xformBBox,closestPointOnBox);
				if(allDists[s] < minDist){
					minDist = allDists[s];
					closestShape = s;
				}
			} else {
				allDists[s] = std::numeric_limits<double>::max();
			}
		}
		std::vector<int> potentialShapes;
		potentialShapes.push_back(closestShape);
		minDist = std::numeric_limits<double>::max();
		while(!potentialShapes.empty()){	
			closestShape = potentialShapes.back();
			potentialShapes.pop_back();
			checkedBox[closestShape]=true;
			if(allDists[closestShape]<minDist){
				const index_t* srcTriangleVertIndices = fBufferCache->fTriangleVertIndices[closestShape]->get();
				const float* srcPositions = fBufferCache->fPositions[closestShape]->get();
				MPoint clsPoint;
				fSpatialSub[closestShape]->closestPointToPoint(fBufferCache->fNumTriangles[closestShape], srcTriangleVertIndices, srcPositions, toThisPoint * fBufferCache->fXFormMatrixInverse[closestShape], clsPoint);
				clsPoint *= fBufferCache->fXFormMatrix[closestShape];
				double dist = clsPoint.distanceTo(toThisPoint);
				if(dist < minDist){
					minDist = dist;
					theClosestPoint = clsPoint;
					for(unsigned int s=0; s<fBufferCache->fNumShapes; s++){
						if(!checkedBox[s] && allDists[s]<minDist){
							std::vector<int>::iterator it = potentialShapes.begin();
							while (it != potentialShapes.end() && allDists[s]<allDists[*it])
							{
								it++;
							}
							potentialShapes.insert(it,s);
						}
					}
				}
			}
		}
		delete[] checkedBox;
		delete[] allDists;
	}
}

MStatus ShapeNode::closestIntersectWithNorm (const MPoint &toThisPoint, const MVector &thisDirection, MPoint &theClosestPoint, MVector &theClosestNormal){
	const double seconds = MAnimControl::currentTime().as(MTime::kSeconds);
	gpuCacheIsectAccelParams accelParams = gpuCacheIsectAccelParams::autoUniformGridParams(); 
	unsigned int numAccels = getIntersectionAccelerator(accelParams, seconds); 

	MStatus returnStatus = MStatus::kFailure;
	if(numAccels > 0 && numAccels == fBufferCache->fNumShapes) {
		double minDist = std::numeric_limits<double>::max();
		for(unsigned int s=0; s<fBufferCache->fNumShapes; s++){
			const index_t* srcTriangleVertIndices = fBufferCache->fTriangleVertIndices[s]->get();
			const float* srcPositions = fBufferCache->fPositions[s]->get();
			MPoint clsPoint;
			MVector clsNormal;
			if( fSpatialSub[s]->closestIntersection(fBufferCache->fNumTriangles[s], srcTriangleVertIndices, srcPositions, toThisPoint * fBufferCache->fXFormMatrixInverse[s], thisDirection * fBufferCache->fXFormMatrixInverse[s], 
				/*maxParam = */999999, clsPoint, clsNormal)){
					clsPoint *= fBufferCache->fXFormMatrix[s];
					clsNormal *= fBufferCache->fXFormMatrix[s];
					double dist = clsPoint.distanceTo(toThisPoint);
					if(dist < minDist){
						minDist = dist;
						theClosestPoint = clsPoint;
						theClosestNormal = clsNormal;
						returnStatus = MStatus::kSuccess;
					}
			}

		} 
	}

	return returnStatus;
}

MBoundingBox ShapeNode::boundingBox() const
{
    // Extract the cached geometry.
    const SubNode::Ptr subNode = getCachedGeometry();
    if (!subNode) {
        return MBoundingBox();
    }

    const SubNodeData::Ptr subNodeData = subNode->getData();
    if (!subNodeData) return MBoundingBox();

    const double seconds = MAnimControl::currentTime().as(MTime::kSeconds);

    // Handle transforms.
    const XformData::Ptr xform =
        boost::dynamic_pointer_cast<const XformData>(subNodeData);
    if (xform) {
        const boost::shared_ptr<const XformSample>& sample =
            xform->getSample(seconds);
        return sample->boundingBox();
    }

    // Handle shapes.
    const ShapeData::Ptr shape =
        boost::dynamic_pointer_cast<const ShapeData>(subNodeData);
    if (shape) {
        const boost::shared_ptr<const ShapeSample>& sample =
            shape->getSample(seconds);
        return sample->boundingBox();
    }
    
    return MBoundingBox();
}

bool ShapeNode::getInternalValueInContext(const MPlug& plug,
    MDataHandle& dataHandle, MDGContext& ctx)
{
    if (plug == aCacheFileName) {
        dataHandle.setString(fCacheFileName);
        return true;
    }
    else if (plug == aCacheGeomPath) {
        dataHandle.setString(fCacheGeomPath);
        return true;
    }

    return MPxNode::getInternalValueInContext(plug, dataHandle, ctx);
}

bool ShapeNode::setInternalValues(
    const MString& newFileName,
    const MString& newGeomPath
)
{
    const MString oldFileName = fCacheFileName;
	const MString oldResolvedFileName = fResolvedCacheFileName;
    const MString oldGeomPath = fCacheGeomPath;

	// Compute the resolved filename
	MFileObject newFile;
	newFile.setRawFullName(newFileName);
	newFile.setResolveMethod(MFileObject::kInputFile);
	MString newResolvedFileName = newFile.resolvedFullName();
	if( newResolvedFileName.length() == 0 )
	{
		newResolvedFileName = newFileName;
	}

    // Early out if nothing has changed.
	//
	// Compare only the raw file names. We still want to update the attributes
	// if the raw file names have changed, even if the resolved file names still
	// point to the same file.
	bool fileChanged = (newFileName != oldFileName);
	bool pathChanged = (newGeomPath != oldGeomPath);
    if (!fileChanged && !pathChanged)
	{
        return true;
	}

	if( fileChanged )
	{
		// Early out if the new file path has already been read.
		if( newResolvedFileName.length() > 0 )
		{
			CacheFileEntry::MPtr entry = CacheFileRegistry::theCache().find(newResolvedFileName);
			if( entry )
			{
				// Invalidate viewport and this shape's cache data.
				fCachedGeometry.reset();
				fCachedMaterial.reset();
				MHWRender::MRenderer::setGeometryDrawDirty(thisMObject());

				// Set the new cache file, path and entry
				fCacheFileName = newFileName;
				fResolvedCacheFileName = newResolvedFileName;
				fCacheGeomPath = newGeomPath;
				fCacheFileEntry = entry;

				// Set to reading file - this will poll for updates against the
				// entry in getCachedGeometry().
				fCacheReadingState = kCacheReadingFile;

				// The fCacheFileName has changed, update the shape registry
				// NOTE: Use resolved file name for registry.
				CacheShapeRegistry::theCache().remove(oldResolvedFileName, thisMObject());
				CacheShapeRegistry::theCache().insert(newResolvedFileName, thisMObject());

				// The fCacheFileEntry has changed, clean up the registry as necessary
				// NOTE: Use resolved file name for registry.
				CacheFileRegistry::theCache().cleanUp(oldResolvedFileName);
				return true;
			}
		}

		// Update the internal attributes
		fCacheFileName = newFileName;
		fResolvedCacheFileName = newResolvedFileName;
		fCacheGeomPath = newGeomPath;

		// The fCacheFileName has changed, update the shape registry
		// NOTE: Use resolved file name for registry.
		CacheShapeRegistry::theCache().remove(oldResolvedFileName, thisMObject());
		CacheShapeRegistry::theCache().insert(newResolvedFileName, thisMObject());

		// Invalidate viewport and force a re-reading of the cache file.
		fCachedGeometry.reset();
		fCachedMaterial.reset();
		fCacheFileEntry.reset();
		MHWRender::MRenderer::setGeometryDrawDirty(thisMObject());

		// The fCacheFileEntry has changed, clean up the registry as necessary
		CacheFileRegistry::theCache().cleanUp(oldResolvedFileName);

		// Insert a CacheFileEntry into the registry so only one read is scheduled
		// for a given file/path pair.
		if( newResolvedFileName.length() > 0 )
		{
			// NOTE: Use resolved file name for registry.
			CacheFileEntry::MPtr newEntry = CacheFileEntry::create( newResolvedFileName );
			CacheFileRegistry::theCache().insert( newResolvedFileName, newEntry );
			fCacheFileEntry = newEntry;
			fCacheReadingState = kCacheReadingFile;
		}
	}
	else
	{
		// Only the geomPath changed
		fCacheGeomPath = newGeomPath;

		// Set to reading file - this will poll for updates against the
		// entry in getCachedGeometry().
		fCacheReadingState = kCacheReadingFile;
	}
    return true;
}

bool ShapeNode::setInternalValueInContext(const MPlug& plug,
    const MDataHandle& dataHandle, MDGContext& ctx)
{
    if (plug == aCacheFileName) {
        MString newFileName = dataHandle.asString();
        return setInternalValues(newFileName, fCacheGeomPath);
    }
    else if (plug == aCacheGeomPath) {
        MString newGeomPath = dataHandle.asString();
        return setInternalValues(fCacheFileName, newGeomPath);
    }

    return MPxNode::setInternalValueInContext(plug, dataHandle, ctx);
}

void ShapeNode::refreshCachedGeometry( bool clearFileCache /* = false */ )
{
    // Back up attributes
    const MString cacheFileName = fCacheFileName;
	const MString resolvedCacheFileName = fResolvedCacheFileName;
    const MString cacheGeomPath = fCacheGeomPath;

    // Cancel background read
    if (fCacheFileEntry && fCacheFileEntry->fReadState != CacheFileEntry::kReadingDone) {
        GlobalReaderCache::theCache().cancelRead(fCacheFileEntry.get());
        fCacheFileEntry->fReadState = CacheFileEntry::kReadingDone;
    }

	// Cancel cache read
	if (fCacheReadingState != kCacheReadingDone) {
		fCacheReadingState = kCacheReadingDone;
	}

	// Remove any CacheFileEntry for this file.
	if( clearFileCache )
	{
		CacheFileRegistry::theCache().remove(resolvedCacheFileName);
	}

	// Remove any CacheShapeRegistry entry for this shape.
	CacheShapeRegistry::theCache().remove(resolvedCacheFileName, thisMObject());

    // Reset this node
    fCacheFileName.clear();
	fResolvedCacheFileName.clear();
    fCacheGeomPath.clear();
    fCachedGeometry.reset();
    fCachedMaterial.reset();
	fCacheFileEntry.reset();

    // Set the attributes again
    setInternalValues(cacheFileName, cacheGeomPath);

	// Update any other shapes that refer to the same file to refresh as well.
	if( clearFileCache )
	{
		refreshOtherCachedShapes( resolvedCacheFileName );
	}
}

void ShapeNode::refreshOtherCachedShapes( const MString& cacheFileName )
{
	// Do not refresh other shapes while reading a file.
	if( MFileIO::isReadingFile() )
	{
		return;
	}

	// Determine the full resolved path from the cacheFileName
	MFnDependencyNode nodeFn;
	std::vector<MObjectHandle> otherShapes;
	CacheShapeRegistry::theCache().find( cacheFileName, otherShapes );
	for( size_t i = 0; i < otherShapes.size(); i++ )
	{
		if( !otherShapes[i].isValid() )
		{
			continue;
		}
		nodeFn.setObject( otherShapes[i].object() );
		assert( nodeFn.typeId() == id );
		ShapeNode* shape = (ShapeNode*) nodeFn.userNode();
		assert( shape );

		// File cache has already been cleared, do not request clearFileCache
		shape->refreshCachedGeometry( false );
	}
}

const SubNode::Ptr& ShapeNode::getCachedGeometry() const
{
    // We can't have both a reader and geometry/material that has already been
    // read!
	CacheFileEntry::MPtr entry = fCacheFileEntry;
    assert(!(entry && entry->fCacheReaderProxy && (entry->fCachedGeometry || entry->fCachedMaterial)));

	// Retrieve the CacheFileEntry for this shape's cacheFile
	if( fCacheReadingState == kCacheReadingFile )
	{
		// Must have a valid entry if this shape is reading from the cache.
		assert(entry);

		if (fCacheFileEntry->fCacheReaderProxy) {
			if (Config::backgroundReading() && MGlobal::mayaState() != MGlobal::kBatch) {
				// We are going to read the cache file in background.
				GlobalReaderCache::theCache().scheduleRead(entry.get(), 
														   "|", 
														   fCacheFileEntry->fCacheReaderProxy);
				entry->fReadState = CacheFileEntry::kReadingHierarchyInProgress;
			}
			else {
				// Display a wait cursor
				WaitCursor waitCursor;

				// Read the cache file now.
				// Make sure that we have a valid cache reader.
				GlobalReaderCache::CacheReaderHolder holder(fCacheFileEntry->fCacheReaderProxy);
				const boost::shared_ptr<CacheReader> cacheReader = holder.getCacheReader();

				if (cacheReader && cacheReader->valid()) {
					entry->fCachedGeometry = cacheReader->readScene(
						"|", !Config::isIgnoringUVs());
					entry->fCachedMaterial = cacheReader->readMaterials();
				}
			}
        
			// We get rid of the fCacheReaderProxy as soon as we start
			// drawing to free up memory. The fCacheReaderProxy was kept
			// opened just in case that another ShapeData node would have
			// been reading from the same cache file to save the reopening
			// of the file.
			//
			// This assumes that setInternalValueInContext() is called on
			// all ShapeNode on scene load before getCachedGeometry() is
			// called on any of them!
			fCacheFileEntry->fCacheReaderProxy.reset();
		}

		// Check if we are reading cache files in the background.
		if (entry->fReadState == CacheFileEntry::kReadingHierarchyInProgress) {
			MString validatedGeometryPath;
			if (GlobalReaderCache::theCache().pullHierarchy(entry.get(), entry->fCachedGeometry, validatedGeometryPath, entry->fCachedMaterial)) {
				// Background reading is done (hierarchy).
				entry->fReadState = CacheFileEntry::kReadingShapesInProgress;

				// Jump to shape done if we have no sub node hierarchy
				if (!entry->fCachedGeometry) {
					entry->fReadState = CacheFileEntry::kReadingDone;
				}

				// Dirty bounding box cache
				const_cast<ShapeNode*>(this)->childChanged(kBoundingBoxChanged);
			}
		}
		else if (entry->fReadState == CacheFileEntry::kReadingShapesInProgress) {
			if (GlobalReaderCache::theCache().pullShape(entry.get(), entry->fCachedGeometry)) {
				// Background reading is done (shapes).
				entry->fReadState = CacheFileEntry::kReadingDone;
			}
		}

		// Retrieve read state from the entry
		bool readingDone = (entry->fReadState == CacheFileEntry::kReadingDone);
		bool readingHierarchyDone = readingDone || (entry->fReadState == CacheFileEntry::kReadingShapesInProgress);

		if( readingHierarchyDone )
		{
			// Generate the SubNode hierarchy for this shape's geomPath
			MString validatedGeomPath;
			CreateSubNodeHierarchy(entry->fCachedGeometry, fCacheGeomPath, validatedGeomPath, fCachedGeometry);
			fCachedMaterial = entry->fCachedMaterial;

			// Update the geomPath with the validated path.
			updateGeomPath( validatedGeomPath );
		}

		if( readingDone )
		{
			fCacheReadingState = kCacheReadingDone;
		}
	}

    return fCachedGeometry;
}

void ShapeNode::updateGeomPath( const MString& validatedGeomPath ) const
{
	// Check the validated geometry path
    if (fCacheGeomPath != validatedGeomPath) {
        if (fCacheGeomPath.length() > 0) {
            // display a warning showing that the user's geometry path is wrong
            MStatus stat;
            MString msgFmt = MStringResource::getString(kFileNotFindWarningMsg, stat);
            MString warningMsg;
            warningMsg.format(msgFmt,
                fCacheGeomPath, fCacheFileName, validatedGeomPath);
            MGlobal::displayWarning(warningMsg);
        }

        fCacheGeomPath = validatedGeomPath;

        // Update the attribute editor. We shouldn't post too many
        // `autoUpdateAttrEd;` to the idle queue.
        MGlobal::executeCommand("if (!stringArrayContains(\"autoUpdateAttrEd;\",`evalDeferred -list`)) "
                                        "evalDeferred \"autoUpdateAttrEd;\";");
    }
}

const MaterialGraphMap::Ptr& ShapeNode::getCachedMaterial() const
{
    getCachedGeometry();  // side effect to load the cached geometry/material
    return fCachedMaterial;
}

const CacheFileEntry::MPtr& ShapeNode::getCacheFileEntry() const
{
	return fCacheFileEntry;
}

const CacheFileEntry::BackgroundReadingState ShapeNode::backgroundReadingState() const
{
	if( fCacheFileEntry )
	{
		return fCacheFileEntry->fReadState;
	}
	return CacheFileEntry::kReadingDone;
}

MStringArray ShapeNode::getFilesToArchive(
    bool shortName, bool unresolvedName, bool markCouldBeImageSequence ) const
{
    MStringArray files;

    if(unresolvedName)
    {
        files.append(fCacheFileName);
    }
    else
    {
        //unresolvedName is false, resolve the path via MFileObject.
        MFileObject fileObject;
        fileObject.setRawFullName(fCacheFileName);
        files.append(fileObject.resolvedFullName());
    }    
    
    return files;
}

void ShapeNode::copyInternalData(MPxNode* source)
{
    if (source && source->typeId() == id) {
        ShapeNode* node = dynamic_cast<ShapeNode*>(source);
        fCacheFileName = node->fCacheFileName;
        fCacheGeomPath = node->fCacheGeomPath;

        // WARNING: This assumes that the geometry is read-only once
        // read.
        fCachedGeometry   = node->fCachedGeometry;
        fCachedMaterial   = node->fCachedMaterial;
		fCacheFileEntry   = node->fCacheFileEntry;
        
        // Setup this shape to read the contents of the entry in the
		// getCachedGeometry() call.
		fCacheReadingState = kCacheReadingFile;
    }
}

bool ShapeNode::match( const MSelectionMask & mask,
                   const MObjectArray& componentList ) const
{
    MSelectionMask gpuCacheMask(ShapeNode::selectionMaskName);
    return mask.intersects(gpuCacheMask) && componentList.length()==0;
}

MSelectionMask ShapeNode::getShapeSelectionMask() const
{
    return MSelectionMask(ShapeNode::selectionMaskName);
}

bool ShapeNode::excludeAsPluginShape() const
{
    // gpuCache node has its own display filter "GPU Cache" in Show menu.
    // We don't want "Plugin Shapes" to filter out gpuCache nodes.
    return false;
}

void ShapeNode::addedToModelCB()
{
	// Update the shape registry with this item.
	CacheShapeRegistry::theCache().insert(fResolvedCacheFileName, thisMObject());

	// This shape has been added to the scene. Refresh the cached geometry
	// to ensure that our cache file entry is valid. This is particularly
	// important in the case of (Undo: delete).
	refreshCachedGeometry(false);
}

void ShapeNode::removedFromModelCB()
{
	// Update the shape registry with this item.
	CacheShapeRegistry::theCache().remove(fResolvedCacheFileName, thisMObject());

	// This shape has been removed from the scene. Clear the cache file entry
	// and notify the registry for clean up.
	fCacheFileEntry.reset();
	CacheFileRegistry::theCache().cleanUp(fResolvedCacheFileName);
}

void ShapeNode::dirtyVP2Geometry( const MString& fileName )
{
	// Dirty VP2 geometry
	// We don't need to call setGeometryDrawDirty() for MPxSubSceneOverride API.
	if( Config::vp2OverrideAPI() == Config::kMPxDrawOverride )
	{
		std::vector<MObjectHandle> shapes;
		CacheShapeRegistry::theCache().find( fileName, shapes );
		size_t nShapes = shapes.size();
		for( size_t i = 0; i < nShapes; i++ )
		{
			if( !shapes[i].isValid() )
			{
				continue;
			}
			MObject shape = shapes[i].object();
			MHWRender::MRenderer::setGeometryDrawDirty( shape, true );
		}
	}
}


//==============================================================================
// CLASS CacheShapeRegistry
//==============================================================================

CacheShapeRegistry CacheShapeRegistry::fsSingleton;
CacheShapeRegistry::Map CacheShapeRegistry::fMap;

CacheShapeRegistry::CacheShapeRegistry()
{}

CacheShapeRegistry::~CacheShapeRegistry()
{}

CacheShapeRegistry& CacheShapeRegistry::theCache()
{
	return fsSingleton;
}

void CacheShapeRegistry::getAll( std::vector<MObjectHandle>& shapes )
{
	shapes.clear();
	for( Map::iterator it = fMap.begin(); it != fMap.end(); it++ )
	{
		shapes.push_back( it->second );
	}
}

void CacheShapeRegistry::find( const MString& key, std::vector<MObjectHandle>& shapes )
{
	shapes.clear();

	std::pair<Map::iterator, Map::iterator> its = fMap.equal_range(key);
	for( Map::iterator it = its.first; it != its.second; it++ )
	{
		shapes.push_back(it->second);
	}
}

bool CacheShapeRegistry::insert( const MString& key, const MObjectHandle& shape )
{
	Map::value_type item( key, shape );
	fMap.insert( item );
	return true;
}

bool CacheShapeRegistry::remove( const MString& key, const MObjectHandle& shape )
{
	std::pair<Map::iterator, Map::iterator> its = fMap.equal_range(key);
	for( Map::iterator it = its.first; it != its.second; it++ )
	{
		if( it->first == key &&
			it->second == shape )
		{
			fMap.erase(it);
			return true;
		}
	}
	return false;
}

void CacheShapeRegistry::clear()
{
	fMap.clear();
}


//==============================================================================
// CLASS DisplayPref
//==============================================================================

DisplayPref::WireframeOnShadedMode DisplayPref::fsWireframeOnShadedMode;
MCallbackId DisplayPref::fsDisplayPrefChangedCallbackId;

MStatus DisplayPref::initCallback()
{
    MStatus stat;
    
    // Register DisplayPreferenceChanged callback
    fsDisplayPrefChangedCallbackId = MEventMessage::addEventCallback(
        "DisplayPreferenceChanged", DisplayPref::displayPrefChanged, NULL, &stat);
    MCHECKERROR(stat, "MEventMessage::addEventCallback(DisplayPreferenceChanged");

    // Trigger the callback manually to init class members
    displayPrefChanged(NULL);

    return MS::kSuccess;
}

MStatus DisplayPref::removeCallback()
{
    MStatus stat;

    // Remove DisplayPreferenceChanged callback
    MEventMessage::removeCallback(fsDisplayPrefChangedCallbackId);
    MCHECKERROR(stat, "MEventMessage::removeCallback(DisplayPreferenceChanged)");

    return MS::kSuccess;
}

void DisplayPref::displayPrefChanged(void*)
{
    MStatus stat;
    // Wireframe on shaded mode: Full/Reduced/None
    MString wireframeOnShadedActive = MGlobal::executeCommandStringResult(
        "displayPref -q -wireframeOnShadedActive", false, false, &stat);
    if (stat) {
        if (wireframeOnShadedActive == "full") {
            fsWireframeOnShadedMode = kWireframeOnShadedFull;
        }
        else if (wireframeOnShadedActive == "reduced") {
            fsWireframeOnShadedMode = kWireframeOnShadedReduced;
        }
        else if (wireframeOnShadedActive == "none") {
            fsWireframeOnShadedMode = kWireframeOnShadedNone;
        }
        else {
            assert(0);
        }
    }
}

DisplayPref::WireframeOnShadedMode DisplayPref::wireframeOnShadedMode()
{
    return fsWireframeOnShadedMode;
}


//==============================================================================
// CLASS ShapeUI
//==============================================================================

void* ShapeUI::creator()
{
    return new ShapeUI;
}

ShapeUI::ShapeUI()
{}

ShapeUI::~ShapeUI()
{}

void ShapeUI::getDrawRequests(const MDrawInfo & info,
                              bool objectAndActiveOnly,
                              MDrawRequestQueue & queue)
{
    // Make sure that the post render callbacks have been properly
    // initialized. We have to verify at each refresh because there is
    // no easy way to recieve a callback when a new modelEditor is
    // created.
    ShapeNode::init3dViewPostRenderCallbacks();
    
    // Get the data necessary to draw the shape
    MDrawData data;
    getDrawData( 0, data );

    // Decode the draw info and determine what needs to be drawn
    M3dView::DisplayStyle  appearance    = info.displayStyle();
    M3dView::DisplayStatus displayStatus = info.displayStatus();

    // Are we displaying gpuCache?
    if (!info.pluginObjectDisplayStatus(Config::kDisplayFilter)) {
        return;
    }

    MDagPath path = info.multiPath();

    switch ( appearance )
    {
        case M3dView::kBoundingBox :
        {
            MDrawRequest request = info.getPrototype( *this );
            request.setDrawData( data );
            request.setToken( kBoundingBox );

            MColor wireframeColor = MHWRender::MGeometryUtilities::wireframeColor(path);
            request.setColor(wireframeColor);

            queue.add( request );
        }break;
        
        case M3dView::kWireFrame :
        {
            MDrawRequest request = info.getPrototype( *this );
            request.setDrawData( data );
            request.setToken( kDrawWireframe );

            MColor wireframeColor = MHWRender::MGeometryUtilities::wireframeColor(path);
            request.setColor(wireframeColor);

            queue.add( request );
        } break;
    

        // All of these modes are interpreted as meaning smooth shaded
        // just as it is done in the viewport 2.0.
        case M3dView::kFlatShaded :
        case M3dView::kGouraudShaded :
        default:    
        {
            ShapeNode* node = (ShapeNode*)surfaceShape();
            if (!node) break;
            const SubNode::Ptr geom = node->getCachedGeometry();
            if (!geom) break;

            // Get the view to draw to
            M3dView view = info.view();

            const bool needWireframe = ((displayStatus == M3dView::kActive) ||
                                        (displayStatus == M3dView::kLead)   ||
                                        (displayStatus == M3dView::kHilite) ||
                                        (view.wireframeOnShaded()));

            // When we need to draw both the shaded geometry and the
            // wireframe mesh, we need to offset the shaded geometry
            // in depth to avoid Z-fighting against the wireframe
            // mesh.
            //
            // On the hand, we don't want to use depth offset when
            // drawing only the shaded geometry because it leads to
            // some drawing artifacts. The reason is a litle bit
            // subtle. At silouhette edges, both front-facing and
            // back-facing faces are meeting. These faces can have a
            // different slope in Z and this can lead to a different
            // Z-offset being applied. When unlucky, the back-facing
            // face can be drawn in front of the front-facing face. If
            // two-sided lighting is enabled, the back-facing fragment
            // can have a different resultant color. This can lead to
            // a rim of either dark or bright pixels around silouhette
            // edges.
            //
            // When the wireframe mesh is drawn on top (even a dotted
            // one), it masks this effect sufficiently that it is no
            // longer distracting for the user, so it is OK to use
            // depth offset when the wireframe mesh is drawn on top.
            const DrawToken shadedDrawToken = needWireframe ?
                kDrawSmoothShadedDepthOffset : kDrawSmoothShaded;
            
            // Get the default material.
            //
            // Note that we will only use the material if the viewport
            // option "Use default material" has been selected. But,
            // we still need to set a material (even an unevaluated
            // one), so that the draw request is indentified as
            // drawing geometry instead of drawing the wireframe mesh.
            MMaterial material = MMaterial::defaultMaterial();

            if (view.usingDefaultMaterial()) {
                // Evaluate the material.
                if ( !material.evaluateMaterial(view, path) ) {
                    MStatus stat;
                    MString msg = MStringResource::getString(kEvaluateMaterialErrorMsg, stat);
                    perror(msg.asChar());
                }

                // Create the smooth shaded draw request
                MDrawRequest request = info.getPrototype( *this );
                request.setDrawData( data );
                
                // This draw request will draw all sub nodes using an
                // opaque default material.
                request.setToken( shadedDrawToken );
                request.setIsTransparent( false );

                request.setMaterial( material );
                queue.add( request );
            }
            else if (view.xray()) {
                // Create the smooth shaded draw request
                MDrawRequest request = info.getPrototype( *this );
                request.setDrawData( data );

                // This draw request will draw all sub nodes using in X-Ray mode
                request.setToken( shadedDrawToken );
                request.setIsTransparent( true );

                request.setMaterial( material );
                queue.add( request );
            }
            else {
                // Opaque draw request
                if (geom->transparentType() != SubNode::kTransparent) {
                    // Create the smooth shaded draw request
                    MDrawRequest request = info.getPrototype( *this );
                    request.setDrawData( data );

                    // This draw request will draw opaque sub nodes
                    request.setToken( shadedDrawToken );

                    request.setMaterial( material );
                    queue.add( request );
                }

                // Transparent draw request
                if (geom->transparentType() != SubNode::kOpaque) {
                    // Create the smooth shaded draw request
                    MDrawRequest request = info.getPrototype( *this );
                    request.setDrawData( data );

                    // This draw request will draw transparent sub nodes
                    request.setToken( shadedDrawToken );
                    request.setIsTransparent( true );

                    request.setMaterial( material );
                    queue.add( request );
                }
            }

            // create a draw request for wireframe on shaded if
            // necessary.
            if (needWireframe && 
                DisplayPref::wireframeOnShadedMode() != DisplayPref::kWireframeOnShadedNone)
            {
                MDrawRequest wireRequest = info.getPrototype( *this );
                wireRequest.setDrawData( data );
                wireRequest.setToken( kDrawWireframeOnShaded );
                wireRequest.setDisplayStyle( M3dView::kWireFrame );

                MColor wireframeColor = MHWRender::MGeometryUtilities::wireframeColor(path);
                wireRequest.setColor(wireframeColor);

                queue.add( wireRequest );
            }
        } break;
    }
}

void ShapeUI::draw(const MDrawRequest & request, M3dView & view) const
{
    // Initialize GL Function Table.
    InitializeGLFT();

    // Get the token from the draw request.
    // The token specifies what needs to be drawn.
    DrawToken token = DrawToken(request.token());

    switch( token )
    {
        case kBoundingBox :
            drawBoundingBox( request, view );
            break;

        case kDrawWireframe :
        case kDrawWireframeOnShaded :
            drawWireframe( request, view );
            break;

        case kDrawSmoothShaded :
            drawShaded( request, view, false );
            break;

        case kDrawSmoothShadedDepthOffset :
            drawShaded( request, view, true );
            break;
    }
}

void ShapeUI::drawBoundingBox(const MDrawRequest & request, M3dView & view) const
{
    // Get the surface shape
    ShapeNode* node = (ShapeNode*)surfaceShape();
    if (!node) return;

    // Get the bounding box    
    MBoundingBox box = node->boundingBox();

    view.beginGL(); 
    {
        // Query current state so it can be restored
        //
        bool lightingWasOn = gGLFT->glIsEnabled( MGL_LIGHTING ) == MGL_TRUE;

        // Setup the OpenGL state as necessary
        //
        if ( lightingWasOn ) {
            gGLFT->glDisable( MGL_LIGHTING );
        }

        gGLFT->glEnable( MGL_LINE_STIPPLE );
        gGLFT->glLineStipple(1,  Config::kLineStippleShortDashed);

        VBOProxy vboProxy;
        vboProxy.drawBoundingBox(box);

        // Restore the state
        //
        if ( lightingWasOn ) {
            gGLFT->glEnable( MGL_LIGHTING );
        }

        gGLFT->glDisable( MGL_LINE_STIPPLE );
    }
    view.endGL();    
}

void ShapeUI::drawWireframe(const MDrawRequest & request, M3dView & view) const
{
    // Get the surface shape
    ShapeNode* node = (ShapeNode*)surfaceShape();
    if ( !node ) return;

    // Extract the cached geometry.
    const SubNode::Ptr rootNode = node->getCachedGeometry();
    if (!rootNode) return;

    const double seconds = MAnimControl::currentTime().as(MTime::kSeconds);

    MMatrix projMatrix;
    view.projectionMatrix(projMatrix);
    MMatrix modelViewMatrix;
    view.modelViewMatrix(modelViewMatrix);

    MMatrix localToPort = modelViewMatrix * projMatrix;

    view.beginGL(); 
    {
        // Query current state so it can be restored
        //
        bool lightingWasOn = gGLFT->glIsEnabled( MGL_LIGHTING ) == MGL_TRUE;

        // Setup the OpenGL state as necessary
        //
        if ( lightingWasOn ) {
            gGLFT->glDisable( MGL_LIGHTING );
        }

        gGLFT->glEnable( MGL_LINE_STIPPLE );
        if (request.token() == kDrawWireframeOnShaded) {
            // Wireframe on shaded is affected by wireframe on shaded mode
            DisplayPref::WireframeOnShadedMode wireframeOnShadedMode = 
                DisplayPref::wireframeOnShadedMode();
            if (wireframeOnShadedMode == DisplayPref::kWireframeOnShadedReduced) {
                gGLFT->glLineStipple(1, Config::kLineStippleDotted);
            }
            else {
                assert(wireframeOnShadedMode != DisplayPref::kWireframeOnShadedNone);
                gGLFT->glLineStipple(1, Config::kLineStippleShortDashed);
            }
        }
        else {
            gGLFT->glLineStipple(1, Config::kLineStippleShortDashed);
        }

        // Draw the wireframe mesh
        //
        {
            Frustum frustum(localToPort.inverse());
            MMatrix xform(modelViewMatrix);
        
            DrawWireframeState state(frustum, seconds);
            DrawWireframeTraversal traveral(state, xform, false, Frustum::kUnknown);
            rootNode->accept(traveral);
        }

        // Restore the state
        //
        if ( lightingWasOn ) {
            gGLFT->glEnable( MGL_LIGHTING );
        }

        gGLFT->glDisable( MGL_LINE_STIPPLE );
    }
    view.endGL(); 
}


void ShapeUI::drawShaded(
    const MDrawRequest & request, M3dView & view, bool depthOffset) const
{
    // Get the surface shape
    ShapeNode* node = (ShapeNode*)surfaceShape();
    if ( !node ) return;

    // Extract the cached geometry.
    const SubNode::Ptr rootNode = node->getCachedGeometry();
    if (!rootNode) return;

    const double seconds = MAnimControl::currentTime().as(MTime::kSeconds);

    MMatrix projMatrix;
    view.projectionMatrix(projMatrix);
    MMatrix modelViewMatrix;
    view.modelViewMatrix(modelViewMatrix);

    MMatrix localToNDC = modelViewMatrix * projMatrix;

    M3dView::LightingMode lightingMode;
    view.getLightingMode(lightingMode);
    unsigned int lightCount;
    view.getLightCount(lightCount);

    const bool noLightSoDrawAsBlack =
        (lightingMode == M3dView::kLightAll ||
         lightingMode == M3dView::kLightSelected ||
         lightingMode == M3dView::kLightActive)
        && lightCount == 0;
    
    view.beginGL(); 
    {
        // Setup the OpenGL state as necessary
        //
        // The most straightforward way to ensure that the OpenGL
        // material parameters are properly restored after drawing is
        // to use push/pop attrib as we have no easy of knowing the
        // current values of all the parameters.
        gGLFT->glPushAttrib(MGL_LIGHTING_BIT);

        // Reset specular and emission materials as we only display diffuse color.
        {
            static const float sBlack[4] = {0.0f, 0.0f, 0.0f, 1.0f};
            gGLFT->glMaterialfv(MGL_FRONT_AND_BACK, MGL_SPECULAR, sBlack);
            gGLFT->glMaterialfv(MGL_FRONT_AND_BACK, MGL_EMISSION, sBlack);
        }
        
        DrawShadedState::TransparentPruneType transparentPrune =
                DrawShadedState::kPruneTransparent;

        const bool isTransparent = request.isTransparent();
        if (isTransparent) {
            // We use premultiplied alpha
            gGLFT->glBlendFunc(MGL_ONE, MGL_ONE_MINUS_SRC_ALPHA);
            transparentPrune = DrawShadedState::kPruneOpaque;

            gGLFT->glDepthMask( false );
        }

        MColor defaultDiffuseColor;
        DrawShadedTypes::ColorType colorType = DrawShadedTypes::kSubNodeColor;
        if (view.usingDefaultMaterial()) {
            if (!noLightSoDrawAsBlack) {
                MMaterial material = request.material();
                material.setMaterial(request.multiPath(), isTransparent);
                material.getDiffuse(defaultDiffuseColor);
            }

            // We must ignore the alpha channel of the default
            // material when the option "Use default material" is
            // selected.
            defaultDiffuseColor[3] = 1.0f;
            transparentPrune = DrawShadedState::kPruneNone;
            colorType = DrawShadedTypes::kDefaultColor;
        }
        else if (view.xray()) {
            transparentPrune = DrawShadedState::kPruneNone;

            if (noLightSoDrawAsBlack) {
                defaultDiffuseColor = MColor(0, 0, 0, 0.3f);
                colorType           = DrawShadedTypes::kDefaultColor;
            }
            else {
                colorType = DrawShadedTypes::kXrayColor;
            }
        }
        else if (noLightSoDrawAsBlack) {
            colorType = DrawShadedTypes::kBlackColor;
        }

        if (noLightSoDrawAsBlack) {
            // The default viewport leaves an unrelated light enabled
            // in the OpenGL state even when there are no light in the
            // scene. We therefore manually disable lighting in that
            // case.
            gGLFT->glDisable(MGL_LIGHTING);
        }
        
        const bool depthOffsetWasEnabled = gGLFT->glIsEnabled(MGL_POLYGON_OFFSET_FILL);
        if (depthOffset && !depthOffsetWasEnabled) {
            // Viewport has set the offset, just enable it
            gGLFT->glEnable(MGL_POLYGON_OFFSET_FILL);  
        }

        // We will override the material color for each individual sub-nodes.!
        gGLFT->glColorMaterial(MGL_FRONT_AND_BACK, MGL_AMBIENT_AND_DIFFUSE);
        gGLFT->glEnable(MGL_COLOR_MATERIAL) ;

        // On Geforce cards, we emulate two-sided lighting by drawing
        // triangles twice because two-sided lighting is 10 times
        // slower than single-sided lighting.
        bool needEmulateTwoSidedLighting = false;
        if (Config::emulateTwoSidedLighting()) {
            // Query face-culling and two-sided lighting state
            bool  cullFace = (gGLFT->glIsEnabled(MGL_CULL_FACE) == MGL_TRUE);
            MGLint twoSidedLighting = MGL_FALSE;
            gGLFT->glGetIntegerv(MGL_LIGHT_MODEL_TWO_SIDE, &twoSidedLighting);

            // Need to emulate two-sided lighting when back-face
            // culling is off (i.e. drawing both sides) and two-sided
            // lLighting is on.
            needEmulateTwoSidedLighting = (!cullFace && twoSidedLighting);
        }

        {
            Frustum frustum(localToNDC.inverse());
            MMatrix xform(modelViewMatrix);

            if (needEmulateTwoSidedLighting) {
                gGLFT->glEnable(MGL_CULL_FACE);
                gGLFT->glLightModeli(MGL_LIGHT_MODEL_TWO_SIDE, 0);

                // first, draw with back-face culling
                {
                    gGLFT->glCullFace(MGL_FRONT);
                    DrawShadedState state(frustum, 
                                          seconds, 
                                          transparentPrune,
                                          colorType,
                                          defaultDiffuseColor,
                                          DrawShadedState::kBackNormals);
                    DrawShadedTraversal traveral(
                        state, xform, xform.det3x3() < 0.0, Frustum::kUnknown);
                    rootNode->accept(traveral);
                }
                
                // then, draw with front-face culling
                {
                    gGLFT->glCullFace(MGL_BACK);
                    DrawShadedState state(frustum, 
                                          seconds, 
                                          transparentPrune,
                                          colorType,
                                          defaultDiffuseColor,
                                          DrawShadedState::kFrontNormals);
                    DrawShadedTraversal traveral(
                        state, xform, xform.det3x3() < 0.0, Frustum::kUnknown);
                    rootNode->accept(traveral);
                }
                
                // restore the OpenGL state
                gGLFT->glDisable(MGL_CULL_FACE);
                gGLFT->glLightModeli(MGL_LIGHT_MODEL_TWO_SIDE, 1);
            }
            else {
                DrawShadedState state(frustum, 
                                      seconds, 
                                      transparentPrune,
                                      colorType,
                                      defaultDiffuseColor,
                                      DrawShadedState::kFrontNormals);
                DrawShadedTraversal traveral(
                    state, xform, xform.det3x3() < 0.0, Frustum::kUnknown);
                rootNode->accept(traveral);
            }
        }

        // Restore the state
        //
        if (isTransparent) {
            gGLFT->glDepthMask( true );

            gGLFT->glBlendFunc(MGL_SRC_ALPHA, MGL_ONE_MINUS_SRC_ALPHA);
        }

        if (depthOffset && !depthOffsetWasEnabled) {
            gGLFT->glDisable(MGL_POLYGON_OFFSET_FILL);
        }
        
        gGLFT->glFrontFace(MGL_CCW);

        gGLFT->glPopAttrib();
    }
    view.endGL(); 
}

//
// Returns the point in world space corresponding to a given
// depth. The depth is specified as 0.0 for the near clipping plane and
// 1.0 for the far clipping plane.
//
MPoint ShapeUI::getPointAtDepth(
    MSelectInfo &selectInfo,
    double     depth)
{
    MDagPath cameraPath;
    M3dView view = selectInfo.view();

    view.getCamera(cameraPath);
    MStatus status;
    MFnCamera camera(cameraPath, &status);

    // Ortho cam maps [0,1] to [near,far] linearly
    // persp cam has non linear z:
    //
    //        fp np
    // -------------------
    // 1. fp - d fp + d np
    //
    // Maps [0,1] -> [np,fp]. Then using linear mapping to get back to
    // [0,1] gives.
    //
    //       d np
    // ----------------  for linear mapped distance
    // fp - d fp + d np

    if (!camera.isOrtho())
    {
        double np = camera.nearClippingPlane();
        double fp = camera.farClippingPlane();

        depth *= np / (fp - depth * (fp - np));
    }

    MPoint     cursor;
    MVector rayVector;
    selectInfo.getLocalRay(cursor, rayVector);
    cursor = cursor * selectInfo.multiPath().inclusiveMatrix();
    short x,y;
    view.worldToView(cursor, x, y);

    MPoint res, neardb, fardb;
    view.viewToWorld(x,y, neardb, fardb);
    res = neardb + depth*(fardb-neardb);

    return res;
}


bool ShapeUI::select(
    MSelectInfo &selectInfo,
    MSelectionList &selectionList,
    MPointArray &worldSpaceSelectPts ) const
{
    // Initialize GL Function Table.
    InitializeGLFT();

    MSelectionMask mask(ShapeNode::selectionMaskName);
    if (!selectInfo.selectable(mask)){
        return false;
    }

    // Check plugin display filter. Invisible geometry can't be selected
    if (!selectInfo.pluginObjectDisplayStatus(Config::kDisplayFilter)) {
        return false;
    }

    // Get the geometry information
    //
    ShapeNode* node = (ShapeNode*)surfaceShape();
    const SubNode::Ptr rootNode = node->getCachedGeometry();
    if (!rootNode) { return false;}

    const double seconds = MAnimControl::currentTime().as(MTime::kSeconds);

    const bool boundingboxSelection =
        (M3dView::kBoundingBox == selectInfo.displayStyle());

    const bool wireframeSelection =
        (M3dView::kWireFrame == selectInfo.displayStyle() ||
         !selectInfo.singleSelection());

    // If all the model editors are Viewport2.0, we will not use VBO for select
    // because VBO will double the memory consumption.
    VBOProxy::VBOMode vboMode = VBOProxy::kUseVBOIfPossible;
    if (Config::vp2OverrideAPI() != Config::kMPxDrawOverride) {
        vboMode = (sModelEditorState == kViewport2Only) ?
            VBOProxy::kDontUseVBO : VBOProxy::kUseVBOIfPossible;
    }
    
    // We select base on edges if the object is displayed in wireframe
    // mode or if we are performing a marquee selection. Else, we
    // select using the object faces (i.e. single-click selection in
    // shaded mode).
    GLfloat minZ;
    {
        Select* selector;
        NbPrimitivesVisitor nbPrimitives(seconds);
        rootNode->accept(nbPrimitives);
        
        if (boundingboxSelection) {
            // We are only drawing 12 edges so we only use GL picking selection.
            selector = new GLPickingSelect(selectInfo);

            selector->processBoundingBox(rootNode, seconds);
        }
        else if (wireframeSelection) {
            if (nbPrimitives.numWires() < Config::openGLPickingWireframeThreshold()) 
                selector = new GLPickingSelect(selectInfo);
            else
                selector = new RasterSelect(selectInfo);
        
            selector->processEdges(rootNode, seconds, nbPrimitives.numWires(), vboMode);
        }
        else {
            if (nbPrimitives.numTriangles() < Config::openGLPickingSurfaceThreshold())
                selector = new GLPickingSelect(selectInfo);
            else
                selector = new RasterSelect(selectInfo);

            selector->processTriangles(rootNode, seconds, nbPrimitives.numTriangles(), vboMode);
        }
        selector->end();
        minZ = selector->minZ();
        delete selector;
    }

    bool selected = (minZ <= 1.0f);
    if ( selected ) {
        // Add the selected item to the selection list

        MSelectionList selectionItem;
        {
            MDagPath path = selectInfo.multiPath();
            MStatus lStatus = path.pop();
            while (lStatus == MStatus::kSuccess)
            {
                if (path.hasFn(MFn::kTransform))
                {
                    break;
                }
                else
                {
                    lStatus = path.pop();
                }
            }
            selectionItem.add(path);
        }        

        MPoint worldSpaceselectionPoint =
            getPointAtDepth(selectInfo, minZ);

        selectInfo.addSelection(
            selectionItem,
            worldSpaceselectionPoint,
            selectionList, worldSpaceSelectPts,
            mask, false );
    }

    return selected;
}

bool ShapeUI::snap(MSelectInfo& snapInfo) const
{
    // Initialize GL Function Table.
    InitializeGLFT();

    // Check plugin display filter. Invisible geometry can't be snapped
    if (!snapInfo.pluginObjectDisplayStatus(Config::kDisplayFilter)) {
        return false;
    }

    // Get the geometry information
    //
    ShapeNode* node = (ShapeNode*)surfaceShape();
    const SubNode::Ptr rootNode = node->getCachedGeometry();
    if (!rootNode) return false;

    const double seconds = MAnimControl::currentTime().as(MTime::kSeconds);

    M3dView view = snapInfo.view();

    const MDagPath & path = snapInfo.multiPath();
    const MMatrix inclusiveMatrix = path.inclusiveMatrix();

    MMatrix projMatrix;
    view.projectionMatrix(projMatrix);
    MMatrix modelViewMatrix;
    view.modelViewMatrix(modelViewMatrix);

    unsigned int vpx, vpy, vpw, vph;
    view.viewport(vpx, vpy, vpw, vph);
    double w_over_two = vpw * 0.5;
    double h_over_two = vph * 0.5;
    double vpoff_x = w_over_two + vpx;
    double vpoff_y = h_over_two + vpy;
    MMatrix ndcToPort;
    ndcToPort(0,0) = w_over_two;
    ndcToPort(1,1) = h_over_two;
    ndcToPort(2,2) = 0.5;
    ndcToPort(3,0) = vpoff_x;
    ndcToPort(3,1) = vpoff_y;
    ndcToPort(3,2) = 0.5;
    
    const MMatrix localToNDC  = modelViewMatrix * projMatrix;
    const MMatrix localToPort = localToNDC * ndcToPort;

    Frustum frustum(localToNDC.inverse());
    
    SnapTraversalState state(
        frustum, seconds, localToPort, inclusiveMatrix, snapInfo);
    SnapTraversal visitor(state, MMatrix::identity, false, Frustum::kUnknown);
    rootNode->accept(visitor);
    return state.selected();
}

void ShapeNode::getExternalContent(MExternalContentInfoTable& table) const
{
    addExternalContentForFileAttr(table, aCacheFileName);
    MPxSurfaceShape::getExternalContent(table);
}

void ShapeNode::setExternalContent(const MExternalContentLocationTable& table)
{
    setExternalContentForFileAttr(aCacheFileName, table);
    MPxSurfaceShape::setExternalContent(table);
}

}
