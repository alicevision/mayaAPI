//
//  Copyright 2012 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//

//-----------------------------------------------------------------------------
#pragma once

#include "threadData.h"

#include <libxml/xmlversion.h>
#undef LIBXML_ICONV_ENABLED
#include <libxml/parser.h>
#include <libxml/xpath.h>

class MModuleLogic {

private:
	MModuleLogic () {}

public:
	static void ModuleDetectionLogicInit (threadData *data) ;
	static void ModuleDetectionLogicCmdExecute (threadData *data) ;
	static void ModuleDetectionLogic (threadData *data, bool bInitNow =true) ;

protected:
	static void InitNewModules (threadData *data, MString &modName, MString &packageFile) ;

	static bool isPackageReady (threadData *data, MString &modFile) ;
	static bool executePackageContents (threadData *data, MString &modFile) ;
	static bool executeComponentEntry (threadData *data, xmlXPathContextPtr &context) ;

} ;
