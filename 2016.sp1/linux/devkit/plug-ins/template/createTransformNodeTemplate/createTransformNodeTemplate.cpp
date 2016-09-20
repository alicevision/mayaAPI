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

#include <maya/MTemplateCommand.h>

class createTransformNodeTemplate;
char cmdName[] = "createTransformNodeTemplate";
char nodeName[] = "transform";

class createTransformNodeTemplate : public MTemplateCreateNodeCommand<createTransformNodeTemplate,cmdName,nodeName>
{
public:
	//
	createTransformNodeTemplate()
	{}
};

static createTransformNodeTemplate _create;

//
//	Entry points
//

MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "2009", "Any");

	status = _create.registerCommand( obj );
	if (!status) 
	{
		status.perror("registerCommand");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = _create.deregisterCommand( obj );
	if (!status) 
	{
		status.perror("deregisterCommand");
		return status;
	}

	return status;
}

