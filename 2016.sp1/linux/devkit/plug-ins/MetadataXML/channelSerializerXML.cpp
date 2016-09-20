#include "channelSerializerXML.h"
#include "streamSerializerXML.h"
#include "metadataXML.h"
#include "metadataXMLPluginStrings.h"
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <maya/MString.h>
#include <maya/MStringResource.h>
#include <maya/adskDataChannel.h>
#include <maya/adskDataChannelSerializer.h>
#include <maya/adskDataStream.h>
#include <maya/adskDataStreamSerializer.h>
#include <assert.h>
#include <libxml/globals.h>
#include <libxml/xmlreader.h>
#include <libxml/parser.h>

using namespace adsk;
using namespace adsk::Data;
using namespace adsk::Data::XML;

ImplementSerializerFormat(ChannelSerializerXML,ChannelSerializer,xmlFormatType);

//----------------------------------------------------------------------
//
//! \brief Default constructor, does nothing
//
ChannelSerializerXML::ChannelSerializerXML()
{
}

//----------------------------------------------------------------------
//
//! \brief Default destructor, does nothing
//
ChannelSerializerXML::~ChannelSerializerXML	()
{
}

//----------------------------------------------------------------------
//
//! \brief Create a Channel based on the XML-formatted data in the input stream.
//
//	This is not normally called directly as a Channel cannot float freely
//	without an Associations parent defining how it is attached to an object.
//	The Associations parser will call the parseDOM() method below to parse
//	a partial tree.
//
//! \param[in] cSrc Channel containing the XML format data to be parsed
//! \param[out] errors Description of problems found when parsing the string
//
//! \return The created Channel, NULL if there was an error creating it
//
adsk::Data::Channel*
ChannelSerializerXML::read(
	std::istream&	cSrc,
	std::string&	errors )	const
{
	unsigned int errorCount = 0;
	adsk::Data::Channel* newChannel = NULL;
	errors = "";

	// The boilerplate part of this code was taken from the libxml website
	// examples. (http://www.xmlsoft.org/examples)
	xmlDocPtr doc = NULL;

	// this initialize the library and check potential ABI mismatches
	// between the version it was compiled for and the actual shared
	// library used.
	LIBXML_TEST_VERSION

	// Since these files can never be too big it's okay to slurp the entire
	// thing into memory and process it as a string.
	cSrc.seekg (0, std::ios::end);
	int size = (int) cSrc.tellg();
	char* memblock = new char [size];
	cSrc.seekg (0, std::ios::beg);
	cSrc.read (memblock, size);

	doc = xmlReadMemory(memblock, size, NULL, NULL, 0);
	xmlNode* rootEl = xmlDocGetRootElement(doc);

	// Walk the DOM and create the Channel from it
	for (xmlNode* curNode = rootEl; curNode; curNode = curNode->next)
	{
		if( ! curNode ) continue;

		// Skip anything unrecognized, for maximum flexibility
		if (curNode->type != XML_ELEMENT_NODE)
		{
			continue;
		}
		if( newChannel )
		{
			// Error to have more than one channel
			REPORT_ERROR_AT_LINE(kChannelXMLTooManyChannels, curNode->line);
			continue;
		}

		newChannel = parseDOM( doc, *curNode, errorCount, errors );
	}

	delete[] memblock;

	// Free the document
	xmlFreeDoc(doc);

	// If there were errors any Stream created will be incorrect so pass
	// back nothing rather than bad data.
	if( errorCount > 0 )
	{
		delete newChannel;
		newChannel = NULL;
	}
	return newChannel;
}

//----------------------------------------------------------------------
//
//! \brief Create a Channel based on a partial XML DOM tree
//
//! \param[in] doc Pointer to the XML DOM being parsed
//! \param[in] channelNode Root of the DOM containing the Channel data
//! \param[out] errorCount Number of errors found in parsing
//! \param[out] errors Description of problems found when parsing the string
//
//! \return The created Channel, even if partially complete
//
adsk::Data::Channel*
ChannelSerializerXML::parseDOM(
	xmlDocPtr		doc,
	xmlNode&		channelNode,
	unsigned int&	errorCount,
	std::string&	errors )	const
{
	// Get the Stream serializer to handle the sub-section of the DOM.
	// If it can't be found then no data can be created.
	const StreamSerializerXML* xmlStreamSerializer = dynamic_cast<const StreamSerializerXML*>( adsk::Data::StreamSerializer::formatByName( xmlFormatType ) );
	if( ! xmlStreamSerializer )
	{
		REPORT_ERROR_AT_LINE(kChannelXMLStreamSerializerMissing, channelNode.line);
		return NULL;
	}

	// Find the Channel name tag
	xmlNode* nameNode = Util::findNamedNode( channelNode.children, xmlTagChannelName );
	const char* channelName = nameNode ? (const char*) Util::findText( doc, nameNode ) : NULL;
	if( ! channelName )
	{
		// Error to have no channel name
		REPORT_ERROR_AT_LINE(kChannelXMLNameMissing, channelNode.line);
		return NULL;
	}

	adsk::Data::Channel newChannel( channelName );

	// Pass on parsing of all of its <stream> children to the Stream parser.
	if( 0 != strcmp( (const char*)channelNode.name, xmlTagChannel ) )
	{
		return NULL;
	}

	// The <stream> tags are the children of the <channel> tag
	for( xmlNode* streamNode=channelNode.children; streamNode; streamNode=streamNode->next )
	{
		// Ignore anything that's not a <stream> tag since it's the only
		// relevant one.
		if( (streamNode->type != XML_ELEMENT_NODE)
		||  (0 != strcmp((const char*)streamNode->name, xmlTagStream) ) )
		{
			continue;
		}

		adsk::Data::Stream* newStream =
			xmlStreamSerializer->parseDOM( doc, *streamNode, errorCount, errors );

		if( errorCount > 0 )
		{
			delete newStream;
		}
		else if( newStream )
		{
			newChannel.setDataStream( *newStream );
		}
	}

	// If there were errors any Stream created will be incorrect so pass
	// back nothing rather than bad data.
	if( errorCount > 0 )
	{
		return NULL;
	}
	return new adsk::Data::Channel( newChannel );
}

//----------------------------------------------------------------------
//
//! \brief Output the Channel object in XML format into the stream
//
//! \param[in] dataToWrite Channel to be formatted
//! \param[out] cDst stream to which the XML format of the Channel is written
//! \param[out] errors Description of problems found when writing the Channel
//
//! \return number of errors found during write, 0 means success
//
int
ChannelSerializerXML::write(
	const adsk::Data::Channel&	dataToWrite,
	std::ostream&				cDst,
	std::string&				errors )	const
{
	unsigned int errorCount = 0;
	// Get the Stream serializer to handle the sub-section of the DOM.
	// If it can't be found then no data can be created.
	const StreamSerializerXML* xmlStreamSerializer = dynamic_cast<const StreamSerializerXML*>( adsk::Data::StreamSerializer::formatByName( xmlFormatType ) );
	if( ! xmlStreamSerializer )
	{
		REPORT_ERROR_AT_LINE(kChannelXMLStreamSerializerMissing, 0.0);
		return 1;
	}

	// The XML header is not written out since the Channel XML is a subsection
	// of the metadata XML
	//cDst << "<?xml version='1.0' encoding='UTF-8'?>"  << std::endl;

	// Start with the main Channel tag
	cDst << xmlTagChannelIndent << "<" << xmlTagChannel << ">" << std::endl;

	// Create the Channel name tag
	cDst << xmlTagStreamIndent << "<" << xmlTagChannelName << ">"
		 << dataToWrite.name() << "</" << xmlTagChannelName << ">" << std::endl;

	// Write out the Stream data
	for( unsigned int s=0; s<dataToWrite.dataStreamCount(); ++s )
	{
		const adsk::Data::Stream* theStream = dataToWrite.dataStream( s );
		// Don't bother writing out empty Streams
		if( ! theStream ) continue;

		errorCount += xmlStreamSerializer->write( *theStream, cDst, errors );
	}

	cDst << xmlTagChannelIndent << "</" << xmlTagChannel << ">" << std::endl;
	return errorCount;
}

//----------------------------------------------------------------------
//
//! \brief Get a description of the XML Channel format
//
//	This actually describes the entire XML metadata format, only a subset
//	of which is the Channel data.
//
//! \param[out] info stream to which the XML format description is output
//
void
ChannelSerializerXML::getFormatDescription(
	std::ostream&	info ) const
{
	MStatus status;
	MString description = MStringResource::getString(kAssociationsXMLInfo, status);
	info << description.asChar();
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
