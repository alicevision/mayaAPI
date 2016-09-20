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

// DISCLAIMER: THIS PLUGIN IS PROVIDED AS IS.  IT IS NOT SUPPORTED BY

//            AUTODESK, SO PLEASE USE AND MODIFY AT YOUR OWN RISK.

//

// PLUGIN NAME: closestPointOnCurve v1.0

// FILE: closestPointOnCurveNode.h

// DESCRIPTION: -String resources for "closestPointOnCurve" plugin

// AUTHOR: QT

// REFERENCES: 

// LAST UPDATED: Oct. 13th, 2001.

// COMPILED AND TESTED ON: Maya 4.0 on Windows


// MAYA HEADER FILES:

#include <maya/MStringResource.h>
#include <maya/MStringResourceId.h>

// MStringResourceIds contain plugin id, unique resource id for
// each string and the default value for the string. 

#define kPluginId  "closestPointOnCurve"

#define kNoValidObject MStringResourceId(kPluginId, "kNoValidObject", \
"A curve or its transform node must be specified as a command argument, or using your current selection.")

#define kInvalidType  MStringResourceId(kPluginId, "kInvalidType", \
"Object ^1s has invalid type.  Only a curve or its transform can be specified.")

#define kNoQueryFlag  MStringResourceId(kPluginId, "kNoQueryFlag", \
"You must specify AT LEAST ONE queryable flag in query mode.  Use the `help` command to list all available flags.") 



