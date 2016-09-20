//
//  Copyright 2012 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//

//-----------------------------------------------------------------------------
#pragma once

#define threadDataDelayName _T("AUTOLOADER_LAPS")
#define threadDataDelayDefault 10 // every 10 seconds (default)
#define szthreadDataDelayDefault _T("10")

typedef struct _threadData {
	static volatile bool bThreadToExecute ;
	static volatile bool bWaitingForCommand ;

	MString szPlatform ;
	MString szVersion ;
	MString szLocale ;
	MStringArray szModules ;

	static struct _threadData *getThreadData () ;
	static MStatus startThread () ;
	static void stopThread () ;

protected:
	static MThreadRetVal AsyncModuleThread (void *data) ;
	static void AsyncModuleThreadEnded (void *data) ;

	static MSpinLock exchangeSpinLock ;
	static bool Maya_InterlockedCompare (volatile int *variable, int compareValue) ;
	static void WaitForAsyncThreads (int val) ;

} threadData ;
