#ifndef streamSerializerXML_h
#define streamSerializerXML_h

#include <maya/adskDataStreamSerializer.h>	// for base class
#include <libxml/tree.h>				// for xmlNode
#include <maya/adskCommon.h>

namespace adsk {
  namespace Data {
	class Stream;
  }
}

// ****************************************************************************
/*!
	\class adsk::Data::Plugin::StreamSerializerXML
 	\brief Class handling the data Stream format type "XML"

	The XML format is an example of a plug-in that creates a new metadata
	serialization type. The initializePlugin method creates a serializer
	information object which automatically registers it so that it
	becomes available anywhere the serialization type is referenced (by name).

	The "XML" format is a metadata format using XML syntax. Its format
	is explicitly defined in the accompanying file metadataSchema.xsd but
	here's a quick summary of what it contains at this level.

		  <stream>
		    <name='STREAM_NAME'/>
			<structure='STREAM_STRUCTURE'/>
			<indexType='STREAM_TYPE'>
			<!-- One per metadata element defined in the stream  -->
			<!-- Repeated elements used for members with dim > 0 -->
			<data>
			  <index>INDEX_VALUE</index>
			  <FIELD1>FIELD1_VALUE</FIELD1>
			  <FIELD2>FIELD2_VALUE_DIM[0]</FIELD2>
			  <FIELD2>FIELD2_VALUE_DIM[1]</FIELD2>
			  <FIELD2>FIELD2_VALUE_DIM[2]</FIELD2>
		  </stream>
	
		  INDEX_VALUE has a type corresponding to the index type used
		  by the Stream (which if unspecified defaults to numeric)

		  FIELD# is the name of a Structure member whose value is being specified.
		  FIELD#_VALUE is the value of that Structure member, in string form.

		  If any of the Structure members do not appear as subtags of the
		  <data> tag then they will assume the default value.
*/
using namespace adsk::Data;
class StreamSerializerXML : public adsk::Data::StreamSerializer
{
	DeclareSerializerFormat(StreamSerializerXML, adsk::Data::StreamSerializer);
public:
	virtual ~StreamSerializerXML();

	// Mandatory implementation overrides
	virtual adsk::Data::Stream*
						read		(std::istream&		cSrc,
									 std::string&		errors)		const;
	virtual int			write		(const adsk::Data::Stream&	dataToWrite,
									 std::ostream&		cDst,
									 std::string&		errors)		const;
	virtual void		getFormatDescription(std::ostream& info)	const;

	// Partial interface to allow passing off parsing of a subsection of a
	// metadata DOM to the Stream subsection
	adsk::Data::Stream* parseDOM	(xmlDocPtr		doc,
									 xmlNode&		streamNode,
									 unsigned int&	errorCount,
									 std::string&	errors )	const;

private:
	StreamSerializerXML();			//! Use theFormat() to create.
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
#endif // structureSerializerXML_h
