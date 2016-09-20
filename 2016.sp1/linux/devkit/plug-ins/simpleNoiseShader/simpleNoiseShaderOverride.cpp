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
// This is the implementation of MPxShadingNodeOverride that tells Maya
// how to build and manage a VP2 shading fragment for the simpleNoiseShader node
// in the DG.
//
// In addition to registering fragments and associating a fragment graph with
// the node, this override also manages some additional parameters on the final
// shading effect. Specifically, the fragment defines a texture and sampler
// parameter pair for the noise lookup table. Those parameters are not
// associated with any attributes on the Maya node and thus must be handled
// manually.
//
// The noise lookup table is a complete copy of Maya's noise table accessed
// through MRenderUtil::valueInNoiseTable(). This is packed into a 3D texture
// which the pixel shader can sample to get noise values to produce a result
// that is consistent with the compute() method of the simpleNoiseShader node.
//
// A static method handles the definition and registration of the actual
// shading fragments and the final fragment graph. Please see the comments in
// registerFragments() for more details.
//

#include "simpleNoiseShaderOverride.h"
#include <maya/MViewport2Renderer.h>
#include <maya/MFragmentManager.h>
#include <maya/MTextureManager.h>
#include <maya/MShaderManager.h>
#include <maya/MRenderUtil.h>
#include <vector>
#include <assert.h>

// Implementation-specific data and helper functions
namespace
{
	const MString sFinalFragmentGraphName("simpleNoise");
	const MString sNoiseLookupTextureName("simpleNoiseLookupTexture");

	const std::vector<float>& GetMayaNoiseTable()
	{
		// Static so as to only pull the noise data once (it's constant)
		static std::vector<float> sNoiseData;
		if (sNoiseData.empty())
		{
			// Pack entire noise table into an array, remapping from
			// the [-1,1] range to the [0,1] range. The pixel shader
			// will do the inverse operation to extract the real
			// data from the texture.
			const unsigned int dataSize = MRenderUtil::noiseTableSize();
			sNoiseData.resize(dataSize);
			for (unsigned int i=0; i<dataSize; i++)
			{
				sNoiseData[i] = (MRenderUtil::valueInNoiseTable(i) + 1.0f)/2.0f;
			}
		}
		assert(sNoiseData.size() == MRenderUtil::noiseTableSize());
		return sNoiseData;
	}
}


MHWRender::MPxShadingNodeOverride* simpleNoiseShaderOverride::creator(
	const MObject& obj)
{
	return new simpleNoiseShaderOverride(obj);
}


simpleNoiseShaderOverride::simpleNoiseShaderOverride(const MObject& obj)
: MPxShadingNodeOverride(obj)
, fObject(obj)
, fNoiseTexture(NULL)
, fNoiseSamplerState(NULL)
, fResolvedNoiseMapName("")
, fResolvedNoiseSamplerName("")
{
}


simpleNoiseShaderOverride::~simpleNoiseShaderOverride()
{
	// Release texture
	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (renderer)
	{
		MHWRender::MTextureManager* textureMgr =
			renderer->getTextureManager();
		if (textureMgr)
		{
			textureMgr->releaseTexture(fNoiseTexture);
			fNoiseTexture = NULL;
		}
	}

	// Release sampler state
	MHWRender::MStateManager::releaseSamplerState(fNoiseSamplerState);
	fNoiseSamplerState = NULL;
}


MHWRender::DrawAPI simpleNoiseShaderOverride::supportedDrawAPIs() const
{
	// Support all available draw APIs (all fragments have an implementation for
	// each).
	return
		MHWRender::kOpenGL |
		MHWRender::kDirectX11 |
		MHWRender::kOpenGLCoreProfile;
}


MString simpleNoiseShaderOverride::fragmentName() const
{
	// Reset cached parameter names since the effect is being rebuilt
	fResolvedNoiseMapName = "";
	fResolvedNoiseSamplerName = "";

	// Return the name of the full fragment graph
	return sFinalFragmentGraphName;
}


void simpleNoiseShaderOverride::getCustomMappings(
	MHWRender::MAttributeParameterMappingList& mappings)
{
	// Set up some mappings for the noise map parameters on fragment, as there
	// is no correspondence to attributes on the node for them.
	MHWRender::MAttributeParameterMapping mapMapping(
		"noiseLookupMap", "", false, true);
	mappings.append(mapMapping);

	MHWRender::MAttributeParameterMapping textureSamplerMapping(
		"noiseLookupMapSampler", "", false, true);
	mappings.append(textureSamplerMapping);
}


void simpleNoiseShaderOverride::updateShader(
	MHWRender::MShaderInstance& shader,
	const MHWRender::MAttributeParameterMappingList& mappings)
{
	// Handle resolved name caching
	if (fResolvedNoiseMapName.length() == 0)
	{
		const MHWRender::MAttributeParameterMapping* mapping =
			mappings.findByParameterName("noiseLookupMap");
		if (mapping)
		{
			fResolvedNoiseMapName = mapping->resolvedParameterName();
		}
	}
	if (fResolvedNoiseSamplerName.length() == 0)
	{
		const MHWRender::MAttributeParameterMapping* mapping =
			mappings.findByParameterName("noiseLookupMapSampler");
		if (mapping)
		{
			fResolvedNoiseSamplerName = mapping->resolvedParameterName();
		}
	}

	// Set the parameters on the shader
	if (fResolvedNoiseMapName.length() > 0 &&
		fResolvedNoiseSamplerName.length() > 0)
	{
		// Set a point-clamp sampler to the shader
		if (!fNoiseSamplerState)
		{
			MHWRender::MSamplerStateDesc desc;
			desc.filter = MHWRender::MSamplerState::kMinMagMipPoint;
			desc.addressU = desc.addressV = desc.addressW =
				MHWRender::MSamplerState::kTexClamp;
			desc.minLOD = 0;
			desc.maxLOD = 0;
			fNoiseSamplerState =
				MHWRender::MStateManager::acquireSamplerState(desc);
		}
		if (fNoiseSamplerState)
		{
			shader.setParameter(fResolvedNoiseSamplerName, *fNoiseSamplerState);
		}

		// Generate the noise lookup table texture if necessary
		if (!fNoiseTexture)
		{
			MHWRender::MRenderer* renderer =
				MHWRender::MRenderer::theRenderer();
			MHWRender::MTextureManager* textureMgr =
				renderer ? renderer->getTextureManager() : NULL;
			if (textureMgr)
			{
				// First, search the texture cache to see if another instance of
				// this override has already generated the texture. We can reuse
				// it to save GPU memory since the noise data is constant.
				fNoiseTexture =
					textureMgr->findTexture(sNoiseLookupTextureName);

				// Not in cache, so we need to actually build the texture
				if (!fNoiseTexture)
				{
					// Get Maya's noise table
					const std::vector<float>& noiseData = GetMayaNoiseTable();

					// Create a 3D texture containing the data
					MHWRender::MTextureDescription desc;
					desc.setToDefault2DTexture();
					desc.fWidth = desc.fHeight = desc.fDepth =
						MRenderUtil::noiseTableCubeSide();
					desc.fFormat = MHWRender::kR32_FLOAT;
					desc.fTextureType = MHWRender::kVolumeTexture;
					desc.fMipmaps = 1;
					fNoiseTexture = textureMgr->acquireTexture(
						sNoiseLookupTextureName,
						desc,
						(const void*)&(noiseData[0]),
						false);
				}
			}
		}

		// Set the texture to the shader instance
		if (fNoiseTexture)
		{
			MHWRender::MTextureAssignment textureAssignment;
			textureAssignment.texture = fNoiseTexture;
			shader.setParameter(fResolvedNoiseMapName, textureAssignment);
		}
	}
}


// Static fragment registration/deregistration methods called from plugin
// init/unint functions. Only need to be called once.
MStatus simpleNoiseShaderOverride::registerFragments()
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
	// Struct declaration fragment. This fragment provides the declaration of
	// the output structure of the simple noise fragment graph giving support
	// for multiple outputs. The struct fragment must be a part of the final
	// fragment graph in order to ensure that the fragment system includes
	// the declaration of the struct in the final effect.
	//
	fragmentName = "simpleNoiseOutput";
	fragmentBody =
		"<fragment uiName=\"simpleNoiseOutput\" name=\"simpleNoiseOutput\" type=\"structure\" class=\"ShadeFragment\" version=\"1.0\"> \r\n"
		"	<description><![CDATA[Struct output for simple noise shader]]></description> \r\n"
		"	<properties> \r\n"
		"		<struct name=\"simpleNoiseOutput\" struct_name=\"simpleNoiseOutput\" /> \r\n"
		"	</properties> \r\n"
		"	<values> \r\n"
		"	</values> \r\n"
		"	<outputs> \r\n"
		"		<alias name=\"simpleNoiseOutput\" struct_name=\"simpleNoiseOutput\" /> \r\n"
		"		<float3 name=\"outColor\" /> \r\n"
		"		<float name=\"outAlpha\" /> \r\n"
		"	</outputs> \r\n"
		"	<implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"Cg\" lang_version=\"2.1\"> \r\n"
		"		<function_name val=\"\" /> \r\n"
		"		<declaration name=\"simpleNoiseOutput\"><![CDATA[ \r\n"
		"struct simpleNoiseOutput \r\n"
		"{ \r\n"
		"	float3 outColor; \r\n"
		"	float outAlpha; \r\n"
		"}; \r\n"
		"		]]></declaration> \r\n"
		"	</implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"HLSL\" lang_version=\"11.0\"> \r\n"
		"		<function_name val=\"\" /> \r\n"
		"		<declaration name=\"simpleNoiseOutput\"><![CDATA[ \r\n"
		"struct simpleNoiseOutput \r\n"
		"{ \r\n"
		"	float3 outColor; \r\n"
		"	float outAlpha; \r\n"
		"}; \r\n"
		"		]]></declaration> \r\n"
		"	</implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"GLSL\" lang_version=\"3.0\"> \r\n"
		"		<function_name val=\"\" /> \r\n"
		"		<declaration name=\"simpleNoiseOutput\"><![CDATA[ \r\n"
		"struct simpleNoiseOutput \r\n"
		"{ \r\n"
		"	vec3 outColor; \r\n"
		"	float outAlpha; \r\n"
		"}; \r\n"
		"		]]></declaration> \r\n"
		"	</implementation> \r\n"
		"	</implementation> \r\n"
		"</fragment> \r\n";
	if (fragmentName != fragmentMgr->addShadeFragmentFromBuffer(fragmentBody, false)) return MS::kFailure;

	// ------------------------------------------------------------------------
	// Actual noise computation fragment. Based on Maya's "wave" noise type
	// from the 2d procedural noise texture node. Computes a struct output
	// containing both color and alpha to match the outputs of the DG node.
	// Uses a 3D texture loaded with the entire Maya noise table for generating
	// results consistent with the compute() method of the associated node.
	//
	fragmentName = "simpleNoiseBase";
	fragmentBody =
		"<fragment uiName=\"simpleNoiseBase\" name=\"simpleNoiseBase\" type=\"plumbing\" class=\"ShadeFragment\" version=\"1.0\"> \r\n"
		"	<description><![CDATA[Computes simple 2D procedural noise]]></description> \r\n"
		"	<properties> \r\n"
		"		<float2 name=\"uvCoord\" semantic=\"mayaUvCoordSemantic\" flags=\"varyingInputParam\" /> \r\n"
		"		<texture3 name=\"noiseLookupMap\" /> \r\n"
		"		<sampler name=\"noiseLookupMapSampler\" /> \r\n"
		"		<float name=\"amplitude\" /> \r\n"
		"		<float name=\"ratio\" /> \r\n"
		"		<int name=\"depthMax\" /> \r\n"
		"		<float name=\"frequency\" /> \r\n"
		"		<float name=\"frequencyRatio\" /> \r\n"
		"		<float name=\"time\" /> \r\n"
		"		<int name=\"numWaves\" /> \r\n"
		"	</properties> \r\n"
		"	<values> \r\n"
		"		<float name=\"amplitude\" value=\"1.0\" /> \r\n"
		"		<float name=\"ratio\" value=\"0.707000\" /> \r\n"
		"		<int name=\"depthMax\" value=\"3\" /> \r\n"
		"		<float name=\"frequency\" value=\"8.0\" /> \r\n"
		"		<float name=\"frequencyRatio\" value=\"2.0\" /> \r\n"
		"		<float name=\"time\" value=\"0.0\" /> \r\n"
		"		<int name=\"numWaves\" value=\"5\" /> \r\n"
		"	</values> \r\n"
		"	<outputs> \r\n"
		"		<struct name=\"simpleNoiseBase\" struct_name=\"simpleNoiseOutput\" /> \r\n"
		"	</outputs> \r\n"
		"	<implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"Cg\" lang_version=\"2.1\"> \r\n"
		"		<function_name val=\"simpleNoise\" /> \r\n"
		"		<source><![CDATA[ \r\n"
		"float simpleNoise_RawNoiseLookup( \r\n"
		"	int index, \r\n"
		"	texture3D noiseLookupMap, \r\n"
		"	sampler3D noiseLookupMapSampler) \r\n"
		"{ \r\n"
		"	int3 index3; \r\n"
		"	index3.x = index; \r\n"
		"	index3.y = (index>> 5); \r\n"
		"	index3.z = (index>> 10); \r\n"
		"	index3 &= 31; \r\n"
		"	float3 uvw = float3(index3) / 32.0f; \r\n"
		"	return (tex3D(noiseLookupMapSampler, uvw).r * 2.0f) - 1.0f; \r\n"
		"} \r\n"
		"simpleNoiseOutput simpleNoise( \r\n"
		"	float2 uv, \r\n"
		"	texture3D noiseLookupMap, \r\n"
		"	sampler3D noiseLookupMapSampler, \r\n"
		"	float amplitude, \r\n"
		"	float ratio, \r\n"
		"	int depthMax, \r\n"
		"	float frequency, \r\n"
		"	float frequencyRatio, \r\n"
		"	float time, \r\n"
		"	int numWaves) \r\n"
		"{ \r\n"
		"	const float M_PI = 3.1415926535897f; \r\n"
		"	const float M_2PI = 2.0f*M_PI; \r\n"
		"	simpleNoiseOutput finalResult; \r\n"
		"	float timeRatio = sqrt(frequencyRatio); \r\n"
		"	uv *= frequency; \r\n"
		"	float cosine = 0.0f; \r\n"
		"	float noise = 0.0f; \r\n"
		"	int depthId = 0; \r\n"
		"	int waveId = 0; \r\n"
		"	int seedOffset = 0; \r\n"
		"	while (depthId<depthMax && waveId<numWaves) { \r\n"
		"		int step = depthId; \r\n"
		"		int seed = 50*step; \r\n"
		"		float2 dir = float2( \r\n"
		"			simpleNoise_RawNoiseLookup(seed + seedOffset++, noiseLookupMap, noiseLookupMapSampler), \r\n"
		"			simpleNoise_RawNoiseLookup(seed + seedOffset++, noiseLookupMap, noiseLookupMapSampler)); \r\n"
		"		float norm = length(dir); \r\n"
		"		if (norm <= 0.0f) continue; \r\n"
		"		dir /= norm; \r\n"
		"		noise += cos(dir.x*uv.x*M_2PI + dir.y*uv.y*M_2PI + M_PI*simpleNoise_RawNoiseLookup(seed + seedOffset++, noiseLookupMap, noiseLookupMapSampler) + time*M_PI); \r\n"
		"		++waveId; \r\n"
		"		if (waveId < numWaves) continue; \r\n"
		"		noise /= float(numWaves); \r\n"
		"		uv *= frequencyRatio; \r\n"
		"		time *= timeRatio; \r\n"
		"		cosine += amplitude * noise; \r\n"
		"		amplitude *= ratio; \r\n"
		"		noise = 0.0f; \r\n"
		"		waveId = 0; \r\n"
		"		seedOffset = 0; \r\n"
		"		++depthId; \r\n"
		"	} \r\n"
		"	cosine = 0.5f*cosine + 0.5f; \r\n"
		"	float noiseVal = (cosine> 1.0f) ? 1.0f : cosine; \r\n"
		"	finalResult.outColor = float3(noiseVal, noiseVal, noiseVal); \r\n"
		"	finalResult.outAlpha = noiseVal; \r\n"
		"	return finalResult; \r\n"
		"} \r\n"
		"		]]></source> \r\n"
		"	</implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"HLSL\" lang_version=\"11.0\"> \r\n"
		"		<function_name val=\"simpleNoise\" /> \r\n"
		"		<source><![CDATA[ \r\n"
		"float simpleNoise_RawNoiseLookup( \r\n"
		"	int index, \r\n"
		"	Texture3D noiseLookupMap, \r\n"
		"	sampler noiseLookupMapSampler) \r\n"
		"{ \r\n"
		"	int3 index3; \r\n"
		"	index3.x = index; \r\n"
		"	index3.y = (index>> 5); \r\n"
		"	index3.z = (index>> 10); \r\n"
		"	index3 &= 31; \r\n"
		"	float3 uvw = float3(index3) / 32.0f; \r\n"
		"	return (noiseLookupMap.SampleLevel(noiseLookupMapSampler, uvw, 0).r * 2.0f) - 1.0f; \r\n"
		"} \r\n"
		"simpleNoiseOutput simpleNoise( \r\n"
		"	float2 uv, \r\n"
		"	Texture3D noiseLookupMap, \r\n"
		"	sampler noiseLookupMapSampler, \r\n"
		"	float amplitude, \r\n"
		"	float ratio, \r\n"
		"	int depthMax, \r\n"
		"	float frequency, \r\n"
		"	float frequencyRatio, \r\n"
		"	float time, \r\n"
		"	int numWaves) \r\n"
		"{ \r\n"
		"	const float M_PI = 3.1415926535897f; \r\n"
		"	const float M_2PI = 2.0f*M_PI; \r\n"
		"	simpleNoiseOutput finalResult; \r\n"
		"	float timeRatio = sqrt(frequencyRatio); \r\n"
		"	uv *= frequency; \r\n"
		"	float cosine = 0.0f; \r\n"
		"	float noise = 0.0f; \r\n"
		"	int depthId = 0; \r\n"
		"	int waveId = 0; \r\n"
		"	int seedOffset = 0; \r\n"
		"	while (depthId<depthMax && waveId<numWaves) { \r\n"
		"		int step = depthId; \r\n"
		"		int seed = 50*step; \r\n"
		"		float2 dir = float2( \r\n"
		"			simpleNoise_RawNoiseLookup(seed + seedOffset++, noiseLookupMap, noiseLookupMapSampler), \r\n"
		"			simpleNoise_RawNoiseLookup(seed + seedOffset++, noiseLookupMap, noiseLookupMapSampler)); \r\n"
		"		float norm = length(dir); \r\n"
		"		if (norm <= 0.0f) continue; \r\n"
		"		dir /= norm; \r\n"
		"		noise += cos(dir.x*uv.x*M_2PI + dir.y*uv.y*M_2PI + M_PI*simpleNoise_RawNoiseLookup(seed + seedOffset++, noiseLookupMap, noiseLookupMapSampler) + time*M_PI); \r\n"
		"		++waveId; \r\n"
		"		if (waveId < numWaves) continue; \r\n"
		"		noise /= float(numWaves); \r\n"
		"		uv *= frequencyRatio; \r\n"
		"		time *= timeRatio; \r\n"
		"		cosine += amplitude * noise; \r\n"
		"		amplitude *= ratio; \r\n"
		"		noise = 0.0f; \r\n"
		"		waveId = 0; \r\n"
		"		seedOffset = 0; \r\n"
		"		++depthId; \r\n"
		"	} \r\n"
		"	cosine = 0.5f*cosine + 0.5f; \r\n"
		"	float noiseVal = (cosine> 1.0f) ? 1.0f : cosine; \r\n"
		"	finalResult.outColor = float3(noiseVal, noiseVal, noiseVal); \r\n"
		"	finalResult.outAlpha = noiseVal; \r\n"
		"	return finalResult; \r\n"
		"} \r\n"
		"		]]></source> \r\n"
		"	</implementation> \r\n"
		"	<implementation render=\"OGSRenderer\" language=\"GLSL\" lang_version=\"3.0\"> \r\n"
		"		<function_name val=\"simpleNoise\" /> \r\n"
		"		<source><![CDATA[ \r\n"
		"float simpleNoise_RawNoiseLookup( \r\n"
		"	int index, \r\n"
		"	sampler3D noiseLookupMapSampler) \r\n"
		"{ \r\n"
		"	ivec3 index3; \r\n"
		"	index3.x = index; \r\n"
		"	index3.y = (index >> 5); \r\n"
		"	index3.z = (index >> 10); \r\n"
		"	index3 &= 31; \r\n"
		"	vec3 uvw = vec3(index3) / 32.0f; \r\n"
		"	return (texture(noiseLookupMapSampler, uvw).r * 2.0f) - 1.0f; \r\n"
		"} \r\n"
		"simpleNoiseOutput simpleNoise( \r\n"
		"	vec2 uv, \r\n"
		"	sampler3D noiseLookupMapSampler, \r\n"
		"	float amplitude, \r\n"
		"	float ratio, \r\n"
		"	int depthMax, \r\n"
		"	float frequency, \r\n"
		"	float frequencyRatio, \r\n"
		"	float time, \r\n"
		"	int numWaves) \r\n"
		"{ \r\n"
		"	const float M_PI = 3.1415926535897f; \r\n"
		"	const float M_2PI = 2.0f*M_PI; \r\n"
		"	simpleNoiseOutput finalResult; \r\n"
		"	float timeRatio = sqrt(frequencyRatio); \r\n"
		"	uv *= frequency; \r\n"
		"	float cosine = 0.0f; \r\n"
		"	float noise = 0.0f; \r\n"
		"	int depthId = 0; \r\n"
		"	int waveId = 0; \r\n"
		"	int seedOffset = 0; \r\n"
		"	while (depthId<depthMax && waveId<numWaves) { \r\n"
		"		int step = depthId; \r\n"
		"		int seed = 50*step; \r\n"
		"		vec2 dir = vec2( \r\n"
		"			simpleNoise_RawNoiseLookup(seed + seedOffset++, noiseLookupMapSampler), \r\n"
		"			simpleNoise_RawNoiseLookup(seed + seedOffset++, noiseLookupMapSampler)); \r\n"
		"		float norm = length(dir); \r\n"
		"		if (norm <= 0.0f) continue; \r\n"
		"		dir /= norm; \r\n"
		"		noise += cos(dir.x*uv.x*M_2PI + dir.y*uv.y*M_2PI + M_PI*simpleNoise_RawNoiseLookup(seed + seedOffset++, noiseLookupMapSampler) + time*M_PI); \r\n"
		"		++waveId; \r\n"
		"		if (waveId < numWaves) continue; \r\n"
		"		noise /= float(numWaves); \r\n"
		"		uv *= frequencyRatio; \r\n"
		"		time *= timeRatio; \r\n"
		"		cosine += amplitude * noise; \r\n"
		"		amplitude *= ratio; \r\n"
		"		noise = 0.0f; \r\n"
		"		waveId = 0; \r\n"
		"		seedOffset = 0; \r\n"
		"		++depthId; \r\n"
		"	} \r\n"
		"	cosine = 0.5f*cosine + 0.5f; \r\n"
		"	float noiseVal = (cosine > 1.0f) ? 1.0f : cosine; \r\n"
		"	finalResult.outColor = vec3(noiseVal, noiseVal, noiseVal); \r\n"
		"	finalResult.outAlpha = noiseVal; \r\n"
		"	return finalResult; \r\n"
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
	//                         simpleNoiseBase
	//                                |
	//                        simpleNoiseOutput
	//                                |
	//                          <FINAL RESULT>
	//
	// Maya's shader translation system will take this graph and connect UV
	// information where appropriate.
	//
	// From Maya's point of view, this graph is fully representative of the
	// associated shading node in the DG. All inputs and outputs on this graph
	// are matched to inputs and outputs on the DG node. Connections to other
	// nodes (input or output) will be mirrored by connections to other
	// fragments using the matching parameters.
	//
	fragmentName = sFinalFragmentGraphName;
	fragmentBody =
		"<fragment_graph name=\"simpleNoise\" ref=\"simpleNoise\" class=\"FragmentGraph\" version=\"1.0\"> \r\n"
		"	<fragments> \r\n"
		"		<fragment_ref name=\"simpleNoiseBase\" ref=\"simpleNoiseBase\" /> \r\n"
		"		<fragment_ref name=\"simpleNoiseOutput\" ref=\"simpleNoiseOutput\" /> \r\n"
		"	</fragments> \r\n"
		"	<connections> \r\n"
		"		<connect from=\"simpleNoiseBase.simpleNoiseBase\" to=\"simpleNoiseOutput.simpleNoiseOutput\" name=\"simpleNoiseOutput\" /> \r\n"
		"	</connections> \r\n"
		"	<properties> \r\n"
		"		<float2 name=\"uvCoord\" ref=\"simpleNoiseBase.uvCoord\" semantic=\"mayaUvCoordSemantic\" flags=\"varyingInputParam\" /> \r\n"
		"		<texture3 name=\"noiseLookupMap\" ref=\"simpleNoiseBase.noiseLookupMap\" /> \r\n"
		"		<sampler name=\"noiseLookupMapSampler\" ref=\"simpleNoiseBase.noiseLookupMapSampler\" /> \r\n"
		"		<float name=\"amplitude\" ref=\"simpleNoiseBase.amplitude\" /> \r\n"
		"		<float name=\"ratio\" ref=\"simpleNoiseBase.ratio\" /> \r\n"
		"		<int name=\"depthMax\" ref=\"simpleNoiseBase.depthMax\" /> \r\n"
		"		<float name=\"frequency\" ref=\"simpleNoiseBase.frequency\" /> \r\n"
		"		<float name=\"frequencyRatio\" ref=\"simpleNoiseBase.frequencyRatio\" /> \r\n"
		"		<float name=\"time\" ref=\"simpleNoiseBase.time\" /> \r\n"
		"		<int name=\"numWaves\" ref=\"simpleNoiseBase.numWaves\" /> \r\n"
		"	</properties> \r\n"
		"	<values> \r\n"
		"		<float name=\"amplitude\" value=\"1.0\" /> \r\n"
		"		<float name=\"ratio\" value=\"0.707000\" /> \r\n"
		"		<int name=\"depthMax\" value=\"3\" /> \r\n"
		"		<float name=\"frequency\" value=\"8.0\" /> \r\n"
		"		<float name=\"frequencyRatio\" value=\"2.0\" /> \r\n"
		"		<float name=\"time\" value=\"0.0\" /> \r\n"
		"		<int name=\"numWaves\" value=\"5\" /> \r\n"
		"	</values> \r\n"
		"	<outputs> \r\n"
		"		<struct name=\"simpleNoiseOutput\" ref=\"simpleNoiseOutput.simpleNoiseOutput\" /> \r\n"
		"	</outputs> \r\n"
		"</fragment_graph> \r\n";
	if (fragmentName != fragmentMgr->addFragmentGraphFromBuffer(fragmentBody)) return MS::kFailure;

	return MS::kSuccess;
}


MStatus simpleNoiseShaderOverride::deregisterFragments()
{
	// Get fragment manager for deregistration
	MHWRender::MFragmentManager* fragmentMgr =
		MHWRender::MRenderer::theRenderer()
			? MHWRender::MRenderer::theRenderer()->getFragmentManager()
			: NULL;
	// No fragment manager, fail
	if (!fragmentMgr) return MS::kFailure;

	// Remove fragments from library on plugin unload
	if (!fragmentMgr->removeFragment("simpleNoiseOutput")) return MS::kFailure;
	if (!fragmentMgr->removeFragment("simpleNoiseBase")) return MS::kFailure;
	if (!fragmentMgr->removeFragment(sFinalFragmentGraphName)) return MS::kFailure;

	return MS::kSuccess;
}

