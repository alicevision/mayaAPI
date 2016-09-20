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

// Example Plugin: helloWorldCmd.cpp
//
// This plugin uses DeclareSimpleCommand macro
// to do the necessary initialization for a simple command
// plugin.
//
// Once this plugin is loaded by the Plugin Manager,
// it can be run from the Maya command line (i.e. script
// editor) like this:
//
//    helloWorld
//
// which will simply print the following message
// in the Output Window:
//
//    Hello World

#include <maya/MIOStream.h>
#include <maya/MSimple.h>

// Use a Maya macro to setup a simple helloWorld class
// with methods for initialization, etc.
//
DeclareSimpleCommand( helloWorld, PLUGIN_COMPANY, "4.5");

// All we need to do is supply the doIt method
// which in this case is only a Hello World example
//
MStatus helloWorld::doIt( const MArgList& )
{
	cout<<"Hello World"<<endl;
	return MS::kSuccess;
}
