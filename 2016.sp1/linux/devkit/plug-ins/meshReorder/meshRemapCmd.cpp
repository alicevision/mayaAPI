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

#include "meshMapUtils.h"
#include "meshRemapCmd.h"

#include <maya/MArgList.h>
#include <maya/MDagPathArray.h>
#include <maya/MFnMesh.h>
#include <maya/MIOStream.h>
#include <maya/MItSelectionList.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MObjectArray.h>
#include <maya/MPlug.h>
#include <maya/MStatus.h>


// CONSTRUCTOR DEFINITION:
meshRemapCommand::meshRemapCommand()
{
	fColorArrays = NULL;
	fRepArray = NULL;
	fClampedArray = NULL;
	fUArrays = NULL;
	fVArrays = NULL;
	fUVCountsArrays = NULL;
	fUVIdsArrays = NULL;
}


// DESTRUCTOR DEFINITION:
meshRemapCommand::~meshRemapCommand()
{
	reset();
}


// METHOD FOR CREATING AN INSTANCE OF THIS COMMAND:
void* meshRemapCommand::creator()
{
	return new meshRemapCommand;
}


MStatus meshRemapCommand::parseArgs(const MArgList& args)
{
	MSelectionList list;
	MString err;
	MStatus stat; 

	if( args.length() != 6 )
	{
		displayError("6 vertices must be specified");
		return MS::kFailure;
	}

	int argIdx = 0;
	for (unsigned int i = 0; i < 2; i++)
	{
		MObjectArray selectedComponent(3);
		MDagPathArray selectedPath;

		selectedPath.setLength(3);

		for (unsigned int j = 0; j < 3; j++)
		{
			MString arg;

			if( ( stat = args.get( argIdx, arg )) != MStatus::kSuccess )
			{
				displayError( "Can't parse arg");
				return stat;
			}

			list.clear();	
			if (! list.add( arg ) )
			{
				err = arg + ": no such component";
				displayError(err);
				return MS::kFailure; // no such component
			}

			MItSelectionList selectionIt (list, MFn::kComponent);
			if (selectionIt.isDone ())
			{
				err = arg + ": not a component";
				displayError (err);
				return MS::kFailure;
			}

			if( selectionIt.getDagPath (selectedPath[j], selectedComponent[j]) != MStatus::kSuccess )
			{
				displayError( "Can't get path for");
				return stat;
			}


			if (!selectedPath[j].node().hasFn(MFn::kMesh) && !(selectedPath[j].node().hasFn(MFn::kTransform) && selectedPath[j].hasFn(MFn::kMesh)))
			{
				err = arg + ": Invalid type!  Only a mesh or its transform can be specified!";
				displayError (err);
				return MStatus::kFailure;
			}

			argIdx++;
		}

		if( i == 0 )
		{
			if( ( stat = meshMapUtils::validateFaceSelection( selectedPath, selectedComponent, &fFaceIdxSrc, &fFaceVtxSrc ) ) != MStatus::kSuccess )
			{
				displayError("Selected vertices don't define a unique face on source mesh");
				return stat;
			}

			fDagPathSrc = selectedPath[0];
		}
		else
		{
			if( ( stat = meshMapUtils::validateFaceSelection( selectedPath, selectedComponent, &fFaceIdxDst, &fFaceVtxDst ) ) != MStatus::kSuccess )
			{
				displayError("Selected vertices don't define a unique face on target mesh");
				return stat;
			}

			fDagPathDst = selectedPath[0];
		}
	}


	if( fDagPathSrc == fDagPathDst )
	{
		displayError("Cannot use one mesh for both source and target");
		return stat;
	}

	return MStatus::kSuccess;
}


// FIRST INVOKED WHEN COMMAND IS CALLED, PARSING THE COMMAND ARGUMENTS, INITIALIZING DEFAULT PARAMETERS, THEN CALLING redoIt():
MStatus meshRemapCommand::doIt(const MArgList& args)
{
	MStatus  stat = MStatus::kSuccess;

	if ( ( stat = parseArgs( args ) ) != MStatus::kSuccess )
	{
		displayError ("Error parsing arguments");
		return stat;
	}

	return redoIt();
}

MStatus meshRemapCommand::redoIt()
{
	int i;
	MStatus  stat = MStatus::kSuccess;
	
	fDagPathDst.extendToShape();
	MFnMesh theMeshDst( fDagPathDst, &stat );
	if( stat != MStatus::kSuccess )
	{
		displayError(" MFnMesh creation");
		return stat;
	}

	//	The destination mesh cannot have history or else this won't work.
	MPlug	historyPlug = theMeshDst.findPlug("inMesh", true);

	if (historyPlug.isDestination()) {
		displayError("The destination mesh has history. Its geometry cannot be modified.");
		return MS::kInvalidParameter;
	}

	fDagPathSrc.extendToShape();
	MFnMesh theMeshSrc( fDagPathSrc, &stat );
	if( stat != MStatus::kSuccess )
	{
		displayError(" MFnMesh creation");
		return stat;
	}

	MFloatPointArray 	origVertices;
	stat = theMeshSrc.getPoints (origVertices, MSpace::kObject );
	if( stat != MStatus::kSuccess )
	{
		displayError(" MFnMesh getPoints");
		return stat;
	}

	// 
	// Traverse the source
	//
	
	// 
	// Initialize the traversal flags and CV mappings for this shape 
	MIntArray 			faceTraversal( theMeshSrc.numPolygons(), false );
	MIntArray 			cvMapping( theMeshSrc.numVertices(), -1 );
	MIntArray 			cvMappingInverse( theMeshSrc.numVertices(), -1 ); 

	//
	//  Starting with the user selected face, recursively rebuild the entire mesh
	//

	MIntArray 			newPolygonCounts;
	MIntArray 			newPolygonConnects;
	MFloatPointArray 	newVertices;

	stat = meshMapUtils::traverseFace( fDagPathSrc, fFaceIdxSrc, fFaceVtxSrc[0], fFaceVtxSrc[1], faceTraversal,
					cvMapping, cvMappingInverse, 
					newPolygonCounts, newPolygonConnects, 
					origVertices, newVertices );
	if ( stat != MStatus::kSuccess )
	{
		displayError(" could not process all the mesh faces.");
		return stat;
	}

	// reuse newPolygonCounts and newPolygonConnects for the src mesh.
	newPolygonCounts.clear();
	newPolygonConnects.clear();
	MItMeshPolygon polyIter(fDagPathSrc.node());
	while(!polyIter.isDone()) 
	{
		newPolygonCounts.append(polyIter.polygonVertexCount());
		for(i = 0; i < (int) polyIter.polygonVertexCount(); i++)
		{
			newPolygonConnects.append(polyIter.vertexIndex(i));
		}
		polyIter.next();
	}

	// 
	// Now, traverse the destination
	//

	MFloatPointArray 	origVerticesDst;
	stat = theMeshDst.getPoints (origVerticesDst, MSpace::kObject );
	if( stat != MStatus::kSuccess )
	{
		displayError(" MFnMesh getPoints");
		return stat;
	}

	// Initialize the traversal flags and CV mappings for this shape 
	MIntArray 			faceTraversalDst( theMeshDst.numPolygons(), false );
	MIntArray 			cvMappingDst( theMeshDst.numVertices(), -1 );
	MIntArray 			cvMappingInverseDst( theMeshDst.numVertices(), -1 ); 

	//
	//  Starting with the user selected face, recursively rebuild the entire mesh
	//

	MIntArray 			newPolygonCountsDst;
	MIntArray 			newPolygonConnectsDst;
	MFloatPointArray 	newVerticesDst;

	stat = meshMapUtils::traverseFace( fDagPathDst, fFaceIdxDst, fFaceVtxDst[0], fFaceVtxDst[1], faceTraversalDst,
					cvMappingDst, cvMappingInverseDst, 
					newPolygonCountsDst, newPolygonConnectsDst, 
					origVerticesDst, newVerticesDst );
	if ( stat != MStatus::kSuccess )
	{
		displayError(" could not process all the mesh faces.");
		return stat;
	}

	//
	// Use the generated maps to build a new CV list that will remap
	// the destination to the source topology
	//
	for( i = 0; i < theMeshDst.numVertices(); i++ )
	{
		newVerticesDst[cvMappingInverse[i]] = origVerticesDst[cvMappingInverseDst[i]];
	}

	// Prepare for undo. Must collect the mesh information here before it is modified by createInPlace() call.
	fVertices.copy(origVerticesDst);
	MItMeshPolygon polyIterDst(fDagPathDst.node());
	while(!polyIterDst.isDone()) 
	{
		fPolygonCounts.append(polyIterDst.polygonVertexCount());
		for(i = 0; i < (int)polyIterDst.polygonVertexCount(); i++)
		{
			fPolygonConnects.append(polyIterDst.vertexIndex(i));
		}
		polyIterDst.next();
	}

	// Store Colors for undo
	fColorSetNames.clear();
	theMeshDst.getColorSetNames(fColorSetNames);
	int numColorSets = fColorSetNames.length();
	fClampedArray = new bool[numColorSets];
	fRepArray = new MFnMesh::MColorRepresentation[numColorSets];
	fColorArrays = new MColorArray[numColorSets];

	for ( i = 0; i < numColorSets; i++ ) 
	{
		fClampedArray[i] = theMeshDst.isColorClamped(fColorSetNames[i]);
		fRepArray[i] = theMeshDst.getColorRepresentation(fColorSetNames[i]);
		theMeshDst.getVertexColors(fColorArrays[i], &(fColorSetNames[i]));
	}

	// Store UVs for undo
	fUVSetNames.clear();
	theMeshDst.getUVSetNames(fUVSetNames);
	int numUVSets = fUVSetNames.length();
	fUArrays = new MFloatArray[numUVSets];
	fVArrays = new MFloatArray[numUVSets];
	fUVIdsArrays = new MIntArray[numUVSets];
	fUVCountsArrays = new MIntArray[numUVSets];
	fUVIdsArrays = new MIntArray[numUVSets];

	for ( i = 0; i < numUVSets; i++ ) 
	{
		theMeshDst.getAssignedUVs(fUVCountsArrays[i], fUVIdsArrays[i], &(fUVSetNames[i]));
		theMeshDst.getUVs(fUArrays[i], fVArrays[i], &(fUVSetNames[i]));
	}

	//
	//  Create the mesh. The old approach of using copyInPlace is doing referenced counting of the mesh, which is
	//  not what we want and has caused strange problem, i.e. reorder not working on a remapped mesh.
	// 
	stat = theMeshDst.createInPlace( newVerticesDst.length(), newPolygonCounts.length(), newVerticesDst, newPolygonCounts, newPolygonConnects );
	if ( stat != MStatus::kSuccess )
	{
		displayError(" mesh copy failed.");
		reset();
		return stat;
	}

	// Copy Colors from src to dst mesh
	MStringArray colorSetNamesSrc;
	theMeshSrc.getColorSetNames(colorSetNamesSrc);
	int numColorSetsSrc = colorSetNamesSrc.length();
	for (i = 0; i < numColorSetsSrc; i++) 
	{
		MIntArray mapping(fVertices.length());;
		if (i == 0) 
		{
			for (unsigned j = 0; j < fVertices.length(); j++) 
			{
				mapping[j] = j;
			}
		}

		MColorArray colorArraySrc;
		bool        clampedSrc;
		MFnMesh::MColorRepresentation repSrc;
		clampedSrc = theMeshSrc.isColorClamped(colorSetNamesSrc[i]);
		repSrc = theMeshSrc.getColorRepresentation(colorSetNamesSrc[i]);
		theMeshSrc.getVertexColors(colorArraySrc, &(colorSetNamesSrc[i]));

		theMeshDst.createColorSet(colorSetNamesSrc[i], NULL, clampedSrc, repSrc);
		if (colorArraySrc.length() > 0 && colorArraySrc.length() == fVertices.length())
		{
			theMeshDst.setVertexColors(colorArraySrc, mapping, NULL, repSrc);
		}
	}

	// Copy UVs from src to dst mesh
	MStringArray uvSetNamesSrc;
	theMeshSrc.getUVSetNames(uvSetNamesSrc);
	int numUVSetsSrc = uvSetNamesSrc.length();
	MString defaultUVSetName;
	theMeshDst.getCurrentUVSetName(defaultUVSetName);
	for (i = 0; i < numUVSetsSrc; i++) 
	{
		MFloatArray uArraySrc;
		MFloatArray vArraySrc;
		theMeshSrc.getUVs(uArraySrc, vArraySrc, &(uvSetNamesSrc[i]));

		if (uvSetNamesSrc[i] != defaultUVSetName) 
		{
			theMeshDst.createUVSet(uvSetNamesSrc[i]);
		}
		if (uArraySrc.length() > 0 && uArraySrc.length() == vArraySrc.length())
		{
			theMeshDst.setUVs(uArraySrc, vArraySrc, &(uvSetNamesSrc[i]));
			MIntArray uvCounts;
			MIntArray uvIds;
			theMeshSrc.getAssignedUVs(uvCounts, uvIds, &(uvSetNamesSrc[i]));
			theMeshDst.assignUVs(uvCounts, uvIds, &(uvSetNamesSrc[i]));
		}
	}

	return stat;
}

MStatus meshRemapCommand::undoIt()
{
	int i;
	MStatus  stat = MStatus::kSuccess;
	MFnMesh theMesh( fDagPathDst );

	stat = theMesh.createInPlace( fVertices.length(), fPolygonCounts.length(), fVertices, fPolygonCounts, fPolygonConnects );

	// Restore Colors from cached data for undo
	int numColorSets = fColorSetNames.length();
	for (i = 0; i < numColorSets; i++) 
	{
		MIntArray mapping(fVertices.length());;
		if (i == 0) 
		{
			for (unsigned j = 0; j < fVertices.length(); j++) 
			{
				mapping[j] = j;
			}
		}

		theMesh.createColorSet(fColorSetNames[i], NULL, fClampedArray[i], fRepArray[i]);
		if (fColorArrays[i].length() > 0 && fColorArrays[i].length() == fVertices.length())
		{
			theMesh.setVertexColors(fColorArrays[i], mapping, NULL, fRepArray[i]);
		}
	}

	// Restore UVs from cached data for undo
	int numUVSets = fUVSetNames.length();
	MString defaultUVSetName;
	theMesh.getCurrentUVSetName(defaultUVSetName);
	for (i = 0; i < numUVSets; i++) 
	{
		if (fUVSetNames[i] != defaultUVSetName)
		{
			theMesh.createUVSet(fUVSetNames[i]);
		}
		if (fUArrays[i].length() > 0 && fUArrays[i].length() == fVArrays[i].length())
		{
			theMesh.setUVs(fUArrays[i], fVArrays[i], &(fUVSetNames[i]));
			theMesh.assignUVs(fUVCountsArrays[i], fUVIdsArrays[i], &(fUVSetNames[i]));
		}
	}

	reset();

	return stat;
}

void meshRemapCommand::reset()
{
	fVertices.clear();
	fPolygonCounts.clear();
	fPolygonConnects.clear();

	fColorSetNames.clear();
	fUVSetNames.clear();
	if (fClampedArray != NULL)
	{
		delete [] fClampedArray;
		fClampedArray = NULL;
	}
	if (fRepArray != NULL)
	{
		delete [] fRepArray;
		fRepArray = NULL;
	}
	if (fUArrays != NULL)
	{
		delete [] fUArrays;
		fUArrays = NULL;
	}
	if (fVArrays != NULL)
	{
		delete [] fVArrays;
		fVArrays = NULL;
	}
	if (fColorArrays != NULL)
	{
		delete [] fColorArrays;
		fColorArrays = NULL;
	}
	if (fUVCountsArrays != NULL)
	{
		delete [] fUVCountsArrays;
		fUVCountsArrays = NULL;
	}
	if (fUVIdsArrays != NULL)
	{
		delete [] fUVIdsArrays;
		fUVIdsArrays = NULL;
	}
}
