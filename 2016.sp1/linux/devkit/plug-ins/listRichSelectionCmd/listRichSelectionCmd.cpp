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
// moveTool.cc
// 
// Description:
//    Interactive tool for moving objects and components.
//
//    This plug-in will register the following two commands in Maya:
//       moveToolCmd <x> <y> <z>
//       moveToolContext
// 
////////////////////////////////////////////////////////////////////////
#include <maya/MIOStream.h>
#include <stdio.h>
#include <stdlib.h>

#include <maya/MPxToolCommand.h>
#include <maya/MFnPlugin.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MRichSelection.h>
#include <maya/MItSelectionList.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MDagPath.h>
#include <maya/MItGeometry.h>
#include <maya/MWeight.h>
#include <maya/MFnComponent.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnDoubleIndexedComponent.h>
#include <maya/MFnTripleIndexedComponent.h>

#define CHECKRESULT(stat,msg)     \
	if ( MS::kSuccess != stat ) { \
		cerr << msg << endl;      \
	}

#define kVectorEpsilon 1.0e-3

/////////////////////////////////////////////////////////////
//
// The move command
//
// - this is a tool command which can be used in tool
//   contexts or in the MEL command window.
//
/////////////////////////////////////////////////////////////
#define		COMMAND_NAME	"listRichSelectionCmd"
#define		DOIT		0
#define		UNDOIT		1
#define		REDOIT		2

class listRichSelectionCmd : public MPxToolCommand
{
public:
	listRichSelectionCmd();
	virtual ~listRichSelectionCmd(); 

	MStatus     doIt( const MArgList& args );
	MStatus     redoIt();
	MStatus     undoIt();
	bool        isUndoable() const;
	MStatus		finalize();
	
public:
	static void* creator();

	void		setVector( double x, double y, double z);
private:
	MVector 	delta;	// the delta vectors
	MStatus 	action( int flag );	// do the work here
};

listRichSelectionCmd::listRichSelectionCmd( )
{
	setCommandString( COMMAND_NAME );
}

listRichSelectionCmd::~listRichSelectionCmd()
{}

void* listRichSelectionCmd::creator()
{
	return new listRichSelectionCmd;
}

bool listRichSelectionCmd::isUndoable() const
//
// Description
//     Set this command to be undoable.
//
{
	return true;
}

void listRichSelectionCmd::setVector( double x, double y, double z)
{
	delta.x = x;
	delta.y = y;
	delta.z = z;
}

MStatus listRichSelectionCmd::finalize()
//
// Description
//     Command is finished, construct a string for the command
//     for journalling.
//
{
    MArgList command;
    command.addArg( commandString() );
    command.addArg( delta.x );
    command.addArg( delta.y );
    command.addArg( delta.z );

	// This call adds the command to the undo queue and sets
	// the journal string for the command.
	//
    return MPxToolCommand::doFinalize( command );
}

MStatus listRichSelectionCmd::doIt( const MArgList& args )
//
// Description
// 		Test MItSelectionList class
//
{
	MStatus stat;
	MVector	vector( 1.0, 0.0, 0.0 );	// default delta
	unsigned i = 0;

	switch ( args.length() )	 // set arguments to vector
	{
		case 1:
			vector.x = args.asDouble( 0, &stat );
			break;
		case 2:
			vector.x = args.asDouble( 0, &stat );
			vector.y = args.asDouble( 1, &stat );
			break;
		case 3:
			vector = args.asVector(i,3);
			break;
		case 0:
		default:
			break;
	}
	delta = vector;

	return action( DOIT );
}

MStatus listRichSelectionCmd::undoIt( )
//
// Description
// 		Undo last delta translation
//
{
	return action( UNDOIT );
}

MStatus listRichSelectionCmd::redoIt( )
//
// Description
// 		Redo last delta translation
//
{
	return action( REDOIT );
}

MStatus listRichSelectionCmd::action( int flag )
//
// Description
// 		Do the actual work here to move the objects	by vector
//
{
	MStatus stat;
	MVector vector = delta;

	switch( flag )
	{
		case UNDOIT:	// undo
			vector.x = -vector.x;
			vector.y = -vector.y;
			vector.z = -vector.z;
			break;
		case REDOIT:	// redo
			break;
		case DOIT:		// do command
			break;
		default:
			break;
	}

	// Create a selection list iterator
	//
	MSelectionList slist;
	MRichSelection rs;
	MGlobal::getRichSelection( rs);

	// Translate all selected objects
	//
	rs.getSelection( slist);
	for( int side = 0; side < 2; side++)
	{
		slist.clear();
		if( side) 
			rs.getSymmetry( slist);
		else
			rs.getSelection( slist);

		if( !slist.isEmpty())
		{
			if( side) 
				MGlobal::displayInfo( "Symmetry:");
			else
				MGlobal::displayInfo( "Selection:");
			for ( MItSelectionList iter( slist, MFn::kInvalid); !iter.isDone(); iter.next() ) 
			{
				// Get path and possibly a component
				//
				MDagPath 	mdagPath;		// Item dag path
				MObject 	mComponent;		// Current component
				iter.getDagPath( mdagPath, mComponent );

				MGlobal::displayInfo( "   " + mdagPath.fullPathName());
				if( !mComponent.isNull())
				{
					MFnComponent componentFn( mComponent);
					if( componentFn.hasWeights())
					{
						int count = componentFn.elementCount();
						MStatus stat;
						MFnSingleIndexedComponent singleFn( mComponent, &stat );
						if ( MS::kSuccess == stat ) 
						{
							for( int i = 0; i < count; i++)
							{
								MWeight weight = componentFn.weight( i);
								MGlobal::displayInfo( MString( "      Component[") + singleFn.element( i) + MString( "] has influence weight ") + weight.influence() + MString( " and seam weight ") + weight.seam());
							}
						}
						MFnDoubleIndexedComponent doubleFn( mComponent, &stat );
						if ( MS::kSuccess == stat ) 
						{
							for( int i = 0; i < count; i++)
							{
								MWeight weight = componentFn.weight( i);
								int u, v;
								doubleFn.getElement( i, u, v);
								MGlobal::displayInfo( MString( "      Component[") + u + MString( ",") + v + MString( "] has influence weight ") + weight.influence() + MString( " and seam weight ") + weight.seam());
							}
						}
						MFnTripleIndexedComponent tripleFn( mComponent, &stat );
						if ( MS::kSuccess == stat ) 
						{
							for( int i = 0; i < count; i++)
							{
								int u, v, w;
								tripleFn.getElement( i, u, v, w);
								MWeight weight = componentFn.weight( i);
								MGlobal::displayInfo( MString( "      Component[") + u + MString( ",") + v + MString( ",") + w + MString( "] has influence weight ") + weight.influence() + MString( " and seam weight ") + weight.seam());
							}
						}
					}
				}
			}
		}
	}
	return MS::kSuccess;
}

///////////////////////////////////////////////////////////////////////
//
// The following routines are used to register/unregister
// the commands we are creating within Maya
//
///////////////////////////////////////////////////////////////////////
MStatus initializePlugin( MObject obj )
{
	MStatus		status;
	MFnPlugin	plugin( obj, PLUGIN_COMPANY, "9.0", "Any" );

	status = plugin.registerCommand( COMMAND_NAME, &listRichSelectionCmd::creator );
	if (!status) {
		status.perror("registerCommand");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj )
{
	MStatus		status;
	MFnPlugin	plugin( obj );

	status = plugin.deregisterCommand( COMMAND_NAME );
	if (!status) {
		status.perror("deregisterCommand");
		return status;
	}

	return status;
}
