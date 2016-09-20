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

#include "CacheReaderAlembic.h"
#include "CacheWriterAlembic.h"

#include "gpuCacheShapeNode.h"
#include "gpuCacheDrawOverride.h"
#include "gpuCacheSubSceneOverride.h"
#include "gpuCacheCmd.h"
#include "gpuCacheConfig.h"
#include "gpuCacheUnitBoundingBox.h"
#include "gpuCacheVBOProxy.h"

#include <maya/MFnPlugin.h>
#include <maya/MDrawRegistry.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionMask.h>

#include "gpuCacheStrings.h"

using namespace GPUCache;

// Register all strings
static MStatus registerMStringResources()
{
    MStringResource::registerString(kCreateBakerErrorMsg);
    MStringResource::registerString(kCreateCacheWriterErrorMsg);
    MStringResource::registerString(kEditQueryFlagErrorMsg);
    MStringResource::registerString(kDirectoryWrongModeMsg);
    MStringResource::registerString(kFileNameWrongModeMsg);
    MStringResource::registerString(kSaveMultipleFilesWrongModeMsg);
    MStringResource::registerString(kFilePrefixWrongModeMsg);
    MStringResource::registerString(kClashOptionWrongModeMsg);
    MStringResource::registerString(kOptimizeWrongModeMsg);
    MStringResource::registerString(kOptimizationThresholdWrongModeMsg);
    MStringResource::registerString(kStartTimeWrongModeMsg);
    MStringResource::registerString(kEndTimeWrongModeMsg);
    MStringResource::registerString(kSimulationRateWrongModeMsg);
    MStringResource::registerString(kSimulationRateWrongValueMsg);
    MStringResource::registerString(kSampleMultiplierWrongModeMsg);
    MStringResource::registerString(kSampleMultiplierWrongValueMsg);
    MStringResource::registerString(kCompressLevelWrongModeMsg);
    MStringResource::registerString(kDataFormatWrongModeMsg);
    MStringResource::registerString(kAnimTimeRangeWrongModeMsg);
    MStringResource::registerString(kAllDagObjectsWrongModeMsg);
    MStringResource::registerString(kRefreshWrongModeMsg);
	MStringResource::registerString(kRefreshAllWrongModeMsg);
	MStringResource::registerString(kRefreshAllOtherFlagsMsg);
    MStringResource::registerString(kWaitForBackgroundReadingWrongModeMsg);
    MStringResource::registerString(kWriteMaterialsWrongModeMsg);
    MStringResource::registerString(kWriteUVsWrongModeMsg);
    MStringResource::registerString(kOptimizeAnimationsForMotionBlurWrongModeMsg);
    MStringResource::registerString(kUseBaseTessellationWrongModeMsg);
    MStringResource::registerString(kIncompatibleQueryMsg);
    MStringResource::registerString(kNoObjectsMsg);
    MStringResource::registerString(kCouldNotSaveFileMsg);
    MStringResource::registerString(kFileDoesntExistMsg);
    MStringResource::registerString(kFileFormatWrongMsg);
    MStringResource::registerString(kFailLoadWFShaderErrorMsg);    
    MStringResource::registerString(kCacheOpenFileErrorMsg);
    MStringResource::registerString(kFileNotFindWarningMsg);
    MStringResource::registerString(kReadMeshErrorMsg);
    MStringResource::registerString(kCloseFileErrorMsg);
    MStringResource::registerString(kReadFileErrorMsg);
    MStringResource::registerString(kOpenFileForWriteErrorMsg);
    MStringResource::registerString(kWriteAlembicErrorMsg);
    MStringResource::registerString(kEvaluateMaterialErrorMsg);
    MStringResource::registerString(kHaveBeenBakedErrorMsg);
    MStringResource::registerString(kNodeWontBakeErrorMsg);
    MStringResource::registerString(kNodeBakedFailedErrorMsg);
    MStringResource::registerString(kNoObjBakable1ErrorMsg);
    MStringResource::registerString(kNoObjBakable2ErrorMsg);
    MStringResource::registerString(kNoObjBaked1ErrorMsg);
    MStringResource::registerString(kNoObjBaked2ErrorMsg);
    MStringResource::registerString(kStartEndTimeErrorMsg);
    MStringResource::registerString(kInterruptedMsg);
    MStringResource::registerString(kExportingMsg);
    MStringResource::registerString(kOptimizingMsg);
    MStringResource::registerString(kWritingMsg);
    MStringResource::registerString(kOutlinerMenuItemLabel);
    MStringResource::registerString(kSelectionMenuItemLabel);
    MStringResource::registerString(kDisplayFilterLabel);
    MStringResource::registerString(kBadNormalsMsg);
    MStringResource::registerString(kBadUVsMsg);
    MStringResource::registerString(kBadNurbsMsg);
    MStringResource::registerString(kUnsupportedGeomMsg);
	MStringResource::registerString(kListFileEntriesWrongModeMsg);
	MStringResource::registerString(kListFileEntriesOtherFlagsMsg);
	MStringResource::registerString(kListShapeEntriesWrongModeMsg);
	MStringResource::registerString(kListShapeEntriesOtherFlagsMsg);

    // Stats
    MStringResource::registerString(kStatsAllFramesMsg);
    MStringResource::registerString(kStatsCurrentFrameMsg);
    MStringResource::registerString(kStatsZeroBuffersMsg);
    MStringResource::registerString(kStatsBuffersMsg);
    MStringResource::registerString(kStatsNbGeomMsg);
    MStringResource::registerString(kStatsWiresMsg);
    MStringResource::registerString(kStatsTrianglesMsg);
    MStringResource::registerString(kStatsVerticesMsg);
    MStringResource::registerString(kStatsNormalsMsg);
    MStringResource::registerString(kStatsUVsMsg);
    MStringResource::registerString(kStatsVP2IndexMsg);
    MStringResource::registerString(kStatsVP2VertexMsg);
    MStringResource::registerString(kStatsVBOIndexMsg);
    MStringResource::registerString(kStatsVBOVertexMsg);
    MStringResource::registerString(kStatsTotalInstancedMsg);
    MStringResource::registerString(kStatsSystemTotalMsg);
    MStringResource::registerString(kStatsVideoTotalMsg);
    MStringResource::registerString(kStatsMaterialsMsg);
    MStringResource::registerString(kGlobalSystemStatsMsg);
    MStringResource::registerString(kGlobalSystemStatsIndexMsg);
    MStringResource::registerString(kGlobalSystemStatsVertexMsg);
    MStringResource::registerString(kGlobalVideoStatsMsg);
    MStringResource::registerString(kGlobalVideoStatsIndexMsg);
    MStringResource::registerString(kGlobalVideoStatsVertexMsg);
    MStringResource::registerString(kGlobalRefreshStatsMsg);
    MStringResource::registerString(kGlobalRefreshStatsUploadMsg);
    MStringResource::registerString(kGlobalRefreshStatsEvictionMsg);

    return MStatus::kSuccess;
}


MStatus initializePlugin( MObject obj )
{
    MStatus   status;
    MFnPlugin plugin( obj, PLUGIN_COMPANY, "1.0", "Any");

    const MString& drawDbClassification = 
        (Config::vp2OverrideAPI() == Config::kMPxSubSceneOverride) ? 
            ShapeNode::drawDbClassificationSubScene : 
            ShapeNode::drawDbClassificationGeometry;

    MString UserClassify = MString("cache:");
    UserClassify += drawDbClassification;

    status = plugin.registerShape(
        ShapeNode::nodeTypeName, ShapeNode::id, 
        ShapeNode::creator, ShapeNode::initialize,
        ShapeUI::creator,
        &UserClassify );
    
    if (!status) {
        status.perror("registerNode");
        return status;
    }

    switch (Config::vp2OverrideAPI()) {
    case Config::kMPxDrawOverride:
        status = MHWRender::MDrawRegistry::registerDrawOverrideCreator(
            ShapeNode::drawDbClassificationGeometry,
            ShapeNode::drawRegistrantId,
            DrawOverride::creator);
        if (!status) {
            status.perror("registerGeometryDrawCreator");
            return status;
        }
        break;
    case Config::kMPxSubSceneOverride:
    default:
        status = MHWRender::MDrawRegistry::registerSubSceneOverrideCreator(
            ShapeNode::drawDbClassificationSubScene,
            ShapeNode::drawRegistrantId,
            SubSceneOverride::creator);
        if (!status) {
            status.perror("registerSubSceneOverrideCreator");
            return status;
        }
        break;
    }

    status = plugin.registerUIStrings(registerMStringResources, "gpuCacheInitStrings");
    if (!status)
    {
        status.perror("registerUIStrings");
        return status;
    }

    status = plugin.registerCommand(
        "gpuCache", Command::creator, Command::cmdSyntax );
    if (!status) {
        status.perror("registerCommand");
        return status;
    }

    if (MGlobal::kInteractive == MGlobal::mayaState()) {
        int polyMeshProirity = MSelectionMask::getSelectionTypePriority( "polymesh");
        bool flag = MSelectionMask::registerSelectionType(ShapeNode::selectionMaskName, polyMeshProirity);
        if (!flag) {
            status.perror("registerSelectionType");
            return MS::kFailure;
        }

        MStatus stat;
        MString selectionGeomMenuItemLabel =
            MStringResource::getString(kSelectionMenuItemLabel, stat);

        MString registerMenuItemCMD = "addSelectTypeItem(\"Surface\",\"";
        registerMenuItemCMD += ShapeNode::selectionMaskName;
        registerMenuItemCMD += "\",\"";
        registerMenuItemCMD += selectionGeomMenuItemLabel;
        registerMenuItemCMD += "\")";

        status = MGlobal::executeCommand(registerMenuItemCMD);
        if (!status) {
            status.perror("addSelectTypeItem");
            return status;
        }

        MString outlinerGeomMenuItemLabel = MStringResource::getString(kOutlinerMenuItemLabel, stat);
        MString registerCustomFilterCMD = "addCustomOutlinerFilter(\"gpuCache\",\"CustomGPUCacheFilter\",\"";
        registerCustomFilterCMD += outlinerGeomMenuItemLabel;
        registerCustomFilterCMD += "\",\"DefaultSubdivObjectsFilter\")";

        status = MGlobal::executeCommand(registerCustomFilterCMD);
        if (!status) {
            status.perror("addCustomOutlinerFilter");
            return status;
        }

    }

    // Register plugin display filter.
    // The filter is registered in both interactive and match mode (Hardware 2.0)
    MString displayFilterLabel = MStringResource::getString(kDisplayFilterLabel, status);
    plugin.registerDisplayFilter(Config::kDisplayFilter,
                                 displayFilterLabel,
                                 drawDbClassification);


    CacheWriter::registerWriter("Alembic", &AlembicCacheWriter::create);
    CacheReader::registerReader("Alembic", &AlembicCacheReader::create);
    MGlobal::executeCommand("setNodeTypeFlag -threadSafe true gpuCache");

    MGlobal::executeCommandOnIdle("gpuCacheCreateUI");

    return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj )
{
    MStatus   status;
    MFnPlugin plugin( obj );

    if (MGlobal::kInteractive == MGlobal::mayaState()) {
        MString unregisterCustomFilterCMD = "deleteCustomOutlinerFilter(\"CustomGPUCacheFilter\")";
        status = MGlobal::executeCommand(unregisterCustomFilterCMD);
        if (!status) {
            status.perror("deleteCustomOutlinerFilter");
            return status;
        }

        MString unregisterMenuItemCMD = "deleteSelectTypeItem(\"Surface\",\"";
        unregisterMenuItemCMD += ShapeNode::selectionMaskName;
        unregisterMenuItemCMD += "\")";

        status = MGlobal::executeCommand(unregisterMenuItemCMD);
        if (!status) {
            status.perror("deleteSelectTypeItem");
            return status;
        }

        bool flag = MSelectionMask::deregisterSelectionType(ShapeNode::selectionMaskName);
        if (!flag) {
            status.perror("deregisterSelectionType");
            return MS::kFailure;
        }

    }

    // Deregister plugin display filter
    plugin.deregisterDisplayFilter(Config::kDisplayFilter);

    // Deregister the one we registered in initializePlugin().
    switch (Config::vp2OverrideAPI()) {
    case Config::kMPxDrawOverride:
        status = MHWRender::MDrawRegistry::deregisterDrawOverrideCreator(
            ShapeNode::drawDbClassificationGeometry,
            ShapeNode::drawRegistrantId);
        if (!status) {
            status.perror("deregisterDrawOverrideCreator");
            return status;
        }
        break;
    case Config::kMPxSubSceneOverride:
    default:
        status = MHWRender::MDrawRegistry::deregisterSubSceneOverrideCreator(
            ShapeNode::drawDbClassificationSubScene,
            ShapeNode::drawRegistrantId);
        if (!status) {
            status.perror("deregisterSubSceneOverrideCreator");
            return status;
        }
        break;
    }

    VBOBuffer::clear();
    UnitBoundingBox::clear();

    status = ShapeNode::uninitialize();
    if (!status) {
        status.perror("ShapeNode::uninitialize()");
        return status;
    }

    status = plugin.deregisterNode( ShapeNode::id );
    if (!status) {
        status.perror("deregisterNode");
        return status;
    }

    status = plugin.deregisterCommand( "gpuCache" );
    if (!status) {
        status.perror("deregisterCommand");
        return status;
    }

    MGlobal::executeCommandOnIdle("gpuCacheDeleteUI");

    return MS::kSuccess;
}

