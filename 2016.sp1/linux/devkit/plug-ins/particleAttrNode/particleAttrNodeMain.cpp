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

#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include <maya/MStatus.h>

// Commands
//

// Nodes
//
#include "particleAttrNode.h"

MStatus initializePlugin( MObject obj )
//
//	Description:
//		this method is called when the plug-in is loaded into Maya.  It 
//		registers all of the services that this plug-in provides with 
//		Maya.
//
//	Arguments:
//		obj - a handle to the plug-in object (use MFnPlugin to access it)
//
{ 
	MStatus   status;
	MFnPlugin pluginFn( obj, PLUGIN_COMPANY, "1.0", "Any");

	// Add plug-in feature registration here
	//
	// Maya DG Nodes
	//
	status = pluginFn.registerNode( "particleAttr",
									particleAttrNode::id,
									particleAttrNode::creator,
									particleAttrNode::initialize,
									MPxNode::kParticleAttributeMapperNode );
	if( !status )
		status.perror("Register particleAttr node failed.");

	MGlobal::displayInfo("particleAttrNode loaded.");

	return status;
}

MStatus uninitializePlugin( MObject obj )
//
//	Description:
//		this method is called when the plug-in is unloaded from Maya. It 
//		deregisters all of the services that it was providing.
//
//	Arguments:
//		obj - a handle to the plug-in object (use MFnPlugin to access it)
//
{
	MStatus   status;
	MFnPlugin pluginFn( obj );

	// Add plug-in feature deregistration here
	//
	status = pluginFn.deregisterNode( particleAttrNode::id );

	MGlobal::displayInfo("particleAttrNode unloaded.");

	return status;
}

