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
// Provides a data type for some arbitrary user geometry.
// 
// A users geometry class can exist in the DAG by creating an
// MPxSurfaceShape (and UI) class for it and can also be passed through
// DG connections by creating an MPxGeometryData class for it.
// 
// MPxGeometryData is the same as MPxData except it provides 
// additional methods to modify the geometry data via an iterator.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _apiMeshData
#define _apiMeshData

#include <maya/MPxGeometryData.h>
#include <maya/MTypeId.h>
#include <maya/MString.h>
#include "apiMeshGeom.h"

class apiMeshData : public MPxGeometryData
{
public:
	//////////////////////////////////////////////////////////////////
	//
	// Overrides from MPxData
	//
	//////////////////////////////////////////////////////////////////

	apiMeshData();
	virtual ~apiMeshData();

	virtual MStatus			readASCII( const MArgList& argList, unsigned& idx );
	virtual MStatus			readBinary( istream& in, unsigned length );
	virtual MStatus			writeASCII( ostream& out );
	virtual MStatus			writeBinary( ostream& out );

	virtual	void			copy ( const MPxData& );

	virtual MTypeId         typeId() const;
	virtual MString         name() const;

	//////////////////////////////////////////////////////////////////
	//
	// Overrides from MPxGeometryData
	//
	//////////////////////////////////////////////////////////////////

	virtual MPxGeometryIterator* iterator( MObjectArray & componentList,
											MObject & component,
											bool useComponents);
	virtual MPxGeometryIterator* iterator( MObjectArray & componentList,
											MObject & component,
											bool useComponents,
											bool world) const;

	virtual bool	updateCompleteVertexGroup( MObject & component ) const;

	//////////////////////////////////////////////////////////////////
	//
	// Helper methods
	//
	//////////////////////////////////////////////////////////////////

	MStatus					readVerticesASCII( const MArgList&, unsigned& );
	MStatus					readNormalsASCII( const MArgList&, unsigned& );
	MStatus					readFacesASCII( const MArgList&, unsigned& );
	MStatus				    readUVASCII( const MArgList&, unsigned& ); 

	MStatus					writeVerticesASCII( ostream& out );
	MStatus					writeNormalsASCII( ostream& out );
	MStatus					writeFacesASCII( ostream& out );
	MStatus					writeUVASCII( ostream& out ); 

	static void * creator();

public:
	static const MString typeName;
	static const MTypeId id;

	// This is the geometry our data will pass though the DG
	//
	apiMeshGeom* fGeometry;
};

#endif /* apimeshData */
