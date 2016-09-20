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

#include <maya/MFnAnimCurve.h>
#include <maya/MObject.h>
#include <maya/MPxAnimCurveInterpolator.h>
#include <maya/MTime.h> 
 
//
// Halfwise linear curve interpolation definition
//
class interpHalf : public MPxAnimCurveInterpolator
{
  public:
						interpHalf();
	virtual				~interpHalf(); 

	// Initialize the interpolator.
	virtual	void		initialize(const MObject &animCurve, unsigned int interval);

	// Compute an interpolated keyframe value at the given time.
	virtual	double		evaluate(const MTime &val);

	static  void*		creator();
	
	// Type id
	static	MFnAnimCurve::TangentType id;

  private:
	double 				sTime;
	double				range;
	double 				beforeVal;
	double 				afterVal;
};
