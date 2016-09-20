//-
// ===========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================
//+

#include "phongShaderOverride.h"
#include <maya/MFnDependencyNode.h>
#include <maya/MShaderManager.h>

MHWRender::MPxSurfaceShadingNodeOverride* phongShaderOverride::creator(const MObject& obj)
{
	return new phongShaderOverride(obj);
}

phongShaderOverride::phongShaderOverride(const MObject& obj)
: MPxSurfaceShadingNodeOverride(obj)
, fObject(obj)
, fResolvedSpecularColorName("")
{
	fSpecularColor[0] = 0.5f;
	fSpecularColor[1] = 0.5f;
	fSpecularColor[2] = 0.5f;
}

phongShaderOverride::~phongShaderOverride()
{
}

MHWRender::DrawAPI phongShaderOverride::supportedDrawAPIs() const
{
	// works in both gl and dx
	return MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile;
}

MString phongShaderOverride::fragmentName() const
{
	// clear fResolvedSpecularColorName since we've rebuilt
	fResolvedSpecularColorName = "";

	// Just reuse Maya's phong surface shader
	return "mayaPhongSurface";
}

void phongShaderOverride::getCustomMappings(
	MHWRender::MAttributeParameterMappingList& mappings)
{
	// The "color" and "incandescence" attributes are all named the same as
	// the corresponding parameters on the fragment so they will map
	// automatically. Need to remap "diffuseReflectivity", "translucenceCoeff",
	// "reflectionGain" and "power" though.
	MHWRender::MAttributeParameterMapping diffuseMapping(
		"diffuse", "diffuseReflectivity", true, true);
	mappings.append(diffuseMapping);

	MHWRender::MAttributeParameterMapping translucenceMapping(
		"translucence", "translucenceCoeff", true, true);
	mappings.append(translucenceMapping);

	MHWRender::MAttributeParameterMapping reflectivityMapping(
		"reflectivity", "reflectionGain", true, true);
	mappings.append(reflectivityMapping);

	MHWRender::MAttributeParameterMapping powerMapping(
		"cosinePower", "power", true, true);
	mappings.append(powerMapping);

	// Our phong only uses a single float for specularity, while the Maya
	// phong fragment uses a full 3-float color. We could add a remap fragment
	// in front to expand the float to a 3-float, but it is simpler here to
	// just set the parameter manually in updateShader(). So add an empty
	// mapping to ensure the parameter gets renamed.
	MHWRender::MAttributeParameterMapping specColMapping(
		"specularColor", "", true, true);
	mappings.append(specColMapping);

}

MString phongShaderOverride::primaryColorParameter() const
{
	// Use the color parameter from the phong fragment as the primary color
	return "color";
}

MString phongShaderOverride::bumpAttribute() const
{
	// Use the "normalCamera" attribute to recognize bump connections
	return "normalCamera";
}

void phongShaderOverride::updateDG()
{
	MStatus status;
	MFnDependencyNode node(fObject, &status);
	if (status)
	{
		float specularity = 0.5f;
		node.findPlug("specularity").getValue(specularity);

		// Expand specularity to a 3-float
		fSpecularColor[0] = specularity;
		fSpecularColor[1] = specularity;
		fSpecularColor[2] = specularity;
	}
}

void phongShaderOverride::updateShader(
	MHWRender::MShaderInstance& shader,
	const MHWRender::MAttributeParameterMappingList& mappings)
{
	// Cache resolved name if found to avoid lookup on every update
	if (fResolvedSpecularColorName.length() == 0)
	{
		const MHWRender::MAttributeParameterMapping* mapping =
			mappings.findByParameterName("specularColor");
		if (mapping)
		{
			fResolvedSpecularColorName = mapping->resolvedParameterName();
		}
	}

	// Set parameter
	if (fResolvedSpecularColorName.length() > 0)
	{
		shader.setParameter(fResolvedSpecularColorName, fSpecularColor);
	}
}
