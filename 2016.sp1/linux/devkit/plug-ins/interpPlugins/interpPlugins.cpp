//-
// ===========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================
//+

// This sample illustrates how to register multiple animation curve interpolator
// plugins into the same library.  The InterpFlag and InterpHalf plugins are
// both registered.

#include <maya/MFnPlugin.h>
#include <maya/MPxAnimCurveInterpolator.h>
#include "interpFlat.h"
#include "interpHalf.h"

//
// Initialize the InterpFlat and InterpHalf plugins.
//
MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "7.0", "Any");

	status = plugin.registerAnimCurveInterpolator( "InterpFlat", 
			interpFlat::id, interpFlat::creator);
	if (!status) {
		status.perror("registerAnimCurveInterpolator");
		return status;
	}

  /*
    The interpHalf plugin looks at the tangent types of the neighboring keyframes, 
    and if they're also interpHalf types then it computes a value at the keyframe 
    which is an average value between the two intervals. This mimics the desired 
    behavior of wanting to use the keyframe values as control points on a nurbs 
    curve where the curve does not pass through the control points. To help support 
    this behavior, the tangent types need to lock themselves so they don't 
    automatically change to some other tangent type as the tangent types of the 
    neighboring keyframes are changed. The kLockType flag is used to select this behavior. 
    
    In addition, the values at the keyframes need to be computed instead of just using 
    the stored keyframe value. This enables the curve to not pass through the keyframe 
    value. The kEvaluateAtKey flag is used to select this behavior.
  */
	status = plugin.registerAnimCurveInterpolator( "InterpHalf", 
			interpHalf::id, interpHalf::creator, MPxAnimCurveInterpolator::kEvaluateAtKey | MPxAnimCurveInterpolator::kLockType);
	if (!status) {
		status.perror("registerAnimCurveInterpolator");
		return status;
	}

	return status;
}

//
// Uninitialize the InterpFlat and InterpHalf plugins.
//
MStatus uninitializePlugin( MObject obj)
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterAnimCurveInterpolator( "InterpHalf" );
	if (!status) {
		status.perror("deregisterAnimCurveInterpolator");
		return status;
	}

	status = plugin.deregisterAnimCurveInterpolator( "InterpFlat" );
	if (!status) {
		status.perror("deregisterAnimCurveInterpolator");
		return status;
	}

	return status;
}
