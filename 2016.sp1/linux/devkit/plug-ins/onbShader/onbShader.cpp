//-
// ===========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================
//+

//
// Example Plugin: onbShader.cpp
//
// Produces dependency graph node onbShader
// This node is an example of how to build a dependency node as a surface
// shader in Maya. The inputs for this node are many, and can be found in
// the Maya UI on the Attribute Editor for the node. The output attributes for
// the node are outColor and outTransparency. To use this shader, create an
// onbShader with Shading Group or connect the outputs to a Shading Group's
// "surfaceShader" attribute.
//
// The actual shader implemented here uses a simplified Oren-Nayar diffuse
// model based on:
//		Engel, Wolfgang et al.: Programming Vertex, Geometry, and Pixel Shaders
//		http://content.gpwiki.org/index.php/D3DBook:(Lighting)_Oren-Nayar
// The diffuse computation is combined with Blinn specular highlighting.
//
// In addition to implementing the dependency node, this plug-in also shows a
// complete implementation of a surface shader for VP2. Unlike lambertShader or
// phongShader this sample does not re-use Maya's built in fragments. All
// fragment code is defined by the plug-in. See onbShaderOverride.cpp for
// details.
//

#include <maya/MPxNode.h>
#include <maya/MFnPlugin.h>
#include <maya/MIOStream.h>
#include <maya/MDrawRegistry.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnLightDataAttribute.h>
#include <algorithm> // for min/max

#include "onbShaderOverride.h"

class OnbShader : public MPxNode
{
public:
	static void* creator();
	static MStatus initialize();
	static MTypeId id;
	static MString nodeName;
	static MString drawDbClassification;
	static MString classification;

	OnbShader();
	virtual ~OnbShader();

	virtual MStatus compute(const MPlug&, MDataBlock&);
	virtual void postConstructor();

private:
	// Output attributes
	static MObject aOutColor;
	static MObject aOutTransparency;

	// Shader attributes
	static MObject aColor;
	static MObject aRoughness;
	static MObject aTransparency;
	static MObject aAmbientColor;
	static MObject aIncandescence;
	static MObject aSpecularColor;
	static MObject aEccentricity;
	static MObject aSpecularRollOff;
	static MObject aNormalCamera;

	// Light attributes for compute()
	static MObject aRayDirection;
	static MObject aLightDirection;
	static MObject aLightIntensity;
	static MObject aLightAmbient;
	static MObject aLightDiffuse;
	static MObject aLightSpecular;
	static MObject aLightShadowFraction;
	static MObject aPreShadowIntensity;
	static MObject aLightBlindData;
	static MObject aLightData;
};

// Static data
MTypeId OnbShader::id(0x00080FFF);
MString OnbShader::nodeName("onbShader");
MString OnbShader::drawDbClassification("drawdb/shader/surface/onbShader");
MString OnbShader::classification("shader/surface:" + drawDbClassification);

// Attributes
MObject OnbShader::aOutColor;
MObject OnbShader::aOutTransparency;
MObject OnbShader::aColor;
MObject OnbShader::aRoughness;
MObject OnbShader::aTransparency;
MObject OnbShader::aAmbientColor;
MObject OnbShader::aIncandescence;
MObject OnbShader::aSpecularColor;
MObject OnbShader::aEccentricity;
MObject OnbShader::aSpecularRollOff;
MObject OnbShader::aNormalCamera;
MObject OnbShader::aRayDirection;
MObject OnbShader::aLightDirection;
MObject OnbShader::aLightIntensity;
MObject OnbShader::aLightAmbient;
MObject OnbShader::aLightDiffuse;
MObject OnbShader::aLightSpecular;
MObject OnbShader::aLightShadowFraction;
MObject OnbShader::aPreShadowIntensity;
MObject OnbShader::aLightBlindData;
MObject OnbShader::aLightData;

// Helper pre-processor defines
#define MAKE_INPUT(attr) \
	CHECK_MSTATUS(attr.setKeyable(true)); \
	CHECK_MSTATUS(attr.setStorable(true)); \
	CHECK_MSTATUS(attr.setReadable(true)); \
	CHECK_MSTATUS(attr.setWritable(true));

#define MAKE_OUTPUT(attr) \
	CHECK_MSTATUS(attr.setKeyable(false)); \
	CHECK_MSTATUS(attr.setStorable(false)); \
	CHECK_MSTATUS(attr.setReadable(true)); \
	CHECK_MSTATUS(attr.setWritable(false));

// Methods
void OnbShader::postConstructor()
{
	setMPSafe(true);
}

OnbShader::OnbShader()
{
}

OnbShader::~OnbShader()
{
}

void* OnbShader::creator()
{
	return new OnbShader();
}

MStatus OnbShader::initialize()
{
	MFnNumericAttribute nAttr;
	MFnLightDataAttribute lAttr;

	// Outputs
	MObject outcR = nAttr.create("outColorR", "ocr", MFnNumericData::kFloat, 0.0);
	MObject outcG = nAttr.create("outColorG", "ocg", MFnNumericData::kFloat, 0.0);
	MObject outcB = nAttr.create("outColorB", "ocb", MFnNumericData::kFloat, 0.0);
	aOutColor = nAttr.create("outColor", "oc", outcR, outcG, outcB);
	MAKE_OUTPUT(nAttr);
	CHECK_MSTATUS(nAttr.setUsedAsColor(true));

	MObject outtR = nAttr.create("outTransparencyR", "otr", MFnNumericData::kFloat, 0.0);
	MObject outtG = nAttr.create("outTransparencyG", "otg", MFnNumericData::kFloat, 0.0);
	MObject outtB = nAttr.create("outTransparencyB", "otb", MFnNumericData::kFloat, 0.0);
	aOutTransparency = nAttr.create("outTransparency", "ot", outtR, outtG, outtB);
	MAKE_OUTPUT(nAttr);
	CHECK_MSTATUS(nAttr.setUsedAsColor(true));

	// Inputs
	MObject cR = nAttr.create("colorR", "cr", MFnNumericData::kFloat, 0.5);
	MObject cG = nAttr.create("colorG", "cg", MFnNumericData::kFloat, 0.5);
	MObject cB = nAttr.create("colorB", "cb", MFnNumericData::kFloat, 0.5);
	aColor = nAttr.create("color", "c", cR, cG, cB);
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setUsedAsColor(true));

	aRoughness = nAttr.create("roughness", "r", MFnNumericData::kFloat);
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setMin(0.0f));
	CHECK_MSTATUS(nAttr.setMax(1.0f));
	CHECK_MSTATUS(nAttr.setDefault(0.3f));

	MObject tR = nAttr.create("transparencyR", "itr", MFnNumericData::kFloat, 0.0);
	MObject tG = nAttr.create("transparencyG", "itg", MFnNumericData::kFloat, 0.0);
	MObject tB = nAttr.create("transparencyB", "itb", MFnNumericData::kFloat, 0.0);
	aTransparency = nAttr.create("transparency", "it", tR, tG, tB);
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setUsedAsColor(true));

	MObject aR = nAttr.create("ambientColorR", "acr", MFnNumericData::kFloat, 0.0);
	MObject aG = nAttr.create("ambientColorG", "acg", MFnNumericData::kFloat, 0.0);
	MObject aB = nAttr.create("ambientColorB", "acb", MFnNumericData::kFloat, 0.0);
	aAmbientColor = nAttr.create("ambientColor", "ambc", aR, aG, aB);
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setUsedAsColor(true));

	MObject iR = nAttr.create("incandescenceR", "ir", MFnNumericData::kFloat, 0.0);
	MObject iG = nAttr.create("incandescenceG", "ig", MFnNumericData::kFloat, 0.0);
	MObject iB = nAttr.create("incandescenceB", "ib", MFnNumericData::kFloat, 0.0);
	aIncandescence = nAttr.create("incandescence", "ic", iR, iG, iB);
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setUsedAsColor(true));

	MObject sR = nAttr.create("specularColorR", "sr", MFnNumericData::kFloat, 1.0);
	MObject sG = nAttr.create("specularColorG", "sg", MFnNumericData::kFloat, 1.0);
	MObject sB = nAttr.create("specularColorB", "sb", MFnNumericData::kFloat, 1.0);
	aSpecularColor = nAttr.create("specularColor", "sc", sR, sG, sB);
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setUsedAsColor(true));

	aEccentricity = nAttr.create("eccentricity", "ecc", MFnNumericData::kFloat);
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setMin(0.0f));
	CHECK_MSTATUS(nAttr.setMax(1.0f));
	CHECK_MSTATUS(nAttr.setDefault(0.1f));

	aSpecularRollOff = nAttr.create("specularRollOff", "sro", MFnNumericData::kFloat);
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setMin(0.0f));
	CHECK_MSTATUS(nAttr.setMax(1.0f));
	CHECK_MSTATUS(nAttr.setDefault(0.7f));

	aNormalCamera = nAttr.createPoint("normalCamera", "n");
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setDefault(1.0f, 1.0f, 1.0f));

	// Attributes for compute()
	aRayDirection = nAttr.createPoint("rayDirection", "rd");
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setHidden(true));

	aLightDirection = nAttr.createPoint("lightDirection", "ld");
	CHECK_MSTATUS(nAttr.setStorable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));
	CHECK_MSTATUS(nAttr.setReadable(true));
	CHECK_MSTATUS(nAttr.setWritable(false));
	CHECK_MSTATUS(nAttr.setDefault(1.0f, 1.0f, 1.0f));

	aLightIntensity = nAttr.createColor("lightIntensity", "li");
	CHECK_MSTATUS(nAttr.setStorable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));
	CHECK_MSTATUS(nAttr.setReadable(true));
	CHECK_MSTATUS(nAttr.setWritable(false));
	CHECK_MSTATUS(nAttr.setDefault(1.0f, 1.0f, 1.0f));

	aLightAmbient = nAttr.create("lightAmbient", "la", MFnNumericData::kBoolean);
	CHECK_MSTATUS(nAttr.setStorable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));
	CHECK_MSTATUS(nAttr.setReadable(true));
	CHECK_MSTATUS(nAttr.setWritable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));

	aLightDiffuse = nAttr.create("lightDiffuse", "ldf", MFnNumericData::kBoolean);
	CHECK_MSTATUS(nAttr.setStorable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));
	CHECK_MSTATUS(nAttr.setReadable(true));
	CHECK_MSTATUS(nAttr.setWritable(false));

	aLightSpecular = nAttr.create("lightSpecular", "ls", MFnNumericData::kBoolean);
	CHECK_MSTATUS(nAttr.setStorable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));
	CHECK_MSTATUS(nAttr.setReadable(true));
	CHECK_MSTATUS(nAttr.setWritable(false));

	aLightShadowFraction = nAttr.create("lightShadowFraction", "lsf", MFnNumericData::kFloat);
	CHECK_MSTATUS(nAttr.setStorable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));
	CHECK_MSTATUS(nAttr.setReadable(true));
	CHECK_MSTATUS(nAttr.setWritable(false));

	aPreShadowIntensity = nAttr.create("preShadowIntensity", "psi", MFnNumericData::kFloat);
	CHECK_MSTATUS(nAttr.setStorable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));
	CHECK_MSTATUS(nAttr.setReadable(true));
	CHECK_MSTATUS(nAttr.setWritable(false));

	aLightBlindData = nAttr.createAddr("lightBlindData", "lbld");
	CHECK_MSTATUS(nAttr.setStorable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));
	CHECK_MSTATUS(nAttr.setReadable(true));
	CHECK_MSTATUS(nAttr.setWritable(false));

	aLightData = lAttr.create("lightDataArray", "ltd",
		aLightDirection,
		aLightIntensity,
		aLightAmbient,
		aLightDiffuse,
		aLightSpecular,
		aLightShadowFraction,
		aPreShadowIntensity,
		aLightBlindData);
	CHECK_MSTATUS(lAttr.setArray(true));
	CHECK_MSTATUS(lAttr.setStorable(false));
	CHECK_MSTATUS(lAttr.setHidden(true));
	CHECK_MSTATUS(lAttr.setDefault(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, 0.0f, 1.0f, NULL));

	// Add Attributes
	CHECK_MSTATUS(addAttribute(aOutColor));
	CHECK_MSTATUS(addAttribute(aOutTransparency));
	CHECK_MSTATUS(addAttribute(aColor));
	CHECK_MSTATUS(addAttribute(aRoughness));
	CHECK_MSTATUS(addAttribute(aTransparency));
	CHECK_MSTATUS(addAttribute(aAmbientColor));
	CHECK_MSTATUS(addAttribute(aIncandescence));
	CHECK_MSTATUS(addAttribute(aSpecularColor));
	CHECK_MSTATUS(addAttribute(aEccentricity));
	CHECK_MSTATUS(addAttribute(aSpecularRollOff));
	CHECK_MSTATUS(addAttribute(aNormalCamera));
	CHECK_MSTATUS(addAttribute(aRayDirection));
	CHECK_MSTATUS(addAttribute(aLightData));

	// Attribute affects relationships
	CHECK_MSTATUS(attributeAffects(aColor, aOutColor));
	CHECK_MSTATUS(attributeAffects(aColor, aOutTransparency));
	CHECK_MSTATUS(attributeAffects(aRoughness, aOutColor));
	CHECK_MSTATUS(attributeAffects(aRoughness, aOutTransparency));
	CHECK_MSTATUS(attributeAffects(aTransparency, aOutColor));
	CHECK_MSTATUS(attributeAffects(aTransparency, aOutTransparency));
	CHECK_MSTATUS(attributeAffects(aAmbientColor, aOutColor));
	CHECK_MSTATUS(attributeAffects(aAmbientColor, aOutTransparency));
	CHECK_MSTATUS(attributeAffects(aIncandescence, aOutColor));
	CHECK_MSTATUS(attributeAffects(aIncandescence, aOutTransparency));
	CHECK_MSTATUS(attributeAffects(aSpecularColor, aOutColor));
	CHECK_MSTATUS(attributeAffects(aSpecularColor, aOutTransparency));
	CHECK_MSTATUS(attributeAffects(aEccentricity, aOutColor));
	CHECK_MSTATUS(attributeAffects(aEccentricity, aOutTransparency));
	CHECK_MSTATUS(attributeAffects(aNormalCamera, aOutColor));
	CHECK_MSTATUS(attributeAffects(aNormalCamera, aOutTransparency));
	CHECK_MSTATUS(attributeAffects(aRayDirection, aOutColor));
	CHECK_MSTATUS(attributeAffects(aRayDirection, aOutTransparency));
	CHECK_MSTATUS(attributeAffects(aSpecularRollOff, aOutColor));
	CHECK_MSTATUS(attributeAffects(aSpecularRollOff, aOutTransparency));
	CHECK_MSTATUS(attributeAffects(aLightDirection, aOutColor));
	CHECK_MSTATUS(attributeAffects(aLightDirection, aOutTransparency));
	CHECK_MSTATUS(attributeAffects(aLightIntensity, aOutColor));
	CHECK_MSTATUS(attributeAffects(aLightIntensity, aOutTransparency));
	CHECK_MSTATUS(attributeAffects(aLightAmbient, aOutColor));
	CHECK_MSTATUS(attributeAffects(aLightAmbient, aOutTransparency));
	CHECK_MSTATUS(attributeAffects(aLightDiffuse, aOutColor));
	CHECK_MSTATUS(attributeAffects(aLightDiffuse, aOutTransparency));
	CHECK_MSTATUS(attributeAffects(aLightSpecular, aOutColor));
	CHECK_MSTATUS(attributeAffects(aLightSpecular, aOutTransparency));
	CHECK_MSTATUS(attributeAffects(aLightShadowFraction, aOutColor));
	CHECK_MSTATUS(attributeAffects(aLightShadowFraction, aOutTransparency));
	CHECK_MSTATUS(attributeAffects(aPreShadowIntensity, aOutColor));
	CHECK_MSTATUS(attributeAffects(aPreShadowIntensity, aOutTransparency));
	CHECK_MSTATUS(attributeAffects(aLightBlindData, aOutColor));
	CHECK_MSTATUS(attributeAffects(aLightBlindData, aOutTransparency));
	CHECK_MSTATUS(attributeAffects(aLightData, aOutColor));
	CHECK_MSTATUS(attributeAffects(aLightData, aOutTransparency));

	return MS::kSuccess;
}

MStatus OnbShader::compute(const MPlug& plug, MDataBlock& block)
{
	// Sanity check
	if (plug != aOutColor && plug.parent() != aOutColor &&
		plug != aOutTransparency && plug.parent() != aOutTransparency)
	{
		return MS::kUnknownParameter;
	}

	// Note that this currently only implements the diffuse portion of the
	// shader and ignores specular. The diffuse portion is the Oren-Nayar
	// computation from:
	//   Engel, Wolfgang et al. Programming Vertex, Geometry, and Pixel Shaders
	//   http://content.gpwiki.org/index.php/D3DBook:(Lighting)_Oren-Nayar
	// Further extensions could be added to this compute method to include
	// the intended Blinn specular component as well as ambient and
	// incandescence components.
	// See the VP2 fragment-based implementation in onbShaderOverride for the
	// full shader.
	MStatus status;
	MFloatVector resultColor(0.0f, 0.0f, 0.0f);
	MFloatVector resultTransparency(0.0f, 0.0f, 0.0f);

	// Get surface shading parameters from input block
	const MFloatVector& surfaceColor =
		block.inputValue(aColor, &status).asFloatVector();
	CHECK_MSTATUS(status);
	const float roughness = block.inputValue(aRoughness, &status).asFloat();
	CHECK_MSTATUS(status);
	const MFloatVector& transparency =
		block.inputValue(aTransparency, &status).asFloatVector();
	CHECK_MSTATUS(status);
	const MFloatVector& surfaceNormal =
		block.inputValue(aNormalCamera, &status).asFloatVector();
	CHECK_MSTATUS(status);
	const MFloatVector& rayDirection =
		block.inputValue(aRayDirection).asFloatVector();
	const MFloatVector viewDirection = -rayDirection;

	// Pre-compute some values that do not vary with lights
	const float NV = viewDirection*surfaceNormal;
	const float acosNV = acosf(NV);
	const float roughnessSq = roughness*roughness;
	const float A = 1.0f - 0.5f*(roughnessSq/(roughnessSq + 0.57f));
	const float B = 0.45f*(roughnessSq/(roughnessSq + 0.09f));

	// Get light list
	MArrayDataHandle lightData = block.inputArrayValue(aLightData, &status);
	CHECK_MSTATUS(status);
	const int numLights = lightData.elementCount(&status);
	CHECK_MSTATUS(status);

	// Iterate through light list and get ambient/diffuse values
	for (int count=1; count<=numLights; count++)
	{
		// Get the current light
		MDataHandle currentLight = lightData.inputValue(&status);
		CHECK_MSTATUS(status);

		// Find diffuse component
		if (currentLight.child(aLightDiffuse).asBool())
		{
			// Get the intensity and direction of that light
			const MFloatVector& lightIntensity =
				currentLight.child(aLightIntensity).asFloatVector();
			const MFloatVector& lightDirection =
				currentLight.child(aLightDirection).asFloatVector();

			// Compute the diffuse factor
			const float NL = lightDirection*surfaceNormal;
			const float acosNL = acosf(NL);
			const float alpha = std::max(acosNV, acosNL);
			const float beta = std::min(acosNV, acosNL);
			const float gamma =
				(viewDirection - (surfaceNormal*NV)) *
				(lightDirection - (surfaceNormal*NL));
			const float C = sinf(alpha)*tanf(beta);
			const float factor =
				std::max(0.0f, NL)*(A + B*std::max(0.0f, gamma)*C);

			// Add to result color
			resultColor += lightIntensity*factor;
		}

		// Advance to the next light.
		if (count < numLights)
		{
			status = lightData.next();
			CHECK_MSTATUS(status);
		}
	}

	// Factor incident light with surface color
	resultColor[0] = resultColor[0]*surfaceColor[0];
	resultColor[1] = resultColor[1]*surfaceColor[1];
	resultColor[2] = resultColor[2]*surfaceColor[2];

	// Set ouput color attribute
	if (plug == aOutColor || plug.parent() == aOutColor)
	{
		// Get the handle to the attribute
		MDataHandle outColorHandle = block.outputValue(aOutColor, &status);
		CHECK_MSTATUS(status);
		MFloatVector& outColor = outColorHandle.asFloatVector();

		// Set the result and mark it clean
		outColor = resultColor;
		outColorHandle.setClean();
	}

	// Set ouput transparency
	if (plug == aOutTransparency || plug.parent() == aOutTransparency)
	{
		// Get the handle to the attribute
		MDataHandle outTransHandle =
			block.outputValue(aOutTransparency, &status);
		CHECK_MSTATUS(status);
		MFloatVector& outTrans = outTransHandle.asFloatVector();

		// Set the result and mark it clean
		outTrans = transparency;
		outTransHandle.setClean();
	}

	return MS::kSuccess;
}



///////////////////////////////////////////////////////
// Init/Un-init fucntions
///////////////////////////////////////////////////////
static const MString sRegistrantId("onbShaderPlugin");
MStatus initializePlugin(MObject obj)
{
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "1.0", "Any");

	CHECK_MSTATUS(plugin.registerNode(
		OnbShader::nodeName,
		OnbShader::id,
		OnbShader::creator,
		OnbShader::initialize,
		MPxNode::kDependNode,
		&OnbShader::classification));

	CHECK_MSTATUS(
		MHWRender::MDrawRegistry::registerSurfaceShadingNodeOverrideCreator(
			OnbShader::drawDbClassification,
			sRegistrantId,
			onbShaderOverride::creator));

	CHECK_MSTATUS(onbShaderOverride::registerFragments());

	return MS::kSuccess;
}

MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);

	CHECK_MSTATUS(plugin.deregisterNode(OnbShader::id));

	CHECK_MSTATUS(
		MHWRender::MDrawRegistry::deregisterSurfaceShadingNodeOverrideCreator(
			OnbShader::drawDbClassification,
			sRegistrantId));

	CHECK_MSTATUS(onbShaderOverride::deregisterFragments());

	return MS::kSuccess;
}
