#include "streamSerializerXML.h"
#include "metadataXML.h"
#include "metadataXMLPluginStrings.h"
#include <sstream>
#include <string>
#include <map>
#include <stdlib.h>
#include <string.h>
#include <maya/MString.h>
#include <maya/MStringResource.h>
#include <maya/adskDataIndex.h>
#include <maya/adskDataStream.h>
#include <maya/adskDataStructure.h>
#include <maya/adskDataStreamSerializer.h>
#include <assert.h>
#include <libxml/globals.h>
#include <libxml/xmlreader.h>
#include <libxml/parser.h>

using namespace adsk;
using namespace adsk::Data;
using namespace adsk::Data::XML;

ImplementSerializerFormat(StreamSerializerXML,StreamSerializer,xmlFormatType);

//----------------------------------------------------------------------
//
//! \brief Default constructor, does nothing
//
StreamSerializerXML::StreamSerializerXML()
{
}

//----------------------------------------------------------------------
//
//! \brief Default destructor, does nothing
//
StreamSerializerXML::~StreamSerializerXML	()
{
}

//----------------------------------------------------------------------
//
//! \brief Create a Stream based on the XML-formatted data in the input stream.
//
//	This is not normally called directly as a Stream cannot float freely
//	without a Channel parent to connect it with an object. The Channel
//	parser will call the parseDOM() method below to parse a partial tree.
//
//! \param[in] cSrc Stream containing the XML format data to be parsed
//! \param[out] errors Description of problems found when parsing the string
//
//! \return The created Stream, NULL if there was an error creating it
//
adsk::Data::Stream*
StreamSerializerXML::read(
	std::istream&	cSrc,
	std::string&	errors )	const
{
	MStatus status;
	unsigned int errorCount = 0;
	adsk::Data::Stream* newStream = NULL;
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
	xmlNode* rootEl = Util::findNamedNode( xmlDocGetRootElement(doc), xmlTagStream );

	// Walk the DOM and create the Stream from it
	newStream = parseDOM( doc, *rootEl, errorCount, errors );

	delete[] memblock;

	// Free the document
	xmlFreeDoc(doc);

	// If there were errors any Stream created will be incorrect so pass
	// back nothing rather than bad data.
	if( errorCount > 0 )
	{
		delete newStream;
		newStream = NULL;
	}
	return newStream;
}

//----------------------------------------------------------------------
//
//! \brief Create a Stream based on a partial XML DOM tree
//
//! \param[in] doc Pointer to the XML DOM being parsed
//! \param[in] streamNode Root of the DOM containing the Stream data
//! \param[out] errorCount Number of errors found in parsing
//! \param[out] errors Description of problems found when parsing the string
//
//! \return The created Stream, even if partially complete
//
adsk::Data::Stream*
StreamSerializerXML::parseDOM(
	xmlDocPtr		doc,
	xmlNode&		streamNode,
	unsigned int&	errorCount,
	std::string&	errors )	const
{
	adsk::Data::Stream* newStream = NULL;

	// Make sure the root is a <stream> tag, otherwise nothing can be done
	if( 0 != strcmp( (const char*)streamNode.name, xmlTagStream ) )
	{
		return NULL;
	}

	// <name> tag
	xmlNode* nameNode = Util::findNamedNode( streamNode.children, xmlTagStreamName );
	const char* streamName = nameNode ? (const char*) Util::findText( doc, nameNode ) : NULL;
	if( ! streamName )
	{
		REPORT_ERROR_AT_LINE(kStreamXMLStreamNameMissing, streamNode.line);
		return newStream;
	}

	// <structure> tag - verify the named structure exists as well
	MString structureName;
	Structure* structure = NULL;
	xmlNode* structureNode = Util::findNamedNode( streamNode.children, xmlTagStreamStructure );
	if( !structureNode )
	{
		REPORT_ERROR_AT_LINE1(kStreamXMLStructureNotFound, structureName, streamNode.line)
		return NULL;
	}
	else
	{
		structureName = (const char*) Util::findText( doc, structureNode );
	}

	structure = Structure::structureByName( structureName.asChar() );
	if( ! structure )
	{
		if( structureName.length() > 0 )
		{
			REPORT_ERROR_AT_LINE1(kStreamXMLStructureNotFound, structureName, streamNode.line)
		}
		else
		{
			REPORT_ERROR_AT_LINE(kStreamXMLStructureMissing, streamNode.line);
		}
		return newStream;
	}

	// Optional <indexType> tag
	xmlNode* indexTypeNode = Util::findNamedNode( streamNode.children, xmlTagStreamIndexType );
	const char* indexTypeName = indexTypeNode ? (const char*)Util::findText(doc, indexTypeNode) : NULL;

	// Okay allocate this here inside the DLL since it will be deleted before
	// parsing is complete. The allocated object is just a wrapper around the
	// real data, which will be allocated properly inside the main app.
	newStream = new adsk::Data::Stream( *structure, streamName );

	if( indexTypeName )
	{
		std::string indexTypeStr( indexTypeName );
		if( ! newStream->setIndexType( indexTypeStr ) )
		{
			MString invalidIndexType( indexTypeName );
			REPORT_ERROR_AT_LINE1(kStreamXMLIndexTypeInvalid, invalidIndexType, streamNode.line);
			delete newStream;
			return NULL;
		}
	}

	adsk::Data::Index::IndexCreator indexCreator = adsk::Data::Index::creator( newStream->indexType() );

	if( ! indexCreator )
	{
		MString invalidIndexType( indexTypeName );
		REPORT_ERROR_AT_LINE1(kStreamXMLIndexTypeInvalid, invalidIndexType, streamNode.line);
		delete newStream;
		return NULL;
	}

	if( streamNode.children && newStream )
	{
		for( xmlNode* dataNode = streamNode.children;
			 dataNode; dataNode = dataNode->next )
		{
			// Ignore everything but the <data> tags
			if( (dataNode->type != XML_ELEMENT_NODE)
			||  (0 != strcmp( (const char*)dataNode->name, xmlTagStreamData) ) )
			{
				continue;
			}

			adsk::Data::Handle newValue( *structure );
			adsk::Data::Index dataIndex;
			bool foundIndex = false;

			// Use this to keep track of how many tags of a given type have
			// been hit so far in the parsing.
			std::map<std::string,int> tagDimensions;

			// Construct the Handle for the new data and populate it
			// with the index tag value and the values in all of the tags whose
			// name matches a Structure member name.
			for( xmlNode* valueNode = dataNode->children;
				 valueNode; valueNode = valueNode->next )
			{
				// Skip non-element types, they have no useful information
				if( valueNode->type != XML_ELEMENT_NODE )
				{
					continue;
				}

				// The <index> tag is found by name
				if( 0 == strcmp( (const char*)valueNode->name, xmlTagStreamDataIndex ) )
				{
					// Set the index value
					dataIndex = indexCreator( (const char*)Util::findText( doc, valueNode ) );
					foundIndex = true;
				}
				// Other tags have names that are dynamically based on
				// structure member names so verify them as we go.
				else
				{
					std::string valueName( (const char*)valueNode->name );
					if( newValue.setPositionByMemberName( (const char*)valueNode->name ) )
					{
						// Parse the Handle data
						std::string handleValue;
						const char* textValue = (const char*)Util::findText( doc, valueNode );
						if( textValue )
						{
							handleValue = textValue;
						}
						if( tagDimensions.find(valueName) == tagDimensions.end() )
						{
							tagDimensions[valueName] = 0;
						}
						else
						{
							tagDimensions[valueName]++;
						}
						unsigned int parseErrors = newValue.fromStr( handleValue, tagDimensions[valueName], errors );
						if( parseErrors > 0 )
						{
							MString invalidValue( handleValue.c_str() );
							MString invalidMember( (const char*) valueNode->name );
							REPORT_ERROR_AT_LINE2(kStreamXMLMemberValueInvalid, invalidValue, invalidMember, dataNode->line);
							errorCount += parseErrors;
						}
					}
					else
					{
						// Report the warning about unrecognized member.
						// Do not increment the errorCount since ignoring it
						// is a reasonable course of action in XML-land.
						MString invalidMember( (const char*) valueNode->name );
						REPORT_ERROR_AT_LINE1(kStreamXMLMemberNameInvalid, invalidMember, dataNode->line);
						errorCount--;
					}
				}
			}

			if( foundIndex )
			{
				// Set the handle into the stream
				if( ! newStream->setElement( dataIndex, newValue ) )
				{
					REPORT_ERROR_AT_LINE(kStreamXMLSetValueFailed, dataNode->line);
					errorCount--;
				}
			}
			else
			{
				// Report the missing index value
				REPORT_ERROR_AT_LINE(kStreamXMLMissingIndex, dataNode->line);
			}
		}
	}

	return newStream;
}

//----------------------------------------------------------------------
//
//! \brief Write the Stream object in XML format into the output stream
//
//! \param[in] dataToWrite Stream to be formatted
//! \param[out] cDst Output stream to which the XML format of the Stream is written
//! \param[out] errors Description of problems found when writing the Stream
//
//! \return number of errors found during write, 0 means success
//
int
StreamSerializerXML::write(
	const adsk::Data::Stream&	dataToWrite,
	std::ostream&				cDst,
	std::string&				errors )	const
{
	errors = "";

	// The XML header is not written out since the Stream XML is a subsection
	// of the metadata XML
	//cDst << "<?xml version='1.0' encoding='UTF-8'?>"  << std::endl;

	// Start with the main Stream tag
	cDst << xmlTagStreamIndent << "<"
		 << xmlTagStream << ">" << std::endl;

	// Order is important here - first <name> then <structure> then <indexType>
	cDst << xmlTagStreamDataIndent << "<"
		 << xmlTagStreamName << ">" << dataToWrite.name() << "</" << xmlTagStreamName << ">" << std::endl;
	cDst << xmlTagStreamDataIndent << "<"
		 << xmlTagStreamStructure << ">" << dataToWrite.structure().name() << "</" << xmlTagStreamStructure << ">" << std::endl;
	cDst << xmlTagStreamDataIndent << "<"
		 << xmlTagStreamIndexType << ">" << dataToWrite.indexType() << "</" << xmlTagStreamIndexType << ">" << std::endl;

	// Write out the Stream data
	for( adsk::Data::Stream::iterator sIter = dataToWrite.cbegin();
		 sIter != dataToWrite.cend(); ++sIter )
	{
		adsk::Data::Handle& idHandle = (*sIter);

		// Write the outer tag indicating metadata information
		adsk::Data::Index theIndex = sIter.index();
		cDst << xmlTagStreamDataIndent << "<" << xmlTagStreamData << ">" << std::endl;

		// First inner tag is the index, shared by all values
		cDst << xmlTagStreamDataValueIndent << "<" << xmlTagStreamDataIndex << ">"
			 << theIndex.asString() << "</" << xmlTagStreamDataIndex << ">" << std::endl;

		// Walk all structure members to add child tags
		for( adsk::Data::Structure::iterator structIt = dataToWrite.structure().begin();
			 structIt != dataToWrite.structure().end(); ++structIt )
		{
			idHandle.setPositionByMemberIndex( structIt.index() );
			for( unsigned int dim=0; dim<structIt->length(); ++dim )
			{
				cDst << xmlTagStreamDataValueIndent << "<"
					 << structIt->name()
					 << ">"
					 << idHandle.str(dim)
					 << "</" << structIt->name() << ">" << std::endl;
			}
		}

		// Close the data member tag
		cDst << xmlTagStreamDataIndent << "</" << xmlTagStreamData << ">" << std::endl;
	}

	cDst << xmlTagStreamIndent << "</" << xmlTagStream << ">" << std::endl;
	return 0;
}

//----------------------------------------------------------------------
//
//! \brief Get a description of the XML Stream format
//
//	This actually describes the entire XML metadata format, only a subset
//	of which is the Stream data.
//
//! \param[out] info Stream to which the XML format description is output
//
void
StreamSerializerXML::getFormatDescription(
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
