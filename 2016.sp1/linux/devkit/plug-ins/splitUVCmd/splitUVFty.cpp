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
 
// 
// File: splitUVFty.cpp
//
// Node Factory: splitUVFty
//
// Author: Maya SDK Wizard
//

#include "splitUVFty.h"

splitUVFty::splitUVFty()
//
//	Description:
//		splitUVFty constructor
//
{
	fSelUVs.clear();
}

splitUVFty::~splitUVFty()
//
//	Description:
//		splitUVFty destructor
//
{}

void splitUVFty::setMesh( MObject mesh )
//
//	Description:
//		Sets the mesh object that this factory will operate on
//
{
	fMesh = mesh;
}

void splitUVFty::setUVIds( MIntArray uvIds )
//
//	Description:
//		Sets the UV Ids that this factory will operate on
//
{
	fSelUVs = uvIds;
}
