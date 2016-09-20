//-
// ===========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================
//+

#include <math.h>

#include <maya/MFnPlugin.h>
#include <maya/MIOStream.h>
#include <maya/MPxNode.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFloatVector.h>
#include <maya/MDrawRegistry.h>
#include <maya/MPxShadingNodeOverride.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MFragmentManager.h>

//
// Node declaration
///////////////////////////////////////////////////////
class brickTextureNode : public MPxNode
{
	public:

    brickTextureNode();
    virtual         ~brickTextureNode();

    virtual MStatus compute( const MPlug&, MDataBlock& );
	virtual void    postConstructor();

    static  void *  creator();
    static  MStatus initialize();

	//  Id tag for use with binary file format
    static  MTypeId id;

	private:

	// Input attributes
	static MObject aColor1;
	static MObject aColor2;
    static MObject aBlurFactor;
	static MObject aUVCoord;
    static MObject aFilterSize;

	// Output attributes
	static MObject aOutColor;
};

//
// Override declaration
///////////////////////////////////////////////////////
class brickTextureNodeOverride : public MHWRender::MPxShadingNodeOverride
{
public:
	static MHWRender::MPxShadingNodeOverride* creator(const MObject& obj);

	virtual ~brickTextureNodeOverride();

	virtual MHWRender::DrawAPI supportedDrawAPIs() const;

	virtual MString fragmentName() const;

private:
	brickTextureNodeOverride(const MObject& obj);

	MString fFragmentName;
};

//
// Node definition
///////////////////////////////////////////////////////
// static data
MTypeId brickTextureNode::id( 0x8100d );

// Attributes
MObject brickTextureNode::aColor1;
MObject brickTextureNode::aColor2;
MObject brickTextureNode::aBlurFactor;
MObject brickTextureNode::aUVCoord;
MObject brickTextureNode::aFilterSize;

MObject brickTextureNode::aOutColor;

#define MAKE_INPUT(attr)	\
    CHECK_MSTATUS(attr.setKeyable(true) );		\
    CHECK_MSTATUS( attr.setStorable(true) );	\
    CHECK_MSTATUS( attr.setReadable(true) );	\
    CHECK_MSTATUS( attr.setWritable(true) );

#define MAKE_OUTPUT(attr)			\
    CHECK_MSTATUS( attr.setKeyable(false) );	\
    CHECK_MSTATUS( attr.setStorable(false) );	\
    CHECK_MSTATUS( attr.setReadable(true) );	\
    CHECK_MSTATUS( attr.setWritable(false) );

inline float step(float t, float c)
{
    return (t < c) ? 0.0f : 1.0f;
}

inline float smoothstep(float t, float a, float b)
{
    if (t < a) return 0.0;
    if (t > b) return 1.0;
    t = (t - a)/(b - a);
    return t*t*(3.0f - 2.0f*t);
}

inline float linearstep(float t, float a, float b)
{
    if (t < a) return 0.0;
    if (t > b) return 1.0;
    return (t - a)/(b - a);
}

inline float MAX(float a, float b)
{
    return (a < b) ? b : a;
}

inline float MIN(float a, float b)
{
    return (a < b) ? a : b;
}

void brickTextureNode::postConstructor( )
{
	setMPSafe(true);
}


// DESCRIPTION:
//
brickTextureNode::brickTextureNode()
{
}

// DESCRIPTION:
//
brickTextureNode::~brickTextureNode()
{
}

// DESCRIPTION:
// creates an instance of the node
void * brickTextureNode::creator()
{
    return new brickTextureNode();
}

// DESCRIPTION:
//
MStatus brickTextureNode::initialize()
{
    MFnNumericAttribute nAttr;

    // Input attributes

	aColor1 = nAttr.createColor("brickColor", "bc");
	MAKE_INPUT(nAttr);
    CHECK_MSTATUS( nAttr.setDefault(.75, .3, .1) );			// Brown

	aColor2 = nAttr.createColor("jointColor", "jc");
	MAKE_INPUT(nAttr);
    CHECK_MSTATUS( nAttr.setDefault(.75, .75, .75) );		// Grey

    aBlurFactor = nAttr.create( "blurFactor", "bf", MFnNumericData::kFloat);
	MAKE_INPUT(nAttr);

	// Implicit shading network attributes

    MObject child1 = nAttr.create( "uCoord", "u", MFnNumericData::kFloat);
    MObject child2 = nAttr.create( "vCoord", "v", MFnNumericData::kFloat);
    aUVCoord = nAttr.create( "uvCoord", "uv", child1, child2);
    MAKE_INPUT(nAttr);
    CHECK_MSTATUS( nAttr.setHidden(true) );

    child1 = nAttr.create( "uvFilterSizeX", "fsx", MFnNumericData::kFloat);
    child2 = nAttr.create( "uvFilterSizeY", "fsy", MFnNumericData::kFloat);
    aFilterSize = nAttr.create("uvFilterSize", "fs", child1, child2);
    MAKE_INPUT(nAttr);
    CHECK_MSTATUS( nAttr.setHidden(true) );

	// Output attributes
    aOutColor = nAttr.createColor("outColor", "oc");
	MAKE_OUTPUT(nAttr);

	// Add attributes to the node database.
    CHECK_MSTATUS( addAttribute(aColor1) );
    CHECK_MSTATUS( addAttribute(aColor2) );
    CHECK_MSTATUS( addAttribute(aBlurFactor) );
    CHECK_MSTATUS( addAttribute(aFilterSize) );
    CHECK_MSTATUS( addAttribute(aUVCoord) );

    CHECK_MSTATUS( addAttribute(aOutColor) );

	// All input affect the output color
    CHECK_MSTATUS( attributeAffects(aColor1,  		aOutColor) );
    CHECK_MSTATUS( attributeAffects(aColor2,  		aOutColor) );
    CHECK_MSTATUS( attributeAffects(aBlurFactor,  	aOutColor) );
    CHECK_MSTATUS( attributeAffects(aFilterSize,	aOutColor) );
    CHECK_MSTATUS( attributeAffects(aUVCoord,		aOutColor) );

    return MS::kSuccess;
}


///////////////////////////////////////////////////////
// DESCRIPTION:
// This function gets called by Maya to evaluate the texture.
//
// Get color1 and color2 from the input block.
// Get UV coordinates from the input block.
// Compute the color of our brick for a given UV coordinate.
// Put the result into the output plug.
///////////////////////////////////////////////////////
MStatus brickTextureNode::compute(const MPlug& plug, MDataBlock& block)
{
	// outColor or individial R, G, B channel
    if((plug != aOutColor) && (plug.parent() != aOutColor))
		return MS::kUnknownParameter;

    MFloatVector resultColor;
    float2 & uv = block.inputValue( aUVCoord ).asFloat2();
    MFloatVector& surfaceColor1 = block.inputValue( aColor1 ).asFloatVector();
    MFloatVector& surfaceColor2 = block.inputValue( aColor2 ).asFloatVector();

    // normalize the UV coords
    uv[0] -= floorf(uv[0]);
    uv[1] -= floorf(uv[1]);

    float borderWidth = 0.1f;
    float brickHeight = 0.4f;
    float brickWidth  = 0.9f;
    float & blur = block.inputValue( aBlurFactor ).asFloat();

    float v1 = borderWidth/2;
    float v2 = v1 + brickHeight;
    float v3 = v2 + borderWidth;
    float v4 = v3 + brickHeight;
    float u1 = borderWidth/2;
    float u2 = brickWidth/2;
    float u3 = u2 + borderWidth;
    float u4 = u1 + brickWidth;

    float2& fs = block.inputValue( aFilterSize ).asFloat2();
    float du = blur*fs[0]/2.0f;
    float dv = blur*fs[1]/2.0f;

    float t = MAX(MIN(linearstep(uv[1], v1 - dv, v1 + dv) -
					  linearstep(uv[1], v2 - dv, v2 + dv),
					  MAX(linearstep(uv[0], u3 - du, u3 + du),
						  1 - linearstep(uv[0], u2 - du, u2 + du))),
				  MIN(linearstep(uv[1], v3 - dv, v3 + dv) -
					  linearstep(uv[1], v4 - dv, v4 + dv),
					  linearstep(uv[0], u1 - du, u1 + du) -
					  linearstep(uv[0], u4 - du, u4 + du)));

    resultColor = t*surfaceColor1 + (1.0f - t)*surfaceColor2;

	// set ouput color attribute
    MDataHandle outColorHandle = block.outputValue( aOutColor );
    MFloatVector& outColor = outColorHandle.asFloatVector();
    outColor = resultColor;
    outColorHandle.setClean();

    return MS::kSuccess;
}

//
// Override definition
///////////////////////////////////////////////////////
MHWRender::MPxShadingNodeOverride* brickTextureNodeOverride::creator(
	const MObject& obj)
{
	return new brickTextureNodeOverride(obj);
}

brickTextureNodeOverride::brickTextureNodeOverride(const MObject& obj)
: MPxShadingNodeOverride(obj)
, fFragmentName("")
{
	// Define fragments needed for VP2 version of shader, this could also be
	// defined in a separate XML file
	static const MString sFragmentName("brickTextureNodePluginFragment");
	static const char* sFragmentBody =
		"<fragment uiName=\"brickTextureNodePluginFragment\" name=\"brickTextureNodePluginFragment\" type=\"plumbing\" class=\"ShadeFragment\" version=\"1.0\">"
		"	<description><![CDATA[Brick procedural texture fragment]]></description>"
		"	<properties>"
		"		<float3 name=\"brickColor\" />"
		"		<float3 name=\"jointColor\" />"
		"		<float name=\"blurFactor\" />"
        "		<float2 name=\"uvCoord\" semantic=\"mayaUvCoordSemantic\" flags=\"varyingInputParam\" />"
        "		<float2 name=\"uvFilterSize\" />"
		"	</properties>"
		"	<values>"
		"		<float3 name=\"brickColor\" value=\"0.75,0.3,0.1\" />"
		"		<float3 name=\"jointColor\" value=\"0.75,0.75,0.75\" />"
		"	</values>"
		"	<outputs>"
		"		<float3 name=\"outColor\" />"
		"	</outputs>"
		"	<implementation>"
		"	<implementation render=\"OGSRenderer\" language=\"Cg\" lang_version=\"2.1\">"
		"		<function_name val=\"brickTextureNodePluginFragment\" />"
		"		<source><![CDATA["
		"float btnplinearstep(float t, float a, float b) \n"
		"{ \n"
    	"	if (t < a) return 0.0f; \n"
		"	if (t > b) return 1.0f; \n"
		"	return (t - a)/(b - a); \n"
		"} \n"
		"float3 brickTextureNodePluginFragment(float3 color1, float3 color2, float blur, float2 uv, float2 fs) \n"
		"{ \n"
		"	uv -= floor(uv); \n"
		"	float v1 = 0.05f; float v2 = 0.45f; float v3 = 0.55f; float v4 = 0.95f; \n"
		"	float u1 = 0.05f; float u2 = 0.45f; float u3 = 0.55f; float u4 = 0.95f; \n"
		"	float du = blur*fs.x/2.0f; \n"
		"	float dv = blur*fs.y/2.0f; \n"
		"	float t = max( \n"
		"		min(btnplinearstep(uv.y, v1 - dv, v1 + dv) - btnplinearstep(uv.y, v2 - dv, v2 + dv), \n"
		"			max(btnplinearstep(uv.x, u3 - du, u3 + du), 1.0f - btnplinearstep(uv.x, u2 - du, u2 + du))), \n"
		"		min(btnplinearstep(uv.y, v3 - dv, v3 + dv) - btnplinearstep(uv.y, v4 - dv, v4 + dv), \n"
		"			btnplinearstep(uv.x, u1 - du, u1 + du) - btnplinearstep(uv.x, u4 - du, u4 + du))); \n"
    	"	return t*color1 + (1.0f - t)*color2; \n"
		"} \n]]>"
		"		</source>"
		"	</implementation>"
		"	<implementation render=\"OGSRenderer\" language=\"HLSL\" lang_version=\"11.0\">"
		"		<function_name val=\"brickTextureNodePluginFragment\" />"
		"		<source><![CDATA["
		"float btnplinearstep(float t, float a, float b) \n"
		"{ \n"
    	"	if (t < a) return 0.0f; \n"
		"	if (t > b) return 1.0f; \n"
		"	return (t - a)/(b - a); \n"
		"} \n"
		"float3 brickTextureNodePluginFragment(float3 color1, float3 color2, float blur, float2 uv, float2 fs) \n"
		"{ \n"
		"	uv -= floor(uv); \n"
		"	float v1 = 0.05f; float v2 = 0.45f; float v3 = 0.55f; float v4 = 0.95f; \n"
		"	float u1 = 0.05f; float u2 = 0.45f; float u3 = 0.55f; float u4 = 0.95f; \n"
		"	float du = blur*fs.x/2.0f; \n"
		"	float dv = blur*fs.y/2.0f; \n"
		"	float t = max( \n"
		"		min(btnplinearstep(uv.y, v1 - dv, v1 + dv) - btnplinearstep(uv.y, v2 - dv, v2 + dv), \n"
		"			max(btnplinearstep(uv.x, u3 - du, u3 + du), 1.0f - btnplinearstep(uv.x, u2 - du, u2 + du))), \n"
		"		min(btnplinearstep(uv.y, v3 - dv, v3 + dv) - btnplinearstep(uv.y, v4 - dv, v4 + dv), \n"
		"			btnplinearstep(uv.x, u1 - du, u1 + du) - btnplinearstep(uv.x, u4 - du, u4 + du))); \n"
    	"	return t*color1 + (1.0f - t)*color2; \n"
		"} \n]]>"
		"		</source>"
		"	</implementation>"
		"	<implementation render=\"OGSRenderer\" language=\"GLSL\" lang_version=\"3.0\">"
		"		<function_name val=\"brickTextureNodePluginFragment\" />"
		"		<source><![CDATA["
		"float btnplinearstep(float t, float a, float b) \n"
		"{ \n"
    	"	if (t < a) return 0.0f; \n"
		"	if (t > b) return 1.0f; \n"
		"	return (t - a)/(b - a); \n"
		"} \n"
		"vec3 brickTextureNodePluginFragment(vec3 color1, vec3 color2, float blur, vec2 uv, vec2 fs) \n"
		"{ \n"
		"	uv -= floor(uv); \n"
		"	float v1 = 0.05f; float v2 = 0.45f; float v3 = 0.55f; float v4 = 0.95f; \n"
		"	float u1 = 0.05f; float u2 = 0.45f; float u3 = 0.55f; float u4 = 0.95f; \n"
		"	float du = blur*fs.x/2.0f; \n"
		"	float dv = blur*fs.y/2.0f; \n"
		"	float t = max( \n"
		"		min(btnplinearstep(uv.y, v1 - dv, v1 + dv) - btnplinearstep(uv.y, v2 - dv, v2 + dv), \n"
		"			max(btnplinearstep(uv.x, u3 - du, u3 + du), 1.0f - btnplinearstep(uv.x, u2 - du, u2 + du))), \n"
		"		min(btnplinearstep(uv.y, v3 - dv, v3 + dv) - btnplinearstep(uv.y, v4 - dv, v4 + dv), \n"
		"			btnplinearstep(uv.x, u1 - du, u1 + du) - btnplinearstep(uv.x, u4 - du, u4 + du))); \n"
    	"	return t*color1 + (1.0f - t)*color2; \n"
		"} \n]]>"
		"		</source>"
		"	</implementation>"
		"	</implementation>"
		"</fragment>";

	// Register fragments with the manager if needed
	MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
	if (theRenderer)
	{
		MHWRender::MFragmentManager* fragmentMgr =
			theRenderer->getFragmentManager();
		if (fragmentMgr)
		{
			// Add fragments if needed
			bool fragAdded = fragmentMgr->hasFragment(sFragmentName);
			if (!fragAdded)
			{
				fragAdded = (sFragmentName == fragmentMgr->addShadeFragmentFromBuffer(sFragmentBody, false));
			}

			// Use the fragment on successful add
			if (fragAdded)
			{
				fFragmentName = sFragmentName;
			}
		}
	}
}

brickTextureNodeOverride::~brickTextureNodeOverride()
{
}

MHWRender::DrawAPI brickTextureNodeOverride::supportedDrawAPIs() const
{
	return MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile;
}

MString brickTextureNodeOverride::fragmentName() const
{
	return fFragmentName;
}

//
// Plugin setup
///////////////////////////////////////////////////////
static const MString sRegistrantId("brickTexturePlugin");

MStatus initializePlugin( MObject obj )
{
	const MString UserClassify( "texture/2d:drawdb/shader/texture/2d/brickTexture" );

    MFnPlugin plugin(obj, PLUGIN_COMPANY, "4.5", "Any");
    CHECK_MSTATUS( plugin.registerNode("brickTexture", brickTextureNode::id,
						&brickTextureNode::creator,
						&brickTextureNode::initialize,
						MPxNode::kDependNode,
						&UserClassify ) );

	CHECK_MSTATUS(
		MHWRender::MDrawRegistry::registerShadingNodeOverrideCreator(
			"drawdb/shader/texture/2d/brickTexture",
			sRegistrantId,
			brickTextureNodeOverride::creator));

    return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj )
{
    MFnPlugin plugin( obj );
    CHECK_MSTATUS( plugin.deregisterNode( brickTextureNode::id ) );

	CHECK_MSTATUS(
		MHWRender::MDrawRegistry::deregisterShadingNodeOverrideCreator(
			"drawdb/shader/texture/2d/brickTexture",
			sRegistrantId));

	return MS::kSuccess;
}
