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

#ifndef _splitUVFty
#define _splitUVFty
// 
// File: splitUVFty.h
//
// Node Factory: splitUVFty
//
// Author: Lonnie Li
//
// Overview:
//
//		The splitUV factory implements the actual splitUV operation. It takes in
//		only two parameters:
//
//			1) A polygonal mesh
//			2) An array of selected UV Ids
//
//		The algorithm works as follows:
//
//			1) Parse the mesh for the selected UVs and collect:
//
//				(a) Number of faces sharing each UV
//					(stored as two arrays: face array, indexing/offset array)
//				(b) Associated vertex Id
//
//			2) Create (N-1) new UVIds for each selected UV, where N represents the number of faces
//			   sharing the UV.
//
//			3) Set each of the new UVs to the same 2D location on the UVmap.
//
//			3) Arbitrarily let the last face in the list of faces sharing this UV to keep the original
//			   UV.
//
//			4) Assign each other face one of the new UVIds.
//
//

#include "polyModifierFty.h"

// General Includes
//
#include <maya/MObject.h>
#include <maya/MIntArray.h>
#include <maya/MString.h>

class splitUVFty : public polyModifierFty
{

public:
				splitUVFty();
	virtual		~splitUVFty();

	void		setMesh( MObject mesh );
	void		setUVIds( MIntArray uvIds );

	// polyModifierFty inherited methods
	//
	MStatus		doIt();

private:
	// Mesh Node - Note: We only make use of this MObject during a single call of
	//					 the splitUV plugin. It is never maintained and used between
	//					 calls to the plugin as the MObject handle could be invalidated
	//					 between calls to the plugin.
	//
	MObject		fMesh;

	// Selected UVs
	//
	MIntArray	fSelUVs;
};

#endif
