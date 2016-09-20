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

// DISCLAIMER: THIS PLUGIN IS PROVIDED AS IS.  IT IS NOT SUPPORTED BY

//            AUTODESK, SO PLEASE USE AND MODIFY AT YOUR OWN RISK.

//

// PLUGIN NAME: closestPointOnCurve v1.0

// FILE: closestTangentUAndDistance.cpp

// DESCRIPTION: -Utility function definition, used by both the command and node.

//              -Please see readme.txt for full details.

// AUTHOR: QT

// REFERENCES: -This plugin's concept is based off of the "closestPointOnSurface" node.

//             -The MEL script AEclosestPointOnSurfaceTemplate.mel was referred to for

//              the AE template MEL script that accompanies the closestPointOnCurve node.

// LAST UPDATED: Oct. 13th, 2001.

// COMPILED AND TESTED ON: Maya 4.0 on Windows





// HEADER FILE:

#include "closestTangentUAndDistance.h"





// FUNCTION WHICH TAKES A SPECIFIED INPUT CURVE AND A WORLDSPACE POSITION, AND FINDS THE CLOSEST POSITION, NORMAL, TANGENT, PARAMETER-U AND CLOSEST-DISTANCE FROM THE CURVE:

void closestTangentUAndDistance(MDagPath curveDagPath, MPoint inPosition, MPoint &position, MVector &normal, MVector &tangent, double &paramU, double &distance, MObject theCurve)

{

   // FIND THE CLOSEST POSITION AND PARAMETER-U FROM THE INPUT POSITION:

   MFnNurbsCurve curveFn(curveDagPath);

   if (theCurve!=MObject::kNullObj)

      curveFn.setObject(theCurve);

   position = curveFn.closestPoint(inPosition, &paramU, 0.0, MSpace::kWorld);



   // FIND THE NORMAL, TANGENT AND DISTANCE FROM THE CLOSEST POINT:

   normal = curveFn.normal(paramU, MSpace::kWorld);

   tangent = curveFn.tangent(paramU, MSpace::kWorld);

   distance = curveFn.distanceToPoint(inPosition, MSpace::kWorld);

}

