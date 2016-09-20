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

// Example Plugin: pluginCallbacks.cpp
//
// This plug-in is an example of a user-defined callbacks for plugin loading/unloading.
// During load/unload specific user callbacks can be invoked to provide information about 
// the file path, and plugin names being manipulated.
//
// MSceneMessage::kBeforePluginLoad will provide the file name being loaded
// MSceneMessage::kAfterPluginLoad will provide the file name being loaded, and the plugin name
// MSceneMessage::kBeforePluginUnload will provide the plugin name 
// MSceneMessage::kAfterPluginUnload will provide the plugin name and the file name being unloaded
// 


#include <maya/MIOStream.h>
#include <maya/MPxNode.h> 
#include <maya/MFnPlugin.h>
#include <maya/MSceneMessage.h>

void prePluginLoadCallback( const MStringArray &str, void* cd )
{
	cerr << "PRE plugin load callback with " << str.length() << " items: \n";
	for (unsigned int i = 0; i < str.length(); i++ )
	{
		cerr << "\tCallback item " << i << " is : " << str[i] << endl;
	}
}

void postPluginLoadCallback( const MStringArray &str, void* cd )
{
	cerr << "POST plugin load callback with " << str.length() << " items: \n";
	for (unsigned int i = 0; i < str.length(); i++ )
	{
		cerr << "\tCallback item " << i << " is : " << str[i] << endl;
	}
}

void prePluginUnloadCallback( const MStringArray &str, void* cd )
{
	cerr << "PRE plugin unload callback with " << str.length() << " items: \n";
	for (unsigned int i = 0; i < str.length(); i++ )
	{
		cerr << "\tCallback item " << i << " is : " << str[i] << endl;
	}
}


void postPluginUnloadCallback( const MStringArray &str, void* cd )
{
	cerr << "POST plugin unload callback with " << str.length() << " items: \n";
	for (unsigned int i = 0; i < str.length(); i++ )
	{
		cerr << "\tCallback item " << i << " is : " << str[i] << endl;
	}
}

static MCallbackId prePluginLoadCallbackId;
static MCallbackId postPluginLoadCallbackId;
static MCallbackId prePluginUnloadCallbackId;
static MCallbackId postPluginUnloadCallbackId;

// The following routines are used to register/unregister
// the command we are creating within Maya
//
MStatus initializePlugin( MObject obj )
{
	MStatus status;

        prePluginLoadCallbackId = MSceneMessage::addCallback(MSceneMessage::kBeforePluginLoad,
                                     prePluginLoadCallback,
                                     NULL, &status);
        postPluginLoadCallbackId = MSceneMessage::addCallback(MSceneMessage::kAfterPluginLoad,
                                     postPluginLoadCallback,
                                     NULL, &status);
        prePluginUnloadCallbackId = MSceneMessage::addCallback(MSceneMessage::kBeforePluginUnload,
                                     prePluginUnloadCallback,
                                     NULL, &status);
        postPluginUnloadCallbackId = MSceneMessage::addCallback(MSceneMessage::kAfterPluginUnload,
                                     postPluginUnloadCallback,
                                     NULL, &status);
	return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj)
{
	MSceneMessage::removeCallback( prePluginLoadCallbackId );
	MSceneMessage::removeCallback( postPluginLoadCallbackId );
	MSceneMessage::removeCallback( prePluginUnloadCallbackId );
	MSceneMessage::removeCallback( postPluginUnloadCallbackId );

	return MS::kSuccess;
}
