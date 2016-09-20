#include "tweakMetadataNode.h"
#include <string.h>
#include <sstream>
#include <maya/MIOStream.h>
#include <math.h>

#include <maya/MPxNode.h>
#include <maya/MFnMesh.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MArrayDataHandle.h>
#include <maya/adskDataAssociations.h>
#include <maya/adskDataChannel.h>
#include <maya/adskDataStream.h>
#include <maya/adskDataHandle.h>
#include <maya/adskDataMember.h>

#define MCheckErr(stat,msg)		\
	if ( MS::kSuccess != stat ) {	\
		cerr << msg;		\
		return MS::kFailure;	\
	}

MTypeId tweakMetadataNode::id( 0x8104F );

// local attributes
//
MObject	tweakMetadataNode::aInMesh;
MObject tweakMetadataNode::aOutMesh;
MObject	tweakMetadataNode::aOperation;

tweakMetadataNode::tweakMetadataNode	() {}
tweakMetadataNode::~tweakMetadataNode	() {}

void* tweakMetadataNode::creator()
{
	return new tweakMetadataNode();
}

const char* tweakMetadataNode::nodeName()
{
	return "tweakMetadata";
}

MStatus tweakMetadataNode::initialize()
{
	MStatus status;

	//----------------------------------------
	MFnEnumAttribute eAttr;
	aOperation = eAttr.create( "operation", "op", kOpNone );
		eAttr.addField( "none",   kOpNone );
		eAttr.addField( "random", kOpRandomize );
		eAttr.addField( "fill",   kOpFill );
		eAttr.addField( "double", kOpDouble );
	MCheckErr( status, "failed to create operation attribute" );
	addAttribute( aOperation );

	//----------------------------------------
	MFnTypedAttribute tAttr;
	aInMesh = tAttr.create( "inMesh", "im", MFnData::kMesh, MObject::kNullObj, &status );
	MCheckErr( status, "failed to create inMesh attribute" );
	addAttribute( aInMesh );

	//----------------------------------------
	aOutMesh = tAttr.create( "outMesh", "om", MFnData::kMesh, MObject::kNullObj, &status );
	MCheckErr( status, "failed to create outMesh attribute" );
	tAttr.setWritable( false );
	tAttr.setStorable( false );
	addAttribute( aOutMesh );

	//----------------------------------------
	attributeAffects(aInMesh, aOutMesh);
	attributeAffects(aOperation, aOutMesh);

	return MStatus::kSuccess;
}

MStatus tweakMetadataNode::compute( const MPlug& plug, MDataBlock& block)
{
	MStatus status = MS::kSuccess;

	MObject thisNode = thisMObject();
	MPlug opPlug(thisNode, aOperation);

	short opType = block.inputValue( opPlug, & status ).asShort();
	if( status != MS::kSuccess )
	{
		return status;
	}

	MPlug inMeshPlug(thisNode, aInMesh);
	MDataHandle inMeshHandle = block.inputValue(inMeshPlug, &status);
	MObject inMeshObj = inMeshHandle.asMesh();
	MFnMesh inputMesh( inMeshObj );
	MCheckErr(status,"ERROR getting inMesh"); 

	// Create a copy of the mesh object. Rely on the underlying geometry
	// object to minimize the amount of duplication that will happen.
	MPlug outMeshPlug(thisNode, aOutMesh);
	MDataHandle outMeshHandle = block.outputValue(outMeshPlug, &status);
	MCheckErr(status,"ERROR getting outMesh"); 
	outMeshHandle.set(inMeshObj);
	MObject outMeshObj = outMeshHandle.asMesh();
	MFnMesh outputMesh( outMeshObj );

	const adsk::Data::Associations* oldAssociations = inputMesh.metadata();

	// Search through all streams in all channels for matching data types.
	if( oldAssociations )
	{
		adsk::Data::Associations associations( *oldAssociations );
		// Everything could be touched so make a unique copy of everything
		associations.makeUnique();
		for( unsigned int c=0; c<associations.channelCount(); ++c )
		{
			adsk::Data::Channel channel( associations.channelAt( c ) );
			//channel.makeUnique();
			for( unsigned int s=0; s<channel.dataStreamCount(); ++s )
			{
				if( ! channel.dataStream( s ) ) continue; // Should never happen
				adsk::Data::Stream* chStream = channel.dataStream( s );

				for( unsigned int el=0; el<chStream->elementCount(); ++el )
				{
					adsk::Data::Handle sHandle = chStream->element( el );
					if( ! sHandle.hasData() ) continue;
					if( sHandle.dataType() != adsk::Data::Member::kInt32 ) continue;
	
					int* hValue = sHandle.asInt32();
					if( ! hValue ) continue;
	
					for( unsigned int l=0; l<sHandle.dataLength(); ++l )
					{
						switch( opType )
						{
							case kOpRandomize:
								// Fill all of the int32 metadata types with a random number
								// between -1000 and +1000
								hValue[l] = int(rand() % 2001) - 1000;
								break;
	
							//----------------------------------------
							case kOpFill:
								// Fill all of the int32 metadata types with a constant
								// Since it doesn't really matter what the constant is pick the
								// last eight bits of the pointer to the stream so that each
								// stream will potentially get a different value.
								hValue[l] = (size_t)(chStream) % 0xff;
								break;
	
							//----------------------------------------
							case kOpDouble:
								// Find all int32 metadata types and double their values
								hValue[l] *= 2;
								break;
	
							//----------------------------------------
							case kOpNone:
							default:
								// No-op moves the mesh through unchanged
								break;
						}
					}
				}
				channel.setDataStream( *chStream );
			}
			associations.setChannel( channel );
		}
		// Put the modified metadata onto the output mesh, leaving the
		// original on the input mesh.
		outputMesh.setMetadata( associations );
		outMeshHandle.setClean();
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
