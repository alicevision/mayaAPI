//-
// Copyright 2011 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+

#include <stdio.h>

#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MFnPlugin.h>

#include <maya/MItDag.h>
#include <maya/MDagPath.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnMesh.h>
#include <maya/MSelectionList.h>

#include <maya/M3dView.h>
#include <maya/MObjectListFilter.h>

/////////////////////////////////////////////////////////////////////////////
/*
Simple custom viewport object filter class

This plugin registers a couple of viewport object filters that can be used
with the modelEditor command to filter what draws in a specified
viewport.

EXAMPLE USAGE:

    modelEditor -q -ofl modelPanel4;
    // Result: myIncludeFilter myExcludeFilter
    modelEditor -q -ofu modelPanel4;
    // Result: My Include Filter My Exclude Filter
    modelEditor -q -obf modelPanel4;
    modelEditor -q -obu modelPanel4;

    // Set the filter
    modelEditor -e -obf "myIncludeFilter" modelPanel4
    // Result: modelPanel4 //
    modelEditor -q -obf modelPanel4;
    // Result: myIncludeFilter //
    modelEditor -q -obu modelPanel4;
    // Result: My Include Filter //

    modelEditor -e -obf "myExcludeFilter" modelPanel4;
    // Result: modelPanel4 //
    modelEditor -q -obf modelPanel4;
    // Result: myExcludeFilter //
    modelEditor -q -obu modelPanel4;
    // Result: My Exclude Filter //

    // Clear the filter
    modelEditor -e -obf "" modelPanel4
    modelEditor -q -obf modelPanel4;
    modelEditor -q -obu modelPanel4;
*/
//////////////////////////////////////////////////////////////////////////////
class viewObjectFilter : public MObjectListFilter
{
public:
	viewObjectFilter( const MString &name )
	: MObjectListFilter(name)
	, mInvertedList(false)
	{
	}
	virtual ~viewObjectFilter()
	{
	}

	virtual bool requireListUpdate()
	{
		// As the update logic only depends on scene updates for the exclusion
		// list computed in this plugin we only return true for inclusion lists.
		return (MObjectListFilter::kInclusionList == filterType());
	}

	virtual MSceneUpdateType dependentOnSceneUpdates()
	{
		return MObjectListFilter::kAddRemoveObjects;
	}

	virtual MStatus getList(MSelectionList &list);

	bool mInvertedList;
};

// List logic. This is a pretty simple example that
// builds a list of mesh shapes to return.
//
MStatus viewObjectFilter::getList(MSelectionList &list)
{
	bool debugFilterUsage = false;
	if (debugFilterUsage)
	{
		unsigned int viewCount = M3dView::numberOf3dViews();
		if (viewCount)
		{
			for (unsigned int i=0; i<viewCount; i++)
			{
				M3dView view;
				if (MStatus::kSuccess == M3dView::get3dView( i, view ))
				{
					if (view.objectListFilterName() == name())
					{
						printf("*** Update filter list %s. Exclusion=%d, Inverted=%d\n",
							name().asChar(), MObjectListFilter::kExclusionList == filterType(),
							mInvertedList);
					}
				}
			}
		}
	}
	// Clear out old list
	list.clear();

	if (mInvertedList)
	{
		MStatus status;
		MItDag::TraversalType traversalType = MItDag::kDepthFirst;
		MItDag dagIterator( traversalType, MFn::kInvalid, &status);

		for ( ; !dagIterator.isDone(); dagIterator.next() )
		{
			MDagPath dagPath;
			status = dagIterator.getPath(dagPath);
			if ( status != MStatus::kSuccess )
			{
				status.perror("MItDag::getPath");
				continue;
			}
			if (dagPath.hasFn( MFn::kMesh ))
			{
				dagIterator.prune();
				continue;
			}
			if (dagPath.childCount() <= 1)
			{
				status = list.add( dagPath );
			}
		}
	}
	else
	{
		// Get a list of all the meshes in the scene
		MItDag::TraversalType traversalType = MItDag::kDepthFirst;
		MFn::Type filter = MFn::kMesh;
		MStatus status;

		MItDag dagIterator( traversalType, filter, &status);
		for ( ; !dagIterator.isDone(); dagIterator.next() )
		{
			MDagPath dagPath;
			status = dagIterator.getPath(dagPath);
			if ( status != MStatus::kSuccess )
			{
				status.perror("MItDag::getPath");
				continue;
			}

			status = list.add( dagPath );
			if ( status != MStatus::kSuccess )
			{
				status.perror("MSelectionList add");
			}
		}
	}

	if (list.length())
	{
		return MStatus::kSuccess;
	}
	return MStatus::kFailure;
}

///////////////////////////////////////////////////
//
// Plug-in functions
//
///////////////////////////////////////////////////

static viewObjectFilter *inclusionFilter = NULL;
static viewObjectFilter *exclusionFilter = NULL;

MStatus initializePlugin( MObject obj )
{
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "1.0", "Any");

	MString inclusionName("myIncludeFilter");
	MString inclusionNameUI("My Include Filter");
	inclusionFilter = new viewObjectFilter(inclusionName);

	MString exclusionName("myExcludeFilter");
	MString exclusionNameUI("My Exclude Filter");
	exclusionFilter = new viewObjectFilter(exclusionName);

	MStatus status1 = MStatus::kFailure;
	MStatus status2 = MStatus::kFailure;
	if (inclusionFilter && exclusionFilter)
	{

		inclusionFilter->setUIName( inclusionNameUI );
		exclusionFilter->setUIName( exclusionNameUI );

		// Switch flag to either use a world exclusion list or
		// compute the exclusion manually as an inclusion list.
		bool performInversionManually = false;
		if (performInversionManually)
		{
			exclusionFilter->mInvertedList = true;
		}
		else
		{
			exclusionFilter->setFilterType( MObjectListFilter::kExclusionList );
		}

		status1 = MObjectListFilter::registerFilter( *inclusionFilter );
		status2 = MObjectListFilter::registerFilter( *exclusionFilter );
	}

	if (status1 != MStatus::kSuccess ||
		status2 != MStatus::kSuccess)
	{
		status1.perror("Failed to register object filters properly");

		if (inclusionFilter)
		{
			MObjectListFilter::deregisterFilter( *inclusionFilter );
			delete inclusionFilter;
			inclusionFilter = NULL;
		}
		if (exclusionFilter)
		{
			MObjectListFilter::deregisterFilter( *exclusionFilter );
			delete exclusionFilter;
			exclusionFilter = NULL;
		}
		return MStatus::kFailure;
	}

	return MStatus::kSuccess;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus deregistered = MStatus::kSuccess;
	if (inclusionFilter)
	{
		MStatus status = MObjectListFilter::deregisterFilter( *inclusionFilter );
		if (status != MStatus::kSuccess)
		{
			deregistered = MStatus::kFailure;
			status.perror("Failed to deregister object inclusion filter properly.");
		}
		delete inclusionFilter;
		inclusionFilter = NULL;
	}
	if (exclusionFilter)
	{
		MStatus status = MObjectListFilter::deregisterFilter( *exclusionFilter );
		if (status != MStatus::kSuccess)
		{
			deregistered = MStatus::kFailure;
			status.perror("Failed to deregister object exclusion filter properly.");
		}
		delete exclusionFilter;
		exclusionFilter = NULL;
	}

	return deregistered;
}


