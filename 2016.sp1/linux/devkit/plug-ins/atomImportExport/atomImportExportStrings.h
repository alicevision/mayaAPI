/** Copyright 2012 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license 
agreement provided at the time of installation or download,  or 
which otherwise accompanies this software in either electronic 
or hard copy form.*/ 

//
//	File Name:	atomFileUtils.cpp
//
// DESCRIPTION: -String resources for "atomImportExport" plugin

// MAYA HEADER FILES:

#include <maya/MStringResource.h>
#include <maya/MStringResourceId.h>

// MStringResourceIds contain plugin id, unique resource id for
// each string and the default value for the string. 

#define kPluginId  "atomImportExport"

#define kNothingSelected MStringResourceId(kPluginId, "kNothingSelected", "Nothing is selected for anim curve import.")

#define kPasteFailed MStringResourceId(kPluginId, "kPasteFailed", "Could not paste the anim curves.")

#define kAnimCurveNotFound MStringResourceId(kPluginId, "kAnimCurveNotFound", "No anim curves were found.")

#define kInvalidAngleUnits MStringResourceId(kPluginId, "kInvalidAngleUnits", "'^1s' is not a valid angular unit. Use rad|deg|min|sec instead.")

#define kInvalidLinearUnits MStringResourceId(kPluginId, "kInvalidLinearUnits","'^1s' is not a valid linear unit. Use mm|cm|m|km|in|ft|yd|mi instead.")

#define kInvalidTimeUnits MStringResourceId(kPluginId, "kInvalidTimeUnits", "'^1s' is not a valid time unit. Use game|film|pal|ntsc|show|palf|ntscf|hour|min|sec|millisec instead.")

#define kInvalidVersion MStringResourceId(kPluginId, "kInvalidVersion", "Reading a version ^1s file with a version ^2s atomImportPlugin.")

#define kSettingToUnit MStringResourceId(kPluginId, "kSettingToUnit", "Setting ^1s to ^2s.")

#define kMissingKeyword MStringResourceId(kPluginId, "kMissingKeyword", "Missing a required keyword: ^1s")

#define kCouldNotReadAnim MStringResourceId(kPluginId, "kCouldNotReadAnim", "Could not read an anim curve.")

#define kCouldNotCreateAnim MStringResourceId(kPluginId, "kCouldNotCreateAnim", "Could not create the anim curve node.")

#define kUnknownKeyword MStringResourceId(kPluginId, "kUnknownKeyword", "^1s is an unrecognized keyword.")

#define kClipboardFailure MStringResourceId(kPluginId, "kClipboardFailure","Could not add the anim curves to the clipboard.")

#define kSettingTanAngleUnit MStringResourceId(kPluginId, "kSettingTanAngleUnit","Setting the tanAngleUnit to ^1s.")

#define kUnknownNode MStringResourceId(kPluginId, "kUnknownNode","Encountered an unknown anim curve type.")

#define kCouldNotKey MStringResourceId(kPluginId, "kCouldNotKey","Could not add a keyframe.")

#define kMissingBrace MStringResourceId(kPluginId, "kMissingBrace","Did not find an expected '}'")

#define kCouldNotExport MStringResourceId(kPluginId, "kCouldNotExport", "Could not read the anim curve for export.")

#define kCouldNotReadStatic MStringResourceId(kPluginId, "kCouldNotReadStatic", "Could not apply the static anim value: ^1s.")

#define kCouldNotReadCached MStringResourceId(kPluginId, "kCouldNotReadCached", "Could not apply the cached anim value: ^1s.")

#define kCachingCanceled MStringResourceId(kPluginId, "kCachingCanceled", "Caching process was canceled. ")

#define kSavingCanceled MStringResourceId(kPluginId, "kSavingCanceled", "Saving process was canceled. ")

#define kExportProgress MStringResourceId(kPluginId, "kExportingATOM", "Export")

#define kBakingProgress MStringResourceId(kPluginId, "kBakingATOM", "Baking")




