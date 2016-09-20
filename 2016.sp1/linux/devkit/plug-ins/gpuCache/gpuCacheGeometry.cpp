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

#include <algorithm>
#include <cassert>
#include <limits>
#include <vector>

//==============================================================================
// PRIVATE FUNCTIONS
//==============================================================================

namespace {

    using namespace GPUCache;


    //==========================================================================
    // CLASS NodeDataValidator
    //==========================================================================

    class NodeDataValidator : public SubNodeVisitor
    {
    public:
        NodeDataValidator() : fIsValid(false) {}

        bool isValid() const { return fIsValid; }

        virtual void visit(const XformData&   xform,
                           const SubNode&     /*subNode*/)
        {
            fIsValid = !xform.getSamples().empty();
        }
        

        virtual void visit(const ShapeData&   shape,
                           const SubNode&     /*subNode*/)
        {
            fIsValid = !shape.getSamples().empty();
        }
        
    private:
        bool fIsValid;
    };
    
}

namespace GPUCache {

//==============================================================================
// CLASS SubNodeData
//==============================================================================

SubNodeData::~SubNodeData()
{}


//==============================================================================
// CLASS ShapeData
//==============================================================================

ShapeData::ShapeData()
{}

ShapeData::~ShapeData()
{}

const boost::shared_ptr<const ShapeSample>&
ShapeData::getSample(double seconds) const
{
    // // We get rid of the fCacheReaderProxy as soon as we start drawing
    // // to free up memory. The fCacheReaderProxy was kept opened just
    // // in case that another ShapeData node would have been reading
    // // from the same cache file to save the reopening of the file.
    
    // fCacheReaderProxy.reset();

    // There should always be at least one sample at this point!
    assert(!fSamples.empty());
    if (fSamples.empty()) {
        static const boost::shared_ptr<const ShapeSample> null;
        return null;
    }
    
    SampleMap::const_iterator it = fSamples.upper_bound(seconds);
    if (it != fSamples.begin()) {
        --it;
    }

    return it->second;
}

void ShapeData::addSample(
    const boost::shared_ptr<const ShapeSample>& sample)
{
    fSamples[sample->timeInSeconds()] = sample;
}

void ShapeData::accept(SubNodeVisitor& visitor,
                       const SubNode&  subNode) const
{
    return visitor.visit(*this, subNode);
}

void ShapeData::setMaterial(const MString& material)
{
    assert(fMaterials.empty());
    fMaterials.push_back(material);
}

void ShapeData::setMaterials(const std::vector<MString>& materials)
{
    assert(fMaterials.empty());
    fMaterials = materials;
}

const std::vector<MString>& ShapeData::getMaterials() const
{
    return fMaterials;
}


//==============================================================================
// CLASS XformData
//==============================================================================

XformData::~XformData()
{}

const boost::shared_ptr<const XformSample>&
XformData::getSample(double seconds) const
{
    // There should always be at least one sample at this point!
    assert(!fSamples.empty());
    if (fSamples.empty()) {
        static const boost::shared_ptr<const XformSample> null;
        return null;
    }

    SampleMap::const_iterator it = fSamples.upper_bound(seconds);
    if (it != fSamples.begin()) {
        --it;
    }

    return it->second;
}

void XformData::accept(SubNodeVisitor& visitor,
                       const SubNode&  subNode) const
{
    return visitor.visit(*this, subNode);
}


//==============================================================================
// CLASS SubNodeVisitor
//==============================================================================

SubNodeVisitor::~SubNodeVisitor()
{}


//==============================================================================
// CLASS SubNode
//==============================================================================

SubNode::SubNode(
    const MString& name,
    const SubNodeData::Ptr& nodeData)
    : fName(name), fNodeData(nodeData), fTransparentType(kOpaque)
{
    // Make it impossible to contruct an invalid sub node!
    NodeDataValidator validator;
    nodeData->accept(validator, *this);
    assert(validator.isValid());
}

SubNode::~SubNode()
{}

}
