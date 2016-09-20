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

#include "gpuCacheGLFT.h"

#include <maya/MHardwareRenderer.h>

namespace {

//==============================================================================
// LOCAL FUNCTIONS
//==============================================================================

    // Global OpenGL function table
    static MGLFunctionTable* getGLFunctionTable()
    {
        MHardwareRenderer* renderer = MHardwareRenderer::theRenderer();
        if (renderer == NULL) {
            return NULL;
        }

        return renderer->glFunctionTable();
    }
}

//==============================================================================
// GLOBAL VARIABLES
//==============================================================================

namespace GPUCache {

MGLFunctionTable* gGLFT = NULL;

void InitializeGLFT() {
    static bool sGLFTInitialized = false;

    if (!sGLFTInitialized) {
        gGLFT = getGLFunctionTable();
        sGLFTInitialized = true;
    }
}

}
