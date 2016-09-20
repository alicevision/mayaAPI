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
#include "slopeShaderBehavior.h"
#include "slopeShaderNode.h"

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
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "4.5", "Any");
	const MString UserClassify( "utility/color" );
	MString command( "if( `window -exists createRenderNodeWindow` )  {refreshCreateRenderNodeWindow(\"" );
	command += UserClassify;
	command += "\");}\n";

	// register nodes
	//
	plugin.registerNode( "slopeShader", slopeShaderNode::id, 
		                  slopeShaderNode::creator, slopeShaderNode::initialize,
		                  slopeShaderNode::kDependNode, &UserClassify );

	//register behaviors
	//
	plugin.registerDragAndDropBehavior( "slopeShaderBehavior",
										slopeShaderBehavior::creator);

	MGlobal::executeCommand(command);
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
	MFnPlugin plugin( obj );
	const MString UserClassify( "utility/color" );
	MString command( "if( `window -exists createRenderNodeWindow` )  {refreshCreateRenderNodeWindow(\"" );
	command += UserClassify;
	command += "\");}\n";

	// deregister nodes
	//
	plugin.deregisterNode( slopeShaderNode::id );
	// deregister behaviors
	//
	plugin.deregisterDragAndDropBehavior( "slopeShaderBehavior" );

	MGlobal::executeCommand(command);
	return status;
}
