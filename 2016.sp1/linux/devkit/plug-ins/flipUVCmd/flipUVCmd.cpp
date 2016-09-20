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

// 
// File: flipUVCmd.cpp
//
// MEL Command: flipUV
//

#include "flipUVCmd.h"

#include <maya/MIOStream.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MIntArray.h>
#include <maya/MFloatArray.h>
#include <maya/MFnMesh.h>

// Flags for this command
static const char * horizFlag			= "-h";
static const char * horizFlagLong		= "-horizontal";
static const char * globalFlag			= "-fg";
static const char * globalFlagLong		= "-flipGlobal";
static const char * extendFlag			= "-es";
static const char * extendFlagLong		= "-extendToShell";

const char * flipUVCmd::cmdName = "flipUV";
//
///////////////////////////////////////////////////////////////////////////////
flipUVCmd::flipUVCmd() :
	horizontal(false),
	extendToShell(false),
	flipGlobal(false)
{}

//
///////////////////////////////////////////////////////////////////////////////
flipUVCmd::~flipUVCmd() {}

//
// Method to create a flipUVCmd object
///////////////////////////////////////////////////////////////////////////////
void *flipUVCmd::creator() { return new flipUVCmd; }

//
// Add additionnal flags for this command. The default setting of the
// syntax is done in the base class.
///////////////////////////////////////////////////////////////////////////////
MSyntax flipUVCmd::newSyntax ()
{
	MStatus status;

	// Get the base class syntax, and append to it.
	MSyntax syntax = MPxPolyTweakUVCommand::newSyntax();

	status = syntax.addFlag(horizFlag, horizFlagLong, MSyntax::kBoolean);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

	status = syntax.addFlag(globalFlag, globalFlagLong, MSyntax::kBoolean);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

	status = syntax.addFlag(extendFlag, extendFlagLong, MSyntax::kBoolean);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

	return syntax;
}

//
// Read the values of the additionnal flags for this command.
///////////////////////////////////////////////////////////////////////////////
MStatus flipUVCmd::parseSyntax (MArgDatabase &argData)
{
	MStatus status;
	
	// Get the flag values, otherwise the default values are used.

	if (argData.isFlagSet(horizFlag)) {
		status = argData.getFlagArgument(horizFlag, 0, horizontal);
		CHECK_MSTATUS_AND_RETURN_IT(status);
	}

	if (argData.isFlagSet(globalFlag)) {
		status = argData.getFlagArgument(globalFlag, 0, flipGlobal);
		CHECK_MSTATUS_AND_RETURN_IT(status);
	}

	if (argData.isFlagSet(extendFlag)) {
		status = argData.getFlagArgument(extendFlag, 0, extendToShell);
		CHECK_MSTATUS_AND_RETURN_IT(status);
	}

	return MS::kSuccess;
}

//
// Change the UVS for the given selection on this mesh object.
//
///////////////////////////////////////////////////////////////////////////////
MStatus flipUVCmd::getTweakedUVs(
	const MObject & meshObj,					// Object
	MIntArray & uvList,						// UVs to move
	MFloatArray & uPos,						// Moved UVs
	MFloatArray & vPos )					// Moved UVs
{
	MStatus status;
	unsigned int i;
	MFloatArray uArray;
	MFloatArray vArray;
	
	MFnMesh mesh( meshObj );

	// Read all UVs from the poly object
	status = mesh.getUVs(uArray, vArray);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	unsigned int nbUvShells = 1;
	MIntArray uvShellIds;
	if ((!flipGlobal) || extendToShell)
	{
		// First, extract the UV shells.
		status = mesh.getUvShellsIds(uvShellIds, nbUvShells);
		CHECK_MSTATUS_AND_RETURN_IT(status);
	}

	if (extendToShell)
	{
		// Find all shells that have at least a selected UV.
		bool *selected = new bool[nbUvShells];
		for (i = 0 ; i<nbUvShells ; i++)
			selected[i] = false;

		for (i = 0 ; i<uvList.length() ; i++)
		{
			int indx = uvList[i];
			selected[uvShellIds[indx]] = true;
		}
		
		// Now recompute a new list of UVs to modify.

		unsigned int numUvs = mesh.numUVs();
		unsigned int numSelUvs = 0;

		// Preallocate a buffer, large enough to hold all Ids. This
		// prevents multiple reallocation from happening when growing
		// the array.
		uvList.setLength(numUvs);

		for (i = 0 ; i<numUvs ; i++)
		{
			if (selected[uvShellIds[i]])
				uvList[numSelUvs++] = i;
		}

		// clamp the array to the proper size.
		uvList.setLength(numSelUvs);

		delete [] selected;
	}

	// For global flips, just pretend there is only one shell
	if (flipGlobal) nbUvShells = 1;

	float *minMax = new float[nbUvShells*4];

	for (i = 0 ; i<nbUvShells ; i++)
	{
		minMax[4*i+0] =  1e30F;				// Min U
		minMax[4*i+1] =  1e30F;				// Min V
		minMax[4*i+2] = -1e30F;				// Max U
		minMax[4*i+3] = -1e30F;				// Max V
	}

	// Get the bounding box of the UVs, for each shell if flipGlobal
	// is true, or for the whole selection if false.
	for (i = 0 ; i<uvList.length() ; i++)
	{
		int indx = uvList[i];
		int shellId = 0;
		if (!flipGlobal) shellId = uvShellIds[indx];

		if (uArray[indx] < minMax[4*shellId+0]) 
			minMax[4*shellId+0] = uArray[indx];
		if (vArray[indx] < minMax[4*shellId+1])
			minMax[4*shellId+1] = vArray[indx];
		if (uArray[indx] > minMax[4*shellId+2]) 
			minMax[4*shellId+2] = uArray[indx];
		if (vArray[indx] > minMax[4*shellId+3])
			minMax[4*shellId+3] = vArray[indx];
	}

	// Adjust the size of the output arrays
	uPos.setLength(uvList.length());
	vPos.setLength(uvList.length());

	for (i = 0 ; i<uvList.length() ; i++)
	{
		int shellId = 0;
		int indx = uvList[i];
		if (!flipGlobal) shellId = uvShellIds[indx];
		
		// Flip U or V along the bounding box center.
		if (horizontal)
		{
			uPos[i] = minMax[4*shellId+0] + minMax[4*shellId+2] -
				uArray[indx];
			vPos[i] = vArray[indx];
		}
		else
		{
			uPos[i] = uArray[indx];
			vPos[i] = minMax[4*shellId+1] + minMax[4*shellId+3] -
				vArray[indx];
		}
	}

	delete [] minMax;

	return MS::kSuccess;
}
