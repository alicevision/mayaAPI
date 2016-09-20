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
class MixtureNode : public MPxNode
{
	public:
                    MixtureNode();
    virtual         ~MixtureNode();

    virtual MStatus compute( const MPlug&, MDataBlock& );
	virtual void    postConstructor();

    static  void *  creator();
    static  MStatus initialize();

	// Id tag for use with binary file format
    static  MTypeId id;

	private:

	// Input attributes
	static MObject aColor1;					// 1st Input color
	static MObject aColor2;					// 2nd Input color
	static MObject aAlphaInput1;			// Input Alpha's for 1st color
	static MObject aAlphaInput2;			// Input Alpha's for 2nd color

	// Output attributes
	static MObject aOutColor;				// Output color
};

//
// Override declaration
///////////////////////////////////////////////////////
class MixtureNodeOverride : public MHWRender::MPxShadingNodeOverride
{
public:
	static MHWRender::MPxShadingNodeOverride* creator(const MObject& obj);

	virtual ~MixtureNodeOverride();

	virtual MHWRender::DrawAPI supportedDrawAPIs() const;

	virtual MString fragmentName() const;

private:
	MixtureNodeOverride(const MObject& obj);

	MString fFragmentName;
};

//
// Node definition
///////////////////////////////////////////////////////
MTypeId MixtureNode::id( 0x81007 );

// Attributes
MObject MixtureNode::aColor1;
MObject MixtureNode::aColor2;
MObject MixtureNode::aAlphaInput1;
MObject MixtureNode::aAlphaInput2;
MObject MixtureNode::aOutColor;

#define MAKE_INPUT(attr)								\
     CHECK_MSTATUS ( attr.setKeyable(true) );  			\
	 CHECK_MSTATUS ( attr.setStorable(true) );			\
     CHECK_MSTATUS ( attr.setReadable(true) ) ;  		\
	 CHECK_MSTATUS ( attr.setWritable(true) );

#define MAKE_OUTPUT(attr)								\
     CHECK_MSTATUS ( attr.setKeyable(false) ) ;  		\
	 CHECK_MSTATUS ( attr.setStorable(false) );			\
     CHECK_MSTATUS ( attr.setReadable(true) ) ;  		\
	 CHECK_MSTATUS ( attr.setWritable(false) );

//
// DESCRIPTION:
///////////////////////////////////////////////////////
void MixtureNode::postConstructor( )
{
	setMPSafe(true);
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
MixtureNode::MixtureNode()
{
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
MixtureNode::~MixtureNode()
{
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
void* MixtureNode::creator()
{
    return new MixtureNode();
}


//
// DESCRIPTION:
///////////////////////////////////////////////////////
MStatus MixtureNode::initialize()
{
    MFnNumericAttribute nAttr;

    // Input attributes

    aColor1 = nAttr.createColor( "color1", "c1" );
    MAKE_INPUT(nAttr);
	 CHECK_MSTATUS ( nAttr.setDefault(0.0f, 1.0f, 0.0f) );		// Green

    aColor2 = nAttr.createColor( "color2", "c2" );
    MAKE_INPUT(nAttr);
     CHECK_MSTATUS ( nAttr.setDefault(0.0f, 0.0f, 1.0f) );		// Blue

    aAlphaInput1= nAttr.createColor( "mask1", "m1" );
    MAKE_INPUT(nAttr);
     CHECK_MSTATUS ( nAttr.setDefault(.5f, .5f, .5f) );

    aAlphaInput2= nAttr.createColor( "mask2", "m2" );
    MAKE_INPUT(nAttr);
     CHECK_MSTATUS ( nAttr.setDefault(.5f, .5f, .5f) );

	// Output attributes

    aOutColor  = nAttr.createColor( "outColor", "oc" );
    MAKE_OUTPUT(nAttr);

	//  Add attributes to the node database.
     CHECK_MSTATUS ( addAttribute(aColor1) );
     CHECK_MSTATUS ( addAttribute(aColor2) );
     CHECK_MSTATUS ( addAttribute(aAlphaInput1) );
     CHECK_MSTATUS ( addAttribute(aAlphaInput2) );
     CHECK_MSTATUS ( addAttribute(aOutColor) );

    // All input affect the output color
     CHECK_MSTATUS ( attributeAffects(aColor1, aOutColor) );
     CHECK_MSTATUS ( attributeAffects(aColor2, aOutColor) );
     CHECK_MSTATUS ( attributeAffects(aAlphaInput1, aOutColor) );
     CHECK_MSTATUS ( attributeAffects(aAlphaInput2, aOutColor) );

    return MS::kSuccess;
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
MStatus MixtureNode::compute(const MPlug& plug, MDataBlock& block )
{
    if ((plug != aOutColor) && (plug.parent() != aOutColor))
		return MS::kUnknownParameter;

    MFloatVector color1 = block.inputValue( aColor1 ).asFloatVector();
    MFloatVector color2 = block.inputValue( aColor2 ).asFloatVector();
    MFloatVector mask1  = block.inputValue( aAlphaInput1 ).asFloatVector();
    MFloatVector mask2  = block.inputValue( aAlphaInput2 ).asFloatVector();

    // Mask1 applied to color1, mask2 applied to color2
    color1[0] *= mask1[0]; color1[1] *= mask1[1]; color1[2] *= mask1[2];
    color2[0] *= mask2[0]; color2[1] *= mask2[1]; color2[2] *= mask2[2];

    // set ouput color attribute
    MDataHandle outColorHandle = block.outputValue( aOutColor );
    MFloatVector& outColor = outColorHandle.asFloatVector();
    outColor = color1 + color2;
    outColorHandle.setClean();

    return MS::kSuccess;
}

//
// Override definition
///////////////////////////////////////////////////////
MHWRender::MPxShadingNodeOverride* MixtureNodeOverride::creator(
	const MObject& obj)
{
	return new MixtureNodeOverride(obj);
}

MixtureNodeOverride::MixtureNodeOverride(const MObject& obj)
: MPxShadingNodeOverride(obj)
, fFragmentName("")
{
	// Define fragments needed for VP2 version of shader, this could also be
	// defined in a separate XML file
	//
	// Define the input and output parameter names to match the input and
	// output attribute names so that the values are automatically populated
	// on the shader.
	static const MString sFragmentName("mixtureNodePluginFragment");
	static const char* sFragmentBody =
		"<fragment uiName=\"mixtureNodePluginFragment\" name=\"mixtureNodePluginFragment\" type=\"plumbing\" class=\"ShadeFragment\" version=\"1.0\">"
		"	<description><![CDATA[Mixture utility fragment]]></description>"
		"	<properties>"
		"		<float3 name=\"color1\" />"
		"		<float3 name=\"color2\" />"
		"		<float3 name=\"mask1\" />"
		"		<float3 name=\"mask2\" />"
		"	</properties>"
		"	<values>"
		"		<float3 name=\"color1\" value=\"0.0,1.0,0.0\" />"
		"		<float3 name=\"color2\" value=\"0.0,0.0,1.0\" />"
		"		<float3 name=\"mask1\" value=\"0.5,0.5,0.5\" />"
		"		<float3 name=\"mask2\" value=\"0.5,0.5,0.5\" />"
		"	</values>"
		"	<outputs>"
		"		<float3 name=\"outColor\" />"
		"	</outputs>"
		"	<implementation>"
		"	<implementation render=\"OGSRenderer\" language=\"Cg\" lang_version=\"2.1\">"
		"		<function_name val=\"mixtureNodePluginFragment\" />"
		"		<source><![CDATA["
		"float3 mixtureNodePluginFragment(float3 color1, float3 color2, float3 mask1, float3 mask2) \n"
		"{ \n"
		"	return mask1*color1 + mask2*color2; \n"
		"} \n]]>"
		"		</source>"
		"	</implementation>"
		"	<implementation render=\"OGSRenderer\" language=\"HLSL\" lang_version=\"11.0\">"
		"		<function_name val=\"mixtureNodePluginFragment\" />"
		"		<source><![CDATA["
		"float3 mixtureNodePluginFragment(float3 color1, float3 color2, float3 mask1, float3 mask2) \n"
		"{ \n"
		"	return mask1*color1 + mask2*color2; \n"
		"} \n]]>"
		"		</source>"
		"	</implementation>"
		"	<implementation render=\"OGSRenderer\" language=\"GLSL\" lang_version=\"3.0\">"
		"		<function_name val=\"mixtureNodePluginFragment\" />"
		"		<source><![CDATA["
		"vec3 mixtureNodePluginFragment(vec3 color1, vec3 color2, vec3 mask1, vec3 mask2) \n"
		"{ \n"
		"	return mask1*color1 + mask2*color2; \n"
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

MixtureNodeOverride::~MixtureNodeOverride()
{
}

MHWRender::DrawAPI MixtureNodeOverride::supportedDrawAPIs() const
{
	return MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile;
}

MString MixtureNodeOverride::fragmentName() const
{
	return fFragmentName;
}

//
// Plugin setup
///////////////////////////////////////////////////////
static const MString sRegistrantId("mixtureShaderPlugin");

MStatus initializePlugin( MObject obj )
{
   const MString UserClassify( "utility/color:drawdb/shader/operation/mixture" );

   MFnPlugin plugin( obj, PLUGIN_COMPANY, "4.5", "Any");
    CHECK_MSTATUS ( plugin.registerNode( "mixture", MixtureNode::id,
                         MixtureNode::creator, MixtureNode::initialize,
                         MPxNode::kDependNode, &UserClassify ) );

	CHECK_MSTATUS(
		MHWRender::MDrawRegistry::registerShadingNodeOverrideCreator(
			"drawdb/shader/operation/mixture",
			sRegistrantId,
			MixtureNodeOverride::creator));

   return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj )
{
   MFnPlugin plugin( obj );
    CHECK_MSTATUS ( plugin.deregisterNode( MixtureNode::id ) );

	CHECK_MSTATUS(
		MHWRender::MDrawRegistry::deregisterShadingNodeOverrideCreator(
			"drawdb/shader/operation/mixture",
			sRegistrantId));

   return MS::kSuccess;
}
