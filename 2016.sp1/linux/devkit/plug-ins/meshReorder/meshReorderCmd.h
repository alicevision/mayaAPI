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
// meshReorderCmd.h
// 
// Description:
// 	Command to reindex a polygon mesh based on a user-defined starting face
//
// Usage:
//      meshOrder mesh.vtx[5] mesh.vtx[23] mesh.vtx[9] 
//
//		The vertices must all be from a common face, be adjacent, and be in
//		order, clockwise or counter-clockwise, around that face. For
//		example, if the face has five vertices A, B, C, D and E, in
//		clockwise order, then you could specify ABC, BCD, DEA, AED, DCB,
//		etc. But ABD would be invalid because B and D are not adjacent, and
//		BCA would be invalid because they are not in order.
//
// See Also:
//      meshReorderTool.cpp : this context allows you to interactively pick vertices and invoke this command
// 
////////////////////////////////////////////////////////////////////////

#ifndef _MESH_REORDER_CMD_H_
#define _MESH_REORDER_CMD_H_

#include <maya/MArgList.h>
#include <maya/MPxCommand.h>
#include <maya/MSelectionList.h>
#include <maya/MIntArray.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MDagPath.h>
#include <maya/MFloatPointArray.h>
#include <maya/MStringArray.h>
#include <maya/MFnMesh.h>
#include <maya/MDGModifier.h>

class MColorArray;
class MFloatArray;

// MAIN CLASS DECLARATION FOR THE MEL COMMAND:
class meshReorderCommand : public MPxCommand
{
   private:
	int                 fFaceIdxSrc;
	MIntArray           fFaceVtxSrc;
	MDagPath            fDagPathSrc;

	// For undo
	MFloatPointArray    fVertices;
	MIntArray           fPolygonCounts;
	MIntArray           fPolygonConnects;
	MIntArray           fCVMapping;
	MIntArray           fCVMappingInverse;
	MColorArray        *fColorArrays;
	MIntArray          *fColorIdsArrays;

	MStatus 			parseArgs(const MArgList&);

	void                collectColorsUVs(MFnMesh &theMesh, bool isUndo);
	void                assignColorsUVs(MFnMesh &theMesh, MIntArray& colorMapping, MIntArray& uvMapping, bool isUndo);
	void                resetColorsUVsMemory();

	// Temp variables used only in collectColorsUVs() and assignColorsUVs()
	// for Colors
	MStringArray        fColorSetNames;
	bool               *fClampedArray;
	MFnMesh::MColorRepresentation *fRepArray;
	MColorArray        *fVertexColorArrays;
	// for UVs
	MStringArray        fUVSetNames;
	MFloatArray        *fUArrays;
	MFloatArray        *fVArrays;
	MIntArray          *fUVIdsArrays;

   public:
	meshReorderCommand();
	virtual ~meshReorderCommand();

	static void* creator();
	MStatus doIt(const MArgList&);
	MStatus redoIt();

	bool    isUndoable() const { return true; }
	MStatus undoIt();

};

#endif

