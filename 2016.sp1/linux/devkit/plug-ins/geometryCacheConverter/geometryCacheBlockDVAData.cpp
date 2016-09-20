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
// File Name : geometryCacheBlockDVAData.cpp
//
// Description : 
//		Stores and outputs cache blocks that carry double vector array data
//
///////////////////////////////////////////////////////////////////////////////

// Project includes
//
#include "geometryCacheBlockDVAData.h"

///////////////////////////////////////////////////////////////////////////////
//
// Methods
//
///////////////////////////////////////////////////////////////////////////////

geometryCacheBlockDVAData::geometryCacheBlockDVAData( 
		const MString& tag, 
		const double* value, 
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

geometryCacheBlockDVAData::~geometryCacheBlockDVAData()
///////////////////////////////////////////////////////////////////////////////
//
// Description : ( public method )
//		Destructor
//
///////////////////////////////////////////////////////////////////////////////
{	
}

const MVectorArray& geometryCacheBlockDVAData::data()
///////////////////////////////////////////////////////////////////////////////
//
// Description : ( public method )
//		Returns the data of this block
//
///////////////////////////////////////////////////////////////////////////////
{	
	return vectorArrayData;
}

void geometryCacheBlockDVAData::outputToAscii( ostream& os )
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
