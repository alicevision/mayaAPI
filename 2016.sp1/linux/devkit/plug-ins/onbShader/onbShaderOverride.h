#ifndef _onbShaderOverride
#define _onbShaderOverride

//-
// ===========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================
//+

// Example Plugin: onbShader.cpp
//
// This is the MPxSurfaceShadingNodeOverride implementation to go along with
// the node defined in onbShader.cpp. This provides draw support in
// Viewport 2.0.
//

#include <maya/MPxSurfaceShadingNodeOverride.h>

class onbShaderOverride : public MHWRender::MPxSurfaceShadingNodeOverride
{
public:
	static MHWRender::MPxSurfaceShadingNodeOverride* creator(const MObject& obj);
	static MStatus registerFragments();
	static MStatus deregisterFragments();

	virtual ~onbShaderOverride();

	virtual MHWRender::DrawAPI supportedDrawAPIs() const;
	virtual MString fragmentName() const;
	virtual MString primaryColorParameter() const;
	virtual MString transparencyParameter() const;
	virtual MString bumpAttribute() const;

private:
	onbShaderOverride(const MObject& obj);

	MObject fObject;
	static const MString sFinalFragmentGraphName;
};

#endif // _onbShaderOverride
