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

////////////////////////////////////////////////////////////////////////
// 
// meshReorderPlugin.cc
// 
// Description:
//    Tool for reindexing meshes based on user defined starting faces
//
//    This plug-in will register the following two commands in Maya:
//       meshReorder
//       meshRemap
// 
////////////////////////////////////////////////////////////////////////

#include "meshReorderCmd.h"
#include "meshReorderTool.h"
#include "meshRemapCmd.h"
#include "meshRemapTool.h"
#include <maya/MFnPlugin.h>


// INITIALIZES THE PLUGIN BY REGISTERING THE COMMAND AND NODE:
MStatus initializePlugin(MObject obj)
{
	MStatus status;

	MFnPlugin plugin(obj, PLUGIN_COMPANY, "4.0", "Any");

	// Set editing tools/contexts
	status = plugin.registerContextCommand( "meshReorderContext", meshReorderContextCmd::creator, "meshReorder", meshReorderCommand::creator );
	if (!status)
	{
	  status.perror("registerContextCommand");
	  return status;
	}
	status = plugin.registerContextCommand( "meshRemapContext", meshRemapContextCmd::creator, "meshRemap", meshRemapCommand::creator );
	if (!status)
	{
	  status.perror("registerContextCommand");
	  return status;
	}

	return status;
}


// UNINITIALIZES THE PLUGIN BY DEREGISTERING THE COMMAND AND NODE:
MStatus uninitializePlugin(MObject obj)
{
	MStatus status;

	MFnPlugin plugin(obj);

	status = plugin.deregisterContextCommand( "meshReorderContext", "meshReorder" );
	if (!status)
	{
		status.perror("deregisterContextCommand");
		return status;
	}

	status = plugin.deregisterContextCommand( "meshRemapContext", "meshRemap" );
	if (!status)
	{
		status.perror("deregisterContextCommand");
		return status;
	}

	return status;
}
