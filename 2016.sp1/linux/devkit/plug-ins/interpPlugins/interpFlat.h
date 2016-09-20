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
#include <maya/MObject.h> 
#include <maya/MPxAnimCurveInterpolator.h>
#include <maya/MTime.h> 
 
//
// Flat animation curve interpolation definition
//
class interpFlat : public MPxAnimCurveInterpolator
{
  public:
						interpFlat();
	virtual				~interpFlat(); 

	// Initialize the interpolator.
	virtual	void		initialize(const MObject &animCurve, unsigned int interval);

	// Compute an interpolated keyframe value at the given time.
	virtual	double		evaluate(const MTime &val);

	static  void*		creator();
	
	// Type id
	static	MFnAnimCurve::TangentType id;

  private:
	MObject				fObj;
	unsigned int		fInterval;
};
