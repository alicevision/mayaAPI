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


#ifndef _apiMeshCreator
#define _apiMeshCreator

///////////////////////////////////////////////////////////////////////////////
//
// apiMeshCreator.h
//
// A DG node that takes a maya mesh as input and outputs apiMeshData.
// If there is no input then the node creates a cube or sphere
// depending on what the shapeType attribute is set to.
//
////////////////////////////////////////////////////////////////////////////////

#include <maya/MPxNode.h>
#include <maya/MTypeId.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MVectorArray.h>
#include <maya/MDoubleArray.h> 

class apiMeshGeomUV; 

class apiMeshCreator : public MPxNode
{
public:
	apiMeshCreator();
	virtual ~apiMeshCreator(); 

    //////////////////////////////////////////////////////////
	//
	// Overrides
	//
    //////////////////////////////////////////////////////////

    virtual MStatus   		compute( const MPlug& plug, MDataBlock& data );

    //////////////////////////////////////////////////////////
	//
	// Helper methods
	//
    //////////////////////////////////////////////////////////

	static  void *          creator();
	static  MStatus         initialize();

	MStatus					computeInputMesh( const MPlug& plug,
											  MDataBlock& datablock,
											  MPointArray& vertices,
											  MIntArray& counts,
											  MIntArray& connects,
											  MVectorArray& normals,
											  apiMeshGeomUV &uvs ); 

	void					buildCube( double cube_size,
									   MPointArray& pa,
									   MIntArray& faceCounts,
									   MIntArray& faceConnects,
									   MVectorArray& normals,
									   apiMeshGeomUV& uvs ); 

	void					buildSphere( double radius,
										 int divisions,
										 MPointArray& pa,
										 MIntArray& faceCounts,
										 MIntArray& faceConnects,
										 MVectorArray& normals, 
										 apiMeshGeomUV& uvs ); 

public:
    //////////////////////////////////////////////////////////
    //
    // Attributes
    //
    //////////////////////////////////////////////////////////
    static	MObject			size;
	static	MObject			shapeType;
    static	MObject			inputMesh;
	static  MObject         outputSurface;
        
public: 
	static	MTypeId		id;
};

#endif /* _apiMeshCreator */
