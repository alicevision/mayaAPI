//-
// ===========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================
//+

#include <maya/MPxNode.h>
#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnLightDataAttribute.h>
#include <maya/MFloatVector.h>
#include <maya/MFnPlugin.h>
#include <maya/MDrawRegistry.h>
#include <maya/MPxSurfaceShadingNodeOverride.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MFragmentManager.h>

//////////////////////////////////////////////////////////////////
// Plugin Depth Shader Class Declaration
//////////////////////////////////////////////////////////////////

class depthShader : public MPxNode
{
	public:
                      depthShader();
    virtual          ~depthShader();

    virtual MStatus   compute( const MPlug&, MDataBlock& );
	virtual void      postConstructor();

    static  void *    creator();
    static  MStatus   initialize();

	//  Id tag for use with binary file format
    static  MTypeId   id;

	private:

	// Input attributes
    static MObject aColorNear;
    static MObject aColorFar;
    static MObject aNear;
    static MObject aFar;
    static MObject aPointCamera;

	// Output attributes
	static MObject aOutColor;
};

//////////////////////////////////////////////////////////////////
// Plugin Depth Shader Override Class Declaration
//////////////////////////////////////////////////////////////////

class depthShaderOverride : public MHWRender::MPxSurfaceShadingNodeOverride
{
public:
	static MHWRender::MPxSurfaceShadingNodeOverride* creator(const MObject& obj);

	virtual ~depthShaderOverride();

	virtual MHWRender::DrawAPI supportedDrawAPIs() const;

	virtual MString fragmentName() const;

private:
	depthShaderOverride(const MObject& obj);

	MString fFragmentName;
};

//////////////////////////////////////////////////////////////////
// Plugin Depth Shader Class Implementation
//////////////////////////////////////////////////////////////////

// static data
MTypeId depthShader::id( 0x81002 );

// Attributes
MObject depthShader::aColorNear;
MObject depthShader::aColorFar;
MObject depthShader::aNear;
MObject depthShader::aFar;
MObject depthShader::aPointCamera;

MObject depthShader::aOutColor;

#define MAKE_INPUT(attr)		\
    CHECK_MSTATUS(attr.setKeyable(true));  		\
	CHECK_MSTATUS(attr.setStorable(true));		\
    CHECK_MSTATUS(attr.setReadable(true)); 		\
	CHECK_MSTATUS(attr.setWritable(true));

#define MAKE_OUTPUT(attr)		\
    CHECK_MSTATUS(attr.setKeyable(false)); 		\
	CHECK_MSTATUS(attr.setStorable(false));		\
    CHECK_MSTATUS(attr.setReadable(true)); 		\
	CHECK_MSTATUS(attr.setWritable(false));

void depthShader::postConstructor( )
{
	setMPSafe(true);
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
depthShader::depthShader()
{
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
depthShader::~depthShader()
{
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
void* depthShader::creator()
{
    return new depthShader();
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
MStatus depthShader::initialize()
{
    MFnNumericAttribute nAttr;

    // Create input attributes

	aColorNear = nAttr.createColor("color", "c");
	MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setDefault(0., 1., 0.));			// Green

	aColorFar = nAttr.createColor("colorFar", "cf");
	MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setDefault(0., 0., 1.));			// Blue

    aNear = nAttr.create("near", "n", MFnNumericData::kFloat);
	MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setMin(0.0f));
	CHECK_MSTATUS(nAttr.setSoftMax(1000.0f));

    aFar = nAttr.create("far", "f", MFnNumericData::kFloat);
	MAKE_INPUT(nAttr);
    CHECK_MSTATUS(nAttr.setMin(0.0f));
	CHECK_MSTATUS(nAttr.setSoftMax(1000.0f));
    CHECK_MSTATUS(nAttr.setDefault(2.0f));

    aPointCamera = nAttr.createPoint("pointCamera", "p");
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setHidden(true));

	// Create output attributes
    aOutColor = nAttr.createColor("outColor", "oc");
	MAKE_OUTPUT(nAttr);

    CHECK_MSTATUS(addAttribute(aColorNear));
    CHECK_MSTATUS(addAttribute(aColorFar));
    CHECK_MSTATUS(addAttribute(aNear) );
    CHECK_MSTATUS(addAttribute(aFar));
    CHECK_MSTATUS(addAttribute(aPointCamera));
    CHECK_MSTATUS(addAttribute(aOutColor));

    CHECK_MSTATUS(attributeAffects(aColorNear, aOutColor));
    CHECK_MSTATUS(attributeAffects(aColorFar, aOutColor));
    CHECK_MSTATUS(attributeAffects(aNear, aOutColor));
    CHECK_MSTATUS(attributeAffects(aFar, aOutColor));
    CHECK_MSTATUS(attributeAffects(aPointCamera, aOutColor));

    return MS::kSuccess;
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
MStatus depthShader::compute(
const MPlug&      plug,
      MDataBlock& block )
{
	// outColor or individial R, G, B channel
    if((plug != aOutColor) && (plug.parent() != aOutColor))
		return MS::kUnknownParameter;

	MFloatVector resultColor;

	// get sample surface shading parameters
	MFloatVector& pCamera = block.inputValue(aPointCamera).asFloatVector();
	MFloatVector& cNear   = block.inputValue(aColorNear).asFloatVector();
	MFloatVector& cFar    = block.inputValue(aColorFar).asFloatVector();
	float nearClip        = block.inputValue(aNear).asFloat();
	float farClip         = block.inputValue(aFar).asFloat();

	// pCamera.z is negative
	float ratio = (farClip + pCamera.z) / ( farClip - nearClip);
	resultColor = cNear * ratio + cFar*(1.f - ratio);

	// set ouput color attribute
	MDataHandle outColorHandle = block.outputValue( aOutColor );
	MFloatVector& outColor = outColorHandle.asFloatVector();
	outColor = resultColor;
	outColorHandle.setClean();

    return MS::kSuccess;
}

//////////////////////////////////////////////////////////////////
// Plugin Depth Shader Override Class Implementation
//////////////////////////////////////////////////////////////////

MHWRender::MPxSurfaceShadingNodeOverride* depthShaderOverride::creator(
	const MObject& obj)
{
	return new depthShaderOverride(obj);
}

depthShaderOverride::depthShaderOverride(const MObject& obj)
: MPxSurfaceShadingNodeOverride(obj)
, fFragmentName("")
{
	// Define fragments needed for VP2 version of shader, this could also be
	// defined in a separate XML file
	//
	// Define the input and output parameter names to match the input and
	// output attribute names so that the values are automatically populated
	// on the shader.
	//
	// Define a separate fragment for computing the camera space position so
	// that the operation can be done in the vertex shader rather than the
	// pixel shader. Then connect the two fragments together in a graph.
	static const MString sFragmentName("depthShaderPluginFragment");
	static const char* sFragmentBody =
		"<fragment uiName=\"depthShaderPluginFragment\" name=\"depthShaderPluginFragment\" type=\"plumbing\" class=\"ShadeFragment\" version=\"1.0\">"
		"	<description><![CDATA[Depth shader fragment]]></description>"
		"	<properties>"
		"		<float name=\"depthValue\" />"
		"		<float3 name=\"color\" />"
		"		<float3 name=\"colorFar\" />"
		"		<float name=\"near\" />"
		"		<float name=\"far\" />"
		"	</properties>"
		"	<values>"
		"		<float name=\"depthValue\" value=\"0.0\" />"
		"		<float3 name=\"color\" value=\"0.0,1.0,0.0\" />"
		"		<float3 name=\"colorFar\" value=\"0.0,0.0,1.0\" />"
		"		<float name=\"near\" value=\"0.0\" />"
		"		<float name=\"far\" value=\"2.0\" />"
		"	</values>"
		"	<outputs>"
		"		<float3 name=\"outColor\" />"
		"	</outputs>"
		"	<implementation>"
		"	<implementation render=\"OGSRenderer\" language=\"Cg\" lang_version=\"2.1\">"
		"		<function_name val=\"depthShaderPluginFragment\" />"
		"		<source><![CDATA["
		"float3 depthShaderPluginFragment(float depthValue, float3 cNear, float3 cFar, float nearClip, float farClip) \n"
		"{ \n"
		"	float ratio = (farClip + depthValue)/(farClip - nearClip); \n"
		"	return cNear*ratio + cFar*(1.0f - ratio); \n"
		"} \n]]>"
		"		</source>"
		"	</implementation>"
		"	<implementation render=\"OGSRenderer\" language=\"HLSL\" lang_version=\"11.0\">"
		"		<function_name val=\"depthShaderPluginFragment\" />"
		"		<source><![CDATA["
		"float3 depthShaderPluginFragment(float depthValue, float3 cNear, float3 cFar, float nearClip, float farClip) \n"
		"{ \n"
		"	float ratio = (farClip + depthValue)/(farClip - nearClip); \n"
		"	return cNear*ratio + cFar*(1.0f - ratio); \n"
		"} \n]]>"
		"		</source>"
		"	</implementation>"
		"	<implementation render=\"OGSRenderer\" language=\"GLSL\" lang_version=\"3.0\">"
		"		<function_name val=\"depthShaderPluginFragment\" />"
		"		<source><![CDATA["
		"vec3 depthShaderPluginFragment(float depthValue, vec3 cNear, vec3 cFar, float nearClip, float farClip) \n"
		"{ \n"
		"	float ratio = (farClip + depthValue)/(farClip - nearClip); \n"
		"	return cNear*ratio + cFar*(1.0f - ratio); \n"
		"} \n]]>"
		"		</source>"
		"	</implementation>"
		"	</implementation>"
		"</fragment>";

	static const MString sVertexFragmentName("depthShaderPluginInterpolantFragment");
	static const char* sVertexFragmentBody =
		"<fragment uiName=\"depthShaderPluginInterpolantFragment\" name=\"depthShaderPluginInterpolantFragment\" type=\"interpolant\" class=\"ShadeFragment\" version=\"1.0\">"
		"	<description><![CDATA[Depth shader vertex fragment]]></description>"
		"	<properties>"
		"		<float3 name=\"Pm\" semantic=\"Pm\" flags=\"varyingInputParam\" />"
		"		<float4x4 name=\"worldViewProj\" semantic=\"worldviewprojection\" />"
		"	</properties>"
		"	<values>"
		"	</values>"
		"	<outputs>"
		"		<float name=\"outDepthValue\" ^1s/>"
		"	</outputs>"
		"	<implementation>"
		"	<implementation render=\"OGSRenderer\" language=\"Cg\" lang_version=\"2.1\">"
		"		<function_name val=\"depthShaderPluginInterpolantFragment\" />"
		"		<source><![CDATA["
		"float depthShaderPluginInterpolantFragment(float depthValue) \n"
		"{ \n"
		"	return depthValue; \n"
		"} \n]]>"
		"		</source>"
		"		<vertex_source><![CDATA["
		"float idepthShaderPluginInterpolantFragment(float3 Pm, float4x4 worldViewProj) \n"
		"{ \n"
		"	float4 pCamera = mul(worldViewProj, float4(Pm, 1.0f)); \n"
		"	return (pCamera.z - pCamera.w*2.0f); \n"
		"} \n]]>"
		"		</vertex_source>"
		"	</implementation>"
		"	<implementation render=\"OGSRenderer\" language=\"HLSL\" lang_version=\"11.0\">"
		"		<function_name val=\"depthShaderPluginInterpolantFragment\" />"
		"		<source><![CDATA["
		"float depthShaderPluginInterpolantFragment(float depthValue) \n"
		"{ \n"
		"	return depthValue; \n"
		"} \n]]>"
		"		</source>"
		"		<vertex_source><![CDATA["
		"float idepthShaderPluginInterpolantFragment(float3 Pm, float4x4 worldViewProj) \n"
		"{ \n"
		"	float4 pCamera = mul(float4(Pm, 1.0f), worldViewProj); \n"
		"	return (pCamera.z - pCamera.w*2.0f); \n"
		"} \n]]>"
		"		</vertex_source>"
		"	</implementation>"
		"	<implementation render=\"OGSRenderer\" language=\"GLSL\" lang_version=\"3.0\">"
		"		<function_name val=\"depthShaderPluginInterpolantFragment\" />"
		"		<source><![CDATA["
		"float depthShaderPluginInterpolantFragment(float depthValue) \n"
		"{ \n"
		"	return depthValue; \n"
		"} \n]]>"
		"		</source>"
		"		<vertex_source><![CDATA["
		"float idepthShaderPluginInterpolantFragment(vec3 Pm, mat4 worldViewProj) \n"
		"{ \n"
		"	vec4 pCamera = worldViewProj * vec4(Pm, 1.0f); \n"
		"	return (pCamera.z - pCamera.w*2.0f); \n"
		"} \n]]>"
		"		</vertex_source>"
		"	</implementation>"
		"	</implementation>"
		"</fragment>";

	static const MString sFragmentGraphName("depthShaderPluginGraph");
	static const char* sFragmentGraphBody =
		"<fragment_graph name=\"depthShaderPluginGraph\" ref=\"depthShaderPluginGraph\" class=\"FragmentGraph\" version=\"1.0\">"
		"	<fragments>"
		"			<fragment_ref name=\"depthShaderPluginFragment\" ref=\"depthShaderPluginFragment\" />"
		"			<fragment_ref name=\"depthShaderPluginInterpolantFragment\" ref=\"depthShaderPluginInterpolantFragment\" />"
		"	</fragments>"
		"	<connections>"
		"		<connect from=\"depthShaderPluginInterpolantFragment.outDepthValue\" to=\"depthShaderPluginFragment.depthValue\" />"
		"	</connections>"
		"	<properties>"
        "		<float3 name=\"Pm\" ref=\"depthShaderPluginInterpolantFragment.Pm\" semantic=\"Pm\" flags=\"varyingInputParam\" />"
        "		<float4x4 name=\"worldViewProj\" ref=\"depthShaderPluginInterpolantFragment.worldViewProj\" semantic=\"worldviewprojection\" />"
		"		<float3 name=\"color\" ref=\"depthShaderPluginFragment.color\" />"
		"		<float3 name=\"colorFar\" ref=\"depthShaderPluginFragment.colorFar\" />"
		"		<float name=\"near\" ref=\"depthShaderPluginFragment.near\" />"
		"		<float name=\"far\" ref=\"depthShaderPluginFragment.far\" />"
		"	</properties>"
		"	<values>"
		"		<float3 name=\"color\" value=\"0.0,1.0,0.0\" />"
		"		<float3 name=\"colorFar\" value=\"0.0,0.0,1.0\" />"
		"		<float name=\"near\" value=\"0.0\" />"
		"		<float name=\"far\" value=\"2.0\" />"
		"	</values>"
		"	<outputs>"
		"		<float3 name=\"outColor\" ref=\"depthShaderPluginFragment.outColor\" />"
		"	</outputs>"
		"</fragment_graph>";

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
			bool vertFragAdded = fragmentMgr->hasFragment(sVertexFragmentName);
			bool graphAdded = fragmentMgr->hasFragment(sFragmentGraphName);
			if (!fragAdded)
			{
				fragAdded = (sFragmentName == fragmentMgr->addShadeFragmentFromBuffer(sFragmentBody, false));
			}
			if (!vertFragAdded)
			{
				// In DirectX, need to specify a semantic for the output of the vertex shader
				MString vertBody;
				if (theRenderer->drawAPI() == MHWRender::kDirectX11)
				{
					vertBody.format(MString(sVertexFragmentBody), MString("semantic=\"extraDepth\" "));
				}
				else
				{
					vertBody.format(MString(sVertexFragmentBody), MString(" "));
				}
				vertFragAdded = (sVertexFragmentName == fragmentMgr->addShadeFragmentFromBuffer(vertBody.asChar(), false));
			}
			if (!graphAdded)
			{
				graphAdded = (sFragmentGraphName == fragmentMgr->addFragmentGraphFromBuffer(sFragmentGraphBody));
			}

			// If we have them all, use the final graph for the override
			if (fragAdded && vertFragAdded && graphAdded)
			{
				fFragmentName = sFragmentGraphName;
			}
		}
	}
}

depthShaderOverride::~depthShaderOverride()
{
}

MHWRender::DrawAPI depthShaderOverride::supportedDrawAPIs() const
{
	return MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile;
}

MString depthShaderOverride::fragmentName() const
{
	return fFragmentName;
}


//////////////////////////////////////////////////////////////////
// Plugin Setup
//////////////////////////////////////////////////////////////////

static const MString sRegistrantId("depthShaderPlugin");

//
// DESCRIPTION:
///////////////////////////////////////////////////////
MStatus initializePlugin( MObject obj )
{
	const MString UserClassify( "shader/surface:drawdb/shader/surface/depthShader" );

	MFnPlugin plugin(obj, PLUGIN_COMPANY, "4.5", "Any");
	CHECK_MSTATUS( plugin.registerNode("depthShader", depthShader::id,
						depthShader::creator, depthShader::initialize,
						MPxNode::kDependNode, &UserClassify ) );

	CHECK_MSTATUS(
		MHWRender::MDrawRegistry::registerSurfaceShadingNodeOverrideCreator(
			"drawdb/shader/surface/depthShader",
			sRegistrantId,
			depthShaderOverride::creator));

	return MS::kSuccess;
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
MStatus uninitializePlugin( MObject obj )
{
	MFnPlugin plugin( obj );
	CHECK_MSTATUS( plugin.deregisterNode( depthShader::id ) );

	CHECK_MSTATUS(
		MHWRender::MDrawRegistry::deregisterSurfaceShadingNodeOverrideCreator(
			"drawdb/shader/surface/depthShader",
			sRegistrantId));

	return MS::kSuccess;
}
