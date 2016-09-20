//-
// ==========================================================================
// Copyright 2011 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

// Description:
//	This example demonstrates how to use left pad X and Y from
//	a game input device to modify the translate attribute of a node. The
//	MEL example code demonstrates how a poly cube can be moved
//	in X and Y.
//
//	There is an attribute updateTranslateXZ that controls if
//	the example will map game input(x,y) to Maya(x,0,z) or
//	Maya(x,y,0).
//
//	NOTE: Windows only example that requires the Direct X
// SDK to be installed in order to build. In addition, the
// game input device driver must be installed also.
//

/*
	// MEL:
	loadPlugin gameInputDevice;
	string $node = `createNode gameInputDevice`;
	string $cube[] = `polyCube`;
	connectAttr ( $node + ".outputTranslate" ) ( $cube[0] + ".translate" );
	setAttr ( $node + ".live" ) 1;
*/

#ifdef _WIN32
#ifndef IPV6STRICT
#define IPV6STRICT
#endif
#include <winsock2.h>
#include <windows.h>
#include <XInput.h>
#pragma comment(lib, "XInput.lib")
#endif // _WIN32

#include <maya/MFnPlugin.h>
#include <maya/MTypeId.h>

#include <api_macros.h>
#include <maya/MIOStream.h>

#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MPxThreadedDeviceNode.h>

class gameInputDeviceNode : public MPxThreadedDeviceNode
{

public:
						gameInputDeviceNode();
	virtual 			~gameInputDeviceNode();
	
	virtual void		postConstructor();
	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );
	virtual void		threadHandler();
	virtual void		threadShutdownHandler();

	static void*		creator();
	static MStatus		initialize();

public:

	static MObject		outputTranslate;
	static MObject 		outputTranslateX;
	static MObject		outputTranslateY;
	static MObject 		outputTranslateZ;
	// Boolean attribute for deciding if we are going
	// to update XZ or XY. Only two inputs from the
	// gameInput in this example
	static MObject		updateTranslateXZ;

	static MTypeId		id;

private:
};

MTypeId gameInputDeviceNode::id( 0x00081053 );
MObject gameInputDeviceNode::outputTranslate;
MObject gameInputDeviceNode::outputTranslateX;
MObject gameInputDeviceNode::outputTranslateY;
MObject gameInputDeviceNode::outputTranslateZ;
MObject gameInputDeviceNode::updateTranslateXZ;

gameInputDeviceNode::gameInputDeviceNode() 
{}

gameInputDeviceNode::~gameInputDeviceNode()
{
	destroyMemoryPools();
}

void gameInputDeviceNode::postConstructor()
{
	MObjectArray attrArray;
	attrArray.append( gameInputDeviceNode::outputTranslate );
	setRefreshOutputAttributes( attrArray );

	// we'll be reading one set of translate x,y, z's at a time
	createMemoryPools( 24, 3, sizeof(double));
}

#ifdef _WIN32
bool checkController(XINPUT_STATE& state)
{
	DWORD dwResult;
	ZeroMemory( &state, sizeof(XINPUT_STATE) );
	dwResult = XInputGetState( 0, &state );
	if( dwResult == ERROR_SUCCESS )
	{
		return true;
	}
	return false;
}
#endif

void gameInputDeviceNode::threadHandler()
{
#ifdef _WIN32
	MStatus status;
	setDone( false );
	while ( ! isDone() )
	{
		if ( ! isLive() )
			continue;

		MCharBuffer buffer;
		status = acquireDataStorage(buffer);
		if ( ! status )
			continue;

		XINPUT_STATE state;
		if ( ! checkController( state ) )
		{
			releaseDataStorage(buffer);
			continue;
		}

		beginThreadLoop();
		{
			float changeX = 0.0, changeY = 0.0;
			// Making sure we are not in the deadzone, we find
			// relative movement values.  A very simple approach
			float leftThumbX = state.Gamepad.sThumbLX;
			float leftThumbY = state.Gamepad.sThumbLY;
			if ( leftThumbX > +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  ) changeX = +1;
			if ( leftThumbX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  ) changeX = -1;
			if ( leftThumbY > +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  ) changeY = +1;
			if ( leftThumbY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  ) changeY = -1;

			double* doubleData = reinterpret_cast<double*>(buffer.ptr());
			doubleData[0] = changeX ; doubleData[1] = changeY; doubleData[2] = 0.0;
			pushThreadData( buffer );
		}
		endThreadLoop();
	}
#endif // _WIN32
	setDone( true );
}

void gameInputDeviceNode::threadShutdownHandler()
{
	setDone( true );
}

void* gameInputDeviceNode::creator()
{
	return new gameInputDeviceNode;
}

MStatus gameInputDeviceNode::initialize()
{
	MStatus status;
	MFnNumericAttribute numAttr;

	outputTranslateX = numAttr.create("outputTranslateX", "otx", MFnNumericData::kDouble, 0.0, &status);
	MCHECKERROR(status, "create outputTranslateX");
	outputTranslateY = numAttr.create("outputTranslateY", "oty", MFnNumericData::kDouble, 0.0, &status);
	MCHECKERROR(status, "create outputTranslateY");
	outputTranslateZ = numAttr.create("outputTranslateZ", "otz", MFnNumericData::kDouble, 0.0, &status);
	MCHECKERROR(status, "create outputTranslateZ");
	outputTranslate = numAttr.create("outputTranslate", "ot", outputTranslateX, outputTranslateY, 
									 outputTranslateZ, &status);
	MCHECKERROR(status, "create outputTranslate");

	updateTranslateXZ = numAttr.create( "updateTranslateXZ", "uxz", MFnNumericData::kBoolean);
	CHECK_MSTATUS( numAttr.setKeyable(true) );
	CHECK_MSTATUS( numAttr.setStorable(true) );
	CHECK_MSTATUS( numAttr.setHidden(false) );
	CHECK_MSTATUS( numAttr.setDefault(true) );

	ADD_ATTRIBUTE(outputTranslate);
	ADD_ATTRIBUTE(updateTranslateXZ);

	ATTRIBUTE_AFFECTS( live, outputTranslate);
	ATTRIBUTE_AFFECTS( frameRate, outputTranslate);
	ATTRIBUTE_AFFECTS( updateTranslateXZ, outputTranslate);

	return MS::kSuccess;
}

MStatus gameInputDeviceNode::compute( const MPlug& plug, MDataBlock& block )
{
	MStatus status;
	if( plug == outputTranslate || plug == outputTranslateX ||
		plug == outputTranslateY || plug == outputTranslateZ )
	{
		// Find the type of translation we will be doing
		bool xzUpdate  = block.inputValue( updateTranslateXZ ).asBool();

		// Access the data and update the output attribute
		MCharBuffer buffer;
		if ( popThreadData(buffer) )
		{
			// Relative data coming in
			double* doubleData = reinterpret_cast<double*>(buffer.ptr());

			MDataHandle outputTranslateHandle = block.outputValue( outputTranslate, &status );
			MCHECKERROR(status, "Error in block.outputValue for outputTranslate");

			double3& outputTranslate = outputTranslateHandle.asDouble3();
			if ( xzUpdate ) 
			{
				// XZ
				outputTranslate[0] += doubleData[0];
				outputTranslate[1] += doubleData[2];
				outputTranslate[2] -= doubleData[1];
			}
			else
			{
				// XY
				outputTranslate[0] += doubleData[0];
				outputTranslate[1] += doubleData[1];
				outputTranslate[2] += doubleData[2];
			}
			block.setClean( plug );

			releaseDataStorage(buffer);
			return ( MS::kSuccess );
		}
		else
		{
			return MS::kFailure;
		}
	}

	return ( MS::kUnknownParameter );
}

MStatus initializePlugin( MObject obj )
{
	MStatus status;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "3.0", "Any");

	status = plugin.registerNode( "gameInputDevice", 
								  gameInputDeviceNode::id,
								  gameInputDeviceNode::creator,
								  gameInputDeviceNode::initialize,
								  MPxNode::kThreadedDeviceNode );
	if( !status ) {
		status.perror("failed to registerNode gameInputDeviceNode");
	}

	return status;
}

MStatus uninitializePlugin( MObject obj )
{
	MStatus status;
	MFnPlugin plugin(obj);

	status = plugin.deregisterNode( gameInputDeviceNode::id );
	if( !status ) {
		status.perror("failed to deregisterNode gameInputDeviceNode");
	}

	return status;
}

