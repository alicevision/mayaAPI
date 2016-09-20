#ifndef associationsSerializerXML_h
#define associationsSerializerXML_h

#include <maya/adskDataAssociationsSerializer.h>	// for base class
#include <libxml/tree.h>				// for xmlNode
#include <maya/adskCommon.h>

namespace adsk {
  namespace Data {
	class Associations;
  }
}

// ****************************************************************************
/*!
	\class adsk::Data::Plugin::AssociationsSerializerXML
 	\brief Class handling the data Associations format type "XML"

	The XML format is an example of a plug-in that creates a new metadata
	serialization type. The initializePlugin method creates a serializer
	information object which automatically registers it so that it
	becomes available anywhere the serialization type is referenced (by name).

	The "XML" format is a metadata format using XML syntax. Its format
	is explicitly defined in the accompanying file metadataSchema.xsd but
	here's a quick summary of what it contains at this level.

		<?xml version='1.0' encoding='UTF-8'?>
		<associations>
			<channel>   <!-- Parsed by ChannelSerializerXML -->
			...
			</channel>
		</associations>
*/
using namespace adsk::Data;
class AssociationsSerializerXML : public AssociationsSerializer
{
	DeclareSerializerFormat(AssociationsSerializerXML, AssociationsSerializer);
public:
	virtual ~AssociationsSerializerXML();

	// Mandatory implementation overrides
	virtual adsk::Data::Associations*
						read		(std::istream&		 cSrc,
									 std::string&		 errors)	const;
	virtual int			write		(const Associations& dataToWrite,
									 std::ostream&		 cDst,
									 std::string&		 errors)	const;
	virtual void		getFormatDescription(std::ostream& info)	const;

	// Partial interface to allow passing off parsing of a subsection of a
	// metadata DOM to the Stream subsection
	adsk::Data::Associations* parseDOM	(xmlDocPtr		doc,
										 xmlNode&		streamNode,
										 unsigned int&	errorCount,
										 std::string&	errors )	const;

private:
	AssociationsSerializerXML();		//! Use theFormat() to create.
};

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
#endif // associationsSerializerXML_h
