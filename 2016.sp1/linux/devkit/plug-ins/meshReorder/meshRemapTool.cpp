//
// Description: Get user selections for mesh vertex/edge remapping 
//
// Author: Bruce Hickey
//

// This is added to prevent multiple definitions of the MApiVersion string.
#define _MApiVersion

#include <maya/MCursor.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnMesh.h>
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MMatrix.h>
#include <maya/MPlug.h>
#include <maya/MPointArray.h>
#include <maya/MVector.h>

#include "meshRemapTool.h"
#include "meshMapUtils.h"


//////////////////////////////////////////////
// The user Context
//////////////////////////////////////////////
const double epsilon = 0.00001;

meshRemapTool::meshRemapTool()
{
	setTitleString ( "Mesh Remap Tool" );
	setCursor( MCursor::editCursor );
	reset();
}

meshRemapTool::~meshRemapTool() {}

void* meshRemapTool::creator()
{
	return new meshRemapTool;
}

void meshRemapTool::toolOnSetup ( MEvent & )
{
	reset();
}

void meshRemapTool::reset()
{
	fNumSelectedPoints = 0;

	fSelectedPathSrc.clear();
	fSelectedComponentSrc.clear();
	fSelectVtxSrc.clear();
	fSelectedFaceSrc = -1;

	fSelectedPathDst.clear();
	fSelectedComponentDst.clear();
	fSelectVtxDst.clear();
	fSelectedFaceDst = -1;

	fCurrentHelpString = "Select 1st point on source mesh.";

	helpStateHasChanged();
}

//
// Selects objects within the user defined area, then process them
// 
MStatus meshRemapTool::doRelease( MEvent & event )
{
	char buf[1024];

	// Perform the base actions
	MStatus stat = MPxSelectionContext::doRelease(event);

	// Get the list of selected items 
	MGlobal::getActiveSelectionList( fSelectionList );

	//
	//  Get the selection's details, we must have exactly one component selected
	//
	MObject component;
	MDagPath path;

	MItSelectionList selectionIt (fSelectionList, MFn::kComponent);

	MStringArray	selections;
	selectionIt.getStrings( selections );
	
	// There are valid cases where only the meshes are selected.
	if( selections.length() == 0 )
	{
		// Handling the case where the user's workflow is
		// 1) select src mesh, select vtx1, vtx2, vtx3 (i.e. fNumSelectedPoints == 0)
		// 2) select dst mesh, select vtx1, vtx2, vtx3 (i.e. fNumSelectedPoints == 3)
		if( fNumSelectedPoints == 0 || fNumSelectedPoints == 3 )
		{
			MItSelectionList selectionDag (fSelectionList, MFn::kDagNode);
			if (!selectionDag.isDone() && selectionDag.getDagPath(path) == MS::kSuccess)
			{
				path.extendToShape();
				// return true there is exactly one mesh selected
				if (path.hasFn(MFn::kMesh))
				{
					selectionDag.next();
					if (selectionDag.isDone())
					{
						//	If this is the destination mesh, make sure that
						//	it doesn't have history.
						if (fNumSelectedPoints == 3)
						{
							return checkForHistory(path);
						}

						return MS::kSuccess;
					}
				}
			}
		}

		// Handling the case where the user is doing the auto remap, i.e.
		// select src mesh, and then dst mesh when fNumSelectedPoints == 0.
		if( fNumSelectedPoints == 0 )
		{
			MItSelectionList selectionDag (fSelectionList, MFn::kDagNode);
			if (!selectionDag.isDone() && selectionDag.getDagPath(path) == MS::kSuccess)
			{
				path.extendToShape();
				// Confirm first selection is a mesh
				if (path.hasFn(MFn::kMesh))
				{
					selectionDag.next();
					if (!selectionDag.isDone() &&
					    selectionDag.getDagPath(path) == MS::kSuccess)
					{
						path.extendToShape();
						// Confirm second selection is a mesh
						if (path.hasFn(MFn::kMesh))
						{
							selectionDag.next();
							// Confirm there are exactly 2 selections
							if (selectionDag.isDone())
							{
								//	Make sure that the destination mesh
								//	doesn't have history.
								return checkForHistory(path);
							}
						}
					}
				}
			}
		}
	}

	if( selections.length() != 1 )
	{
		MGlobal::displayError( "Must select exactly one vertex" );
		return MS::kSuccess;
	}

	if (selectionIt.isDone ())
	{
		MGlobal::displayError( "Selected item not a vertex" );
		return MS::kSuccess;
	}

	if( selectionIt.getDagPath (path, component) != MStatus::kSuccess )
	{
		MGlobal::displayError( "Must select a mesh or its vertex" );
		return MS::kSuccess;
	}

	if (!path.node().hasFn(MFn::kMesh) && !(path.node().hasFn(MFn::kTransform) && path.hasFn(MFn::kMesh)))
	{
		MGlobal::displayError( "Must select a mesh or its transform" );
		return MS::kSuccess;
	}

	//	If this is the first vertex of the destination mesh, make sure that
	//	it doesn't have history.
	if ((fNumSelectedPoints == 3) && (checkForHistory(path) != MS::kSuccess))
	{
		return MS::kSuccess;
	}

	MItMeshVertex fIt ( path, component, &stat );
	if( stat != MStatus::kSuccess )
	{
		MGlobal::displayError( "MItMeshVertex failed");
		return MStatus::kFailure;
	}

	if (fIt.count() != 1 )
	{
		sprintf(buf, "Invalid selection '%s'. Vertices must be picked one at a time.", selections[0].asChar() );
		MGlobal::displayError( buf );
		return MS::kSuccess;
	}
	else
	{
		sprintf(buf, "Accepting vertex '%s'", selections[0].asChar() );
		MGlobal::displayInfo(  buf );
	}

	//
	// Now that we know it's valid, process the selection, the first 3 items are the source, the second
	// 3 define the target
	//

	if( fNumSelectedPoints < 3 )
	{
		fSelectedPathSrc.append( path );
		fSelectedComponentSrc.append( component );
	}
	else 
	{
		fSelectedPathDst.append( path );
		fSelectedComponentDst.append( component );
	}

	//
	//  When each of the source/target are defined, process them. An error/invalid selection will restart the selection for
	//  that particular mesh.
	//  
	if( fNumSelectedPoints == 2 )
	{
		if( ( stat = meshMapUtils::validateFaceSelection( fSelectedPathSrc, fSelectedComponentSrc, &fSelectedFaceSrc, &fSelectVtxSrc ) ) != MStatus::kSuccess )
		{
			MGlobal::displayError("Selected vertices don't define a unique face on source mesh");
			reset();
			return stat;
		}
	}

	//
	// Once the target is defined, invoke the command
	//
	if( fNumSelectedPoints == 5 )
	{
		if( ( stat = meshMapUtils::validateFaceSelection( fSelectedPathDst, fSelectedComponentDst, &fSelectedFaceDst, &fSelectVtxDst ) ) != MStatus::kSuccess )
		{
			MGlobal::displayError("Selected vertices don't define a unique face on destination mesh");
			reset();
			return stat;
		}

		executeCmd();
	}
	else
	{
		//
		// We don't have all the details yet, just move to the next item
		//
		fNumSelectedPoints++;	
	}

	helpStateHasChanged();

	return stat;
}

MStatus meshRemapTool::checkForHistory(const MDagPath& mesh)
{
	MFnMesh	meshFn(mesh);
	MPlug	historyPlug = meshFn.findPlug("inMesh", true);

	if (historyPlug.isDestination())
	{
		MGlobal::displayError("Destination mesh has history. Its geometry cannot be modified.");
		return MS::kInvalidParameter;
	}

	return MS::kSuccess;
}

MStatus meshRemapTool::resolveMapping()
{
	// Get the list of selected items 
	MGlobal::getActiveSelectionList( fSelectionList );
	if( fSelectionList.length() != 2)
	{
		reset();
		helpStateHasChanged();
		return MStatus::kFailure;
	}

	MDagPath dagPath;
	MObject  component;
	if( fSelectionList.getDagPath(0, dagPath, component) != MStatus::kSuccess)
	{
		MGlobal::displayError(" Invalid source mesh");
		return MStatus::kFailure;
	}
	dagPath.extendToShape();
	// Repeat this 3 times one for each vertex
	fSelectedPathSrc.append(dagPath);
	fSelectedPathSrc.append(dagPath);
	fSelectedPathSrc.append(dagPath);

	if( fSelectionList.getDagPath(1, dagPath, component) != MStatus::kSuccess)
	{
		MGlobal::displayError(" Invalid destination mesh");
		return MStatus::kFailure;
	}
	dagPath.extendToShape();
	// Repeat this 3 times one for each vertex
	fSelectedPathDst.append(dagPath);
	fSelectedPathDst.append(dagPath);
	fSelectedPathDst.append(dagPath);

	// Find the vertices in Object Space of the first polygon
	MPointArray srcPts;
	MIntArray   srcVertIds, dstVertIds;
	MItMeshPolygon faceIterSrc(fSelectedPathSrc[0]);
	unsigned srcFaceId = faceIterSrc.index();
	faceIterSrc.getPoints(srcPts, MSpace::kObject);
	faceIterSrc.getVertices(srcVertIds);

	// Tranfrom the vertices to dst World Space
	unsigned i, j;
	MMatrix m = fSelectedPathDst[0].inclusiveMatrix();
	for(i = 0; i < srcPts.length(); i++)
	{
	    srcPts[i] = srcPts[i] * m;
	}

	MItMeshPolygon faceIterDst(fSelectedPathDst[0]);
	while (!faceIterDst.isDone())
	{
		MPointArray dstPts;
		faceIterDst.getPoints(dstPts, MSpace::kWorld);
		if (arePointsOverlap(srcPts, dstPts))
		{
			// Set face and vertex indices
			fSelectedFaceSrc = srcFaceId;
			fSelectedFaceDst = faceIterDst.index();
			fSelectVtxSrc.append(srcVertIds[0]);
			fSelectVtxSrc.append(srcVertIds[1]);
			fSelectVtxSrc.append(srcVertIds[2]);

			faceIterDst.getVertices(dstVertIds);
			for (j = 0; j < dstPts.length(); j++)
			{
				MVector v = dstPts[j] - srcPts[0];
				if (v * v < epsilon)
				{
					fSelectVtxDst.append(dstVertIds[j]);
					break;
				}
			}

			for (j = 0; j < dstPts.length(); j++)
			{
				MVector v = dstPts[j] - srcPts[1];
				if (v * v < epsilon)
				{
					fSelectVtxDst.append(dstVertIds[j]);
					break;
				}
			}

			for (j = 0; j < dstPts.length(); j++)
			{
				MVector v = dstPts[j] - srcPts[2];
				if (v * v < epsilon)
				{
					fSelectVtxDst.append(dstVertIds[j]);
					break;
				}
			}

			return MStatus::kSuccess;
		}
		faceIterDst.next();
	}
	return MStatus::kFailure;
}

bool meshRemapTool::arePointsOverlap(const MPointArray& srcPts, const MPointArray& dstPts) const
{
	unsigned i, j;
	for(i = 0; i < 3; i++) 
	{
		bool overlap = false;
		const MPoint &pt = srcPts[i];
		for(j = 0; j < dstPts.length(); j++) 
		{
			MVector v = dstPts[j] - pt;
			if (v * v < epsilon)
			{
				overlap = true;
				break;
			}
		}
		if (!overlap) return false;
	}
	return true;
}

void meshRemapTool::completeAction()
{
	if( resolveMapping() != MStatus::kSuccess )
	{
		reset();
		return;
	}
	executeCmd();
}

void meshRemapTool::executeCmd()
{
	char cmdString[200];
	sprintf(cmdString, "meshRemap %s.vtx[%d] %s.vtx[%d] %s.vtx[%d] %s.vtx[%d] %s.vtx[%d] %s.vtx[%d]",
		fSelectedPathSrc[0].partialPathName().asChar(), fSelectVtxSrc[0],
		fSelectedPathSrc[1].partialPathName().asChar(), fSelectVtxSrc[1],
		fSelectedPathSrc[2].partialPathName().asChar(), fSelectVtxSrc[2],
		fSelectedPathDst[0].partialPathName().asChar(), fSelectVtxDst[0],
		fSelectedPathDst[1].partialPathName().asChar(), fSelectVtxDst[1],
		fSelectedPathDst[2].partialPathName().asChar(), fSelectVtxDst[2]);

	MStatus st = MGlobal::executeCommand(cmdString, true, true);
	if (st)
	{
		MGlobal::displayInfo( "Mesh remapping complete" );
	}

	MGlobal::clearSelectionList();
	reset();
}

void meshRemapTool::helpStateHasChanged()
//
// Description:
//              Set up the correct information in the help window based
//              on the current state.  
//
{
	switch (fNumSelectedPoints)
	{
		case 0:
		 fCurrentHelpString = "For auto remap select source mesh and then destination mesh and Press Enter. For manual remap select 1st vertex on source mesh.";
		 break;
		case 1:
		 fCurrentHelpString = "Select 2nd vertex on source mesh.";
		 break;
		case 2:
		 fCurrentHelpString = "Select 3rd vertex on source mesh.";
		 break;
		case 3:
		 fCurrentHelpString = "Select 1st vertex on target mesh.";
		 break;
		case 4:
		 fCurrentHelpString = "Select 2nd vertex on target mesh.";
		 break;
		case 5:
		 fCurrentHelpString = "Select 3rd vertex on target mesh.";
		 break;
		default:
		 // nothing, shouldn't be here
		 break;
	 }

	setHelpString( fCurrentHelpString );
}

MPxContext* meshRemapContextCmd::makeObj()
{
	return new meshRemapTool;
}

void* meshRemapContextCmd::creator()
{
	return new meshRemapContextCmd;
}

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
