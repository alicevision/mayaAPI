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

// PLUGIN NAME: pointOnMeshInfo v1.0

// FILE: getPointAndNormal.h

// DESCRIPTION: -Utility function declaration, used by both the command and node.

//              -Please see readme.txt for full details.

// AUTHOR: QT

// REFERENCES: -This plugin is based off of the `pointOnSurface` MEL command and "pointOnSurfaceInfo" node.

//             -The pointOnSubdNode.cpp plugin example from the Maya API Devkit was also used for reference.

//             -The MEL script AEpointOnSurfaceInfoTemplate.mel was referred to for the AE template MEL script

//              that accompanies the pointOnMeshInfo node.

// LAST UPDATED: Oct. 11th, 2001.

// COMPILED AND TESTED ON: Maya 4.0 on Windows





#ifndef _getPointAndNormal_h

#define _getPointAndNormal_h





// MAYA HEADER FILES:

#include <maya/MObject.h>

#include <maya/MDagPath.h>

#include <maya/MItMeshPolygon.h>

#include <maya/MFnMesh.h>

#include <maya/MFloatArray.h>

#include <maya/MPoint.h>

#include <maya/MVector.h>





// FUNCTION DECLARATION:

void getPointAndNormal(MDagPath meshDagPath, int faceIndex, bool relative, double parameterU, double parameterV, MPoint &point, MVector &normal, MObject theMesh=MObject::kNullObj);





#endif

