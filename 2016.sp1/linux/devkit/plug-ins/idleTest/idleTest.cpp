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

#include <stdio.h>

#define OPENMAYA_EXPORT

#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MEventMessage.h>
#include <maya/MUiMessage.h>
#include <maya/MFnPlugin.h>

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>

///////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

///////////////////////////////////////////////////
//
// Declarations
//
///////////////////////////////////////////////////

// Callback function for messages
//
// static void idleCB(void * data);
// static void uiDeletedCB(void * data);

// Static pointer to the current primeFinder so we can delete
// it if the plug-in is unloaded.
//
class primeFinder;
static primeFinder* currentPrimeFinder = 0;

///////////////////////////////////////////////////
//
// Command class declaration
//
///////////////////////////////////////////////////
class idleTest : public MPxCommand
{
public:
	idleTest();
	virtual                 ~idleTest(); 

	MStatus                 doIt( const MArgList& args );

	static MSyntax			newSyntax();
	static void*			creator();

private:
	MStatus                 parseArgs( const MArgList& args );

	int						primeCount;
};

///////////////////////////////////////////////////
//
// Computation class implementation
//
///////////////////////////////////////////////////
class primeFinder
{
public:
	primeFinder(int max);
	~primeFinder();

protected:
	static void				idleCB(void * data);
	static void				uiDeletedCB(void * data);

	MString					list;			// The list widget
	int						test;			// Next number to test
	int						found;			// How many numbers have been found
	int						count;			// How many numbers will we find
	int	 *					primes;			// The list of numbers.

	MCallbackId				idleCallbackId;
	MCallbackId				uiDeletedCallbackId;
};


///////////////////////////////////////////////////
//
// Command class implementation
//
///////////////////////////////////////////////////

// Constructor
//
idleTest::idleTest()
:	primeCount(2)
{
}

// Destructor
//
idleTest::~idleTest()
{
	// Do nothing
}

// creator
//
void* idleTest::creator()
{
	return (void *) (new idleTest);
}

// newSyntax
//
MSyntax idleTest::newSyntax()
{
	MSyntax syntax;

	// How many prime numbers should we generate.
	//
	syntax.addArg(MSyntax::kLong);

	return syntax;
}

// parseArgs
//
MStatus idleTest::parseArgs(const MArgList& args)
{
	MStatus			status;
	MArgDatabase	argData(syntax(), args);

	status = argData.getCommandArgument(0, primeCount);

	if (!status)
	{
		status.perror("could not parse integer command argument");
		return status;
	}

	return status;
}

// doIt
//
MStatus idleTest::doIt(const MArgList& args)
{
	MStatus status;

	status = parseArgs(args);
	if (!status)
	{
		return status;
	}

	try
	{
		// Construct a new computation object.
		//
		/* primeFinder* pf = */ new primeFinder(primeCount);
	}
	catch(MStatus status)
	{
		return status;
	}
	catch(...)
	{
		throw;
	}

	return status;
}

///////////////////////////////////////////////////
//
// primeFinder implementation
//
///////////////////////////////////////////////////
primeFinder::primeFinder(int max)
:	test(3)
,	found(1)	// Initialize to 1 because we put 2 in the list automatically
,	count(max)
{
	MStatus status;

	// Allocate our list of primes.
	//
	primes = new int[max];
	if (!primes)
	{
		throw (MStatus(MStatus::kInsufficientMemory));
	}

	primes[0] = 2;

	// Create the UI
	//
	MString window,     form,     close;
	char   *windowCmd, *formCmd, *closeCmd, *listCmd, *attachCmd, *showCmd;

	char tmpStr[1024];

	windowCmd =
		"window -wh 200 400 -t \"Prime Numbers\" -in Primes;";

	formCmd = "formLayout";

	listCmd = "textScrollList -a 2;";

	closeCmd = "button -l \"Close\" -c \"deleteUI -window %s;\";";

	attachCmd = "formLayout -edit "
				"-an %s	top "
				"-af %s	bottom 5 "
				"-ap %s	left   0 30 "
				"-ap %s	right  0 70 "

				"-af %s	left   5 "
				"-af %s	top    5 "
				"-af %s	right  5 "
				"-ac %s	bottom 5 %s "
				"%s;";

	showCmd = "showWindow %s;";

	const char *c, *l, *f, *w;

	status = MGlobal::executeCommand(windowCmd, window, true);
	if (!status)
		throw(status);
	w = window.asChar();

	status = MGlobal::executeCommand(formCmd, form, true);
	if (!status)
		throw(status);
	f = form.asChar();

	status = MGlobal::executeCommand(listCmd, list, true);
	if (!status)
		throw(status);
	l = list.asChar();

	sprintf(tmpStr, closeCmd, window.asChar());
	status = MGlobal::executeCommand(tmpStr, close, true);
	if (!status)
		throw(status);
	c = close.asChar();

	sprintf(tmpStr, attachCmd, c, c, c, c, l, l, l, l, c, f);
	status = MGlobal::executeCommand(tmpStr, true);
	if (!status)
		throw(status);

	sprintf(tmpStr, showCmd, w);
	status = MGlobal::executeCommand(tmpStr, true);
	if (!status)
		throw(status);

	// Add the callbacks
	//
	idleCallbackId
		= MEventMessage::addEventCallback("idle",
										  &primeFinder::idleCB,
										  (void *) this);

	uiDeletedCallbackId
		= MUiMessage::addUiDeletedCallback(window,
										   &primeFinder::uiDeletedCB,
										   (void *) this);

	// This does not actually work correctly if more than one primeFinder
	// is constructed at a time but I don't want to spend the time to put
	// in a list of available primeFinders in a simple test program.
	//
	if (currentPrimeFinder == NULL)
	{
		currentPrimeFinder = this;
	}
}

primeFinder::~primeFinder()
{
	delete [] primes;

	if (idleCallbackId)
	{
		MMessage::removeCallback(idleCallbackId);
	}

	MMessage::removeCallback(uiDeletedCallbackId);

	if (currentPrimeFinder == this)
	{
		currentPrimeFinder = NULL;
	}
}

// Idle callback.  Test one number for primeness
//
void primeFinder::idleCB(void * data)
{
	primeFinder *pf = (primeFinder *) data;

	int i;

	for (i = 0; i < pf->found; ++i)
	{
		if (pf->test % pf->primes[i] == 0)
		{
			// Not a prime, update test and return
			//
			pf->test += 2;
			return;
		}
	}

	// It is a prime, update the primeFinder data
	//
	pf->primes[pf->found++] = pf->test;

	// Update the list widget
	//
	char tmpStr[1024];

	int rows;

	MGlobal::executeCommand(MString("textScrollList -q -nr ") + pf->list, rows);

	sprintf(tmpStr, "textScrollList -e -a %d -shi %d %s;",
			pf->test, MAX(pf->found + 1 - rows, 1), pf->list.asChar());

	MGlobal::executeCommand(tmpStr);

	// Increment test to the next candidate number
	//
	pf->test += 2;

	// Finally, if we have found the requested number of primes, cancel
	// the idle callback.
	//
	if (pf->found >= pf->count && pf->idleCallbackId)
	{
		MMessage::removeCallback(pf->idleCallbackId);
		pf->idleCallbackId = 0;
	}

	return;
}

void primeFinder::uiDeletedCB(void * data)
{
	primeFinder *pf = (primeFinder *) data;

	MGlobal::displayWarning("primeFinder window deleted.  Callbacks cancelled.\n");

	delete pf;
}

///////////////////////////////////////////////////
//
// Plug-in functions
//
///////////////////////////////////////////////////

MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");

	// Register the command so we can actually do some work
	//
	status = plugin.registerCommand("idleTest",
									idleTest::creator,
									idleTest::newSyntax);

	if (!status)
	{
		status.perror("registerCommand");
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus   status;
	MFnPlugin plugin( obj );

	// If there is a current primeFinder, delete it.  The uninitialize
	// call can still cause problems if there were more than one primeFinder
	// running and the plug-in is unloaded, but I was too lazy to put in
	// a robust list of active primeFinders for a simple test program.
	//
	if (currentPrimeFinder)
	{
		delete currentPrimeFinder;
	}

	// Deregister the command
	//
	status = plugin.deregisterCommand("idleTest");

	if (!status)
	{
		status.perror("deregisterCommand");
	}

	return status;
}

