#include <maya/MStatus.h>
#include <maya/MFnPlugin.h>
#include <maya/adskDataStructureSerializer.h>
#include "structureSerializerXML.h"
#include <maya/adskDataAssociationsSerializer.h>
#include "associationsSerializerXML.h"
#include <maya/adskDataChannelSerializer.h>
#include "channelSerializerXML.h"
#include <maya/adskDataStreamSerializer.h>
#include "streamSerializerXML.h"
#include <libxml/globals.h>
#include <libxml/xmlreader.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

//======================================================================
//
// This is what looks like an empty plug-in. The work happens outside
// the normal plug-in mechanism, enabling an XML serializer for metadata
// Structures and Streams. Since they are not part of the M* class
// mechanism they don't get registered like commands or nodes would.
//
// Since the serializers are all used on demand all the plug-in has to do
// is register and deregister it by creating and destroying the
// initializers to handle the serializer lifetime.
//
// In order to build this plug-in you will need libxml2. On Linux and Mac
// this is a standard library. On Windows you will need a local copy.
//
//======================================================================
//
// EXTRA INSTRUCTIONS
// ------------------
// If you do not have libxml2 already you can download and install it
// from the main site:
//
// 		http://xmlsoft.org
//
// For Linux users you should already have the libraries and will only need to
// install the XML development kit to get the headers (probably libxml2-devel)
// and ensure the Makefile path points to the installation directory (by
// default it is /usr/include/libxml2).
//
// Windows users should modify their Visual Studio project to add the
// appropriate installation paths to the settings
// 		'Additional Include Directories'
// 	and
// 		'Additional Library Directories'
// before building.
//
//======================================================================

using namespace adsk::Data;

//----------------------------------------------------------------------
//
//! Initialization objects to register the XML format types.
//! Most code will initialize these as static objects but since we want
//! them to deregister when the plug-in unloads they have to be dynamically
//! allocated.
//
static SerializerInitializer<StructureSerializer>*		_structureXMLInitializer	= NULL;
static SerializerInitializer<AssociationsSerializer>*	_associationsXMLInitializer	= NULL;
static SerializerInitializer<ChannelSerializer>*		_channelXMLInitializer		= NULL;
static SerializerInitializer<StreamSerializer>*			_streamXMLInitializer		= NULL;

MStatus initializePlugin( MObject obj )
{ 
	MFnPlugin plugin ( obj, "Autodesk", "1.0", "Any" );

	// Constructors of the initializers will register the format types
	_structureXMLInitializer = new SerializerInitializer<StructureSerializer>( StructureSerializerXML::theFormat() );
	_associationsXMLInitializer = new SerializerInitializer<AssociationsSerializer>( AssociationsSerializerXML::theFormat() );
	_channelXMLInitializer = new SerializerInitializer<ChannelSerializer>( ChannelSerializerXML::theFormat() );
	_streamXMLInitializer = new SerializerInitializer<StreamSerializer>( StreamSerializerXML::theFormat() );
	return MS::kSuccess;
}

//======================================================================
//
MStatus uninitializePlugin( MObject obj )
{
	// Destructors of the initializers will deregister the format types
	if( _structureXMLInitializer )
	{
		delete _structureXMLInitializer;
		_structureXMLInitializer = NULL;
	}
	if( _associationsXMLInitializer )
	{
		delete _associationsXMLInitializer;
		_associationsXMLInitializer = NULL;
	}
	if( _channelXMLInitializer )
	{
		delete _channelXMLInitializer;
		_channelXMLInitializer = NULL;
	}
	if( _streamXMLInitializer )
	{
		delete _streamXMLInitializer;
		_streamXMLInitializer = NULL;
	}
	return MS::kSuccess;
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
