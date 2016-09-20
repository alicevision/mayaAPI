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

#ifndef _apiMeshGeom
#define _apiMeshGeom

////////////////////////////////////////////////////////////////////////////////
//
// This class holds the underlying geometry for the shape or data.
// This is where geometry specific data and methods should go.
//
////////////////////////////////////////////////////////////////////////////////

#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MFloatArray.h> 
#include <maya/MVectorArray.h>

class apiMeshGeomUV; 

class apiMeshGeomUV { 
  public: 
	apiMeshGeomUV() { reset(); } 
	~apiMeshGeomUV() {} 

	int					uvId( int faceVertexIndex ) const;
	void				getUV( int uvId, float &u, float &v ) const; 
	float				u( int uvId ) const; 
	float				v( int uvId ) const; 
	int					uvcount() const; 
	void				append_uv( float u, float v ); 
	void				reset(); 
	
	MIntArray			faceVertexIndex; 
	MFloatArray			ucoord; 
	MFloatArray			vcoord; 
};

inline void apiMeshGeomUV::reset()
{
	ucoord.clear(); vcoord.clear(); faceVertexIndex.clear(); 
}

inline void apiMeshGeomUV::append_uv( float u, float v )
{
	ucoord.append( u ); 
	vcoord.append( v ); 
}

inline int apiMeshGeomUV::uvId( int fvi ) const
{
	return faceVertexIndex[fvi]; 
}

inline void apiMeshGeomUV::getUV( int uvId, float &u, float &v ) const
{
	u = ucoord[uvId]; 
	v = vcoord[uvId]; 
}

inline float apiMeshGeomUV::u( int uvId ) const
{
	return ucoord[uvId]; 
}

inline float apiMeshGeomUV::v( int uvId ) const
{
	return vcoord[uvId]; 
}

inline int apiMeshGeomUV::uvcount( ) const 
{
	return ucoord.length(); 
}


class apiMeshGeom
{
public:
	apiMeshGeom();
	~apiMeshGeom();
	apiMeshGeom& operator=( const apiMeshGeom& );

public:
    MPointArray	  vertices;
    MIntArray	  face_counts;
    MIntArray	  face_connects;
    MVectorArray  normals;
	apiMeshGeomUV uvcoords; 
    int			  faceCount;
};

#endif /* _apiMeshGeom */
