//
//  Copyright 2012 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//

//-----------------------------------------------------------------------------
#include "StdAfx.h"

#include "moduleLogicCmd.h"

//-----------------------------------------------------------------------------
MStatus moduleLogicCmd::doIt (const MArgList &args) {
	MModuleLogic::ModuleDetectionLogic (threadData::getThreadData (), true) ;
	return (MS::kSuccess) ;
}
