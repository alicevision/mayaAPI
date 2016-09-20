//
// Copyright (C) 2002-2003 NVIDIA 
// 
// File: cgfxFindImage.h

//-
// ==========================================================================
// Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

#ifndef _cgfxFindImage_h_
#define _cgfxFindImage_h_

#include "cgfxShaderCommon.h"

#include <maya/MString.h>
#include <maya/MStringArray.h>

#define _CGFX_PLUGIN_MAX_COMPILER_ARGS_		128		// maximum # of compiler arguments

MString cgfxFindFile(const MString& name, bool projectRelative = false);
void	cgfxGetFxIncludePath( const MString &fxFile, MStringArray &pathOptions );

#endif /* _cgfxFindImage_h */
