#ifndef _meshOpNode
#define _meshOpNode

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

#include "polyModifierNode.h"
#include "meshOpFty.h"

// General Includes
//
#include <maya/MTypeId.h>
 
class meshOpNode : public polyModifierNode
{
public:
						meshOpNode();
	virtual				~meshOpNode(); 

	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );

	static  void*		creator();
	static  MStatus		initialize();

public:

	// There needs to be a MObject handle declared for each attribute that
	// the node will have.  These handles are needed for getting and setting
	// the values later.
	//
	// The polyModifierNode class has predefined the standard inMesh 
	// and outMesh attributes. We define an input attribute for the 
	// component list and the operation type
	//
	static  MObject		cpList;
	static  MObject		opType;

	// The typeid is a unique 32bit indentifier that describes this node.
	// It is used to save and retrieve nodes of this type from the binary
	// file format.  If it is not unique, it will cause file IO problems.
	//
	static	MTypeId		id;

	meshOpFty			fmeshOpFactory;
};

#endif
