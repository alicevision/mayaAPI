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

//	File Name: narrowPolyViewerCmd.h
//
//	Description:
//		Code to test drawing with multiple cameras in the same view.
//

#ifndef NARROWPOLYVIEWERCMD_H
#define NARROWPOLYVIEWERCMD_H

#include <maya/MDagPathArray.h>
#include <maya/MPxModelEditorCommand.h>

#include "narrowPolyViewer.h"

#define kInitFlag			"-in"
#define kInitFlagLong		"-initilize"

#define kResultsFlag		"-r"
#define kResultsFlagLong	"-results"

#define kClearFlag			"-cl"
#define kClearFlagLong		"-clear"

#define kToleranceFlag		"-tol"
#define kToleranceFlagLong	"-tolerance"

#define kViewCmdName		"narrowPolyViewer"

class MPx3dModelView;
class MObject;
class MDagPath;

class narrowPolyViewerCmd: public MPxModelEditorCommand
{
public:
	typedef MPxModelEditorCommand ParentClass;

						narrowPolyViewerCmd();
	virtual				~narrowPolyViewerCmd();

	static void*		creator();

	virtual MStatus		doEditFlags();

	virtual MStatus 	doQueryFlags();

	virtual MStatus 	appendSyntax();

protected:

	MStatus initTests(narrowPolyViewer &view);
	MStatus testResults(narrowPolyViewer &view);
	MStatus clearResults(narrowPolyViewer &view);
	MDagPathArray 		fCameraList;

private:
	static const char*  className() { return "narrowPolyViewerCmd"; }
};
#endif
