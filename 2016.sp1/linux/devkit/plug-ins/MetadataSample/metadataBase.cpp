///////////////////////////////////////////////////////////////////////////////
//
// base MEL command for multiple rep shape
//
////////////////////////////////////////////////////////////////////////////////

#include "metadataBase.h"
#include "metadataPluginStrings.h"

#include <maya/MArgList.h>
#include <maya/MSyntax.h>
#include <maya/MSelectionList.h>
#include <maya/MFileObject.h>
#include <maya/adskDataAssociationsSerializer.h>

static const char* flagFile					( "-f" );
static const char* flagFileLong				( "-file" );
static const char* flagMetadataFormat		( "-mf" );
static const char* flagMetadataFormatLong	( "-metadataFormat" );

using namespace adsk::Data;

//============================================================================
//
// Get the syntax information. Initializes the shared flags. Derived commands
// can add on their own flags after calling this routine.
//
MSyntax metadataBase::cmdSyntax()
{
	MSyntax syntax;
    syntax.addFlag(flagMetadataFormat,  flagMetadataFormatLong,	MSyntax::kString);
    syntax.addFlag(flagFile,			flagFileLong,			MSyntax::kString);
	
	syntax.useSelectionAsDefault(true);
    syntax.setObjectType(MSyntax::kSelectionList, 1);

	// Default mode has no query or edit. Derived commands can
	// enable either if they are relevant
    syntax.enableQuery( false );
    syntax.enableEdit( false );

    return syntax;
}

//======================================================================
//
metadataBase::metadataBase()
: fMode(kCreate)
, fSerializer( (const AssociationsSerializer*)0 )
, fFile( (MFileObject*) 0 )
, fObjects()
{
}

//======================================================================
//
metadataBase::~metadataBase()
{
	// If a file object was created it should go away
	delete fFile;
}

//======================================================================
//
// Normally data stream commands are not undoable. Derived classes can
// override and provide undo information.
//
bool metadataBase::isUndoable() const
{
    return false;
}

//======================================================================
//
bool metadataBase::hasSyntax() const
{
    return true;
}

//======================================================================
//
// Look through the arg database and verify that the arguments are
// valid. Only checks the common flags so derived classes should call
// this parent method first before checking their own flags.
//
MStatus metadataBase::checkArgs(MArgDatabase& argsDb)
{
	MStatus status;

	//----------------------------------------
	// Save the command arguments and modes for undo/redo purposes.
	if (argsDb.isEdit())
	{
		if (argsDb.isQuery())
		{
			MString msg = MStringResource::getString(kEditQueryFlagErrorMsg, status);
			displayError(msg);
			return MS::kFailure;
		}
		fMode = kEdit;
	}
	else if (argsDb.isQuery())
	{
		fMode = kQuery;
	}

	//----------------------------------------
	// -file flag
	//
	// Initialize the local file object for use if the flag is specified
	//
	fFileFlag.parse(argsDb, flagFile);
	if( !fFileFlag.isModeValid(fMode) )
	{
		MString fmt = MStringResource::getString(kOnlyCreateModeMsg, status);
		MString msg;
		msg.format( fmt, flagFile );
		displayError(msg);
		return MS::kFailure;
	}

	// The file flag isn't mandatory so just initialize it if it exists.
	if( fFileFlag.isSet() )
	{
		if( ! fFileFlag.isArgValid() )
		{
			MString fmt = MStringResource::getString(kInvalidFlag, status);
			MString msg;
			msg.format( fmt, flagFileLong );
			displayError(msg);
			return MS::kFailure;
		}
		fFile = new MFileObject();
		fFile->setRawFullName( fFileFlag.arg() );
	}

	// The file flag may indicate an existing file. The derived commands
	// should verify existence when required.

	//----------------------------------------
	// -metadataFormat flag
	//
	// Initialize the serializer if the type is valid. Method will return
	// failure and display an informative error message if a valid
	// serialization type was not specified
	//
	fMetadataFormatFlag.parse(argsDb, flagMetadataFormat);
	if( !fMetadataFormatFlag.isModeValid(fMode) )
	{
		MString fmt = MStringResource::getString(kOnlyCreateModeMsg, status);
		MString msg;
		msg.format( fmt, flagFile );
		displayError(msg);
		return MS::kFailure;
	}

	// Default to the internal "raw" format. Bit of a cheat to use this string
	// directly but there's no way to get it indirectly.
	MString rawFormatType( "raw" );
	MString formatType;
	formatType = fMetadataFormatFlag.arg( rawFormatType );

	fSerializer = AssociationsSerializer::formatByName( formatType.asChar() );
	if( ! fSerializer )
	{
		MString fmt = MStringResource::getString(kMetadataFormatNotFound, status);
		MString msg;
		msg.format( fmt, fMetadataFormatFlag.arg().asChar() );
		displayError(msg);
		status = MS::kNotFound;
		return MS::kFailure;
	}

	//----------------------------------------
	// (selection list)
	//
	// Commands need at least one object on which to operate so gather up
	// the list of objects specified and/or selected.
	//

	// Empty out the list of objects on which to operate so that it can be
	// populated from the selection or specified lists.
	fObjects.clear();

	MSelectionList objects;
	status = argsDb.getObjects(objects);
	MStatError(status, "argsDb.getObjects()");
	for (unsigned int i = 0; i<objects.length(); ++i)
	{
		MObject dgNode;
		status = objects.getDependNode(i, dgNode);
		MStatError(status, "objects.getDependNode()");	

		fObjects.append( dgNode );
	}

	if( (fObjects.length() == 0) && (fMode != kQuery) )
	{
		MString msg = MStringResource::getString(kObjectNotFoundError, status);
		displayError(msg);
		return MStatus::kFailure;
	}

	return status;
}

//======================================================================
//
// Check the mode information and call the appropriate virtual method
// to perform the operation. It checks for all modes, even those that
// might be disabled, so that it can be reused anywhere.
//
MStatus metadataBase::doIt(const MArgList& args)
{
	MStatus status;

	MArgDatabase argsDb(syntax(), args, &status);
	if (MS::kSuccess != status) return status;

    status = checkArgs(argsDb);
	if( MS::kSuccess == status )
	{
		clearResult();
		switch (fMode)
		{
			case kCreate:  status = doCreate();   break;
			case kEdit:    status = doEdit();     break;
			case kQuery:   status = doQuery();    break;
		}
	}
	return status;
}

//======================================================================
//
// Since this isn't a real command it doesn't do anything. This method
// is defined anyway so that the derived commands can choose to override
// or not.
//
MStatus metadataBase::doCreate()
{
    return MS::kSuccess;
}

//======================================================================
//
// Since this isn't a real command it doesn't do anything. This method
// is defined anyway so that the derived commands can choose to override
// or not.
//
MStatus metadataBase::doEdit()
{
    return MS::kSuccess;
}

//======================================================================
//
// Since this isn't a real command it doesn't do anything. This method
// is defined anyway so that the derived commands can choose to override
// or not.
//
MStatus metadataBase::doQuery()
{
    return MS::kSuccess;
}

//======================================================================
//
// Since this isn't a real command it doesn't do anything. This method
// is defined anyway so that the derived commands can choose to override
// or not.
//
MStatus metadataBase::redoIt()
{
    return MS::kSuccess;
}

//======================================================================
//
// Since this isn't a real command it doesn't do anything. This method
// is defined anyway so that the derived commands can choose to override
// or not.
//
MStatus metadataBase::undoIt()
{
    return MS::kSuccess;
}

//-
// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//+

