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

// FILE: pointOnMeshInfoNode.h

// DESCRIPTION: -Class declaration for "pointOnMeshInfo" node.

//              -Please see readme.txt for full details.

// AUTHOR: QT

// REFERENCES: -This plugin is based off of the `pointOnSurface` MEL command and "pointOnSurfaceInfo" node.

//             -The pointOnSubdNode.cpp plugin example from the Maya API Devkit was also used for reference.

//             -The MEL script AEpointOnSurfaceInfoTemplate.mel was referred to for the AE template MEL script

//              that accompanies the pointOnMeshInfo node.

// LAST UPDATED: Oct. 11th, 2001.

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

class pointOnMeshInfoNode : public MPxNode

{

   public:

      // CLASS METHOD DECLARATIONS:

      pointOnMeshInfoNode();

      virtual ~pointOnMeshInfoNode();

      static void *creator();

      static MStatus initialize();

      virtual MStatus compute(const MPlug &plug, MDataBlock &data);



      // CLASS DATA DECLARATIONS:

      static MTypeId id;

      static MObject aInMesh;

      static MObject aFaceIndex;

	  static MObject aRelative;

      static MObject aParameterU;

      static MObject aParameterV;

      static MObject aPosition;

      static MObject aPositionX;

      static MObject aPositionY;

      static MObject aPositionZ;

      static MObject aNormal;

      static MObject aNormalX;

      static MObject aNormalY;

      static MObject aNormalZ;

};

