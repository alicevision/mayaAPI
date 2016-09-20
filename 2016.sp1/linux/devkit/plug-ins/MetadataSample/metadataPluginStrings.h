#ifndef _metadataPluginStrings_h_
#define _metadataPluginStrings_h_

#include <maya/MStringResource.h>
#include <maya/MStringResourceId.h>

// MStringResourceIds contain plug-in id, unique resource id for
// each string and the default value for the string.

#define kPluginId  "adskmetadata"

#define kInvalidComponentType	MStringResourceId	(kPluginId, "kInvalidComponentType",	"Component type ^1s not one of the legal values ('edge', 'face', 'vertex', 'faceVertex')")
#define kFlagMandatory			MStringResourceId	(kPluginId, "kFlagMandatory",			"Missing mandatory flag ^1s.")
#define kOnlyCreateModeMsg 		MStringResourceId	(kPluginId, "kOnlyCreateModeMsg",		"^1s flag can only be used in create mode.")
#define kInvalidFlag			MStringResourceId	(kPluginId, "kInvalidFileFlag",			"Value for flag ^1s is invalid.")
#define kFileIgnored			MStringResourceId	(kPluginId, "kFileIgnored",				"The ^1s flag is overridden by the ^2s flag. It will be ignored.")
#define kFileNotFound			MStringResourceId	(kPluginId, "kFileNotFound",			"File ^1s does not exist, import aborted.")
#define kFileOrStringNeeded		MStringResourceId	(kPluginId, "kFileOrStringNeeded",		"If no file is specified the ^1s flag must have a value.")
#define kInvalidStream			MStringResourceId	(kPluginId, "kInvalidStream",			"Stream name is not legal.")
#define kInvalidString			MStringResourceId	(kPluginId, "kInvalidString",			"String is not legal.")
#define kEditQueryFlagErrorMsg	MStringResourceId	(kPluginId, "kEditQueryFlagErrorMsg",	"Can't specify edit and query flags simultanously.")
#define kObjectNotFoundError	MStringResourceId	(kPluginId, "kObjectNotFoundError",		"Object ^1s not found")
#define kTypeUnspecified		MStringResourceId	(kPluginId, "kTypeUnspecified",			"(unspecified)")
#define kMetadataFormatNotFound	MStringResourceId	(kPluginId, "kMetadataFormatNotFound",	"Metadata format type '^1s' was not found")

#define kCreateMetadataCreateFailed			MStringResourceId	(kPluginId, "kCreateMetadataCreateFailed",		"Could not create new metadata.")
#define kCreateMetadataHasStream			MStringResourceId	(kPluginId, "kCreateMetadataHasStream",			"A stream named '^1s' already exists - skipping creation.")
#define kCreateMetadataStructureNotFound	MStringResourceId	(kPluginId, "kCreateMetadataStructureNotFound",	"Could not find structure '^1s'.")
#define kCreateMetadataNoStructureName		MStringResourceId	(kPluginId, "kCreateMetadataNoStructureName",	"The 'structureName' flag is mandatory.")
#define kCreateMetadataNoStreamName			MStringResourceId	(kPluginId, "kCreateMetadataNoStreamName",		"The 'streamName' flag is mandatory.")
#define kCreateMetadataNoChannelName		MStringResourceId	(kPluginId, "kCreateMetadataNoChannelName",		"The 'channelName' flag is mandatory.")

#define kImportMetadataStringReadFailed		MStringResourceId	(kPluginId, "kImportMetadataStringReadFailed",	"Metadata read from string arg failed with '^1s'")
#define kImportMetadataFileReadFailed		MStringResourceId	(kPluginId, "kImportMetadataFileReadFailed",	"Metadata read from file '^1s' failed with '^2s'")
#define kImportMetadataSetMetadataFailed	MStringResourceId	(kPluginId, "kImportMetadataSetMetadataFailed",	"Metadata could not be set on object '^1s'")
#define kImportMetadataUndoMissing			MStringResourceId	(kPluginId, "kImportMetadataUndoMissing",		"Undo information not present for importMetadata")
#define kImportMetadataResult				MStringResourceId	(kPluginId, "kImportMetadataResult",			"^1s/^2s/^3s")

#define kExportMetadataFailedFileWrite		MStringResourceId	(kPluginId, "kExportMetadataFailedFileWrite",	"Failed while exporting metadata to file")
#define kExportMetadataFailedStringWrite	MStringResourceId	(kPluginId, "kExportMetadataFailedStringWrite",	"Failed while exporting metadata to string")
#define kExportMetadataFormatType			MStringResourceId	(kPluginId, "kExportMetadataFormatType",		"Format type for ^1s : ^2s")
#define kExportMetadataUndoMissing			MStringResourceId	(kPluginId, "kExportMetadataUndoMissing",		"Undo information not present for exportMetadata")

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

#endif // _metadataPluginStrings_h_
