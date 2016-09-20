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

/* 

	file : userMsgCmd.cpp
	class: userMessage
	----------------------
	This is an example to demonstrate the usages of the MUserEventMessage class.
	It allows the user to create, destroy, and post to user-defined events
	identified by strings.

	The command "userMessage" supports the following options:

		-r/-register string : Register a new event type with the given name.
			Registration also attaches two callback functions to the event,
			userCallback1 and userCallback2.

		-d/-deregister string : Deregister an existing event with the given name

		-p/-post string : Post the event.  In this case, it simply notifies
			userCallback1 and userCallback2, which print info messages.

		-t/-test : Run a basic set of tests that demonstrate how the user event
			types can be used.  See userMessage::runTests()

	Only one option should be specified per invokation.
	
*/

#include <maya/MFnPlugin.h>
#include <maya/MStatus.h>
#include <maya/MPxCommand.h>
#include <maya/MMessage.h>
#include <maya/MUserEventMessage.h>
#include <maya/MGlobal.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <maya/MSyntax.h>

// Syntax string definitions
static const char *postFlag = "-p";
static const char *postLongFlag = "-post";
static const char *registerFlag = "-r";
static const char *registerLongFlag = "-register";
static const char *deregisterFlag = "-d";
static const char *deregisterLongFlag = "-deregister";
static const char *testFlag = "-t";
static const char *testLongFlag = "-test";

class userMessage : public MPxCommand
{
	public:
		virtual MStatus doIt( const MArgList& );
		void runTests();

		static void *creator();
		static MSyntax newSyntax();

		// callback functions.
		static void userCallback1(void* clientData);
		static void userCallback2(void* clientData);

	private:
		static MString stringClientData;
};

// Define a string that will be passed to the callback functions
MString userMessage::stringClientData = "Sample Client Data (an MString object)";

MStatus userMessage::doIt( const MArgList& args )
{
	MStatus status = MS::kSuccess;

	MArgDatabase argData(syntax(), args);

	if (argData.isFlagSet(deregisterFlag))
	{
		MString event;
		argData.getFlagArgument(deregisterFlag, 0, event);
		status = MUserEventMessage::deregisterUserEvent(event);
	}
	else if (argData.isFlagSet(registerFlag))
	{
		// Register the new event and add two fixed callbacks to it.
		MString event;
		argData.getFlagArgument(registerFlag, 0, event);
		if (!MUserEventMessage::isUserEvent(event)) {
			status = MUserEventMessage::registerUserEvent(event);
			if (status == MS::kSuccess)
			{
				MUserEventMessage::addUserEventCallback(event,userCallback1,(void*) &stringClientData,&status);
				MUserEventMessage::addUserEventCallback(event,userCallback2,(void*) &stringClientData,&status);
			}
		}
	}
	else if (argData.isFlagSet(postFlag))
	{
		MString event;
	 	argData.getFlagArgument(postFlag, 0, event);
		status = MUserEventMessage::postUserEvent(event);
	}
	else if (argData.isFlagSet(testFlag))
	{
		runTests();
	}

	return status;
}


void* userMessage::creator() 
{	
	return new userMessage;
}

MSyntax userMessage::newSyntax() {
	MSyntax syntax;
	syntax.addFlag( postFlag, postLongFlag, MSyntax::kString );
	syntax.addFlag( registerFlag, registerLongFlag, MSyntax::kString );
	syntax.addFlag( deregisterFlag, deregisterLongFlag, MSyntax::kString );
	syntax.addFlag( testFlag, testLongFlag );
	return syntax;
}

void userMessage::userCallback1(void* clientData) {
	MGlobal::displayInfo("Entered userMessage::userCallback1");
	if (clientData != 0)
	{
		MString receivedDataMsg(MString("Received data: ") + *((MString*) clientData));
		MGlobal::displayInfo(receivedDataMsg);
	}
}

void userMessage::userCallback2(void* clientData) {
	MGlobal::displayInfo("Entered userMessage::userCallback2");
	if (clientData != 0)
	{
		MString receivedDataMsg(MString("Received data: ") + *((MString*) clientData));
		MGlobal::displayInfo(receivedDataMsg);
	}
}

void userMessage::runTests() {

	MStatus status;

	// Test 1: Try to register callback for nonexistent event

	MGlobal::displayInfo("Starting Test 1");

	MUserEventMessage::addUserEventCallback("TestEvent",userCallback1,0,&status);
	if (!status) {
		MGlobal::displayInfo("Test 1 passed");
	}
	else {
		MGlobal::displayInfo("Test 1 failed");
	}

	// Test 2: Register and deregister an event 
	// - Expected output: Entered userMessage::userCallback1

	MGlobal::displayInfo("Starting Test 2");

	MUserEventMessage::registerUserEvent("TestEvent");
	MUserEventMessage::addUserEventCallback("TestEvent",userCallback1,0,&status);
	MUserEventMessage::postUserEvent("TestEvent");
	MUserEventMessage::deregisterUserEvent("TestEvent");

	// Test 3: The event should be gone
	MGlobal::displayInfo("Starting Test 3");
	MUserEventMessage::addUserEventCallback("TestEvent",userCallback1,0,&status);
	if (!status) {
		MGlobal::displayInfo("Test 3 passed");
	}
	else {
		MGlobal::displayInfo("Test 3 failed");
	}

	// Test 4: Try adding multiple callbacks to an event
	// Expected output: Entered userMessage::userCallback1
	//					Entered userMessage::userCallback2

	MGlobal::displayInfo("Starting Test 4");

	MUserEventMessage::registerUserEvent("TestEvent");
	MUserEventMessage::addUserEventCallback("TestEvent",userCallback1,0,&status);
	MUserEventMessage::addUserEventCallback("TestEvent",userCallback2,0,&status);
	MUserEventMessage::postUserEvent("TestEvent");
	MUserEventMessage::deregisterUserEvent("TestEvent");
   
	// Test 5: Try adding and posting to multiple events
	// Expected output: Posting first event
	//					Entered userMessage::userCallback1
	//					Entered userMessage::userCallback2
	//					Posting second event
	//					Entered userMessage::userCallback1
	//					Entered userMessage::userCallback2

	MGlobal::displayInfo("Starting Test 5");

	MUserEventMessage::registerUserEvent("TestEvent");
	MUserEventMessage::registerUserEvent("TestEvent2");
	MUserEventMessage::addUserEventCallback("TestEvent",userCallback1,0,&status);
	MUserEventMessage::addUserEventCallback("TestEvent",userCallback2,0,&status);
	MUserEventMessage::addUserEventCallback("TestEvent2",userCallback1,0,&status);
	MUserEventMessage::addUserEventCallback("TestEvent2",userCallback2,0,&status);
	MGlobal::displayInfo("Posting first event");
	MUserEventMessage::postUserEvent("TestEvent");
	MGlobal::displayInfo("Posting second event");
	MUserEventMessage::postUserEvent("TestEvent2");
	MUserEventMessage::deregisterUserEvent("TestEvent");
	MUserEventMessage::deregisterUserEvent("TestEvent2");
 
	MGlobal::displayInfo("Completed all tests");
}


// standard initialize and uninitialize functions

MStatus initializePlugin(MObject obj)
{
	// Version number may need to change in the future
	//
	MFnPlugin pluginFn(obj, PLUGIN_COMPANY, "6.0");

	MStatus status;
	status = pluginFn.registerCommand("userMessage", userMessage::creator, userMessage::newSyntax);

	if( !status)
		status.perror("register Command failed");

	return status;
}


MStatus uninitializePlugin ( MObject obj )
{
	MFnPlugin pluginFn(obj);
	MStatus status = MS::kSuccess;
	
	//remove call backs
	MUserEventMessage::deregisterUserEvent("TestFoo1");
	MUserEventMessage::deregisterUserEvent("TestFoo2");

	status = pluginFn.deregisterCommand("userMessage");
	
	return status;
}

