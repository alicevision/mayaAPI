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

#ifndef _splitUVCmd
#define _splitUVCmd
// 
// File: splitUVCmd.h
//
// MEL Command: splitUV
//
// Author: Lonnie Li
//
// Overview:
//
//		The purpose of the splitUV command is to unshare (split) any selected UVs
//		on a given object.
//
// How it works:
//
//		This command is based on the polyModifierCmd. It relies on the polyModifierCmd
//		to manage "how" the effects of the splitUV operation are applied (ie. directly
//		on the mesh or through a modifier node). See polyModifierCmd.h for more details
//
//		To understand the algorithm behind the splitUV operation, refer to splitUVFty.h.
//
// Limitations:
//
//		(1) Can only operate on a single mesh at a given time. If there are more than one
//			mesh with selected UVs, only the first mesh found in the selection list is
//			operated on.
//

#include "polyModifierCmd.h"
#include "splitUVFty.h"

// Function Sets
//
#include <maya/MFnComponentListData.h>

// Forward Class Declarations
//
class MArgList;

class splitUV : public polyModifierCmd
{

public:
	////////////////////
	// Public Methods //
	////////////////////

				splitUV();
	virtual		~splitUV();

	static		void* creator();

	bool		isUndoable() const;

	MStatus		doIt( const MArgList& );
	MStatus		redoIt();
	MStatus		undoIt();

	/////////////////////////////
	// polyModifierCmd Methods //
	/////////////////////////////

	MStatus		initModifierNode( MObject modifierNode );
	MStatus		directModifier( MObject mesh );

private:
	/////////////////////
	// Private Methods //
	/////////////////////

	bool		validateUVs();
	MStatus		pruneUVs( MIntArray& validUVIndices );

	//////////////////
	// Private Data //
	//////////////////

	// Selected UVs
	//
	// Note: The MObject, fComponentList, is only ever accessed on a single call to the plugin.
	//		 It is never accessed between calls and is stored on the class for access in the
	//		 overriden initModifierNode() method.
	//
	MObject						fComponentList;
	MIntArray					fSelUVs;

	// splitUV Factory
	//
	splitUVFty				fSplitUVFactory;
};

#endif
