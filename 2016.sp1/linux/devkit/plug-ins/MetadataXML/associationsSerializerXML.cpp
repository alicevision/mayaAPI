#include "associationsSerializerXML.h"
#include "channelSerializerXML.h"
#include "metadataXML.h"
#include "metadataXMLPluginStrings.h"
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <maya/MString.h>
#include <maya/MStringResource.h>
#include <maya/adskDataAssociations.h>
#include <maya/adskDataChannel.h>
#include <maya/adskDataAssociationsSerializer.h>
#include <maya/adskDataChannelSerializer.h>
#include <assert.h>
#include <libxml/globals.h>
#include <libxml/xmlreader.h>
#include <libxml/parser.h>

using namespace adsk;
using namespace adsk::Data;
using namespace adsk::Data::XML;

ImplementSerializerFormat(AssociationsSerializerXML,AssociationsSerializer,xmlFormatType);

//----------------------------------------------------------------------
//
//! \brief Default constructor, does nothing
//
AssociationsSerializerXML::AssociationsSerializerXML()
{
}

//----------------------------------------------------------------------
//
//! \brief Default destructor, does nothing
//
AssociationsSerializerXML::~AssociationsSerializerXML	()
{
}

//----------------------------------------------------------------------
//
//! \brief Create a Associations based on the XML-formatted data in the input stream.
//
//! \param[in] cSrc Input stream containing the XML format data to be parsed
//! \param[out] errors Description of problems found when parsing the string
//
//! \return The created Associations, NULL if there was an error creating it
//
adsk::Data::Associations*
AssociationsSerializerXML::read(
	std::istream&	cSrc,
	std::string&	errors )	const
{
	unsigned int errorCount = 0;
	adsk::Data::Associations* newAssociations = NULL;
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

	xmlNode* mainNode = Util::findNamedNode( xmlDocGetRootElement(doc), xmlTagAssociations );

	// Root must be a <associations> tag with no attributes followed by a list
	// of <channel> tags containing the XML Channel data. Pass on parsing of
	// the <channel> child to the Channel parser.
	if( ! mainNode )
	{
		return NULL;
	}

	newAssociations = parseDOM( doc, *mainNode, errorCount, errors );

	delete[] memblock;

	// Free the document
	xmlFreeDoc(doc);

	// If there were errors any Stream created will be incorrect so pass
	// back nothing rather than bad data.
	if( errorCount > 0 )
	{
		delete newAssociations;
		newAssociations = NULL;
	}
	return newAssociations;
}

//----------------------------------------------------------------------
//
//! \brief Create Associations based on a partial XML DOM tree
//
//! \param[in] doc Pointer to XML DOM object
//! \param[in] associationsNode Root of the DOM containing the Associations data
//! \param[out] errorCount Number of errors found in parsing
//! \param[out] errors Description of problems found when parsing the string
//
//! \return The created Stream, even if partially complete
//
adsk::Data::Associations*
AssociationsSerializerXML::parseDOM(
	xmlDocPtr		doc,
	xmlNode&		associationsNode,
	unsigned int&	errorCount,
	std::string&	errors )	const
{
	// Get the Channel serializer to handle the sub-section of the DOM.
	// If it can't be found then no data can be created.
	const ChannelSerializerXML* xmlChannelSerializer = dynamic_cast<const ChannelSerializerXML*>( adsk::Data::ChannelSerializer::formatByName( xmlFormatType ) );
	if( ! xmlChannelSerializer )
	{
		REPORT_ERROR(kAssociationsXMLChannelSerializerMissing);
		return NULL;
	}

	adsk::Data::Associations* newAssociations = adsk::Data::Associations::create();

	// The <channel> tags are the children of the <associations> tag
	for( xmlNode* channelNode = associationsNode.children;
		 channelNode; channelNode = channelNode->next )
	{
		adsk::Data::Channel* newChannel = NULL;

		// Skip over any non-<channel> tags
		if( (channelNode->type != XML_ELEMENT_NODE)
		||  (0 != strcmp((const char*)channelNode->name, xmlTagChannel) ) )
		{
			continue;
		}

		// It's an error to have more than one channel in an association
		if( newChannel )
		{
			REPORT_ERROR_AT_LINE(kChannelXMLTooManyChannels, channelNode->line);
			continue;
		}

		newChannel = xmlChannelSerializer->parseDOM( doc, *channelNode, errorCount, errors );

		if( newChannel && (errorCount == 0) )
		{
			newAssociations->setChannel( *newChannel );
		}

		// The parser allocated this object but it's no longer needed so
		// release it.
		delete newChannel;
	}

	// If there were errors any Associations created will be incorrect so pass
	// back nothing rather than bad data.
	if( errorCount > 0 )
	{
		delete newAssociations;
		newAssociations = NULL;
	}

	return newAssociations;
}

//----------------------------------------------------------------------
//
//! \brief Output the Associations object in XML format into the stream
//
//! \param[in] dataToWrite Associations to be formatted
//! \param[out] cDst Stream to which the XML format of the Associations is written
//! \param[out] errors Description of problems found when writing the Associations
//
//! \return number of errors found during write, 0 means success
//
int
AssociationsSerializerXML::write(
	const adsk::Data::Associations&	dataToWrite,
	std::ostream&					cDst,
	std::string&					errors )	const
{
	unsigned int errorCount = 0;
	// Get the Channel serializer to handle the sub-section of the DOM.
	// If it can't be found then no data can be created.
	const ChannelSerializerXML* xmlChannelSerializer = dynamic_cast<const ChannelSerializerXML*>( adsk::Data::ChannelSerializer::formatByName( xmlFormatType ) );
	if( ! xmlChannelSerializer )
	{
		REPORT_ERROR_AT_LINE(kAssociationsXMLChannelSerializerMissing, 0.0);
		return 1;
	}

	// Standard header boilerplate
	cDst << "<?xml version='1.0' encoding='UTF-8'?>"  << std::endl;

	// The entire object is put into the <associations> tag
	cDst << "<" << xmlTagAssociations << ">" << std::endl;

	// Write out the Associations/Channel data
	for( unsigned int s=0; s<dataToWrite.channelCount(); ++s )
	{
		const adsk::Data::Channel theChannel = dataToWrite.channelAt( s );
		errorCount += xmlChannelSerializer->write( theChannel, cDst, errors );
	}

	// Close the <associations> tag
	cDst << "</" << xmlTagAssociations << ">" << std::endl;
	return errorCount;
}

//----------------------------------------------------------------------
//
//! \brief Get a description of the XML Associations format
//
//! \param[out] info Stream to which the XML format description is output
//
void
AssociationsSerializerXML::getFormatDescription(
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
