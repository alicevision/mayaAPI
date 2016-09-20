#ifndef _gpuCacheRasterSelect_h_
#define _gpuCacheRasterSelect_h_

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

#include "gpuCacheSelect.h"

#include <maya/MSelectInfo.h>

namespace GPUCache {

/*==============================================================================
 * CLASS RasterSelect
 *============================================================================*/

// Rasterization based slection
class RasterSelect : public Select
{
public:
    typedef unsigned int index_t;

    //  Description:
    //      Begin a rasterization-based selection.
    //
    //      Until the call to end(), the user uses the calls
    //      processVertices(), processEdges() and processVertices() to
    //      specify the geometry to test for selection hits. The
    //      selection will performed by reading-back the rasterized
    //      primitives.
    //
    //      The selection region is defined by selectInfo.selectRect().
    //    
    //  Notes:
    //      On some hardware, rasterization-based selection can be up to a
    //      100 times faster than selection based on OpenGL picking (such
    //      as M3dView::begin/endSelect()) when applied to large meshes.
    //
    //      When using rasterization-based selection, the user should not
    //      change any OpenGL state that would affect the color of the
    //      generated fragments. This includes:
    //         - Current color
    //         - Alpha-Blending
    //         - Shading
    //         - Lighting
    //         - Texturing
    //         - Etc.
    RasterSelect(MSelectInfo& selectInfo);
    virtual ~RasterSelect();
    
    // Base class virtual overrides */
    virtual void processEdges(const SubNode::Ptr rootNode,
                              double seconds,
                              size_t numWires,
                              VBOProxy::VBOMode vboMode);

    virtual void processTriangles(const SubNode::Ptr rootNode,
                                  double seconds,
                                  size_t numTriangles,
                                  VBOProxy::VBOMode vboMode);

    virtual void processBoundingBox(const SubNode::Ptr rootNode,
                                    double seconds);

    
    virtual void end();
    virtual bool isSelected() const;
    virtual float minZ() const;
    
private:
    MSelectInfo     fSelectInfo;
    float           fMinZ;
    GLboolean       fWasDepthTestEnabled;
};

}

#endif
