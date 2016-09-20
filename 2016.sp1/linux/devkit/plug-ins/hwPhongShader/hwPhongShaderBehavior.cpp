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

#include <maya/MIOStream.h>

#include <maya/MGlobal.h>
#include <maya/MPlugArray.h>
#include <maya/MFnDagNode.h>
#include <maya/MObjectArray.h>

#include "hwPhongShaderBehavior.h"

// Set VERBOSE to 0 to suppress diagnostic outputs. Set it to 1 to
// display when the override methods are called.

#define VERBOSE 0

//
//	Description:
//		Constructor
//
hwPhongShaderBehavior::hwPhongShaderBehavior()
{
}

//
//	Description:
//		Destructor
//
hwPhongShaderBehavior::~hwPhongShaderBehavior()
{
}

//
//	Description:
//		Returns a new instance of this class
//
void *hwPhongShaderBehavior::creator()
{
	return new hwPhongShaderBehavior;
}

//
//	Description:
//		Overloaded function from MPxDragAndDropBehavior
//	this method will return true if it is going to handle the connection
//	between the two nodes given.
//
///////////////////////////////////////////////////////////////////////////////
bool hwPhongShaderBehavior::shouldBeUsedFor(
	MObject & sourceNode, 
	MObject & destinationNode,
	MPlug   & /* sourcePlug */,
	MPlug   & /* destinationPlug */)
{
	bool result = false;

	// Handle dropping a hw shader on a maya shader
	//
	if ((MFnDependencyNode(sourceNode).typeName() == "hwPhongShader") &&
		destinationNode.hasFn(MFn::kLambert))
		result = true;

#if VERBOSE
	cerr << "shouldBeUsedFor "<<MFnDependencyNode(sourceNode).name().asChar() 
		 << " " << MFnDependencyNode(destinationNode).name().asChar()  
		 << (result?" true\n":" false\n");
#endif

	return result;
}

//
// Connect source plug to destination.
//
///////////////////////////////////////////////////////////////////////////////
static MStatus connectAttr(
	const MPlug &srcPlug, 
	const MPlug &destPlug, 
	bool		force)
{
	MStatus result = MS::kFailure;

	if(!srcPlug.isNull() && !destPlug.isNull())
	{
		MString cmd = "connectAttr ";
		if (force) cmd += "-f ";
		cmd += srcPlug.name() + " " + destPlug.name();
		result = MGlobal::executeCommand(cmd);

#if VERBOSE
		cerr << cmd.asChar()<<"\n";
#endif
	}
	return result;
}

//
//	Description:
//		Overloaded function from MPxDragAndDropBehavior
//	this method will handle the connection between the hwPhongShader
//	and the shader it is assigned to as well as any meshes that it is
//	assigned to.
//
///////////////////////////////////////////////////////////////////////////////
MStatus hwPhongShaderBehavior::connectNodeToNode( 
	MObject & sourceNode, 
	MObject & destinationNode, 
	bool      force )
{
	MStatus result = MS::kFailure;
	MFnDependencyNode src(sourceNode);

	if ((src.typeName() == "hwPhongShader") &&
		destinationNode.hasFn(MFn::kLambert))
	{
		MFnDependencyNode dest(destinationNode);
		result = connectAttr(src.findPlug("outColor"), 
							 dest.findPlug("hardwareShader"), force);
	}

#if VERBOSE
	if (result != MS::kSuccess)
		cerr << "connectNodeToNode "<<src.name().asChar() 
			 << " " << MFnDependencyNode(destinationNode).name().asChar()  
			 << " failed\n";
#endif

	return result;
}

//
//	Description:
//		Overloaded function from MPxDragAndDropBehavior
//	this method will assign the correct output from the hwPhong shader 
//	onto the given attribute.
//
///////////////////////////////////////////////////////////////////////////////
MStatus hwPhongShaderBehavior::connectNodeToAttr( 
	MObject & sourceNode,
	MPlug   & destinationPlug,
	bool      force )

{
	MStatus result = MS::kFailure;
	MFnDependencyNode src(sourceNode);

	//if we are dragging from a hwPhongShader
	//to a shader than connect the outColor
	//plug to the plug being passed in
	//	
	if ((src.typeName() == "hwPhongShader") &&
		destinationPlug.node().hasFn(MFn::kLambert))
	{
		result = connectAttr(src.findPlug("outColor"), destinationPlug, force);
	}

#if VERBOSE
	if (result != MS::kSuccess)
		cerr << "connectNodeToAttr "<<src.name().asChar() 
			 << " " << destinationPlug.name().asChar()  
			 << " failed\n";
#endif

	return result;
}

//
//
///////////////////////////////////////////////////////////////////////////////
MStatus hwPhongShaderBehavior::connectAttrToNode( 
	MPlug   & sourcePlug, 
	MObject & destinationNode,
	bool      force )
{
	MStatus result = MS::kFailure;
	MObject sourceNode = sourcePlug.node();
	MFnDependencyNode src(sourceNode);

	if ((src.typeName() == "hwPhongShader") &&
		destinationNode.hasFn(MFn::kLambert))
	{
		MFnDependencyNode dest(destinationNode);
		result = connectAttr(sourcePlug, dest.findPlug("hardwareShader"),
							 force);
	}

#if VERBOSE
	if (result != MS::kSuccess)
		cerr << "connectNodeAttrToNode "<<sourcePlug.name().asChar() 
			 << " " << MFnDependencyNode(destinationNode).name().asChar()  
			 << " failed\n";
#endif

	return result;
}

//
//
///////////////////////////////////////////////////////////////////////////////
MStatus hwPhongShaderBehavior::connectAttrToAttr( 
	MPlug & sourcePlug, 
	MPlug & destinationPlug, 
	bool    force )
{
#if VERBOSE
	cerr << "In connectAttrToAttr "<<sourcePlug.name().asChar() 
		 << " " << destinationPlug.name().asChar()
		 << "\n";
#endif

	return connectAttr(sourcePlug, destinationPlug, force);
}
