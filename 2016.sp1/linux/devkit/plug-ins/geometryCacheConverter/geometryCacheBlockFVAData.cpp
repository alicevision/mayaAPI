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
// File Name : geometryCacheBlockFVAData.cpp
//
// Description : 
//		Stores and outputs cache blocks that carry float vector array data
//
///////////////////////////////////////////////////////////////////////////////

// Project includes
//
#include "geometryCacheBlockFVAData.h"

///////////////////////////////////////////////////////////////////////////////
//
// Methods
//
///////////////////////////////////////////////////////////////////////////////

geometryCacheBlockFVAData::geometryCacheBlockFVAData( 
		const MString& tag, 
		const float* value, 
		const uint& size )
///////////////////////////////////////////////////////////////////////////////
//
// Description : ( public method )
//		Constructor
//
///////////////////////////////////////////////////////////////////////////////
{
	blockTag = tag;
	group = false;

	for( uint i = 0; i < size * 3; i+=3 ) {
		vectorArrayData.append( MVector( value[i], value[i+1], value[i+2] ) );
	}
}

geometryCacheBlockFVAData::~geometryCacheBlockFVAData()
///////////////////////////////////////////////////////////////////////////////
//
// Description : ( public method )
//		Destructor
//
///////////////////////////////////////////////////////////////////////////////
{	
}

const MVectorArray& geometryCacheBlockFVAData::data()
///////////////////////////////////////////////////////////////////////////////
//
// Description : ( public method )
//		Returns the data of this block
//
///////////////////////////////////////////////////////////////////////////////
{	
	return vectorArrayData;
}

void geometryCacheBlockFVAData::outputToAscii( ostream& os )
///////////////////////////////////////////////////////////////////////////////
//
// Description : ( public method )
//		Outputs the data of this block to Ascii
//
///////////////////////////////////////////////////////////////////////////////
{
	MString tabs = "";
	if( !group )
		tabs = "\t";

	os << tabs << "[" << blockTag << "]\n";

	for( uint i = 0; i <  vectorArrayData.length(); i++ ) {
		os << tabs << tabs <<  vectorArrayData[i] << "\n";
	}
}
