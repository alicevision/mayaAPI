//-
// ==========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

//============================= DESCRIPTION =================================
//	This is an example of how to write an API spring node. 
//
//===========================================================================

#include <maya/MIOStream.h>
#include <math.h>

#include "simpleSpring.h"
#include <maya/MFnDependencyNode.h>
#include <maya/MFnNumericAttribute.h>


//=================================================================
// Implement your new attributes here if your node has.
// Here is an example code.
//=================================================================
MObject simpleSpring::aSpringFactor;


MTypeId simpleSpring::id( 0x80017 );


simpleSpring::simpleSpring()
:	factor(0.0)
{
}


simpleSpring::~simpleSpring()
{
}


void *simpleSpring::creator()
{
    return new simpleSpring;
}


MStatus simpleSpring::initialize()
//
//	Descriptions:
//		Initialize the node, attributes.
//
{
	MStatus status;

	MFnNumericAttribute numAttr;

	//=================================================================
	// Initialize your new attributes here. Here is an example.
	//=================================================================
	aSpringFactor = numAttr.create("springFactor","sf",MFnNumericData::kDouble);
	numAttr.setDefault( 1.0 );
	numAttr.setKeyable( true );
	status = addAttribute( aSpringFactor );
	McheckErr(status, "ERROR adding aSpringFactor attribute.\n");

	return( MS::kSuccess );
}


MStatus simpleSpring::compute(const MPlug& /*plug*/, MDataBlock& block)
//
//	Descriptions:
//		In this simple example, do nothing in this method. But get the
//		spring factor here for "applySpringLaw" to compute output force.
//
//	Note: always let this method return "kUnknownParameter" so that 
//  "applySpringLaw" can be called when Maya needs to compute spring force.
//
//	It is recommended to only override compute() to get user defined
//  attributes.
//
{
	// Get spring factor,
	//
	factor = springFactor( block );

	// Note: return "kUnknownParameter" so that Maya spring node can
	// compute spring force for this plug-in simple spring node.
	//
	return( MS::kUnknownParameter );
}


MStatus simpleSpring::applySpringLaw
(
	double /*stiffness*/, double /*damping*/, double restLength,
	double /*endMass1*/, double /*endMass2*/,
	const MVector &endP1, const MVector &endP2,
	const MVector &/*endV1*/, const MVector &/*endV2*/,
	MVector &forceV1, MVector &forceV2
)
//
//	Descriptions:
//		In this overridden method, the attribute, aSpringFactor, is used
//		to compute output force with a simple spring law.
//
//		F = - factor * (L - restLength) * Vector of (endP1 - endP2).
//
{
	MVector distV = endP1 - endP2;
	double L = distV.length();
	distV.normalize();

	double F = factor * (L - restLength);
	forceV1 = - F * distV;
	forceV2 = - forceV1;

	return( MS::kSuccess );
}


MStatus initializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "3.0", "Any");

	status = plugin.registerNode( "simpleSpring", simpleSpring::id,
							&simpleSpring::creator, &simpleSpring::initialize,
							MPxNode::kSpringNode );
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	return status;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj);

	status = plugin.deregisterNode( simpleSpring::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	return status;
}

