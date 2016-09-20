#include <maya/MStatus.h>
#include <maya/MFnPlugin.h>
#include "exportMetadataCmd.h"
#include "importMetadataCmd.h"
#include "createMetadataCmd.h"
#include "tweakMetadataNode.h"

//======================================================================
//
MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin ( obj, "Autodesk", "1.0", "Any" );

	//======================================================================
	status = plugin.registerCommand( exportMetadataCmd::name(),
									 exportMetadataCmd::creator,
									 exportMetadataCmd::cmdSyntax );
	if ( !status )
	{
		status.perror("registerCommand(exportMetadata)");
		return status;
	}

	//======================================================================
	status = plugin.registerCommand( importMetadataCmd::name(),
									 importMetadataCmd::creator,
									 importMetadataCmd::cmdSyntax );
	if ( !status )
	{
		status.perror("registerCommand(importMetadata)");
		return status;
	}

	//======================================================================
	status = plugin.registerCommand( createMetadataCmd::name(),
									 createMetadataCmd::creator,
									 createMetadataCmd::cmdSyntax );
	if ( !status )
	{
		status.perror("registerCommand(createMetadata)");
		return status;
	}

	//======================================================================
	status = plugin.registerNode( tweakMetadataNode::nodeName(),
								  tweakMetadataNode::id,
								  tweakMetadataNode::creator,
								  tweakMetadataNode::initialize );
	if ( !status )
	{
		status.perror("registerNode(tweakMetadata)");
		return status;
	}

	return MS::kSuccess;
}

//======================================================================
//
MStatus uninitializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterNode( tweakMetadataNode::id );
	if ( ! status )
	{
		status.perror("deregisterNode(tweakMetadata)");
		return status;
	}

	status = plugin.deregisterCommand( createMetadataCmd::name() );
	if ( ! status )
	{
		status.perror("deregisterCommand(createMetadata)");
		return status;
	}

	status = plugin.deregisterCommand( importMetadataCmd::name() );
	if ( ! status )
	{
		status.perror("deregisterCommand(importMetadata)");
		return status;
	}

	status = plugin.deregisterCommand( exportMetadataCmd::name() );
	if ( ! status )
	{
		status.perror("deregisterCommand(exportMetadata)");
		return status;
	}

	return MS::kSuccess;
}

//-
// ==================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
// 
// This computer source code  and related  instructions and comments are
// the unpublished confidential and proprietary information of Autodesk,
// Inc. and are  protected  under applicable  copyright and trade secret
// law. They may not  be disclosed to, copied or used by any third party
// without the prior written consent of Autodesk, Inc.
// ==================================================================
//+
