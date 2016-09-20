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

	file : deletedMsgCmd.cpp
	class: deletedMessage
	----------------------
	
	This plugin demonstrates each of the node deletion callbacks available in 
	the Maya API.  Callbacks are added to nodes by invoking the command:

		deletedMessage <node 1> [<node 2> ...]
	
	This command will register 3 callbacks on the nodes.

	1) MNodeMessage::addNodeAboutToDeleteCallback is used to register an about
	   to delete callback on the nodes.  This callback is executed once when 
	   the deletion operation is first performed, and is used to add commands
	   to a DG modifier to be executed before the node is deleted.  Since the
	   operations performed by the DG modifier are undoable, when the node 
	   deletion is undone, the additional DG modifications added by this 
	   callback will also be undone or redone.
	
	2) MNodeMessage::addNodePreRemovalCallback is used to register a callback
	   that gets called whenever the deletion sequence is performed, whether 
	   it is the first time or on a redo of the delete.  This callback will be
	   called before any other changes are made as a result of the deletion,
	   such as disconnecting any connections on the node.

	3) MDGMessage::addNodeRemovedCallback is used to register a callback that
	   is called when the node is removed.  This callback will be received 
	   after the pre-removal callback, and after connections from the node are
	   disconnected.

	Here is an example of the expected behavior of the callbacks registered on 
	"nurbsSphere1" when the deletion is performed the first time:

		// Removal callback node: makeNurbSphere1
		// Removal callback node: nurbsSphereShape1
		// About to delete callback for node: nurbsSphere1
		// Pre-removal callback for node: nurbsSphere1
		// Removal callback node: nurbsSphere1

	Here is an example of the expected behavior of the callbacks when the 
	deletion is redone:

		// Removal callback node: makeNurbSphere1
		// Removal callback node: nurbsSphereShape1
		// Pre-removal callback for node: nurbsSphere1
		// Removal callback node: nurbsSphere1

*/

#include <maya/MFnPlugin.h>
#include <maya/MStatus.h>
#include <maya/MPxCommand.h>
#include <maya/MMessage.h>
#include <maya/MNodeMessage.h>
#include <maya/MDGMessage.h>
#include <maya/MGlobal.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <maya/MSyntax.h>
#include <maya/MSelectionList.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MDGModifier.h>

#include <maya/MPlugArray.h>
#include <maya/MPlug.h>
#include <maya/MCallbackIdArray.h>

// Syntax string definitions

class deletedMessage : public MPxCommand
{
	public:
		virtual MStatus doIt( const MArgList& );

		static void *creator();
		static MSyntax newSyntax();

		static void removeCallbacks();

		// callback functions.
		static void aboutToDeleteCB(MObject& node, MDGModifier& modifier, 
			void* clientData);
		static void preRemovalCB(MObject& node, void* clientData);	
		static void removeCB(MObject& node, void* clientData);

	private:
		static MCallbackIdArray callbackIds;
		static bool nodeRemovedCBRegistered;
};

MCallbackIdArray deletedMessage::callbackIds;
bool deletedMessage::nodeRemovedCBRegistered = false;

MStatus deletedMessage::doIt( const MArgList& args )
{
	MStatus status = MS::kSuccess;

	MArgDatabase argData(syntax(), args);
	MSelectionList objects;
	argData.getObjects(objects);

	for (unsigned int i = 0; i < objects.length(); i++) {
		MObject node;
		objects.getDependNode(i, node);
		
		callbackIds.append(
			MNodeMessage::addNodeAboutToDeleteCallback (node, aboutToDeleteCB, NULL, &status)
		);
		if (!status) {
			MGlobal::displayWarning("Could not attach about to delete callback for node.");
			continue;
		}
		callbackIds.append(
			MNodeMessage::addNodePreRemovalCallback (node, preRemovalCB, NULL, &status)
		);
		if (!status) {
			MGlobal::displayWarning("Could not attach pre-removal callback for node.");
			continue;
		}
		if (!nodeRemovedCBRegistered)
		{
			callbackIds.append(
				MDGMessage::addNodeRemovedCallback(removeCB, "dependNode", NULL, &status)
			);
			if (!status) {
				MGlobal::displayWarning("Could not attach node removal callback.");
				continue;
			}
			nodeRemovedCBRegistered = true;
		}
	}
	
	return status;
}


void* deletedMessage::creator() 
{	
	return new deletedMessage;
}

MSyntax deletedMessage::newSyntax() {
	MSyntax syntax;
	syntax.setObjectType(MSyntax::kSelectionList);
	syntax.setMinObjects(1);
	syntax.useSelectionAsDefault(true);
	return syntax;
}

void deletedMessage::aboutToDeleteCB(MObject& node, MDGModifier& modifier, void* clientData) {
	MFnDependencyNode nodeFn(node);
	MGlobal::displayInfo(MString("About to delete callback for node: ") + nodeFn.name());

	//
	// If there were any other operations on the DG that needed to be performed
	// before the node was removed, they could be added to the MDGModifier.
	// For example, attributes could be removed from other nodes or connections
	// on related nodes could be disconnected.  In this case there are no 
	// operations to be added so just return.
	//
}

void deletedMessage::preRemovalCB(MObject& node, void* clientData) {
	MFnDependencyNode nodeFn(node);
	MGlobal::displayInfo(MString("Pre-removal callback for node: ") + nodeFn.name());
}

void deletedMessage::removeCB(MObject& node, void* clientData) {
	MFnDependencyNode nodeFn(node);
	MGlobal::displayInfo(MString("Removal callback node: ") + nodeFn.name());
}

void deletedMessage::removeCallbacks() {
	MMessage::removeCallbacks(callbackIds);
}

// standard initialize and uninitialize functions

MStatus initializePlugin(MObject obj)
{
	MFnPlugin pluginFn(obj, PLUGIN_COMPANY, "6.0");

	MStatus status;
	status = pluginFn.registerCommand("deletedMessage", deletedMessage::creator, deletedMessage::newSyntax);

	if( !status)
		status.perror("register Command failed");

	return status;
}


MStatus uninitializePlugin ( MObject obj )
{
	MFnPlugin pluginFn(obj);
	MStatus status = MS::kSuccess;

	deletedMessage::removeCallbacks();

	status = pluginFn.deregisterCommand("deletedMessage");

	return status;
}

