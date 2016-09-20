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

#include <maya/MFnTransform.h>
#include <maya/MItGeometry.h>

#include <maya/MWeight.h>
#include <maya/MMatrix.h>
#include <maya/MPlane.h>

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
#define		RICHMOVENAME	"richMoveCmd"
#define		DOIT		0
#define		UNDOIT		1
#define		REDOIT		2

class richMoveCmd : public MPxToolCommand
{
public:
	richMoveCmd();
	virtual ~richMoveCmd(); 

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

richMoveCmd::richMoveCmd( )
{
	setCommandString( RICHMOVENAME );
}

richMoveCmd::~richMoveCmd()
{}

void* richMoveCmd::creator()
{
	return new richMoveCmd;
}

bool richMoveCmd::isUndoable() const
//
// Description
//     Set this command to be undoable.
//
{
	return true;
}

void richMoveCmd::setVector( double x, double y, double z)
{
	delta.x = x;
	delta.y = y;
	delta.z = z;
}

MStatus richMoveCmd::finalize()
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

MStatus richMoveCmd::doIt( const MArgList& args )
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

MStatus richMoveCmd::undoIt( )
//
// Description
// 		Undo last delta translation
//
{
	return action( UNDOIT );
}

MStatus richMoveCmd::redoIt( )
//
// Description
// 		Redo last delta translation
//
{
	return action( REDOIT );
}

MStatus richMoveCmd::action( int flag )
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
	MSpace::Space spc = MSpace::kWorld;
	MRichSelection rs;
	MGlobal::getRichSelection( rs);

	// Translate all selected objects
	//
	rs.getSelection( slist);
	if( !slist.isEmpty())
	{
		for ( MItSelectionList iter( slist, MFn::kInvalid); !iter.isDone(); iter.next() ) 
		{
			// Get path and possibly a component
			//
			MDagPath 	mdagPath;		// Item dag path
			MObject 	mComponent;		// Current component
			iter.getDagPath( mdagPath, mComponent );
			MPlane seam;
			rs.getSymmetryPlane( mdagPath, spc, seam);

			if( mComponent.isNull())
			{
				// Transform move
				MFnTransform transFn( mdagPath, &stat );
				if ( MS::kSuccess == stat ) {
					stat = transFn.translateBy( vector, spc );
					CHECKRESULT(stat,"Error doing translate on transform");
					continue;
				}
			}
			else
			{
				// Component move
				iter.getDagPath( mdagPath, mComponent );
				for( MItGeometry geoIter( mdagPath, mComponent); !geoIter.isDone(); geoIter.next())
				{
					MVector origPosition = geoIter.position( spc);
					MWeight weight = geoIter.weight();

					// Calculate the soft move
					MVector position = origPosition + vector * weight.influence();

					// Calculate the soft seam
					position += seam.normal() * (weight.seam() * (seam.directedDistance( origPosition) - seam.directedDistance( position)));
					geoIter.setPosition( position, spc);
				}
			}
		}
	}

	// Translate all symmetry objects
	//
	slist.clear();
	rs.getSymmetry( slist);
	if( !slist.isEmpty())
	{
		for ( MItSelectionList iter( slist, MFn::kInvalid); !iter.isDone(); iter.next() ) 
		{
			// Get path and possibly a component
			//
			MDagPath 	mdagPath;		// Item dag path
			MObject 	mComponent;		// Current component
			iter.getDagPath( mdagPath, mComponent );
			MPlane seam;
			rs.getSymmetryPlane( mdagPath, spc, seam);

			// Reflect our world space move
			//
			MMatrix symmetryMatrix;
			rs.getSymmetryMatrix( mdagPath, spc, symmetryMatrix);
			MVector symmetryVector =  vector * symmetryMatrix;

			if( mComponent.isNull())
			{
				// Transform move
				MFnTransform transFn( mdagPath, &stat );
				if ( MS::kSuccess == stat ) {
					stat = transFn.translateBy( symmetryVector, spc );
					CHECKRESULT(stat,"Error doing translate on transform");
					continue;
				}
			}
			else
			{
				// Component move
				iter.getDagPath( mdagPath, mComponent );
				for( MItGeometry geoIter( mdagPath, mComponent); !geoIter.isDone(); geoIter.next())
				{
					MVector origPosition = geoIter.position( spc);
					MWeight weight = geoIter.weight();

					// Calculate the soft move
					MVector position = origPosition + symmetryVector * weight.influence();

					// Calculate the soft seam
					position += seam.normal() * (weight.seam() * (seam.directedDistance( origPosition) - seam.directedDistance( position)));
					geoIter.setPosition( position, spc);
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

	status = plugin.registerCommand( RICHMOVENAME, &richMoveCmd::creator );
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

	status = plugin.deregisterCommand( RICHMOVENAME );
	if (!status) {
		status.perror("deregisterCommand");
		return status;
	}

	return status;
}
