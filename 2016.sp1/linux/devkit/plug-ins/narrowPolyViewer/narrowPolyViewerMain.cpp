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

//
//	File Name: narrowPolyViewerMain.cpp
//
//	Description:
//		All of the global plug-in intialization code goes here. This allows
//		for other commands/views to be derived from narrowPolyViewerCmd and 
//		narrowPolyViewer with errors and warning about multiple inclusions of 
//		MFnPlugin and multiple definitions on initializePlugin, unintializePlugin.
//

#include "narrowPolyViewerCmd.h"

#include <maya/MFnPlugin.h>

MStatus initializePlugin(MObject obj)
{
	MFnPlugin cmdPlugin (obj, PLUGIN_COMPANY, 
						 "5.0", "Any");

	MStatus cmdStatus = cmdPlugin.registerModelEditorCommand(
										kViewCmdName,
									 	narrowPolyViewerCmd::creator,
										narrowPolyViewer::creator);

	if (MS::kSuccess != cmdStatus) {
		cmdStatus.perror("registerModelEditorCommand");
		return cmdStatus;
	}

	return cmdStatus;
}

MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin cmdPlugin(obj);
	MStatus cmdStatus = cmdPlugin.deregisterModelEditorCommand(kViewCmdName);

	if (MS::kSuccess != cmdStatus) {
		cmdStatus.perror("deregisterModelEditorCommand");
		return cmdStatus;
	}

	return cmdStatus;
}
