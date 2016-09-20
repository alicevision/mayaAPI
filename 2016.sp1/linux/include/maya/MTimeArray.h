#ifndef _MTimeArray
#define _MTimeArray
//-
// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//+
//
// CLASS:    MTimeArray
//
// ****************************************************************************

#if defined __cplusplus

// ****************************************************************************
// INCLUDED HEADER FILES


#include <maya/MTime.h>
#include <maya/MStatus.h>

// ****************************************************************************
// CLASS DECLARATION (MTimeArray)

//! \ingroup OpenMaya
//! \brief  Array of MTime data type. 
/*!
	This class implements an array of MTimes.  Common convenience functions
	are available, and the implementation is compatible with the internal
	Maya implementation so that it can be passed efficiently between plugins
	and internal maya data structures.
*/
class OPENMAYA_EXPORT MTimeArray
{

public:
					MTimeArray();
					MTimeArray( const MTimeArray& other );
BEGIN_NO_SCRIPT_SUPPORT:
    //! NO SCRIPT SUPPORT
					MTimeArray( const MTime src[], unsigned int count );
END_NO_SCRIPT_SUPPORT:
					MTimeArray( unsigned int initialSize,
								const MTime &initialValue );
					~MTimeArray();
 	const MTime&	operator[]( unsigned int index ) const;
 	MStatus			set( const MTime& element, unsigned int index );
	MStatus			setLength( unsigned int length );
 	unsigned int		length() const;
 	MStatus			remove( unsigned int index );
 	MStatus			insert( const MTime & element, unsigned int index );
 	MStatus			append( const MTime & element );
 	MStatus			clear();
	void			setSizeIncrement ( unsigned int newIncrement );
	unsigned int		sizeIncrement () const;

BEGIN_NO_SCRIPT_SUPPORT:

    //! NO SCRIPT SUPPORT
 	MTime&			operator[]( unsigned int index );

    //! NO SCRIPT SUPPORT
	MStatus			get( MTime array[] ) const;

	//! NO SCRIPT SUPPORT
	friend OPENMAYA_EXPORT std::ostream &operator<<(std::ostream &os,
											   const MTimeArray &array);

END_NO_SCRIPT_SUPPORT:

	MTimeArray&		operator = (const MTimeArray&);
 	MStatus         copy( const MTimeArray& source );

	static const char* className();

protected:
// No protected members

private:

 	void* fArray;
};

#endif /* __cplusplus */
#endif /* _MTimeArray */
