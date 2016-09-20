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

// DISCLAIMER: THIS PLUGIN IS PROVIDED AS IS.  IT IS NOT SUPPORTED BY

//            AUTODESK, SO PLEASE USE AND MODIFY AT YOUR OWN RISK.

//

// PLUGIN NAME: closestPointOnCurve v1.0

// FILE: closestPointOnCurvePlugin.cpp

// DESCRIPTION: -Defines the functions used to load and unload the plugin.

//              -Please see readme.txt for full details.

// AUTHOR: QT

// REFERENCES: -This plugin's concept is based off of the "closestPointOnSurface" node.

//             -The MEL script AEclosestPointOnSurfaceTemplate.mel was referred to for

//              the AE template MEL script that accompanies the closestPointOnCurve node.

// LAST UPDATED: Oct. 13th, 2001.

// COMPILED AND TESTED ON: Maya 4.0 on Windows





// HEADER FILES:

#include "closestPointOnCurveCmd.h"

#include "closestPointOnCurveNode.h"

#include "closestPointOnCurveStrings.h"

#include <maya/MFnPlugin.h>


// Register all strings used by the plugin C++ code
static MStatus registerMStringResources(void)
{
	MStringResource::registerString(kNoValidObject);
	MStringResource::registerString(kInvalidType);
	MStringResource::registerString(kNoQueryFlag);
	return MS::kSuccess;
}




// INITIALIZES THE PLUGIN BY REGISTERING COMMAND AND NODE:

MStatus initializePlugin(MObject obj)

{

   MStatus status;



   MFnPlugin plugin(obj, PLUGIN_COMPANY, "4.0", "Any");


   // Register string resources used in the code and scripts
   // This is done first, so the strings are available. 
   status = plugin.registerUIStrings(registerMStringResources, "closestPointOnCurveInitStrings");
   if (!status)
   {
      status.perror("registerUIStrings");
      return status;
   }


   status = plugin.registerCommand("closestPointOnCurve", closestPointOnCurveCommand::creator, closestPointOnCurveCommand::newSyntax);

   if (!status)

   {

      status.perror("registerCommand");

      return status;

   }



   status = plugin.registerNode("closestPointOnCurve", closestPointOnCurveNode::id, closestPointOnCurveNode::creator, closestPointOnCurveNode::initialize);

   if (!status)

   {

      status.perror("registerNode");

      return status;

   }



   return status;

}





// UNINITIALIZES THE PLUGIN BY DEREGISTERING COMMAND AND NODE:

MStatus uninitializePlugin(MObject obj)

{

   MStatus status;



   MFnPlugin plugin(obj);



   status = plugin.deregisterCommand("closestPointOnCurve");

   if (!status)

   {

      status.perror("deregisterCommand");

      return status;

   }



   status = plugin.deregisterNode(closestPointOnCurveNode::id);

   if (!status)

   {

      status.perror("deregisterNode");

      return status;

   }



   return status;

}

