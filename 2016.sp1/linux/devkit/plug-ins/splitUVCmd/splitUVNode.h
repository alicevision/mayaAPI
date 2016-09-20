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

#ifndef _splitUVNode
#define _splitUVNode
// 
// File: splitUVNode.h
//
// Dependency Graph Node: splitUVNode
//
// Authors: Lonnie Li, Jeyprakash Michaelraj
//

#include "polyModifierNode.h"
#include "splitUVFty.h"

// General Includes
//
#include <maya/MTypeId.h>
 
class splitUVNode : public polyModifierNode
{
public:
						splitUVNode();
	virtual				~splitUVNode(); 

	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );

	static  void*		creator();
	static  MStatus		initialize();

public:

	// There needs to be a MObject handle declared for each attribute that
	// the node will have.  These handles are needed for getting and setting
	// the values later.
	//
	// polyModifierNode has predefined the standard inMesh and outMesh attributes.
	//
	// We define an input attribute for our UV list input
	//
	static  MObject		uvList;

	// The typeid is a unique 32bit indentifier that describes this node.
	// It is used to save and retrieve nodes of this type from the binary
	// file format.  If it is not unique, it will cause file IO problems.
	//
	static	MTypeId		id;

	splitUVFty			fSplitUVFactory;
};

#endif
