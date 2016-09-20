//-
// ==========================================================================
// Copyright 2010 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

// Example Plugin: nodeIconCmd.cpp
//
// This plugin defines a command that associates an icon with one of more
// dependency nodes for display in Maya's user interface. One place that
// the icon will show up is in the DAG outliner. The node to operate on is
// either specified as the optional final argument to the command, or if
// no last argument is given the command operates on the selected nodes.
//
// USAGE:
//		nodeIcon [flags] [nodeName]
//
// FLAGS:
//		-edit(e)
//			Specifies that the nodeIcon command operates in edit mode.
//			If neither -query nor -edit is specified, -edit is assumed.
//
//		-query(q)
//			Specifies that the nodeIcon command operates in query mode.
//
//		-icon(i) [filename]
//			Specifies the name of the file containing the icon to be
//			assigned to all specified nodes. To set a node to use the default
//			Maya icon specify the empty string (e.g. ""). Icons must be of
//			type "png", and the filenames may either be absolute or else
//			 relative to the XBMLANGPATH environment variable.
//
// MEL EXAMPLES:
//		To assign the icon filename mySphereIcon.png to the node pSphereShape1,
//			nodeIcon -icon "C:/Temp/mySphereIcon.png" pSphereShape1;
//
//		To query the icon filename associated with pSphereShape1,
//			nodeIcon -q pSphereShap1;
//
//		To revert to the default icon, assign the empty string as the filename,
//			nodeIcon -icon "" pSphereShap1;
//
//		Instead of specifying the node name on the command line, we can
//		operate on the selected nodes:
//			select pSphereShape1 pTorusShape1;
//			nodeIcon -icon "C:/Temp/myQuadricIcon.png";
//

// HEADER FILES...
//
#include <maya/MFnPlugin.h> 
#include <maya/MArgDatabase.h> 
#include <maya/MPxCommand.h> 
#include <maya/MGlobal.h> 
#include <maya/MIOStream.h> 
#include <maya/MSyntax.h> 
#include <maya/MSelectionList.h> 
#include <maya/MFnDependencyNode.h> 


// COMMAND FLAGS...
//
#define	kFlagIcon				"-i"
#define	kFlagIconLong			"-icon"


// CLASS...
//
class nodeIcon : public MPxCommand
{
public:
					nodeIcon();
	virtual			~nodeIcon();
	
	virtual bool	hasSyntax();
	static MSyntax	newSyntax();

	static void		*creator();

	MStatus			doIt( const MArgList &args );
};


// METHODS...
//

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//		Constructor for the nodeIcon class. This is the default
// constructor.
//
//////////////////////////////////////////////////////////////////////////////


nodeIcon::nodeIcon()
{
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//		Destructor for the nodeIcon class. It gets called when
// the node is being deleted.
//
//////////////////////////////////////////////////////////////////////////////


nodeIcon::~nodeIcon()
{
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//		This method allows Maya to instantiate new nodeIcon.
//
// Returns:
//		void 	*pointer	: Returns the newly allocated nodeIcon
//							  object.
//		void	NULL		: Memory allocation or other error occurred.
//
//////////////////////////////////////////////////////////////////////////////


void	*nodeIcon::creator()
{
	return( new nodeIcon() );
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//		Specifies whether or not the command has a syntax object.
//
// Returns:
//		true				: The command has a syntax object.
//		false				: The command has no syntax object.
//
//////////////////////////////////////////////////////////////////////////////


bool	 nodeIcon::hasSyntax()
{
	return( true );
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//		Creates the syntax object for the nodeIcon command.
//
// Returns:
//		MSyntax				: The newly created syntax object.
//
//////////////////////////////////////////////////////////////////////////////


MSyntax	 nodeIcon::newSyntax()
{
	MSyntax		syntax;

	// Define the flags for this command.
	//
	syntax.addFlag( kFlagIcon,
			kFlagIconLong,
			MSyntax::kString );


	// Allow the user to select the nodes we will operate on, as well as allow
	// him/her to specify the node on the command line.
	//
	syntax.useSelectionAsDefault( true );
	syntax.setObjectType( MSyntax::kSelectionList );
	syntax.setMinObjects( 1 );

	// This command is both queryable as well as editable.
	//
	syntax.enableQuery( true );
	syntax.enableEdit( true );

	return( syntax );
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//		Performs the command by extracting the command-line arguments and
// acting upon their values.
//
// Returns:
//		MStatus	kSuccess		: Successfully executed the command.
//		MStatus	kFailure		: An error occurred.
//
//////////////////////////////////////////////////////////////////////////////


MStatus	nodeIcon::doIt( const MArgList &args )
{
	MString			iconName;
	MStatus			status;

	// Create the argument database from the args parameter.
	//
	MArgDatabase	argData( syntax(), args, &status );
	if ( MStatus::kSuccess != status ) {
		// User supplied incorrect command-line arguments: they don't match
		// the syntax for the command. So just return the status.
		//
		return( status );
	}

	// See what flags were specified. Edit is the default.
	//
	bool	queryUsed = argData.isQuery();
	bool	iconUsed = argData.isFlagSet( kFlagIcon );

	// Get the objects from the global selection list.
	//
	MSelectionList sList;
	status = argData.getObjects( sList );
	if ( MS::kSuccess != status ) {
		MGlobal::displayError(
				"nodeIcon: could not query the selection list" );
		return( MS::kFailure );
	}
	int	count = sList.length();
	if ( MS::kSuccess != status || 0 == count ) {
		MGlobal::displayError(
				"nodeIcon: you need to specify at least one node" );
		return( MS::kFailure );
	}

	// Get any flag arguments.
	//
	if ( iconUsed ) {
		CHECK_MSTATUS_AND_RETURN_IT( argData.getFlagArgument(
				kFlagIcon, 0, iconName ) );
	} else if ( !queryUsed ) {
		MGlobal::displayError(
				"nodeIcon: the -icon flag needs to be specified in edit mode" );
		return( MS::kFailure );
	}

	// Query or set the node icon for each node in the selection list.
	//
	int	i;
	for ( i = 0; i < count; i++ ) {
		// Get the node from the selection list.
		//
		MObject	node;
		status = sList.getDependNode( i, node );
		if ( MS::kSuccess != status ) {
			MGlobal::displayError(
					"nodeIcon: only nodes can be selected" );
			return( MS::kFailure );
		}
		MFnDependencyNode nodeFn( node );

		// If query, return the name of the custom nodeIcon assigned
		// to the node (if one is defined). If editing, assign the icon
		// file name specified via the -icon flag (which is empty if the
		// user omitted -icon).
		//
		MString result;
		if ( queryUsed ) {
			result = nodeFn.icon( &status );
			if ( MS::kSuccess != status ) {
				MGlobal::displayError(
						"nodeIcon: could not query the icon assigned to the node" );
				return( MS::kFailure );
			}
		} else {
			status = nodeFn.setIcon( iconName );
			if ( MS::kSuccess != status ) {
				MGlobal::displayError(
						"nodeIcon: the filename specified by the -icon flag could not be opened" );
				return( status );
			}
			result = iconName;
		}
		MPxCommand::appendToResult( result );
	}

	return( status );
}

MStatus	initializePlugin( MObject obj )
{
	MStatus		status;
	MFnPlugin	plugin( obj, PLUGIN_COMPANY, "10.0", "Any" );

	status = plugin.registerCommand( "nodeIcon", nodeIcon::creator,
			nodeIcon::newSyntax );

	if ( !status ) {
		MGlobal::displayError(
				"nodeIcon: failed to register the plug-in" );
	}

	return( status );
}

MStatus	uninitializePlugin( MObject obj )
{
	MStatus		status;
	MFnPlugin	plugin( obj );

	status = plugin.deregisterCommand( "nodeIcon" );

	if ( !status ) {
		MGlobal::displayError(
				"nodeIcon: failed to deregister the plug-in" );
	}

	return( status );
}
