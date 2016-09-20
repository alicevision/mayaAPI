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

//
//	Class:	particleAttrNode
//
//	Author:	Lonnie Li
//
//	Description:
//
//		particleAttrNode is an example node that extends the MPxParticleAttributeMapperNode
//	type. These nodes allow us to define custom nodes to map per-particle attribute data
//	to a particle shape.
//
//		In this particular example, we have defined two operations:
//
//		- compute2DTexture()
//		- computeMesh()
//
//		compute2DTexture() replicates the internal behaviour of Maya's internal 'arrayMapper'
//	node at the API level. Given an input texture node and a U/V coord per particle input,
//	this node will evaluate the texture at the given coordinates and map the result back
//	to the outColorPP or outValuePP attributes. See the method description for compute2DTexture()
//	for details on how to setup a proper attribute mapping graph.
//
//		computeMesh() is a user-defined behaviour. It is called when the 'computeNode' attribute
//	is connected to a polygonal mesh. From there, given a particle count, it will map the
//	object space vertex positions of the mesh to a user-defined 'outPositions' vector attribute.
//	This 'outPositions' attribute can then be connected to a vector typed, per-particle attribute
//	on the shape to drive. In our particular example we drive the particle shape's 'rampPosition'
//	attribute. For further details, see the method description for computeMesh() to setup
//	this example.
//

#include <maya/MPxParticleAttributeMapperNode.h>
#include <maya/MTypeId.h>

class MPlug;
class MDataBlock;

class particleAttrNode : public MPxParticleAttributeMapperNode
{
public:
						particleAttrNode();
	virtual				~particleAttrNode();

	static  void*		creator();
	static  MStatus		initialize();

	virtual MStatus		compute( const MPlug& plug, MDataBlock& block );

protected:
	MStatus				compute2DTexture( const MPlug& plug, MDataBlock& block );
	MStatus				computeMesh( const MPlug& plug, MDataBlock& block );

public:
	static  MTypeId		id;
	static	MObject		outPositionPP;
	static	MObject		particleCount;

private:
};

