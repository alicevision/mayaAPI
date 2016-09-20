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

// MAYA HEADERS
#include <maya/MArgList.h>
#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>

// MAIN CLASS 
class closestPointOnNurbsSurfaceCmd : public MPxCommand
{
	public:
		closestPointOnNurbsSurfaceCmd();
		virtual ~closestPointOnNurbsSurfaceCmd();
		static void* creator();
		bool isUndoable() const;
		
		virtual MStatus doIt(const MArgList&);
		virtual MStatus undoIt();
};
