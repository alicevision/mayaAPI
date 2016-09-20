#ifndef _animEngine
#define _animEngine

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

EtChannel *engineAnimReadCurves (EtFileName fileName, EtInt *numCurves);
EtVoid engineAnimFreeChannelList (EtChannel *channelList);
EtValue engineAnimEvaluate (EtCurve *animCurve, EtTime time);

#endif
