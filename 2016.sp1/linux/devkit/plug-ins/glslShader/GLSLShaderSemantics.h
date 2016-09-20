#ifndef _GLSLSHADER_GLSLSHADERSEMANTICS_H
#define _GLSLSHADER_GLSLSHADERSEMANTICS_H
// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.

#include <maya/MUniformParameter.h>

namespace glslShaderSemantic
{
	extern const char* kSTANDARDSGLOBAL;
	extern const char* kUndefined;

	extern const char* kWorld;
	extern const char* kWorldTranspose;
	extern const char* kWorldInverse;
	extern const char* kWorldInverseTranspose;

	extern const char* kView;
	extern const char* kViewTranspose;
	extern const char* kViewInverse;
	extern const char* kViewInverseTranspose;

	extern const char* kProjection;
	extern const char* kProjectionTranspose;
	extern const char* kProjectionInverse;
	extern const char* kProjectionInverseTranspose;

	extern const char* kWorldView;
	extern const char* kWorldViewTranspose;
	extern const char* kWorldViewInverse;
	extern const char* kWorldViewInverseTranspose;

	extern const char* kViewProjection;
	extern const char* kViewProjectionTranspose;
	extern const char* kViewProjectionInverse;
	extern const char* kViewProjectionInverseTranspose;

	extern const char* kWorldViewProjection;
	extern const char* kWorldViewProjectionTranspose;
	extern const char* kWorldViewProjectionInverse;
	extern const char* kWorldViewProjectionInverseTranspose;

	extern const char* kViewDirection;
	extern const char* kViewPosition;
	extern const char* kLocalViewer;

	extern const char* kViewportPixelSize;
	extern const char* kBackgroundColor;

	extern const char* kFrame;
	extern const char* kFrameNumber;
	extern const char* kAnimationTime;
	extern const char* kTime;

	extern const char* kColor;
	extern const char* kLightColor;
	extern const char* kAmbient;
	extern const char* kLightAmbientColor;
	extern const char* kSpecular;
	extern const char* kLightSpecularColor;
	extern const char* kDiffuse;
	extern const char* kLightDiffuseColor;
	extern const char* kNormal;
	extern const char* kBump;
	extern const char* kEnvironment;

	extern const char* kPosition;
	extern const char* kAreaPosition0;
	extern const char* kAreaPosition1;
	extern const char* kAreaPosition2;
	extern const char* kAreaPosition3;
	extern const char* kDirection;

	extern const char* kTexCoord;
	extern const char* kTangent;
	extern const char* kBinormal;

	extern const char* kShadowMap;
	extern const char* kShadowColor;
	extern const char* kShadowFlag;
	extern const char* kShadowMapBias;
	extern const char* kShadowMapMatrix;
	extern const char* kShadowMapXForm;
	extern const char* kStandardsGlobal;

	extern const char* kLightEnable;
	extern const char* kLightIntensity;
	extern const char* kLightFalloff;
	extern const char* kFalloff;
	extern const char* kHotspot;
	extern const char* kLightType;
	extern const char* kDecayRate;

	extern const char* kLightRange;
	extern const char* kLightAttenuation0;
	extern const char* kLightAttenuation1;
	extern const char* kLightAttenuation2;
	extern const char* kLightTheta;
	extern const char* kLightPhi;

	extern const char* kTranspDepthTexture;
	extern const char* kOpaqueDepthTexture;

	// Maya custom semantics
	extern const char* kMayaSwatchRender;
	extern const char* kBboxExtraScale;
	extern const char* kOpacity;
	extern const char* kMayaGammaCorrection;
}

namespace glslShaderAnnotation
{
	// Technique Annotations
	extern const char* kTransparency;
	extern const char* kSupportsAdvancedTransparency;
	extern const char* kIndexBufferType;
	extern const char* kTextureMipmaplevels;
	extern const char* kObject;

	extern const char* kOverridesDrawState;
	extern const char* kVariableNameAsAttributeName;
	extern const char* kExtraScale;
	extern const char* kOverridesNonMaterialItems;

	// Pass annotations
	extern const char* kDrawContext;
	extern const char* kNonMaterialItemsPass;
	extern const char* kPrimitiveFilter;

	// Uniform annotations
	extern const char* kUIMin;
	extern const char* kUIMax;
	extern const char* kUISoftMin;
	extern const char* kUISoftMax;
	extern const char* kSpace;
	extern const char* kUIFieldNames;

	extern const char* kSasBindAddress;
	extern const char* kSasUiControl;
	extern const char* kUIWidget;
	extern const char* kUIGroup;
	extern const char* kUIOrder;

	// Texture annotations
	extern const char* kMipmaplevels;
}

namespace glslShaderAnnotationValue
{
	extern const char* kValueTransparent;
	extern const char* kValueOpaque;
	extern const char* kValueTrue;
	extern const char* kValueFalse;

	extern const char* kPosition;
	extern const char* kDirection;
	extern const char* kColor;
	extern const char* kColour;
	extern const char* kDiffuse;
	extern const char* kSpecular;
	extern const char* kAmbient;

	extern const char* kLight;
	extern const char* kLamp;
	extern const char* kPoint;
	extern const char* kSpot;
	extern const char* kDirectional;

	extern const char* kObject;
	extern const char* kWorld;
	extern const char* kView;
	extern const char* kCamera;

	extern const char* kSas_Skeleton_MeshToJointToWorld_0_;
	extern const char* kSas_Camera_WorldToView;
	extern const char* kSas_Camera_Projection;
	extern const char* kSas_Time_Now;
	extern const char* k_Position;
	extern const char* k_Direction;
	extern const char* k_Directional;

	extern const char* kColorPicker;

	extern const char* kFatLine;
	extern const char* kFatPoint;
}


#endif
