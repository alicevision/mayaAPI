#ifndef _TESTNOBJECTNODE_
#define _TESTNOBJECTNODE_

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
#include <maya/MnCloth.h>

class testNobjectNode : public MPxNode
{
public:
	virtual MStatus compute(const MPlug &plug, MDataBlock &dataBlock);
    testNobjectNode();

	static void *creator() 
	{ 
		return new testNobjectNode; 
	}

	static MStatus initialize();
	static const MTypeId id;

	// attributes
	static MObject currentState;
	static MObject startState;
	static MObject nextState;
	static MObject currentTime;
	static MObject inputGeom;
	static MObject outputGeom;

	MnCloth  fNObject;

};
 
#endif

