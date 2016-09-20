#ifndef _sceneAssemblyStrings_h_
#define _sceneAssemblyStrings_h_

////////////////////////////////////////////////////////////////////////////////
//
// sceneAssembly plugin localization strings
//
////////////////////////////////////////////////////////////////////////////////

#include <maya/MStringResource.h>
#include <maya/MStringResourceId.h>
// MStringResourceIds contain plug-in id, unique resource id for
// each string and the default value for the string.

#define kPluginId				"sceneAssembly"

// If a MStringResourceId is added to this list, please register in registerMStringRes() 
// in sceneAssemblyPluginMain.cpp

#define rRegisterUIStringError		      MStringResourceId( kPluginId, "rRegisterUIStringError",				"Failed to register ^1s.")
#define rRegisterNodeError			      MStringResourceId( kPluginId, "rRegisterNodeError",					"Failed to register ^1s node: ^2s.")
#define rDeregisterNodeError		      MStringResourceId( kPluginId, "rDeregisterNodeError",					"Failed to deregister ^1s node: ^2s.")
#define rRegisterAssembliesError          MStringResourceId( kPluginId, "rRegisterAssembliesError",     		"Failed to register assemblies: ^1s.")
#define rRegisterRepresentationsError     MStringResourceId( kPluginId, "rRegisterRepresentationsError",     	"Failed to register representations: ^1s.")
#define rRegisterCmdError			      MStringResourceId( kPluginId, "rRegisterCmdError",					"Failed to register ^1s command: ^2s.")
#define rAssemblyDefnImportError          MStringResourceId( kPluginId, "rAssemblyDefnImportError",     		"Failed to import assembly definition file ^1s into assembly reference ^2s.")
#define rAssemblyDefnNotFoundError        MStringResourceId( kPluginId, "rAssemblyDefnNotFoundError",     		"Assembly definition node not found in file ^1s for assembly reference ^2s.")
#define rMultAssemblyDefnFoundError       MStringResourceId( kPluginId, "rMultAssemblyDefnFoundError",     		"Multiple assembly definition nodes found in file ^1s for assembly reference ^2s.")
#define rRegisterRepFactoryError	      MStringResourceId( kPluginId, "rRegisterRepFactoryError",				"Failed to register factory for representation type ^1s.")
#define rDeregisterRepFactoryError	      MStringResourceId( kPluginId, "rDeregisterRepFactoryError",			"Failed to deregister factory for representation type ^1s.")
#define rCreateGPUCacheNodeError	      MStringResourceId( kPluginId, "rCreateGPUCacheNodeError",				"Failed to create gpuCache node, the gpuCache plugin is not loaded.")
#define	rEditQueryError				      MStringResourceId( kPluginId, "rEditQueryError",						"Can not specify -edit and -query flags simultaneously.")
#define	rRepTypeObjArgError			      MStringResourceId( kPluginId, "rRepTypeObjArgError",					"Missing the object string as a representation type.")
#define	rListRepTypesFlagError            MStringResourceId( kPluginId, "rListRepTypesFlagError",		        "The flag -listRepTypes can only be used in query mode.")
#define	rRegisterFilePathEditorError      MStringResourceId( kPluginId, "rRegisterFilePathEditorError",		    "Failed to register ^1s to FilePathEditor.")
#define	rDeregisterFilePathEditorError    MStringResourceId( kPluginId, "rDeregisterFilePathEditorError",		"Failed to deregister ^1s from FilePathEditor.")
#define	rChannelNameFlagError             MStringResourceId( kPluginId, "rChannelNameFlagError",                "The -channelName flag was not set.")
#define	rAccessorNotFoundError            MStringResourceId( kPluginId, "rAccessorNotFoundError",               "Can not find accessor for file ^1s.")
#define	rCannotReadFileError              MStringResourceId( kPluginId, "rCannotReadFileError",                 "Can not read file ^1s (errors = ^2s).")
#define	rMissingStreamInChannelError      MStringResourceId( kPluginId, "rMissingStreamInChannelError",         "Can not find a stream for channel ^1s.")
#define	rMissingElementInStreamError      MStringResourceId( kPluginId, "rMissingElementInStreamError",         "Can not find an element in the stream for channel ^1s.")
#define	rMissingMemberInElementError      MStringResourceId( kPluginId, "rMissingMemberInElementError",         "Can not find data member ^1s in the stream for channel ^2s.")
#define	rInvalidMemberDataTypeError       MStringResourceId( kPluginId, "rInvalidMemberDataTypeError",          "Invalid data type for member in stream for channel ^1s. Expected a string type.")
#define	rSetDataOnChannelError 			  MStringResourceId( kPluginId, "rSetDataOnChannelError",               "Error while setting the data in channel ^1s (^2s).")
#define	rWriteMetadataError               MStringResourceId( kPluginId, "rWriteMetadataError",                  "Error while writing metadata.")
#define	rDataFlagError                    MStringResourceId( kPluginId, "rDataFlagError",		                "The flag -data can only be used in edit mode.")

#endif

//-
//*****************************************************************************
// Copyright 2015 Autodesk, Inc.  All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy
// form.
//*****************************************************************************
//+
