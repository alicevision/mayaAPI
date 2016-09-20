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

#include <math.h>

#include <maya/MPxNode.h>
#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFloatVector.h>
#include <maya/MFnPlugin.h>

//
// DESCRIPTION:
///////////////////////////////////////////////////////

class DispNode : public MPxNode
{
	public:
                  DispNode();
	virtual      ~DispNode();

	virtual MStatus compute( const MPlug&, MDataBlock& );
	virtual void    postConstructor();

	static  void *  creator();
	static  MStatus initialize();

	static  MTypeId id;

	private:

	static MObject aColor;
	static MObject aInputValue;
	static MObject aOutColor;
	static MObject aOutTransparency;
	static MObject aOutDisplacement;
};

MTypeId DispNode::id( 0x81011 );

MObject DispNode::aColor;
MObject DispNode::aInputValue;
MObject DispNode::aOutColor;
MObject DispNode::aOutTransparency;
MObject DispNode::aOutDisplacement;

void DispNode::postConstructor( )
{
	setMPSafe(true);
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
DispNode::DispNode()
{
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
DispNode::~DispNode()
{
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
void* DispNode::creator()
{
    return new DispNode();
}



//
// DESCRIPTION:
///////////////////////////////////////////////////////
MStatus DispNode::initialize()
{
	MFnNumericAttribute nAttr; 

	// Inputs
	aColor = nAttr.createColor( "color", "c" );
	CHECK_MSTATUS(nAttr.setKeyable(true));
	CHECK_MSTATUS(nAttr.setStorable(true));
	CHECK_MSTATUS(nAttr.setDefault(1.0f, 1.0f, 1.0f));

	aInputValue = nAttr.create( "factor", "f", MFnNumericData::kFloat);
	CHECK_MSTATUS(nAttr.setKeyable(true));
	CHECK_MSTATUS(nAttr.setStorable(true));
    CHECK_MSTATUS(nAttr.setDefault(1.0f));

	// Outputs

    aOutColor  = nAttr.createColor( "outColor", "oc" );
    CHECK_MSTATUS(nAttr.setStorable(false));
    CHECK_MSTATUS(nAttr.setHidden(false));
    CHECK_MSTATUS(nAttr.setReadable(true));
    CHECK_MSTATUS(nAttr.setWritable(false));

    aOutTransparency  = nAttr.createColor( "outTransparency", "ot" );
    CHECK_MSTATUS(nAttr.setStorable(false));
    CHECK_MSTATUS(nAttr.setHidden(false));
    CHECK_MSTATUS(nAttr.setReadable(true));
    CHECK_MSTATUS(nAttr.setWritable(false));

    aOutDisplacement = nAttr.create( "displacement", "od", 
									 MFnNumericData::kFloat);
    CHECK_MSTATUS(nAttr.setStorable(false));
    CHECK_MSTATUS(nAttr.setHidden(false));
    CHECK_MSTATUS(nAttr.setReadable(true));
    CHECK_MSTATUS(nAttr.setWritable(false));


    CHECK_MSTATUS(addAttribute(aColor));
    CHECK_MSTATUS(addAttribute(aInputValue));
    CHECK_MSTATUS(addAttribute(aOutColor));
    CHECK_MSTATUS(addAttribute(aOutTransparency));
    CHECK_MSTATUS(addAttribute(aOutDisplacement));

    CHECK_MSTATUS(attributeAffects (aColor, aOutColor));
    CHECK_MSTATUS(attributeAffects (aColor, aOutDisplacement));

    return MS::kSuccess;
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
MStatus DispNode::compute(
const MPlug&      plug,
      MDataBlock& block ) 
{ 
    if ((plug == aOutColor) || (plug.parent() == aOutColor) ||
		(plug == aOutDisplacement))
	{
		MFloatVector resultColor(0.0,0.0,0.0);

		MFloatVector&  InputColor = block.inputValue( aColor ).asFloatVector();
		float MultValue = block.inputValue( aInputValue ).asFloat();

		resultColor = InputColor;

		float scalar = resultColor[0] + resultColor[1] + resultColor[2];
		if (scalar != 0.0) {
			scalar /= 3.0;
			scalar *= MultValue;
		}

		// set ouput color attribute
		MDataHandle outColorHandle = block.outputValue( aOutColor );
		MFloatVector& outColor = outColorHandle.asFloatVector();
		outColor = resultColor;
		outColorHandle.setClean();

		MDataHandle outDispHandle = block.outputValue( aOutDisplacement );
		float& outDisp = outDispHandle.asFloat();
		outDisp = scalar;
		outDispHandle.setClean( );
	}
	else if ((plug == aOutTransparency) || (plug.parent() == aOutTransparency))
	{
		// set output transparency to be opaque
		MFloatVector transparency(0.0,0.0,0.0);
		MDataHandle outTransHandle = block.outputValue( aOutTransparency );
		MFloatVector& outTrans = outTransHandle.asFloatVector();
		outTrans = transparency;
		outTransHandle.setClean( );
	}
	else
		return MS::kUnknownParameter;

    return MS::kSuccess;
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
MStatus initializePlugin( MObject obj )
{
   const MString UserClassify( "shader/displacement" );

   MFnPlugin plugin( obj, PLUGIN_COMPANY, "4.5", "Any");
   CHECK_MSTATUS( plugin.registerNode( "dispNodeExample", DispNode::id, 
                         DispNode::creator, DispNode::initialize,
                         MPxNode::kDependNode, &UserClassify ) );

   return MS::kSuccess;
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
MStatus uninitializePlugin( MObject obj )
{
   MFnPlugin plugin( obj );
   CHECK_MSTATUS( plugin.deregisterNode( DispNode::id ) );

   return MS::kSuccess;
}
