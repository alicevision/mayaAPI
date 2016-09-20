#ifndef _MSelectionContext
#define _MSelectionContext
//-
// ===========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================
//+

#if defined __cplusplus

// ****************************************************************************
// INCLUDED HEADER FILES

#include <maya/MStatus.h>

class MFloatPoint;
class MMatrix;
class MPoint;
class MSelectionMask;
class MVector;

// ****************************************************************************
// NAMESPACE

namespace MHWRender
{
// ****************************************************************************
// CLASS DECLARATION (MSelectionContext)
//! \ingroup OpenMayaRender
//! \brief Selection context used in MPxGeometryOverride::updateSelectionGranularity
/*!
This class gives control on the viewport 2.0 selection behavior.
*/
class OPENMAYARENDER_EXPORT MSelectionContext
{
public:
	//! Specifies granularity level to use for the selection
	enum SelectionLevel {
		kNone,		//!< No selection available.
		kObject,	//!< Object level.
					//!< Objects are selected as a whole. Components are not directly accessible.
		kComponent	//!< Component level.
					//!< Components such as vertices, edges and faces are selectable.
	};

	SelectionLevel selectionLevel(MStatus* ReturnStatus = NULL) const;
	MStatus setSelectionLevel(SelectionLevel level);

	static const char* className();

private:
	MSelectionContext(void* data);
	virtual ~MSelectionContext();
	void* fData;

}; // MSelectionContext

// ****************************************************************************
// CLASS DECLARATION (MIntersection)
//! \ingroup OpenMayaRender
//! \brief Describes the intersection of a selection hit.
/*!
This class gives a description of an intersection when a selection hit occurs.
*/
class OPENMAYARENDER_EXPORT MIntersection
{
public:
	MSelectionContext::SelectionLevel selectionLevel(MStatus* ReturnStatus = NULL) const;
	int index(MStatus* ReturnStatus = NULL) const;
	MStatus barycentricCoordinates(float& a, float& b) const;
	float edgeInterpolantValue(MStatus* ReturnStatus = NULL) const;
	MFloatPoint intersectionPoint(MStatus* ReturnStatus = NULL) const;
	int instanceID(MStatus* ReturnStatus = NULL) const;

	static const char* className();

private:
	MIntersection(void* data);
	~MIntersection();
	void* fData;

}; // MIntersection

// ****************************************************************************
// CLASS DECLARATION (MSelectionInfo)
//! \ingroup OpenMayaRender
//! \brief Selection information used in MPxGeometryOverride::refineSelectionPath
/*!
MSelectionInfo is used with user defined shape selection and is passed
as an argument to the MPxGeometryOverride::select refineSelectionPath.
This class encapsulates all the selection state information for
selected objects.
*/
class OPENMAYARENDER_EXPORT MSelectionInfo
{
public:
	bool singleSelection(MStatus* ReturnStatus = NULL) const;
	bool selectClosest(MStatus* ReturnStatus = NULL) const;
	bool selectable(MSelectionMask& mask, MStatus* ReturnStatus = NULL) const;
	bool selectableComponent(bool displayed, MSelectionMask& mask, MStatus* ReturnStatus = NULL) const;

	MStatus selectRect(unsigned int& x, unsigned int& y,
					unsigned int& width, unsigned int& height) const;

	bool isRay(MStatus* ReturnStatus = NULL) const;
	MMatrix getAlignmentMatrix(MStatus* ReturnStatus = NULL) const;
	MStatus getLocalRay(MPoint& pnt, MVector& vec) const;

	bool selectForHilite(const MSelectionMask& mask, MStatus* ReturnStatus = NULL) const;

	bool selectOnHilitedOnly(MStatus* ReturnStatus = NULL) const;

	static const char* className();

private:
	MSelectionInfo(void* data);
	~MSelectionInfo();
	void* fData;

};

} // namespace MHWRender

#endif /* __cplusplus */
#endif /* _MSelectionContext */

