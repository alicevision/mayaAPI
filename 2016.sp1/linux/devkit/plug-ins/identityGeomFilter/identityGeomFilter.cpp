//-
// ==========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

//
//  File: identityGeomFilter.cpp
//
//  Description:
//      Empty implementation of a deformer. This node
//      performs no deformation and is basically an empty
//      shell that can be used to create actual deformers.
//
//
//      Use this script to create a simple example with the identity node.
//      
//      loadPlugin identityGeomFilter;
//      
//      polyTorus -r 1 -sr 0.5 -tw 0 -sx 50 -sy 50 -ax 0 1 0 -cuv 1 -ch 1;
//      deformer -type "identityGeomFilter";
//      select -cl;

#include <maya/MFnPlugin.h>
#include <maya/MTypeId.h> 

#include <maya/MStringArray.h>

#include <maya/MPxGeometryFilter.h> 
#include <maya/MItGeometry.h>
#include <maya/MPoint.h>


class identityGeomFilter : public MPxGeometryFilter
{
public:
    static  void*   creator();
    static  MStatus initialize();

    // Deformation function
    //
    virtual MStatus deform(MDataBlock&    block,
                           MItGeometry&   iter,
                           const MMatrix& mat,
                           unsigned int multiIndex);

    static const MTypeId id;
};

const MTypeId identityGeomFilter::id( 0x0008002F );


void* identityGeomFilter::creator()
{
    return new identityGeomFilter();
}

MStatus identityGeomFilter::initialize()
{
    return MStatus::kSuccess;
}


MStatus
identityGeomFilter::deform( MDataBlock& block,
                      MItGeometry& iter,
                      const MMatrix& /*m*/,
                      unsigned int multiIndex)
//
// Method: deform
//
// Description:   "Deforms" the point with an identity transformation
//
// Arguments:
//   block      : the datablock of the node
//   iter       : an iterator for the geometry to be deformed
//   m          : matrix to transform the point into world space
//   multiIndex : the index of the geometry that we are deforming
//
//
{
    MStatus returnStatus;

	// float env = block.inputValue( envelope ).asFloat();
    
    // Iterate through each point in the geometry.
    //
    for ( ; !iter.isDone(); iter.next()) {
        MPoint pt = iter.position();
        
        // Perform some calculation on pt.
        // e.g.
		// pt.x += 1.0 * env;
        
        // Set the final position.
        iter.setPosition(pt);
    }
    return returnStatus;
}


// standard initialization procedures
//

MStatus initializePlugin( MObject obj )
{
    MStatus result;

    MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");
    result = plugin.registerNode(
        "identityGeomFilter" ,
        identityGeomFilter::id ,
        &identityGeomFilter::creator ,
        &identityGeomFilter::initialize ,
        MPxNode::kGeometryFilter
        );

    return result;
}

MStatus uninitializePlugin( MObject obj )
{
    MStatus result;

    MFnPlugin plugin( obj );
    result = plugin.deregisterNode( identityGeomFilter::id );

    return result;
}
