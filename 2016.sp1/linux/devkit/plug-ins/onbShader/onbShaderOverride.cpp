//-
// ===========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================
//+

#include "onbShaderOverride.h"
#include <maya/MViewport2Renderer.h>
#include <maya/MFragmentManager.h>

//
// This is the implementation of MPxSurfaceShadingNodeOverride that tells Maya
// how to build and manage a VP2 shading fragment for the onbShader node in the
// DG. Note that the implementation is trivial for the most part. The reason for
// this is because we have made the final fragment graph parameters match in
// name and type to the attributes on the shading node. If that was not the
// case, or if there were paramters without a one-to-one mapping from
// attributes on the node then this class would be more complicated, perhaps
// requiring the implementation of several of the other virtual methods from
// MPxShadingNodeOverride (the base class to MPxSurfaceShadingNodeOverride).
//
// Here we simply provide the final fragment graph name in fragmentName(),
// and also provide parameter names for certain material properties (color,
// transparency and bump).
//
// A static method handles the definition and registration of the actual
// shading fragments and the final fragment graph. Please see the comments in
// registerFragments() for more details.
//

const MString onbShaderOverride::sFinalFragmentGraphName("onbShaderSurface");

MHWRender::MPxSurfaceShadingNodeOverride* onbShaderOverride::creator(
	const MObject& obj)
{
	return new onbShaderOverride(obj);
}

onbShaderOverride::onbShaderOverride(const MObject& obj)
: MPxSurfaceShadingNodeOverride(obj)
, fObject(obj)
{
}

onbShaderOverride::~onbShaderOverride()
{
}

MHWRender::DrawAPI onbShaderOverride::supportedDrawAPIs() const
{
	// Support all available draw APIs (all fragments have an implementation for
	// each).
	return
		MHWRender::kOpenGL |
		MHWRender::kDirectX11 |
		MHWRender::kOpenGLCoreProfile;
}

MString onbShaderOverride::fragmentName() const
{
	// Return the name of the full fragment graph
	return sFinalFragmentGraphName;
}

MString onbShaderOverride::primaryColorParameter() const
{
	// Use the color parameter from the our fragment graph as the primary color
	return "color";
}

MString onbShaderOverride::transparencyParameter() const
{
	// Use the transparency parameter from our fragment graph for transparency
	return "transparency";
}

MString onbShaderOverride::bumpAttribute() const
{
	// Use the normalCamera attribute to recognize bump connections
	return "normalCamera";
}



// Static fragment registration/deregistration methods called from plugin
// init/unint functions. Only need to be called once.
MStatus onbShaderOverride::registerFragments()
{
	// Get fragment manager for registration
	MHWRender::MFragmentManager* fragmentMgr =
		MHWRender::MRenderer::theRenderer()
			? MHWRender::MRenderer::theRenderer()->getFragmentManager()
			: NULL;
	// No fragment manager, fail
	if (!fragmentMgr) return MS::kFailure;
	// Fragments are already registered, return success
	if (fragmentMgr->hasFragment(sFinalFragmentGraphName)) return MS::kSuccess;

	MString fragmentName;
	char* fragmentBody = NULL;

	// ------------------------------------------------------------------------
	// Pass-through fragment for float3 type, used to provide clarity and as to
	// where duplicated parameters come from (in this case it is used three
	// times, once each for Nw, Vw and Lw, which are all used on both
	// onbShaderGeom and onbDiffuse.
	//
	fragmentName = "onbFloat3PassThrough";
	fragmentBody =
		"<fragment uiName=\"onbFloat3PassThrough\" name=\"onbFloat3PassThrough\" type=\"plumbing\" class=\"ShadeFragment\" version=\"1.0\"> \r\n"
		"	<description><![CDATA[Pass-through fragment for float3 type.]]></description> \r\n"
		"	<properties> \r\n"
		"		<float3 name=\"input\" /> \r\n"
		"	</properties> \r\n"
		"	<values> \r\n"
		"	</values> \r\n"
		"	<outputs> \r\n"
		"		<float3 name=\"output\" /> \r\n"
		"	</outputs> \r\n"
		"	<implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"Cg\" lang_version=\"2.1\"> \r\n"
		"		<function_name val=\"onbFloat3PassThrough\" /> \r\n"
		"		<source><![CDATA[ \r\n"
		"float3 onbFloat3PassThrough(float3 value) \r\n"
		"{ \r\n"
		"	return value; \r\n"
		"} \r\n"
		"		]]></source> \r\n"
		"	</implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"HLSL\" lang_version=\"11.0\"> \r\n"
		"		<function_name val=\"onbFloat3PassThrough\" /> \r\n"
		"		<source><![CDATA[ \r\n"
		"float3 onbFloat3PassThrough(float3 value) \r\n"
		"{ \r\n"
		"	return value; \r\n"
		"} \r\n"
		"		]]></source> \r\n"
		"	</implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"GLSL\" lang_version=\"3.0\"> \r\n"
		"		<function_name val=\"onbFloat3PassThrough\" /> \r\n"
		"		<source><![CDATA[ \r\n"
		"vec3 onbFloat3PassThrough(vec3 value) \r\n"
		"{ \r\n"
		"	return value; \r\n"
		"} \r\n"
		"		]]></source> \r\n"
		"	</implementation> \r\n"
		"	</implementation> \r\n"
		"</fragment> \r\n";
	if (fragmentName != fragmentMgr->addShadeFragmentFromBuffer(fragmentBody, false)) return MS::kFailure;

	// ------------------------------------------------------------------------
	// Fragment to pre-compute some dot products needed by diffuse and specular
	// computations. Results are packed into a float4.
	//
	fragmentName = "onbShaderGeom";
	fragmentBody =
		"<fragment uiName=\"onbShaderGeom\" name=\"onbShaderGeom\" type=\"plumbing\" class=\"ShadeFragment\" version=\"1.0\"> \r\n"
		"	<description><![CDATA[Computes shader geometric components (dot products)]]></description> \r\n"
		"	<properties> \r\n"
		"		<float3 name=\"Nw\" flags=\"varyingInputParam\" /> \r\n"
		"		<float3 name=\"Lw\" /> \r\n"
		"		<float3 name=\"HLw\" /> \r\n"
		"		<float3 name=\"Vw\" flags=\"varyingInputParam\" /> \r\n"
		"	</properties> \r\n"
		"	<values> \r\n"
		"		<float3 name=\"Lw\" value=\"0.0,0.0,0.0\" /> \r\n"
		"		<float3 name=\"HLw\" value=\"0.0,0.0,0.0\" /> \r\n"
		"	</values> \r\n"
		"	<outputs> \r\n"
		"		<float4 name=\"onbShaderGeom\" /> \r\n"
		"	</outputs> \r\n"
		"	<implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"Cg\" lang_version=\"2.1\"> \r\n"
		"		<function_name val=\"onbShaderGeom\" /> \r\n"
		"		<source><![CDATA[ \r\n"
		"float4 onbShaderGeom( \r\n"
		"	float3 Nw, \r\n"
		"	float3 Lw, \r\n"
		"	float3 HLw, \r\n"
		"	float3 Vw) \r\n"
		"{ \r\n"
		"	float3 Hw = normalize(HLw + Vw); \r\n"
		"	float NL = saturate(dot(Nw, Lw)); \r\n"
		"	float NV = saturate(dot(Nw, Vw)); \r\n"
		"	float NH = saturate(dot(Nw, Hw)); \r\n"
		"	float VH = dot(Vw, Hw); \r\n"
		"	return float4(NL, NV, NH, VH); \r\n"
		"} \r\n"
		"		]]></source> \r\n"
		"	</implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"HLSL\" lang_version=\"11.0\"> \r\n"
		"		<function_name val=\"onbShaderGeom\" /> \r\n"
		"		<source><![CDATA[ \r\n"
		"float4 onbShaderGeom( \r\n"
		"	float3 Nw, \r\n"
		"	float3 Lw, \r\n"
		"	float3 HLw, \r\n"
		"	float3 Vw) \r\n"
		"{ \r\n"
		"	float3 Hw = normalize(HLw + Vw); \r\n"
		"	float NL = saturate(dot(Nw, Lw)); \r\n"
		"	float NV = saturate(dot(Nw, Vw)); \r\n"
		"	float NH = saturate(dot(Nw, Hw)); \r\n"
		"	float VH = dot(Vw, Hw); \r\n"
		"	return float4(NL, NV, NH, VH); \r\n"
		"} \r\n"
		"		]]></source> \r\n"
		"	</implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"GLSL\" lang_version=\"3.0\"> \r\n"
		"		<function_name val=\"onbShaderGeom\" /> \r\n"
		"		<source><![CDATA[ \r\n"
		"vec4 onbShaderGeom( \r\n"
		"	vec3 Nw, \r\n"
		"	vec3 Lw, \r\n"
		"	vec3 HLw, \r\n"
		"	vec3 Vw) \r\n"
		"{ \r\n"
		"	vec3 Hw = normalize(HLw + Vw); \r\n"
		"	float NL = saturate(dot(Nw, Lw)); \r\n"
		"	float NV = saturate(dot(Nw, Vw)); \r\n"
		"	float NH = saturate(dot(Nw, Hw)); \r\n"
		"	float VH = dot(Vw, Hw); \r\n"
		"	return vec4(NL, NV, NH, VH); \r\n"
		"} \r\n"
		"		]]></source> \r\n"
		"	</implementation> \r\n"
		"	</implementation> \r\n"
		"</fragment> \r\n";
	if (fragmentName != fragmentMgr->addShadeFragmentFromBuffer(fragmentBody, false)) return MS::kFailure;

	// ------------------------------------------------------------------------
	// Fragment to compute the diffuse contribution for shading for a single
	// light source. Implements the simplified Oren-Nayar computation  based on
	//   Engel, Wolfgang et al. Programming Vertex, Geometry, and Pixel Shaders
	//   http://content.gpwiki.org/index.php/D3DBook:(Lighting)_Oren-Nayar
	//
	fragmentName = "onbDiffuse";
	fragmentBody =
		"<fragment uiName=\"onbDiffuse\" name=\"onbDiffuse\" type=\"plumbing\" class=\"ShadeFragment\" version=\"1.0\"> \r\n"
		"	<description><![CDATA[Computes Oren-Nayar diffuse component]]></description> \r\n"
		"	<properties> \r\n"
		"		<float4 name=\"shaderGeomInput\" /> \r\n"
		"		<float3 name=\"Nw\" flags=\"varyingInputParam\" /> \r\n"
		"		<float3 name=\"Lw\" /> \r\n"
		"		<float3 name=\"Vw\" flags=\"varyingInputParam\" /> \r\n"
		"		<float3 name=\"diffuseI\" /> \r\n"
		"		<float name=\"roughness\" /> \r\n"
		"	</properties> \r\n"
		"	<values> \r\n"
		"		<float4 name=\"shaderGeomInput\" value=\"0.0,0.0,0.0,0.0\" /> \r\n"
		"		<float3 name=\"Lw\" value=\"0.0,0.0,0.0\" /> \r\n"
		"		<float3 name=\"diffuseI\" value=\"0.0,0.0,0.0\" /> \r\n"
		"		<float name=\"roughness\" value=\"0.3\" /> \r\n"
		"	</values> \r\n"
		"	<outputs> \r\n"
		"		<float3 name=\"onbDiffuse\" /> \r\n"
		"	</outputs> \r\n"
		"	<implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"Cg\" lang_version=\"2.1\"> \r\n"
		"		<function_name val=\"onbDiffuse\" /> \r\n"
		"		<source><![CDATA[ \r\n"
		"float3 onbDiffuse( \r\n"
		"	float4 NL_NV_NH_VH, \r\n"
		"	float3 Nw, \r\n"
		"	float3 Lw, \r\n"
		"	float3 Vw, \r\n"
		"	float3 diffuseI, \r\n"
		"	float roughness) \r\n"
		"{ \r\n"
		"	// GPU implementation of Oren-Nayar computation from: \r\n"
		"	//	Engel, Wolfgang et al.: Programming Vertex, Geometry, and Pixel Shaders \r\n"
		"	//	http://content.gpwiki.org/index.php/D3DBook:(Lighting)_Oren-Nayar \r\n"
		"	float acosNV = acos(NL_NV_NH_VH.y); \r\n"
		"	float acosNL = acos(NL_NV_NH_VH.x); \r\n"
		"	float alpha = max(acosNV, acosNL); \r\n"
		"	float beta = min(acosNV, acosNL); \r\n"
		"	float gamma = dot(Vw - Nw*NL_NV_NH_VH.y, Lw - Nw*NL_NV_NH_VH.x); \r\n"
		"	float roughnessSq = roughness*roughness; \r\n"
		"	float A = 1.0f - 0.5f*(roughnessSq/(roughnessSq + 0.57f)); \r\n"
		"	float B = 0.45f*(roughnessSq/(roughnessSq + 0.09f)); \r\n"
		"	float C = sin(alpha)*tan(beta); \r\n"
		"	float factor = max(0.0f, NL_NV_NH_VH.x)*(A + B*max(0.0f, gamma)*C); \r\n"
		"	return factor*diffuseI; \r\n"
		"} \r\n"
		"		]]></source> \r\n"
		"	</implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"HLSL\" lang_version=\"11.0\"> \r\n"
		"		<function_name val=\"onbDiffuse\" /> \r\n"
		"		<source><![CDATA[ \r\n"
		"float3 onbDiffuse( \r\n"
		"	float4 NL_NV_NH_VH, \r\n"
		"	float3 Nw, \r\n"
		"	float3 Lw, \r\n"
		"	float3 Vw, \r\n"
		"	float3 diffuseI, \r\n"
		"	float roughness) \r\n"
		"{ \r\n"
		"	// GPU implementation of Oren-Nayar computation from: \r\n"
		"	//	Engel, Wolfgang et al.: Programming Vertex, Geometry, and Pixel Shaders \r\n"
		"	//	http://content.gpwiki.org/index.php/D3DBook:(Lighting)_Oren-Nayar \r\n"
		"	float acosNV = acos(NL_NV_NH_VH.y); \r\n"
		"	float acosNL = acos(NL_NV_NH_VH.x); \r\n"
		"	float alpha = max(acosNV, acosNL); \r\n"
		"	float beta = min(acosNV, acosNL); \r\n"
		"	float gamma = dot(Vw - Nw*NL_NV_NH_VH.y, Lw - Nw*NL_NV_NH_VH.x); \r\n"
		"	float roughnessSq = roughness*roughness; \r\n"
		"	float A = 1.0f - 0.5f*(roughnessSq/(roughnessSq + 0.57f)); \r\n"
		"	float B = 0.45f*(roughnessSq/(roughnessSq + 0.09f)); \r\n"
		"	float C = sin(alpha)*tan(beta); \r\n"
		"	float factor = max(0.0f, NL_NV_NH_VH.x)*(A + B*max(0.0f, gamma)*C); \r\n"
		"	return factor*diffuseI; \r\n"
		"} \r\n"
		"		]]></source> \r\n"
		"	</implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"GLSL\" lang_version=\"3.0\"> \r\n"
		"		<function_name val=\"onbDiffuse\" /> \r\n"
		"		<source><![CDATA[ \r\n"
		"vec3 onbDiffuse( \r\n"
		"	vec4 NL_NV_NH_VH, \r\n"
		"	vec3 Nw, \r\n"
		"	vec3 Lw, \r\n"
		"	vec3 Vw, \r\n"
		"	vec3 diffuseI, \r\n"
		"	float roughness) \r\n"
		"{ \r\n"
		"	// GPU implementation of Oren-Nayar computation from: \r\n"
		"	//	Engel, Wolfgang et al.: Programming Vertex, Geometry, and Pixel Shaders \r\n"
		"	//	http://content.gpwiki.org/index.php/D3DBook:(Lighting)_Oren-Nayar \r\n"
		"	float acosNV = acos(NL_NV_NH_VH.y); \r\n"
		"	float acosNL = acos(NL_NV_NH_VH.x); \r\n"
		"	float alpha = max(acosNV, acosNL); \r\n"
		"	float beta = min(acosNV, acosNL); \r\n"
		"	float gamma = dot(Vw - Nw*NL_NV_NH_VH.y, Lw - Nw*NL_NV_NH_VH.x); \r\n"
		"	float roughnessSq = roughness*roughness; \r\n"
		"	float A = 1.0f - 0.5f*(roughnessSq/(roughnessSq + 0.57f)); \r\n"
		"	float B = 0.45f*(roughnessSq/(roughnessSq + 0.09f)); \r\n"
		"	float C = sin(alpha)*tan(beta); \r\n"
		"	float factor = max(0.0f, NL_NV_NH_VH.x)*(A + B*max(0.0f, gamma)*C); \r\n"
		"	return factor*diffuseI; \r\n"
		"} \r\n"
		"		]]></source> \r\n"
		"	</implementation> \r\n"
		"	</implementation> \r\n"
		"</fragment> \r\n";
	if (fragmentName != fragmentMgr->addShadeFragmentFromBuffer(fragmentBody, false)) return MS::kFailure;

	// ------------------------------------------------------------------------
	// Fragment to compute the specular contribution for shading for a single
	// light source. Implements blinn specular computation.
	//
	fragmentName = "onbSpecular";
	fragmentBody =
		"<fragment uiName=\"onbSpecular\" name=\"onbSpecular\" type=\"plumbing\" class=\"ShadeFragment\" version=\"1.0\"> \r\n"
		"	<description><![CDATA[Computes blinn specular component]]></description> \r\n"
		"	<properties> \r\n"
		"		<float3 name=\"specularI\" /> \r\n"
		"		<float4 name=\"shaderGeomInput\" /> \r\n"
		"		<float3 name=\"specularColor\" /> \r\n"
		"		<float name=\"eccentricity\" /> \r\n"
		"		<float name=\"specularRollOff\" /> \r\n"
		"	</properties> \r\n"
		"	<values> \r\n"
		"		<float3 name=\"specularI\" value=\"0.0,0.0,0.0\" /> \r\n"
		"		<float4 name=\"shaderGeomInput\" value=\"0.0,0.0,0.0,0.0\" /> \r\n"
		"		<float3 name=\"specularColor\" value=\"1.0,1.0,1.0\" /> \r\n"
		"		<float name=\"eccentricity\" value=\"0.1\" /> \r\n"
		"		<float name=\"specularRollOff\" value=\"0.7\" /> \r\n"
		"	</values> \r\n"
		"	<outputs> \r\n"
		"		<float3 name=\"onbSpecular\" /> \r\n"
		"	</outputs> \r\n"
		"	<implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"Cg\" lang_version=\"2.1\"> \r\n"
		"		<function_name val=\"onbSpecular\" /> \r\n"
		"		<source><![CDATA[ \r\n"
		"float3 onbSpecular( \r\n"
		"	float3 specularI, \r\n"
		"	float4 NL_NV_NH_VH, \r\n"
		"	float3 specularColor, \r\n"
		"	float ecc, \r\n"
		"	float rolloff) \r\n"
		"{ \r\n"
		"	const float eps = 0.00001f; \r\n"
		"	float ecc2 = ecc * ecc - 1.0f; \r\n"
		"	float NH = NL_NV_NH_VH.z; \r\n"
		"	float d = (ecc2 + 1.0f) / (1.0f + ecc2 * NH * NH); \r\n"
		"	d *= 2.0f * d * d; \r\n"
		"	float NL = NL_NV_NH_VH.x; \r\n"
		"	float NV = NL_NV_NH_VH.y; \r\n"
		"	float VH = NL_NV_NH_VH.w + eps; \r\n"
		"	NH *= 0.5f; \r\n"
		"	float g = saturate(min(NH*NV/VH, NH*NL/VH)); \r\n"
		"	float k = max((1.0f - NV), (1.0f - NL)); \r\n"
		"	k = k * k * k; \r\n"
		"	float f = k + (1.0f - k) * rolloff; \r\n"
		"	return specularI * saturate(d * f * g) * specularColor; \r\n"
		"} \r\n"
		"		]]></source> \r\n"
		"	</implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"HLSL\" lang_version=\"11.0\"> \r\n"
		"		<function_name val=\"onbSpecular\" /> \r\n"
		"		<source><![CDATA[ \r\n"
		"float3 onbSpecular( \r\n"
		"	float3 specularI, \r\n"
		"	float4 NL_NV_NH_VH, \r\n"
		"	float3 specularColor, \r\n"
		"	float ecc, \r\n"
		"	float rolloff) \r\n"
		"{ \r\n"
		"	const float eps = 0.00001f; \r\n"
		"	float ecc2 = ecc * ecc - 1.0f; \r\n"
		"	float NH = NL_NV_NH_VH.z; \r\n"
		"	float d = (ecc2 + 1.0f) / (1.0f + ecc2 * NH * NH); \r\n"
		"	d *= 2.0f * d * d; \r\n"
		"	float NL = NL_NV_NH_VH.x; \r\n"
		"	float NV = NL_NV_NH_VH.y; \r\n"
		"	float VH = NL_NV_NH_VH.w + eps; \r\n"
		"	NH *= 0.5f; \r\n"
		"	float g = saturate(min(NH*NV/VH, NH*NL/VH)); \r\n"
		"	float k = max((1.0f - NV), (1.0f - NL)); \r\n"
		"	k = k * k * k; \r\n"
		"	float f = k + (1.0f - k) * rolloff; \r\n"
		"	return specularI * saturate(d * f * g) * specularColor; \r\n"
		"} \r\n"
		"		]]></source> \r\n"
		"	</implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"GLSL\" lang_version=\"3.0\"> \r\n"
		"		<function_name val=\"onbSpecular\" /> \r\n"
		"		<source><![CDATA[ \r\n"
		"vec3 onbSpecular( \r\n"
		"	vec3 specularI, \r\n"
		"	vec4 NL_NV_NH_VH, \r\n"
		"	vec3 specularColor, \r\n"
		"	float ecc, \r\n"
		"	float rolloff) \r\n"
		"{ \r\n"
		"	const float eps = 0.00001f; \r\n"
		"	float ecc2 = ecc * ecc - 1.0f; \r\n"
		"	float NH = NL_NV_NH_VH.z; \r\n"
		"	float d = (ecc2 + 1.0f) / (1.0f + ecc2 * NH * NH); \r\n"
		"	d *= 2.0f * d * d; \r\n"
		"	float NL = NL_NV_NH_VH.x; \r\n"
		"	float NV = NL_NV_NH_VH.y; \r\n"
		"	float VH = NL_NV_NH_VH.w + eps; \r\n"
		"	NH *= 0.5f; \r\n"
		"	float g = saturate(min(NH*NV/VH, NH*NL/VH)); \r\n"
		"	float k = max((1.0f - NV), (1.0f - NL)); \r\n"
		"	k = k * k * k; \r\n"
		"	float f = k + (1.0f - k) * rolloff; \r\n"
		"	return specularI * saturate(d * f * g) * specularColor; \r\n"
		"} \r\n"
		"		]]></source> \r\n"
		"	</implementation> \r\n"
		"	</implementation> \r\n"
		"</fragment> \r\n";
	if (fragmentName != fragmentMgr->addShadeFragmentFromBuffer(fragmentBody, false)) return MS::kFailure;

	// ------------------------------------------------------------------------
	// Light accumulator fragment. Special fragment type that instructs the
	// underlying fragment system to generate a light loop based on input from
	// the auto-generated "selector" fragment. The value "mayaLightSelector16"
	// gives the name of the selector fragment. This fragment is created by
	// Maya and connected at the appropriate time. The selector fragment
	// supports up to 16 lights. This accumulator fragment causes the
	// generation of code that computes the diffuse and specular shading for
	// each light and then gathers the scaled sum of the results.
	//
	fragmentName = "onb16LightAccum";
	fragmentBody =
		"<fragment uiName=\"onb16LightAccum\" name=\"onb16LightAccum\" type=\"accum\" class=\"ShadeFragment\" version=\"1.0\"> \r\n"
		"	<description><![CDATA[Accumulates specular & diffuse irradiance for 16 lights]]></description> \r\n"
		"	<properties> \r\n"
		"		<float3 name=\"scaledDiffuse\" /> \r\n"
		"		<float3 name=\"scaledSpecular\" /> \r\n"
		"		<string name=\"selector\" /> \r\n"
		"	</properties> \r\n"
		"	<values> \r\n"
		"		<string name=\"selector\" value=\"mayaLightSelector16\" /> \r\n"
		"	</values> \r\n"
		"	<outputs> \r\n"
		"		<alias name=\"scaledDiffuse\" /> \r\n"
		"		<alias name=\"scaledSpecular\" /> \r\n"
		"	</outputs> \r\n"
		"	<implementation> \r\n"
		"	</implementation> \r\n"
		"</fragment> \r\n";
	if (fragmentName != fragmentMgr->addShadeFragmentFromBuffer(fragmentBody, false)) return MS::kFailure;

	// ------------------------------------------------------------------------
	// Struct declaration fragment. This fragment provides the declaration of
	// the output structure of the onb fragment graph giving support for
	// multiple outputs. The struct fragment must be a part of the final
	// fragment graph in order to ensure that the fragment system includes
	// the declaration of the struct in the final effect.
	//
	fragmentName = "onbShaderOutput";
	fragmentBody =
		"<fragment uiName=\"onbShaderOutput\" name=\"onbShaderOutput\" type=\"structure\" class=\"ShadeFragment\" version=\"1.0\"> \r\n"
		"	<description><![CDATA[Struct output for onb shader]]></description> \r\n"
		"	<properties> \r\n"
		"		<struct name=\"onbShaderOutput\" struct_name=\"onbShaderOutput\" /> \r\n"
		"	</properties> \r\n"
		"	<values> \r\n"
		"	</values> \r\n"
		"	<outputs> \r\n"
		"		<alias name=\"onbShaderOutput\" struct_name=\"onbShaderOutput\" /> \r\n"
		"		<float3 name=\"outColor\" /> \r\n"
		"		<float3 name=\"outTransparency\" /> \r\n"
		"		<float4 name=\"outSurfaceFinal\" /> \r\n"
		"	</outputs> \r\n"
		"	<implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"Cg\" lang_version=\"2.1\"> \r\n"
		"		<function_name val=\"\" /> \r\n"
		"		<declaration name=\"onbShaderOutput\"><![CDATA[ \r\n"
		"struct onbShaderOutput \r\n"
		"{ \r\n"
		"	float3 outColor; \r\n"
		"	float3 outTransparency; \r\n"
		"	float4 outSurfaceFinal; \r\n"
		"}; \r\n"
		"		]]></declaration> \r\n"
		"	</implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"HLSL\" lang_version=\"11.0\"> \r\n"
		"		<function_name val=\"\" /> \r\n"
		"		<declaration name=\"onbShaderOutput\"><![CDATA[ \r\n"
		"struct onbShaderOutput \r\n"
		"{ \r\n"
		"	float3 outColor; \r\n"
		"	float3 outTransparency; \r\n"
		"	float4 outSurfaceFinal; \r\n"
		"}; \r\n"
		"		]]></declaration> \r\n"
		"	</implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"GLSL\" lang_version=\"3.0\"> \r\n"
		"		<function_name val=\"\" /> \r\n"
		"		<declaration name=\"onbShaderOutput\"><![CDATA[ \r\n"
		"struct onbShaderOutput \r\n"
		"{ \r\n"
		"	vec3 outColor; \r\n"
		"	vec3 outTransparency; \r\n"
		"	vec4 outSurfaceFinal; \r\n"
		"}; \r\n"
		"		]]></declaration> \r\n"
		"	</implementation> \r\n"
		"	</implementation> \r\n"
		"</fragment> \r\n";
	if (fragmentName != fragmentMgr->addShadeFragmentFromBuffer(fragmentBody, false)) return MS::kFailure;

	// ------------------------------------------------------------------------
	// Combiner fragment. Takes in the computed diffuse and specular results
	// and combines them with the other shader parameters to produce the final
	// color, transparency and surface (color with alpha) values. The ambientIn
	// parameter is populated by Maya with level of global ambient light based
	// on ambient lights in the scene.
	//
	fragmentName = "onbCombiner";
	fragmentBody =
		"<fragment uiName=\"onbCombiner\" name=\"onbCombiner\" type=\"plumbing\" class=\"ShadeFragment\" version=\"1.0\"> \r\n"
		"	<description><![CDATA[Combines ambient, diffuse, specular components additively]]></description> \r\n"
		"	<properties> \r\n"
		"		<float3 name=\"diffuseIrradIn\" flags=\"constantParam\" /> \r\n"
		"		<float3 name=\"specularIrradIn\" flags=\"constantParam\" /> \r\n"
		"		<float3 name=\"color\" /> \r\n"
		"		<float3 name=\"transparency\" /> \r\n"
		"		<float3 name=\"ambientColor\" /> \r\n"
		"		<float3 name=\"ambientIn\" /> \r\n"
		"		<float3 name=\"incandescence\" /> \r\n"
		"	</properties> \r\n"
		"	<values> \r\n"
		"		<float3 name=\"diffuseIrradIn\" value=\"0.0,0.0,0.0\" /> \r\n"
		"		<float3 name=\"specularIrradIn\" value=\"0.0,0.0,0.0\" /> \r\n"
		"		<float3 name=\"color\" value=\"0.5,0.5,0.5\" /> \r\n"
		"		<float3 name=\"transparency\" value=\"0.0,0.0,0.0\" /> \r\n"
		"		<float3 name=\"ambientColor\" value=\"0.0,0.0,0.0\" /> \r\n"
		"		<float3 name=\"ambientIn\" value=\"0.0,0.0,0.0\" /> \r\n"
		"		<float3 name=\"incandescence\" value=\"0.0,0.0,0.0\" /> \r\n"
		"	</values> \r\n"
		"	<outputs> \r\n"
		"		<struct name=\"onbCombiner\" struct_name=\"onbShaderOutput\" /> \r\n"
		"	</outputs> \r\n"
		"	<implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"Cg\" lang_version=\"2.1\"> \r\n"
		"		<function_name val=\"onbCombiner\" /> \r\n"
		"		<source><![CDATA[ \r\n"
		"onbShaderOutput onbCombiner( \r\n"
		"	float3 diffuseIrradIn, \r\n"
		"	float3 specularIrradIn, \r\n"
		"	float3 color, \r\n"
		"	float3 transparency, \r\n"
		"	float3 ambientColor, \r\n"
		"	float3 ambientIn, \r\n"
		"	float3 incandescence) \r\n"
		"{ \r\n"
		"	onbShaderOutput result; \r\n"
		"	result.outColor = (ambientColor+ambientIn)*color; \r\n"
		"	result.outColor += diffuseIrradIn*color; \r\n"
		"	float3 opacity = saturate(1.0f - transparency);  \r\n"
		"	result.outColor *= opacity; \r\n"
		"	result.outColor += specularIrradIn; \r\n"
		"	result.outColor += incandescence; \r\n"
		"	result.outTransparency = transparency; \r\n"
		"	const float3 intenseVec = float3(0.3333, 0.3333, 0.3333);  \r\n"
		"	result.outSurfaceFinal = float4(result.outColor, dot(opacity, intenseVec));  \r\n"
		"	return result; \r\n"
		"} \r\n"
		"		]]></source> \r\n"
		"	</implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"HLSL\" lang_version=\"11.0\"> \r\n"
		"		<function_name val=\"onbCombiner\" /> \r\n"
		"		<source><![CDATA[ \r\n"
		"onbShaderOutput onbCombiner( \r\n"
		"	float3 diffuseIrradIn, \r\n"
		"	float3 specularIrradIn, \r\n"
		"	float3 color, \r\n"
		"	float3 transparency, \r\n"
		"	float3 ambientColor, \r\n"
		"	float3 ambientIn, \r\n"
		"	float3 incandescence) \r\n"
		"{ \r\n"
		"	onbShaderOutput result; \r\n"
		"	result.outColor = (ambientColor+ambientIn)*color; \r\n"
		"	result.outColor += diffuseIrradIn*color; \r\n"
		"	float3 opacity = saturate(1.0f - transparency);  \r\n"
		"	result.outColor *= opacity; \r\n"
		"	result.outColor += specularIrradIn; \r\n"
		"	result.outColor += incandescence; \r\n"
		"	result.outTransparency = transparency; \r\n"
		"	const float3 intenseVec = float3(0.3333, 0.3333, 0.3333);  \r\n"
		"	result.outSurfaceFinal = float4(result.outColor, dot(opacity, intenseVec));  \r\n"
		"	return result; \r\n"
		"} \r\n"
		"		]]></source> \r\n"
		"	</implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"GLSL\" lang_version=\"3.0\"> \r\n"
		"		<function_name val=\"onbCombiner\" /> \r\n"
		"		<source><![CDATA[ \r\n"
		"onbShaderOutput onbCombiner( \r\n"
		"	vec3 diffuseIrradIn, \r\n"
		"	vec3 specularIrradIn, \r\n"
		"	vec3 color, \r\n"
		"	vec3 transparency, \r\n"
		"	vec3 ambientColor, \r\n"
		"	vec3 ambientIn, \r\n"
		"	vec3 incandescence) \r\n"
		"{ \r\n"
		"	onbShaderOutput result; \r\n"
		"	result.outColor = (ambientColor+ambientIn)*color; \r\n"
		"	result.outColor += diffuseIrradIn*color; \r\n"
		"	vec3 opacity = saturate(1.0f - transparency);  \r\n"
		"	result.outColor *= opacity; \r\n"
		"	result.outColor += specularIrradIn; \r\n"
		"	result.outColor += incandescence; \r\n"
		"	result.outTransparency = transparency; \r\n"
		"	const vec3 intenseVec = vec3(0.3333, 0.3333, 0.3333);  \r\n"
		"	result.outSurfaceFinal = vec4(result.outColor, dot(opacity, intenseVec));  \r\n"
		"	return result; \r\n"
		"} \r\n"
		"		]]></source> \r\n"
		"	</implementation> \r\n"
		"	</implementation> \r\n"
		"</fragment> \r\n";
	if (fragmentName != fragmentMgr->addShadeFragmentFromBuffer(fragmentBody, false)) return MS::kFailure;

	// ------------------------------------------------------------------------
	// Full, final graph made up of previously registered fragments. A picture
	// which flows from top to bottom.
	//
	//      onbFloat3PassThrough(Nw)--------------------\
	//            |                                     |
	//            |  nbFloat3PassThrough(Vw)---------\  |
	//            |   |                              |  |
	//            |   |    nbFloat3PassThrough(Lw)   |  |
	//            |   |     |       |                |  |
	//            |   |     |       |                |  |
	//            onbShaderGeom     \-------------\  |  |
	//                |    |                      |  |  |
	//                |    \-------------------\  |  |  |
	//                |                        |  |  |  |
	//                |                        |  |  |  |
	//          onbSpecular                    onbDiffuse
	//                     \                  /
	//                      \                /
	//                       onb16LightAccum
	//                              |
	//                         onbCombiner
	//                              |
	//                        onbShaderOutput
	//                              |
	//                        <FINAL RESULT>
	//
	// Maya's shader translation system will take this graph and connect light
	// information where appropriate, allowing this to be inserted in the body
	// of the light loop.
	//
	// From Maya's point of view, this graph is fully representative of the
	// associated shading node in the DG. All inputs and outputs on this graph
	// are matched to inputs and outputs on the DG node. Connections to other
	// nodes (input or output) will be mirrored by connections to other
	// fragments using the matching parameters.
	//
	// The parameters Lw and HLw are automatically hooked up to the appropriate
	// light fragments by Maya. Nw and Vw are varying parameters that are
	// automatically generated by the vertex shader.
	//
	fragmentName = sFinalFragmentGraphName;
	fragmentBody =
		"<fragment_graph name=\"onbShaderSurface\" ref=\"onbShaderSurface\" class=\"FragmentGraph\" version=\"1.0\"> \r\n"
		"	<fragments> \r\n"
		"		<fragment_ref name=\"nwPassThrough\" ref=\"onbFloat3PassThrough\" /> \r\n"
		"		<fragment_ref name=\"vwPassThrough\" ref=\"onbFloat3PassThrough\" /> \r\n"
		"		<fragment_ref name=\"lwPassThrough\" ref=\"onbFloat3PassThrough\" /> \r\n"
		"		<fragment_ref name=\"onbShaderGeom\" ref=\"onbShaderGeom\" /> \r\n"
		"		<fragment_ref name=\"onbDiffuse\" ref=\"onbDiffuse\" /> \r\n"
		"		<fragment_ref name=\"onbSpecular\" ref=\"onbSpecular\" /> \r\n"
		"		<fragment_ref name=\"onb16LightAccum\" ref=\"onb16LightAccum\" /> \r\n"
		"		<fragment_ref name=\"onbCombiner\" ref=\"onbCombiner\" /> \r\n"
		"		<fragment_ref name=\"onbShaderOutput\" ref=\"onbShaderOutput\" /> \r\n"
		"	</fragments> \r\n"
		"	<connections> \r\n"
		"		<connect from=\"nwPassThrough.output\" to=\"onbDiffuse.Nw\" name=\"Nw\" /> \r\n"
		"		<connect from=\"vwPassThrough.output\" to=\"onbDiffuse.Vw\" name=\"Vw\" /> \r\n"
		"		<connect from=\"lwPassThrough.output\" to=\"onbDiffuse.Lw\" name=\"Lw\" /> \r\n"
		"		<connect from=\"nwPassThrough.output\" to=\"onbShaderGeom.Nw\" name=\"Nw\" /> \r\n"
		"		<connect from=\"vwPassThrough.output\" to=\"onbShaderGeom.Vw\" name=\"Vw\" /> \r\n"
		"		<connect from=\"lwPassThrough.output\" to=\"onbShaderGeom.Lw\" name=\"Lw\" /> \r\n"
		"		<connect from=\"onbShaderGeom.onbShaderGeom\" to=\"onbDiffuse.shaderGeomInput\" name=\"shaderGeomInput\" /> \r\n"
		"		<connect from=\"onbShaderGeom.onbShaderGeom\" to=\"onbSpecular.shaderGeomInput\" name=\"shaderGeomInput\" /> \r\n"
		"		<connect from=\"onbDiffuse.onbDiffuse\" to=\"onb16LightAccum.scaledDiffuse\" name=\"scaledDiffuse\" /> \r\n"
		"		<connect from=\"onbSpecular.onbSpecular\" to=\"onb16LightAccum.scaledSpecular\" name=\"scaledSpecular\" /> \r\n"
		"		<connect from=\"onb16LightAccum.scaledDiffuse\" to=\"onbCombiner.diffuseIrradIn\" name=\"diffuseIrradIn\" /> \r\n"
		"		<connect from=\"onb16LightAccum.scaledSpecular\" to=\"onbCombiner.specularIrradIn\" name=\"specularIrradIn\" /> \r\n"
		"		<connect from=\"onbCombiner.onbCombiner\" to=\"onbShaderOutput.onbShaderOutput\" name=\"onbShaderOutput\" /> \r\n"
		"	</connections> \r\n"
		"	<properties> \r\n"
		"		<float3 name=\"Nw\" ref=\"nwPassThrough.input\" semantic=\"Nw\" flags=\"varyingInputParam\" /> \r\n"
		"		<float3 name=\"Vw\" ref=\"vwPassThrough.input\" semantic=\"Vw\" flags=\"varyingInputParam\" /> \r\n"
		"		<float3 name=\"Lw\" ref=\"lwPassThrough.input\" /> \r\n"
		"		<float3 name=\"HLw\" ref=\"onbShaderGeom.HLw\" /> \r\n"
		"		<float3 name=\"diffuseI\" ref=\"onbDiffuse.diffuseI\" /> \r\n"
		"		<float name=\"roughness\" ref=\"onbDiffuse.roughness\" /> \r\n"
		"		<float3 name=\"specularI\" ref=\"onbSpecular.specularI\" /> \r\n"
		"		<float3 name=\"specularColor\" ref=\"onbSpecular.specularColor\" /> \r\n"
		"		<float name=\"eccentricity\" ref=\"onbSpecular.eccentricity\" /> \r\n"
		"		<float name=\"specularRollOff\" ref=\"onbSpecular.specularRollOff\" /> \r\n"
		"		<string name=\"selector\" ref=\"onb16LightAccum.selector\" /> \r\n"
		"		<float3 name=\"color\" ref=\"onbCombiner.color\" /> \r\n"
		"		<float3 name=\"transparency\" ref=\"onbCombiner.transparency\" /> \r\n"
		"		<float3 name=\"ambientColor\" ref=\"onbCombiner.ambientColor\" /> \r\n"
		"		<float3 name=\"ambientIn\" ref=\"onbCombiner.ambientIn\" /> \r\n"
		"		<float3 name=\"incandescence\" ref=\"onbCombiner.incandescence\" /> \r\n"
		"	</properties> \r\n"
		"	<values> \r\n"
		"		<float3 name=\"Lw\" value=\"0.0,0.0,0.0\" /> \r\n"
		"		<float3 name=\"HLw\" value=\"0.0,0.0,0.0\" /> \r\n"
		"		<float3 name=\"diffuseI\" value=\"0.0,0.0,0.0\" /> \r\n"
		"		<float name=\"roughness\" value=\"0.3\" /> \r\n"
		"		<float3 name=\"specularI\" value=\"0.0,0.0,0.0\" /> \r\n"
		"		<float3 name=\"specularColor\" value=\"1.0,1.0,1.0\" /> \r\n"
		"		<float name=\"eccentricity\" value=\"0.1\" /> \r\n"
		"		<float name=\"specularRollOff\" value=\"0.7\" /> \r\n"
		"		<string name=\"selector\" value=\"mayaLightSelector16\" /> \r\n"
		"		<float3 name=\"color\" value=\"0.5,0.5,0.5\" /> \r\n"
		"		<float3 name=\"transparency\" value=\"0.0,0.0,0.0\" /> \r\n"
		"		<float3 name=\"ambientColor\" value=\"0.0,0.0,0.0\" /> \r\n"
		"		<float3 name=\"ambientIn\" value=\"0.0,0.0,0.0\" /> \r\n"
		"		<float3 name=\"incandescence\" value=\"0.0,0.0,0.0\" /> \r\n"
		"	</values> \r\n"
		"	<outputs> \r\n"
		"		<struct name=\"onbShaderOutput\" ref=\"onbShaderOutput.onbShaderOutput\" /> \r\n"
		"	</outputs> \r\n"
		"</fragment_graph> \r\n";
	if (fragmentName != fragmentMgr->addFragmentGraphFromBuffer(fragmentBody)) return MS::kFailure;

	return MS::kSuccess;
}

MStatus onbShaderOverride::deregisterFragments()
{
	// Get fragment manager for deregistration
	MHWRender::MFragmentManager* fragmentMgr =
		MHWRender::MRenderer::theRenderer()
			? MHWRender::MRenderer::theRenderer()->getFragmentManager()
			: NULL;
	// No fragment manager, fail
	if (!fragmentMgr) return MS::kFailure;

	// Remove fragments from library on plugin unload
	if (!fragmentMgr->removeFragment("onbFloat3PassThrough")) return MS::kFailure;
	if (!fragmentMgr->removeFragment("onbShaderGeom")) return MS::kFailure;
	if (!fragmentMgr->removeFragment("onbDiffuse")) return MS::kFailure;
	if (!fragmentMgr->removeFragment("onbSpecular")) return MS::kFailure;
	if (!fragmentMgr->removeFragment("onb16LightAccum")) return MS::kFailure;
	if (!fragmentMgr->removeFragment("onbShaderOutput")) return MS::kFailure;
	if (!fragmentMgr->removeFragment("onbCombiner")) return MS::kFailure;
	if (!fragmentMgr->removeFragment(sFinalFragmentGraphName)) return MS::kFailure;

	return MS::kSuccess;
}

