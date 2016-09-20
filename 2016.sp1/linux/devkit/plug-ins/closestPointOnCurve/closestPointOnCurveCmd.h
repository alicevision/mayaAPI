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

// FILE: closestPointOnCurveCmd.h

// DESCRIPTION: -Class declaration for `closestPointOnCurve` MEL command.

//              -Please see readme.txt for full details.

// AUTHOR: QT

// REFERENCES: -This plugin's concept is based off of the "closestPointOnSurface" node.

//             -The MEL script AEclosestPointOnSurfaceTemplate.mel was referred to for

//              the AE template MEL script that accompanies the closestPointOnCurve node.

// LAST UPDATED: Oct. 13th, 2001.

// COMPILED AND TESTED ON: Maya 4.0 on Windows





// HEADER FILES:

#include <maya/MGlobal.h>

#include <maya/MPxCommand.h>

#include <maya/MArgList.h>

#include <maya/MSyntax.h>

#include <maya/MArgDatabase.h>

#include <maya/MDGModifier.h>

#include <maya/MString.h>

#include <maya/MPlug.h>

#include <maya/MSelectionList.h>

#include <maya/MDoubleArray.h>

#include <maya/MPoint.h>





// MAIN CLASS DECLARATION FOR THE MEL COMMAND:

class closestPointOnCurveCommand : public MPxCommand

{

   public:

      closestPointOnCurveCommand();

      virtual ~closestPointOnCurveCommand();

      static void *creator();

      static MSyntax newSyntax();

      bool isUndoable() const;

      MStatus doIt(const MArgList&);

      MStatus redoIt();

      MStatus undoIt();



   private:

	  bool queryFlagSet, inPositionFlagSet, positionFlagSet, normalFlagSet, tangentFlagSet, paramUFlagSet, distanceFlagSet;

      MString closestPointOnCurveNodeName;

      MPoint inPosition;

      MSelectionList sList;

};

