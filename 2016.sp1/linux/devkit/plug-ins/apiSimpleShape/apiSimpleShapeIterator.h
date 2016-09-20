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

////////////////////////////////////////////////////////////////////////////////
//
// Component iterator for control-point based geometry
//
// This is used by the translate/rotate/scale manipulators to 
// determine where to place the manipulator when components are
// selected.
//
// As well deformers use this class to deform points of the shape.
//
////////////////////////////////////////////////////////////////////////////////

#include <maya/MPxGeometryIterator.h>
#include <maya/MPoint.h>

class MVectorArray;

class apiSimpleShapeIterator : public MPxGeometryIterator
{
public:
	apiSimpleShapeIterator( void * userGeometry, MObjectArray & components );
	apiSimpleShapeIterator( void * userGeometry, MObject & components );

    //////////////////////////////////////////////////////////
	//
	// Overrides
	//
    //////////////////////////////////////////////////////////

	virtual void		reset();
	virtual MPoint		point() const;
	virtual void		setPoint( const MPoint & ) const;
	virtual int			iteratorCount() const;
	virtual bool        hasPoints() const;

public:
	MVectorArray * 		geometry;
};
