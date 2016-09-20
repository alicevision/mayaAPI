//-
// ==========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

///////////////////////////////////////////////////////////////////////////////
//
// apiMeshGeom.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "apiMeshGeom.h"

apiMeshGeom::apiMeshGeom() : faceCount( 0 )
{}

apiMeshGeom::~apiMeshGeom() {}

/* override */
apiMeshGeom& apiMeshGeom::operator=( const apiMeshGeom& other )
//
// Copy the geometry
//
{
	if ( &other != this ) {
   		vertices      = other.vertices;
		face_counts   = other.face_counts;
		face_connects = other.face_connects;
		normals       = other.normals;
		uvcoords	  = other.uvcoords; 
		faceCount     = other.faceCount;
	}

	return *this;
}
