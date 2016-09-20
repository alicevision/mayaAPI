#ifndef _MArrayDataBuilder
#define _MArrayDataBuilder
//-
// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//+
//
// CLASS:    MArrayDataBuilder
//
// ****************************************************************************

#if defined __cplusplus

// ****************************************************************************
// INCLUDED HEADER FILES


#include <maya/MStatus.h>
#include <maya/MTypes.h>

// ****************************************************************************
// FORWARD DECLARATIONS

class MDataBlock;
class MDataHandle;
class MArrayDataHandle;
class MObject;

// ****************************************************************************
// CLASS DECLARATION (MArrayDataBuilder)

//! \ingroup OpenMaya
//! \brief Array builder for arrays in data blocks. 
/*!
  An MArrayDataBuilder is used to construct data for attributes/plugs that
  have an array of data.  An array builder is used with an MArrayDataHandle
  for managing data.
*/
class OPENMAYA_EXPORT MArrayDataBuilder
{

public:
    //! Obsolete ...
	MArrayDataBuilder ( const MObject & attribute, unsigned int numElements,
						MStatus * ReturnStatus = NULL );

	MArrayDataBuilder ( MDataBlock *, const MObject &attribute, unsigned int,
						MStatus * ReturnStatus = NULL );

	MDataHandle      addLast(MStatus * ReturnStatus = NULL);
	MDataHandle      addElement( unsigned int index,
								 MStatus * ReturnStatus = NULL );
	MArrayDataHandle addLastArray(MStatus * ReturnStatus = NULL );
	MArrayDataHandle addElementArray( unsigned int index,
									  MStatus * ReturnStatus = NULL  );
	MStatus          removeElement( unsigned int index );
	unsigned int         elementCount( MStatus * ReturnStatus = NULL ) const;
	MStatus          growArray( unsigned int amount );
	MStatus          setGrowSize( unsigned int size );

    MArrayDataBuilder( const MArrayDataBuilder &other );
	MArrayDataBuilder& operator=( const MArrayDataBuilder& other );
    ~MArrayDataBuilder();

	static	const char*	className();

protected:
// No protected members

private:
	void constructClass(MDataBlock *, const MObject &attribute, unsigned int,
						MStatus * ReturnStatus = NULL );
	friend class MArrayDataHandle;
	MArrayDataBuilder();
	MArrayDataBuilder( void* );
#ifdef MBits64_
	char data[40];
#else
	char data[28];
#endif
	bool fIsNull;
};

#endif /* __cplusplus */
#endif /* _MArrayDataBuilder */
