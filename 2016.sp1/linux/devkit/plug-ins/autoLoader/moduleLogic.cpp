//
//  Copyright 2015 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//

//-----------------------------------------------------------------------------
#include "StdAfx.h"

#include "moduleLogic.h"
#include "moduleLogicCmd.h"

#ifndef MAC_PLUGIN
#define MAXPATHLEN 1024
#endif
#define LINE (MAXPATHLEN * 3)
#define KEYWORD_MAYAVERSION _T("MAYAVERSION")
#define KEYWORD_PLATFORM _T("PLATFORM")
#define KEYWORD_LOCALE _T("LOCALE")
#define KEYWORD_SEPARATOR _T(":")

//-----------------------------------------------------------------------------
void MModuleLogic::ModuleDetectionLogicInit (threadData *data) {
    
    //First lets make sure that the the autoLoad plugin doesn't flood the command window
    //with commands when 'echo all commands' is set
    MGlobal::executeCommand (_T("commandEcho -addFilter {\"loadModule\", \"moduleDetectionLogic\"};")) ;

    data->szPlatform =MGlobal::executeCommandStringResult (_T("about -os;")) ; // return 'win64' on Windows x64
	data->szVersion =MGlobal::mayaVersion () ; // return '2013 x64' on Maya 2013 / 2013.5 x64
	data->szVersion =data->szVersion.substringW (0, 3) ; // remove '.5' and ' x64' if present
	data->szLocale =MGlobal::executeCommandStringResult (_T("about -uil;")) ; // return 'en_US' on Maya English version

	//- Now initialize the modules
	MGlobal::executeCommand (_T("moduleInfo -lm;"), data->szModules, false, false) ;
	for ( unsigned int i =0 ; i < data->szModules.length () ; i++ ) {
		MString cmd, modName =data->szModules [i] ;
		cmd.format (_T("moduleInfo -d -mn \"^1s\";"), modName) ;
		MString modFile =MGlobal::executeCommandStringResult (cmd, false, false) ;
		// Old module system (.mod or .txt file in modules folder), nothing additional things to process
		// .mod file do not support anything more than setting variables, so we are ok
		MString ext =modFile.substring (modFile.rindex (_T('.')) + 1, modFile.length () - 1).toLowerCase () ;
		if ( ext != _T("xml") )
			continue ;
		MModuleLogic::InitNewModules (data, modName, modFile) ;
	}
}

void MModuleLogic::InitNewModules (threadData *data, MString &modName, MString &packageFile) {
	MModuleLogic::executePackageContents (data, packageFile) ;
}

//-----------------------------------------------------------------------------
void MModuleLogic::ModuleDetectionLogicCmdExecute (threadData *data) {
	if ( threadData::bWaitingForCommand == false ) {
		threadData::bWaitingForCommand =true ;
		MGlobal::executeCommandOnIdle (kmoduleLogicCmdName, false) ;
	}
}

void MModuleLogic::ModuleDetectionLogic (threadData *data, bool bInitNow /*=true*/) {
	//MGlobal::executeCommand (_T("loadModule -all;"), false, false) ;
	//data->szModules.clear () ;
	//MGlobal::executeCommand (_T("moduleInfo -lm;"), data->szModules, false, false) ;
	
	MString cmd ;
	MStringArray newModules, newModulesName ;
	MStatus ms =MGlobal::executeCommand (_T("loadModule -scan;"), newModules, false, false) ;
	for ( unsigned int i =0 ; i < newModules.length () ; i++ ) {
		MString modFile =newModules [i] ;
		MString ext =modFile.substring (modFile.rindex (_T('.')) + 1, modFile.length () - 1).toLowerCase () ;
		// Old module system does not describe its package content, so we assume all files are on disk
		// For the new module system, the package can list all Vital files, so we need to check they are 
		// all present before initializing the module. Otherwise, we will wait a bit longer and try again
		if ( ext == _T("xml") && !MModuleLogic::isPackageReady (data, modFile) )
			continue ;
		cmd.format (_T("loadModule -load \"^1s\";"), modFile) ;
		ms =MGlobal::executeCommand (cmd, newModulesName, false, false) ;
		data->szModules.append (newModulesName [0]) ;
		// Old module system (.mod or .txt file in modules folder), nothing additional things to process
		// .mod file do not support anything more than setting variables, so we are ok
		if ( ext != _T("xml") )
			continue ;
		// Update the plug-in manager to show new plug-ins if any
		// (do it now before it appears in Plug-in Manager Misc section)
		MGlobal::executeCommand (_T("if ( `exists updatePluginModule` ) updatePluginModule();"), false, false) ;
		MModuleLogic::InitNewModules (data, newModulesName [0], modFile) ;
	}
	//newModulesName.clear () ;
	//ms =MGlobal::executeCommand (_T("moduleInfo -lm;"), newModulesName, false, false) ;

	// Make sure the thread can call us again
	threadData::bWaitingForCommand =false ;
}

//-----------------------------------------------------------------------------
bool MModuleLogic::isPackageReady (threadData *data, MString &modFile) {
	xmlDocPtr doc =xmlParseFile (modFile.asChar ()) ;
	if ( doc == NULL )
		return (false) ;
	xmlXPathContextPtr context =xmlXPathNewContext (doc) ;

	MFileObject fmod ;
	fmod.setFullName (modFile) ;
	MString modPath =fmod.resolvedPath () ;

	MString mxpath ;
	mxpath.format (_T("//Components/RuntimeRequirements[(not(@OS) or contains(@OS, '^1s')) and (not(@Platform) or contains(@Platform, 'Maya'))]"), data->szPlatform) ;
	xmlChar *xpath =(xmlChar *)mxpath.asChar () ;
	xmlXPathObjectPtr result =xmlXPathEvalExpression (xpath, context) ;
	bool bOk =true ;
	if ( result != NULL && !xmlXPathNodeSetIsEmpty (result->nodesetval) ) {
		for ( int j =0 ; j < result->nodesetval->nodeNr && bOk ; j++ ) {
			xmlNodePtr reqNode =result->nodesetval->nodeTab [j] ;
			context->node =reqNode->parent ;
			//- We ignore ./MayaEnv XML nodes since Maya already processed them

			//- Search for ./ComponentEntry XML nodes
			xmlChar *xpathComp =(xmlChar *)"./ComponentEntry" ;
			xmlXPathObjectPtr resultEntries =xmlXPathEvalExpression (xpathComp, context) ;
			if ( resultEntries != NULL && !xmlXPathNodeSetIsEmpty (resultEntries->nodesetval) ) {
				for ( int k =0 ; k < resultEntries->nodesetval->nodeNr && bOk ; k++ ) {
					xmlNodePtr compNode =resultEntries->nodesetval->nodeTab [k] ;
					xmlChar *compName =xmlGetProp (compNode, (xmlChar *)"ModuleName") ;

					MFileObject fobj ;
					fobj.setRawPath (modPath) ;
					fobj.setRawName (MString ((char *)compName)) ;
					if ( !fobj.exists () )
						bOk =false ;

					if ( compName != NULL )
					{
						xmlFree (compName) ;
					}
				}
			}
			if ( resultEntries != NULL )
			{
				xmlXPathFreeObject (resultEntries) ;
			}
		}
	}
	if ( result != NULL )
	{
		xmlXPathFreeObject (result) ;
	}

	if ( context != NULL )
	{
		xmlXPathFreeContext (context) ;
	}
	xmlFreeDoc (doc) ; // Pointer already tested above
	return (bOk) ;
}

//-----------------------------------------------------------------------------
bool MModuleLogic::executePackageContents (threadData *data, MString &modFile) {
	xmlDocPtr doc =xmlParseFile (modFile.asChar ()) ;
	if ( doc == NULL )
		return (false) ;
	xmlXPathContextPtr context =xmlXPathNewContext (doc) ;

	MString mxpath ;
	mxpath.format (_T("//Components/RuntimeRequirements[(not(@OS) or contains(@OS, '^1s')) and (not(@Platform) or contains(@Platform, 'Maya'))]"), data->szPlatform) ;
	xmlChar *xpath =(xmlChar *)mxpath.asChar () ;
	xmlXPathObjectPtr result =xmlXPathEvalExpression (xpath, context) ;
	if ( result != NULL && !xmlXPathNodeSetIsEmpty (result->nodesetval) ) {
		for ( int j =0 ; j < result->nodesetval->nodeNr ; j++ ) {
			xmlNodePtr reqNode =result->nodesetval->nodeTab [j] ;
			context->node =reqNode->parent ;
			//- We ignore ./MayaEnv XML nodes since Maya already processed them

			//- Search for ./ComponentEntry XML nodes
			MModuleLogic::executeComponentEntry (data, context) ;
		}
	}
	if ( result != NULL )
	{
		xmlXPathFreeObject (result) ;
	}

	if ( context != NULL )
	{
		xmlXPathFreeContext (context) ;
	}
	xmlFreeDoc (doc) ; // Pointer already tested above
	return (true) ;
}

bool MModuleLogic::executeComponentEntry (threadData *data, xmlXPathContextPtr &context) {
	MString cmd ;
	xmlChar *xpath =(xmlChar *)"./ComponentEntry" ;
	xmlXPathObjectPtr result =xmlXPathEvalExpression (xpath, context) ;
	if ( result != NULL && !xmlXPathNodeSetIsEmpty (result->nodesetval) ) {
		for ( int j =0 ; j < result->nodesetval->nodeNr ; j++ ) {
			xmlNodePtr compNode =result->nodesetval->nodeTab [j] ;
			xmlChar *compName =xmlGetProp (compNode, (xmlChar *)"ModuleName") ;
			xmlChar *autoload =xmlGetProp (compNode, (xmlChar *)"AutoLoad") ;
			xmlChar *cmdInvoke =xmlGetProp (compNode, (xmlChar *)"LoadOnCommandInvocation") ;

			if ( autoload != NULL && xmlStrcasecmp (autoload, (xmlChar *)"true") == 0 ) {
				MFileObject fobj ;
				fobj.setRawName (MString ((char *)compName)) ;
				// For AutoLoad, plug-ins must be in the MAYA_PLUG_IN_PATH
				cmd.format (_T("if ( !`pluginInfo -query -loaded \"^1s\"` ) loadPlugin -quiet \"^1s\";"), fobj.resolvedName ()) ;
				MGlobal::executeCommand (cmd, false,  false) ;
				cmd.format (_T("pluginInfo -q -path \"^1s\";"), fobj.resolvedName ()) ;
				MString st =MGlobal::executeCommandStringResult (cmd, false,  false) ;
				cmd.format (_T("pluginInfo -edit -autoload true \"^1s\";"), st) ;
				MGlobal::executeCommand (cmd, false,  false) ;
			} else if ( cmdInvoke != NULL && xmlStrcasecmp (cmdInvoke, (xmlChar *)"true") == 0 ) {
				// Not supported yet
			}

			if ( cmdInvoke != NULL )
			{
				xmlFree (cmdInvoke) ;
			}
			if ( autoload != NULL )
			{
				xmlFree (autoload) ;
			}
			if ( compName != NULL )
			{
				xmlFree (compName) ;
			}
		}
	}
	if ( result != NULL )
	{
		xmlXPathFreeObject (result) ;
	}
	return (true) ;
}
