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

#include <maya/MPxPolyTweakUVCommand.h>

class MSyntax;
class MArgDatabase;
class MIntArray;
class MFloatArray;
class MObject;

class flipUVCmd : public MPxPolyTweakUVCommand
{
	public:

	flipUVCmd();
	~flipUVCmd();

	static void *creator();
	static MSyntax newSyntax ();

	static const char * cmdName;

	MStatus parseSyntax (MArgDatabase &argData);

	MStatus getTweakedUVs(
		const MObject & mesh,				// 
		MIntArray & uvList,					// UVs to move
		MFloatArray & uPos,					// Moved UVs
		MFloatArray & vPos );				// Moved UVs

	private:

	bool horizontal;						// Axis to flip
	bool extendToShell;						// Extend selection to the
											// whole shell
	bool flipGlobal;						// Flip globally or per shell
};
