// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.

#include "GLSLShaderSemantics.h"

namespace glslShaderSemantic
{
	const char* kSTANDARDSGLOBAL						= "STANDARDSGLOBAL";
	const char* kUndefined								= "Undefined";

	const char* kWorld									= "World";
	const char* kWorldTranspose							= "WorldTranspose";
	const char* kWorldInverse							= "WorldInverse";
	const char* kWorldInverseTranspose					= "WorldInverseTranspose";

	const char* kView									= "View";
	const char* kViewTranspose							= "ViewTranspose";
	const char* kViewInverse							= "ViewInverse";
	const char* kViewInverseTranspose					= "ViewInverseTranspose";

	const char* kProjection								= "Projection";
	const char* kProjectionTranspose					= "ProjectionTranspose";
	const char* kProjectionInverse						= "ProjectionInverse";
	const char* kProjectionInverseTranspose				= "ProjectionInverseTranspose";

	const char* kWorldView								= "WorldView";
	const char* kWorldViewTranspose						= "WorldViewTranspose";
	const char* kWorldViewInverse						= "WorldViewInverse";
	const char* kWorldViewInverseTranspose				= "WorldViewInverseTranspose";

	const char* kViewProjection							= "ViewProjection";
	const char* kViewProjectionTranspose				= "ViewProjectionTranspose";
	const char* kViewProjectionInverse					= "ViewProjectionInverse";
	const char* kViewProjectionInverseTranspose			= "ViewProjectionInverseTranspose";

	const char* kWorldViewProjection					= "WorldViewProjection";
	const char* kWorldViewProjectionTranspose			= "WorldViewProjectionTranspose";
	const char* kWorldViewProjectionInverse				= "WorldViewProjectionInverse";
	const char* kWorldViewProjectionInverseTranspose	= "WorldViewProjectionInverseTranspose";

	const char* kViewDirection							= "ViewDirection";
	const char* kViewPosition							= "ViewPosition";
	const char* kLocalViewer							= "LocalViewer";

	const char* kViewportPixelSize						= "ViewportPixelSize";
	const char* kBackgroundColor						= "BackgroundColor";

	const char* kFrame									= "Frame";
	const char* kFrameNumber							= "FrameNumber";
	const char* kAnimationTime							= "AnimationTime";
	const char* kTime									= "Time";

	const char* kColor									= "Color";
	const char* kLightColor								= "LightColor";
	const char* kAmbient								= "Ambient";
	const char* kLightAmbientColor						= "LightAmbientColor";
	const char* kSpecular								= "Specular";
	const char* kLightSpecularColor						= "LightSpecularColor";
	const char* kDiffuse								= "Diffuse";
	const char* kLightDiffuseColor						= "LightDiffuseColor";
	const char* kNormal									= "Normal";
	const char* kBump									= "Bump";
	const char* kEnvironment							= "Environment";

	const char* kPosition								= "Position";
	const char* kAreaPosition0							= "AreaPosition0";
	const char* kAreaPosition1							= "AreaPosition1";
	const char* kAreaPosition2							= "AreaPosition2";
	const char* kAreaPosition3							= "AreaPosition3";
	const char* kDirection								= "Direction";

	const char* kTexCoord								= "TexCoord";
	const char* kTangent								= "Tangent";
	const char* kBinormal								= "Binormal";

	const char* kShadowMap								= "ShadowMap";
	const char* kShadowColor							= "ShadowColor";
	const char* kShadowFlag								= "ShadowFlag";
	const char* kShadowMapBias							= "ShadowMapBias";
	const char* kShadowMapMatrix						= "ShadowMapMatrix";
	const char* kShadowMapXForm							= "ShadowMapXForm";
	const char* kStandardsGlobal						= "StandardsGlobal";

	// Used to determine the type of a light parameter
	const char* kLightEnable							= "LightEnable";
	const char* kLightIntensity							= "LightIntensity";
	const char* kLightFalloff							= "LightFalloff";
	const char* kFalloff								= "Falloff";
	const char* kHotspot								= "Hotspot";
	const char* kLightType								= "LightType";
	const char* kDecayRate								= "DecayRate";

	const char* kLightRange								= "LightRange";
	const char* kLightAttenuation0						= "LightAttenuation0";
	const char* kLightAttenuation1						= "LightAttenuation1";
	const char* kLightAttenuation2						= "LightAttenuation2";
	const char* kLightTheta								= "LightTheta";
	const char* kLightPhi								= "LightPhi";

	const char* kTranspDepthTexture						= "transpdepthtexture";
	const char* kOpaqueDepthTexture						= "opaquedepthtexture";

	//--------------------------
	// Maya custom semantics

	// Define a boolean parameter to flag for swatch rendering
	const char* kMayaSwatchRender						= "MayaSwatchRender";

	// Define a float parameter to use to control the bounding box extra scale
	const char* kBboxExtraScale							= "BoundingBoxExtraScale";

	// Define a float parameter to use to check for transparency flag
	// Used in collaboration with the kIsTransparent annotation
	const char* kOpacity								= "Opacity";

	// Define a boolean parameter for full screen gamma correction
	const char* kMayaGammaCorrection					= "MayaGammaCorrection";
}


namespace glslShaderAnnotation
{
	// Technique annotations
	const char* kTransparency							= "Transparency";
	const char* kSupportsAdvancedTransparency			= "supportsAdvancedTransparency";
	const char* kIndexBufferType						= "index_buffer_type";
	const char* kTextureMipmaplevels					= "texture_mipmaplevels";
	const char* kObject									= "Object";

	// Define if the technique should follow the Maya transparent object rendering or is self-managed (multi-passes)
	const char* kOverridesDrawState						= "overridesDrawState";
	const char* kExtraScale								= "extraScale";
	const char* kOverridesNonMaterialItems				= "overridesNonMaterialItems";

	// Allow the shader writer to force the variable name to become the attribute name, even if UIName annotation is used
	const char* kVariableNameAsAttributeName			= "VariableNameAsAttributeName";

	// Pass annotations
	const char* kDrawContext							= "drawContext";
	const char* kNonMaterialItemsPass					= "nonMaterialItemsPass";
	const char* kPrimitiveFilter						= "primitiveFilter";

	// Uniform annotations
	const char* kUIMin									= "UIMin";
	const char* kUIMax									= "UIMax";
	const char* kUISoftMin								= "UISoftMin";
	const char* kUISoftMax								= "UISoftMax";
	const char* kSpace									= "Space";
	const char* kUIFieldNames							= "UIFieldNames";

	// Used to determine the semantic of a parameter
	const char* kSasBindAddress							= "SasBindAddress";
	const char* kSasUiControl							= "SasUiControl";
	const char* kUIWidget								= "UIWidget";
	const char* kUIGroup								= "UIGroup";
	const char* kUIOrder								= "UIOrder";
	
	// Texture annotations
	const char* kMipmaplevels							= "mipmaplevels";
}

namespace glslShaderAnnotationValue
{
	// Transparency values, used with kTransparency semantic
	const char* kValueTransparent						= "Transparent";
	const char* kValueOpaque							= "Opaque";

	const char* kValueTrue								= "true";
	const char* kValueFalse								= "false";

	// Used to try to determine an unknown semantic
	const char* kPosition								= "Position";
	const char* kDirection								= "Direction";
	const char* kColor									= "Color";
	const char* kColour									= "Colour";
	const char* kDiffuse								= "Diffuse";
	const char* kSpecular								= "Specular";
	const char* kAmbient								= "Ambient";

	// Supported values for the kObject annotation
	const char* kLight									= "Light";
	const char* kLamp									= "Lamp";
	const char* kPoint									= "Point";
	const char* kSpot									= "Spot";
	const char* kDirectional							= "Directional";

	// Supported values for the kSpace annotation
	const char* kObject									= "Object";
	const char* kWorld									= "World";
	const char* kView									= "View";
	const char* kCamera									= "Camera";

	// Supported values for the kSasBindAddress annotation
	const char* kSas_Skeleton_MeshToJointToWorld_0_		= "Sas.Skeleton.MeshToJointToWorld[0]";
	const char* kSas_Camera_WorldToView					= "Sas.Camera.WorldToView";
	const char* kSas_Camera_Projection					= "Sas.Camera.Projection";
	const char* kSas_Time_Now							= "Sas.Time.Now";
	const char* k_Position								= ".Position";
	const char* k_Direction								= ".Direction";
	const char* k_Directional							= ".Directional";

	// Supported value for the kSasUiControl annotation
	const char* kColorPicker							= "ColorPicker";

	// Supported value for kPrimitiveFilter annotation
	const char* kFatLine								= "fatLine";
	const char* kFatPoint								= "fatPoint";
}
