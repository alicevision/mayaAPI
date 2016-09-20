//
//  Copyright 2012 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//

//-----------------------------------------------------------------------------
//- StdAfx.h : include file for standard system include files,
//-      or project specific include files that are used frequently,
//-      but are changed infrequently
//-----------------------------------------------------------------------------
#pragma once

#include <maya/MTypeId.h>
#include <maya/MTypes.h>
#include <maya/MString.h>
#include <maya/MThreadUtils.h>
#include <maya/MThreadAsync.h>
#include <maya/MSpinLock.h>
#include <maya/MPxCommand.h>
#include <maya/MFileObject.h>
#include <maya/MIOStream.h>
#include <maya/MSceneMessage.h>

#ifdef NT_PLUGIN
#include <tchar.h>
#else
#define _T(a) a
#endif

#define _ST(a) #a
#define AUTOLOADER_THREAD

#include "threadData.h"
#include "moduleLogic.h"
