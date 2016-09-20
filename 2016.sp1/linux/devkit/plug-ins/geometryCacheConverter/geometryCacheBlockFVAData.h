#ifndef	_GEOMETRY_CACHE_BLOCK_FVA_DATA
#define _GEOMETRY_CACHE_BLOCK_FVA_DATA

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
// File Name : geometryCacheBlockFVAData.h
//
// Description : 
//		Stores and outputs cache blocks that carry float vector array data
//		Note that the Maya geometry cache file format is subject to change in 
//		future versions of Maya.
//
///////////////////////////////////////////////////////////////////////////////

// Project includes
//
#include "geometryCacheBlockBase.h"

// Maya includes
//
#include <maya/MVectorArray.h>

class geometryCacheBlockFVAData : public geometryCacheBlockBase
{
public:
	// Constructor / Destructor methods
	//
				geometryCacheBlockFVAData( const MString& tag, const float* value, const uint& size );
	virtual		~geometryCacheBlockFVAData();

	// Access to the data
	//
	const MVectorArray& data();

	// Outputs the data to an output stream
	//
	virtual	void		outputToAscii( ostream& os );

private:
	// Data Member
	//
	MVectorArray vectorArrayData;
};

#endif
