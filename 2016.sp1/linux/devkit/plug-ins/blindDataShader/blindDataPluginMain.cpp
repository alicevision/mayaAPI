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


#include "blindDataMesh.h"
#include "blindDataShader.h"

#include <maya/MFnPlugin.h>

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
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "5.0", "Any");
	const MString UserClassify( "shader/surface/utility" );

	status = plugin.registerNode( "blindDataShader",
								  blindDataShader::id, 
			                      blindDataShader::creator,
								  blindDataShader::initialize,
								  MPxNode::kHwShaderNode,
								  &UserClassify );
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	status = plugin.registerNode( "blindDataMesh",
								  blindDataMesh::id,
								  blindDataMesh::creator,
								  blindDataMesh::initialize );
	if (!status) {
		status.perror("registerNode");
		return status;
	}

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

	status = plugin.deregisterNode( blindDataMesh::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	plugin.deregisterNode( blindDataShader::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	return status;
}
