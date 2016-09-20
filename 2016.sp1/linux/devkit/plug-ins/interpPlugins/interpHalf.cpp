//-
// ===========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================
//+

// This sample plugin shows how to allow the interpolator to perform its own
// evaluations at the keyframe time locations, allowing the curve to not pass
// through the keyframe values.  In this example, the curve is linearly
// interpolated to values halfway between the keyframe values and the keyframe
// values in the neighboring spans.

#include <stdio.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MObject.h>
#include <maya/MPxAnimCurveInterpolator.h>
#include <maya/MTime.h> 
#include <maya/MTypeId.h>
#include "interpHalf.h"
 

// Static initialization
// Please note, if using a custom range, for eg 72 through 79, and setting up
// enums for tangent types, please allocate them thusly:
// MFnAnimCurve::TangentType interpHalf::id = (MFnAnimCurve::TangentType) (72);

MFnAnimCurve::TangentType interpHalf::id = MFnAnimCurve::kTangentShared1;

// Constructor and destructor
interpHalf::interpHalf()
{
	sTime = 0.0;
	range = 0.0;
}
interpHalf::~interpHalf() {}

// Creator
void* interpHalf::creator()
{
	return new interpHalf();
}

//
// The span is being setup for evaluation.  Just store the start time
// and time range over the span.  
//
void interpHalf::initialize(const MObject &animCurve, unsigned int interval)
{
	// Determine whether the span before and span after this span has this
	// tangent type.  If so, begin the interpolation halfway between the
	// starting keyframe value of this span and the starting keyframe value
	// of the previous span.  Otherwise, just begin this span at the starting
	// key value.  Likewise for the ending key.

	// Determine the starting time and value.
	MFnAnimCurve curveFn(animCurve);
	sTime = curveFn.time(interval).as(MTime::kSeconds);
	bool isBeforeHalf = false;
	int beforeInd = interval - 1;
	if (beforeInd < 0)
		beforeInd = 0;
	else
	{
		// Check the previous span's tangent type to see if it is this type.
		MFnAnimCurve::TangentType beforeType = curveFn.outTangentType(beforeInd);
		if (beforeType == MFnAnimCurve::kTangentShared1)
			isBeforeHalf = true;
	}
	if (isBeforeHalf)
		beforeVal = 0.5 * curveFn.value(beforeInd) + 0.5 * curveFn.value(interval);
	else
		beforeVal = curveFn.value(interval);
	
	// Determine the ending value based on its tangent type.
	int afterInd = interval + 1;
	MFnAnimCurve::TangentType afterType = curveFn.outTangentType(afterInd);
	if (afterType == MFnAnimCurve::kTangentShared1)
		afterVal = 0.5 * curveFn.value(interval) + 0.5 * curveFn.value(afterInd);
	else
		afterVal = curveFn.value(afterInd);

	// Set the time range.
	range = curveFn.time(afterInd).as(MTime::kSeconds) - sTime;
}

//
// Evaluate the curve at the given time.
//
double interpHalf::evaluate(const MTime &val)
{
	// Interpolate the before and after values.
	double sec = val.as(MTime::kSeconds);
	double alpha = (sec-sTime) / range;
	return (1.0 - alpha) * beforeVal + alpha * afterVal;
}
