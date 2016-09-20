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
// Example Plugin: simpleNoiseShader.cpp
//
// Produces dependency graph node simpleNoise
// This node is an example of how to build a dependency node as a texture
// shader in Maya. The inputs for this node are many, and can be found in
// the Maya UI on the Attribute Editor for the node. The output attributes for
// the node are outColor and outAlpha.
//
// The actual texture implemented here is based on Maya's "wave" noise type
// from the 2d procedural noise texture node.
//
// In addition to implementing the dependency node, this plug-in also shows a
// complete implementation of a texture shader for VP2. See
// simpleNoiseShaderOverride.cpp for details.
//

#include <maya/MPxNode.h>
#include <maya/MFnPlugin.h>
#include <maya/MIOStream.h>
#include <maya/MDrawRegistry.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MRenderUtil.h>

#include "simpleNoiseShaderOverride.h"

class SimpleNoiseShader : public MPxNode
{
public:
	static void* creator();
	static MStatus initialize();
	static const MTypeId id;
	static const MString nodeName;
	static const MString drawDbClassification;
	static const MString classification;

	SimpleNoiseShader();
	virtual ~SimpleNoiseShader();

	virtual MStatus compute(const MPlug&, MDataBlock&);
	virtual void postConstructor();

private:
	// Output attributes
	static MObject aOutColor;
	static MObject aOutAlpha;

	// Input attributes
	static MObject aUVCoord;
	static MObject aFilterSize;
	static MObject aAmplitude;
	static MObject aRatio;
	static MObject aDepthMax;
	static MObject aFrequency;
	static MObject aFrequencyRatio;
	static MObject aTime;
	static MObject aNumWaves;
};

// Static data
const MTypeId SimpleNoiseShader::id(0x00080FFE);
const MString SimpleNoiseShader::nodeName("simpleNoise");
const MString SimpleNoiseShader::drawDbClassification("drawdb/shader/texture/2d/" + SimpleNoiseShader::nodeName);
const MString SimpleNoiseShader::classification("texture/2d:" + drawDbClassification);

// Attributes
MObject SimpleNoiseShader::aOutColor;
MObject SimpleNoiseShader::aOutAlpha;
MObject SimpleNoiseShader::aUVCoord;
MObject SimpleNoiseShader::aFilterSize;
MObject SimpleNoiseShader::aAmplitude;
MObject SimpleNoiseShader::aRatio;
MObject SimpleNoiseShader::aDepthMax;
MObject SimpleNoiseShader::aFrequency;
MObject SimpleNoiseShader::aFrequencyRatio;
MObject SimpleNoiseShader::aTime;
MObject SimpleNoiseShader::aNumWaves;

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
void SimpleNoiseShader::postConstructor()
{
	setMPSafe(true);
}

SimpleNoiseShader::SimpleNoiseShader()
{
}

SimpleNoiseShader::~SimpleNoiseShader()
{
}

void* SimpleNoiseShader::creator()
{
	return new SimpleNoiseShader();
}


MStatus SimpleNoiseShader::initialize()
{
	MFnNumericAttribute nAttr;

	// Outputs
	MObject outcR = nAttr.create("outColorR", "ocr", MFnNumericData::kFloat, 0.0);
	MObject outcG = nAttr.create("outColorG", "ocg", MFnNumericData::kFloat, 0.0);
	MObject outcB = nAttr.create("outColorB", "ocb", MFnNumericData::kFloat, 0.0);
	aOutColor = nAttr.create("outColor", "oc", outcR, outcG, outcB);
	MAKE_OUTPUT(nAttr);
	CHECK_MSTATUS(nAttr.setUsedAsColor(true));

	aOutAlpha = nAttr.create("outAlpha", "oa", MFnNumericData::kFloat);
	MAKE_OUTPUT(nAttr);

	// Inputs
	MObject child1 = nAttr.create("uCoord", "u", MFnNumericData::kFloat);
	MObject child2 = nAttr.create("vCoord", "v", MFnNumericData::kFloat);
	aUVCoord = nAttr.create("uvCoord", "uv", child1, child2);
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setHidden(true));

	child1 = nAttr.create("uvFilterSizeX", "fsx", MFnNumericData::kFloat);
	child2 = nAttr.create("uvFilterSizeY", "fsy", MFnNumericData::kFloat);
	aFilterSize = nAttr.create("uvFilterSize", "fs", child1, child2);
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setHidden(true));

	aAmplitude = nAttr.create("amplitude", "a", MFnNumericData::kFloat);
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setMin(0.0f));
	CHECK_MSTATUS(nAttr.setMax(1.0f));
	CHECK_MSTATUS(nAttr.setDefault(1.0f));

	aRatio = nAttr.create("ratio", "ra", MFnNumericData::kFloat);
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setMin(0.0f));
	CHECK_MSTATUS(nAttr.setMax(1.0f));
	CHECK_MSTATUS(nAttr.setDefault(0.707f));

	aDepthMax = nAttr.create("depthMax", "dm", MFnNumericData::kShort);
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setMin(1));
	CHECK_MSTATUS(nAttr.setMax(80));
	CHECK_MSTATUS(nAttr.setDefault(3));

	aFrequency = nAttr.create("frequency", "fq", MFnNumericData::kFloat);
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setMin(0.0f));
	CHECK_MSTATUS(nAttr.setMax(100.0f));
	CHECK_MSTATUS(nAttr.setDefault(8.0f));

	aFrequencyRatio = nAttr.create("frequencyRatio", "fr", MFnNumericData::kFloat);
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setMin(1.0f));
	CHECK_MSTATUS(nAttr.setMax(10.0f));
	CHECK_MSTATUS(nAttr.setDefault(2.0f));

	aTime = nAttr.create("time", "ti", MFnNumericData::kFloat);
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setMin(0.0f));
	CHECK_MSTATUS(nAttr.setMax(1.0f));
	CHECK_MSTATUS(nAttr.setDefault(0.0f));

	aNumWaves = nAttr.create("numWaves", "nw", MFnNumericData::kShort);
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setMin(1));
	CHECK_MSTATUS(nAttr.setMax(20));
	CHECK_MSTATUS(nAttr.setDefault(5));

	// Add Attributes
	CHECK_MSTATUS(addAttribute(aOutColor));
	CHECK_MSTATUS(addAttribute(aOutAlpha));
	CHECK_MSTATUS(addAttribute(aUVCoord));
	CHECK_MSTATUS(addAttribute(aFilterSize));
	CHECK_MSTATUS(addAttribute(aAmplitude));
	CHECK_MSTATUS(addAttribute(aRatio));
	CHECK_MSTATUS(addAttribute(aDepthMax));
	CHECK_MSTATUS(addAttribute(aFrequency));
	CHECK_MSTATUS(addAttribute(aFrequencyRatio));
	CHECK_MSTATUS(addAttribute(aTime));
	CHECK_MSTATUS(addAttribute(aNumWaves));

	// Attribute affects relationships
	CHECK_MSTATUS(attributeAffects(aUVCoord, aOutColor));
	CHECK_MSTATUS(attributeAffects(aUVCoord, aOutAlpha));
	CHECK_MSTATUS(attributeAffects(aFilterSize, aOutColor));
	CHECK_MSTATUS(attributeAffects(aFilterSize, aOutAlpha));
	CHECK_MSTATUS(attributeAffects(aAmplitude, aOutColor));
	CHECK_MSTATUS(attributeAffects(aAmplitude, aOutAlpha));
	CHECK_MSTATUS(attributeAffects(aRatio, aOutColor));
	CHECK_MSTATUS(attributeAffects(aRatio, aOutAlpha));
	CHECK_MSTATUS(attributeAffects(aDepthMax, aOutColor));
	CHECK_MSTATUS(attributeAffects(aDepthMax, aOutAlpha));
	CHECK_MSTATUS(attributeAffects(aFrequency, aOutColor));
	CHECK_MSTATUS(attributeAffects(aFrequency, aOutAlpha));
	CHECK_MSTATUS(attributeAffects(aFrequencyRatio, aOutColor));
	CHECK_MSTATUS(attributeAffects(aFrequencyRatio, aOutAlpha));
	CHECK_MSTATUS(attributeAffects(aTime, aOutColor));
	CHECK_MSTATUS(attributeAffects(aTime, aOutAlpha));
	CHECK_MSTATUS(attributeAffects(aNumWaves, aOutColor));
	CHECK_MSTATUS(attributeAffects(aNumWaves, aOutAlpha));

	return MS::kSuccess;
}

MStatus SimpleNoiseShader::compute(const MPlug& plug, MDataBlock& block)
{
	// Sanity check
	if (plug != aOutColor && plug.parent() != aOutColor && plug != aOutAlpha)
	{
		return MS::kUnknownParameter;
	}

	MStatus status;
	MFloatVector resultColor(0.0f, 0.0f, 0.0f);
	float resultAlpha = 1.0f;

	// Get attribute values
	const float2& uv = block.inputValue(aUVCoord, &status).asFloat2();
	CHECK_MSTATUS(status);
	float amplitude = block.inputValue(aAmplitude, &status).asFloat();
	CHECK_MSTATUS(status);
	const float ratio = block.inputValue(aRatio, &status).asFloat();
	CHECK_MSTATUS(status);
	const unsigned int depthMax = block.inputValue(aDepthMax, &status).asShort();
	CHECK_MSTATUS(status);
	const float frequency = block.inputValue(aFrequency, &status).asFloat();
	CHECK_MSTATUS(status);
	const float frequencyRatio = block.inputValue(aFrequencyRatio, &status).asFloat();
	CHECK_MSTATUS(status);
	float time = block.inputValue(aTime, &status).asFloat();
	CHECK_MSTATUS(status);
	const unsigned int numWaves = block.inputValue(aNumWaves, &status).asShort();
	CHECK_MSTATUS(status);

	// Compute noise
	static const float fM_PI = (float)M_PI;
	static const float M_2PI = 2.0f*fM_PI;
	const float timeRatio = sqrt(frequencyRatio);
	float uvX = uv[0] * frequency;
	float uvY = uv[1] * frequency;
	float cosine = 0.0f;
	float noise = 0.0f;
	unsigned int depthId = 0;
	unsigned int waveId = 0;
	unsigned int seedOffset = 0;
	while (depthId<depthMax && waveId<numWaves)
	{
		const unsigned int step = depthId;
		const unsigned int seed = 50*step;
		float dirX = MRenderUtil::valueInNoiseTable(seed + seedOffset++);
		float dirY = MRenderUtil::valueInNoiseTable(seed + seedOffset++);
		const float norm = sqrtf(dirX*dirX + dirY*dirY);
		if (norm <= 0.0f) continue;

		dirX /= norm;
		dirY /= norm;
		noise += cosf(
			dirX*uvX*M_2PI +
			dirY*uvY*M_2PI +
			fM_PI*MRenderUtil::valueInNoiseTable(seed + seedOffset++) +
			time*fM_PI);

		++waveId;
		if (waveId < numWaves) continue;
		noise /= float(numWaves);
		uvX *= frequencyRatio;
		uvY *= frequencyRatio;
		time *= timeRatio;
		cosine += amplitude * noise;
		amplitude *= ratio;
		noise = 0.0f;
		waveId = 0;
		seedOffset = 0;
		++depthId;
	}
	cosine = 0.5f*cosine + 0.5f;
	const float noiseVal = (cosine > 1.0f) ? 1.0f : cosine;

	// Set results
	resultColor[0] = resultColor[1] = resultColor[2] = noiseVal;
	resultAlpha = noiseVal;

	// Set output color attribute
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

	// Set output alpha attribute
	if (plug == aOutAlpha)
	{
		// Get the handle to the attribute
		MDataHandle outAlphaHandle =
			block.outputValue(aOutAlpha, &status);
		CHECK_MSTATUS(status);
		float& outAlpha = outAlphaHandle.asFloat();

		// Set the result and mark it clean
		outAlpha = resultAlpha;
		outAlphaHandle.setClean();
	}

	return MS::kSuccess;
}

///////////////////////////////////////////////////////
// Init/Un-init functions
///////////////////////////////////////////////////////
static const MString sRegistrantId("simpleNoiseShaderPlugin");
MStatus initializePlugin(MObject obj)
{
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "1.0", "Any");

	CHECK_MSTATUS(plugin.registerNode(
		SimpleNoiseShader::nodeName,
		SimpleNoiseShader::id,
		SimpleNoiseShader::creator,
		SimpleNoiseShader::initialize,
		MPxNode::kDependNode,
		&SimpleNoiseShader::classification));

	CHECK_MSTATUS(
		MHWRender::MDrawRegistry::registerShadingNodeOverrideCreator(
			SimpleNoiseShader::drawDbClassification,
			sRegistrantId,
			simpleNoiseShaderOverride::creator));

	CHECK_MSTATUS(simpleNoiseShaderOverride::registerFragments());

	return MS::kSuccess;
}

MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);

	CHECK_MSTATUS(plugin.deregisterNode(SimpleNoiseShader::id));

	CHECK_MSTATUS(
		MHWRender::MDrawRegistry::deregisterShadingNodeOverrideCreator(
			SimpleNoiseShader::drawDbClassification,
			sRegistrantId));

	CHECK_MSTATUS(simpleNoiseShaderOverride::deregisterFragments());

	return MS::kSuccess;
}

