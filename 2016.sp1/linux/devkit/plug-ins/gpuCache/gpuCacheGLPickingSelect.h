#ifndef _gpuCacheGLPickingSelect_h_
#define _gpuCacheGLPickingSelect_h_

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
 * CLASS GLPickingSelect
 *============================================================================*/

// GL picking based slection
class GLPickingSelect : public Select
{
public:

    // Begin a selection using OpenGL picking
    // 
    // Until the call to end(), the user uses the calls
    // processVertices(), processEdges() and processVertices() to
    // specify the geometry to test for selection hits. 
    // 
    // The selection region is defined by selectInfo.selectRect().
    GLPickingSelect(MSelectInfo& selectInfo);
    virtual ~GLPickingSelect();

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
    MSelectInfo                 fSelectInfo;
    float                       fMinZ;
};

} // namespace GPUCache

#endif
