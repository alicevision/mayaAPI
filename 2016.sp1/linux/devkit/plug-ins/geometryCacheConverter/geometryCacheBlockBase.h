#ifndef	_GEOMETRY_CACHE_BLOCK_BASE
#define _GEOMETRY_CACHE_BLOCK_BASE

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
// File Name: geometryCacheBlockBase.h
//
// Description : 
//		A base interface for storing and outputing a cache block tag and data.
//		Note that the Maya geometry cache file format is subject to change in 
//		future versions of Maya.
//
///////////////////////////////////////////////////////////////////////////////

// Maya includes
//
#include <maya/MIffTag.h>
#include <maya/MString.h>

// Other includes
//
#include <fstream>

class geometryCacheBlockBase
{
public:
	// Constructor / Destructor methods
	//
				geometryCacheBlockBase();
				geometryCacheBlockBase( const MString& tag );
	virtual		~geometryCacheBlockBase();

	// Access to the data
	//
	const bool&		isGroup();
	const MString&	tag();

	// Outputs the data of this block to Ascii
	//
	virtual	void	outputToAscii( ostream& os );

protected:
	// Data Members
	//
	MString		blockTag;	// The tag identifier of this block
	bool		group;		// Indicates if this is a group block
};

#endif
