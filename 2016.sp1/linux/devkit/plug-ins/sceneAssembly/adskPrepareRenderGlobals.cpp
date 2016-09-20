#include "adskPrepareRenderGlobals.h"

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringData.h>

#include <iostream>             // For CHECK_MSTATUS_AND_RETURN_IT.
using namespace std;            // For CHECK_MSTATUS macros.

namespace {

//==============================================================================
// LOCAL DECLARATIONS
//==============================================================================

/*----- constants -----*/

const char* const ICON_NAME = "adskPrepareRenderGlobals.png";

}

//==============================================================================
// CLASS AdskPrepareRenderGlobals
//==============================================================================

const MTypeId AdskPrepareRenderGlobals::id(0x580000b3);

const MString AdskPrepareRenderGlobals::typeName("adskPrepareRenderGlobals");

MObject AdskPrepareRenderGlobals::aRepName;
MObject AdskPrepareRenderGlobals::aRepLabel;
MObject AdskPrepareRenderGlobals::aRepType;
MObject AdskPrepareRenderGlobals::aUseRegEx;

//------------------------------------------------------------------------------
//
void* AdskPrepareRenderGlobals::creator()
{
   return new AdskPrepareRenderGlobals;
}

//------------------------------------------------------------------------------
//
MStatus AdskPrepareRenderGlobals::initialize()
{    
   MStatus stat;  
   MFnStringData stringFn;
   MObject emptyStr = stringFn.create( &stat );
   
   MObject* attrib[3];
   attrib[0] = &aRepName;
   attrib[1] = &aRepLabel;
   attrib[2] = &aRepType;
   const char* longName[] = {"repName", "repLabel", "repType"};
   const char* shortName[] = {"rna", "rla", "rty"};
   
   MFnTypedAttribute stringAttrFn;
   for (int i = 0; i < 3; i++)
   {      
      *attrib[i] = stringAttrFn.create(longName[i], shortName[i], MFnData::kString, emptyStr);     
      stat = MPxNode::addAttribute(*attrib[i]);
      CHECK_MSTATUS_AND_RETURN_IT(stat);
   }
   
   MFnNumericAttribute boolAttrFn;
   aUseRegEx = boolAttrFn.create("useRegExp", "urx", MFnNumericData::kBoolean, 0);  
   stat = MPxNode::addAttribute(aUseRegEx);
   CHECK_MSTATUS_AND_RETURN_IT(stat);

   return  stat;
}

//------------------------------------------------------------------------------
//
MStatus AdskPrepareRenderGlobals::uninitialize()
{
   return MStatus::kSuccess;
}

//------------------------------------------------------------------------------
//
AdskPrepareRenderGlobals::AdskPrepareRenderGlobals() 
{}

//------------------------------------------------------------------------------
//
AdskPrepareRenderGlobals::~AdskPrepareRenderGlobals()
{}

//------------------------------------------------------------------------------
//
MString AdskPrepareRenderGlobals::getDefaultIcon() const
{
   return MString(ICON_NAME);
}


//-
//*****************************************************************************
// Copyright 2015 Autodesk, Inc.  All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy
// form.
//*****************************************************************************
//+

