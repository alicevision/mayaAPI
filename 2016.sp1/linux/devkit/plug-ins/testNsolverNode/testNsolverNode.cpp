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

#include "testNsolverNode.h"
#include <maya/MIOStream.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MGlobal.h>
#include <maya/MTime.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MFnComponentListData.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MDagPath.h>
#include <maya/MPoint.h>
#include <maya/MFloatPointArray.h>
#include <maya/MItMeshVertex.h>
#include <maya/MFnPlugin.h>
#include <math.h>

/*

This example shows a custom solver at work.  Two nCloth objects are created,
one is disconnected from the default nucleus solver, and hooked to
this custom solver node. This node creates an MnSolver object, and gives
it the same properties as the default maya settings for the nucleus
solver, so that it should solve any connected objects the same way

To be useable with standard maya nThing nodes, a custom solver needs
these 3 array attributes:

-startState     To be connected from the cloth objects to the solver
-currentState   To be connected from the cloth objects to the solver
-nextState      To be connected from the solver object to the cloth objects

and  a 4th attribute that is the current time.

In a more complete solver node, you would want to connect constraint objects
as well, but that's a topic for a different example.

We've hardcoded a lot of the settings that would normally be attributes, 
but only because we want to hilight the ones that are required for the solver.

At the start frame (which we hardcoded to 1), we always rebuild the solver
relationships from scratch. We don't want to mess with the solver 
relationships too much while it's running, so we wait until the next rewind.
In this simple example, that means removing all the objects and collisions
from the solver, and re-adding whatever is currently connected.

When a solve is needed, the first nThing to get a refresh will pull on the
nextState attribute.  When the solver gets a pull on one next state element,
it wil pull on either the all the currentState attribute elements,
or the all the startStates, depending on the current time.
This pull forces all the nThings to update their current state,
to reflect the effects of any external animation.
Once all the nThings have been updated, the solve() call on the solver
will update all the objects that have been assigned to that solver.

After that, we mark the plug clean to indicate that the solve is completed.
You may notive that we're not actually passing back any data or an updated
MnObject, This is because when we added an object to the MnSolver at the start frame,
it actually got a pointer to the internal data of the source object, and
each frame, it updates it directly. The rest of the connections are there so that
we can force synchronization, and make sure that any external animation is
updated.

A motivated individual could add some current and start state connections for passive
objects, and just skip the next state connections on those.

Below is some example code to test this plugin:

//**************************************************************************
//Note: Before running this code, make sure the plugin testNsolverNode is loaded!
//**************************************************************************
global proc setupCustomSolverScene()
{

    file -f -new;

    string $pPlane1[] = `polyPlane -w 5 -h 5 -sx 10 -sy 10 -ax 0 1 0 -cuv 2 -ch 1`;
    move -r -10 0 0;
    createNCloth 0;

    string $pPlane2[] = `polyPlane -w 5 -h 5 -sx 10 -sy 10 -ax 0 1 0 -cuv 2 -ch 1`;
    createNCloth 0;

    //Hookup plane2 (the cloth object created for plane2 is named nClothShape2) to our custom solver instead.

    //First, disconnect it from the default nucleus solver:
    disconnectAttr nClothShape2.currentState nucleus1.inputActive[1];
    disconnectAttr nClothShape2.startState nucleus1.inputActiveStart[1];
    disconnectAttr nucleus1.outputObjects[1] nClothShape2.nextState;
    disconnectAttr nucleus1.startFrame nClothShape2.startFrame;

    //create our custom solver:
    createNode testNsolverNode;

    //Hookup plane2 to our custom solver:
    connectAttr testNsolverNode1.nextState[0] nClothShape2.nextState;
    connectAttr nClothShape2.currentState testNsolverNode1.currentState[0];
    connectAttr nClothShape2.startState testNsolverNode1.startState[0];
    connectAttr time1.outTime testNsolverNode1.currentTime;
}

*/

const MTypeId testNsolverNode::id( 0x85005 );


#include <maya/MFnNObjectData.h>
#include <maya/MnCloth.h>

MObject testNsolverNode::startState;
MObject testNsolverNode::currentState;
MObject testNsolverNode::nextState;
MObject testNsolverNode::currentTime;


inline void statCheck( MStatus stat, MString msg )
{
	if ( !stat )
	{
		cout<<msg<<"\n";
	}
}

testNsolverNode::testNsolverNode()
{
	solver.createNSolver();
	solver.setGravity(9.8f);
	solver.setStartTime((float)(1.0/24.0));
}

MStatus testNsolverNode::compute(const MPlug &plug, MDataBlock &data)
{
	MStatus stat;

	if ( plug == nextState )
	{
	
		//get the value of the currentTime 
		MTime currTime = data.inputValue(currentTime).asTime();
		float solveTime = (float)currTime.as(MTime::kSeconds);

		MObject inputData;
		// start frame setup
		if(currTime.value() <= 1.0) {
			// actually, there will be multiple pulls on next state at the start frame
			// if there are multiple nCloth objects - you'll need to check and make sure that 
			// you only initialize once, or take a performance hit
			// you could also re-initialize if a connection is made or broke at the start frame
			MArrayDataHandle multiDataHandle = data.inputArrayValue(startState);
			int count =  multiDataHandle.elementCount();
			cerr << "multi handle elementcount = " << count << "\n";
			solver.removeAllCollisions();
			for (int i = 0; i < count; i++) {
			  // yes, I suppose  you could be more careful about sparse indices
			  // and use next() to iterate, but this example is more about
			  // using MnSolver than about careful use of multis
				multiDataHandle.jumpToElement(i);
				inputData = multiDataHandle.inputValue().data();
		
				MFnNObjectData inputNData(inputData);		
				MnCloth * nObj = NULL;
				inputNData.getObjectPtr(nObj);	

				// remove and re-add all objects at start frame in case 
				// objects have been added or removed

				solver.removeNObject(nObj);
				solver.addNObject(nObj);
				delete nObj;	

			}
			solver.makeAllCollide();
		} else {

			MArrayDataHandle multiDataHandle = data.inputArrayValue(currentState);
			int count =  multiDataHandle.elementCount();
			for (int i = 0; i < count; i++) {
				multiDataHandle.jumpToElement(i);
				inputData =multiDataHandle.inputValue().data();
		
				MFnNObjectData inputNData(inputData);		
				MnCloth * nObj = NULL;
				inputNData.getObjectPtr(nObj);
				delete nObj;	
			}

		}

		solver.setGravity(9.8f);
		solver.setGravityDir(0.0f, -1.0f, 0.0f);
		solver.setAirDensity(1.0f);
		solver.setWindSpeed(0.0f);
		solver.setWindDir(0.0f, 1.0f, 0.0f);
		solver.setWindNoiseIntensity(0.0f);
		solver.setDisabled(false);
		solver.setSubsteps(3);
		solver.setMaxIterations(4);

	
		solver.solve(solveTime);
		data.setClean(plug);
	}
	else if ( plug == currentState )
	{	
		data.setClean(plug);

	}
	else if (plug == startState) {
		data.setClean(plug);
	}
	else {
		stat = MS::kUnknownParameter;
	}
	return stat;
}



MStatus testNsolverNode::initialize()
{
	MStatus stat;

	MFnTypedAttribute tAttr;
	startState = tAttr.create("startState", "sst", MFnData::kNObject, MObject::kNullObj, &stat );

	statCheck(stat, "failed to create startState");
	tAttr.setWritable(true);
	tAttr.setStorable(true);
	tAttr.setHidden(true);
	tAttr.setArray(true);


	currentState = tAttr.create("currentState", "cst", MFnData::kNObject, MObject::kNullObj, &stat );

	statCheck(stat, "failed to create currentState");
	tAttr.setWritable(true);
	tAttr.setStorable(true);
	tAttr.setHidden(true);
	tAttr.setArray(true);
	

	nextState = tAttr.create("nextState", "nst", MFnData::kNObject, MObject::kNullObj, &stat );

	statCheck(stat, "failed to create nextState");
	tAttr.setWritable(true);
	tAttr.setStorable(true);
	tAttr.setHidden(true);
	tAttr.setArray(true);

   	MFnUnitAttribute uniAttr;
	currentTime = uniAttr.create( "currentTime", "ctm" , MFnUnitAttribute::kTime,  0.0, &stat  );    	

	addAttribute(startState);
	addAttribute(currentState);
	addAttribute(nextState);
	addAttribute(currentTime);
	
	attributeAffects(startState, nextState);
	attributeAffects(currentState, nextState);	
	attributeAffects(currentTime, nextState);	

	return MStatus::kSuccess;
}

MStatus initializePlugin ( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin(obj, "Autodesk - nCloth Prototype 4", "8.5", "Any");	

	status = plugin.registerNode ( "testNsolverNode", testNsolverNode::id,testNsolverNode ::creator, testNsolverNode::initialize );

	if ( !status )
	{
		status.perror("registerNode");
		return status;
	}

	return status;
}

MStatus uninitializePlugin ( MObject obj )
{
	MStatus	  status;
	MFnPlugin plugin(obj);

	status = plugin.deregisterNode(testNsolverNode::id);
	if ( !status )
	{
		status.perror("deregisterNode");
		return status;
	}

	return status;
}
