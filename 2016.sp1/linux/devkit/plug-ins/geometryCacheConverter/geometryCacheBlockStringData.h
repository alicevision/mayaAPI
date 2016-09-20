#ifndef	_GEOMETRY_CACHE_BLOCK_STRING_DATA
#define _GEOMETRY_CACHE_BLOCK_STRING_DATA

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
// File Name : geometryCacheBlockStringData.h
//
// Description : 
//		Stores and outputs cache blocks that carry string data
//		Note that the Maya geometry cache file format is subject to change in 
//		future versions of Maya.
//
///////////////////////////////////////////////////////////////////////////////

// Project includes
//
#include "geometryCacheBlockBase.h"

class geometryCacheBlockStringData : public geometryCacheBlockBase
{
public:
	// Constructor / Destructor methods
	//
				geometryCacheBlockStringData( const MString& tag, const MString& value );
	virtual		~geometryCacheBlockStringData();

	// Access to the data
	//
	const MString& data();

	// Outputs the data to an output stream
	//
	virtual	void		outputToAscii( ostream& os );

private:
	// Data Member
	//
	MString stringData;		// char* data from the file saved as MString
};

#endif
