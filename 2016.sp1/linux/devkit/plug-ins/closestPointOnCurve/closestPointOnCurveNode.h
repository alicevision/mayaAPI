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

//

// DISCLAIMER: THIS PLUGIN IS PROVIDED AS IS.  IT IS NOT SUPPORTED BY

//            AUTODESK, SO PLEASE USE AND MODIFY AT YOUR OWN RISK.

//

// PLUGIN NAME: closestPointOnCurve v1.0

// FILE: closestPointOnCurveNode.h

// DESCRIPTION: -Class declaration for "closestPointOnCurve" node.

//              -Please see readme.txt for full details.

// AUTHOR: QT

// REFERENCES: -This plugin's concept is based off of the "closestPointOnSurface" node.

//             -The MEL script AEclosestPointOnSurfaceTemplate.mel was referred to for

//              the AE template MEL script that accompanies the closestPointOnCurve node.

// LAST UPDATED: Oct. 13th, 2001.

// COMPILED AND TESTED ON: Maya 4.0 on Windows





// MAYA HEADER FILES:

#include <maya/MPxNode.h>

#include <maya/MTypeId.h>

#include <maya/MDataBlock.h>

#include <maya/MDataHandle.h>

#include <maya/MFnTypedAttribute.h>

#include <maya/MFnNumericAttribute.h>

#include <maya/MDagPath.h>

#include <maya/MPlug.h>

#include <maya/MPoint.h>

#include <maya/MVector.h>





// MAIN CLASS DECLARATION FOR THE CUSTOM NODE:

class closestPointOnCurveNode : public MPxNode

{

   public:

      // CLASS METHOD DECLARATIONS:

      closestPointOnCurveNode();

      virtual ~closestPointOnCurveNode();

      static void *creator();

      static MStatus initialize();

      virtual MStatus compute(const MPlug &plug, MDataBlock &data);



      // CLASS DATA DECLARATIONS:

      static MTypeId id;

      static MObject aInCurve;

      static MObject aInPosition;

      static MObject aInPositionX;

      static MObject aInPositionY;

      static MObject aInPositionZ;

      static MObject aPosition;

      static MObject aPositionX;

      static MObject aPositionY;

      static MObject aPositionZ;

      static MObject aNormal;

      static MObject aNormalX;

      static MObject aNormalY;

      static MObject aNormalZ;

      static MObject aTangent;

      static MObject aTangentX;

      static MObject aTangentY;

      static MObject aTangentZ;

      static MObject aParamU;

	  static MObject aDistance;

};

