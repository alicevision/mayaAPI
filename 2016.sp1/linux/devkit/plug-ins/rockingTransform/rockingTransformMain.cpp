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

#include "rockingTransform.h"

//
// Plug-in entry point
//
MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "6.5", "Any");

	// Classifiy the node as a transform.  This causes Viewport
	// 2.0 to treat the node the same way it treats a regular
	// transform node.
	// 
	// The boolean flag determines whether an
	// explicit classification is provided as an argumment
	// to registerTransform().
	//
	// If not explicitly provided, then 
	// the classification will automatically be added
	// as part of registerTransform().
	//
	// The sample code has been left to not explicitly set
	// the classification.
	// 
	static const bool explicitlySetVP2classification = false;
	const MString classification = "drawdb/geometry/transform/rockingTransform";
	status = plugin.registerTransform(	"rockingTransform", 
										rockingTransformNode::id, 
						 				&rockingTransformNode::creator, 
										&rockingTransformNode::initialize,
						 				&rockingTransformMatrix::creator,
										rockingTransformMatrix::id,
										explicitlySetVP2classification ? 
											&classification : NULL);
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

	status = plugin.deregisterNode( rockingTransformNode::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	return status;
}

