//-
// ===========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================
//+

// This sample plugin is slightly more complicated that the InterpZero plugin in
// that it makes use of the setSpan() method to perform some setup before evaluation
// begins.  In this case, it stores the starting keyframe value so it can be
// returned for all evaluations.

#include <maya/MFnAnimCurve.h>
#include <maya/MPxAnimCurveInterpolator.h>
#include <maya/MTime.h>
#include "interpFlat.h"
 
// Static initialization
MFnAnimCurve::TangentType interpFlat::id = MFnAnimCurve::kTangentShared4;

// Constructor and destructor
interpFlat::interpFlat() {}
interpFlat::~interpFlat() {}

// Creator
void* interpFlat::creator()
{
	return new interpFlat();
}

//
// The span is being setup for evaluation.  Store the starting
// keyframe value.
//
void interpFlat::initialize(const MObject &animCurve, unsigned int interval)
{
	fObj = animCurve;
	fInterval = interval;
}

//
// Evaluate the curve at the given time.
//
double interpFlat::evaluate(const MTime &val)
{
	// Just return the keyframe value at the start of the interval.
	MFnAnimCurve curveFn(fObj);
	return curveFn.value(fInterval);
}
