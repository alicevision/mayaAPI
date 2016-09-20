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
//  File: basicBlendShape.cpp
//
//  Description:
//      Rudimentary implementation of a blendshape.
//
//      Use this script to create a simple example.
/*      
loadPlugin basicBlendShape;
polyTorus -r 1 -sr 0.5 -tw 0 -sx 50 -sy 50 -ax 0 1 0 -cuv 1 -ch 1;
polyTorus -r 1 -sr 0.5 -tw 0 -sx 50 -sy 50 -ax 0 1 0 -cuv 1 -ch 1;
scale -r 0.5 1 1;
makeIdentity -apply true -t 1 -r 1 -s 1 -n 0 -pn 1;
select -r pTorus1;
deformer -type "basicBlendShape";
blendShape -edit -t pTorus1 0 pTorus2 1.0 basicBlendShape1;
*/

#include <maya/MFnPlugin.h>
#include <maya/MTypeId.h> 

#include <maya/MMatrixArray.h>
#include <maya/MStringArray.h>
#include <maya/MFloatArray.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>

#include <maya/MPxBlendShape.h> 
#include <maya/MItGeometry.h>
#include <maya/MFnPointArrayData.h>


class basicBlendShape : public MPxBlendShape
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

const MTypeId basicBlendShape::id( 0x00080031 );


void* basicBlendShape::creator()
{
    return new basicBlendShape();
}

MStatus basicBlendShape::initialize()
{
    return MStatus::kSuccess;
}


MStatus
basicBlendShape::deform( MDataBlock& block,
                      MItGeometry& iter,
                      const MMatrix& /*m*/,
                      unsigned int multiIndex)
//
// Method: deform
//
// Description:   Deforms the point with a simple smooth skinning algorithm
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

	// get the weights
	//
	MArrayDataHandle weightMH = block.inputArrayValue( weight );
	unsigned int numWeights = weightMH.elementCount();
	MFloatArray weights;
	for ( unsigned int w=0; w<numWeights; ++w ) {
		weights.append( weightMH.inputValue().asFloat() );
		weightMH.next();
	}

	// get the input targets
	// as a point array per weight
	//
	MArrayDataHandle inputTargetMH = block.inputArrayValue( inputTarget );
	returnStatus = inputTargetMH.jumpToElement( multiIndex );
	if ( !returnStatus ) {
		return returnStatus;
	}
	MPointArray* targets = new MPointArray[ numWeights ];
	MDataHandle inputTargetH = inputTargetMH.inputValue();
	MArrayDataHandle inputTargetGroupMH = inputTargetH.child( inputTargetGroup );
	for ( unsigned int w=0; w<numWeights; ++w ) {
		// inputPointsTarget is computed on pull,
		// so can't just read it out of the datablock
		MPlug plug( thisMObject(), inputPointsTarget );
		plug.selectAncestorLogicalIndex( multiIndex, inputTarget );
		plug.selectAncestorLogicalIndex( w, inputTargetGroup );
		// ignore deformer chains here and just take the first one
		plug.selectAncestorLogicalIndex( 6000, inputTargetItem );
		MDGContext context = block.context();
		targets[ w ] = MFnPointArrayData( plug.asMObject( context ) ).array();
	}

    // Iterate through each point in the geometry.
    //
    for ( ; !iter.isDone(); iter.next()) {
        MPoint pt = iter.position();
		unsigned int index = iter.index();
		for ( unsigned int w=0; w<numWeights; ++w ) {
			float wgt = weights[w];
			const MPointArray& pts = targets[w];

			inputTargetGroupMH.jumpToArrayElement( w );
			MArrayDataHandle targetWeightsMH = inputTargetGroupMH.inputValue().child( targetWeights );
			if ( targetWeightsMH.jumpToElement( index ) ) {
				wgt *= targetWeightsMH.inputValue().asFloat();
			}

			if ( index < pts.length() ) {
				pt += pts[index] * wgt;
			}
		}
		// Set the final position.
		iter.setPosition( pt );
    }

	delete [] targets;

    return returnStatus;
}


// standard initialization procedures
//

MStatus initializePlugin( MObject obj )
{
    MStatus result;

    MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");
    result = plugin.registerNode(
        "basicBlendShape" ,
        basicBlendShape::id ,
        &basicBlendShape::creator ,
        &basicBlendShape::initialize ,
        MPxNode::kBlendShape
        );

    return result;
}

MStatus uninitializePlugin( MObject obj )
{
    MStatus result;

    MFnPlugin plugin( obj );
    result = plugin.deregisterNode( basicBlendShape::id );

    return result;
}
