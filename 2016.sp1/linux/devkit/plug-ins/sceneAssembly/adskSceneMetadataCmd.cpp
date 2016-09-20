#include "sceneAssemblyStrings.h"
#include "adskSceneMetadataCmd.h"

#include <maya/MSyntax.h>
#include <maya/MGlobal.h>
#include <maya/adskDataAssociations.h>
#include <maya/adskDataAccessor.h>
#include <maya/adskDataAccessorMaya.h>
#include <maya/adskDataStream.h>

#include <iostream>
#include <string>

using namespace std;
using namespace adsk::Data;

// Constants
const char gStructureName   [] = "adskSceneMetadataCmdStructure";   
const char gMemberName      [] = "adskSceneMetadataCmdDataString";
const char gStreamName      [] = "adskSceneMetadataCmdStream";

//==============================================================================
// CLASS AdskSceneMetadataCmd
//==============================================================================


//------------------------------------------------------------------------------
//
AdskSceneMetadataCmd::AdskSceneMetadataCmd()
    : fMode(kEdit)
{
}

//------------------------------------------------------------------------------
//
AdskSceneMetadataCmd::~AdskSceneMetadataCmd()
{
}

//------------------------------------------------------------------------------
//
void* AdskSceneMetadataCmd::creator()
{
    return new AdskSceneMetadataCmd();
}

//------------------------------------------------------------------------------
//
MSyntax AdskSceneMetadataCmd::cmdSyntax()
{
    MSyntax syntax;

    syntax.addFlag( "-c", "-channelName",   MSyntax::kString );   
    syntax.addFlag( "-d", "-data",			MSyntax::kString );

    // In query mode, the command needs to know what channel to look for
    syntax.makeFlagQueryWithFullArgs( "-channelName", false );

    syntax.setObjectType( MSyntax::kStringObjects, 1, 1 );
    syntax.enableQuery( true );
    syntax.enableEdit( true );

    return syntax;
}

//------------------------------------------------------------------------------
//
const char* AdskSceneMetadataCmd::name()
{
    return "adskSceneMetadataCmd";
}

//------------------------------------------------------------------------------
//
MStatus AdskSceneMetadataCmd::doIt(const MArgList& args)
{
    MStatus status;
    MArgDatabase argsDb( syntax(), args, &status );
    if ( !status ) 
    {
        return status;
    }

    // Validate the mode
    if (argsDb.isEdit()) 
    {
        if (argsDb.isQuery()) 
        {
            displayError( MStringResource::getString( rEditQueryError, status));
            return MS::kFailure;
        }
        fMode = kEdit;
    }
    else if (argsDb.isQuery()) 
    {
        fMode = kQuery;
    }

    // Parse and validate the flags
    fChannelName.parse	( argsDb, "-channelName" ); 
    fData.parse			( argsDb, "-data" );  

    if ( !fData.isModeValid(fMode) ) 
    {
        displayError( MStringResource::getString( rDataFlagError, status));
        return MS::kFailure;
    }

    // The channel name flag needs to be set in all modes
    if ( !fChannelName.isSet() || !fChannelName.isArgValid() || fChannelName.arg() == "" )
    {
        displayError( MStringResource::getString( rChannelNameFlagError, status) );
        return MS::kFailure;
    }

    // Retrieve the scene path
    MStringArray objs;   
    status = argsDb.getObjects( objs );
    CHECK_MSTATUS_AND_RETURN_IT( status );

    // cmdSyntax above enforces one and only one object
    MString scenePath = objs[0];

    switch ( fMode ) 
    {     
    case kEdit:     status = doEdit( scenePath );  break;
    case kQuery:    status = doQuery( scenePath ); break;
    default: break;
    }

    return status;
}

//------------------------------------------------------------------------------
//
MStatus AdskSceneMetadataCmd::doEdit( MString scenePath )
{
    return setMetadata( scenePath );
}

//------------------------------------------------------------------------------
//
MStatus AdskSceneMetadataCmd::doQuery( MString scenePath )
{    
    return getMetadata( scenePath );
}

MStatus AdskSceneMetadataCmd::getMetadata( MString scenePath )
{
    // This string gets populated with errors by the Metadata library when they occur
    std::string errors;

    // Retrieve the accessor
    std::auto_ptr< Accessor > accessor( getAccessorForScene( scenePath ) );
    if ( NULL == accessor.get() )
    {
        return MS::kFailure;
    }

    // Retrieve the scene associations
    Associations associations;
    if ( MS::kSuccess != getSceneAssociations(*(accessor.get()), associations) )
    {
        return MS::kFailure;
    }

    // Look for the specified channel
    std::string channelName( fChannelName.arg().asChar() );
    Channel *channel = associations.findChannel( channelName );
    if ( NULL == channel )
    {
        // The specified channel was not found in the metadata. 
        // There is simply no metadata of interest in that scene file
        setResult( "" );
        return MS::kSuccess;
    }

    // There should only be one stream in our metadata. Make sure there is at least one
    if ( 0 == channel->dataStreamCount() )
    {
        MStatus resStatus;
        MString errorString = MStringResource::getString( rMissingStreamInChannelError, resStatus );
        errorString.format( errorString, fChannelName.arg() );
        displayError( errorString );
        return MS::kFailure;
    }

    Stream *stream = channel->dataStream( 0 );

    // There should only be one element in the stream. Make sure there is at least one
    if ( 0 == stream->elementCount() )
    {
        MStatus resStatus;
        MString errorString = MStringResource::getString( rMissingElementInStreamError, resStatus );
        errorString.format( errorString, fChannelName.arg() );
        displayError( errorString );
        return MS::kFailure;
    }

    Handle handle = stream->element( 0 );

    // Position the handle on our data of interest
    if ( false == handle.setPositionByMemberName( gMemberName ) )
    {
        MStatus resStatus;
        MString errorString = MStringResource::getString( rMissingMemberInElementError, resStatus );
        errorString.format( errorString, gMemberName, fChannelName.arg() );
        displayError( errorString );
        return MS::kFailure;
    }

    if ( Member::kString != handle.dataType() )
    {
        MStatus resStatus;
        MString errorString = MStringResource::getString( rInvalidMemberDataTypeError, resStatus );
        errorString.format( errorString, fChannelName.arg() );
        displayError( errorString );
        return MS::kFailure;
    }

    char **strings = handle.asString();

    setResult( strings[0] );
    return MS::kSuccess;
}

MStatus AdskSceneMetadataCmd::setMetadata( MString scenePath )
{
    // This string gets populated with errors by the Metadata library when they occur
    std::string errors;

    // Retrieve the accessor
    std::auto_ptr< Accessor > accessor( getAccessorForScene( scenePath ) );
    if ( NULL == accessor.get() )
    {
        setResult( false );
        return MS::kFailure;
    }

    // Retrieve the accessor
    Associations associations;
    if ( MS::kSuccess != getSceneAssociations(*(accessor.get()), associations) )
    {
        setResult( false );
        return MS::kFailure;
    }

    // Is our structure registered yet?
    Structure *structure = NULL;
    for ( Structure::ListIterator itrStructList = Structure::allStructures().begin(); itrStructList != Structure::allStructures().end(); ++itrStructList )
    {
        if ( std::string((*itrStructList)->name()) == std::string(gStructureName) )
        {
            structure = *itrStructList;
            break;
        }
    }

    if ( NULL == structure )
    {
        // Register our structure since it is not registered yet.
        structure = Structure::create();
        structure->setName( gStructureName );
        structure->addMember( Member::kString, 1, gMemberName );

        Structure::registerStructure( *structure );
    }

    // Make sure our structure is known by the accessor
    const Accessor::StructureSet &accessorStructures = accessor->structures();
    if ( accessorStructures.find( structure ) == accessorStructures.end() )
    {
        // Build a new structure set from the existing one
        Accessor::StructureSet updatedStructures = accessorStructures;

        // Add our structure to it
        updatedStructures.insert( structure );

        // Assign the new structure set
        accessor->setStructures( updatedStructures );
    }

    // Retrieve or create the specified channel
    std::string channelName( fChannelName.arg().asChar() );
    Channel channel = associations.channel( channelName );

    // Create the stream
    Stream stream( *structure, std::string( gStreamName ) );

    // Set the stream in the channel
    channel.setDataStream( stream );

    // Create an handle to the data itself
    Handle handle( *structure );

    // Set our string
    std::string dataString( fData.arg().asChar() );
    if ( 0 != handle.fromStr( dataString, 0, errors ) )
    {
        MStatus resStatus;
        MString errorString = MStringResource::getString( rSetDataOnChannelError, resStatus );
        errorString.format( errorString, fChannelName.arg(), errors.c_str() );
        displayError( errorString );

        setResult( false );
        return MS::kFailure;
    }

    // Set the handle in the stream
    stream.setElement( 0, handle );

    // Write the new scene file metadata
    if ( !accessor->write( errors ) )
    {
        MStatus resStatus;
        MString errorString = MStringResource::getString( rWriteMetadataError, resStatus );
        displayError( errorString );

        setResult( false );
        return MS::kFailure;
    }

    // true is success;
    setResult( true );
    return MS::kSuccess;
}

std::auto_ptr< Accessor > AdskSceneMetadataCmd::getAccessorForScene( MString scenePath )
{
    // This string gets populated with errors by the Metadata library when they occur
    std::string errors;
    std::string stdScenePath( scenePath.asChar() );
    std::auto_ptr< Accessor > accessor( Accessor::accessorByExtension( stdScenePath ) );
    if ( NULL == accessor.get() ) 
    {
        MStatus resStatus;
        MString errorString = MStringResource::getString( rAccessorNotFoundError, resStatus );
        errorString.format( errorString, scenePath );
        displayError( errorString );
        
        return std::auto_ptr< Accessor >();
    }

    // Optimization: only read the scene associations.
    std::set< std::string > wantedAssociations;
    wantedAssociations.insert( AccessorMaya::getSceneAssociationsName() );

    if ( !accessor->read(   stdScenePath, 
                            NULL,                   // read all the structures
                            &wantedAssociations,    
                            errors ) ) 
    {
        MStatus resStatus;
        MString errorString = MStringResource::getString( rCannotReadFileError, resStatus );
        errorString.format( errorString, scenePath, errors.c_str() );
        displayError( errorString );

        return std::auto_ptr< Accessor >();
    }

    return accessor;
}

MStatus AdskSceneMetadataCmd::getSceneAssociations( Accessor &accessor, Associations &out_associations )
{
    // Retrieve the scene-level association
    const Accessor::AssociationsMap &associationsMap = accessor.associations();
    Accessor::AssociationsMap::const_iterator iterAssociations = associationsMap.find( AccessorMaya::getSceneAssociationsName() );
    if ( iterAssociations == associationsMap.end() )
    {
        // We could not find the scene associations in the given file. Create them
        out_associations = Associations::create();
        accessor.associations()[AccessorMaya::getSceneAssociationsName()] = out_associations;
    }
    else
    {
        out_associations = (*iterAssociations).second;
    }

    return MS::kSuccess;
}

//-
//*****************************************************************************
// Copyright 2015 Autodesk, Inc.  All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy
// form.
//*****************************************************************************
//+
