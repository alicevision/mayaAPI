#ifndef _metadataXML_h_
#define _metadataXML_h_

#include <libxml/globals.h>
#include <libxml/xmlreader.h>
#include <libxml/parser.h>

///////////////////////////////////////////////////////////////////////////////
//
// Contains shared information for the XML plug-ins
//
////////////////////////////////////////////////////////////////////////////////

// XML tag strings. Not translated from English since they are keyworkds but
// shared among more than one serialization class.
//
#define xmlFormatType				"XML_DOM"
//
// Structure format tags
//
#define xmlTagStructure				"structure"
#define xmlTagStructureName			"name"
#define xmlTagStructureMember		"member"
#define xmlTagStructureMemberDim	"dim"
#define xmlTagStructureMemberName	"name"
#define xmlTagStructureMemberType	"type"
//
// Stream format tags
//
#define xmlTagStream				"stream"
#define xmlTagStreamData			"data"
#define xmlTagStreamDataIndent		"            "
#define xmlTagStreamDataIndex		"index"
#define xmlTagStreamDataValueIndent	"                "
#define xmlTagStreamHasDefault		"hasDefault"
#define xmlTagStreamIndent			"        "
#define xmlTagStreamIndexType		"indexType"
#define xmlTagStreamMember			"member"
#define xmlTagStreamName			"name"
#define xmlTagStreamStructure		"structure"
//
// Channel format tags
//
#define xmlTagChannel				"channel"
#define xmlTagChannelIndent			"    "
#define xmlTagChannelName			"name"
//
// Associations format tags
//
#define xmlTagAssociations			"associations"

// Handy utility macros for reporting errors by line number
//
#define REPORT_ERROR( ERROR )									\
	{															\
	  MStatus status;											\
	  MString msg = MStringResource::getString(ERROR, status);	\
	  if( errorCount > 0 ) errors += '\n';						\
	  errors += msg.asChar();									\
	  errors += '\n';											\
	  errorCount++;												\
	}
#define REPORT_ERROR_AT_LINE( ERROR, LINE )						\
	{															\
	  MStatus status;											\
	  MString fmt = MStringResource::getString(ERROR, status);	\
	  MString lineNo;											\
	  lineNo = LINE;											\
	  MString msg;												\
	  msg.format( fmt, lineNo );								\
	  if( errorCount > 0 ) errors += '\n';						\
	  errors += msg.asChar();									\
	  errors += '\n';											\
	  errorCount++;												\
	}
#define REPORT_ERROR_AT_LINE1( ERROR, MSG, LINE )				\
	{															\
	  MStatus status;											\
	  MString fmt = MStringResource::getString(ERROR, status);	\
	  MString lineNo;											\
	  lineNo = LINE;											\
	  MString msg;												\
	  msg.format( fmt, MSG, lineNo );							\
	  if( errorCount > 0 ) errors += '\n';						\
	  errors += msg.asChar();									\
	  errors += '\n';											\
	  errorCount++;												\
	}
#define REPORT_ERROR_AT_LINE2( ERROR, MSG1, MSG2, LINE )		\
	{															\
	  MStatus status;											\
	  MString fmt = MStringResource::getString(ERROR, status);	\
	  MString lineNo;											\
	  lineNo = LINE;											\
	  MString msg;												\
	  msg.format( fmt, MSG1, MSG2, lineNo );					\
	  if( errorCount > 0 ) errors += '\n';						\
	  errors += msg.asChar();									\
	  errors += '\n';											\
	  errorCount++;												\
	}

//----------------------------------------------------------------------
//
//! Helper methods for extracting information out of the XML DOM
//
namespace adsk {
	namespace Data {
		namespace XML {

class Util
{
public:
	static xmlNode* findNamedNode( xmlNode*	rootNode, const char* childName );
	static xmlChar* findText	 ( xmlDocPtr doc, xmlNode* node );
};

		} // namespace XML
	} // namespace Data
} // namespace adsk

//-
//==========================================================================
// Copyright (c) 2012 Autodesk, Inc.
// All rights reserved.
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//==========================================================================
//+

#endif // _metadataXML_h_
