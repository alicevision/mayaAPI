#include "importMetadataCmd.h"
#include "metadataPluginStrings.h"
#include <maya/MObject.h>
#include <maya/MSyntax.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MStringResource.h>
#include <maya/MArgDatabase.h>
#include <maya/MFileObject.h>
#include <maya/adskDataStream.h>
#include <maya/adskDataChannel.h>
#include <maya/adskDataAssociations.h>
#include <maya/adskDataAssociationsSerializer.h>
#include <maya/adskDataChannelSerializer.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>

using namespace adsk::Data;

// Flags specific to the import command
static const char* flagString		( "-s" );
static const char* flagStringLong	( "-string" );

//======================================================================
//
// Creator function: returns a new command object
//
void* importMetadataCmd::creator()
{
	return new importMetadataCmd;
}

//======================================================================
//
// Returns the name of this command
//
const char* importMetadataCmd::name()
{
	return "importMetadata";
}

//======================================================================
//
// Create default syntax and add command-specific syntax
//
MSyntax importMetadataCmd::cmdSyntax()
{
	MSyntax syntax = metadataBase::cmdSyntax();

    syntax.addFlag(flagString,	flagStringLong,	MSyntax::kString);

	return syntax;
}

//======================================================================
//
// Default command constructor
//
importMetadataCmd::importMetadataCmd()
{
}

//======================================================================
//
// Destructor, does nothing
//
importMetadataCmd::~importMetadataCmd()
{
}

//======================================================================
//
// This command is undoable (but must be in create mode)
//
bool
importMetadataCmd::isUndoable() const
{
	return fMode == kCreate;
}

//======================================================================
//
// Check the parsed arguments and do/undo/redo the command as appropriate
//
MStatus importMetadataCmd::checkArgs(MArgDatabase& argsDb)
{
	MStatus status = metadataBase::checkArgs( argsDb );
	if( status != MS::kSuccess ) return status;

	//----------------------------------------
	// -string flag
	//
	// If specified then the -file flag is not allowed.
	//
	fStringFlag.parse(argsDb, flagString);
	if( fStringFlag.isSet() )
	{
		if( fFile )
		{
			MString fmt = MStringResource::getString(kFileIgnored, status);
			MString msg;
			msg.format( fmt, flagString );
			displayWarning( msg );
		}

		if( !fStringFlag.isModeValid(fMode) )
		{
			MString fmt = MStringResource::getString(kOnlyCreateModeMsg, status);
			MString msg;
			msg.format( fmt, flagString );
			displayError(msg);
			return MS::kFailure;
		}
		if( ! fStringFlag.isArgValid() )
		{
			MString errMsg( MStringResource::getString(kInvalidString, status) );
			displayError( errMsg );
			return MS::kFailure;
		}
		fString = fStringFlag.arg();
	}
	else if( ! fFile )
	{
		MString fmt = MStringResource::getString(kFileOrStringNeeded, status);
		MString msg;
		msg.format( fmt, flagString );
		displayError( msg );
		return MS::kFailure;
	}
	else if( ! fFile->exists() )
	{
		MString fmt = MStringResource::getString(kFileNotFound, status);
		MString msg;
		msg.format( fmt, fFileFlag.arg().asChar() );
		displayError(msg);
		status = MS::kNotFound;
	}

	return status;
}

//======================================================================
//
// Do the import in create mode. The metadata will be retrieved from the
// file or string and imported onto the selected object(s) presuming the
// specified format. Successful execution will see the imported metadata on
// the object(s), returning the names of the newly created or modified streams
// in the format OBJECT/CHANNEL_TYPE/STREAM
//
// The previously existing metadata is preserved for later undo.
//
MStatus importMetadataCmd::doCreate()
{
	assert( fSerializer );
	std::string errors;

	Associations* associationsRead = (Associations*)0;
	MStatus status;

	if( fString.length() > 0 )
	{
		std::stringstream inStream( fString.asChar() );
		associationsRead = fSerializer->read( inStream, errors );
		if( ! associationsRead )
		{
			MString fmt = MStringResource::getString(kImportMetadataStringReadFailed, status);
			MString msg;
			MString err( errors.c_str() );
			msg.format( fmt, err );
			displayError(msg);
			return MS::kFailure;
		}
	}
	else if( fFile )
	{
		std::ifstream inStream( fFile->resolvedFullName().asChar() );
		associationsRead = fSerializer->read( inStream, errors );
		inStream.close();
		if( ! associationsRead )
		{
			MString fmt = MStringResource::getString(kImportMetadataFileReadFailed, status);
			MString msg;
			MString err( errors.c_str() );
			msg.format( fmt, fFile->resolvedFullName(), err );
			displayError(msg);
			return MS::kFailure;
		}
	}
	else
	{
		// This isn't a recoverable error since this situation should have
		// been reported in the arg checking. Just assert and fail
		// immediately.
		assert( fFile );
		return MS::kFailure;
	}

	// Should have already handled this
	assert( associationsRead );

	MString resultFmt = MStringResource::getString(kImportMetadataResult, status);
	for( unsigned int i=0; i<fObjects.length(); ++i )
	{
		MFnDependencyNode node( fObjects[i], &status );
		// Should have filtered out non-objects already but check anyway
		if( MS::kSuccess != status ) continue;

		displayInfo( node.name(&status) );

		fDGModifier.setMetadata( fObjects[i], *associationsRead );

		if( MS::kSuccess == fDGModifier.doIt() )
		{
			for( unsigned int c=0; c<associationsRead->channelCount(); ++c )
			{
				adsk::Data::Channel channel = associationsRead->channelAt(c);
				MString cName( channel.name().c_str() );
				for( unsigned int s=0; s<channel.dataStreamCount(); ++s )
				{
					adsk::Data::Stream* cStream = channel.dataStream(s);
					if( cStream )
					{
						MString sName( cStream->name().c_str() );
						MString msg;
						msg.format( resultFmt, node.name(), cName, sName );
						appendToResult( msg );
					}
				}
			}
		}
		else
		{
			MString fmt = MStringResource::getString(kImportMetadataSetMetadataFailed, status);
			MString msg;
			msg.format( fmt, node.name() );
			displayError( msg );
			status = MS::kFailure;
		}
	}

    return status;
}

//======================================================================
//
// Redo the import, restoring the originally imported metadata onto the
// object(s).
//
MStatus importMetadataCmd::redoIt()
{
    return fDGModifier.doIt();
}

//======================================================================
//
// Undo the import, restoring the prior metadata to the object(s).
//
MStatus importMetadataCmd::undoIt()
{
    return fDGModifier.undoIt();
}

//-
// ==================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
// 
// This computer source code  and related  instructions and comments are
// the unpublished confidential and proprietary information of Autodesk,
// Inc. and are  protected  under applicable  copyright and trade secret
// law. They may not  be disclosed to, copied or used by any third party
// without the prior written consent of Autodesk, Inc.
// ==================================================================
//+
