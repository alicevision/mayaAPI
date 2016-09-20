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

#include <maya/MGlobal.h>
#include <maya/MPxNode.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h> 
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFloatVector.h>
#include <maya/MFloatPoint.h>
#include <maya/MFloatMatrix.h>
#include <maya/MFnPlugin.h>

class mySChecker : public MPxNode
{
	public:
	mySChecker();
	virtual ~mySChecker();

	virtual MStatus compute( const MPlug&, MDataBlock& );
	virtual void    postConstructor();

	static  void *  creator();
	static  MStatus initialize();

	//  Id tag for use with binary file format
	static const MTypeId id;

	private:

	// Input attributes
	static MObject aColor1;
	static MObject aColor2;
	static MObject aPlaceMat;
	static MObject aPointWorld;
    static MObject aBias;

	// Output attributes
	static MObject aOutColor;
	static MObject aOutAlpha;
};


// Static data
const MTypeId mySChecker::id( 0x8100b );

// Attributes
MObject        mySChecker::aColor1;
MObject        mySChecker::aColor2;
MObject        mySChecker::aPlaceMat;
MObject        mySChecker::aPointWorld;
MObject        mySChecker::aBias;
 
MObject        mySChecker::aOutColor;
MObject        mySChecker::aOutAlpha;

#define MAKE_INPUT(attr)								\
    attr.setKeyable(true);  attr.setStorable(true);		\
    attr.setReadable(true); attr.setWritable(true);

#define MAKE_OUTPUT(attr)								\
    attr.setKeyable(false); attr.setStorable(false);	\
    attr.setReadable(true); attr.setWritable(false);

void mySChecker::postConstructor( )
{
	setMPSafe(true);
}

mySChecker::mySChecker()
{
}

mySChecker::~mySChecker()
{
}

// creates an instance of the node
void * mySChecker::creator()
{
    return new mySChecker();
}

// initializes attribute information
MStatus mySChecker::initialize()
{
    MFnMatrixAttribute mAttr; 
    MFnNumericAttribute nAttr; 
	MObject x, y, z;

	aColor1 = nAttr.createColor("color1", "c1");
	MAKE_INPUT(nAttr);
	nAttr.setDefault(0., .58824, .644);		// Light blue

	aColor2 = nAttr.createColor("color2", "c2");
	MAKE_INPUT(nAttr);
	nAttr.setDefault(1., 1., 1.);			// White

    aBias = nAttr.create( "bias", "b", MFnNumericData::k3Float);
	MAKE_INPUT(nAttr);
    nAttr.setMin(0.0f);
    nAttr.setMax(1.0f);
    nAttr.setDefault(0.5f, 0.5f, 0.5f);

    aPlaceMat = mAttr.create("placementMatrix", "pm",
							 MFnMatrixAttribute::kFloat);
    MAKE_INPUT(mAttr);

	// Internal shading attribute, implicitely connected.
    aPointWorld = nAttr.createPoint("pointWorld", "pw");
	MAKE_INPUT(nAttr);
    nAttr.setHidden(true);

	// Create output attributes
    aOutColor = nAttr.createColor("outColor", "oc");
	MAKE_OUTPUT(nAttr);

    aOutAlpha = nAttr.create( "outAlpha", "oa", MFnNumericData::kFloat);
	MAKE_OUTPUT(nAttr);

    // Add the attributes here
    addAttribute(aColor1);
    addAttribute(aColor2);
    addAttribute(aPointWorld);
    addAttribute(aPlaceMat);
    addAttribute(aBias);

    addAttribute(aOutColor);
    addAttribute(aOutAlpha);

    // all input affect the output color and alpha
    attributeAffects (aColor1, aOutColor);
    attributeAffects (aColor1, aOutAlpha);

    attributeAffects (aColor2, aOutColor);
    attributeAffects (aColor2, aOutAlpha);

    attributeAffects (aPointWorld, aOutColor);
    attributeAffects (aPointWorld, aOutAlpha);

    attributeAffects (aPlaceMat, aOutColor);
    attributeAffects (aPlaceMat, aOutAlpha);

    attributeAffects (aBias, aOutColor);
    attributeAffects (aBias, aOutAlpha);

    return MS::kSuccess;
}


//
// This function gets called by Maya to evaluate the texture.
//
MStatus mySChecker::compute(const MPlug& plug, MDataBlock& block) 
{
	// outColor or individial R, G, B channel, or alpha
    if((plug != aOutColor) && (plug.parent() != aOutColor) && 
	   (plug != aOutAlpha))
		return MS::kUnknownParameter;

	MFloatVector resultColor;
	float3 & worldPos = block.inputValue(aPointWorld).asFloat3();
	MFloatMatrix& mat = block.inputValue(aPlaceMat).asFloatMatrix();
    float3 & bias = block.inputValue(aBias).asFloat3();

	MFloatPoint pos(worldPos[0], worldPos[1], worldPos[2]);
	pos *= mat;								// Convert into solid space

    // normalize the point
    int count = 0;
	if (pos.x - floor(pos.x) < bias[0]) count++;
	if (pos.y - floor(pos.y) < bias[1]) count++;
	if (pos.z - floor(pos.z) < bias[2]) count++;

    if (count & 1)
		resultColor = block.inputValue(aColor2).asFloatVector();
	else
		resultColor = block.inputValue(aColor1).asFloatVector();

	// Set ouput color attribute
	MDataHandle outColorHandle = block.outputValue( aOutColor );
	MFloatVector& outColor = outColorHandle.asFloatVector();
	outColor = resultColor;
	outColorHandle.setClean();

    // Set ouput alpha attribute
	MDataHandle outAlphaHandle = block.outputValue( aOutAlpha );
	float& outAlpha = outAlphaHandle.asFloat();
	outAlpha = ( count & 1) ? 1.0f : 0.0f;
	outAlphaHandle.setClean();

    return MS::kSuccess;
}


MStatus initializePlugin( MObject obj )
{
    const MString UserClassify( "texture/3d" );

    MFnPlugin plugin( obj, PLUGIN_COMPANY, "4.5", "Any");
    plugin.registerNode("solidChecker", mySChecker::id, 
						&mySChecker::creator, &mySChecker::initialize,
						MPxNode::kDependNode, &UserClassify );

    return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj )
{
    MFnPlugin plugin( obj );
    plugin.deregisterNode( mySChecker::id );

    return MS::kSuccess;
}
