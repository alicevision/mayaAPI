//-
// ===========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================
//+

///////////////////////////////////////////////////////////////////
// DESCRIPTION: A simple example of programmable checker texture.
//
// Inputs:
//
//  BiasU, BiasV: Control for the center of the checker.
//  Color1, Color2: the 2 colors for the checker.
//  UV: uv coordinate we're evaluating now.
//
// Output:
//
//  outColor: the result color.
//
// Need to enter the following commands before using:
//
//  shadingNode -asTexture checkerTexture;
//  shadingNode -asUtility place2dTexture;
//  connectAttr place2dTexture1.outUV checkerTexture1.uvCoord;
///////////////////////////////////////////////////////////////////

#include <math.h>
#include <cstdlib>

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
#include <maya/MDrawRegistry.h>
#include <maya/MPxShadingNodeOverride.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MFragmentManager.h>
#include <maya/MGlobal.h>

//
// Node declaration
///////////////////////////////////////////////////////
class CheckerNode : public MPxNode
{
	public:
                    CheckerNode();
    virtual         ~CheckerNode();

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
    static MObject aBias;
	static MObject aUVCoord;

	// Output attributes
	static MObject aOutColor;
	static MObject aOutAlpha;
};

//
// Override declaration
///////////////////////////////////////////////////////
class CheckerNodeOverride : public MHWRender::MPxShadingNodeOverride
{
public:
	static MHWRender::MPxShadingNodeOverride* creator(const MObject& obj);

	virtual ~CheckerNodeOverride();

	virtual MHWRender::DrawAPI supportedDrawAPIs() const;

	virtual MString fragmentName() const;

private:
	CheckerNodeOverride(const MObject& obj);

	MString fFragmentName;
};

//
// Node definition
///////////////////////////////////////////////////////
// static data
const MTypeId CheckerNode::id( 0x81006 );

// Attributes
MObject  CheckerNode::aColor1;
MObject  CheckerNode::aColor2;
MObject  CheckerNode::aBias;
MObject  CheckerNode::aUVCoord;

MObject  CheckerNode::aOutColor;
MObject  CheckerNode::aOutAlpha;

#define MAKE_INPUT(attr)	\
    CHECK_MSTATUS(attr.setKeyable(true) );		\
    CHECK_MSTATUS(attr.setStorable(true) );		\
    CHECK_MSTATUS(attr.setReadable(true) );		\
    CHECK_MSTATUS(attr.setWritable(true) );

#define MAKE_OUTPUT(attr)	\
    CHECK_MSTATUS(attr.setKeyable(false) );		\
    CHECK_MSTATUS(attr.setStorable(false) );	\
    CHECK_MSTATUS(attr.setReadable(true) ); 	\
    CHECK_MSTATUS(attr.setWritable(false) );

void CheckerNode::postConstructor( )
{
	setMPSafe(true);
}


CheckerNode::CheckerNode()
{
}

CheckerNode::~CheckerNode()
{
}

// creates an instance of the node
void * CheckerNode::creator()
{
    return new CheckerNode();
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
MStatus CheckerNode::initialize()
{
    MFnNumericAttribute nAttr;

    // Input attributes

	aColor1 = nAttr.createColor("color1", "c1");
	MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setDefault(0., .58824, .644) );		// Light blue

	aColor2 = nAttr.createColor("color2", "c2");
	MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setDefault(1., 1., 1.) );			// White

    aBias = nAttr.create( "bias", "b", MFnNumericData::k2Float);
	MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setMin(0.0f, 0.0f) );
    CHECK_MSTATUS(nAttr.setMax(1.0f, 1.0f) );
    CHECK_MSTATUS(nAttr.setDefault(0.5f, 0.5f) );

	// Implicit shading network attributes

    MObject child1 = nAttr.create( "uCoord", "u", MFnNumericData::kFloat);
    MObject child2 = nAttr.create( "vCoord", "v", MFnNumericData::kFloat);
    aUVCoord = nAttr.create( "uvCoord","uv", child1, child2);
    MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setHidden(true) );

	// Output attributes

    aOutColor = nAttr.createColor("outColor", "oc");
	MAKE_OUTPUT(nAttr);

    aOutAlpha = nAttr.create( "outAlpha", "oa", MFnNumericData::kFloat);
	MAKE_OUTPUT(nAttr);

	// Add attributes to the node database.

    CHECK_MSTATUS(addAttribute(aColor1));
    CHECK_MSTATUS(addAttribute(aColor2));
    CHECK_MSTATUS(addAttribute(aBias));
    CHECK_MSTATUS(addAttribute(aUVCoord));

    CHECK_MSTATUS(addAttribute(aOutColor));
    CHECK_MSTATUS(addAttribute(aOutAlpha));

    // All input affect the output color and alpha
    CHECK_MSTATUS(attributeAffects (aColor1,  aOutColor));
    CHECK_MSTATUS(attributeAffects(aColor1, aOutAlpha));

    CHECK_MSTATUS(attributeAffects (aColor2,  aOutColor));
    CHECK_MSTATUS(attributeAffects(aColor2, aOutAlpha));

    CHECK_MSTATUS(attributeAffects(aBias, aOutColor));
    CHECK_MSTATUS(attributeAffects(aBias, aOutAlpha));

    CHECK_MSTATUS(attributeAffects (aUVCoord, aOutColor));
    CHECK_MSTATUS(attributeAffects(aUVCoord, aOutAlpha));

    return MS::kSuccess;
}


///////////////////////////////////////////////////////
// DESCRIPTION:
// This function gets called by Maya to evaluate the texture.
//
// Get color1 and color2 from the input block.
// Get UV coordinates from the input block.
// Compute the color/alpha of our checker for a given UV coordinate.
// Put the result into the output plug.
///////////////////////////////////////////////////////

MStatus CheckerNode::compute(
const MPlug&      plug,
      MDataBlock& block )
{
	// outColor or individial R, G, B channel, or alpha
    if((plug != aOutColor) && (plug.parent() != aOutColor) &&
	   (plug != aOutAlpha))
		return MS::kUnknownParameter;

    MFloatVector resultColor;
    float2 & uv = block.inputValue( aUVCoord ).asFloat2();
    float2 & bias = block.inputValue( aBias ).asFloat2();

    int count = 0;
	if (uv[0] - floorf(uv[0]) < bias[0]) count++;
	if (uv[1] - floorf(uv[1]) < bias[1]) count++;

    if (count & 1)
        resultColor = block.inputValue( aColor2 ).asFloatVector();
    else
        resultColor = block.inputValue( aColor1 ).asFloatVector();

    // Set ouput color attribute
    MDataHandle outColorHandle = block.outputValue( aOutColor );
    MFloatVector& outColor = outColorHandle.asFloatVector();
    outColor = resultColor;
    outColorHandle.setClean();

    // Set ouput alpha attribute
	MDataHandle outAlphaHandle = block.outputValue( aOutAlpha );
	float& outAlpha = outAlphaHandle.asFloat();
	outAlpha = (count & 1) ? 1.f : 0.f;
	outAlphaHandle.setClean();

    return MS::kSuccess;
}

//
// Override definition
///////////////////////////////////////////////////////
MHWRender::MPxShadingNodeOverride* CheckerNodeOverride::creator(
	const MObject& obj)
{
	return new CheckerNodeOverride(obj);
}

CheckerNodeOverride::CheckerNodeOverride(const MObject& obj)
: MPxShadingNodeOverride(obj)
, fFragmentName("")
{
	// Fragments are defined in separate XML files, add the checker node
	// directory to the search path and load from the files.
	static const MString sFragmentName("checkerNodePluginFragment");
	static const MString sFragmentOutputName("checkerNodePluginFragmentOutput");
	static const MString sFragmentGraphName("checkerNodePluginGraph");
	MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
	if (theRenderer)
	{
		MHWRender::MFragmentManager* fragmentMgr =
			theRenderer->getFragmentManager();
		if (fragmentMgr)
		{
			// Add search path (once only)
			static bool sAdded = false;
			if (!sAdded)
			{
				MString location;
				if( ! MGlobal::executeCommand(MString("getModulePath -moduleName \"devkit\""), location, false) ) {
					location = MString(getenv("MAYA_LOCATION")) + MString("/devkit");
				} 
				location += "/plug-ins/checkerShader";
				fragmentMgr->addFragmentPath(location);
				sAdded = true;
			}

			// Add fragments if needed
			bool fragAdded = fragmentMgr->hasFragment(sFragmentName);
			bool structAdded = fragmentMgr->hasFragment(sFragmentOutputName);
			bool graphAdded = fragmentMgr->hasFragment(sFragmentGraphName);
			if (!fragAdded)
			{
				fragAdded = (sFragmentName == fragmentMgr->addShadeFragmentFromFile(sFragmentName + ".xml", false));
			}
			if (!structAdded)
			{
				structAdded = (sFragmentOutputName == fragmentMgr->addShadeFragmentFromFile(sFragmentOutputName + ".xml", false));
			}
			if (!graphAdded)
			{
				graphAdded = (sFragmentGraphName == fragmentMgr->addFragmentGraphFromFile(sFragmentGraphName + ".xml"));
			}

			// If we have them all, use the final graph for the override
			if (fragAdded && structAdded && graphAdded)
			{
				fFragmentName = sFragmentGraphName;
			}
		}
	}
}

CheckerNodeOverride::~CheckerNodeOverride()
{
}

MHWRender::DrawAPI CheckerNodeOverride::supportedDrawAPIs() const
{
	return MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile;
}

MString CheckerNodeOverride::fragmentName() const
{
	return fFragmentName;
}

static const MString sRegistrantId("checkerTexturePlugin");

//
// Plugin setup
///////////////////////////////////////////////////////
MStatus initializePlugin( MObject obj )
{
	const MString UserClassify( "texture/2d:drawdb/shader/texture/2d/checkerTexture" );

	MFnPlugin plugin(obj, PLUGIN_COMPANY, "4.5", "Any");
	CHECK_MSTATUS( plugin.registerNode("checkerTexture", CheckerNode::id,
					   CheckerNode::creator, CheckerNode::initialize,
					   MPxNode::kDependNode, &UserClassify ) );

	CHECK_MSTATUS(
		MHWRender::MDrawRegistry::registerShadingNodeOverrideCreator(
			"drawdb/shader/texture/2d/checkerTexture",
			sRegistrantId,
			CheckerNodeOverride::creator));

	return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj )
{
	MFnPlugin plugin( obj );
	CHECK_MSTATUS( plugin.deregisterNode( CheckerNode::id ) );

	CHECK_MSTATUS(
		MHWRender::MDrawRegistry::deregisterShadingNodeOverrideCreator(
			"drawdb/shader/texture/2d/checkerTexture",
			sRegistrantId));

	return MS::kSuccess;
}
