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

#include "gpuCacheConfig.h"
#include "gpuCacheVramQuery.h"

#include <maya/MString.h>
#include <maya/MGlobal.h>

#include <stdio.h>
#include <limits>


// On Windows, the max macro conflicts with
// std::numeric_limits<T>::max().
#ifdef max
#undef max
#endif

//==============================================================================
// LOCAL FUNCTIONS
//==============================================================================

namespace {

using namespace GPUCache;

//------------------------------------------------------------------------------
//
bool expandEnv(MString& expandedEnv, const MString& env)
{
    const MString envQuery = MString("$") + env;
    expandedEnv = envQuery.expandEnvironmentVariablesAndTilde();
    return expandedEnv != envQuery;
}


//------------------------------------------------------------------------------
//
size_t getVP2OverrideAPIDefault()
{
    // Default value
    MString vp2OverrideEnv;
    if (expandEnv(vp2OverrideEnv, "MAYA_GPUCACHE_VP2_OVERRIDE_API")) {
        if (vp2OverrideEnv == "MPxDrawOverride") {
            return Config::kMPxDrawOverride;
        }
        else if (vp2OverrideEnv == "MPxSubSceneOverride") {
            return Config::kMPxSubSceneOverride;
        }
        else {
            printf("MAYA_GPUCACHE_VP2_OVERRIDE_API is set but it is neither "
                   "MPxDrawOverride nor MPxSubSceneOverride. "
                   "Using MPxSubSceneOverride instead.\n");
            return Config::kMPxSubSceneOverride;
        }
    }
    
    return Config::kMPxSubSceneOverride;
}


//------------------------------------------------------------------------------
//
bool getIgnoreUVsDefault()
{
    // Default value
    bool result = 1;
    return result;
}


//------------------------------------------------------------------------------
//
size_t getMinVertsForVBOsDefault()
{
    // Default value
    //
    // FIXME: No serious tuning regarding the optimal value of this
    // value has been performed upto now!
    size_t result = 128;
    return result;
}


//------------------------------------------------------------------------------
//
size_t getMaxVBOCountDefault()
{
    // Default value
    //
    // FIXME: No serious tuning regarding the optimal value of this
    // value has been performed up to now!
#ifdef OSMac_
    // The GPU memory manager on Mac seems to become completely
    // overloaded when we allocate to many buffers.
    const size_t defVal = 8192;
#else    
    const size_t defVal = std::numeric_limits<int>::max();
#endif

    size_t result = defVal;
    return result;
}


//------------------------------------------------------------------------------
//
size_t getMaxVBOSizeDefault()
{
    size_t result;
    
    // Detect the dedicated VRAM and use the following heuristic
    // for sizing the VBO cache.
    //
    //   VRAM   Used for    Available for
    //   (MB)   gpuCache's  other uses (MB)
    //   (MB)   VBOs (MB)
    //      0        0           0
    //    128        0         128
    //    512      256         256
    //   1024      640         384
    //   2048     1536         512
    //   3072     2560         512
    
    const size_t vramMB = VramQuery::queryVram() / 1024 / 1024;

    float resultMB = 0.0f;
    
    if (vramMB < 128) {
        resultMB = 0.f;
    }
    else if (vramMB < 512) {
        resultMB = (vramMB -  128) * (( 256.f -   0.f) / ( 512.f -  128.f)) +   0.f;
    }
    else if (vramMB < 1024) {
        resultMB = (vramMB -  512) * (( 640.f - 256.f) / (1024.f -  512.f)) + 256.f;
    }
    else if (vramMB < 2048) {
        resultMB = (vramMB - 1024) * ((1536.f - 640.f) / (2048.f - 1024.f)) + 640.f;
    }
    else {
        resultMB = vramMB - 512;
    }

    result = size_t(resultMB * 1024 * 1024);
    return result;
}


//------------------------------------------------------------------------------
//
bool getUseVertexArrayWhenVRAMIsLowDefault()
{
    bool result;
    
#if defined(_WIN32)
    // On Windows, using a temporary VBO is 3 times faster than
    // using vertex arrays. (Tested with an NVidia Quadro gfx).
    result = false;
#elif defined(LINUX)
    // On Linux, using vertex arrays is 2 times faster than using
    // a temporary VBO. (Tested with an NVidia Quadro gfx).
    //
    // Unfortunately, the NVidia driver seems to have a bug where
    // drawing using vertex arrays causes memory corruption. So,
    // this can't be used reliably on Quadro cards...
    //
    // (BTW, this has never been tested on a Linux machine with an
    // NVidia GeForce or an ATI graphic card so, using temporary
    // VBOs might not necessarily be the best option on these
    // platforms!)
    result = !VramQuery::isQuadro();
#else        
    // On MacOS, using vertex arrays is 3 times faster than using
    // a temporary VBO. (Tested with an AMD Radeon HD 6770M).
    result = true;
#endif

    return result;
}


//------------------------------------------------------------------------------
//
bool getUseVertexArrayForGLPickingDefault()
{
    bool result;
    
#ifdef OSMac_
    // Do not use VBO in conjunction with GL picking on Mac. When
    // profiling on Mac OS X 10.7.2 / NVidia GT330M, we have found
    // out that using VBO is 20X (i.e. 2000%) slower than simply
    // using Vertex Arrays....
    result = true;
#else
    result = false;
#endif

    return result;
}


//------------------------------------------------------------------------------
//
bool getUseGLPrimitivesInsteadOfVADefault()
{
    bool result = false;
    
#if defined(_WIN32)
    if (VramQuery::isQuadro() ) {
        // For some reason, using vertex arrays on Windows/nVidia
        // Quadro gfx leads to memory corruption. Using primitive
        // OpenGL calls instead as a workaround.
        // nVidia has fixed the memory corruption bug in 295.65
        int driverVersion[3] = {0};
        VramQuery::driverVersion(driverVersion);
        if (driverVersion[0] < 295 || 
                (driverVersion[0] == 295 && driverVersion[1] < 65)) {
            result = true;
        }
    }
#endif

    return result;
}


//------------------------------------------------------------------------------
//
bool getEmulateTwoSidedLightingDefault()
{
    bool result;
    
    // check Geforce graphics cards on windows
#ifdef _WIN32
    result = VramQuery::isGeforce();
#else
    result = false;
#endif

    return result;
}


//==============================================================================
// SELECTION METHODS EVs
//==============================================================================

// The environment variables listed below are used to control the
// method used to perform a given selection.
//
// Below the given threshold value, we use OpenGL picking. Above this
// value, we use either raster-based picking or CPU-based picking,
// because these methods are faster for large objects. The threshold
// value is respectively expressed in terms number of vertices, edges
// or triangles per object. There are different threshold value for
// kSurfaceSelectMethod and kWireframeSelectMethod.  A negative value
// means to always use OpenGL picking. A zero value means to never use
// OpenGL picking.

//------------------------------------------------------------------------------
//
size_t getOpenGLPickingWireframeThresholdDefault()
{
#ifdef OSMac_
    // On Mac, OpenGL picking seems to be hardware accelerated since
    // it is always faster than raster-based picking.
    const int defVal = std::numeric_limits<int>::max();
#else    
    const int defVal = 128;
#endif

    return defVal;
}


//------------------------------------------------------------------------------
//
size_t getOpenGLPickingSurfaceThresholdDefault()
{
#ifdef OSMac_
    // On Mac, OpenGL picking seems to be hardware accelerated since
    // it is always faster than raster-based picking.
    const int defVal = std::numeric_limits<int>::max();
#else    
    const int defVal = 1024;
#endif

    return defVal;
}


//------------------------------------------------------------------------------
//
bool getBackgroundReadingDefault()
{
    return true;
}


//------------------------------------------------------------------------------
//
size_t getBackgroundReadingRefreshDefault()
{
    return 1000;
}


//------------------------------------------------------------------------------
//
bool getUseHardwareInstancingDefault()
{
    return true;
}


//------------------------------------------------------------------------------
//
size_t getHardwareInstancingThresholdDefault()
{
    return 2;
}


}

namespace GPUCache {

//==============================================================================
// CLASS Config
//==============================================================================

const MColor Config::kDefaultGrayColor = MColor(0.5f, 0.5f, 0.5f) * 0.8f;
const MColor Config::kDefaultTransparency = MColor(0.0f, 0.0f, 0.0f);
const unsigned short Config::kLineStippleShortDashed = 0x0303;
const unsigned short Config::kLineStippleDotted = 0x0101;
const MString Config::kDisplayFilter = "gpuCacheDisplayFilter";

bool   Config::sInitialized = false;

size_t Config::sDefaultMaxVBOSize;
size_t Config::sDefaultMaxVBOCount;
size_t Config::sDefaultMinVertsForVBOs;
bool   Config::sDefaultUseVertexArrayWhenVRAMIsLow;
bool   Config::sDefaultUseVertexArrayForGLPicking;
size_t Config::sDefaultOpenGLPickingWireframeThreshold;
size_t Config::sDefaultOpenGLPickingSurfaceThreshold;
bool   Config::sDefaultUseGLPrimitivesInsteadOfVA;
bool   Config::sDefaultEmulateTwoSidedLighting;
bool   Config::sDefaultIsIgnoringUVs;
size_t Config::sDefaultVP2OverrideAPI;
bool   Config::sDefaultBackgroundReading;
size_t Config::sDefaultBackgroundReadingRefresh;
bool   Config::sDefaultUseHardwareInstancing;
size_t Config::sDefaultHardwareInstancingThreshold;

size_t Config::sMaxVBOSize;
size_t Config::sMaxVBOCount;
size_t Config::sMinVertsForVBOs;
bool   Config::sUseVertexArrayWhenVRAMIsLow;
bool   Config::sUseVertexArrayForGLPicking;
size_t Config::sOpenGLPickingWireframeThreshold;
size_t Config::sOpenGLPickingSurfaceThreshold;
bool   Config::sUseGLPrimitivesInsteadOfVA;
bool   Config::sEmulateTwoSidedLighting;
bool   Config::sIsIgnoringUVs;
size_t Config::sVP2OverrideAPI;
bool   Config::sBackgroundReading;
size_t Config::sBackgroundReadingRefresh;
bool   Config::sUseHardwareInstancing;
size_t Config::sHardwareInstancingThreshold;

void syncIntOptionVar(bool automatic, const char * autoOptVar, const char * valueOptVar, size_t defaultValue, size_t& dest, int multiplier=1)
{
    bool existAuto = false;
    int autoValue = MGlobal::optionVarIntValue(autoOptVar, &existAuto);
    if ( !automatic && existAuto && autoValue == 0 ) {
        bool exist = false;
        int value = MGlobal::optionVarIntValue(valueOptVar, &exist);
        if (exist) {
            dest = value * multiplier;
        }
        else {
            dest = defaultValue;
            MGlobal::setOptionVarValue(valueOptVar, static_cast<int>(defaultValue/multiplier));
        }
    }
    else {
        dest = defaultValue;
        MGlobal::setOptionVarValue(autoOptVar, 1);
        MGlobal::setOptionVarValue(valueOptVar, static_cast<int>(defaultValue/multiplier));
    }
}

void syncBoolOptionVar(bool automatic, const char * autoOptVar, const char * valueOptVar, bool defaultValue, bool& dest, bool valueToCompare)
{
    bool existAuto = false;
    int autoValue = MGlobal::optionVarIntValue(autoOptVar, &existAuto);
    if ( !automatic && existAuto && autoValue == 0 ){
        bool exist = false;
        int value = MGlobal::optionVarIntValue(valueOptVar, &exist);
        if (exist) {
            dest = (static_cast<bool>(value) == valueToCompare);
        }
        else {
            dest = defaultValue;
            MGlobal::setOptionVarValue(valueOptVar, defaultValue ? 1 : 0);
        }
    }
    else {
        dest = defaultValue;
        MGlobal::setOptionVarValue(autoOptVar, 1);
        MGlobal::setOptionVarValue(valueOptVar, defaultValue ? 1 : 0);
    }
}

void syncBoolOptionVar(bool automatic, const char * autoOptVar, const char * valueOptVar, bool defaultValue, bool& dest, int valueToCompare)
{
    bool existAuto = false;
    int autoValue = MGlobal::optionVarIntValue(autoOptVar, &existAuto);

    // convert defaultInt to radiobox enum.
    int defaultInt = (valueToCompare==1 ? (defaultValue ? 1 : 2) : (defaultValue ? 2 : 1) );

    if ( !automatic && existAuto && autoValue == 0 ){
        bool exist = false;
        int value = MGlobal::optionVarIntValue(valueOptVar, &exist);
        if (exist) {
            dest = (value == valueToCompare);
        }
        else {
            dest = defaultValue;
            MGlobal::setOptionVarValue(valueOptVar, defaultInt);
        }
    }
    else {
        dest = defaultValue;
        MGlobal::setOptionVarValue(autoOptVar, 1);
        MGlobal::setOptionVarValue(valueOptVar, defaultInt);
    }
}

Config::VP2OverrideAPI Config::vp2OverrideAPI()
{
    // Once initialized, we save the API choice to this local static variable.
    // The return value of vp2OverrideAPI() should be the same regardless of 
    // the user preference until the plug-in is unloaded.
    static bool initialized = false;
    static VP2OverrideAPI sCurrentVP2OverrideAPI = kMPxSubSceneOverride;

    // This must be initialized separately with other config variables
    // because vp2OverrideAPI() is called on plugin load.
    if (!initialized) {
        // Initialize the current and default values
        sVP2OverrideAPI = sDefaultVP2OverrideAPI = getVP2OverrideAPIDefault();

        // If there is no pref or 'automatic' is chosen
        bool existAllAuto = false;
        int allAutoValue = MGlobal::optionVarIntValue("gpuCacheAllAuto", &existAllAuto);
        bool automatic = !existAllAuto || allAutoValue == 1;

        // Sync with option var (read user pref)
        syncIntOptionVar(automatic, "gpuCacheVP2OverrideAPIAuto", "gpuCacheVP2OverrideAPI", sDefaultVP2OverrideAPI, sVP2OverrideAPI);
        sCurrentVP2OverrideAPI = (Config::VP2OverrideAPI)sVP2OverrideAPI;
        initialized = true;
    }

    return sCurrentVP2OverrideAPI;
}

bool Config::isIgnoringUVs()
{
    initialize();
    return sIsIgnoringUVs;
}

size_t Config::minVertsForVBOs()
{
    initialize();
    return sMinVertsForVBOs;
}

size_t Config::maxVBOCount()
{
    initialize();
    return sMaxVBOCount;
}

size_t Config::maxVBOSize()
{
    initialize();
    return sMaxVBOSize;
}

bool Config::useVertexArrayWhenVRAMIsLow()
{
    initialize();
    return sUseVertexArrayWhenVRAMIsLow;
}

bool Config::useVertexArrayForGLPicking()
{
    initialize();
    return sUseVertexArrayForGLPicking;
}

bool Config::useGLPrimitivesInsteadOfVA()
{
    initialize();
    return sUseGLPrimitivesInsteadOfVA;
}

bool Config::emulateTwoSidedLighting()
{
    initialize();
    return sEmulateTwoSidedLighting;
}

size_t Config::openGLPickingWireframeThreshold()
{
    initialize();
    return sOpenGLPickingWireframeThreshold;
}

size_t Config::openGLPickingSurfaceThreshold()
{
    initialize();
    return sOpenGLPickingSurfaceThreshold;
}

bool Config::backgroundReading()
{
    initialize();
    return sBackgroundReading;
}

size_t Config::backgroundReadingRefresh()
{
    initialize();
    return sBackgroundReadingRefresh;
}

bool Config::useHardwareInstancing()
{
    initialize();
    return sUseHardwareInstancing;
}

size_t Config::hardwareInstancingThreshold()
{
    initialize();
    return sHardwareInstancingThreshold;
}

void Config::refresh()
{
    if (!sInitialized) {
        initialize();
        return;  // refresh() is called in initialize()
    }

    bool existAllAuto = false;
    int allAutoValue = MGlobal::optionVarIntValue("gpuCacheAllAuto", &existAllAuto);
    if (!existAllAuto) {
        MGlobal::setOptionVarValue("gpuCacheAllAuto", 1);
    }
    bool automatic = !existAllAuto || allAutoValue == 1;
    syncIntOptionVar(automatic, "gpuCacheMaxVramAuto", "gpuCacheMaxVram", sDefaultMaxVBOSize, sMaxVBOSize, 1024*1024);
    syncIntOptionVar(automatic, "gpuCacheMaxNumOfBuffersAuto", "gpuCacheMaxNumOfBuffers", sDefaultMaxVBOCount, sMaxVBOCount);
    syncIntOptionVar(automatic, "gpuCacheMinVerticesPerShapeAuto", "gpuCacheMinVerticesPerShape", sDefaultMinVertsForVBOs, sMinVertsForVBOs);
    syncBoolOptionVar(automatic, "gpuCacheLowVramOperationAuto", "gpuCacheLowMemMode", sDefaultUseVertexArrayWhenVRAMIsLow, sUseVertexArrayWhenVRAMIsLow, 2);

    syncBoolOptionVar(automatic, "gpuCacheGlSelectionModeAuto", "gpuCacheGlSelectionMode", sDefaultUseVertexArrayForGLPicking, sUseVertexArrayForGLPicking, 1);
    syncIntOptionVar(automatic, "gpuCacheSelectionWireThresholdAuto", "gpuCacheSelectionWireThreshold", sDefaultOpenGLPickingWireframeThreshold, sOpenGLPickingWireframeThreshold);
    syncIntOptionVar(automatic, "gpuCacheSelectionSurfaceThresholdAuto", "gpuCacheSelectionSurfaceThreshold", sDefaultOpenGLPickingSurfaceThreshold, sOpenGLPickingSurfaceThreshold);

    syncBoolOptionVar(automatic, "gpuCacheDisableVertexArraysAuto", "gpuCacheUseVertexArrays", sDefaultUseGLPrimitivesInsteadOfVA, sUseGLPrimitivesInsteadOfVA, 2);
    syncBoolOptionVar(automatic, "gpuCacheTwoSidedLightingAuto", "gpuCacheTwoSidedLightingMode", sDefaultEmulateTwoSidedLighting, sEmulateTwoSidedLighting, 2);
    syncBoolOptionVar(automatic, "gpuCacheUvCoordinatesAuto", "gpuCacheIgnoreUv", sDefaultIsIgnoringUVs, sIsIgnoringUVs, true);
    syncIntOptionVar(automatic, "gpuCacheVP2OverrideAPIAuto", "gpuCacheVP2OverrideAPI", sDefaultVP2OverrideAPI, sVP2OverrideAPI);
    syncBoolOptionVar(automatic, "gpuCacheBackgroundReadingAuto", "gpuCacheBackgroundReading", sDefaultBackgroundReading, sBackgroundReading, true);
    syncIntOptionVar(automatic, "gpuCacheBackgroundReadingRefreshAuto", "gpuCacheBackgroundReadingRefresh", sDefaultBackgroundReadingRefresh, sBackgroundReadingRefresh);
    syncBoolOptionVar(automatic, "gpuCacheUseHardwareInsancingAuto", "gpuCacheUseHardwareInstancing", sDefaultUseHardwareInstancing, sUseHardwareInstancing, true);
    syncIntOptionVar(automatic, "gpuCacheHardwareInstancingThresholdAuto", "gpuCacheHardwareInstancingThreshold", sDefaultHardwareInstancingThreshold, sHardwareInstancingThreshold);
}

void Config::initialize()
{
    // Initialize once on demand
    if (!sInitialized) {
        // Initialize the default values
        sDefaultMaxVBOSize                      = getMaxVBOSizeDefault();
        sDefaultMaxVBOCount                     = getMaxVBOCountDefault();
        sDefaultMinVertsForVBOs                 = getMinVertsForVBOsDefault();
        sDefaultUseVertexArrayWhenVRAMIsLow     = getUseVertexArrayWhenVRAMIsLowDefault();
        sDefaultUseVertexArrayForGLPicking      = getUseVertexArrayForGLPickingDefault();
        sDefaultOpenGLPickingWireframeThreshold = getOpenGLPickingWireframeThresholdDefault();
        sDefaultOpenGLPickingSurfaceThreshold   = getOpenGLPickingSurfaceThresholdDefault();
        sDefaultUseGLPrimitivesInsteadOfVA      = getUseGLPrimitivesInsteadOfVADefault();
        sDefaultEmulateTwoSidedLighting         = getEmulateTwoSidedLightingDefault();
        sDefaultIsIgnoringUVs                   = getIgnoreUVsDefault();
        sDefaultBackgroundReading               = getBackgroundReadingDefault();
        sDefaultBackgroundReadingRefresh        = getBackgroundReadingRefreshDefault();
        sDefaultUseHardwareInstancing           = getUseHardwareInstancingDefault();
        sDefaultHardwareInstancingThreshold     = getHardwareInstancingThresholdDefault();

        // Initialize current values with default values
        sMaxVBOSize                      = sDefaultMaxVBOSize;
        sMaxVBOCount                     = sDefaultMaxVBOCount;
        sMinVertsForVBOs                 = sDefaultMinVertsForVBOs;
        sUseVertexArrayWhenVRAMIsLow     = sDefaultUseVertexArrayWhenVRAMIsLow;
        sUseVertexArrayForGLPicking      = sDefaultUseVertexArrayForGLPicking;
        sOpenGLPickingWireframeThreshold = sDefaultOpenGLPickingWireframeThreshold;
        sOpenGLPickingSurfaceThreshold   = sDefaultOpenGLPickingSurfaceThreshold;
        sUseGLPrimitivesInsteadOfVA      = sDefaultUseGLPrimitivesInsteadOfVA;
        sEmulateTwoSidedLighting         = sDefaultEmulateTwoSidedLighting;
        sIsIgnoringUVs                   = sDefaultIsIgnoringUVs;
        sBackgroundReading               = sDefaultBackgroundReading;
        sBackgroundReadingRefresh        = sDefaultBackgroundReadingRefresh;
        sUseHardwareInstancing           = sDefaultUseHardwareInstancing;
        sHardwareInstancingThreshold     = sDefaultHardwareInstancingThreshold;

        sInitialized = true;
    
        // Sync with option vars
        refresh();

#ifdef _WIN32
        // Emit a warning if the graphics driver has known issues.
        // Quadro driver interferes with ReadFile function. (MAYA-32563)
        if (VramQuery::isQuadro()) {
            int driverVersion[3] = {0};
            VramQuery::driverVersion(driverVersion);
            if (driverVersion[0] != 0 && getenv("MAYA_GPUCACHE_WORKAROUND_QUADRO_PAGE_READONLY") == NULL &&
                (driverVersion[0] < 332 ||
                    (driverVersion[0] == 332 && driverVersion[1] < 50))) {
                printf("The graphics driver (%d.%d.%d) has known issues and might not work properly with gpuCache.\n",
                    driverVersion[0], driverVersion[1], driverVersion[2]);
                printf("Please upgrade the graphics driver to the latest version. (> 332.50)\n");
                printf("Otherwise, set MAYA_GPUCACHE_WORKAROUND_QUADRO_PAGE_READONLY env if the driver has to be kept.\n");
            }
        }
#endif
    }
}

} // namespace GPUCache

