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
// apiMeshIterator.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "apiMeshIterator.h"
#include <maya/MIOStream.h>

apiMeshGeomIterator::apiMeshGeomIterator( void * geom, MObjectArray & comps )
	: MPxGeometryIterator( geom, comps ),
	geometry( (apiMeshGeom*)geom )
{
	reset();
}

apiMeshGeomIterator::apiMeshGeomIterator( void * geom, MObject & comps )
	: MPxGeometryIterator( geom, comps ),
	geometry( (apiMeshGeom*)geom )
{
	reset();
}

/* override */
void apiMeshGeomIterator::reset()
//
// Description
//
//  	
//   Resets the iterator to the start of the components so that another
//   pass over them may be made.
//
{
	MPxGeometryIterator::reset();
	setCurrentPoint( 0 );
	if ( NULL != geometry ) {
		int maxVertex = geometry->vertices.length();
		setMaxPoints( maxVertex );
	}
}

/* override */
MPoint apiMeshGeomIterator::point() const
//
// Description
//
//    Returns the point for the current element in the iteration.
//    This is used by the transform tools for positioning the
//    manipulator in component mode. It is also used by deformers.	 
//
{
	MPoint pnt;
	if ( NULL != geometry ) {
		pnt = geometry->vertices[ index() ];
	}
	return pnt;
}

/* override */
void apiMeshGeomIterator::setPoint( const MPoint & pnt ) const
//
// Description
//
//    Set the point for the current element in the iteration.
//    This is used by deformers.	 
//
{
	if ( NULL != geometry ) {
		geometry->vertices.set( pnt, index() );
	}
}

/* override */
int apiMeshGeomIterator::iteratorCount() const
{
//
// Description
//
//    Return the number of vertices in the iteration.
//    This is used by deformers such as smooth skinning
//
	return geometry->vertices.length();
	
}

/* override */
bool apiMeshGeomIterator::hasPoints() const
//
// Description
//
//    Returns true since the shape data has points.
//
{
	return true;
}

