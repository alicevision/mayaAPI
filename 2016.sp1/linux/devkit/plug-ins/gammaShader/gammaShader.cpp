//-
// ===========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================
//+

///////////////////////////////////////////////////////
//
// DESCRIPTION:  Gamma correction utility node
//
///////////////////////////////////////////////////////

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
#include <maya/MDrawRegistry.h>
#include <maya/MPxShadingNodeOverride.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MFragmentManager.h>

//
// Node declaration
///////////////////////////////////////////////////////
class Gamma:public MPxNode
{
	public:
                    Gamma();
    virtual         ~Gamma();

	virtual MStatus compute( const MPlug&, MDataBlock& );
	virtual void    postConstructor();

    static void *  creator();
    static MStatus initialize();

	// Id tag for use with binary file format
    static MTypeId id;

	private:

	// Input attributes
	static MObject aColor;					// Input color
    static MObject aGamma;					// R, G, B correction

	// Output attributes
	static MObject aOutColor;
};

//
// Override declaration
///////////////////////////////////////////////////////
class GammaOverride : public MHWRender::MPxShadingNodeOverride
{
public:
	static MHWRender::MPxShadingNodeOverride* creator(const MObject& obj);

	virtual ~GammaOverride();

	virtual MHWRender::DrawAPI supportedDrawAPIs() const;

	virtual MString fragmentName() const;

private:
	GammaOverride(const MObject& obj);

	MString fFragmentName;
};

//
// Node definition
///////////////////////////////////////////////////////
MTypeId Gamma::id( 0x81009 );

// Attributes
MObject Gamma::aColor;
MObject Gamma::aGamma;
MObject Gamma::aOutColor;

#define MAKE_INPUT(attr)								\
 CHECK_MSTATUS ( attr.setKeyable(true) );  			 	\
 CHECK_MSTATUS ( attr.setStorable(true) );				\
 CHECK_MSTATUS ( attr.setReadable(true) );  			\
 CHECK_MSTATUS ( attr.setWritable(true) );

#define MAKE_OUTPUT(attr)								\
  CHECK_MSTATUS ( attr.setKeyable(false) ); 			\
  CHECK_MSTATUS ( attr.setStorable(false) );			\
  CHECK_MSTATUS ( attr.setReadable(true) );  			\
  CHECK_MSTATUS ( attr.setWritable(false) );

//
// DESCRIPTION:
///////////////////////////////////////////////////////
void Gamma::postConstructor( )
{
	setMPSafe(true);
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
Gamma::Gamma()
{
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
Gamma::~Gamma()
{
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
void *Gamma::creator()
{
    return new Gamma();
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
MStatus Gamma::initialize()
{
    MFnNumericAttribute		nAttr;

    // Input attributes

    aColor = nAttr.createColor( "color", "c" );
    MAKE_INPUT(nAttr);

	aGamma = nAttr.createPoint( "gamma", "g" );
    MAKE_INPUT(nAttr);
	CHECK_MSTATUS ( nAttr.setDefault(1.f, 1.f, 1.f) );

	// Output attributes

    aOutColor= nAttr.createColor( "outColor", "oc" );
    MAKE_OUTPUT(nAttr);

	//  Add attributes to the node database.
    CHECK_MSTATUS ( addAttribute(aColor) );
    CHECK_MSTATUS ( addAttribute(aGamma) );
    CHECK_MSTATUS ( addAttribute(aOutColor) );

    // All input affect the output color
    CHECK_MSTATUS ( attributeAffects( aColor, aOutColor ));
    CHECK_MSTATUS ( attributeAffects( aGamma, aOutColor ));

    return MS::kSuccess;
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
MStatus Gamma::compute(const MPlug &plug, MDataBlock &block)
{
    if ((plug != aOutColor) && (plug.parent() != aOutColor))
		return MS::kUnknownParameter;

    MFloatVector & icol = block.inputValue( aColor ).asFloatVector();
    MFloatVector & igam = block.inputValue( aGamma ).asFloatVector();

    MDataHandle och = block.outputValue( aOutColor );
    MFloatVector & ocol = och.asFloatVector();
    ocol[0]= powf(icol[0], 1.f/igam[0]);
    ocol[1]= powf(icol[1], 1.f/igam[1]);
    ocol[2]= powf(icol[2], 1.f/igam[2]);
    och.setClean();

    return MS::kSuccess;
}

//
// Override definition
///////////////////////////////////////////////////////
MHWRender::MPxShadingNodeOverride* GammaOverride::creator(
	const MObject& obj)
{
	return new GammaOverride(obj);
}

GammaOverride::GammaOverride(const MObject& obj)
: MPxShadingNodeOverride(obj)
, fFragmentName("")
{
	// Define fragments needed for VP2 version of shader, this could also be
	// defined in a separate XML file
	//
	// Define the input and output parameter names to match the input and
	// output attribute names so that the values are automatically populated
	// on the shader.
	static const MString sFragmentName("gammaShaderPluginFragment");
	static const char* sFragmentBody =
		"<fragment uiName=\"gammaShaderPluginFragment\" name=\"gammaShaderPluginFragment\" type=\"plumbing\" class=\"ShadeFragment\" version=\"1.0\">"
		"	<description><![CDATA[Gamma utility fragment]]></description>"
		"	<properties>"
		"		<float3 name=\"color\" />"
		"		<float3 name=\"gamma\" />"
		"	</properties>"
		"	<values>"
		"		<float3 name=\"color\" value=\"0.5,0.5,0.5\" />"
		"		<float3 name=\"gamma\" value=\"1.0,1.0,1.0\" />"
		"	</values>"
		"	<outputs>"
		"		<float3 name=\"outColor\" />"
		"	</outputs>"
		"	<implementation>"
		"	<implementation render=\"OGSRenderer\" language=\"Cg\" lang_version=\"2.1\">"
		"		<function_name val=\"gammaShaderPluginFragment\" />"
		"		<source><![CDATA["
		"float3 gammaShaderPluginFragment(float3 icol, float3 igam) \n"
		"{ \n"
		"	float3 result; \n"
		"	result.r = pow(icol.r, 1.0f/igam.r); \n"
		"	result.g = pow(icol.g, 1.0f/igam.g); \n"
		"	result.b = pow(icol.b, 1.0f/igam.b); \n"
		"	return result; \n"
		"} \n]]>"
		"		</source>"
		"	</implementation>"
		"	<implementation render=\"OGSRenderer\" language=\"HLSL\" lang_version=\"11.0\">"
		"		<function_name val=\"gammaShaderPluginFragment\" />"
		"		<source><![CDATA["
		"float3 gammaShaderPluginFragment(float3 icol, float3 igam) \n"
		"{ \n"
		"	float3 result; \n"
		"	result.r = pow(icol.r, 1.0f/igam.r); \n"
		"	result.g = pow(icol.g, 1.0f/igam.g); \n"
		"	result.b = pow(icol.b, 1.0f/igam.b); \n"
		"	return result; \n"
		"} \n]]>"
		"		</source>"
		"	</implementation>"
		"	<implementation render=\"OGSRenderer\" language=\"GLSL\" lang_version=\"3.0\">"
		"		<function_name val=\"gammaShaderPluginFragment\" />"
		"		<source><![CDATA["
		"vec3 gammaShaderPluginFragment(vec3 icol, vec3 igam) \n"
		"{ \n"
		"	vec3 result; \n"
		"	result.r = pow(icol.r, 1.0f/igam.r); \n"
		"	result.g = pow(icol.g, 1.0f/igam.g); \n"
		"	result.b = pow(icol.b, 1.0f/igam.b); \n"
		"	return result; \n"
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

GammaOverride::~GammaOverride()
{
}

MHWRender::DrawAPI GammaOverride::supportedDrawAPIs() const
{
	return MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile;
}

MString GammaOverride::fragmentName() const
{
	return fFragmentName;
}

//
// Plugin setup
///////////////////////////////////////////////////////
static const MString sRegistrantId("gammaShaderPlugin");

MStatus initializePlugin( MObject obj )
{
   const MString UserClassify( "utility/color:drawdb/shader/operation/gammaNode" );

   MFnPlugin plugin( obj, PLUGIN_COMPANY, "4.5", "Any");
   CHECK_MSTATUS ( plugin.registerNode( "gammaNode", Gamma::id,
                         Gamma::creator, Gamma::initialize,
                         MPxNode::kDependNode, &UserClassify ) );

	CHECK_MSTATUS(
		MHWRender::MDrawRegistry::registerShadingNodeOverrideCreator(
			"drawdb/shader/operation/gammaNode",
			sRegistrantId,
			GammaOverride::creator));

   return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj )
{
   MFnPlugin plugin( obj );
   CHECK_MSTATUS ( plugin.deregisterNode( Gamma::id ) );

	CHECK_MSTATUS(
		MHWRender::MDrawRegistry::deregisterShadingNodeOverrideCreator(
			"drawdb/shader/operation/gammaNode",
			sRegistrantId));

   return MS::kSuccess;
}
