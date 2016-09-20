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

// PLUGIN NAME: pointOnMeshInfo v1.0

// FILE: pointOnMeshCmd.h

// DESCRIPTION: -Class declaration for `pointOnMesh` MEL command.

//              -Please see readme.txt for full details.

// AUTHOR: QT

// REFERENCES: -This plugin is based off of the `pointOnSurface` MEL command and "pointOnSurfaceInfo" node.

//             -The pointOnSubdNode.cpp plugin example from the Maya API Devkit was also used for reference.

//             -The MEL script AEpointOnSurfaceInfoTemplate.mel was referred to for the AE template MEL script

//              that accompanies the pointOnMeshInfo node.

// LAST UPDATED: Oct. 11th, 2001.

// COMPILED AND TESTED ON: Maya 4.0 on Windows





// HEADER FILES:

#include <maya/MGlobal.h>

#include <maya/MPxCommand.h>

#include <maya/MArgList.h>

#include <maya/MString.h>

#include <maya/MPlug.h>

#include <maya/MSelectionList.h>

#include <maya/MDoubleArray.h>





// MAIN CLASS DECLARATION FOR THE MEL COMMAND:

class pointOnMeshCommand : public MPxCommand

{

   public:

      pointOnMeshCommand();

      virtual ~pointOnMeshCommand();

      static void* creator();

      bool isUndoable() const;

      MStatus doIt(const MArgList&);

      MStatus redoIt();

      MStatus undoIt();



   private:

      bool nodeCreated, positionSpecified, normalSpecified, faceIndexSpecified, relativeSpecified, parameterUSpecified, parameterVSpecified;

      MString meshNodeName, pointOnMeshInfoName;

      int faceIndex;

	  bool relative;

      double parameterU, parameterV;

};

