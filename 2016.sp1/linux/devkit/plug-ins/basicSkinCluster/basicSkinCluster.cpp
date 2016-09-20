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
//  File: basicSkinCluster.cpp
//
//  Description:
//      Rudimentary implementation of a skin cluster.
//
//      Use this script to create a simple example.
/*      
loadPlugin basicSkinCluster;
      
proc connectJointCluster( string $j, int $i )
{
    if ( !objExists( $j+".lockInfluenceWeights" ) )
    {
        select -r $j;
        addAttr -sn "liw" -ln "lockInfluenceWeights" -at "bool";
    }
    connectAttr ($j+".liw") ("basicSkinCluster1.lockWeights["+$i+"]");
    connectAttr ($j+".worldMatrix[0]") ("basicSkinCluster1.matrix["+$i+"]");
    connectAttr ($j+".objectColorRGB") ("basicSkinCluster1.influenceColor["+$i+"]");
	float $m[] = `getAttr ($j+".wim")`;
	setAttr ("basicSkinCluster1.bindPreMatrix["+$i+"]") -type "matrix" $m[0] $m[1] $m[2] $m[3] $m[4] $m[5] $m[6] $m[7] $m[8] $m[9] $m[10] $m[11] $m[12] $m[13] $m[14] $m[15];
}

joint -p 1 0 1 ;
joint -p 0 0 0 ;
joint -e -zso -oj xyz -sao yup joint1;
joint -p 1 0 -1 ;
joint -e -zso -oj xyz -sao yup joint2;
polyTorus -r 1 -sr 0.5 -tw 0 -sx 50 -sy 50 -ax 0 1 0 -cuv 1 -ch 1;
deformer -type "basicSkinCluster";
setAttr basicSkinCluster1.useComponentsMatrix 1;
connectJointCluster( "joint1", 0 );
connectJointCluster( "joint2", 1 );
connectJointCluster( "joint3", 2 );
skinCluster -e -maximumInfluences 3 basicSkinCluster1;	// forces computation of default weights
*/

#include <maya/MFnPlugin.h>
#include <maya/MTypeId.h> 

#include <maya/MMatrixArray.h>
#include <maya/MStringArray.h>

#include <maya/MPxSkinCluster.h> 
#include <maya/MItGeometry.h>
#include <maya/MPoint.h>
#include <maya/MFnMatrixData.h>


class basicSkinCluster : public MPxSkinCluster
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

const MTypeId basicSkinCluster::id( 0x00080030 );


void* basicSkinCluster::creator()
{
    return new basicSkinCluster();
}

MStatus basicSkinCluster::initialize()
{
    return MStatus::kSuccess;
}


MStatus
basicSkinCluster::deform( MDataBlock& block,
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
    
	// get the influence transforms
	//
	MArrayDataHandle transformsHandle = block.inputArrayValue( matrix );
	int numTransforms = transformsHandle.elementCount();
	if ( numTransforms == 0 ) {
		return MS::kSuccess;
	}
	MMatrixArray transforms;
	for ( int i=0; i<numTransforms; ++i ) {
		transforms.append( MFnMatrixData( transformsHandle.inputValue().data() ).matrix() );
		transformsHandle.next();
	}
	MArrayDataHandle bindHandle = block.inputArrayValue( bindPreMatrix );
	if ( bindHandle.elementCount() > 0 ) {
		for ( int i=0; i<numTransforms; ++i ) {
			transforms[i] = MFnMatrixData( bindHandle.inputValue().data() ).matrix() * transforms[i];
			bindHandle.next();
		}
	}

	MArrayDataHandle weightListHandle = block.inputArrayValue( weightList );
	if ( weightListHandle.elementCount() == 0 ) {
		// no weights - nothing to do
		return MS::kSuccess;
	}

    // Iterate through each point in the geometry.
    //
    for ( ; !iter.isDone(); iter.next()) {
        MPoint pt = iter.position();
		MPoint skinned;

		// get the weights for this point
		MArrayDataHandle weightsHandle = weightListHandle.inputValue().child( weights );

		// compute the skinning
		for ( int i=0; i<numTransforms; ++i ) {
			if ( MS::kSuccess == weightsHandle.jumpToElement( i ) ) {
				skinned += ( pt * transforms[i] ) * weightsHandle.inputValue().asDouble();
			}
		}
        
		// Set the final position.
		iter.setPosition( skinned );

		// advance the weight list handle
		weightListHandle.next();
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
        "basicSkinCluster" ,
        basicSkinCluster::id ,
        &basicSkinCluster::creator ,
        &basicSkinCluster::initialize ,
        MPxNode::kSkinCluster
        );

    return result;
}

MStatus uninitializePlugin( MObject obj )
{
    MStatus result;

    MFnPlugin plugin( obj );
    result = plugin.deregisterNode( basicSkinCluster::id );

    return result;
}
