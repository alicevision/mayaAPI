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

#include <maya/MFnPlugin.h>
#include <maya/MPxTransform.h>
#include <maya/MPxTransformationMatrix.h>
#include <maya/MTransformationMatrix.h>

#include "rockingTransformCheck.h"

//
// Plug-in entry point
//
MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "6.5", "Any");

	status = plugin.registerTransform(	"rockingTransformCheck", 
										rockingTransformCheckNode::idCheck, 
						 				&rockingTransformCheckNode::creator, 
										&rockingTransformNode::initialize,
						 				&rockingTransformCheckMatrix::creator,
										rockingTransformCheckMatrix::idCheck);
	if (!status) {
		status.perror("registerNode");
		return status;
	}
	return status;
}

//
// Plug-in exit point
//
MStatus uninitializePlugin( MObject obj)
{
	MStatus   status;
	MFnPlugin plugin(obj);

	status = plugin.deregisterNode( rockingTransformCheckNode::idCheck );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	return status;
}

