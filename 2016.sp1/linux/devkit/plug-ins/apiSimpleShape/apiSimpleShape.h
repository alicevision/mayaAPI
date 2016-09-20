#ifndef _apiSimpleShape
#define _apiSimpleShape

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

///////////////////////////////////////////////////////////////////////////////
//
// apiSimpleShape.h
//
// Implements a new type of shape node in maya called apiSimpleShape.
//
// To use it
//
// loadPlugin apiSimpleShape
// string $node = `createNode apiSimpleShape`; // You'll see nothing.
//
//
// // Now add some CVs, one
// string $attr = $node + ".controlPoints[0]";
// setAttr $attr 2 2 2; 					// Now you'll have something on screen, in wireframe mode
//
//
// // or a bunch
// int $idx = 0;
// for ( $i=0; $i<100; $i++)
// {
//    for ( $j=0; $j<100; $j++)
//    {
//        string $attr = $node + ".controlPoints[ " + $idx + "]";
//        setAttr $attr $i $j 3;
//        $idx++;
//    }
// }
//
//
// INPUTS
//     NONE
//
// OUTPUTS
// 	   NONE
//
////////////////////////////////////////////////////////////////////////////////

#include <maya/MTypeId.h> 
#include <maya/MPxComponentShape.h> 

class MPxGeometryIterator;

class apiSimpleShape : public MPxComponentShape
{
public:
	apiSimpleShape();
	virtual ~apiSimpleShape(); 

	// Associates a user defined iterator with the shape (components)
	//
	virtual MPxGeometryIterator*	geometryIteratorSetup( MObjectArray&, MObject&, bool forReadOnly = false );
	virtual bool                    acceptsGeometryIterator( bool  writeable=true );
	virtual bool                    acceptsGeometryIterator( MObject&, bool writeable=true, bool forReadOnly = false);

	static  void *          creator();
	static  MStatus         initialize();

	static	MTypeId		id;
};

#endif /* _apiSimpleShape */
