#ifndef _gpuCacheDrawTraversal_h_
#define _gpuCacheDrawTraversal_h_

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

#include "gpuCacheGeometry.h"
#include "gpuCacheFrustum.h"
#include "gpuCacheVBOProxy.h"

#include <boost/foreach.hpp>

namespace GPUCache {
    
//==========================================================================
// CLASS DrawTraversalState
//==========================================================================

// Minimal traveral state
class DrawTraversalState
{
public:

    enum TransparentPruneType {
        kPruneNone,
        kPruneOpaque,
        kPruneTransparent
    };

    DrawTraversalState(
        const Frustum&             frustrum,
        const double               seconds,
        const TransparentPruneType transparentPrune)
        : fFrustum(frustrum),
          fSeconds(seconds),
          fTransparentPrune(transparentPrune)
    {}
	virtual ~DrawTraversalState() {}

    const Frustum&  frustum() const { return fFrustum; }
    double          seconds() const { return fSeconds; }
    TransparentPruneType transparentPrune() const { return fTransparentPrune; }

    VBOProxy&       vboProxy()      { return fVBOProxy; }
    
private:
	DrawTraversalState(const DrawTraversalState&);
	const DrawTraversalState& operator=(const DrawTraversalState&);
            
private:
    const Frustum              fFrustum;
    const double               fSeconds;
    const TransparentPruneType fTransparentPrune;
    VBOProxy                   fVBOProxy;
};


//==============================================================================
// CLASS DrawTraversal
//==============================================================================

// A traversal template implementing coordinate transformation and
// hierarchical view frustum culling. The user only to implement a
// draw function with the following signature:
//
// void draw(const boost::shared_ptr<const ShapeSample>& sample);
//
// Implemented using the "Curiously recurring template pattern".

template <class Derived, class State>
class DrawTraversal : public SubNodeVisitor
{
public:

    State& state() const            { return fState; }
    const MMatrix& xform() const    { return fXform; }

    
protected:
    
    DrawTraversal(
        State&                  state,
        const MMatrix&          xform,
        bool                    isReflection,
        Frustum::ClippingResult parentClippingResult
    )
        : fState(state),
          fXform(xform),
          fIsReflection(isReflection),
          fParentClippingResult(parentClippingResult)
    {}

    bool isReflection() const { return fIsReflection; }
    const SubNode& subNode() const { return *fSubNode; }


private:
    
    virtual void visit(const XformData&   xform,
                       const SubNode&     subNode)
    {
        if (state().transparentPrune() == State::kPruneOpaque &&
                subNode.transparentType() == SubNode::kOpaque) {
            return;
        }
        else if (state().transparentPrune() == State::kPruneTransparent &&
                subNode.transparentType() == SubNode::kTransparent) {
            return;
        }

        const boost::shared_ptr<const XformSample>& sample =
            xform.getSample(fState.seconds());
        if (!sample) return;

        if (!sample->visibility()) return;

        // Perform view frustum culling
        // All bounding boxes are already in the axis of the root xform sub-node
        Frustum::ClippingResult clippingResult = Frustum::kInside;
        if (fParentClippingResult != Frustum::kInside) {
            // Only check the bounding box if the bounding box intersects with
            // the view frustum
            clippingResult = state().frustum().test(
                    sample->boundingBox(), fParentClippingResult);

            // Prune this sub-hierarchy if the bounding box is
            // outside the view frustum
            if (clippingResult == Frustum::kOutside) {
                return;
            }
        }

        // Flip the global reflection flag back-and-​forth depending on
        // the reflection of the local matrix.
        Derived traversal(fState, sample->xform() * fXform,
                          bool(fIsReflection ^ sample->isReflection()),
                          clippingResult);

        // Recurse into children sub nodes. Expand all instances.
        BOOST_FOREACH(const SubNode::Ptr& child,
                      subNode.getChildren() ) {
            child->accept(traversal);
        }
    }
        
    virtual void visit(const ShapeData&   shape,
                       const SubNode&     subNode)
    {
        const boost::shared_ptr<const ShapeSample>& sample =
            shape.getSample(fState.seconds());
        if (!sample) return;

        fSubNode = &subNode;
        static_cast<Derived*>(this)->draw(sample);
    }

private:
	DrawTraversal(const DrawTraversal&);
	const DrawTraversal& operator=(const DrawTraversal&);
        

private:
    
    State&                        fState;
    const SubNode*                fSubNode;
    const MMatrix                 fXform;
    const bool                    fIsReflection;
    const Frustum::ClippingResult fParentClippingResult;
};

} // namespace GPUCache

#endif
