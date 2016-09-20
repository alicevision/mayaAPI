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

#include <maya/MPxDragAndDropBehavior.h>

class hwPhongShaderBehavior : public MPxDragAndDropBehavior
{

	public:

	hwPhongShaderBehavior();
	virtual ~hwPhongShaderBehavior();

	virtual bool shouldBeUsedFor(MObject &sourceNode, MObject &destinationNode,
								 MPlug &sourcePlug, MPlug &destinationPlug);

	virtual MStatus connectNodeToNode(MObject &sourceNode, 
									  MObject &destinationNode, bool force );

	virtual MStatus connectNodeToAttr(MObject &sourceNode,
									  MPlug &destinationPlug, bool force );

	virtual MStatus connectAttrToNode(MPlug &sourceNode, 
									  MObject &destinationNode, bool force );

	virtual MStatus connectAttrToAttr(MPlug &sourceNode,
									  MPlug &destinationPlug, bool force );

	static void *creator();

	protected:

	MObject findShadingEngine(MObject &node);

	private:
};

