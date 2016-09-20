#ifndef _gpuCacheString_h_
#define _gpuCacheString_h_

//-
//**************************************************************************/
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk 
// license agreement provided at the time of installation or download, 
// or which otherwise accompanies this software in either electronic 
// or hard copy form.
//**************************************************************************/
//+


////////////////////////////////////////////////////////////////////////////////
//
// File: gpuCacheStrings.h
//
// gpuCache plugin localization strings
//
////////////////////////////////////////////////////////////////////////////////

#include <maya/MStringResource.h>
#include <maya/MStringResourceId.h>
// MStringResourceIds contain plugin id, unique resource id for
// each string and the default value for the string.

#define kPluginId  "gpuCache"

#define kCreateBakerErrorMsg MStringResourceId(kPluginId, "kCreateBakerErrorMsg",\
                 "Couldn't create baker!")

#define kCreateCacheWriterErrorMsg MStringResourceId(kPluginId, "kCreateCacheWriterErrorMsg",\
                 "Couldn't create cacheWriter!")

#define kEditQueryFlagErrorMsg MStringResourceId(kPluginId, "kEditQueryFlagErrorMsg",\
                 "Can't specify -edit and -query flags simultanously.")

#define kDirectoryWrongModeMsg MStringResourceId(kPluginId, "kDirectoryWrongModeMsg",\
                 "The flag -directory can only be used in create mode.")

#define kFileNameWrongModeMsg MStringResourceId(kPluginId, "kFileNameModeMsg",\
                 "The flag -fileName can only be used in create mode.")

#define kSaveMultipleFilesWrongModeMsg MStringResourceId(kPluginId, "kSaveMultipleFilesWrongModeMsg",\
                 "The flag -saveMultipleFiles can only be used in create mode.")

#define kFilePrefixWrongModeMsg MStringResourceId(kPluginId, "kFilePrefixWrongModeMsg",\
                 "The flag -filePrefix can only be used in create mode.")

#define kClashOptionWrongModeMsg MStringResourceId(kPluginId, "kClashOptionWrongModeMsg",\
                 "The flag -clashOption can only be used in create mode.")

#define kOptimizeWrongModeMsg MStringResourceId(kPluginId, "kOptimizeWrongModeMsg",\
                 "The flag -optimize can only be used in create mode.")

#define kOptimizationThresholdWrongModeMsg MStringResourceId(kPluginId, "kOptimizationThresholdWrongModeMsg",\
                 "The flag -optimizationThreshold can only be used in create mode.")

#define kStartTimeWrongModeMsg MStringResourceId(kPluginId, "kStartTimeWrongModeMsg", \
                 "The flag -startTime can only be used in create mode.")

#define kEndTimeWrongModeMsg MStringResourceId(kPluginId, "kEndTimeWrongModeMsg",\
                 "The flag -endTime can only be used in create mode.")

#define kSimulationRateWrongModeMsg MStringResourceId(kPluginId, "kSimulationRateWrongModeMsg",\
                 "The flag -simulationRate can only be used in create mode.")

#define kSimulationRateWrongValueMsg MStringResourceId(kPluginId, "kSimulationRateWrongValueMsg",\
                 "The simulationRate value is invalid. It must be at least ^1s frame.")

#define kSampleMultiplierWrongModeMsg MStringResourceId(kPluginId, "kSampleMultiplierWrongModeMsg",\
                 "The flag -sampleMultiplier can only be used in create mode.")

#define kSampleMultiplierWrongValueMsg MStringResourceId(kPluginId, "kSampleMultiplierWrongValueMsg",\
                 "The sampleMultiplier value is invalid. It must be at least 1.")

#define kCompressLevelWrongModeMsg MStringResourceId(kPluginId, "kCompressLevelWrongModeMsg",\
                 "The flag -compressLevel can only be used in create mode.")

#define kDataFormatWrongModeMsg MStringResourceId(kPluginId, "kDataFormatWrongModeMsg",\
                 "The flag -dataFormat can only be used in create mode.")

#define kAnimTimeRangeWrongModeMsg MStringResourceId(kPluginId, "kAnimTimeRangeWrongModeMsg",\
                 "The flag -animTimeRange can only be used in query mode.")

#define kGpuManufacturerWrongModeMsg MStringResourceId(kPluginId, "kGpuManufacturerWrongModeMsg",\
                 "The flag -gpuManufacturer can only be used in query mode.")

#define kGpuModelWrongModeMsg MStringResourceId(kPluginId, "kGpuModelWrongModeMsg",\
                 "The flag -gpuModel can only be used in query mode.")

#define kGpuDriverVersionWrongModeMsg MStringResourceId(kPluginId, "kGpuDriverVersionWrongModeMsg",\
                 "The flag -gpuDriverVersion can only be used in query mode.")

#define kGpuMemorySizeWrongModeMsg MStringResourceId(kPluginId, "kGpuMemorySizeWrongModeMsg",\
                 "The flag -gpuMemorySize can only be used in query mode.")

#define kAllDagObjectsWrongModeMsg MStringResourceId(kPluginId, "kAllDagObjectsWrongModeMsg",\
                 "The flag -allDagObjects can only be used in create mode.")

#define kRefreshWrongModeMsg MStringResourceId(kPluginId, "kRefreshWrongModeMsg",\
                 "The flag -refresh can only be used in edit mode.")

#define kRefreshAllWrongModeMsg MStringResourceId(kPluginId, "kRefreshAllWrongModeMsg",\
                 "The flag -refreshAll can only be used in create mode.")

#define kRefreshAllOtherFlagsMsg MStringResourceId(kPluginId, "kRefreshAllOtherFlagsMsg",\
                 "The flag -refreshAll cannot be used with other flags.")

#define kRefreshSettingsWrongModeMsg MStringResourceId(kPluginId, "kRefreshSettingsWrongModeMsg",\
                 "The flag -refreshSettings can only be used in edit mode.")

#define kWaitForBackgroundReadingWrongModeMsg MStringResourceId(kPluginId, "kWaitForBackgroundReadingWrongModeMsg",\
                 "The flag -waitForBackgroundReading can only be used in query mode.")

#define kWriteMaterialsWrongModeMsg MStringResourceId(kPluginId, "kWriteMaterialsWrongModeMsg",\
                 "The flag -writeMaterials can only be used in create mode.")

#define kWriteUVsWrongModeMsg MStringResourceId(kPluginId, "kWriteUVsWrongModeMsg",\
                 "The flag -writeUVs can only be used in create mode.")

#define kOptimizeAnimationsForMotionBlurWrongModeMsg MStringResourceId(kPluginId, "kOptimizeAnimationsForMotionBlurWrongModeMsg",\
                 "The flag -optimizeAnimationsForMotionBlur can only be used in create mode.")

#define kUseBaseTessellationWrongModeMsg MStringResourceId(kPluginId, "kUseBaseTessellationWrongModeMsg",\
                 "The flag -useBaseTessellation can only be used in create mode.")

#define kIncompatibleQueryMsg MStringResourceId(kPluginId, "kIncompatibleQueryMsg",\
                 "The set of query flags are incompatible.")

#define kNoObjectsMsg MStringResourceId(kPluginId, "kNoObjectsMsg",\
                 "This command requires at least 1 object(s).")

#define kCouldNotSaveFileMsg MStringResourceId(kPluginId, "kCouldNotSaveFileMsg",\
                 "Could not save file ^1s.")

#define kFileDoesntExistMsg MStringResourceId(kPluginId, "kFileDoesntExistMsg",\
                 "File does not exist: ^1s.")

#define kFileFormatWrongMsg MStringResourceId(kPluginId, "kFileFormatWrongMsg",\
                 "Unrecognized file format or broken file: ^1s.")

#define kFailLoadWFShaderErrorMsg MStringResourceId(kPluginId, "kFailLoadWFShaderErrorMsg",\
                 "Failed to load wireframe shader: ^1s^2s")

#define kCacheOpenFileErrorMsg MStringResourceId(kPluginId, "kCacheOpenFileErrorMsg",\
                 "Error occurred when opening file for reading: ^1s. Reason: ^2s")

#define kFileNotFindWarningMsg MStringResourceId(kPluginId, "kFileNotFindWarningMsg",\
                 "\"^1s\" is not found in ^2s. Use \"^3s\" instead.")

#define kReadMeshErrorMsg MStringResourceId(kPluginId, "kReadMeshErrorMsg",\
                 "Error occurred when reading mesh: ^1s, ^2s. Reason: ^3s")

#define kCloseFileErrorMsg MStringResourceId(kPluginId, "kCloseFileErrorMsg",\
                 "Error occurred when closing file: ^1s. Reason: ^2s")

#define kReadFileErrorMsg MStringResourceId(kPluginId, "kReadFileErrorMsg",\
                 "Error occurred when reading file: ^1s. Reason: ^2s")

#define kOpenFileForWriteErrorMsg MStringResourceId(kPluginId, "kOpenFileForWriteErrorMsg",\
                 "Error occurred when opening file for writing: ^1s. Reason: ^2s")

#define kWriteAlembicErrorMsg MStringResourceId(kPluginId, "kWriteAlembicErrorMsg",\
                "Error occurred when writing Alembic file: ^1s. Reason: ^2s")

#define kEvaluateMaterialErrorMsg MStringResourceId(kPluginId, "kEvaluateMaterialErrorMsg",\
                "Couldn't evaluate material\n")

#define kHaveBeenBakedErrorMsg MStringResourceId(kPluginId, "kHaveBeenBakedErrorMsg",\
                "^1s has already been baked.")

#define kNodeWontBakeErrorMsg MStringResourceId(kPluginId, "kNodeWontBakeErrorMsg",\
                "^1s won't be baked, the command was invoked in query mode")

#define kNodeBakedFailedErrorMsg MStringResourceId(kPluginId, "kNodeBakedFailedErrorMsg",\
                "^1s can't be baked.")

#define kNoObjBakable1ErrorMsg MStringResourceId(kPluginId, "kNoObjBakable1ErrorMsg",\
                "No objects in the selection can be baked. At least one of them has already been baked.")

#define kNoObjBakable2ErrorMsg MStringResourceId(kPluginId, "kNoObjBakable2ErrorMsg",\
                "No objects in the selection can be baked.")

#define kNoObjBaked1ErrorMsg MStringResourceId(kPluginId, "kNoObjBaked1ErrorMsg",\
                "No gpuCache objects found in the selection. "\
                "At least one of them can be baked but the command was invoked in query mode.")

#define kNoObjBaked2ErrorMsg MStringResourceId(kPluginId, "kNoObjBaked2ErrorMsg",\
                "No gpuCache objects found in the selection.")

#define kStartEndTimeErrorMsg MStringResourceId(kPluginId, "kStartEndTimeErrorMsg",\
                "Start time must be less than or equal to end time.")

#define kInterruptedMsg MStringResourceId(kPluginId, "kInterruptedMsg",\
                "Interrupted by user.")

#define kExportingMsg MStringResourceId(kPluginId, "kExportingMsg",\
                "Exporting...")

#define kOptimizingMsg MStringResourceId(kPluginId, "kOptimizingMsg",\
                "Optimizing...")

#define kWritingMsg MStringResourceId(kPluginId, "kWritingMsg",\
                "Writing...")

#define kOutlinerMenuItemLabel MStringResourceId(       \
    kPluginId, "kOutlinerMenuItemLabel",            \
    "GPU Cache")

#define kSelectionMenuItemLabel MStringResourceId(    \
                kPluginId, "kSelectionMenuItemLabel", \
                "GPU Cache")

#define kDisplayFilterLabel MStringResourceId(kPluginId, "kDisplayFilterLabel", "GPU Cache")

#define kBadNormalsMsg MStringResourceId(kPluginId, "kBadNormalsMsg",\
                "Bad normals. The number of normals does not match its scope.")

#define kBadUVsMsg MStringResourceId(kPluginId, "kBadUVsMsg",\
                "Bad UVs. The number of UVs does not match its scope.")

#define kBadNurbsMsg MStringResourceId(kPluginId, "kBadNurbsMsg",\
                "Bad NURBS surface. The NURBS surface sample is ignored.")

#define kUnsupportedGeomMsg MStringResourceId(kPluginId, "kUnsupportedGeomMsg",\
                "The specified geometry is not supported. Ignoring ^1s.")

#define kListFileEntriesWrongModeMsg MStringResourceId(kPluginId, "kListFileEntriesWrongModeMsg",\
                 "The flag -listFileEntries can only be used in create mode.")

#define kListFileEntriesOtherFlagsMsg MStringResourceId(kPluginId, "kListFileEntriesOtherFlagsMsg",\
                 "The flag -listFileEntries cannot be used with other flags.")

#define kListShapeEntriesWrongModeMsg MStringResourceId(kPluginId, "kListShapeEntriesWrongModeMsg",\
                 "The flag -listShapeEntries can only be used in create mode.")

#define kListShapeEntriesOtherFlagsMsg MStringResourceId(kPluginId, "kListShapeEntriesOtherFlagsMsg",\
                 "The flag -listShapeEntries cannot be used with other flags.")

/*==============================================================================
 * Statistics
 *============================================================================*/

#define kStatsAllFramesMsg MStringResourceId(    \
        kPluginId, "kStatsAllFramesMsg",         \
        "Statistics for all frames:")

#define kStatsCurrentFrameMsg MStringResourceId(    \
        kPluginId, "kStatsCurrentFrameMsg",         \
        "Statistics for the current frame:")

#define kStatsZeroBuffersMsg MStringResourceId(    \
        kPluginId, "kStatsZeroBuffersMsg",         \
        "    ^1s:   buffers: 0")

#define kStatsBuffersMsg MStringResourceId(    \
        kPluginId, "kStatsBuffersMsg",         \
        "    ^1s:   buffers: ^2s, average: ^3s, min: ^4s, max: ^5s, total: ^6s (^7s ^8s)")

#define kStatsNbGeomMsg MStringResourceId(    \
        kPluginId, "kStatsNbGeomMsg",         \
        "  Nb of gpuCache nodes = ^1s, nb of internal sub nodes: ^2s")

#define kStatsWiresMsg     MStringResourceId(kPluginId, "kStatsWiresMsg",     "Wires     ")
#define kStatsTrianglesMsg MStringResourceId(kPluginId, "kStatsTrianglesMsg", "Triangles ")
#define kStatsVerticesMsg  MStringResourceId(kPluginId, "kStatsVerticesMsg",  "Vertices  ")
#define kStatsNormalsMsg   MStringResourceId(kPluginId, "kStatsNormalsMsg",   "Normals   ")
#define kStatsUVsMsg       MStringResourceId(kPluginId, "kStatsUVsMsg",       "UVs       ")
#define kStatsVP2IndexMsg  MStringResourceId(kPluginId, "kStatsVP2IndexMsg",  "VP2 Index ")
#define kStatsVP2VertexMsg MStringResourceId(kPluginId, "kStatsVP2VertexMsg", "VP2 Vertex")
#define kStatsVBOIndexMsg  MStringResourceId(kPluginId, "kStatsVBOIndexMsg",  "VBO Index ")
#define kStatsVBOVertexMsg MStringResourceId(kPluginId, "kStatsVBOVertexMsg", "VBO Vertex")

#define kStatsTotalInstancedMsg MStringResourceId(    \
        kPluginId, "kStatsTotalInstancedMsg",         \
        "    Total instanced: ^1s wires, ^2s triangles")

#define kStatsSystemTotalMsg MStringResourceId(       \
        kPluginId, "kStatsSystemTotalMsg",            \
        "  Using a total of ^1s ^2s of system memory")

#define kStatsVideoTotalMsg MStringResourceId(       \
        kPluginId, "kStatsVideoTotalMsg",            \
        "  Using a total of ^1s ^2s of video memory\n")

#define kStatsMaterialsMsg MStringResourceId(       \
        kPluginId, "kStatsMaterialsMsg",            \
        "  Materials: ^1s graphs, ^2s nodes\n")

#define kGlobalSystemStatsMsg MStringResourceId(                        \
        kPluginId, "kGlobalSystemStatsMsg",                             \
        "Total of system memory buffers allocated by gpuCache nodes: ^1s buffers (^2s ^3s)")
#define kGlobalSystemStatsIndexMsg MStringResourceId(   \
        kPluginId, "kGlobalSystemStatsIndexMsg",        \
        "  ^1s index buffers (^2s ^3s)")
#define kGlobalSystemStatsVertexMsg MStringResourceId(   \
        kPluginId, "kGlobalSystemStatsVertexMsg",       \
        "  ^1s vertex buffers (^2s ^3s)")

#define kGlobalVideoStatsMsg MStringResourceId(                        \
        kPluginId, "kGlobalVideoStatsMsg",                             \
        "Total of video memory buffers allocated by gpuCache nodes: ^1s buffers (^2s ^3s)")
#define kGlobalVideoStatsIndexMsg MStringResourceId(   \
        kPluginId, "kGlobalVideoStatsIndexMsg",        \
        "  ^1s VBO index buffers (^2s ^3s)")
#define kGlobalVideoStatsVertexMsg MStringResourceId(   \
        kPluginId, "kGlobalVideoStatsVertexMsg",       \
        "  ^1s VBO vertex buffers (^2s ^3s)")

#define kGlobalRefreshStatsMsg MStringResourceId(                        \
        kPluginId, "kGlobalRefreshStatsMsg",                             \
        "Video memory buffers operations since the plug-in was loaded:")
#define kGlobalRefreshStatsUploadMsg MStringResourceId(   \
        kPluginId, "kGlobalRefreshStatsUploadMsg",        \
        "  ^1s VBO buffers uploaded (^2s ^3s)")
#define kGlobalRefreshStatsEvictionMsg MStringResourceId(   \
        kPluginId, "kGlobalRefreshStatsEvictionMsg",       \
        "  ^1s VBO buffers evicted (^2s ^3s)")

#endif


