#ifndef _blindDataShader
#define _blindDataShader

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

//
// ***************************************************************************
//
// How to use:
// This plugin is an example plug-in. Its purpose is to show how to use
// custom blind data in the hardware shader node. To try out the plugin,
// you can run the provided mel script: "blindDataShader.mel" after manually
// loading the plugin into memory.
//
// The mel script creates a polygonal grid-plane with random heights.
// Each vertex has an associated color value that is stored as three
// independant double blind data values: "red", "green" and "blue".
// The height of the vertices is used to calculate the color. All this
// work is done in the blindDataMesh class (blindDataMesh.h/.cpp).
//
// The mel script then associates the mesh with a new blindDataShader node
// (blindDataShader.h/.cpp) This node gets from the blind data of the mesh
// the vertex color values and renders the mesh.
//
// To get the blind data associated with the vertex IDs, you must do the
// following:
// 1. In your MPxHwShaderNode-extended class, you must overwrite the virtual
//    function: "provideVertexIDs" and return true. The "geometry" function
//    will receive the component IDs of the vertices in the "vertexIDs" formal
//    parameter. If you do not overwrite this function or it returns false,
//    that parameter will be NULL.
// 2. You can create a MFnMesh object from the MDrawRequest object and use it
//    to get the raw blind data.
//

#include <maya/MPxHwShaderNode.h>

class blindDataShader : public MPxHwShaderNode
{
public:
	virtual MStatus	bind(const MDrawRequest& request, M3dView& view);
	virtual MStatus	unbind(const MDrawRequest& request, M3dView& view);

	virtual MStatus	geometry( const MDrawRequest& request,
							  M3dView& view,
							  int prim,
							  unsigned int writable,
							  int indexCount,
							  const unsigned int * indexArray,
							  int vertexCount,
							  const int * vertexIDs,
							  const float * vertexArray,
							  int normalCount,
							  const float ** normalArrays,
							  int colorCount,
							  const float ** colorArrays,
							  int texCoordCount,
							  const float ** texCoordArrays);

	virtual bool provideVertexIDs() { return true; }

public:
	// Standard Node functions
	//
    virtual MStatus compute( const MPlug&, MDataBlock& );
    static  void *  creator();
    static  MStatus initialize();
    static  MTypeId id;
};

#endif /* _blindDataShader */
