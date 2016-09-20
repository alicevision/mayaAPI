#include "createMetadataCmd.h"
#include "metadataPluginStrings.h"
#include <maya/MObject.h>
#include <maya/MSyntax.h>
#include <maya/MString.h>
#include <maya/MStringResource.h>
#include <maya/MFnMesh.h>
#include <maya/MFileObject.h>
#include <maya/MSelectionList.h>
#include <maya/adskDataStream.h>
#include <maya/adskDataChannel.h>
#include <maya/adskDataAssociations.h>
#include <float.h>
#include <string>

using namespace adsk::Data;

#define F_RAND float((4000000.0 * ((float)rand()/(float)RAND_MAX)) - 2000000.0)
#define D_RAND ((4000000000.0 * ((double)rand()/(double)RAND_MAX)) - 2000000000.0)

// Command flag names
static const char* flagChannelName		( "-cn" );
static const char* flagChannelNameLong	( "-channelName" );
static const char* flagStreamName		( "-sn" );
static const char* flagStreamNameLong	( "-streamName" );
static const char* flagStructure		( "-s" );
static const char* flagStructureLong	( "-structure" );

//============================================================================
//
// Get the syntax information. Initializes the shared flags. Derived commands
// can add on their own flags after calling this routine.
//
MSyntax createMetadataCmd::cmdSyntax()
{
	MSyntax syntax;
    syntax.addFlag(flagChannelName,	flagChannelNameLong, MSyntax::kString);
    syntax.addFlag(flagStreamName,	flagStreamNameLong,	 MSyntax::kString);
    syntax.addFlag(flagStructure,	flagStructureLong,	 MSyntax::kString);
	
	syntax.useSelectionAsDefault(true);
    syntax.setObjectType(MSyntax::kSelectionList, 1);

	// No query or edit.
    syntax.enableQuery( false );
    syntax.enableEdit( false );

	// Seed the random numbers for later
	srand( 123 );

    return syntax;
}

//======================================================================
//
// Creator function: returns a new command object
//
void* createMetadataCmd::creator()
{
	return new createMetadataCmd;
}

//======================================================================
//
// Returns the name of this command
//
const char* createMetadataCmd::name()
{
	return "createMetadata";
}

//======================================================================
//
// Default command constructor
//
createMetadataCmd::createMetadataCmd()
: fNodes()
, fDGModifier()
, fIndexList()
, fChannelName()
, fStreamName()
, fStructure( (adsk::Data::Structure*) 0 )
{
}

//======================================================================
//
// Destructor, does nothing
//
createMetadataCmd::~createMetadataCmd()
{
}

//======================================================================
//
// This command is undoable
//
bool
createMetadataCmd::isUndoable() const
{
	return true;
}

//======================================================================
//
// Check the parsed arguments and do/undo/redo the command as appropriate
//
MStatus createMetadataCmd::checkArgs(MArgDatabase& argsDb)
{
	MStatus status = MS::kSuccess;

	//----------------------------------------
	// -structure flag
	//
	fStructureFlag.parse(argsDb, flagStructure);
	if( fStructureFlag.isSet() )
	{
		if( ! fStructureFlag.isArgValid() )
		{
			MString errMsg( MStringResource::getString(kInvalidString, status) );
			displayError( errMsg );
			return MS::kFailure;
		}
		MString structureName( fStructureFlag.arg() );
		fStructure = adsk::Data::Structure::structureByName( structureName.asChar() );
		if( ! fStructure )
		{
			MString fmt( MStringResource::getString(kCreateMetadataStructureNotFound, status) );
			MString msg;
			msg.format( fmt, structureName );
			displayError(msg);
			return MS::kFailure;
		}
	}
	else
	{
		MString err( MStringResource::getString(kCreateMetadataNoStructureName,status) );
		displayError( err );
		return MS::kFailure;
	}

	//----------------------------------------
	// -streamName flag
	//
	fStreamNameFlag.parse(argsDb, flagStreamName);
	if( fStreamNameFlag.isSet() )
	{
		if( ! fStreamNameFlag.isArgValid() )
		{
			MString errMsg( MStringResource::getString(kInvalidString, status) );
			displayError( errMsg );
			return MS::kFailure;
		}
		fStreamName = fStreamNameFlag.arg();
	}
	else
	{
		MString err( MStringResource::getString(kCreateMetadataNoStreamName,status) );
		displayError( err );
		return MS::kFailure;
	}

	//----------------------------------------
	// -channelName flag
	//
	fChannelNameFlag.parse(argsDb, flagChannelName);
	if( fChannelNameFlag.isSet() )
	{
		if( ! fChannelNameFlag.isArgValid() )
		{
			MString errMsg( MStringResource::getString(kInvalidString, status) );
			displayError( errMsg );
			return MS::kFailure;
		}
		fChannelName = fChannelNameFlag.arg().asChar();
	}
	else
	{
		MString err( MStringResource::getString(kCreateMetadataNoChannelName,status) );
		displayError( err );
		return MS::kFailure;
	}

	//----------------------------------------
	// (selection list)
	//
	// Commands need at least one node on which to operate so gather up
	// the list of nodes specified and/or selected.
	//

	// Empty out the list of nodes on which to operate so that it can be
	// populated from the selection or specified lists.
	fNodes.clear();

	MSelectionList objects;
	status = argsDb.getObjects(objects);
	MStatError(status, "argsDb.getObjects()");
	for (unsigned int i = 0; i<objects.length(); ++i)
	{
		MObject	node;
		status = objects.getDependNode(i, node);
		MStatError(status, "objects.getDependNode()");	
		fNodes.append( node );
	}

	if( fNodes.length() == 0 )
	{
		MString msg = MStringResource::getString(kObjectNotFoundError, status);
		displayError(msg);
		return MStatus::kFailure;
	}

	return status;
}

//======================================================================
//
// Do the metadata creation. The metadata will be randomly initialized
// based on the channel type and the structure specified. For recognized
// components the number of metadata elements will correspond to the count
// of components in the selected mesh, otherwise a random number of metadata
// elements between 1 and 100 will be created (at consecutive indices).
//
// The previously existing metadata is preserved for later undo.
//
MStatus createMetadataCmd::doIt(const MArgList& args)
{
	MStatus status;

	MArgDatabase argsDb(syntax(), args, &status);
	if (MS::kSuccess != status) return status;

    status = checkArgs(argsDb);
	if( MS::kSuccess != status )
	{
		return status;
	}

	clearResult();

	for( unsigned int i=0; i<fNodes.length(); ++i )
	{
		MFnDependencyNode node( fNodes[i] );
		// Get the current metadata (empty if none yet)
		adsk::Data::Associations newMetadata( node.metadata() );
		adsk::Data::Channel newChannel = newMetadata.channel( fChannelName );

		// Check to see if the requested stream name already exists
		adsk::Data::Stream* oldStream = newChannel.dataStream( fStreamName.asChar() );
		if( oldStream )
		{
			MString fmt = MStringResource::getString(kCreateMetadataHasStream, status);
			MString msg;
			msg.format( fmt, fStreamName );
			displayError( msg );
			status = MS::kFailure;
			continue;
		}

		adsk::Data::Stream newStream( *fStructure, fStreamName.asChar() );

		unsigned int indexCount = 0;

		// Treat the channel type initializations different for meshes
		if( fNodes[i].hasFn(MFn::kMesh) )
		{
			MFnMesh mesh( fNodes[i], &status );
			// Get mesh-specific channel type parameters
			if( fChannelName == "face" )
			{
				indexCount = mesh.numPolygons();
			}
			else if( fChannelName == "edge" )
			{
				indexCount = mesh.numEdges();
			}
			else if( fChannelName == "vertex" )
			{
				indexCount = mesh.numVertices();
			}
			else if( fChannelName == "vertexFace" )
			{
				indexCount = mesh.numFaceVertices();
			}
			else
			{
				indexCount = rand() % 100 + 1;
			}
		}
		else
		{
			// Create generic channel type information
			indexCount = rand() % 100 + 1;
		}

		// Fill specified stream ranges with random data
		unsigned int structureMemberCount = fStructure->memberCount();
		for( unsigned int m=0; m<indexCount; ++m )
		{
			// Walk each structure member and fill with random data
			// tailored to the member data type.
			adsk::Data::Handle handle( *fStructure );
			for( unsigned int i=0; i<structureMemberCount; ++i )
			{
				handle.setPositionByMemberIndex( i );
				for( unsigned int d=0; d<handle.dataLength(); ++d )
				{
					switch( handle.dataType() )
					{
						case adsk::Data::Member::kBoolean:
						{
							bool* data = handle.asBoolean();
							data[d] = (rand() % 2 == 1);
							break;
						}
						case adsk::Data::Member::kDouble:
						{
							double* data = handle.asDouble();
							data[d] = D_RAND;
							break;
						}
						case adsk::Data::Member::kDoubleMatrix4x4:
						{
							double* data = handle.asDoubleMatrix4x4();
							data[d*16+0] = D_RAND;
							data[d*16+1] = D_RAND;
							data[d*16+2] = D_RAND;
							data[d*16+3] = D_RAND;
							data[d*16+4] = D_RAND;
							data[d*16+5] = D_RAND;
							data[d*16+6] = D_RAND;
							data[d*16+7] = D_RAND;
							data[d*16+8] = D_RAND;
							data[d*16+9] = D_RAND;
							data[d*16+10] = D_RAND;
							data[d*16+11] = D_RAND;
							data[d*16+12] = D_RAND;
							data[d*16+13] = D_RAND;
							data[d*16+14] = D_RAND;
							data[d*16+15] = D_RAND;
							break;
						}
						case adsk::Data::Member::kFloat:
						{
							float* data = handle.asFloat();
							data[d] = F_RAND;
							break;
						}
						case adsk::Data::Member::kFloatMatrix4x4:
						{
							float* data = handle.asFloatMatrix4x4();
							data[d*16+0] = F_RAND;
							data[d*16+1] = F_RAND;
							data[d*16+2] = F_RAND;
							data[d*16+3] = F_RAND;
							data[d*16+4] = F_RAND;
							data[d*16+5] = F_RAND;
							data[d*16+6] = F_RAND;
							data[d*16+7] = F_RAND;
							data[d*16+8] = F_RAND;
							data[d*16+9] = F_RAND;
							data[d*16+10] = F_RAND;
							data[d*16+11] = F_RAND;
							data[d*16+12] = F_RAND;
							data[d*16+13] = F_RAND;
							data[d*16+14] = F_RAND;
							data[d*16+15] = F_RAND;
							break;
						}
						case adsk::Data::Member::kInt8:
						{
							signed char* data = handle.asInt8();
							data[d] = rand() % 255 - 127;
							break;
						}
						case adsk::Data::Member::kInt16:
						{
							short* data = handle.asInt16();
							data[d] = rand() % 65535 - 32767;
							break;
						}
						case adsk::Data::Member::kInt32:
						{
							int* data = handle.asInt32();
							data[d] = (rand()/2) * (rand() % 2 == 1 ? 1 : -1);
							break;
						}
						case adsk::Data::Member::kInt64:
						{
							int64_t* data = handle.asInt64();
							data[d] = rand() * 10 * (rand() % 2 == 1 ? 1 : -1);
							break;
						}
						case adsk::Data::Member::kUInt8:
						{
							unsigned char* data = handle.asUInt8();
							data[d] = rand() % 255;
							break;
						}
						case adsk::Data::Member::kUInt16:
						{
							unsigned short* data = handle.asUInt16();
							data[d] = rand() % 65535;
							break;
						}
						case adsk::Data::Member::kUInt32:
						{
							unsigned int* data = handle.asUInt32();
							data[d] = rand();
							break;
						}
						case adsk::Data::Member::kUInt64:
						{
							uint64_t* data = handle.asUInt64();
							data[d] = rand() * 10;
							break;
						}
						case adsk::Data::Member::kString:
						{
							char** data = handle.asString();
							data[d] = (char*) malloc( sizeof(char) * 9);
							for( unsigned int s=0; s<8; ++s )
							{
								data[d][s] = rand() % 26 + 'a';
							}
							data[d][8] = '\0';
							break;
						}

						default:
						{
							// Shouldn't ever happen
							assert( false );
							break;
						}
					}
				}
			}
			newStream.setElement( m, handle );
		}
		newChannel.setDataStream( newStream );
		newMetadata.setChannel( newChannel );

		fDGModifier.setMetadata( fNodes[i], newMetadata );
		status = fDGModifier.doIt();
		if( MS::kSuccess == status )
		{
			// Set the result to the number of actual metadata values set as a
			// triple value:
			//	 	(# nodes, # metadata elements, # members per element)
			//
			MIntArray theResult;
			theResult.append( (int) fNodes.length() );
			theResult.append( (int) indexCount );
			theResult.append( (int) structureMemberCount );
			setResult( theResult );
		}
		else
		{
			MString fmt = MStringResource::getString(kCreateMetadataCreateFailed, status);
			MString msg;
			msg.format( fmt, node.name() );
			displayError( msg );
		}
	}
    return status;
}

//======================================================================
//
// Redo the import, restoring the originally imported metadata onto the
// mesh(es).
//
MStatus createMetadataCmd::redoIt()
{
    return fDGModifier.doIt();
}

//======================================================================
//
// Undo the import, restoring the prior metadata to the mesh(es).
//
MStatus createMetadataCmd::undoIt()
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
