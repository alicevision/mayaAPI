//
//  Copyright 2012 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//

//-----------------------------------------------------------------------------
#include "StdAfx.h"

#if !defined( OSWin_ )
#include <unistd.h>
#endif

#include "threadData.h"
#include <maya/MCommonSystemUtils.h>

/*static*/ volatile bool threadData::bThreadToExecute =true ;
/*static*/ volatile bool threadData::bWaitingForCommand =false ;
static threadData tdata ;

struct _threadData *threadData::getThreadData () {
	return (&tdata) ;
}

//-----------------------------------------------------------------------------
// Compute function. This function is called from multiple asynchronous threads
MThreadRetVal threadData::AsyncModuleThread (void *data) {
	threadData *myData =(threadData *)data ;
	while ( threadData::bThreadToExecute ) {
		int ithreadDataDelay =threadDataDelayDefault ;
		char *val =getenv (threadDataDelayName) ;
		if ( val != NULL && *val != _T('\0') )
			ithreadDataDelay =atoi (val) ;

		MModuleLogic::ModuleDetectionLogicCmdExecute (myData) ;
#if defined(_WIN32) || defined(_WIN64)
		Sleep (ithreadDataDelay * 1000) ;
#else
		sleep (ithreadDataDelay) ;
#endif
	}
	return (0) ;
}

//-----------------------------------------------------------------------------
// Variable to track threads. As each thread finishes it's work it
// increments this variable. The main thread will wait until this
// variable is equal to the thread count, meaning all threads have
// completed their work variable will equal the thread count
static volatile int g_async_count =0 ;
static MSpinLock asyncSpinLock ;

// Thread completion callback.
// Increment thread completion variable. Uses a lock to prevent race
// conditions where two threads attempt to update the variable simultaneously
void threadData::AsyncModuleThreadEnded (void *data) {
	asyncSpinLock.lock () ;
	g_async_count++ ;
	asyncSpinLock.unlock () ;
}

MStatus threadData::startThread () {
	// Define Thread Laps as an environment variable
	if ( MCommonSystemUtils::getEnv (threadDataDelayName) == "" )
	{
		MCommonSystemUtils::putEnv (threadDataDelayName, szthreadDataDelayDefault) ;
	}

	// Start threads. Each thread makes a call to AsyncModuleThread
	// The stopThread function called in uninitializePlugin() waits until 
	// all threads have completed (but we got only one here).
	threadData::bThreadToExecute =true ;
	threadData::bWaitingForCommand =false ;
	MStatus ms =MThreadAsync::init () ;
	if ( ms != MS::kSuccess )
		return (ms) ;
	ms =MThreadAsync::createTask (
		threadData::AsyncModuleThread,
		(void *)threadData::getThreadData (),
		threadData::AsyncModuleThreadEnded,
		NULL
	) ;
	return (ms) ;
}

//-----------------------------------------------------------------------------
// Test if variable matches the expected value. Locks required to
// ensure thread safe access to variables.
/*static*/ MSpinLock threadData::exchangeSpinLock ;

bool threadData::Maya_InterlockedCompare (volatile int *variable, int compareValue) {
	exchangeSpinLock.lock () ;
	bool rtn =(*variable == compareValue) ;
	exchangeSpinLock.unlock () ;
	return (rtn) ;
}

// Barrier function. Main thread enters here and polls the count
// variable until all worker threads have indicated they have
// completed by incrementing this count.
void threadData::WaitForAsyncThreads (int val) {
	while ( !threadData::Maya_InterlockedCompare (&g_async_count, val)) {
#if defined(_WIN32) || defined(_WIN64)
		Sleep (0) ;
#else
		sleep (0) ;
#endif
	}
}

void threadData::stopThread () {
	// Ask the thread to terminate
	threadData::bThreadToExecute =false ;

	// Waits until all threads have completed before continuing
	threadData::WaitForAsyncThreads (1) ;
	MThreadAsync::release () ; // release async thread
}
