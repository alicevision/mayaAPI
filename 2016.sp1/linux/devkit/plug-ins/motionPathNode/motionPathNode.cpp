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

#include <maya/MDataHandle.h> 
#include <maya/MFnNumericAttribute.h> 
#include <maya/MFnPlugin.h> 
#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#include <maya/MMatrix.h>
#include <maya/MPxMotionPathNode.h> 
#include <maya/MQuaternion.h> 
#include <maya/MStatus.h> 
#include <maya/MTransformationMatrix.h>
#include <maya/MTypeId.h>
 
////////////////////////////////////
// Plugin MotionPathNode Class //
////////////////////////////////////

// INTRODUCTION:
//	This class will create a custom "motionPathNode" node which
//	illustrates how a developer can extend Maya's motionPath
//	functionality by creating a surfboard that follows a path.
//
// HOW TO USE THIS PLUG-IN:
//
// Part 1, Getting started
//	(1) Compile this plug-in.
//	(2) Load the compiled plug-in into Maya via the plug-in manager.
//	(3) Create an anim curve to define how the motion progresses along
//		the path. Here we assume the animation is 114 frames long and
//		progresses linearly in time.
//			string $ac = `createNode animCurveTU -n "animCurve"`;
//			setKeyframe -t 1 -v 0 $ac;
//			setKeyframe -t 114 -v 1 $ac;
//	(4) Create a path for the motion to navigate: we use a NURBS circle.
//			string $tmp[] = `circle -ch on -o on -nr 0 1 0 -r 16.688138`;
//			string $path = $tmp[0];
//	(5) Create a motionPathNode and connect the path and anim curve.
//			$mp = `createNode motionPathNode`;
//			setAttr ($mp+".fractionMode") true;
//			connectAttr ($ac+".output") ($mp+".uValue");
//			connectAttr ($path+".worldSpace[0]") ($mp+".geometryPath");
//
// Part 2, Translation along the path
//	(6) Create a surfboard and connect to the motionPathNode.
//			$tmp = `polySphere`;
//			scale 2 0.1 1;
//			string $sp = $tmp[0];
//			connectAttr ($mp+".allCoordinates") ($sp+".translate");
//	(7) Play the animation and the surfboard will move along the path,
//		wobbling back and forth. Change the "offset" plug to affect
//		the magnitude of the offset effect and change "wobbleRate" to
//		affect the frequency of the wobble.
// 	   		currentTime 1;
// 	   		play -wait;
//
// Part 3, Add rotation so the surfboard follows the path
// 	(8)	Turn on the "follow" attribute to enable rotation.
// 			setAttr ($mp+".follow") 1;
//
// 	(9)	Define the axes then play again. The surfboard should
// 		now follow the path.
// 			setAttr ($mp+".frontAxis") 0;
// 			setAttr ($mp+".upAxis") 1;
// 	   		currentTime 1;
// 	   		play -wait;
//
// Part 4, Make the surfboard bank into the curve
//	(10) Enable banking to add rotation based on the sharpness of
//		 the curve by rotating the surfboard about the front vector.
// 			setAttr ($mp+".bank") true;
// 			setAttr ($mp+".bankScale") 5.0;
//

// Useful constants.
//
#define ALMOST_ZERO	1.0e-5
#define	TWO_PI		( 2.0 * 3.1415927 )

// Declare our motionPathNode class which derives from MPxMotionPath.
//
class motionPathNode : public MPxMotionPathNode
{
public:
						motionPathNode();
	virtual				~motionPathNode(); 

	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );

	static  void*		creator();
	static  MStatus		initialize();

	// Member variables...
	//
	static	MObject		offset;			// The "offset" attribute
	static	MObject		wobbleRate;		// The "wobbleRate" attribute

	static	MTypeId		id;				// The IFF type id

private:
	static MStatus		affectsOutput( MObject& attr );
};

// Attributes defined on this node.
//
MObject	motionPathNode::offset;
MObject	motionPathNode::wobbleRate;

// IFF type ID
// Each node requires a unique identifier which is used by
// MFnDependencyNode::create() to identify which node to create, and by
// the Maya file format.
//
// For local testing of nodes you can use any identifier between
// 0x00000000 and 0x0007ffff, but for any node that you plan to use for
// more permanent purposes, you should get a universally unique id from
// Autodesk Support. You will be assigned a unique range that you can manage
// on your own.
//
MTypeId motionPathNode::id( 0x0008002D );

// This node does not need to perform any special actions on creation or
// destruction
//
motionPathNode::motionPathNode()
{
}
motionPathNode::~motionPathNode()
{
}

// The compute() method does the actual work of the node using the inputs
// of the node to generate its output.
//
// Compute takes two parameters: plug and data.
// - Plug is the the data value that needs to be recomputed
// - Data provides handles to all of the nodes attributes, only these
//   handles should be used when performing computations.
//
MStatus motionPathNode::compute( const MPlug& plug, MDataBlock& data )
{
	double	f;
	MStatus status;

	// Read the attributes we need from the datablock.
	//
	double	uVal			= data.inputValue( uValue ).asDouble();
	bool	fractionModeVal	= data.inputValue( fractionMode ).asBool();
	bool	followVal		= data.inputValue( follow ).asBool();
	int		frontAxisVal	= data.inputValue( frontAxis ).asShort();
	int		upAxisVal		= data.inputValue( upAxis ).asShort();
	bool	bankVal			= data.inputValue( bank ).asBool();
	double  bankScaleVal	= data.inputValue( bankScale ).asDouble();
	double  bankThresholdVal= data.inputValue( bankThreshold ).asDouble();
	double	offsetVal		= data.inputValue( offset ).asDouble();
	double	wobbleRateVal	= data.inputValue( wobbleRate ).asDouble();

	// Make sure the value is fractional.
	//
	if ( fractionModeVal ) {
		f = uVal;
	} else {
		f = parametricToFractional( uVal, &status );
	}
	CHECK_MSTATUS_AND_RETURN_IT( status );

	// To compute the sample location on the path, first wrap the fraction
	// around the start of the the path in case it goes past the end
	// to prevent clamping, then compute the sample location on the path.
	// 
	f = wraparoundFractionalValue( f, &status );
	CHECK_MSTATUS_AND_RETURN_IT( status );

	MPoint location = position( data, f, &status );
	CHECK_MSTATUS_AND_RETURN_IT( status );

	// Get the orthogonal vectors on the motion path.
	//
	const MVector worldUp = MGlobal::upAxis();
	MVector	front, side, up;
	CHECK_MSTATUS_AND_RETURN_IT( getVectors( data, f, front, side, up,
			&worldUp ) );

	// If follow (i.e. rotation) is enabled, check if banking is also
	// enabled and if so, bank into the turn.
	//
	if ( followVal && bankVal ) {
		MQuaternion bankQuat = banking( data, f, worldUp,
				bankScaleVal, bankThresholdVal, &status );
		CHECK_MSTATUS_AND_RETURN_IT( status );
		up = up.rotateBy( bankQuat );
		side = front ^ up;
	}

	// Compute the wobble that moves the sphere back and forth as it
	// traverses the path.
	//
	if ( fabs( offsetVal ) > ALMOST_ZERO
				&& fabs( wobbleRateVal ) > ALMOST_ZERO ) {
		double wobble = offsetVal * sin( TWO_PI * wobbleRateVal * f );
		MVector tmp = side * wobble;
		location += tmp;
	}

	// Write the result values to the output plugs.
	//
	data.outputValue( allCoordinates ).set( location.x, location.y,
			location.z );
	if ( followVal ) {
		MTransformationMatrix	resultOrientation = matrix( front, side,
				up, frontAxisVal, upAxisVal, &status );
		CHECK_MSTATUS_AND_RETURN_IT( status );
		MTransformationMatrix::RotationOrder ro
				= (MTransformationMatrix::RotationOrder)
					( data.inputValue( rotateOrder ).asShort() + 1 );
		double  rot[3];
		status = MTransformationMatrix( resultOrientation ).getRotation(
				rot, ro );
		CHECK_MSTATUS_AND_RETURN_IT( status );
		data.outputValue( rotate ).set( rot[0], rot[1], rot[2] );
	}
	
	return( MS::kSuccess );
}

// The creator() method allows Maya to instantiate instances of this node.
// It is called every time a new instance of the node is requested by
// either the createNode command or the MFnDependencyNode::create()
// method.
//
// In this case creator simply returns a new motionPathNode object.
//
void* motionPathNode::creator()
{
	return( new motionPathNode() );
}

// The initialize method is called only once when the node is first
// registered with Maya. In general,
//
MStatus motionPathNode::initialize()
{
	MFnNumericAttribute		nAttr;

	//====================================================================
	//          I N P U T     A T T R I B U T E S
	//====================================================================
	//
	offset = nAttr.create( "offset", "o",
			MFnNumericData::kDouble, 4.0 ); 
	addAttribute( offset );

	wobbleRate = nAttr.create( "wobbleRate", "w",
			MFnNumericData::kDouble, 10.0 ); 
	addAttribute( wobbleRate );

	//====================================================================
	//          A F F E C T S    R E L A T I O N S H I P S
	//====================================================================
	//
	CHECK_MSTATUS_AND_RETURN_IT( affectsOutput( offset ) );
	CHECK_MSTATUS_AND_RETURN_IT( affectsOutput( wobbleRate ) );

	return( MS::kSuccess );
}

MStatus	motionPathNode::affectsOutput( MObject& attr )
{
	CHECK_MSTATUS_AND_RETURN_IT( attributeAffects( attr, rotate ) );
	CHECK_MSTATUS_AND_RETURN_IT( attributeAffects( attr, rotateX ) );
	CHECK_MSTATUS_AND_RETURN_IT( attributeAffects( attr, rotateY ) );
	CHECK_MSTATUS_AND_RETURN_IT( attributeAffects( attr, rotateZ ) );
	CHECK_MSTATUS_AND_RETURN_IT( attributeAffects( attr, allCoordinates ) );
	CHECK_MSTATUS_AND_RETURN_IT( attributeAffects( attr, xCoordinate ) );
	CHECK_MSTATUS_AND_RETURN_IT( attributeAffects( attr, yCoordinate ) );
	CHECK_MSTATUS_AND_RETURN_IT( attributeAffects( attr, zCoordinate ) );

	return( MS::kSuccess );
}

// These methods load and unload the plugin, registerNode registers the
// new node type with maya
//
MStatus initializePlugin( MObject obj )
{ 
	MStatus		status;
	MFnPlugin	plugin( obj );

	status = plugin.registerNode( "motionPathNode",
			motionPathNode::id,
			motionPathNode::creator,
			motionPathNode::initialize,
			MPxNode::kMotionPathNode );
	if (!status) {
		status.perror("registerNode");
		return( status );
	}

	return( status );
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus		status;
	MFnPlugin	plugin( obj );

	status = plugin.deregisterNode( motionPathNode::id );
	if (!status) {
		status.perror("deregisterNode");
		return( status );
	}

	return( status );
}
