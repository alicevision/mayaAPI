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
// polyMessageCmd.cpp
//
// Description:
//     Sample plug-in that demonstrates how to register/de-register
//     a callback with the MPolyMessage class.
//
//     This plug-in will register a new command in maya called
//     "polyMessage" which adds a callback for the all nodes on
//     the active selection list. A message is printed to stdout
//     whenever a component id of one of the poly nodes is modified.
//
#include <maya/MIOStream.h>
#include <maya/MPxCommand.h>
#include <maya/MFnPlugin.h>
#include <maya/MArgList.h>
#include <maya/MUintArray.h>
#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>
#include <maya/MPlug.h>
#include <maya/MPolyMessage.h>
#include <maya/MDagPath.h>
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

void userCB( MUintArray componentIds[], unsigned int count, void *clientData )
{
	cout << "poly component id modified";
	cout << endl;
	
	if ( count != MPolyMessage::kLastErrorIndex )
		return;
	
	unsigned int i, id;
	unsigned int kDeletedId = MPolyMessage::deletedId();
	MUintArray vertexIds = componentIds[MPolyMessage::kVertexIndex];
	for ( i = 0; i < vertexIds.length(); i++ )
	{
		id = vertexIds[i];
		if ( id == kDeletedId )
			cout << "vertex " << i << " deleted" << endl;
		else if ( i != id )
			cout << "vertex " << i << " " << id << endl;
	}
	
	MUintArray edgeIds = componentIds[MPolyMessage::kEdgeIndex];
	for ( i = 0; i < edgeIds.length(); i++ )
	{
		id = edgeIds[i];
		if ( id == kDeletedId )
			cout << "edge " << i << " deleted"  << endl;
		else if ( i != id )
			cout << "edge " << i << " " << id << endl;
	}
	
	MUintArray faceIds = componentIds[MPolyMessage::kFaceIndex];
	for ( i = 0; i < faceIds.length(); i++ )
	{
		id = faceIds[i];
		if ( id == kDeletedId )
			cout << "face " << i << " deleted" << endl;
		else if ( i != id )
			cout << "face " << i << " " << id << endl;
	}
}

//////////////////////////////////////////////////////////////////////////
//
// Command class declaration
//
//////////////////////////////////////////////////////////////////////////

class polyMessageCmd : public MPxCommand
{
public:
					polyMessageCmd() {};
	virtual			~polyMessageCmd(); 
	MStatus			doIt( const MArgList& args );
	static void*	creator();
};

//////////////////////////////////////////////////////////////////////////
//
// Command class implementation
//
//////////////////////////////////////////////////////////////////////////

polyMessageCmd::~polyMessageCmd() {}

void* polyMessageCmd::creator()
{
	return new polyMessageCmd();
}

MStatus polyMessageCmd::doIt( const MArgList& )
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
        
        MDagPath dp;
        MObject shapeNode = node;
        if ( MS::kSuccess == MDagPath::getAPathTo( node, dp ) )
			if ( MS::kSuccess == dp.extendToShape() )
				shapeNode = dp.node();
        
        bool wantIdChanges[3];
        wantIdChanges[MPolyMessage::kVertexIndex] = true;
        wantIdChanges[MPolyMessage::kEdgeIndex] = true;        
        wantIdChanges[MPolyMessage::kFaceIndex] = true;
        
	    id = MPolyMessage::addPolyComponentIdChangedCallback( shapeNode,
														wantIdChanges, 3,
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
	    	cout << "MPolyMessage.addCallback failed\n";
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
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "6.0", "Any");

	return plugin.registerCommand( "polyMessage", polyMessageCmd::creator );
}

MStatus uninitializePlugin( MObject obj)
{
	// Remove all callbacks
	//
	for (unsigned int i=0; i<callbackIds.length(); i++ ) {
		MMessage::removeCallback( (MCallbackId)callbackIds[i] );
	}

	MFnPlugin plugin( obj );
	return plugin.deregisterCommand( "polyMessage" );
}

