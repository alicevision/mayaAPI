//
//  File: tweakMetadataNode.h
//
//  Description:
// 		Example implementation of a node which takes in a mesh geometry
//      and modifies the metadata inside it in a manner
//      described by the "operation" attribute.
//        
//      To test the node, use the following Python commands
/*
		import maya.cmds as cmds
		cmds.loadPlugin( 'metadataPlugin' )
		tweakStruct = cmds.dataStructure( format='raw',
									asString='name=TweakStructure:int32=value' )
		tweak = cmds.createNode( 'tweakMetadata' )
		(xform, creator) = cmds.polyPlane( name='testPlane' )
		shape = cmds.listRelatives( xform, children=True )[0]
		cmds.connectAttr( '%s.outMesh' % creator, '%s.inMesh' % tweak )
		cmds.disconnectAttr( '%s.outMesh' % creator, '%s.inMesh' % shape )
		cmds.connectAttr( '%s.outMesh' % tweak, '%s.inMesh' % shape )
		cmds.setAttr( '%s.operation' % tweak, 1 )
		cmds.exportMetadata( shape )
		//
		// Output should show a set of metadata channels with random numbers
		//
		// Play around with the subdivision on "polyPlane" to generate
		// different metadata as component counts change.
		//
		// Notice that every evaluation causes a different set of random
		// numbers to be generated. To keep consistency with your metadata
		// you have to follow the DG principle of "the same inputs will
		// produce the same outputs". Try adding a random seed to this
		// example node to make the random numbers reproducible.
		//
*/

#include <maya/MPxNode.h> 

class tweakMetadataNode : public MPxNode
{
public:
				tweakMetadataNode	();
	virtual		~tweakMetadataNode	();

	static  void*		creator		();
	static  MStatus		initialize	();
	static  const char*	nodeName		();

	// Tweak function
	//
    virtual MStatus   	compute		( const MPlug&, MDataBlock&);

	// Types of operations this node can perform
	//
	enum eOpTypes { kOpNone, kOpRandomize, kOpFill, kOpDouble };

public:
	// Node attributes
	static  MObject	aOperation;
	static  MObject	aInMesh;
	static  MObject	aOutMesh;

	// This has to be globally unique or it could cause problems with file I/O
	static  MTypeId	id;

private:
};

//-
// ==================================================================
// Copyright 2012 Autodesk, Inc.  All rights reserved.
// 
// This computer source code  and related  instructions and comments are
// the unpublished confidential and proprietary information of Autodesk,
// Inc. and are  protected  under applicable  copyright and trade secret
// law. They may not  be disclosed to, copied or used by any third party
// without the prior written consent of Autodesk, Inc.
// ==================================================================
//+
