#ifndef _phongShaderOverride
#define _phongShaderOverride

//-
// ===========================================================================
// Copyright 2012 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================
//+

// Example Plugin: lambertShader.cpp
//
// This is the MPxSurfaceShadingNodeOverride implementation to go along with
// the node defined in lambertShader.cpp. This provides draw support in
// Viewport 2.0.
//

#include <maya/MPxSurfaceShadingNodeOverride.h>

class phongShaderOverride : public MHWRender::MPxSurfaceShadingNodeOverride
{
public:
	static MHWRender::MPxSurfaceShadingNodeOverride* creator(const MObject& obj);

	virtual ~phongShaderOverride();

	virtual MHWRender::DrawAPI supportedDrawAPIs() const;

	virtual MString fragmentName() const;
	virtual void getCustomMappings(
		MHWRender::MAttributeParameterMappingList& mappings);

	virtual MString primaryColorParameter() const;
	virtual MString bumpAttribute() const;

	virtual void updateDG();
	virtual void updateShader(
		MHWRender::MShaderInstance& shader,
		const MHWRender::MAttributeParameterMappingList& mappings);

private:
	phongShaderOverride(const MObject& obj);

	MObject fObject;
	float fSpecularColor[3];
	mutable MString fResolvedSpecularColorName;
};

#endif // _phongShaderOverride
