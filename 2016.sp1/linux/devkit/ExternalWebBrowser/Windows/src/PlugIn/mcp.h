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

#include <npapi.h>
#include <npupp.h>

extern NPNetscapeFuncs* browser;

// Command Port Stuff
int mcpRead(SOCKET fd, void *ptr, unsigned int nbytes);
int mcpWrite(SOCKET fd, void *ptr, unsigned int nbytes);
SOCKET mcpOpen(char *name);
void mcpClose(SOCKET fd);
