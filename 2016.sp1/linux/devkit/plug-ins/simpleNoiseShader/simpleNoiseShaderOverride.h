#ifndef _simpleNoiseShaderOverride
#define _simpleNoiseShaderOverride

//-
// ===========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================
//+

// Example Plugin: simpleNoiseShader.cpp
//
// This is the MPxShadingNodeOverride implementation to go along with the node
// defined in simpleNoiseShader.cpp. This provides draw support in Viewport 2.0.
//

#include <maya/MPxShadingNodeOverride.h>

class simpleNoiseShaderOverride : public MHWRender::MPxShadingNodeOverride
{
public:
	static MHWRender::MPxShadingNodeOverride* creator(const MObject& obj);
	static MStatus registerFragments();
	static MStatus deregisterFragments();

	virtual ~simpleNoiseShaderOverride();

	virtual MHWRender::DrawAPI supportedDrawAPIs() const;
	virtual MString fragmentName() const;
	virtual void getCustomMappings(
		MHWRender::MAttributeParameterMappingList& mappings);
	virtual void updateShader(
		MHWRender::MShaderInstance& shader,
		const MHWRender::MAttributeParameterMappingList& mappings);

private:
	simpleNoiseShaderOverride(const MObject& obj);

	MObject fObject;
	MHWRender::MTexture* fNoiseTexture;
	const MHWRender::MSamplerState* fNoiseSamplerState;
	mutable MString fResolvedNoiseMapName;
	mutable MString fResolvedNoiseSamplerName;
};

#endif // _simpleNoiseShaderOverride
