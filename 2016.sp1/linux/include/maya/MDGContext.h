#ifndef _MDGContext
#define _MDGContext
//-
// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//+
//
// CLASS:    MDGContext
//
// ****************************************************************************

#if defined __cplusplus

// ****************************************************************************
// INCLUDED HEADER FILES


#include <maya/MStatus.h>
#include <maya/MTypes.h>

// ****************************************************************************
// DECLARATIONS

class MObject;
class MTime;

// ****************************************************************************
// CLASS DECLARATION (MDGContext)

//! \ingroup OpenMaya
//! \brief Dependency graph (DG) context class. 
/*!

  Control the way in which dependency nodes are evaluated.

  DG contexts are used to define the way in which a dependency node is
  going to be evaluated. Examples of such contexts include "normal",
  "at a given time, "for a specific instance", etc.

  MDGContext is mainly used in two places; within methods that trigger
  evaluations, to define what kind of evaluate is being requested, and
  within data blocks (MDataBlock), to identify how the data was
  created.
*/
class OPENMAYA_EXPORT MDGContext
{
public:

	// Normal
	MDGContext( );

	// Timed
	MDGContext( const MTime & when );

	MDGContext( const MDGContext& in );

	~MDGContext();



    // Method for determining whether the context is the "normal" one,
	// ie. the one used for normal evaluation
	bool     	isNormal( MStatus * ReturnStatus = NULL ) const;

	MStatus 	getTime( MTime & ) const;

	MDGContext&	operator =( const MDGContext& other );

	static const char* className();

	// Default context "Normal"
	static		MDGContext	fsNormal;


protected:
// No protected members

private:
	const void * data;
	friend class MPlug;
	friend class MDataBlock;
	MDGContext( const void* );
};

#endif /* __cplusplus */
#endif /* _MDGContext */
