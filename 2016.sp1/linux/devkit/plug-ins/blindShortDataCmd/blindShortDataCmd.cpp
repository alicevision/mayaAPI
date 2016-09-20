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
// Adds a dynamic attribute to selected dependency nodes.
// Dynamic attributes can be used as blind data.
//
// Usage: Load this plugin.
//        Select an object in Maya, and execute the MEL command
//		  blindShortData.  To see the new attribute, bring up the
//		  Attribute Editor window and look under "Extras" tab.
//
////////////////////////////////////////////////////////////////////////

#include <maya/MIOStream.h>

#include <maya/MFnPlugin.h>
#include <maya/MString.h>
#include <maya/MArgList.h>

#include <maya/MPxCommand.h>

#include <maya/MGlobal.h>
#include <maya/MItSelectionList.h>
#include <maya/MSelectionList.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MFnNumericAttribute.h>

class blindShortData : public MPxCommand
{
public:
					blindShortData() {};
	virtual			~blindShortData(); 

	MStatus			doIt( const MArgList& args );
	virtual MStatus	redoIt();
	static void*	creator();
};

blindShortData::~blindShortData() {}

void* blindShortData::creator()
{
	return new blindShortData();
}

MStatus blindShortData::doIt( const MArgList& )
{
	MStatus stat = redoIt();
	return stat;
}

MStatus blindShortData::redoIt()
//     This method performs the action of the command.
//     
//     Iterate over all selected items and for each attach a new
//		dynamic attribute. 
{
	MStatus stat;			// Status code
	MObject dependNode;		// Selected dependency node

	// Create a selection list iterator
	//
	MSelectionList slist;
	MGlobal::getActiveSelectionList( slist );
	MItSelectionList iter( slist, MFn::kInvalid, &stat );

	// Iterate over all selected dependency nodes
	//
	for ( ; !iter.isDone(); iter.next() ) 
	{
		// Get the selected dependency node and create
		// a function set for it
		//
		if ( MS::kSuccess != iter.getDependNode( dependNode ) ) {
			cerr << "Error getting the dependency node" << endl;
			continue;
		}
		MFnDependencyNode fnDN( dependNode, &stat );
		if ( MS::kSuccess != stat ) {
			cerr << "Error creating MFnDependencyNode" << endl;
			continue;
		}

		// Create a new attribute to use as out blind data
		//
		MFnNumericAttribute fnAttr;
		const MString fullName( "blindData" );
		const MString briefName( "bd" );
		double attrDefault = 99;
		MObject newAttr = fnAttr.create( 	fullName, briefName, 
											MFnNumericData::kShort, 
											attrDefault, &stat );
		if ( MS::kSuccess != stat ) {
			cerr << "Error creating new attribute" << endl;
			continue;
		}

		// Now add the new attribute to this dependency node
		//
		stat = fnDN.addAttribute(newAttr,MFnDependencyNode::kLocalDynamicAttr);
		if ( MS::kSuccess != stat ) {
			cerr << "Error adding dynamic attribute" << endl;
		}

	}

	return MS::kSuccess;
}


//////////////////////////////////////////////////////////////////////////////
//
// The following routines are used to register/unregister
// the command we are creating within Maya
//
//////////////////////////////////////////////////////////////////////////////
MStatus initializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");

	status = plugin.registerCommand( "blindShortData",
									 blindShortData::creator );
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

	status = plugin.deregisterCommand( "blindShortData" );
	if (!status) {
		status.perror("deregisterCommand");
		return status;
	}

	return status;
}
