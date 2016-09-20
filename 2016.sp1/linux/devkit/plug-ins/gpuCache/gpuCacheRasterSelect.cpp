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

#include "gpuCacheRasterSelect.h"

#include "gpuCacheSample.h"
#include "gpuCacheVBOProxy.h"
#include "gpuCacheDrawTraversal.h"
#include "gpuCacheGLFT.h"
#include "CacheReader.h"

#include <algorithm>


namespace {
        
using namespace GPUCache;

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

            // Note that we draw the vertices in addition to the wireframe
            // edges or mesh faces. This is necessary to make sure that primitive
            // will generate at least one pixel-fragment when it gets
            // rasterized, i.e. to handle case where the primitives are so
            // small on screen that they fall in-between the pixels.
            state().vboProxy().drawWireframe(sample, state().vboMode());
            state().vboProxy().drawVertices(sample, state().vboMode());
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

            // Note that we draw the vertices in addition to the wireframe
            // edges or mesh faces. This is necessary to make sure that primitive
            // will generate at least one pixel-fragment when it gets
            // rasterized, i.e. to handle case where the primitives are so
            // small on screen that they fall in-between the pixels.
            const size_t numGroups = sample->numIndexGroups();
            for (size_t groupId = 0; groupId < numGroups; groupId++) {
                state().vboProxy().drawTriangles(
                    sample, groupId, VBOProxy::kNoNormals, VBOProxy::kNoUVs,
                    state().vboMode());
                state().vboProxy().drawVertices(sample, state().vboMode());
            }
        }
    };

}

namespace GPUCache {

//==============================================================================
// CLASS RasterSelect
//==============================================================================

#define MAX_RASTER_SELECT_RENDER_SIZE 16

//------------------------------------------------------------------------------
//
RasterSelect::RasterSelect(
    MSelectInfo& selectInfo
) 
    : fSelectInfo(selectInfo),
      fMinZ(std::numeric_limits<float>::max())
{
    M3dView view = fSelectInfo.view();

    view.beginGL();

    unsigned int sxl, syl, sw, sh;
    fSelectInfo.selectRect(sxl, syl, sw, sh);

    unsigned int vxl, vyl, vw, vh;
    view.viewport(vxl, vyl, vw, vh);

    // Compute a new matrix that, when it is post-multiplied with the
    // projection matrix, will cause the picking region to fill only
    // a small region.

    const unsigned int width = (MAX_RASTER_SELECT_RENDER_SIZE < sw) ?
        MAX_RASTER_SELECT_RENDER_SIZE : sw;
    const unsigned int height = (MAX_RASTER_SELECT_RENDER_SIZE < sh) ?
        MAX_RASTER_SELECT_RENDER_SIZE : sh;

    const double sx = double(width) / double(sw);
    const double sy = double(height) / double(sh);

    const double fx = 2.0 / double(vw);
    const double fy = 2.0 / double(vh);
    
    MMatrix  selectMatrix;
    selectMatrix.matrix[0][0] = sx;
    selectMatrix.matrix[1][1] = sy;
    selectMatrix.matrix[3][0] = -1.0 - sx * (fx * (sxl - vxl) - 1.0);
    selectMatrix.matrix[3][1] = -1.0 - sy * (fy * (syl - vyl) - 1.0);
    
        
    MMatrix projMatrix;
    view.projectionMatrix(projMatrix);

    ::glMatrixMode(GL_PROJECTION);
    ::glPushMatrix();
    ::glLoadMatrixd(selectMatrix[0]);
    ::glMultMatrixd(projMatrix[0]);
    ::glMatrixMode(GL_MODELVIEW);

    ::glScissor(vxl, vyl, width, height);
    ::glEnable(GL_SCISSOR_TEST);
    ::glClear(GL_DEPTH_BUFFER_BIT);

    fWasDepthTestEnabled = ::glIsEnabled(GL_DEPTH_TEST);
    if (!fWasDepthTestEnabled) {
        ::glEnable(GL_DEPTH_TEST);
    }
}


//------------------------------------------------------------------------------
//
RasterSelect::~RasterSelect() 
{}


//------------------------------------------------------------------------------
//
void RasterSelect::processEdges(
    const SubNode::Ptr rootNode,
    double seconds,
    size_t /* numWires */,
    VBOProxy::VBOMode vboMode
)
{
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

    {
        Frustum frustum(localToPort.inverse());
        MMatrix xform(modelViewMatrix);
        
        DrawWireframeState state(frustum, seconds, vboMode);
        DrawWireframeTraversal traveral(state, xform, false, Frustum::kUnknown);
        rootNode->accept(traveral);
    }
}


//------------------------------------------------------------------------------
//
void RasterSelect::processTriangles(
    const SubNode::Ptr rootNode,
    double seconds,
    size_t /* numTriangles */,
    VBOProxy::VBOMode vboMode
)
{
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

    {
        Frustum frustum(localToPort.inverse());
        MMatrix xform(modelViewMatrix);
        
        DrawShadedState state(frustum, seconds, vboMode);
        DrawShadedTraversal traveral(state, xform, false, Frustum::kUnknown);
        rootNode->accept(traveral);
    }
}


//------------------------------------------------------------------------------
//
void RasterSelect::processBoundingBox(
    const SubNode::Ptr rootNode,
    double seconds
)
{
    // Not implemented.
    // Bounding Box selection is done by using GL Picking.
    assert(0);
}


//------------------------------------------------------------------------------
//
void RasterSelect::end()
{
    M3dView view = fSelectInfo.view();
    
    unsigned int sxl, syl, sw, sh;
    fSelectInfo.selectRect(sxl, syl, sw, sh);

    unsigned int vxl, vyl, vw, vh;
    view.viewport(vxl, vyl, vw, vh);

    const unsigned int width = (MAX_RASTER_SELECT_RENDER_SIZE < sw) ?
        MAX_RASTER_SELECT_RENDER_SIZE : sw;
    const unsigned int height = (MAX_RASTER_SELECT_RENDER_SIZE < sh) ?
        MAX_RASTER_SELECT_RENDER_SIZE : sh;

    float* selDepth = new float[MAX_RASTER_SELECT_RENDER_SIZE*
                                MAX_RASTER_SELECT_RENDER_SIZE];
    

    GLint buffer;
    ::glGetIntegerv( GL_READ_BUFFER, &buffer );
    ::glReadBuffer( GL_BACK );
    ::glReadPixels(vxl, vyl, width, height,
                   GL_DEPTH_COMPONENT, GL_FLOAT, (void *)selDepth); 
    ::glReadBuffer( buffer );

    for (unsigned int j=0; j<height; ++j) {
        for (unsigned int i=0; i<width; ++i) {
            const GLfloat depth = selDepth[j*width + i];
            if (depth < 1.0f) {
                fMinZ = std::min(fMinZ, depth);
            }
        }   
    }

    ::glMatrixMode(GL_PROJECTION);
    ::glPopMatrix();
    ::glMatrixMode(GL_MODELVIEW);

    ::glDisable(GL_SCISSOR_TEST);

    if (!fWasDepthTestEnabled) {
        ::glDisable(GL_DEPTH_TEST);        
    }

    view.endGL();
}

//------------------------------------------------------------------------------
//
bool RasterSelect::isSelected() const
{
    return fMinZ != std::numeric_limits<float>::max();
}


//------------------------------------------------------------------------------
//
float RasterSelect::minZ() const
{
    return fMinZ;
}

}
