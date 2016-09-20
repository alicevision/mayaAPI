#ifndef saveSwatchesCmd_h
#define saveSwatchesCmd_h
//-
// ==========================================================================
// Copyright 2009 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+
#include <maya/MPxCommand.h>

class MArgList;

class SaveSwatchesCmd : public MPxCommand
{
public:
	static void*	creator()		{ return new SaveSwatchesCmd(); }
	MStatus			doIt(const MArgList& args);

	static const MString			commandName;
};

#endif
