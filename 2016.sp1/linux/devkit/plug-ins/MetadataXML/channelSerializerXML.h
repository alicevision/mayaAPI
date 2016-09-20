#ifndef channelSerializerXML_h
#define channelSerializerXML_h

#include <maya/adskDataChannelSerializer.h>	// for base class
#include <libxml/tree.h>				// for xmlNode
#include <maya/adskCommon.h>

namespace adsk {
  namespace Data {
	class Channel;
  }
}

// ****************************************************************************
/*!
	\class adsk::Data::Plugin::ChannelSerializerXML
 	\brief Class handling the data Channel format type "XML"

	The XML format is an example of a plug-in that creates a new metadata
	serialization type. The initializePlugin method creates a serializer
	information object which automatically registers it so that it
	becomes available anywhere the serialization type is referenced (by name).

	The "XML" format is a metadata format using XML syntax. Its format
	is explicitly defined in the accompanying file metadataSchema.xsd but
	here's a quick summary of what it contains at this level.

		  <channel>
			<stream>   <!-- Parsed by StreamSerializerXML -->
			...
			</stream>
		  </channel>
*/
using namespace adsk::Data;
class ChannelSerializerXML : public ChannelSerializer
{
	DeclareSerializerFormat(ChannelSerializerXML, ChannelSerializer);
public:
	virtual ~ChannelSerializerXML();

	// Mandatory implementation overrides
	virtual adsk::Data::Channel*
						read		(std::istream&		cSrc,
									 std::string&		errors)		const;
	virtual int			write		(const adsk::Data::Channel&	dataToWrite,
									 std::ostream&		cDst,
									 std::string&		errors)		const;
	virtual void		getFormatDescription(std::ostream& info)	const;

	// Partial interface to allow passing off parsing of a subsection of a
	// metadata DOM to the Stream subsection
	adsk::Data::Channel* parseDOM	(xmlDocPtr		doc,
									 xmlNode&		channelNode,
									 unsigned int&	errorCount,
									 std::string&	errors )	const;

private:
	ChannelSerializerXML();			//! Use theFormat() to create.
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
#endif // channelSerializerXML_h
