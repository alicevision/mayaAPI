#ifndef _TESTNUCLEUSNODE_
#define _TESTNUCLEUSNODE_

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
#include <maya/MnSolver.h>

class testNsolverNode : public MPxNode
{
public:
	testNsolverNode();
	virtual MStatus compute(const MPlug &plug, MDataBlock &dataBlock);

	static void *creator() 
	{ 
		return new testNsolverNode; 
	}

	static MStatus initialize();
	static const MTypeId id;

// member variables:
	static MObject startState;
	static MObject currentState;
	static MObject nextState;
	static MObject currentTime;
	MnSolver solver;

};
 
#endif

