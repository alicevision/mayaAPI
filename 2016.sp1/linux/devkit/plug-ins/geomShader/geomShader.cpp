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

///////////////////////////////////////////////////////
//
// DESCRIPTION:
//
///////////////////////////////////////////////////////

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
class GeomNode : public MPxNode
{
	public:
                     GeomNode();
   virtual           ~GeomNode();

   virtual MStatus   compute( const MPlug&, MDataBlock& );
   virtual void      postConstructor();

   static  void *    creator();
   static  MStatus   initialize();

	//  Id tag for use with binary file format
   static  MTypeId   id;

	private:

	// Input attributes
	static MObject aPoint;
    static MObject aScale;
    static MObject aOffset;

	// Output attributes
	static MObject aOutColor;
};

// static data
MTypeId GeomNode::id( 0x81004 );

// Attributes
MObject  GeomNode::aPoint;
MObject  GeomNode::aScale;
MObject  GeomNode::aOffset;
MObject  GeomNode::aOutColor;

#define MAKE_INPUT(attr)								\
  CHECK_MSTATUS ( attr.setKeyable(true) ); 				\
  CHECK_MSTATUS ( attr.setStorable(true) );				\
  CHECK_MSTATUS ( attr.setReadable(true) ); 			\
  CHECK_MSTATUS ( attr.setWritable(true) );

#define MAKE_OUTPUT(attr)								\
  CHECK_MSTATUS (attr.setKeyable(false) );  			\
  CHECK_MSTATUS ( attr.setStorable(false));				\
  CHECK_MSTATUS ( attr.setReadable(true) ); 			\
  CHECK_MSTATUS ( attr.setWritable(false));

// DESCRIPTION:
//
///////////////////////////////////////////////////////
void GeomNode::postConstructor( )
{
	setMPSafe(true);
}

// DESCRIPTION:
//
///////////////////////////////////////////////////////
GeomNode::GeomNode()
{
}

// DESCRIPTION:
//
///////////////////////////////////////////////////////
GeomNode::~GeomNode()
{
}

// DESCRIPTION:
//
///////////////////////////////////////////////////////
void* GeomNode::creator()
{
    return new GeomNode();
}

// DESCRIPTION:
//
///////////////////////////////////////////////////////
MStatus GeomNode::initialize()
{
    MFnNumericAttribute nAttr; 

    // Input attributes

    aPoint = nAttr.createPoint( "pointObj", "p" );
    CHECK_MSTATUS ( nAttr.setStorable(false) );
    CHECK_MSTATUS ( nAttr.setHidden(true) );

    aScale = nAttr.createPoint( "scale", "s" );
    MAKE_INPUT(nAttr);
    CHECK_MSTATUS ( nAttr.setDefault(1.0f, 1.0f, 1.0f) );

    aOffset = nAttr.createPoint( "offset", "o" );
    MAKE_INPUT(nAttr);

	// Output attributes
    aOutColor  = nAttr.createColor( "outColor", "oc" );
    MAKE_OUTPUT(nAttr);

	// Add attributes to the node database.
    CHECK_MSTATUS ( addAttribute(aPoint) );
    CHECK_MSTATUS ( addAttribute(aScale) );
    CHECK_MSTATUS ( addAttribute(aOffset) );

    CHECK_MSTATUS ( addAttribute(aOutColor) );

	// All input affect the output color
    CHECK_MSTATUS ( attributeAffects (aPoint, aOutColor) );
    CHECK_MSTATUS ( attributeAffects (aScale, aOutColor) );
    CHECK_MSTATUS ( attributeAffects (aOffset,aOutColor) );

    return MS::kSuccess;
}

// DESCRIPTION:
//
///////////////////////////////////////////////////////
MStatus GeomNode::compute(
const MPlug&      plug,
      MDataBlock& block ) 
{
	// outColor or individial R, G, B channel
    if((plug != aOutColor) && (plug.parent() != aOutColor))
		return MS::kUnknownParameter;

    MFloatVector resultColor;

    MFloatVector & point  = block.inputValue( aPoint ).asFloatVector();
    MFloatVector & scale  = block.inputValue( aScale ).asFloatVector();
    MFloatVector & offset = block.inputValue( aOffset ).asFloatVector();

    // Scale + Offset original geometry position
    resultColor.x = point.x * scale.x + offset.x;
    resultColor.y = point.y * scale.y + offset.y;
    resultColor.z = point.z * scale.z + offset.z;

    // set ouput color attribute
    MDataHandle outColorHandle = block.outputValue( aOutColor );
    MFloatVector& outColor = outColorHandle.asFloatVector();
    outColor = resultColor;
    outColorHandle.setClean();

    return MS::kSuccess;
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
MStatus initializePlugin( MObject obj )
{
   const MString UserClassify( "utility/general" );

   MFnPlugin plugin( obj, PLUGIN_COMPANY, "4.5", "Any");
   CHECK_MSTATUS ( plugin.registerNode( "geomNode", GeomNode::id, 
                         GeomNode::creator, GeomNode::initialize,
                         MPxNode::kDependNode, &UserClassify ) );

   return MS::kSuccess;
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
MStatus uninitializePlugin( MObject obj )
{
   MFnPlugin plugin( obj );
   CHECK_MSTATUS ( plugin.deregisterNode( GeomNode::id ) );

   return MS::kSuccess;
}
