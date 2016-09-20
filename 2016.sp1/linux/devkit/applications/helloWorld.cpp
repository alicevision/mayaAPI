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

#include <maya/MLibrary.h>
#include <maya/MIOStream.h>
#include <maya/MGlobal.h>

int main(int /*argc*/, char **argv)
{
	MStatus status;

	status = MLibrary::initialize (true, argv[0], true);
	if ( !status ) {
		status.perror("MLibrary::initialize");
		return (1);
	}
	
	// Write the text out in 3 different ways.
	cout << "Hello World! (cout)\n";
	MGlobal::displayInfo("Hello world! (script output)" );
	MGlobal::executeCommand( "print \"Hello world! (command script output)\\n\"", true );

	MLibrary::cleanup();

	return (0);
}
