#ifndef _gpuCacheGLFT_h_
#define _gpuCacheGLFT_h_

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

#include <maya/MGLFunctionTable.h>

//==============================================================================
// GLOBAL VARIABLES
//==============================================================================

namespace GPUCache {

extern MGLFunctionTable* gGLFT;

void InitializeGLFT();

}

#endif
