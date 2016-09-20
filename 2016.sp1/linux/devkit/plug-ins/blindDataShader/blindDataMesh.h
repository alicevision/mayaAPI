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

class MPlug;
class MStatus;
class MDataBlock;

#include <maya/MPxNode.h>
#include <maya/MObject.h>

class blindDataMesh : public MPxNode
{
public:
	// Standard Node functions
	//
	virtual MStatus compute(const MPlug& plug, MDataBlock& data);
	static void* creator();
	static MStatus initialize();

	// OutputMesh Plug. It will contain the mesh plane with blind data.
	// This is the object that will be shaded by the blindDataShader.
	//
	static MObject outputMesh;

	// Random number Generator seed
	//
	static MObject seed;

	// Node type ID. This is a unique identifier used to recognize
	// the node class.
	//
	static MTypeId id;

protected:
	// This function creates a plane on the X-Z plane with random 
	// height values.
	//
	MObject createMesh( long seed, MObject& outData, MStatus& stat );

	// This function adds the global blind data node for the color
	// blind data. It returns the blind data ID.
	//
	MStatus setMeshBlindData( MObject& mesh );
};


