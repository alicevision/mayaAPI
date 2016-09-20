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
// meshRemapCmd.h
//
// Description:
//      Transfer the vertex/edge/face information from one mesh onto another.
//      The meshes are traversed based on three user supplied CVs, for each
//      mesh.
//
//      The CV/edge/face information is mapped based on the traversal
//      order within each mesh.
//
// Usage:
//              meshRemap srcMesh.vtx[5] srcMesh.vtx[23] srcMesh.vtx[9] dstMesh.vtx[13] dstMesh.vtx[16] dstMesh.vtx[17]
//
//		The vertices for each mesh must all be from a common face, be
//		adjacent, and be in order, clockwise or counter-clockwise, around
//		that face. For example, if the face has five vertices A, B, C, D
//		and E, in clockwise order, then you could specify ABC, BCD, DEA,
//		AED, DCB, etc. But ABD would be invalid because B and D are not
//		adjacent, and BCA would be invalid because they are not in order.
//
// See Also:
//      meshRemapTool.cpp : this context allows you to interactively pick
//      vertices and invoke this command
//
////////////////////////////////////////////////////////////////////////

#ifndef _MESH_REMAP_CMD_H_
#define _MESH_REMAP_CMD_H_

#include <maya/MPxCommand.h>
#include <maya/MDagPath.h>
#include <maya/MSelectionList.h>
#include <maya/MIntArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MStringArray.h>
#include <maya/MFnMesh.h>

class MArgList;
class MFloatArray;
class MColorArray;

// MAIN CLASS DECLARATION FOR THE MEL COMMAND:
class meshRemapCommand : public MPxCommand
{
	private:
 		int					fFaceIdxSrc;
 		int					fFaceIdxDst;

 		MIntArray			fFaceVtxSrc;
 		MIntArray			fFaceVtxDst;		

		MDagPath 			fDagPathSrc;
		MDagPath 			fDagPathDst;

		// For undo
		MFloatPointArray		fVertices;
		MIntArray 			fPolygonCounts;
		MIntArray 			fPolygonConnects;
		// For Colors undo
		MStringArray                    fColorSetNames;
		MColorArray                    *fColorArrays;
		MFnMesh::MColorRepresentation  *fRepArray;
		bool                           *fClampedArray;
		// For UVs undo
		MStringArray                    fUVSetNames;
		MFloatArray                    *fUArrays;
		MFloatArray                    *fVArrays;
		MIntArray                      *fUVCountsArrays;
		MIntArray                      *fUVIdsArrays;

	MStatus parseArgs(const MArgList&);

   public:
      meshRemapCommand();
      virtual ~meshRemapCommand();
      static void* creator();

      MStatus doIt(const MArgList&);
      MStatus redoIt();

      bool    isUndoable() const { return true; }
      MStatus undoIt();

      void    reset();
};

#endif
