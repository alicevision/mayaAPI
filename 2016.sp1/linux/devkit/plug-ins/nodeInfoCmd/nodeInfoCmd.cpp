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
// node_info.cc
//
// This plugin prints out type and connected plug information for
// the selected dependency nodes.
//
////////////////////////////////////////////////////////////////////////

#include <maya/MIOStream.h>
#include <maya/MFnPlugin.h>
#include <maya/MString.h>
#include <maya/MStringResource.h>
#include <maya/MStringResourceId.h>
#include <maya/MArgList.h>

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>

#include <maya/MGlobal.h>
#include <maya/MDagPath.h>
#include <maya/MItSelectionList.h>
#include <maya/MSelectionList.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MPlugArray.h>
#include <maya/MPlug.h>

#define kQuietFlag		"-q"
#define kQuietFlagLong	"-quiet"

// String resource constants and registration

#define rPluginId  "nodeInfoCmd"
#define rConnFound MStringResourceId(rPluginId, "rConnFound", \
									"Number of connections found: ^1s")
#define rPlugInfo  MStringResourceId(rPluginId, "rPlugInfo", \
									"  Plug Info: ")
#define rPlugDestOf MStringResourceId(rPluginId, "rPlugDestOf",\
									"    This plug is a dest of: ")

static MStatus registerMStringResources(void)
{
	MStringResource::registerString(rConnFound);
	MStringResource::registerString(rPlugInfo);
	MStringResource::registerString(rPlugDestOf);
	return MS::kSuccess;
}

///////////////////////////////////////////////////
//
// Command class declaration
//
///////////////////////////////////////////////////
class nodeInfo : public MPxCommand
{
public:
					nodeInfo();
	virtual			~nodeInfo(); 

	MStatus			doIt( const MArgList& args );

	static MSyntax	newSyntax();
	static void*	creator();

private:
	void			printType( const MObject& node, const MString& prefix );
	MStatus			parseArgs( const MArgList& args );
	bool			quiet;
};



///////////////////////////////////////////////////
//
// Command class implementation
//
///////////////////////////////////////////////////

nodeInfo::nodeInfo()
	: quiet (false)
{}

nodeInfo::~nodeInfo() {}

void* nodeInfo::creator()
{
	return new nodeInfo;
}

MSyntax nodeInfo::newSyntax()
{
	 MSyntax syntax;
	 syntax.addFlag(kQuietFlag, kQuietFlagLong);
	 return syntax;
}

MStatus nodeInfo::parseArgs( const MArgList& args )
{
	MStatus			stat;
	MArgDatabase	argData(syntax(), args);

	if (argData.isFlagSet(kQuietFlag))
		quiet = true;

	return stat;
}



MStatus nodeInfo::doIt( const MArgList& args )
//
// Description
//     This method performs the action of the command.
//
//     This method iterates over all selected items and
//     prints out connected plug and dependency node type
//     information.
//
{
	MStatus stat;			// Status code
	MObject dependNode;		// Selected dependency node

	stat = parseArgs ( args );
	if ( !stat )
		return stat;

	// Create a selection list iterator
	//
	MSelectionList slist;
	MGlobal::getActiveSelectionList( slist );
	MItSelectionList iter( slist, MFn::kInvalid,&stat );


	// Iterate over all selected dependency nodes
	//
	for ( ; !iter.isDone(); iter.next() ) 
	{
		// Get the selected dependency node
		//
		stat = iter.getDependNode( dependNode );
		if ( !stat ) {
			stat.perror("getDependNode");
			continue;
		}

		// Create a function set for the dependency node
		//
		MFnDependencyNode fnDependNode( dependNode );

		// Check the type of the dependency node
		//
		MString nodeName = fnDependNode.name();
		nodeName += ": ";
		printType( dependNode, nodeName );

		// Get all connected plugs to this node
		//
		MPlugArray connectedPlugs;
		stat = fnDependNode.getConnections( connectedPlugs );

		int numberOfPlugs = connectedPlugs.length();
		if ( !quiet )
		{
			MString msgFmt = MStringResource::getString(rConnFound, stat);
			MString msg;
			MString arg;
			arg.set(numberOfPlugs);
			msg.format(msgFmt, arg);
			cerr << msg << endl;
		}

		// Print out the dependency node name and attributes
		// for each plug
		//
		MString pInfoMsg = MStringResource::getString(rPlugInfo, stat);
		for ( int i=0; i<numberOfPlugs; i++ ) 
		{
			MPlug plug = connectedPlugs[i];
			MString pinfo = plug.info();
			if ( !quiet )
			{
				cerr << pInfoMsg << pinfo << endl;
			}

			// Now get the plugs that this plug is the
			// destination of and print the node type.
			//
			MPlugArray array;
			plug.connectedTo( array, true, false );

			MString dInfoMsg = MStringResource::getString(rPlugDestOf, stat);
			for ( unsigned int j=0; j<array.length(); j++ )
			{
				MObject mnode = array[j].node();
				printType( mnode, dInfoMsg );
			}
		}
	}
	return MS::kSuccess;
}

void nodeInfo::printType( const MObject& node, const MString& prefix )
//
// Description
//     This method prints the type of the given node.
//     prefix is a string that gets printed before the type name.
//
{
	if ( !quiet)
		cerr << prefix << node.apiTypeStr() << endl;

}



/////////////////////////////////////////////////////////////////
//
// The following routines are used to register/unregister
// the command we are creating within Maya
//
/////////////////////////////////////////////////////////////////
MStatus initializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");

	status = plugin.registerUIStrings(registerMStringResources, "nodeInfoCmdInitStrings");
	if (!status) {
		status.perror("registerUIStrings");
		return status;
	}
	status = plugin.registerCommand( "nodeInfo",
									  nodeInfo::creator,
									  nodeInfo::newSyntax);
	if (!status) {
		status.perror("registerCommand");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus	  status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterCommand( "nodeInfo" );
	if (!status) {
		status.perror("deregisterCommand");
		return status;
	}

	return status;
}
