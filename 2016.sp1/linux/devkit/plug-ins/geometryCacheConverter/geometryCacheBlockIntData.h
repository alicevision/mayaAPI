#ifndef	_GEOMETRY_CACHE_BLOCK_INT_DATA
#define _GEOMETRY_CACHE_BLOCK_INT_DATA

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
// File Name : geometryCacheBlockIntData.h
//
// Description : 
//		Stores and outputs cache blocks that carry int data
//		Note that the Maya geometry cache file format is subject to change in 
//		Future versions of Maya.
//
///////////////////////////////////////////////////////////////////////////////

// Project includes
//
#include "geometryCacheBlockBase.h"

class geometryCacheBlockIntData : public geometryCacheBlockBase
{
public:
	// Constructor / Destructor methods
	//
				geometryCacheBlockIntData( const MString& tag, const int& value );
	virtual		~geometryCacheBlockIntData();

	// Access to the data
	//
	const int& data();

	// Outputs the data to an output stream
	//
	virtual	void		outputToAscii( ostream& os );

private:
	// Data Member
	//
	int intData;
};

#endif
