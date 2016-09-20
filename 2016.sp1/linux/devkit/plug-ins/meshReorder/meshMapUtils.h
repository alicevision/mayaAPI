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

////////////////////////////////////////////////////////////////////////
//
// meshMapUtils.cpp
//
// Description:
//    Utility functions for the meshRemap and meshReorder commands
//       moveToolContext
//
////////////////////////////////////////////////////////////////////////
#ifndef _MESH_MAP_UTILS_H_
#define _MESH_MAP_UTILS_H_

#include "maya/MStatus.h"

class MIntArray;
class MFloatPointArray;
class MDagPath;
class MDagPathArray;
class MObjectArray;

class meshMapUtils
{
   public:
	static MStatus traverseFace(
			MDagPath& path,
			int faceIdx,
			int v0,
			int v1,
			MIntArray& faceTraversal,
			MIntArray& cvMapping,
			MIntArray& cVMappingInverse,
			MIntArray& newPolygonCounts,
			MIntArray& newPolygonConnects,
			MFloatPointArray& origVertices,
			MFloatPointArray& newVertices
	);

	static void  intersectArrays( MIntArray&, MIntArray &);
	static MStatus validateFaceSelection( MDagPathArray&, MObjectArray&, int *, MIntArray *);

};
#endif
