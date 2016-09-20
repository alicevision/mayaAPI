#ifndef _cgfxVector_h_
#define _cgfxVector_h_
//
// Copyright (C) 2002 NVIDIA 
// 
// File: cgfxVector.h
//
// Dependency Graph Node: cgfxVector
//
// Description:
//	The cgfxVector node is used to convert a vector in the scene to
//	world coordinates.  The inputs are a vector in local coordinates,
//	a flag indicating whether the vector is a position or a direction,
//	and a matrix that will transoform the vector to world coordinates.
//	This matrix is generally the worldInverseMatrix of the vector.
//
// Author: Jim Atkinson
//
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
#include "cgfxShaderCommon.h"

#include <maya/MPxNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MTypeId.h> 

class cgfxVector : public MPxNode
{
public:
						cgfxVector();
	virtual				~cgfxVector(); 

	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );

	// Create the node ...
	//
	static  void*		creator();

	// ... and initialize it.
	//
	static  MStatus		initialize();

public:

	// The typeid is a unique 32bit indentifier that describes this node.
	// It is used to save and retrieve nodes of this type from the binary
	// file format.  If it is not unique, it will cause file IO problems.
	//
    static  MTypeId sId;

	// There needs to be a MObject handle declared for each attribute that
	// the node will have.  These handles are needed for getting and setting
	// the values later.
	//
	// Input vector attribute
	//
	static  MObject	sVector;

	static	MObject sVectorX;
	static	MObject sVectorY;
	static	MObject sVectorZ;

	// Input position/direction flag.  If isDirection is set then
	// the vector represents a direction and the W coordinate is
	// 0.0.  If it is not set then W is 1.0.
	//
	static	MObject	sIsDirection;

	// Input matrix attribute
	//
	static	MObject sMatrix;

	// Output world coordinate vector attribute
	//
	static	MObject	sWorldVector;
	static	MObject	sWorldVectorX;
	static	MObject	sWorldVectorY;
	static	MObject	sWorldVectorZ;
	static	MObject	sWorldVectorW;
};

#endif /* _cgfxVector_h_ */
