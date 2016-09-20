#ifndef _MDagPath
#define _MDagPath
//-
// ==========================================================================
// Copyright 2012 Autodesk, Inc.  All rights reserved.
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
// ==========================================================================
//+
//
// CLASS:    MDagPath
//
// ****************************************************************************
//
// CLASS DESCRIPTION (MDagPath)
//
//	The DAG Path Class provides methods for obtaining one or all Paths to a
//	specified DAG Node, determining if a Path is valid and if two Paths are
//	equivalent, obtaining the length, transform, and inclusive and exclusive
//	matrices of Path, as well as performing Path to Path assignment.
//
// ****************************************************************************

#if defined __cplusplus

// ****************************************************************************
// INCLUDED HEADER FILES


#include <maya/MStatus.h>
#include <maya/MTypes.h>
#include <maya/MStatus.h>
#include <maya/MObject.h>

// ****************************************************************************
// DECLARATIONS

class MMatrix;
class MDagPathArray;
class MString;


// ****************************************************************************
// DECLARATIONS
//
//! \ingroup OpenMaya
//! \brief A data structure to store the per path draw override information
/*!
	A simple draw override data structure.
	Draw override is per DAG path information. It checks starting from current
	object and upwards until an enabled draw override is found.
*/

// ****************************************************************************
class OPENMAYA_EXPORT MDAGDrawOverrideInfo{
public:
	/*!
		Draw override type
	*/
	enum DrawOverrideDisplayType {
		kDisplayTypeNormal,    //!< Display as normal
		kDisplayTypeReference, //!< Display as reference
		kDisplayTypeTemplate   //!< Display as templated(not selectable, only show as wireframe)
	};

	/*!
		Draw override LOD
	*/
	enum DrawOverrideLOD {
		kLODFull,             //!< Display all render items like shaded, wireframe as full detailed
		kLODBoundingBox       //!< Display only a bounding box, nothing else
	};

	MDAGDrawOverrideInfo():
		//! Draw override enabled or not
		fOverrideEnabled(false),
		//! Display type like normal, reference, templated
		fDisplayType(kDisplayTypeNormal),
		//! Level of detail like Full or bounding box
		fLOD(kLODFull),
		//! Whether allow to draw shaded item
		fEnableShading(true),
		//! Whether allow to draw textured item
		fEnableTexturing(true),
		//! Whether the whole geometry is visible
		fEnableVisible(true),
		//! Whether the object is visible during playback
		fPlaybackVisible(true)
		{}

public:
	bool fOverrideEnabled;
	DrawOverrideDisplayType fDisplayType;
	DrawOverrideLOD fLOD;
	bool	fEnableShading;
	bool	fEnableTexturing;
	bool	fEnableVisible;
	bool	fPlaybackVisible;
};


// ****************************************************************************
// CLASS DECLARATION (MDagPath)

//! \ingroup OpenMaya
//! \brief  DAG Path.
/*!
Provides methods for obtaining one or all Paths to a specified DAG Node,
determining if a Path is valid and if two Paths are equivalent, obtaining the
length, transform, and inclusive and exclusive matrices of a Path, as well as
performing Path to Path assignment.

DAG Paths may be used as parameters to some methods in the DAG Node Function
Set (MFnDagNode).

It is often useful to combine DAG Paths into DAG Path arrays
(MDagPathArray).

Use this DAG Path Class to obtain and query Paths to DAG Nodes.  A DAG path
is a path from the world node to a particular object in the DAG.

If a DAG object is instanced, then an MDagPath is required to identify a
particular instance.  DAG paths are also required when doing world space
transformations.

Use this class in conjunction with the DAG Iterator (MItDag), DAG Node
Function Set (MFnDagNode) and DAG Path Array class (MDagPathArray) to query and
edit the DAG.
*/
class OPENMAYA_EXPORT MDagPath
{

public:
	MDagPath();
	MDagPath( const MDagPath& src );
	virtual ~MDagPath();

	static MStatus	getAllPathsTo( const MObject & node,
								   MDagPathArray & pathArray );
	static MStatus	getAPathTo( const MObject & node,
								MDagPath & path );
	bool            hasFn( MFn::Type type,
						   MStatus * ReturnStatus = NULL ) const;
	MFn::Type       apiType(MStatus * ReturnStatus = NULL) const;

	bool        	isValid(MStatus * ReturnStatus = NULL) const;
	MObject      	node(MStatus * ReturnStatus = NULL) const;
	MObject   		transform( MStatus * ReturnStatus = NULL ) const;
	unsigned int    length(MStatus * ReturnStatus = NULL) const;
	MStatus         extendToShape();
	MStatus         extendToShapeDirectlyBelow( unsigned int );
	MStatus         numberOfShapesDirectlyBelow( unsigned int& ) const;
	MStatus         push( const MObject &child );
	MStatus         pop( unsigned int num = 1 );
	unsigned int        childCount(  MStatus * ReturnStatus = NULL ) const;
	MObject 	    child( unsigned int i, MStatus * ReturnStatus = NULL ) const;
	MMatrix      	inclusiveMatrix(MStatus * ReturnStatus = NULL) const;
	MMatrix      	exclusiveMatrix(MStatus * ReturnStatus = NULL) const;
	MMatrix      	inclusiveMatrixInverse(MStatus * ReturnStatus = NULL)const;
	MMatrix      	exclusiveMatrixInverse(MStatus * ReturnStatus = NULL)const;
	MDagPath&		operator = ( const MDagPath& src);
	bool			operator == ( const MDagPath& src) const;
	MStatus         set( const MDagPath& src);

	unsigned int		pathCount(MStatus * ReturnStatus = NULL) const;
	MStatus			getPath( MDagPath & path, unsigned int i = 0 ) const;
	MString         fullPathName(MStatus *ReturnStatus = NULL) const;
	MString         partialPathName(MStatus *ReturnStatus = NULL) const;

	bool            isInstanced( MStatus *ReturnStatus = NULL ) const;
	unsigned int    instanceNumber( MStatus *ReturnStatus = NULL ) const;

	bool            isVisible( MStatus *ReturnStatus = NULL ) const;
	bool            isTemplated( MStatus *ReturnStatus = NULL ) const;

	MDAGDrawOverrideInfo getDrawOverrideInfo() const;

	unsigned int getDisplayStatus() const;

	static const char* className();

	//! Obsolete
	static MDagPath getAPathTo( const MObject & node,
								MStatus * ReturnStatus = NULL );

protected:
// No protected members

private:

 	void * data;
};

#endif /* __cplusplus */
#endif /* _MDagPath */
