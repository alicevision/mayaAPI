//
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

#include <maya/MIOStream.h>

#include <maya/MPxNode.h> 
#include <maya/MFnPlugin.h>
#include <maya/MString.h> 
#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MRampAttribute.h>


class exampleRampAttrNode : public MPxNode
{
public:
	exampleRampAttrNode() {};
	virtual        ~exampleRampAttrNode() {};
	static  void   *creator();
	static  MStatus initialize();
	virtual MStatus compute(const MPlug &plug, MDataBlock  &dataBlock);

	static MObject input1;
	static MObject input2;
	static MTypeId  id;

};

MTypeId exampleRampAttrNode::id( 0x81027 );

MObject exampleRampAttrNode::input1;
MObject exampleRampAttrNode::input2;

void *exampleRampAttrNode::creator()
{
   return((void *) new exampleRampAttrNode);
}

MStatus exampleRampAttrNode::initialize()
{
	MStatus stat;

	MString curveRamp("curveRamp");
	MString cvr("cvr");
	MString colorRamp("colorRamp");
	MString clr("clr");

	input1 = MRampAttribute::createCurveRamp(curveRamp, cvr);
	input2 = MRampAttribute::createColorRamp(colorRamp, clr);

	stat = addAttribute(input1);
	if(!stat)
	{
		cout << "ERROR in adding curveRamp Attribute!\n";
	}
	stat = addAttribute(input2);
	if(!stat)
	{
		cout << "ERROR in adding colorRamp Attribute!\n";
	}

	return stat;
}

MStatus exampleRampAttrNode::compute( const MPlug &plug, MDataBlock  &dataBlock ) 
{
	return(MStatus::kSuccess);
}

MStatus initializePlugin(MObject obj)
{
	MStatus   status;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "7.0", "Any");

	status = plugin.registerNode("exampleRampAttrNode", exampleRampAttrNode::id,
		   	exampleRampAttrNode::creator, exampleRampAttrNode::initialize);
	if (!status) 
	{
		status.perror("registerNode");
		return status;
	}

	return status;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus   status;
	MFnPlugin plugin(obj);

	status = plugin.deregisterNode(exampleRampAttrNode::id);
	if (!status) 
	{
		status.perror("deregisterNode");
		return status;
	}

	return status;
}
