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
// A simple client device node that uses UDP to transfer data.
// Sample only runs on Linux.
//
// Run Maya and execute the MEL code below.  In a shell,
// run the Python code and enter 3 numbers to update the
// cube's translate. 
//

/*

// MEL:
loadPlugin udpDevice;
string $node = `createNode udpDevice`;
string $cube[] = `polyCube`;
connectAttr ( $node + ".outputTranslate" ) ( $cube[0] + ".translate" );
setAttr ( $node + ".live" ) 1;

# Python: run from a Linux command line as a Python script
import socket
clientSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
while True:
	data = raw_input("Type 3 numbers for translate(. to exit): ")
	if data <> '.':
		clientSocket.sendto(data, ("localhost",7555))
	else:
		break
clientSocket.close()

*/

#ifdef LINUX
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#endif

#include <maya/MFnPlugin.h>
#include <maya/MTypeId.h>

#include <api_macros.h>
#include <maya/MIOStream.h>

#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MPxClientDeviceNode.h>

class udpDeviceNode : public MPxClientDeviceNode
{

public:
				udpDeviceNode();
	virtual 		~udpDeviceNode();
	
	virtual void		postConstructor();
	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );
	virtual void		threadHandler( const char* serverName, const char* deviceName );
	virtual void		threadShutdownHandler();

	static void*		creator();
	static MStatus		initialize();

public:

	static MObject		outputTranslate;
	static MObject 		outputTranslateX;
	static MObject		outputTranslateY;
	static MObject 		outputTranslateZ;

	static MTypeId		id;

private:
};

MTypeId udpDeviceNode::id( 0x00081052 );
MObject udpDeviceNode::outputTranslate;
MObject udpDeviceNode::outputTranslateX;
MObject udpDeviceNode::outputTranslateY;
MObject udpDeviceNode::outputTranslateZ;

udpDeviceNode::udpDeviceNode() 
{}

udpDeviceNode::~udpDeviceNode()
{
	destroyMemoryPools();
}

void udpDeviceNode::postConstructor()
{
	MObjectArray attrArray;
	attrArray.append( udpDeviceNode::outputTranslate );
	setRefreshOutputAttributes( attrArray );

	// we'll be reading one character line of size 1024
	createMemoryPools( 1, 1024, sizeof(char));
}

void udpDeviceNode::threadHandler( const char* serverName, const char* deviceName )
{
	setDone( false );
	if ( serverName != NULL && deviceName != NULL )
		printf("udpThreadHandler: %s %s\n",serverName,deviceName);

#ifdef LINUX
	int sock;
	int bytesRead;
	socklen_t addressLength;
	struct sockaddr_in serverAddress , clientAddress;
	

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(7555);
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serverAddress.sin_zero),8);

	addressLength = sizeof(struct sockaddr);

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
	{
		return;
	}

	if (bind(sock,(struct sockaddr *)&serverAddress,
		sizeof(struct sockaddr)) == -1)
	{
		return;
	}

	MStatus status;
	MCharBuffer buffer;
	char receiveBuffer[1024];

	while ( !isDone() )
	{
		if ( ! isLive() )
			continue;
		
		// select() modifies its parameters so reset everytime
		fd_set read_set;
		FD_ZERO( &read_set );
		FD_SET( sock, &read_set );
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 500000; // 1/2 second
		if ( select( sock+1, &read_set, NULL, NULL, &tv ) == -1 )
			break;
		if ( ! FD_ISSET( sock, &read_set ) )
			continue;

		receiveBuffer[0] = 0;
		bytesRead = recvfrom(sock,receiveBuffer,1024,0,
		    (struct sockaddr *)&clientAddress, &addressLength);
		    
		const char *receivedFromServer = inet_ntoa(clientAddress.sin_addr);
		unsigned short receivedFromPort = ntohs(clientAddress.sin_port);
		
		if ( NULL == receivedFromServer )
			continue;
		
		printf("(%s , %d) connection : %s \n",receivedFromServer,receivedFromPort, receiveBuffer);

		// Simple test to make sure we are getting data from the right server
		// printf("%s %s\n",serverName, receivedFromServer);
		if ( 0 != strcmp( serverName, receivedFromServer ) )
			continue;

		// Get the storage once we have data from the server
		status = acquireDataStorage(buffer);
		if ( ! status )
			continue;

		beginThreadLoop();
		{
			receiveBuffer[bytesRead] = '\0';
			double* doubleData = reinterpret_cast<double*>(buffer.ptr());
			doubleData[0] = 0.0 ; doubleData[1] = 0.0; doubleData[2] = 0.0;
			MStringArray sa;
			MString s( receiveBuffer );
			if ( s.split( ' ', sa ) )
			{
				if ( sa.length() == 3 )
				{
					int i = 0;
					for ( i = 0; i < 3; i++ )
					{
						doubleData[i] = ( sa[i].isDouble() ? sa[i].asDouble() : 0.0 );
					}
				}	
			}
			pushThreadData( buffer );
		}
		endThreadLoop();

	}
	
	// Close the socket
	close( sock );
	
#endif // LINUX

	setDone( true );
}

void udpDeviceNode::threadShutdownHandler()
{
	setDone( true );
}

void* udpDeviceNode::creator()
{
	return new udpDeviceNode;
}

MStatus udpDeviceNode::initialize()
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
	
	ADD_ATTRIBUTE(outputTranslate);

	ATTRIBUTE_AFFECTS( live, outputTranslate);
	ATTRIBUTE_AFFECTS( frameRate, outputTranslate);
	ATTRIBUTE_AFFECTS( serverName, outputTranslate);
	ATTRIBUTE_AFFECTS( deviceName, outputTranslate);

	return MS::kSuccess;
}

MStatus udpDeviceNode::compute( const MPlug& plug, MDataBlock& block )
{
	MStatus status;
	if( plug == outputTranslate || plug == outputTranslateX ||
		plug == outputTranslateY || plug == outputTranslateZ )
	{
		MCharBuffer buffer;
		if ( popThreadData(buffer) )
		{
			double* doubleData = reinterpret_cast<double*>(buffer.ptr());

			MDataHandle outputTranslateHandle = block.outputValue( outputTranslate, &status );
			MCHECKERROR(status, "Error in block.outputValue for outputTranslate");

			double3& outputTranslate = outputTranslateHandle.asDouble3();
			outputTranslate[0] = doubleData[0];
			outputTranslate[1] = doubleData[1];
			outputTranslate[2] = doubleData[2];

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

	status = plugin.registerNode( "udpDevice", 
								  udpDeviceNode::id,
								  udpDeviceNode::creator,
								  udpDeviceNode::initialize,
								  MPxNode::kClientDeviceNode );
	if( !status ) {
		status.perror("failed to registerNode udpDeviceNode");
	}

	return status;
}

MStatus uninitializePlugin( MObject obj )
{
	MStatus status;
	MFnPlugin plugin(obj);

	status = plugin.deregisterNode( udpDeviceNode::id );
	if( !status ) {
		status.perror("failed to deregisterNode udpDeviceNode");
	}

	return status;
}

