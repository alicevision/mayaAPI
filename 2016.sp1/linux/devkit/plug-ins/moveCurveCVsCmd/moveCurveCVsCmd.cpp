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
//		Iterates over selected CVs - sets CV to world 0.0
//
////////////////////////////////////////////////////////////////////////

#include <maya/MIOStream.h>
#include <maya/MPxCommand.h>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MPoint.h>
#include <maya/MDagPath.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MItCurveCV.h>

class moveCurveCVs : public MPxCommand
{
public:
					moveCurveCVs() {};
	virtual			~moveCurveCVs(); 

	MStatus			doIt( const MArgList& args );
	static void*	creator();
};

moveCurveCVs::~moveCurveCVs() {}

void* moveCurveCVs::creator()
{
	return new moveCurveCVs();
}

MStatus moveCurveCVs::doIt( const MArgList& )
{
	MStatus stat;

	// Create a selection list iterator
	// set the filter moveCurveCVsComponents
	//
	MSelectionList list;
	MGlobal::getActiveSelectionList( list );
	MItSelectionList * iter = new MItSelectionList( list,
													MFn::kCurveCVComponent,
													&stat );

	if ( MS::kSuccess == stat ) {
		MDagPath 	mdagPath;		// Item dag path
		MObject  	mComponent;		// Current component
		double 		coord[4] = {0.0,0.0,0.0,0.0};
		MPoint 		point( coord );

		// Iterate over all selected curves
		//
		for ( ; !iter->isDone(); iter->next() ) 
		{
			// Create a function set to operate on selected curves
			//
			iter->getDagPath( mdagPath, mComponent );
			MItCurveCV fnmoveCurveCVs( mdagPath, mComponent, &stat );

			if ( MS::kSuccess == stat ) {
				for ( ; !fnmoveCurveCVs.isDone(); fnmoveCurveCVs.next() )
				{
					fnmoveCurveCVs.setPosition( point, MSpace::kWorld );
				}
				fnmoveCurveCVs.updateCurve();
			}
			else {
				cerr << "Function set error\n";
			}
		}
	}
	else {
		cerr << "Error creating selection list iterator\n";
	}
	return stat;
}


MStatus initializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");

	status = plugin.registerCommand( "moveCurveCVs", moveCurveCVs::creator );
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

	status = plugin.deregisterCommand( "moveCurveCVs" );
	if (!status) {
		status.perror("deregisterCommand");
		return status;
	}

	return status;
}
