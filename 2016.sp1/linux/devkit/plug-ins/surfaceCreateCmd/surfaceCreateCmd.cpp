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

#include <maya/MStatus.h>
#include <maya/MObject.h>
#include <maya/MFnPlugin.h>
#include <maya/MDoubleArray.h>
#include <maya/MPointArray.h>
#include <maya/MPoint.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MString.h>
#include <maya/MPxCommand.h>

#include <maya/MIOStream.h> 
#include <math.h> 

class surfaceCreate : public MPxCommand {
public:

					surfaceCreate();
	virtual			~surfaceCreate();

	virtual MStatus	doIt( const MArgList& args );

	static void*	creator();
};

#define NUM_SPANS        30
#define WIDTH            10.0
#define VERTICAL_SCALING 4.0

surfaceCreate::~surfaceCreate() {}

surfaceCreate::surfaceCreate() {}

void* surfaceCreate::creator()
{
	return new surfaceCreate();
}

MStatus surfaceCreate::doIt( const MArgList& )
//
//	Description:
//		Plugin command to test Selection List Iterator.
//
//
{  
	// Set up knots
	//
	MDoubleArray knotArray;
	int i;
    // Add extra starting knots so that the first CV matches
	// the curve start point
	//
	knotArray.append( 0.0 );
	knotArray.append( 0.0 );
	for ( i = 0; i <= NUM_SPANS; i++ ) {
		knotArray.append( (double)i );
	}
	// Add extra ending knots so that the last CV matches the curve end point
	//
	knotArray.append( (double)i );
	knotArray.append( (double)i );
 
	// Now, Set up CVs
	//
	MPointArray cvArray;
	
	// We need a 2D array of CVs with NUM_SPANS + 3 CVs on a side
	//
	int last = NUM_SPANS + 3;
	for ( i = 0; i < last; i++ ) {
		for ( int j = 0; j < last; j++ ) {
			MPoint cv;
			cv.x = (((double)(j))/((double)(NUM_SPANS + 3)) * WIDTH) 
				- (WIDTH/2.0);
			cv.z = (((double)(i))/((double)(NUM_SPANS + 3)) * WIDTH) 
				- (WIDTH/2.0);
			double dist = sqrt( cv.x*cv.x + cv.z*cv.z );
			cv.y = cos( dist ) * VERTICAL_SCALING;
			cvArray.append( cv );
		}
	}

	// Create the surface
	// 
	MFnNurbsSurface mfnNurbsSurf;

	MStatus stat;
	mfnNurbsSurf.create( cvArray, knotArray, knotArray, 3, 3, 
						 MFnNurbsSurface::kOpen, MFnNurbsSurface::kOpen,
						 true, MObject::kNullObj, &stat );

	if ( MS::kSuccess != stat )
		cerr << "surfaceCreate failed: status " << stat << endl;

	return stat;
}

MStatus initializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");

	status = plugin.registerCommand( "surfaceCreate", surfaceCreate::creator );
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
	status = plugin.deregisterCommand( "surfaceCreate" );
	if (!status) {
		status.perror("registerCommand");
		return status;
	}
	
	return status;
}
