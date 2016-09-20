#ifndef _fileEngine
#define _fileEngine

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

EtFileHandle engineFileOpen (EtFileName fileName);
EtVoid engineFileClose (EtFileHandle fileHandle);
EtBoolean engineFileReadByte (EtFileHandle fileHandle, EtByte *byte);
EtByte *engineFileReadWord (EtFileHandle fileHandle);
EtInt engineFileReadInt (EtFileHandle fileHandle);
EtFloat engineFileReadFloat (EtFileHandle fileHandle);

#endif
