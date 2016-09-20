//
//  Copyright 2012 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//

//-----------------------------------------------------------------------------
#include "StdAfx.h"

#include <maya/MFnPlugin.h>

#include "moduleLogicCmd.h"

//-----------------------------------------------------------------------------
MCallbackId mayaExitingId =0 ;
void mayaExitingCB (void *clientData) {
#ifdef AUTOLOADER_THREAD
	if ( MGlobal::mayaState () != MGlobal::kBatch )
	{
		threadData::stopThread () ;
	}
#endif
}

//-----------------------------------------------------------------------------
//- initializePlugin is called when the plug-in is loaded into Maya. It 
//- registers all of the services that this plug-in provides with Maya.
//-		obj - a handle to the plug-in object (use MFnPlugin to access it)
MStatus initializePlugin (MObject obj) {
	MFnPlugin plugin (obj, PLUGIN_COMPANY, _T("1.0"), _T("Any")) ;

	// Before launching the module detection, we need to collected the ones 
	// already present.
	MModuleLogic::ModuleDetectionLogicInit (threadData::getThreadData ()) ;

	plugin.registerCommand (kmoduleLogicCmdName, moduleLogicCmd::creator) ;
#ifdef AUTOLOADER_THREAD
	if ( MGlobal::mayaState () != MGlobal::kBatch )
	{
		threadData::startThread () ;
	}
#endif

	mayaExitingId =MSceneMessage::addCallback (MSceneMessage::kMayaExiting, mayaExitingCB) ;

	return (MS::kSuccess) ;
}

//- uninitializePlugin is called when the plug-in is unloaded from Maya. It 
//- deregisters all of the services that it was providing.
//-		obj - a handle to the plug-in object (use MFnPlugin to access it)
MStatus uninitializePlugin (MObject obj) {
	MFnPlugin plugin (obj) ;

	MSceneMessage::removeCallback (mayaExitingId) ;

#ifdef AUTOLOADER_THREAD
	if ( MGlobal::mayaState () != MGlobal::kBatch )
	{
		threadData::stopThread () ;
	}
#endif
	plugin.deregisterCommand (kmoduleLogicCmdName) ;

	return (MS::kSuccess) ;
}
