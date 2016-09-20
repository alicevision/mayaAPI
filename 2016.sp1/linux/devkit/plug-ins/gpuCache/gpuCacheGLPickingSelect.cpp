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

#include "gpuCacheGLPickingSelect.h"

#include "gpuCacheSample.h"
#include "gpuCacheVBOProxy.h"
#include "gpuCacheDrawTraversal.h"
#include "gpuCacheGLFT.h"
#include "gpuCacheUtil.h"
#include "CacheReader.h"

#include <algorithm>


namespace {

using namespace GPUCache;

//==============================================================================
// LOCAL FUNCTIONS
//==============================================================================

    const unsigned int  MAX_HW_DEPTH_VALUE = 0xffffffff;

    //
    // Returns the minimal depth of the pick buffer.
    //
    unsigned int closestElem(unsigned int nbPick, const GLuint* buffPtr)
    {
        if (NULL == buffPtr) {
            return 0;
        }

        unsigned int Zdepth = MAX_HW_DEPTH_VALUE;
        for (int index = nbPick ; index > 0 ;--index)
        {
            if (buffPtr[0] &&               // Non void item
                (Zdepth > buffPtr[1]))      // Closer to camera
            {
                Zdepth = buffPtr[1];        // Zmin distance
            }
            buffPtr += buffPtr[0] + 3;      // Next item
        }
        return Zdepth;
    }

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
            const Frustum&    frustrum,
            const double      seconds,
            VBOProxy::VBOMode vboMode)
            : DrawTraversalState(frustrum, seconds, kPruneNone),
              fVBOMode(vboMode)
        {}

        VBOProxy::VBOMode vboMode() const
        { return fVBOMode; }

    private:
        VBOProxy::VBOMode fVBOMode;
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

            state().vboProxy().drawWireframe(
                sample,
                Config::useVertexArrayForGLPicking() ?
                VBOProxy::kDontUseVBO : state().vboMode() );
        }
    };


    //==========================================================================
    // CLASS DrawShadedTraversal
    //==========================================================================

    class DrawShadedState : public DrawTraversalState
    {
    public:
        DrawShadedState(
            const Frustum&    frustrum,
            const double      seconds,
            VBOProxy::VBOMode vboMode)
            : DrawTraversalState(frustrum, seconds, kPruneNone),
              fVBOMode(vboMode)
        {}

        VBOProxy::VBOMode vboMode() const
        { return fVBOMode; }

    private:
        VBOProxy::VBOMode fVBOMode;
    };
        
    class DrawShadedTraversal
        : public DrawTraversal<DrawShadedTraversal, DrawShadedState>
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

            const size_t numGroups = sample->numIndexGroups();
            for (size_t groupId = 0; groupId < numGroups; groupId++) {
                state().vboProxy().drawTriangles(
                    sample, groupId,
                    VBOProxy::kNoNormals, VBOProxy::kNoUVs,
                    Config::useVertexArrayForGLPicking() ?
                    VBOProxy::kDontUseVBO : state().vboMode());
            }
        }
    };
}


namespace GPUCache {

//==============================================================================
// CLASS GLPickingSelect
//==============================================================================

//------------------------------------------------------------------------------
//
GLPickingSelect::GLPickingSelect(
    MSelectInfo& selectInfo
) 
    : fSelectInfo(selectInfo),
      fMinZ(std::numeric_limits<float>::max())
{}


//------------------------------------------------------------------------------
//
GLPickingSelect::~GLPickingSelect() 
{}


//------------------------------------------------------------------------------
//
void GLPickingSelect::processEdges(
    const SubNode::Ptr rootNode,
    const double seconds,
    const size_t numWires,
    VBOProxy::VBOMode vboMode
)
{
    const unsigned int bufferSize = (unsigned int)std::min(numWires,size_t(100000));
    boost::shared_array<GLuint> buffer(new GLuint[bufferSize*4]);

    M3dView view = fSelectInfo.view();

    MMatrix projMatrix;
    view.projectionMatrix(projMatrix);
    MMatrix modelViewMatrix;
    view.modelViewMatrix(modelViewMatrix);

    unsigned int x, y, w, h;
    view.viewport(x, y, w, h);
    double viewportX = static_cast<int>(x);   // can be less than 0
    double viewportY = static_cast<int>(y);   // can be less than 0
    double viewportW = w;
    double viewportH = h;

    fSelectInfo.selectRect(x, y, w, h);
    double selectX = static_cast<int>(x);  // can be less than 0
    double selectY = static_cast<int>(y);  // can be less than 0
    double selectW = w;
    double selectH = h;

    MMatrix selectAdjustMatrix;
    selectAdjustMatrix[0][0] = viewportW / selectW;
    selectAdjustMatrix[1][1] = viewportH / selectH;
    selectAdjustMatrix[3][0] = ((viewportX + viewportW/2.0) - (selectX + selectW/2.0)) / 
        viewportW * 2.0 * selectAdjustMatrix[0][0];
    selectAdjustMatrix[3][1] = ((viewportY + viewportH/2.0) - (selectY + selectH/2.0)) /
        viewportH * 2.0 * selectAdjustMatrix[1][1];

    MMatrix localToPort = modelViewMatrix * projMatrix * selectAdjustMatrix;

    view.beginSelect(buffer.get(), bufferSize*4);
    view.pushName(0);
    {
        Frustum frustum(localToPort.inverse());
        MMatrix xform(modelViewMatrix);
        
        DrawWireframeState state(frustum, seconds, vboMode);
        DrawWireframeTraversal traveral(state, xform, false, Frustum::kUnknown);
        rootNode->accept(traveral);
    }
    view.popName();
    int nbPick = view.endSelect();

    if (nbPick > 0) {
        unsigned int Zdepth = closestElem(nbPick, buffer.get());    
        float depth = float(Zdepth)/MAX_HW_DEPTH_VALUE;
        fMinZ = std::min(depth,fMinZ);
    }
}

//------------------------------------------------------------------------------
//
void GLPickingSelect::processTriangles(
    const SubNode::Ptr rootNode,
    const double seconds,
    const size_t numTriangles,
    VBOProxy::VBOMode vboMode
)
{
    const unsigned int bufferSize = (unsigned int)std::min(numTriangles,size_t(100000));
    boost::shared_array<GLuint>buffer (new GLuint[bufferSize*4]);

    M3dView view = fSelectInfo.view();

    MMatrix projMatrix;
    view.projectionMatrix(projMatrix);
    MMatrix modelViewMatrix;
    view.modelViewMatrix(modelViewMatrix);

    unsigned int x, y, w, h;
    view.viewport(x, y, w, h);
    double viewportX = static_cast<int>(x);   // can be less than 0
    double viewportY = static_cast<int>(y);   // can be less than 0
    double viewportW = w;
    double viewportH = h;

    fSelectInfo.selectRect(x, y, w, h);
    double selectX = static_cast<int>(x);  // can be less than 0
    double selectY = static_cast<int>(y);  // can be less than 0
    double selectW = w;
    double selectH = h;

    MMatrix selectAdjustMatrix;
    selectAdjustMatrix[0][0] = viewportW / selectW;
    selectAdjustMatrix[1][1] = viewportH / selectH;
    selectAdjustMatrix[3][0] = ((viewportX + viewportW/2.0) - (selectX + selectW/2.0)) / 
        viewportW * 2.0 * selectAdjustMatrix[0][0];
    selectAdjustMatrix[3][1] = ((viewportY + viewportH/2.0) - (selectY + selectH/2.0)) /
        viewportH * 2.0 * selectAdjustMatrix[1][1];

    MMatrix localToPort = modelViewMatrix * projMatrix * selectAdjustMatrix;

    view.beginSelect(buffer.get(), bufferSize*4);
    view.pushName(0);
    {
        Frustum frustum(localToPort.inverse());
        MMatrix xform(modelViewMatrix);
        
        DrawShadedState state(frustum, seconds, vboMode);
        DrawShadedTraversal traveral(state, xform, false, Frustum::kUnknown);
        rootNode->accept(traveral);
    }
    view.popName();
    int nbPick = view.endSelect();

    if (nbPick > 0) {
        unsigned int Zdepth = closestElem(nbPick, buffer.get());    
        float depth = float(Zdepth)/MAX_HW_DEPTH_VALUE;
        fMinZ = std::min(depth,fMinZ);
    }
}


//------------------------------------------------------------------------------
//
void GLPickingSelect::processBoundingBox(
    const SubNode::Ptr rootNode,
    const double       seconds
)
{
    // Allocate select buffer.
    const unsigned int bufferSize = 12;
    GLuint buffer[12*4];

    // Get the current viewport.
    M3dView view = fSelectInfo.view();

    // Get the bounding box.
    MBoundingBox boundingBox = BoundingBoxVisitor::boundingBox(rootNode, seconds);

    // Draw the bounding box.
    view.beginSelect(buffer, bufferSize*4);
    view.pushName(0);
    {
        VBOProxy vboProxy;
        vboProxy.drawBoundingBox(boundingBox);
    }
    view.popName();
    int nbPick = view.endSelect();

    // Determine the closest point.
    if (nbPick > 0) {
        unsigned int Zdepth = closestElem(nbPick, buffer);    
        float depth = float(Zdepth)/MAX_HW_DEPTH_VALUE;
        fMinZ = std::min(depth,fMinZ);
    }
}


//------------------------------------------------------------------------------
//
void GLPickingSelect::end()
{}


//------------------------------------------------------------------------------
//
bool GLPickingSelect::isSelected() const
{
    return fMinZ != std::numeric_limits<float>::max();
}


//------------------------------------------------------------------------------
//
float GLPickingSelect::minZ() const
{
    return fMinZ;
}

}

