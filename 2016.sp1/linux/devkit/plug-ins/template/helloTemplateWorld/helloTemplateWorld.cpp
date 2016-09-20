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

class helloTemplateWorld;
char cmdName[] = "helloTemplateWorld";

class helloTemplateWorld : public MTemplateAction<helloTemplateWorld,cmdName, MTemplateCommand_nullSyntax >
{
public:
	helloTemplateWorld() {}

	virtual ~helloTemplateWorld() {}

	virtual MStatus	doIt ( const MArgList& )
	{
		this->displayInfo("Hello Template World...");
		return MS::kSuccess;
	}
};

static helloTemplateWorld _hello;

//
//	Entry points
//

MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "2009", "Any");

	status = _hello.registerCommand( obj );
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

	status = _hello.deregisterCommand( obj );
	if (!status) 
	{
		status.perror("deregisterCommand");
		return status;
	}

	return status;
}

