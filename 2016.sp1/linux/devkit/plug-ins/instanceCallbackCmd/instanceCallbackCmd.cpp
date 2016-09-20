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
    
/*
	File: instanceCallbackCmd.cpp
	Command: "instCallbackCmd"

    Description
		class InstanceCallbackCmd 
		This plugin demonstrates the functionality added to MDagMessage class.
		The new functions added allow callbacks to be added for
			1) Instance Added for a specified node(and its instances)
			2) Instance Removed for a specified node(and its instances)
			3) Instance Added for any node
			4) Instance Removed for any node

	
	  This plugin: 
	  i.   Draws a circle, 
	  ii.  Gets its dagPath using an iterator
	  iii. Adds callback for instance added and removed for this circle.

		The callback functions just displays a message indicating the 
		invocation of the registered callback function.

*/

#include <maya/MFnPlugin.h>
#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MString.h>
#include <maya/MGlobal.h>
#include <maya/MLockMessage.h>
#include <maya/MFnAttribute.h>
#include <maya/MFnMesh.h>
#include <maya/MFloatPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MDagPath.h>
#include <maya/MDagMessage.h>
#include <maya/MFnNurbsCurve.h>


// callback function for instance added
void addCallbackFunc (MDagPath &dagPath, MDagPath &otherPath,   
		void *clientData/*, MLockMessage::LockDAGEvent eventType, bool &decision*/ )
{
	MGlobal::displayInfo("CALLBACK-FUNCTION REGISTERED FOR INSTANCE ADDED INVOKED");	
}


// callback function for instance removed
void remCallbackFunc ( MDagPath &dagPath, MDagPath &otherPath, void *clientData)
{
	MGlobal::displayInfo("CALLBACK-FUNCTION REGISTERED FOR INSTANCE REMOVED INVOKED");
}



 
class InstanceCallbackCmd : public MPxCommand
{

public:
		
	MStatus doIt( const MArgList& args );
	static void* creator();
};




void* InstanceCallbackCmd::creator()
{
	return new InstanceCallbackCmd;
}


MStatus InstanceCallbackCmd::doIt( const MArgList& args )
{
	
	MStatus status = MS::kSuccess;

	// Draw a circle and get its dagPath
	// using an iterator
	MGlobal::executeCommand("circle");
	MFnNurbsCurve circle;
		
	
	MDagPath dagPath;
	MItDependencyNodes iter( MFn::kNurbsCurve , &status);

	for(iter.reset(); !iter.isDone() ; iter.next())
	{
		MObject item = iter.item();
		if(item.hasFn(MFn::kNurbsCurve))
		{
			circle.setObject(item);
			circle.getPath(dagPath);
			MGlobal::displayInfo("DAG_PATH is " + dagPath.fullPathName());
			
			if(dagPath.isValid())
			{
				// register callback for instance add AND remove
				//
				MDagMessage::addInstanceAddedCallback   ( dagPath,addCallbackFunc,	NULL, &status);
				MDagMessage::addInstanceRemovedCallback ( dagPath,remCallbackFunc,	NULL, &status);

				MGlobal::displayInfo("CALLBACK ADDED FOR INSTANCE ADD/REMOVE");
				
			}
			
		}
	}

	
	if (status != MS::kSuccess)
	{
		MGlobal::displayInfo("STATUS RETURNED IS NOT SUCCESS");
	}
	
	return status;
}




////////////////////////////////////////////////////////////////
//
//				Init // UnInit
//
////////////////////////////////////////////////////////////////
MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "6.0", "Any");
	plugin.registerCommand( "instCallbackCmd", InstanceCallbackCmd::creator);
	MGlobal::displayInfo("PLUGIN LOADED");
	return status;
}

MStatus uninitializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj );
	plugin.deregisterCommand( "instCallbackCmd" );
	return status;
}
