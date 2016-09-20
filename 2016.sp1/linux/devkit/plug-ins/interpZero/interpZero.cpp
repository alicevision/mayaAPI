//-
// ===========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================
//+

// This sample plugin illustrates the minimum amount of code required to write
// a new animation curve interpolator.  The plugin simply returns 0.0 for all
// evaluations.

#include <stdio.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MFnPlugin.h>
#include <maya/MPxAnimCurveInterpolator.h>

class MTime;

//
// Simple animation curve interpolation definition
//
class interpZero : public MPxAnimCurveInterpolator
{
  public:
						interpZero();
	virtual				~interpZero(); 

	/// Compute an interpolated keyframe value at the given time.
	virtual	double		evaluate(const MTime &val);

	static  void*		creator();
	
	// Type id
	static	MFnAnimCurve::TangentType id;
};

// Static initialization
MFnAnimCurve::TangentType interpZero::id = MFnAnimCurve::kTangentShared3;

// Constructor and destructor
interpZero::interpZero() {}
interpZero::~interpZero() {}

// Creator
void* interpZero::creator()
{
	return new interpZero();
}

//
// Evaluate the curve at the given time.
//
double interpZero::evaluate(const MTime &val)
{
	return 0.0;
}

MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "7.0", "Any");

	status = plugin.registerAnimCurveInterpolator("InterpZero", 
			interpZero::id, interpZero::creator);
	if (!status) {
		status.perror("registerAnimCurveInterpolator");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterAnimCurveInterpolator("InterpZero");
	if (!status) {
		status.perror("deregisterAnimCurveInterpolator");
		return status;
	}

	return status;
}
