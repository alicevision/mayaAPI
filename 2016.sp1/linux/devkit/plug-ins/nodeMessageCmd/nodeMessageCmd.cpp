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

//
// nodeMessageCmd.cc
//
// Description:
//     Sample plug-in that demonstrates how to register/de-register
//     a callback with the MNodeMessage class.
//
//     This plug-in will register a new command in maya called
//     "nodeMessage" which adds a callback for the all nodes on
//     the active selection list. A message is printed to stdout 
//     whenever a connection is made or broken for those nodes.
//
#include <maya/MIOStream.h>
#include <maya/MPxCommand.h>
#include <maya/MFnPlugin.h>
#include <maya/MArgList.h>
#include <maya/MIntArray.h>
#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>
#include <maya/MPlug.h>
#include <maya/MNodeMessage.h>
#include <maya/MCallbackIdArray.h>

// This table will keep track of the registered callbacks
// so they can be removed when the plug-ins is unloaded.
//
MCallbackIdArray callbackIds;

//////////////////////////////////////////////////////////////////////////
//
// Callback function
//
//    Prints out plug information when connections are made or broken.
//    See MNodeMessage.h for all of the available AttributeMessage types.
//
//////////////////////////////////////////////////////////////////////////

void userCB( MNodeMessage::AttributeMessage msg, MPlug & plug,
             MPlug & otherPlug, void* )
{
	if ( msg & MNodeMessage::kConnectionMade ) {
		cout << "Connection made ";
	} else if ( msg & MNodeMessage::kConnectionBroken ) {
		cout << "Connection broken ";
	} else {
		return;
	}
	cout << plug.info();
	if ( msg & MNodeMessage::kOtherPlugSet ) {
		if ( msg & MNodeMessage::kIncomingDirection ) {
				cout << "  <--  " << otherPlug.info();
		} else {
				cout << "  -->  " << otherPlug.info();
		}
	}
	cout << endl;
}

//////////////////////////////////////////////////////////////////////////
//
// Command class declaration
//
//////////////////////////////////////////////////////////////////////////

class nodeMessageCmd : public MPxCommand
{
public:
					nodeMessageCmd() {};
	virtual			~nodeMessageCmd(); 
	MStatus			doIt( const MArgList& args );
	static void*	creator();
};

//////////////////////////////////////////////////////////////////////////
//
// Command class implementation
//
//////////////////////////////////////////////////////////////////////////

nodeMessageCmd::~nodeMessageCmd() {}

void* nodeMessageCmd::creator()
{
	return new nodeMessageCmd();
}

MStatus nodeMessageCmd::doIt( const MArgList& )
//
// Takes the  nodes that are on the active selection list and adds an
// attriubte changed callback to each one.
//
{
	MStatus 		stat;
	MObject 		node;
	MSelectionList 	list;
    MCallbackId     id;
    
	// Register node callbacks for all nodes on the active list.
	//
	MGlobal::getActiveSelectionList( list );

    for ( unsigned int i=0; i<list.length(); i++ )
    {
        list.getDependNode( i, node );
        
	    id = MNodeMessage::addAttributeChangedCallback( node,
                                                        userCB,
                                                        NULL,
                                                        &stat);

    	// If the callback was successfully added then add the
        // callback id to our callback table so it can be removed
        // when the plugin is unloaded.
	    //
	    if ( stat ) {
		    callbackIds.append( id );
    	} else {
	    	cout << "MNodeMessage.addCallback failed\n";
    	}
    }
        
	return stat;
}

//////////////////////////////////////////////////////////////////////////
//
// Plugin registration
//
//////////////////////////////////////////////////////////////////////////

MStatus initializePlugin( MObject obj )
{
	MFnPlugin plugin( obj );
	return plugin.registerCommand( "nodeMessage", nodeMessageCmd::creator );
}

MStatus uninitializePlugin( MObject obj)
{
	// Remove all callbacks
	//
	for (unsigned int i=0; i<callbackIds.length(); i++ ) {
		MMessage::removeCallback( (MCallbackId)callbackIds[i] );
	}

	MFnPlugin plugin( obj );
	return plugin.deregisterCommand( "nodeMessage" );
}
