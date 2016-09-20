#ifndef _metadataPluginXMLStrings_h_
#define _metadataPluginXMLStrings_h_

#include <maya/MStringResource.h>
#include <maya/MStringResourceId.h>

// MStringResourceIds contain plug-in id, unique resource id for
// each string and the default value for the string.

#define kPluginId  "adskmetadataXMLDOM"

//######################################################################
//
// Structure XML strings
//
#define kStructureXMLMemberDimNotFound		MStringResourceId	(kPluginId, "kStructureXMLMemberDimNotFound",		"Member dim value not found at line ^1s")
#define kStructureXMLMemberNameNotFound		MStringResourceId	(kPluginId, "kStructureXMLMemberNameNotFound",		"Member name value not found at line ^1s")
#define kStructureXMLMemberTypeInvalid		MStringResourceId	(kPluginId, "kStructureXMLMemberTypeInvalid",		"Member type value not recognized at line ^1s")
#define kStructureXMLMemberTypeNotFound		MStringResourceId	(kPluginId, "kStructureXMLMemberTypeNotFound",		"Member type value not found at line ^1s")
#define kStructureXMLStructureNameNotFound	MStringResourceId	(kPluginId, "kStructureXMLStructureNameNotFound",	"Structure name not found at line ^1s")
#define kStructureXMLStructureTagNotFound	MStringResourceId	(kPluginId, "kStructureXMLStructureTagNotFound",	"Structure tag not found at line ^1s")
#define kStructureXMLInfoPre				MStringResourceId	(kPluginId, "kStructureXMLInfoPre",\
	"\nThe file structureSchema.xsd  contains  the official"\
	"\nstructure validation  format. Below is a less formal"\
	"\ndescription more useful to a non-technical user."\
	"\n"\
	"\nThe first line is always a standard boilerplate"\
	"\n    <?xml version='1.0' encoding='UTF-8'?>"\
	"\n"\
	"\nThe second line is always the main <structure> tag"\
	"\n    <structure>"\
	"\n"\
	"\nInside  of the <structure> section  is the mandatory"\
	"\nstructure name"\
	"\n    <name>STRUCTURE_NAME</name>"\
	"\n"\
	"\nfollowed by a list of member tags"\
	"\n    <member>"\
	"\n"\
	"\nInside each  <member> tag is  a mandatory <name> tag"\
	"\nand a mandatory <type> tag"\
	"\n    <name>STRUCTURE_NAME</name>"\
	"\n    <type>MEMBER_TYPE</type>"\
	"\n"\
	"\nSTRUCTURE_NAME is the name to give to the structure."\
	"\nMEMBER_TYPE is one of the valid structure member"\
	"\ntypes taken from adsk::Data::Structure:"\
	"\n    (")
#define kStructureXMLInfoPost				MStringResourceId	(kPluginId, "kStructureXMLInfoPost",\
	")\n\n"\
	"\n<dim> is an optional tag denoting the length of"\
	"\nthe  data  member  type  with  a  default of 1."\
	"\nFor example:"\
	"\n    <type>float</type>"\
	"\n    <dim>3</dim>"\
	"\n"\
	"\ndenotes a member consisting of 3 float values."\
	"\n"\
	"\nAt the end of each member is a closing member tag"\
	"\n    </member>"\
	"\n"\
	"\nAt the end of the file is a structure closing tag"\
	"\n    </structure>"\
	"\n"\
	"\nAs in all XML  the  tags may  be separated by any"\
	"\namount  of  whitespace,  including   none.  Files"\
	"\nwritten out in  this  format  will  use  standard"\
	"\nindentation  for  nested  tags  and  newlines  to"\
	"\nseparate tags.  The  original  formatting will be"\
	"\nlost after the file is read." )
//
//######################################################################

//######################################################################
//
// Associations XML strings
//
#define kAssociationsXMLChannelSerializerMissing	MStringResourceId(kPluginId, "kChannelXMLChannelSerializerMissing",	"Cannot find XML serializer for Channel data")
//
//######################################################################

//######################################################################
//
// Channel XML strings
//
#define kChannelXMLStreamSerializerMissing	MStringResourceId(kPluginId, "kChannelXMLStreamSerializerMissing",	"Cannot find XML serializer for Stream data")
#define kChannelXMLTooManyChannels			MStringResourceId(kPluginId, "kChannelXMLTooManyChannels",			"Redundant Channel at line ^1s ignored")
#define kChannelXMLNameMissing				MStringResourceId(kPluginId, "kChannelXMLNameMissing",				"Missing Channel name at line ^1s.")
//
//######################################################################

//######################################################################
//
// Stream XML strings
//
#define kStreamXMLIndexTypeInvalid		MStringResourceId(kPluginId, "kStreamXMLIndexTypeInvalid",		"Index type '^1s' not recognized at line ^2s")
#define kStreamXMLMemberNameInvalid		MStringResourceId(kPluginId, "kStreamXMLMemberNameInvalid",		"Member '^1s' not recognized at line ^2s")
#define kStreamXMLMemberValueInvalid	MStringResourceId(kPluginId, "kStreamXMLMemberValueInvalid",	"Could not parse '^1s' for member '^2s' at line ^3s")
#define kStreamXMLMissingIndex			MStringResourceId(kPluginId, "kStreamXMLMissingIndex",			"Missing mandatory attribute 'index' at line ^1s")
#define kStreamXMLSetValueFailed		MStringResourceId(kPluginId, "kStreamXMLSetValueFailed",		"Failed to set new metadata value at line ^1s")
#define kStreamXMLStreamNameMissing		MStringResourceId(kPluginId, "kStreamXMLStreamNameMissing",		"Stream name missing at line ^1s")
#define kStreamXMLStructureMissing		MStringResourceId(kPluginId, "kStreamXMLStructureNameMissing",	"Structure name missing at line ^1s")
#define kStreamXMLStructureNotFound		MStringResourceId(kPluginId, "kStreamXMLStructureNameNotFound",	"Structure '^1s' not found at line ^2s")
#define kStreamXMLTooManyStreams		MStringResourceId(kPluginId, "kStreamXMLTooManyStreams",		"Redundant Stream at line ^1s ignored")

#define kAssociationsXMLInfo			MStringResourceId(kPluginId, "kAssociationsXMLInfo",\
	"\nThe file metadataSchema.xsd contains the official"\
	"\nmetadata  validation  format,  including Streams."\
	"\nBelow is a less formal description  of the format"\
	"\nmore suited to a non-technical user. See also the"\
	"\nsample file 'metdataSample.xml' for an example of"\
	"\na full metadata file."\
	"\n"\
	"\nThe first line is always a standard boilerplate"\
	"\n    <?xml version='1.0' encoding='UTF-8'?>"\
	"\n"\
	"\nThe remainder of the  file is a series  of nested"\
	"\ntags whose structure corresponds to the structure"\
	"\nof the metadata. There is a list of 'association'"\
	"\ntags,  each  containing  a 'channel'  tag,  which"\
	"\ncontains a list  of 'stream'  tags, which in turn"\
	"\ncontain the actual metadata values."\
	"\n"\
	"\nHere is a general picture of the outer tags:"\
	"\n"\
	"\n<associations>"\
	"\n    <channel>"\
	"\n      <name>CHANNEL_NAME</name>"\
	"\n      <stream ...>"\
	"\n      </stream>"\
	"\n    </channel>"\
	"\n    <channel>"\
	"\n      <name>OTHER_CHANNEL_NAME</name>"\
	"\n      <stream ...>"\
	"\n      </stream>"\
	"\n    </channel>"\
	"\n</associations>"\
	"\n"\
	"\nThe Stream XML subsection  consists of a <stream>"\
	"\ntag containing the name, indexType of the Stream,"\
	"\nand the name of the Structure used by the Stream."\
	"\n"\
	"\n <stream>"\
	"\n   <name>STREAM_NAME</name>"\
	"\n   <structure>STRUCTURE_NAME</structure>"\
	"\n   <indexType>INDEX_TYPE_NAME</indexType>  <!-- optional -->"\
	"\n   <!-- Zero or more data tags -->"\
	"\n </stream>"\
	"\n"\
	"\nThe  indexType  tag  is optional, defaulting to a"\
	"\nsimple integer index."\
	"\n"\
	"\nThe individual metadata  element values and their"\
	"\nassociated index  values are stored in the set of"\
	"\nenclosed  <data> tags. The association is similar"\
	"\nto  an array  or Python  dictionary. The enclosed"\
	"\ntag names must match the names of  the  Structure"\
	"\nmembers. The data in the index tag must be in a"\
	"\nformat compatible to what is named in the main"\
	"\n<stream> tag's <indexType> value. For example if"\
	"\nthe Structure contained an  integer  named  'ID'"\
	"\nand  a  float[3]  named 'origin' within a Stream"\
	"\nthat uses the default integer indexType then a"\
	"\nfew sample tags are:"\
	"\n"\
	"\n <data>"\
	"\n   <index>1</index>"\
	"\n   <ID>1</ID>"\
	"\n   <origin>2.0</origin>"\
	"\n   <origin>3.0</origin>"\
	"\n   <origin>4.0</origin>"\
	"\n </data>"\
	"\n <data>"\
	"\n   <index>2</index>"\
	"\n   <ID>3</ID>"\
	"\n   <origin>0.0</origin>"\
	"\n   <origin>3.0</origin>"\
	"\n   <origin>0.0</origin>"\
	"\n </data>"\
	"\n <data>"\
	"\n   <index>3</index>"\
	"\n   <ID>19</ID>"\
	"\n   <origin>2.0</origin>"\
	"\n   <origin>3.0</origin>"\
	"\n   <origin>0.0</origin>"\
	"\n </data>"\
	"\n <data>"\
	"\n   <index>19</index>"\
	"\n   <ID>21</ID>"\
	"\n   <origin>0.0</origin>"\
	"\n   <origin>0.0</origin>"\
	"\n   <origin>0.0</origin>"\
	"\n </data>"\
	"\n"\
	"\nAs  a shortform  Structure members  not appearing"\
	"\nas attributes  in the 'metadata' tag assume their"\
	"\ndefault value.  This includes  the case  where no"\
	"\nmembers appear and the  entire Structure gets the"\
	"\ndefault values:"\
	"\n"\
	"\n <data index='999' />"\
	"\n"\
	"\nAt the end of the data is a section closing tag:"\
	"\n"\
	"\n </stream>")
//
//######################################################################

//-
//**************************************************************************/
// Copyright (c) 2012 Autodesk, Inc.
// All rights reserved.
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
//+

#endif // _metadataPluginXMLStrings_h_
