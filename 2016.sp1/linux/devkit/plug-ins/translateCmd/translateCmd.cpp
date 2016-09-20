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

////////////////////////////////////////////////////////////////////////
//
//		Translate selected objects
//
////////////////////////////////////////////////////////////////////////

#include <maya/MIOStream.h>

#include <maya/MPxCommand.h>
#include <maya/MFnPlugin.h>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MDagPath.h>

#include <maya/MItCurveCV.h>
#include <maya/MItSurfaceCV.h>
#include <maya/MItMeshVertex.h>


class translate : public MPxCommand
{
public:
					translate() {};
	virtual			~translate(); 

	MStatus			doIt( const MArgList& args );
	static void*	creator();
};

translate::~translate() {}

void* translate::creator()
{
	return new translate();
}

// If given no arguments, translate all selected objects by 1 unit on
// the X axis.  If 3 double parameters are given, they specify the X,
// Y, and Z components of the vector to translate objects by.
MStatus translate::doIt( const MArgList& args )
{
	MStatus stat;
	MVector 	vector( 1.0, 0.0, 0.0 );

	if ( args.length() == 3 )  {
		unsigned i = 0;
		vector = args.asVector(i,3);
	}

	// Create a selection list iterator
	//
	MSelectionList slist;
	MGlobal::getActiveSelectionList( slist );
	MItSelectionList iter( slist, MFn::kInvalid, &stat );

	if ( MS::kSuccess == stat ) {
		MDagPath 	mdagPath;		// Item dag path
		MObject 	mComponent;		// Current component

		// Translate all selected objects
		//
		for ( ; !iter.isDone(); iter.next() ) 
		{
			// Get path and possibly a component
			//
			iter.getDagPath( mdagPath, mComponent );

			MItCurveCV cvFn( mdagPath, mComponent, &stat );
			if ( MS::kSuccess == stat ) {
				for ( ; !cvFn.isDone(); cvFn.next() ) {
					if ( MS::kFailure == cvFn.translateBy( vector ) ) {
						cerr << "Error setting CV\n";
					}
				}
				cvFn.updateCurve();
			}

			MItSurfaceCV sCvFn( mdagPath, mComponent, true, &stat );
			if ( MS::kSuccess == stat ) {
				for ( ; !sCvFn.isDone(); sCvFn.nextRow() ) {
					for ( ; !sCvFn.isRowDone(); sCvFn.next() ) {
						if ( MS::kFailure == sCvFn.translateBy( vector ) ) {
							cerr << "Error setting CV\n";
						}
					}
				}
				sCvFn.updateSurface();
			}

			MItMeshVertex vtxFn( mdagPath, mComponent, &stat );
			if ( MS::kSuccess == stat ) {
				for ( ; !vtxFn.isDone(); vtxFn.next() ) {
					if ( MS::kFailure == vtxFn.translateBy( vector ) ) {
						cerr << "Error setting Vertex\n";
					}
				}
				vtxFn.updateSurface();
			}

		}
	}
	else {
		cerr << "Error creating selection list iterator\n";
	}
	return MS::kSuccess;
}


//
// The following routines are used to register/unregister
// the command we are creating within Maya
//
MStatus initializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");

	status = plugin.registerCommand( "translate", translate::creator );
	if (!status) {
		status.perror("registerCommand");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterCommand( "translate" );
	if (!status) {
		status.perror("deregisterCommand");
		return status;
	}

	return status;
}
