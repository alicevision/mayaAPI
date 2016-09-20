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

	file : undoRedoMsgCmd.cpp
	class: undoRedoMsg
	----------------------
	This is an example to demonstrate  how to listen to undo and redo
	message events.  
	
	The syntax of the command is:
		undoRedoMsg add;	
		undoRedoMsg remove;
	The add argument causes listening to undo/redo to be turned on.
	The remove argument causes undo/redo listening to be removed.
*/

#include <maya/MFnPlugin.h>
#include <maya/MStatus.h>
#include <maya/MPxCommand.h>
#include <maya/MMessage.h>
#include <maya/MEventMessage.h>
#include <maya/MGlobal.h>
#include <maya/MArgList.h>
#include <maya/MCallbackIdArray.h>

#define UndoString "Undo"
#define RedoString "Redo"

// Static array to track the callback ids used
static MCallbackIdArray callbackIds;

//
// The command which adds and removes the
// callback listening
//
class undoRedoMsg : public MPxCommand
{
	public:
		virtual MStatus doIt( const MArgList& );
		void runTests();

		static void *creator();
};

//
// Two callbacks - undo/redo
//		These callbacks should not change
//		the state of the scene.  You can
//		update UI for modify local variables
//		etc. in these callbacks.
//
static void undoCB( void *clientData )
{
	MString info = "undoCallback : clientData = ";
	if ( clientData )
		info += (unsigned)(MUintPtrSz)clientData;
	else
		info += "NULL"; 
	MGlobal::displayInfo( info );
}

static void redoCB( void *clientData )
{
	MString info = "redoCallback : clientData ";
	if ( clientData )
		info += (unsigned)(MUintPtrSz) clientData;
	else
		info += "NULL"; 
	MGlobal::displayInfo( info );
}

MStatus undoRedoMsg::doIt( const MArgList& args )
{
	MStatus result = MS::kSuccess;
	
	if ( args.length() > 0 )
	{
		MString argStr;

		unsigned last = args.length();
		for ( unsigned i = 0; i < last; i++ ) 
		{
			args.get( i, argStr );

			if ( argStr == "add" )
			{
				MCallbackId undoId = MEventMessage::addEventCallback( UndoString, undoCB, NULL, &result );
				if ( MS::kSuccess != result )
					return MS::kFailure;
				callbackIds.append( undoId );
				
				MCallbackId redoId = MEventMessage::addEventCallback( RedoString, redoCB, NULL, &result );
				if ( MS::kSuccess != result )
					return MS::kFailure;	
				callbackIds.append( redoId );
			}
			else if ( argStr == "remove" )
			{
				if ( MS::kSuccess != MMessage::removeCallbacks( callbackIds ) )
					return MS::kFailure;
			}
			else
			{
				MGlobal::displayInfo("Failure condition");
				return MS::kFailure;
			} 
		}
	}

	return result;
}

//
// Creator for the command
//
void* undoRedoMsg::creator() 
{	
	return new undoRedoMsg;
}

// standard initialize and uninitialize functions

MStatus initializePlugin(MObject obj)
{
	// Version number may need to change in the future
	//
	MFnPlugin pluginFn(obj, PLUGIN_COMPANY, "6.0");

	MStatus status;
	status = pluginFn.registerCommand("undoRedoMsg", undoRedoMsg::creator);

	if( !status)
		status.perror("register Command failed");

	return status;
}

MStatus uninitializePlugin ( MObject obj )
{
	MFnPlugin pluginFn(obj);
	MStatus status = MS::kSuccess;
	
	// Remove the callbacks in case the command for doing
	// this has not been done.
	MMessage::removeCallbacks( callbackIds );
	
	status = pluginFn.deregisterCommand("undoRedoMsg");
	
	return status;
}

