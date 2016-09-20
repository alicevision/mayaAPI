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

#include <maya/MVectorArray.h>

#include "apiSimpleShapeIterator.h"

apiSimpleShapeIterator::apiSimpleShapeIterator( void * geom, MObjectArray & comps )
	: MPxGeometryIterator( geom, comps ),
	geometry( (MVectorArray*)geom )
{
	reset();
}

apiSimpleShapeIterator::apiSimpleShapeIterator( void * geom, MObject & comps )
	: MPxGeometryIterator( geom, comps ),
	geometry( (MVectorArray*)geom )
{
	reset();
}

/* override */
void apiSimpleShapeIterator::reset()
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
	if ( NULL != geometry ) 
	{
		int maxVertex = geometry->length();
		setMaxPoints( maxVertex );
	}
}

/* override */
MPoint apiSimpleShapeIterator::point() const
//
// Description
//
//    Returns the point for the current element in the iteration.
//    This is used by the transform tools for positioning the
//    manipulator in component mode. It is also used by deformers.	 
//
{
	MPoint pnt;
	if ( NULL != geometry ) 
	{
		pnt = (*geometry)[index()];
	}
	return pnt;
}

/* override */
void apiSimpleShapeIterator::setPoint( const MPoint & pnt ) const
//
// Description
//
//    Set the point for the current element in the iteration.
//    This is used by deformers.	 
//
{
	if ( NULL != geometry ) 
	{
		(*geometry)[index()] = pnt;
	}
}

/* override */
int apiSimpleShapeIterator::iteratorCount() const
{
//
// Description
//
//    Return the number of vertices in the iteration.
//    This is used by deformers such as smooth skinning
//
	return geometry->length();
	
}

/* override */
bool apiSimpleShapeIterator::hasPoints() const
//
// Description
//
//    Returns true since the shape data has points.
//
{
	return true;
}
