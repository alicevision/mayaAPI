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

#include <sys/types.h>
#include <npapi.h>
#include <npupp.h>

extern NPNetscapeFuncs* browser;

// Command Port Stuff
int mcpRead(int fd, void *ptr, unsigned int nbytes);
int mcpWrite(int fd, void *ptr, unsigned int nbytes);
int mcpOpen(char *name);
void mcpClose(int fd);
