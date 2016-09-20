#ifndef _TESTNPASSIVENODE_
#define _TESTNPASSIVENODE_

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

#include <maya/MPxNode.h>
#include <maya/MnRigid.h>

class testNpassiveNode : public MPxNode
{
public:
	virtual MStatus compute(const MPlug &plug, MDataBlock &dataBlock);
    testNpassiveNode();

	static void *creator() 
	{ 
		return new testNpassiveNode; 
	}

	static MStatus initialize();
	static const MTypeId id;

	// attributes
	static MObject currentState;
	static MObject startState;
	static MObject currentTime;
	static MObject inputGeom;

	MnRigid  fNObject;

};
 
#endif

