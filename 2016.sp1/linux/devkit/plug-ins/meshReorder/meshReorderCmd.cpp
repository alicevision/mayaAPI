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
#include "meshReorderCmd.h"

#include <maya/MArgList.h>
#include <maya/MDagPathArray.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MIOStream.h>
#include <maya/MItSelectionList.h>
#include <maya/MItMeshVertex.h>
#include <maya/MPlug.h>
#include <maya/MString.h>

// CONSTRUCTOR DEFINITION:
meshReorderCommand::meshReorderCommand()
{
	fClampedArray = NULL;
	fRepArray = NULL;
	fColorArrays = NULL;
	fVertexColorArrays = NULL;
	fColorIdsArrays = NULL;
	fUArrays = NULL;
	fVArrays = NULL;
	fUVIdsArrays = NULL;
}


// DESTRUCTOR DEFINITION:
meshReorderCommand::~meshReorderCommand()
{
	resetColorsUVsMemory();
}


// METHOD FOR CREATING AN INSTANCE OF THIS COMMAND:
void* meshReorderCommand::creator()
{
	return new meshReorderCommand;
}


MStatus meshReorderCommand::parseArgs(const MArgList& args)
{
	MStatus stat;
	MString err;


	if( args.length() != 3 )
	{
		displayError("3 vertices must be specified");
		return MS::kFailure;
	}

	MObjectArray selectedComponent(3);
	MDagPathArray selectedPath;

	selectedPath.setLength(3);

	int argIdx = 0;
	for (unsigned int j = 0; j < 3; j++)
	{
		MString arg;

		if( ( stat = args.get( argIdx, arg )) != MStatus::kSuccess )
		{
			displayError( "Can't parse arg");
			return stat;
		}

		MSelectionList list;
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

	if( ( stat = meshMapUtils::validateFaceSelection( selectedPath, selectedComponent, &fFaceIdxSrc, &fFaceVtxSrc ) ) != MStatus::kSuccess )
	{
		displayError("Selected vertices don't define a unique face on source mesh");
		return stat;
	}

	fDagPathSrc = selectedPath[0];

	return stat;
}

// FIRST INVOKED WHEN COMMAND IS CALLED, PARSING THE COMMAND ARGUMENTS, INITIALIZING DEFAULT PARAMETERS, THEN CALLING redoIt():
MStatus meshReorderCommand::doIt(const MArgList& args)
{
	MStatus  stat = MStatus::kSuccess;

	if ( ( stat = parseArgs( args ) ) != MStatus::kSuccess )
	{
		displayError ("Error parsing arguments");
		return stat;
	}

	return redoIt();
}

MStatus meshReorderCommand::redoIt()
{
	MStatus  stat = MStatus::kSuccess;

	MIntArray 			newPolygonCounts;
	MIntArray 			newPolygonConnects;
	MFloatPointArray 	origVertices;
	MFloatPointArray 	newVertices;

	unsigned i;

	MFnMesh theMesh( fDagPathSrc, &stat );
	if( stat != MStatus::kSuccess )
	{
		displayError(" MFnMesh creation");
		return stat;
	}

	//	The mesh cannot have history or this won't work.
	MPlug	historyPlug = theMesh.findPlug("inMesh", true);

	if (historyPlug.isDestination()) {
		displayError("The mesh has history. Its geometry cannot be modified.");
		return MS::kInvalidParameter;
	}

	stat = theMesh.getPoints (origVertices, MSpace::kObject );
	if( stat != MStatus::kSuccess )
	{
		displayError(" MFnMesh getPoints");
		return stat;
	}

	// Initialize the traversal flags and CV mappings for this shape 
	MIntArray 			faceTraversal( theMesh.numPolygons(), false );
	MIntArray cvMapping(theMesh.numVertices(), -1);
	MIntArray cvMappingInverse(theMesh.numVertices(), -1);

	//
	//  Starting with the user selected face, recursively rebuild the entire mesh
	//
	stat = meshMapUtils::traverseFace( fDagPathSrc, fFaceIdxSrc, fFaceVtxSrc[0], fFaceVtxSrc[1], faceTraversal,
					cvMapping, cvMappingInverse, 
					newPolygonCounts, newPolygonConnects, 
					origVertices, newVertices );

	if ( stat != MStatus::kSuccess )
	{
		displayError(" could not process all the mesh faces.");
		return stat;
	}

	// Store mesh vertices and connectivity information for undo. Must collect the information here before it is 
	// modified by createInPlace() call.
	fVertices.copy(origVertices);
	MItMeshPolygon polyIter(fDagPathSrc.node());
	while(!polyIter.isDone()) 
	{
		fPolygonCounts.append(polyIter.polygonVertexCount());
		for(i = 0; i < polyIter.polygonVertexCount(); i++)
		{
			fPolygonConnects.append(polyIter.vertexIndex(i));
		}
		polyIter.next();
	}

	fColorSetNames.clear();
	theMesh.getColorSetNames(fColorSetNames);
	int numColorSets = fColorSetNames.length();
	fColorArrays = new MColorArray[numColorSets];
	fColorIdsArrays = new MIntArray[numColorSets];

	collectColorsUVs(theMesh, false);

	stat = theMesh.createInPlace( newVertices.length(), newPolygonCounts.length(), newVertices, newPolygonCounts, newPolygonConnects );

	if ( stat != MStatus::kSuccess )
	{
		displayError(" MFnMesh::createInPlace failed.");

		fCVMapping.clear();
		fCVMappingInverse.clear();
		fVertices.clear();
		fPolygonCounts.clear();
		fPolygonConnects.clear();
		delete [] fColorArrays;  fColorArrays = NULL;
		delete [] fColorIdsArrays;fColorIdsArrays = NULL;

		// Free memeory allocated in collectColorsUVs()
		resetColorsUVsMemory();

		return stat;
	}

	assignColorsUVs(theMesh, cvMapping, cvMappingInverse, false);

	// Store fCVMapping and fCVMappingInverse for undo
	fCVMapping = cvMapping;
	fCVMappingInverse = cvMappingInverse;

	return stat;
}

MStatus meshReorderCommand::undoIt()
{
	MStatus  stat = MStatus::kSuccess;
	MFnMesh theMesh( fDagPathSrc );

	fColorSetNames.clear();
	theMesh.getColorSetNames(fColorSetNames);

	collectColorsUVs(theMesh, true);

	stat = theMesh.createInPlace( fVertices.length(), fPolygonCounts.length(), fVertices, fPolygonCounts, fPolygonConnects );

	assignColorsUVs(theMesh, fCVMappingInverse, fCVMapping, true);

	fCVMapping.clear();
	fCVMappingInverse.clear();
	fVertices.clear();
	fPolygonCounts.clear();
	fPolygonConnects.clear();
	delete [] fColorArrays;  fColorArrays = NULL;
	delete [] fColorIdsArrays;fColorIdsArrays = NULL;

	return stat;
}

void meshReorderCommand::collectColorsUVs(MFnMesh &theMesh, bool isUndo)
{
	int i;
	// Store Colors
	int numColorSets = fColorSetNames.length();
	fClampedArray = new bool[numColorSets];
	fRepArray = new MFnMesh::MColorRepresentation[numColorSets];
	fVertexColorArrays = new MColorArray[numColorSets];

	for (i = 0; i < numColorSets; i++)
	{
		fClampedArray[i] = theMesh.isColorClamped(fColorSetNames[i]);
		fRepArray[i] = theMesh.getColorRepresentation(fColorSetNames[i]);
		// We need to use two different approaches to set colors because unfortunately 
		// setVertexColors() doesn't work in the case of undo and setColors() will
		// require a huge effort to construct the colorIds in the non undo case.
		theMesh.getVertexColors(fVertexColorArrays[i], &(fColorSetNames[i]));

		if(!isUndo) {
			theMesh.getColors(fColorArrays[i], &(fColorSetNames[i]));
			unsigned nth = 0;
			fColorIdsArrays[i].setLength(theMesh.numColors(fColorSetNames[i]));
			MItMeshPolygon polyIter(fDagPathSrc.node());
			while(!polyIter.isDone()) 
			{
				for(unsigned j = 0; j < polyIter.polygonVertexCount(); j++)
				{
					unsigned polygonIdx = polyIter.index();
					int colorId;
					theMesh.getColorIndex(polygonIdx, j, colorId, &(fColorSetNames[i]));
					fColorIdsArrays[i][nth] = colorId;
					nth++;
				}
				polyIter.next();
			}
		}
		theMesh.deleteColorSet(fColorSetNames[i]);
	}

	// Store UVs 
	fUVSetNames.clear();
	theMesh.getUVSetNames(fUVSetNames);
	int numUVSets = fUVSetNames.length();
	fUArrays = new MFloatArray[numUVSets];
	fVArrays = new MFloatArray[numUVSets];
	fUVIdsArrays = new MIntArray[numUVSets];

	for (i = 0; i < numUVSets; i++) 
	{
		MIntArray uvCounts;
		MIntArray uvIds;
		theMesh.getAssignedUVs(uvCounts, uvIds, &(fUVSetNames[i]));

		unsigned nth = 0;
		fUVIdsArrays[i].setLength(fVertices.length());
		MItMeshPolygon polyIter(fDagPathSrc.node());
		while(!polyIter.isDone()) 
		{
			for(unsigned j = 0; j < polyIter.polygonVertexCount(); j++)
			{
				unsigned vertexIdx = polyIter.vertexIndex(j);
				fUVIdsArrays[i][vertexIdx] = uvIds[nth];
				nth++;
			}
			polyIter.next();
		}

		theMesh.getUVs(fUArrays[i], fVArrays[i], &(fUVSetNames[i]));
		theMesh.deleteUVSet(fUVSetNames[i]);
	}
}

void meshReorderCommand::assignColorsUVs(MFnMesh &theMesh, MIntArray &colorMapping, MIntArray &uvMapping, bool isUndo)
{
	int i;

	// Copy Colors 
	MString defaultColorSetName;
	theMesh.getCurrentColorSetName(defaultColorSetName);
	int numColorSets = fColorSetNames.length();
	for (i = 0; i < numColorSets; i++) 
	{
		// Not to duplicate default color set
		if (fColorSetNames[i] != defaultColorSetName)
		{
			theMesh.createColorSet(fColorSetNames[i], NULL, fClampedArray[i], fRepArray[i]);
		}

		if (fVertexColorArrays[i].length() > 0 && fVertexColorArrays[i].length() == fVertices.length())
		{
			// We need to use two different approaches to set colors because unfortunately 
			// setVertexColors() doesn't work in the case of undo and setColors() will
			// require a huge effort to construct the colorIds in the non undo case.
			if(!isUndo) 
			{
				theMesh.setVertexColors(fVertexColorArrays[i], colorMapping, NULL, fRepArray[i]);
			} else {
				theMesh.setColors(fColorArrays[i], NULL, fRepArray[i]);
				theMesh.assignColors(fColorIdsArrays[i], &(fColorSetNames[i]));
			}
		}
	}

	// Copy UVs
	MString defaultUVSetName;
	theMesh.getCurrentUVSetName(defaultUVSetName);
	int numUVSets = fUVSetNames.length();
	for (i = 0; i < numUVSets; i++) 
	{
		// Not to duplicate default uv set
		if (fUVSetNames[i] != defaultUVSetName)
		{
			theMesh.createUVSet(fUVSetNames[i]);
		}

		if (fUArrays[i].length() > 0 && fUArrays[i].length() == fVArrays[i].length())
		{
			theMesh.setUVs(fUArrays[i], fVArrays[i], &(fUVSetNames[i]));
			MItMeshPolygon polyIter(fDagPathSrc.node());
			while(!polyIter.isDone()) 
			{
				for(unsigned j = 0; j < polyIter.polygonVertexCount(); j++)
				{
					unsigned polygonIdx = polyIter.index();
					unsigned vertexIdx = polyIter.vertexIndex(j);
					int uvId = fUVIdsArrays[i][uvMapping[vertexIdx]];
					theMesh.assignUV(polygonIdx, j, uvId);
				}
				polyIter.next();
			}
		}
	}

	resetColorsUVsMemory();
}

void meshReorderCommand::resetColorsUVsMemory()
{
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
	if (fVertexColorArrays != NULL)
	{
		delete [] fVertexColorArrays;
		fVertexColorArrays = NULL;
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
	if (fUVIdsArrays != NULL)
	{
		delete [] fUVIdsArrays;
		fUVIdsArrays = NULL;
	}
	fColorSetNames.clear();
	fUVSetNames.clear();
}
