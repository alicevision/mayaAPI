#include "exportMetadataCmd.h"
#include "metadataPluginStrings.h"
#include <maya/MObject.h>
#include <maya/MSyntax.h>
#include <maya/MPxCommand.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MStringResource.h>
#include <maya/MArgDatabase.h>
#include <maya/MFileObject.h>
#include <maya/MFnDependencyNode.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <maya/adskDataChannel.h>
#include <maya/adskDataAssociations.h>
#include <maya/adskDataAssociationsSerializer.h>
#include <maya/adskDataChannelSerializer.h>
#include <maya/adskDataStreamSerializer.h>
#include <maya/adskDataStructureSerializer.h>

//======================================================================
//
// Creator function: returns a new command object
//
void* exportMetadataCmd::creator()
{
	return new exportMetadataCmd ;
}

//======================================================================
//
// Returns the name of this command
//
const char* exportMetadataCmd::name()
{
	return "exportMetadata";
}

//======================================================================
//
// Get the base syntax and allow query mode
//
MSyntax exportMetadataCmd::cmdSyntax()
{
	MSyntax mySyntax = metadataBase::cmdSyntax();
	mySyntax.enableQuery( true );
	return mySyntax;
}

//======================================================================
//
// Default command constructor
//
exportMetadataCmd::exportMetadataCmd()
{
}

//======================================================================
//
// Destructor, does nothing
//
exportMetadataCmd::~exportMetadataCmd()
{
}

//======================================================================
//
// Check the parsed arguments
//
MStatus exportMetadataCmd::checkArgs(MArgDatabase& argsDb)
{
	return metadataBase::checkArgs( argsDb );
}

//======================================================================
//
// Do the command in create mode. Run the export, which sends all of the
// specified metadata out to a file (if the -file flag was specified) or
// as a returned string (if the -file flag was not specified)
//
MStatus exportMetadataCmd::doCreate()
{
	assert( fSerializer );
	MStatus status;

	assert( fObjects.length() == 1 );
	MFnDependencyNode node( fObjects[0], &status );

	// Should have filtered out non-objects already
    assert( status );
	if( ! status ) return status;

	displayInfo( node.name(&status) );

	const adsk::Data::Associations* associationsToWrite = node.metadata();
	if( ! associationsToWrite ) return MS::kFailure;
	std::string	errors;

	// Dump either to a file or the return string, depending on which was
	// requested.
	//
	status = MS::kSuccess;
	if( fFile )
	{
		MString path( fFile->resolvedFullName() );
		std::ofstream destination( fFile->resolvedFullName().asChar() );
		if( fSerializer->write( *associationsToWrite, destination, errors ) == 0 )
		{
			setResult( path );
		}
		else
		{
			MString errMsg( MStringResource::getString(kExportMetadataFailedFileWrite, status) );
			displayError( errMsg );
			status = MS::kFailure;
		}
		destination.close();
	}
	else
	{
		std::stringstream	writtenData;
		if( fSerializer->write( *associationsToWrite, writtenData, errors ) == 0 )
		{
			setResult( writtenData.str().c_str() );
		}
		else
		{
			MString errMsg( MStringResource::getString(kExportMetadataFailedStringWrite, status) );
			displayError( errMsg );
			status = MS::kFailure;
		}
	}

	if( errors.length() > 0 )
	{
		displayError( errors.c_str() );
		return MS::kFailure;
	}

    return MS::kSuccess;
}

//======================================================================
//
// Do the command in query mode. It only does one thing, print the Stream,
// Channel, Associations, and Structure formats available.
//
MStatus exportMetadataCmd::doQuery()
{
	assert( fSerializer );
	MStatus status = MS::kSuccess;

	std::set<const adsk::Data::StreamSerializer*>::iterator sFmtIt;
	for( sFmtIt = adsk::Data::StreamSerializer::allFormats().begin();
		 sFmtIt != adsk::Data::StreamSerializer::allFormats().end(); sFmtIt++ )
	{
		const adsk::Data::StreamSerializer* fmt = *sFmtIt;
		MString fmtMsg( MStringResource::getString(kExportMetadataFormatType, status) );
		MString fmtType( "Stream" );
		MString fmtName( fmt->formatType() );
		MString msg;
		msg.format( fmtMsg, fmtType, fmtName );
		appendToResult( msg );
	}

	std::set<const adsk::Data::ChannelSerializer*>::iterator cFmtIt;
	for( cFmtIt = adsk::Data::ChannelSerializer::allFormats().begin();
		 cFmtIt != adsk::Data::ChannelSerializer::allFormats().end(); cFmtIt++ )
	{
		const adsk::Data::ChannelSerializer* fmt = *cFmtIt;
		MString fmtMsg( MStringResource::getString(kExportMetadataFormatType, status) );
		MString fmtType( "Channel" );
		MString fmtName( fmt->formatType() );
		MString msg;
		msg.format( fmtMsg, fmtType, fmtName );
		appendToResult( msg );
	}

	std::set<const adsk::Data::AssociationsSerializer*>::iterator aFmtIt;
	for( aFmtIt = adsk::Data::AssociationsSerializer::allFormats().begin();
		 aFmtIt != adsk::Data::AssociationsSerializer::allFormats().end(); aFmtIt++ )
	{
		const adsk::Data::AssociationsSerializer* fmt = *aFmtIt;
		MString fmtMsg( MStringResource::getString(kExportMetadataFormatType, status) );
		MString fmtType( "Associations" );
		MString fmtName( fmt->formatType() );
		MString msg;
		msg.format( fmtMsg, fmtType, fmtName );
		appendToResult( msg );
	}

	std::set<const adsk::Data::StructureSerializer*>::iterator fmtIt;
	for( fmtIt = adsk::Data::StructureSerializer::allFormats().begin();
		 fmtIt != adsk::Data::StructureSerializer::allFormats().end(); fmtIt++ )
	{
		const adsk::Data::StructureSerializer* fmt = *fmtIt;
		MString fmtMsg( MStringResource::getString(kExportMetadataFormatType, status) );
		MString fmtType( "Structure" );
		MString fmtName( fmt->formatType() );
		MString msg;
		msg.format( fmtMsg, fmtType, fmtName );
		appendToResult( msg );
	}

	return status;
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
