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
// apiSimpleShape.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include <math.h>           
#include <maya/MIOStream.h>

#include <maya/MFnPlugin.h>

#include "apiSimpleShape.h"           
#include "apiSimpleShapeUI.h"           
#include "apiSimpleShapeIterator.h"


////////////////////////////////////////////////////////////////////////////////
//
// Shape implementation
//
////////////////////////////////////////////////////////////////////////////////

MTypeId apiSimpleShape::id( 0x8009a );

apiSimpleShape::apiSimpleShape() {}

apiSimpleShape::~apiSimpleShape() {}

void* apiSimpleShape::creator()
//
// Description
//
//    Called internally to create a new instance of the users MPx node.
//
{
	return new apiSimpleShape();
}

MStatus apiSimpleShape::initialize()
//
// Description
//
//    Attribute (static) initialization.
//
{ 
	return MS::kSuccess;
}


MPxGeometryIterator* apiSimpleShape::geometryIteratorSetup(MObjectArray& componentList,
													MObject& components,
													bool forReadOnly )
//
// Description
//
//    Creates a geometry iterator compatible with his shape.
//
// Arguments
//
//    componentList - list of components to be iterated
//    components    - component to be iterator
//    forReadOnly   -
//
// Returns
//
//    An iterator for the components
//
{
	apiSimpleShapeIterator * result = NULL;
	if ( components.isNull() ) 
	{
		result = new apiSimpleShapeIterator( getControlPoints(), componentList );
	}
	else 
	{
		result = new apiSimpleShapeIterator( getControlPoints(), components );
	}
	return result;
}

bool apiSimpleShape::acceptsGeometryIterator( bool writeable )
//
// Description
//
//    Specifies that this shape can provide an iterator for getting/setting
//    control point values.
//
// Arguments
//
//    writable - maya asks for an iterator that can set points if this is true
//
{
	return true;
}

bool apiSimpleShape::acceptsGeometryIterator( MObject&, bool writeable, bool forReadOnly )
//
// Description
//
//    Specifies that this shape can provide an iterator for getting/setting
//    control point values.
//
// Arguments
//
//    writable   - maya asks for an iterator that can set points if this is true
//    forReadOnly - maya asking for an iterator for querying only
//
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////
//
// Node registry
//
// Registers/Deregisters apiSimpleShape user defined shape.
//
////////////////////////////////////////////////////////////////////////////////

MStatus initializePlugin( MObject obj )
{ 
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "5.0", "Any");
	MStatus stat = plugin.registerShape( "apiSimpleShape", apiSimpleShape::id,
								   &apiSimpleShape::creator,
								   &apiSimpleShape::initialize,
								   &apiSimpleShapeUI::creator  );
	if ( ! stat ) {
		cerr << "Failed to register shape\n";
	}

	return stat;
}

MStatus uninitializePlugin( MObject obj)
{
	MFnPlugin plugin( obj );
	MStatus stat;

	stat = plugin.deregisterNode( apiSimpleShape::id );
	if ( ! stat ) {
		cerr << "Failed to deregister shape : apiSimpleShape \n";
	}

	return stat;
}
