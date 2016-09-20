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
// Pick.cc
//		- Pick objects by name
//
//     EGs:  doPick curveShape1
//           doPick "curveShape*"
//
////////////////////////////////////////////////////////////////////////

#include <maya/MIOStream.h>

#include <maya/MPxCommand.h>
#include <maya/MFnPlugin.h>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>

class Pick : public MPxCommand
{
public:
					Pick() {};
	virtual			~Pick(); 

	MStatus     	doIt( const MArgList& args );
	static void*	creator();
};

Pick::~Pick() {}

void* Pick::creator()
{
	return new Pick();
}
	
MStatus Pick::doIt( const MArgList& args )
{
	MStatus res = MS::kSuccess;

	unsigned len = args.length();
	if ( len > 0 ) {
		MString object_name( args.asString(0) );

		if ( MS::kSuccess != MGlobal::selectByName( object_name ) )
			cerr << "Object " << object_name.asChar() << " not found\n";
	} else {
		cerr << "No Object name specified\n";
	}

	return res;
}

MStatus initializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");

	status = plugin.registerCommand( "pick", Pick::creator );
	if (!status) {
		status.perror("registerCommand");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterCommand( "pick" );
	if (!status) {
		status.perror("deregisterCommand");
		return status;
	}

	return status;
}
