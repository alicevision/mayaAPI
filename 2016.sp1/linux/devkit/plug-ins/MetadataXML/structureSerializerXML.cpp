#include "structureSerializerXML.h"
#include "metadataXML.h"
#include "metadataXMLPluginStrings.h"
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <maya/MString.h>
#include <maya/MStringResource.h>
#include <maya/adskDataStructure.h>
#include <assert.h>
#include <libxml/globals.h>
#include <libxml/xmlreader.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

ImplementSerializerFormat(StructureSerializerXML,StructureSerializer,xmlFormatType);

//----------------------------------------------------------------------
//
//! \brief Default constructor, does nothing
//
StructureSerializerXML::StructureSerializerXML()
{
}

//----------------------------------------------------------------------
//
//! \brief Default destructor, does nothing
//
StructureSerializerXML::~StructureSerializerXML	()
{
}

//----------------------------------------------------------------------
//
//! \brief Create a Structure based on the XML-formatted data in the input stream.
//
//! \param[in] cSrc Stream containing the XML format data to be parsed
//! \param[out] errors Description of problems found when parsing the string
//
//! \return The created structure, NULL if there was an error creating it
//
Structure*
StructureSerializerXML::read(
	std::istream&			cSrc,
	std::string&			errors )	const
{
	unsigned int errorCount = 0;
	Structure* newStructure = NULL;
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

	xmlNode* structureNode = XML::Util::findNamedNode( xmlDocGetRootElement(doc), xmlTagStructure );
	do
	{
		if( ! structureNode )
		{
			REPORT_ERROR_AT_LINE(kStructureXMLStructureTagNotFound, 1);
			break;
		}

		// Below the <structure> tag is a <name> tag and a set of <member> tags.
		xmlNode* structureNameNode = XML::Util::findNamedNode( structureNode->children, xmlTagStructureName );

		// First process the <name> tag to get the structure name
		xmlChar* structureName = XML::Util::findText( doc, structureNameNode );
		if( ! structureName )
		{
			REPORT_ERROR_AT_LINE(kStructureXMLStructureNameNotFound, structureNameNode ? structureNameNode->line : structureNode->line);
			break;
		}

		// Need to create the structure this way so that on Windows the
		// structure is not in the plug-in's heap, which would cause crashes
		// when the main application tries to delete it or the plug-in is
		// unloaded.
		newStructure = Structure::create();

		newStructure->setName( (const char*)structureName );
		xmlFree( structureName );
	
		// Next walk the list of <member> child tags
		for (xmlNode* childNode = structureNode->children; childNode; childNode = childNode->next)
		{
			// Skip anything unrecognized, for maximum flexibility
			if( (childNode->type != XML_ELEMENT_NODE)
			||  ( 0 != strcmp((const char*)childNode->name, xmlTagStructureMember) ) )
			{
				continue;
			}

			// <dim> tag is optional for all members
			xmlNode* memberDimNode = XML::Util::findNamedNode( childNode->children, xmlTagStructureMemberDim );
			int memberDim = 1;
			if( memberDimNode )
			{
				xmlChar* value = XML::Util::findText( doc, memberDimNode );
				if( value )
				{
					memberDim = atol( (const char*)value );
					xmlFree( value );
				}
				else
				{
					REPORT_ERROR_AT_LINE( kStructureXMLMemberDimNotFound, memberDimNode->line );
					break;
				}
			}

			// <name> tag is mandatory for all members
			xmlNode* memberNameNode = XML::Util::findNamedNode( childNode->children, xmlTagStructureMemberName );
			xmlChar* memberName = memberNameNode ? XML::Util::findText( doc, memberNameNode ) : NULL;
			if( ! memberName )
			{
				REPORT_ERROR_AT_LINE(kStructureXMLMemberNameNotFound, (memberNameNode ? memberNameNode->line : childNode->line));
				break;
			}

			// <type> tag is mandatory for all members
			xmlNode* memberTypeNode = XML::Util::findNamedNode( childNode->children, xmlTagStructureMemberType );
			xmlChar* memberType = memberTypeNode ? XML::Util::findText( doc, memberTypeNode ) : NULL;
			if( ! memberType )
			{
				REPORT_ERROR_AT_LINE(kStructureXMLMemberTypeNotFound, (memberTypeNode ? memberTypeNode->line : childNode->line));
				xmlFree( memberName );
				break;
			}
			Member::eDataType dataType = Member::typeFromName((const char*)memberType);
			if( dataType == Member::kInvalidType )
			{
				REPORT_ERROR_AT_LINE(kStructureXMLMemberTypeInvalid, memberTypeNode->line);
			}
			else
			{
				newStructure->addMember( dataType, memberDim, (const char*) memberName );
			}
			xmlFree( memberType );
			xmlFree( memberName );
		}
		break;
	} while( true );

	delete[] memblock;

	// Free the document
	xmlFreeDoc(doc);

	// If there were errors any structure created will be incorrect so pass
	// back nothing rather than bad data.
	if( errorCount > 0 )
	{
		// Sneaky little trick to get the structure to delete itself even
		// though the destructor cannot be called directly.
		if( newStructure )
		{
			newStructure->ref();
			newStructure->unref();
			newStructure = NULL;
		}
	}
	return newStructure;
}

//----------------------------------------------------------------------
//
//! \brief Output the Structure object in XML format into the stream
//
//! \param[in] dataToWrite Structure to be formatted
//! \param[out] cDst Stream to which the XML format of the structure is written
//
//! \return number of errors found during write, 0 means success
//
int
StructureSerializerXML::write(
	const Structure&	dataToWrite,
	std::ostream&		cDst )	const
{
	cDst << "<?xml version='1.0' encoding='UTF-8'?>"  << std::endl;

	xmlChar* structureName = xmlEncodeSpecialChars(NULL, (const xmlChar*)dataToWrite.name());

	// Start with the main structure tag containing the name
	cDst << "<" << xmlTagStructure << ">" << std::endl;
	cDst << "    <" << xmlTagStructureName << ">"
					<< structureName
		 << "    </" << xmlTagStructureName << ">" << std::endl;
	cDst << "</" << xmlTagStructureName << ">" << std::endl;

	xmlFree( structureName );

	// Write out each structure member in its own tag
	for( Structure::iterator structIt = dataToWrite.begin();
		 structIt != dataToWrite.end();
		 ++structIt )
	{
		xmlChar* memberName = xmlEncodeSpecialChars(NULL, (const xmlChar*)structIt->name());
		xmlChar* memberType = xmlEncodeSpecialChars(NULL, (const xmlChar*)Member::typeName(structIt->type()));

		cDst << "    <" << xmlTagStructureMember << ">" << std::endl;
		cDst << "        <" << xmlTagStructureMemberName << ">" << memberName
			 << "        </" << xmlTagStructureMemberName << ">" << std::endl;
		cDst << "        <" << xmlTagStructureMemberType << ">" << memberType
			 << "        </" << xmlTagStructureMemberType << ">" << std::endl;
		if( structIt->length() != 1 )
		{
			cDst << "        <" << xmlTagStructureMemberDim << ">" << structIt->length()
				 << "        </" << xmlTagStructureMemberDim << ">" << std::endl;
		}
		cDst << "    </" << xmlTagStructureMember << ">" << std::endl;

		xmlFree( memberName );
		xmlFree( memberType );
	}
	cDst << "</" << xmlTagStructure << ">" << std::endl;
	return 0;
}

//----------------------------------------------------------------------
//
//! \brief Get a description of the XML structure format
//
//! \param[out] info Stream to which the XML format description is output
//
void
StructureSerializerXML::getFormatDescription(
	std::ostream&	info ) const
{
	MStatus status;
	// The message is split into two parts to make it easier to insert the
	// list of accepted structure member types.
	MString msgPre = MStringResource::getString(kStructureXMLInfoPre, status);
	MString msgPost = MStringResource::getString(kStructureXMLInfoPost, status);

	info << msgPre.asChar();
	for( unsigned int i=Member::kFirstType; i<Member::kLastType; i++ )
	{
		if( i != Member::kFirstType ) info << ", ";
		info << Member::typeName((Member::eDataType)i);
	}
	info << msgPost.asChar();
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
