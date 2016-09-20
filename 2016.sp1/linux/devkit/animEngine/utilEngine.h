#ifndef _utilEngine
#define _utilEngine

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

#include <engine.h>

EtBoolean engineUtilStringsMatch (EtByte *string1, EtByte *string2);
EtVoid engineUtilCopyString (EtByte *src, EtByte *dest);
EtByte *engineUtilAllocate (EtInt bytes);
EtVoid engineUtilFree (EtByte *block);

#endif
