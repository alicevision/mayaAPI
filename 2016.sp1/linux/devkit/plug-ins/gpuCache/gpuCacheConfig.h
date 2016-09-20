#ifndef _gpuCacheConfig_h_
#define _gpuCacheConfig_h_

//-
//**************************************************************************/
// Copyright 2012 Autodesk, Inc. All rights reserved. 
//
// Use of this software is subject to the terms of the Autodesk 
// license agreement provided at the time of installation or download, 
// or which otherwise accompanies this software in either electronic 
// or hard copy form.
//**************************************************************************/
//+

#include <stddef.h>
#include <maya/MColor.h>
#include <maya/MString.h>

namespace GPUCache {

/*==============================================================================
 * CLASS Config
 *============================================================================*/

// Flags that control the configuration of the gpuCache plug-in at
// compile time.

class Config
{

public: 
    static void refresh();

    /*----- constants -----*/

    // Maya default color constants.
    static const MColor kDefaultGrayColor;
    static const MColor kDefaultTransparency;

    // Wireframe line styles
    static const unsigned short kLineStippleShortDashed;
    static const unsigned short kLineStippleDotted;

    // Display filter name
    static const MString kDisplayFilter;

    /*----- static member functions -----*/

    enum VP2OverrideAPI {
        kMPxSubSceneOverride,
        kMPxDrawOverride
    };
    // Controls which API is used to draw into Viewport 2.0.
    //
    static VP2OverrideAPI vp2OverrideAPI();

    // Controls whether UV coordinates are used. When used they are
    // computed when baking, they loaded in memory by the cache reader
    // and they are used whenever the material requires it. When
    // disabled, none of these steps are taken and the node therefore
    // uses less memory.
    //
    static bool isIgnoringUVs();

    // Minimum number of vertices that a shape must contain before we
    // decide to use VBOs. This is a heuristic to avoid allocating too
    // many VBOs, which ends causing performance problems on some
    // platforms (i.e. on Mac in particular). The assumption is that
    // for small objects, using vertex arrays is just as efficient.
    //
    static size_t minVertsForVBOs();
    
    // Maximum number of VBOs that will be allocated. This is a
    // heuristic to avoid allocating too many VBOs, which ends causing
    // some catastrophic performance problems on some platforms
    // (i.e. on Mac in particular).
    //
    static size_t maxVBOCount();
    
    // Maximum total size of the VBOs that the gpuCache plug-in will
    // allocate (measured in bytes). This is a heuristic to avoid
    // allocating too many VBOs, which ends causing some catastrophic
    // performance problems on some platforms where over allocation
    // causes VBOs to be evicted to main memory thus increasing the
    // total memory usage of Maya.
    //
    static size_t maxVBOSize();

    // Indicates whether we should switch to using vertex arrays to
    // draw the geometry when running low on video memory and there is
    // not enough video memory available to keep more VBOs around from
    // frame to frame. The alternative is to use temporary VBOs that
    // immediately get deleted after each draw. This allows one to
    // benchmark which approach is faster on a given platform.
    //
    static bool useVertexArrayWhenVRAMIsLow();

    // Indicates whether we should use vertex arrays, instead of VBOs,
    // to draw the geometry when performing OpenGL picking. This
    // allows one to benchmark which approach is faster on a given
    // platform.
    //
    static bool useVertexArrayForGLPicking();

    // Indicates whether we should avoid using vertex arrays and use
    // GL primitives instead.. This is used mainly to avoid
    // graphic device driver bugs.
    //
    static bool useGLPrimitivesInsteadOfVA();

    // Indicates whether we need to emulate two-sided lighting on the
    // current graphic card. On some graphic cards, OpenGL two-sided
    // lighting can be 10 to 20X slower than one-sided lighting and
    // emulation ends-up being faster. 
    //
    static bool emulateTwoSidedLighting();
    
    // Threshold values that controls whether OpenGL picking or
    // raster-based picking should be used. Above this
    // value, we use raster-based picking.
    //
    static size_t openGLPickingWireframeThreshold();
    static size_t openGLPickingSurfaceThreshold();

    // Indicates whether we will load cache files in the background.
    // Control is returned to Maya GUI thread immediately.
    // A separate TBB worker thread will load the cache file.
    //
    static bool backgroundReading();

    // The time interval between two idle refresh commands when reading
    // the cache file in background. (Milliseconds)
    //
    static size_t backgroundReadingRefresh();

    // Indicates whether we will support hardware instancing in Viewport 2.0
    // Viewport 2.0 will make use of the instancing API for identical render items.
    // (e.g. glDrawElementsInstanced in OpenGL).
    // This is on by default. "GPU Instancing" in Hardware Renderer 2.0 Settings
    // should also be enabled. Otherwise, this switch will not take effect.
    //
    static bool useHardwareInstancing();

    // The minimum number of identical render items that we will start treat them
    // as instances. This is the threshold that trigger hardware instancing.
    static size_t hardwareInstancingThreshold();

    // Initialize the Config. It will read hardware parameters and set all fields.
    //
    static void initialize();

private:
    static bool sInitialized;

    static size_t sDefaultVP2OverrideAPI;
    static bool sDefaultIsIgnoringUVs;
    static size_t sDefaultMinVertsForVBOs;
    static size_t sDefaultMaxVBOCount;
    static size_t sDefaultMaxVBOSize;
    static bool sDefaultUseVertexArrayWhenVRAMIsLow;
    static bool sDefaultUseVertexArrayForGLPicking;
    static bool sDefaultUseGLPrimitivesInsteadOfVA;
    static bool sDefaultEmulateTwoSidedLighting;
    static size_t sDefaultOpenGLPickingWireframeThreshold;
    static size_t sDefaultOpenGLPickingSurfaceThreshold;
    static bool sDefaultBackgroundReading;
    static size_t sDefaultBackgroundReadingRefresh;
    static bool sDefaultUseHardwareInstancing;
    static size_t sDefaultHardwareInstancingThreshold;

    static size_t sVP2OverrideAPI;
    static bool sIsIgnoringUVs;
    static size_t sMinVertsForVBOs;
    static size_t sMaxVBOCount;
    static size_t sMaxVBOSize;
    static bool sUseVertexArrayWhenVRAMIsLow;
    static bool sUseVertexArrayForGLPicking;
    static bool sUseGLPrimitivesInsteadOfVA;
    static bool sEmulateTwoSidedLighting;
    static size_t sOpenGLPickingWireframeThreshold;
    static size_t sOpenGLPickingSurfaceThreshold;
    static bool sBackgroundReading;
    static size_t sBackgroundReadingRefresh;
    static bool sUseHardwareInstancing;
    static size_t sHardwareInstancingThreshold;
};

} // namespace GPUCache

#endif
