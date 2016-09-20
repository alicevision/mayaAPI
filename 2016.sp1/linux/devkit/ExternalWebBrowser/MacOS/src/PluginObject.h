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

#import <WebKit/npapi.h>
#import <WebKit/npfunctions.h>
#import <WebKit/npruntime.h>

typedef struct
{
    NPClass *_class;
    uint32_t referenceCount;
    NPP npp;
    NPWindow*	window;
	NPString	port;
	int			socket;
} PluginObject;

extern NPClass *getPluginClass(void);

