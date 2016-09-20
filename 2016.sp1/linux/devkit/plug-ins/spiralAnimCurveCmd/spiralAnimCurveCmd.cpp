//-
// ==========================================================================
// Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

#include <math.h>
#include <maya/MIOStream.h>

#include <maya/MGlobal.h>
#include <maya/MFnPlugin.h> 
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MMatrix.h>
#include <maya/MDagPath.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MFnDagNode.h>
#include <maya/MPxCommand.h>
#include <maya/MString.h>

///////////////
// Constants //
///////////////

#define OUTWARD_VELOCITY 0.075
#define RADIAL_VELOCITY  0.05
#define NUM_FRAMES       120

//////////////////////
// Class Definition //
//////////////////////

class spiralAnimCurve : public MPxCommand {
public:

					spiralAnimCurve() {};
	virtual			~spiralAnimCurve();

	virtual MStatus	doIt( const MArgList& args );
	static void*	creator();
};

//////////////////////////
// Class Implementation //
//////////////////////////

#define NUM_SPANS        30
#define WIDTH            10.0
#define VERTICAL_SCALING 4.0

spiralAnimCurve::~spiralAnimCurve() {}

void* spiralAnimCurve::creator()
{
	return new spiralAnimCurve();
}

// Set keyframes to move selected object in a spiral
MStatus spiralAnimCurve::doIt( const MArgList& )
{   
	// Get the Active Selection List
	//
	MStatus status;
	MSelectionList	sList;
	MGlobal::getActiveSelectionList( sList );

	// Create an iterator for the selection list
	//
	MItSelectionList iter( sList, MFn::kDagNode, &status );
	if ( MS::kSuccess != status ) {
		cerr << "Failure in plugin setup";
		return MS::kFailure;
	}
	
	MDagPath mObject;	
	MObject mComponent;

	for ( ; !iter.isDone(); iter.next() ) {
		status = iter.getDagPath( mObject, mComponent );

		// Check if there was an error
		//
		if ( MS::kSuccess != status ) continue;

		// We don't handle components
		//
		if ( !mComponent.isNull() ) continue;
		
		// Create the function set
		//
		MFnDagNode fnSet( mObject, &status );

		if ( MS::kSuccess != status ) {
			cerr << "Failure to create function set\n";
			continue;
		}

		// Get the plug for the X-translation channel
		//
		MString attrName( "translateX" );
		const MObject attrX = fnSet.attribute( attrName, &status );
		if ( MS::kSuccess != status ) {
			cerr << "Failure to find attribute\n";
		}
		MFnAnimCurve acFnSetX;
		acFnSetX.create( mObject.transform(), attrX, NULL, &status ); 

		if ( MS::kSuccess != status ) {
			cerr << "Failure creating MFnAnimCurve function set (translateX)\n";
			continue;
		}

		// Repeat for Y-translation
		//
		attrName.set( "translateZ" );
		const MObject attrZ = fnSet.attribute( attrName, &status );
		if ( MS::kSuccess != status ) {
			cerr << "Failure to find attribute\n";
		}
		MFnAnimCurve acFnSetZ;
		acFnSetZ.create( mObject.transform(), attrZ, NULL, &status ); 

		if ( MS::kSuccess != status ) {
			cerr << "Failure creating MFnAnimCurve function set (translateZ)\n";
			continue;
		}
		
		// Build spiral animation
		// 
		for( int i = 1; i <= NUM_FRAMES; i++ ) {
			// Build the keyframe at frame i
			//
			double x = sin( (double)i * RADIAL_VELOCITY ) * 
				            ( (double)i * OUTWARD_VELOCITY ); 
			double z = cos( (double)i * RADIAL_VELOCITY ) * 
				            ( (double)i * OUTWARD_VELOCITY );

			// cerr << "Setting keys - frame: " << i << "  x: " << x << "  z: " << z << endl;

			MTime tm( (double)i, MTime::kFilm );
			if ( ( MS::kSuccess != acFnSetX.addKeyframe( tm, x ) ) ||
				 ( MS::kSuccess != acFnSetZ.addKeyframe( tm, z ) ) ) {
				cerr << "Error setting the keyframe\n"; 
			}
		}

	}

	return status;
}

//////////////////////////////////
// Register command with system //
//////////////////////////////////

MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");

	status = plugin.registerCommand( "spiralAnimCurve",
									 spiralAnimCurve::creator ); 
	if (!status) {
		status.perror("registerCommand");
		return status;
	}
	
	return status;
}

MStatus uninitializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterCommand( "spiralAnimCurve" );
	if (!status) {
		status.perror("deregisterCommand");
		return status;
	}
	
	return status;
}
