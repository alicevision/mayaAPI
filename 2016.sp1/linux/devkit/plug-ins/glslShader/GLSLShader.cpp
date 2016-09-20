//-
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+

#include "GLSLShader.h"
#include "GLSLShaderSemantics.h"
#include "GLSLShaderStrings.h"


#if defined(OSMac_)
# include <OpenGL/gl.h>
#else
# include <GL/gl.h>
#endif



#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnDependencyNode.h>
#include <maya/M3dView.h>

#include <maya/MGlobal.h>
#include <maya/MFileIO.h>
#include <maya/MString.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MFnAmbientLight.h>
#include <maya/MFnMessageAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnStringArrayData.h>
#include <maya/MDGModifier.h>
#include <maya/MEventMessage.h>
#include <maya/MSceneMessage.h>
#include <maya/MPlugArray.h>
#include <maya/MFileObject.h>
#include <maya/MModelMessage.h>
#include <maya/MAngle.h>
#include <maya/MImageFileInfo.h>
#include <maya/MRenderUtil.h>
#include <maya/MAnimControl.h>
#include <maya/MGeometryManager.h>
#include <maya/MHardwareRenderer.h>

#include <maya/MRenderProfile.h>
#include <maya/MGeometryList.h>
#include <maya/MPointArray.h>

#include <maya/MViewport2Renderer.h>
#include <maya/MDrawContext.h>
#include <maya/MTextureManager.h>
#include <maya/MHWGeometryUtilities.h>
#include <maya/MRenderUtilities.h>
#include <maya/MGeometryRequirements.h>
#include <maya/MRenderTargetManager.h>
#include <maya/MUIDrawManager.h>

#include <maya/MHardwareRenderer.h>
#include <maya/MGLFunctionTable.h>


#include <iostream>
#include <sstream>
#include <algorithm>
#include <list>

#if defined(OSLinux_) || defined(OSMac_)
#include <strings.h>
#define STRICMP(X,Y) strcasecmp(X,Y)
#define MSTRICMP(X,Y) strcasecmp(X.asChar(),Y)
#elif defined(OSWin_)
#define STRICMP(X,Y) stricmp(X,Y)
#define MSTRICMP(X,Y) stricmp(X.asChar(),Y)
using namespace std;
#endif

const MTypeId GLSLShaderNode::m_TypeId(0x00081101);
const MString GLSLShaderNode::m_TypeName("GLSLShader");
const MString GLSLShaderNode::m_RegistrantId("GLSLShaderRegistrantId");
const MString GLSLShaderNode::m_drawDbClassification("drawdb/shader/surface/GLSLShader");

static MObject sShader;
static MObject sTechnique;
static MObject sTechniques;
static MObject sDescription;
static MObject sDiagnostics;
static MObject sEffectUniformParameters;
static MObject sLightInfo;

#define M_CHECK(assertion)  if (assertion) ; else throw ((GLSLShaderNamespace::InternalError*)__LINE__)
namespace GLSLShaderNamespace
{
#ifdef _WIN32
	class InternalError;    // Never defined.  Used like this:
	//   throw (InternalError*)__LINE__;
#else
	struct InternalError
	{
		char* message;
	};
	//   throw (InternalError*)__LINE__;
#endif
}



// Convert Maya light type to glslShader light type
static GLSLShaderNode::ELightType getLightType(const MHWRender::MLightParameterInformation* lightParam)
{
	GLSLShaderNode::ELightType type = GLSLShaderNode::eUndefinedLight;

	MString lightType = lightParam->lightType();

	// The 3rd letter of the light name is a perfect hash,
	// so let's cut on the number of string comparisons.
	if (lightType.length() > 2) {
		switch (lightType.asChar()[2])
		{
		case 'o':
			if (STRICMP(lightType.asChar(),"spotLight") == 0)
				type = GLSLShaderNode::eSpotLight;
			break;

		case 'r':
			if (STRICMP(lightType.asChar(),"directionalLight") == 0)
			{
				// The headlamp used in the "Use default lighting" mode
					// does not have the same set of attributes as a regular
						// directional light, so we must disambiguate them
							// otherwise we might not know how to fetch shadow data
								// from the regular kind.
									if (lightParam->lightPath().isValid())
										type = GLSLShaderNode::eDirectionalLight;
									else
										type = GLSLShaderNode::eDefaultLight;
			}
			break;

		case 'i':
			if (STRICMP(lightType.asChar(),"pointLight") == 0)
				type = GLSLShaderNode::ePointLight;
			break;

		case 'b':
			if (STRICMP(lightType.asChar(),"ambientLight") == 0)
				type = GLSLShaderNode::eAmbientLight;
			break;

		case 'l':
			if (STRICMP(lightType.asChar(),"volumeLight") == 0)
				type = GLSLShaderNode::eVolumeLight;
			break;

		case 'e':
			if (STRICMP(lightType.asChar(),"areaLight") == 0)
				type = GLSLShaderNode::eAreaLight;
			break;
		}
	}
	return type;
}

// Find a substring, if not found also try for lowercase substring
static int findSubstring(const MString& haystack, const MString& needle)
{
	int at = haystack.indexW(needle);
	if(at < 0)
	{
		MString needleLowerCase = needle;
		needleLowerCase.toLowerCase();
		at = haystack.indexW(needleLowerCase);
	}
	return at;
}

// Convenient function to remove all non alpha-numeric characters from a string (remplaced by _ )
static MString sanitizeName(const MString& dirtyName)
{
	std::string retVal(dirtyName.asChar());

	for (size_t i=0; i<retVal.size(); ++i)
		if (!isalnum(retVal[i]))
			retVal.replace(i, 1, "_");

	return MString(retVal.c_str());
}

// Convenient function to find a string in an array and add it if not found
static int findInArray(MStringArray& where, const MString& what, bool appendIfNotFound)
{
	unsigned int index = 0;
	for (; index < where.length(); ++index)
	{
		if (where[index] == what || sanitizeName(where[index]) == what)
			return (int)index;
	}

	if (appendIfNotFound)
	{
		where.append(what);
		return (int)index;
	}

	return -1;
}

static const wchar_t layerNameSeparator(L'\r');
void getTextureDesc(const MHWRender::MDrawContext& context, const MUniformParameter& uniform, MString &fileName, MString &layerName, int &alphaChannelIdx)
{
	if(!uniform.isATexture())
		return;

	fileName = uniform.getAsString(context);
	if(fileName.length() == 0)  // file name is empty no need to process the layer name
		return;

	layerName.clear();
	alphaChannelIdx = -1;

	// Find the file/layer separator .. texture name set for the uv editor .. cf GLSLShader::renderImage()
	const int idx = fileName.indexW(layerNameSeparator);
	if(idx >= 0)
	{
		MStringArray splitData;
		fileName.split(layerNameSeparator, splitData);
		if(splitData.length() > 2)
			alphaChannelIdx = splitData[2].asInt();
		if(splitData.length() > 1)
			layerName = splitData[1];
		fileName = splitData[0];
	}
	else
	{
		// Look for the layerSetName attribute
		MObject node = uniform.getSource().node();
		MFnDependencyNode dependNode;
		dependNode.setObject(node);

		MPlug plug = dependNode.findPlug("layerSetName");
		if(!plug.isNull()) {
			plug.getValue(layerName);
		}

		// Look for the alpha channel index :
		// - get the select alpha channel name
		// - get the list of all alpha channels
		// - resolve index
		plug = dependNode.findPlug("alpha");
		if(!plug.isNull()) {
			MString alphaChannel;
			plug.getValue(alphaChannel);

			if(alphaChannel.length() > 0) {
				if(alphaChannel == "Default") {
					alphaChannelIdx = 1;
				}
				else {
					plug = dependNode.findPlug("alphaList");
					if(!plug.isNull()) {
						MDataHandle dataHandle;
						plug.getValue(dataHandle);
						if(dataHandle.type() == MFnData::kStringArray) {
							MFnStringArrayData stringArrayData (dataHandle.data());

							MStringArray allAlphaChannels;
							stringArrayData.copyTo(allAlphaChannels);

							unsigned int count = allAlphaChannels.length();
							for(unsigned int idx = 0; idx < count; ++idx) {
								const MString& channel = allAlphaChannels[idx];
								if(channel == alphaChannel) {
									alphaChannelIdx = idx + 2;
									break;
								}
							}
						}
					}
				}
			}
		}
	}
}


// Always good to reuse attributes whenever possible.
//
// In order to fully reuse the technique enum attribute, we need to
// clear it of its previous contents, which is something that is not
// yet possible with the MFnEnumAttribute function set. We still can
// achieve the required result with a proper MEL command to reset the
// enum strings.
static bool resetTechniqueEnumAttribute(const GLSLShaderNode& shader)
{
	MStatus stat;
	MFnDependencyNode node(shader.thisMObject(), &stat);
	if (!stat) return false;

	// Reset the .techniqueEnum attribute if exists
	MObject attr = node.attribute("techniqueEnum", &stat);
	if (stat && !attr.isNull() && attr.apiType() == MFn::kEnumAttribute)
	{
		MFnEnumAttribute enumAttr(attr);
		MString addAttrCmd = enumAttr.getAddAttrCmd();
		if (addAttrCmd.indexW(" -en ") >= 0)
		{
			MPlug techniquePlug = node.findPlug(attr, false);
			MString resetCmd = "addAttr -e -en \"\" ";
			MGlobal::executeCommand(resetCmd + techniquePlug.name(), false, false);
		}
	}

	return true;
}

static MObject buildTechniqueEnumAttribute(const GLSLShaderNode& shader)
{
	MStatus stat;
	MFnDependencyNode node(shader.thisMObject(), &stat);
	if (!stat) return MObject::kNullObj;

	// Reset the .techniqueEnum attribute
	resetTechniqueEnumAttribute(shader);

	// Create the new .techniqueEnum attribute
	MObject attr = node.attribute("techniqueEnum", &stat);
	if (attr.isNull())
	{
		MFnEnumAttribute enumAttr;
		attr = enumAttr.create("techniqueEnum", "te", 0, &stat);
		if (!stat || attr.isNull()) return MObject::kNullObj;

		// Set attribute flags
		enumAttr.setInternal( true );
		enumAttr.setStorable( false );
		enumAttr.setKeyable( true );  // show in Channel Box
		enumAttr.setAffectsAppearance( true );
		enumAttr.setNiceNameOverride("Technique");

		// Add the attribute to the node
		node.addAttribute(attr);
	}

	// Set attribute fields
	MFnEnumAttribute enumAttr(attr);
	const MStringArray& techniques = shader.techniqueNames();
	M_CHECK(techniques.length() < (unsigned	int)std::numeric_limits<short>::max());
	for (unsigned int i = 0; i < techniques.length(); ++i)
	{
		enumAttr.addField(techniques[i], (short)i);
	}

	return attr;
}


// Determine if scene light is compatible with shader light
static bool isLightAcceptable(GLSLShaderNode::ELightType shaderLightType, GLSLShaderNode::ELightType sceneLightType)
{
	// a Spot light is acceptable for any light types, providing both the direction and position properties.
	if(sceneLightType == GLSLShaderNode::eSpotLight)
		return true;

	// a Directional light only provides direction property.
	if(sceneLightType == GLSLShaderNode::eDirectionalLight || sceneLightType == GLSLShaderNode::eDefaultLight)
		return (shaderLightType == GLSLShaderNode::eDirectionalLight || shaderLightType == GLSLShaderNode::eAmbientLight);

	// a Point light only provides position property, same for volume and area lights
	if(sceneLightType == GLSLShaderNode::ePointLight ||
		sceneLightType == GLSLShaderNode::eAreaLight ||
		sceneLightType == GLSLShaderNode::eVolumeLight)
		return (shaderLightType == GLSLShaderNode::ePointLight || shaderLightType == GLSLShaderNode::eAmbientLight);

	// an Ambient light provides neither direction nor position properties.
	if(sceneLightType == GLSLShaderNode::eAmbientLight)
		return (shaderLightType == GLSLShaderNode::eAmbientLight);

	return false;
}

// The light information in the draw context has M attributes that we
// want to match to the N attributes of the shader. In order to do so
// in less than O(MxN) we create this static mapping between a light
// semantic and the corresponding DC light attribute names whose value
// needs to be fetched to refresh a shader parameter value.
typedef std::vector<MStringArray> TNamesForSemantic;
typedef std::vector<TNamesForSemantic> TSemanticNamesForLight;
static TSemanticNamesForLight sSemanticNamesForLight(GLSLShaderNode::eLightCount);

static void buildDrawContextParameterNames(GLSLShaderNode::ELightType lightType, const MHWRender::MLightParameterInformation* lightParam)
{
	TNamesForSemantic& namesForLight(sSemanticNamesForLight[lightType]);
	namesForLight.resize(GLSLShaderNode::eLastParameterType);

	MStringArray params;
	lightParam->parameterList(params);
	for (unsigned int p = 0; p < params.length(); ++p)
	{
		MString pname = params[p];
		MHWRender::MLightParameterInformation::StockParameterSemantic semantic = lightParam->parameterSemantic( pname );

		switch (semantic)
		{
		case MHWRender::MLightParameterInformation::kWorldPosition:
			namesForLight[GLSLShaderNode::eLightPosition].append(pname);
			if (pname == "LP0")
				namesForLight[GLSLShaderNode::eLightAreaPosition0].append(pname);
			if (pname == "LP1")
				namesForLight[GLSLShaderNode::eLightAreaPosition1].append(pname);
			if (pname == "LP2")
				namesForLight[GLSLShaderNode::eLightAreaPosition2].append(pname);
			if (pname == "LP3")
				namesForLight[GLSLShaderNode::eLightAreaPosition3].append(pname);
			break;
		case MHWRender::MLightParameterInformation::kWorldDirection:
			namesForLight[GLSLShaderNode::eLightDirection].append(pname);
			break;
		case MHWRender::MLightParameterInformation::kIntensity:
			namesForLight[GLSLShaderNode::eLightIntensity].append(pname);
			break;
		case MHWRender::MLightParameterInformation::kColor:
			namesForLight[GLSLShaderNode::eLightColor].append(pname);
			namesForLight[GLSLShaderNode::eLightAmbientColor].append(pname);
			namesForLight[GLSLShaderNode::eLightSpecularColor].append(pname);
			namesForLight[GLSLShaderNode::eLightDiffuseColor].append(pname);
			break;
			// Parameter type extraction for shadow maps
		case MHWRender::MLightParameterInformation::kGlobalShadowOn:
		case MHWRender::MLightParameterInformation::kShadowOn:
			namesForLight[GLSLShaderNode::eLightShadowOn].append(pname);
			break;
		case MHWRender::MLightParameterInformation::kShadowViewProj:
			namesForLight[GLSLShaderNode::eLightShadowViewProj].append(pname);
			break;
		case MHWRender::MLightParameterInformation::kShadowMap:
			namesForLight[GLSLShaderNode::eLightShadowOn].append(pname);
			namesForLight[GLSLShaderNode::eLightShadowMap].append(pname);
			break;
		case MHWRender::MLightParameterInformation::kShadowColor:
			namesForLight[GLSLShaderNode::eLightShadowColor].append(pname);
			break;
		case MHWRender::MLightParameterInformation::kShadowBias:
			namesForLight[GLSLShaderNode::eLightShadowMapBias].append(pname);
			break;
		case MHWRender::MLightParameterInformation::kCosConeAngle:
			namesForLight[GLSLShaderNode::eLightHotspot].append(pname);
			namesForLight[GLSLShaderNode::eLightFalloff].append(pname);
			break;
		case MHWRender::MLightParameterInformation::kDecayRate:
			namesForLight[GLSLShaderNode::eDecayRate].append(pname);
			break;
		default:
			break;
		}
	}
}

static const MStringArray& drawContextParameterNames(GLSLShaderNode::ELightType lightType, int paramType, const MHWRender::MLightParameterInformation* lightParam)
{
	if (sSemanticNamesForLight[lightType].size() == 0)
		buildDrawContextParameterNames(lightType, lightParam);

	return sSemanticNamesForLight[lightType][paramType];
}

static MUniformParameter::DataType convertToUniformDataType(MHWRender::MShaderInstance::ParameterType dataType)
{
	switch (dataType)
	{
	case MHWRender::MShaderInstance::kInvalid:
		return MUniformParameter::kTypeUnknown;
	case MHWRender::MShaderInstance::kBoolean:
		return MUniformParameter::kTypeBool;
	case MHWRender::MShaderInstance::kInteger:
		return MUniformParameter::kTypeInt;
	case MHWRender::MShaderInstance::kFloat:
	case MHWRender::MShaderInstance::kFloat2:
	case MHWRender::MShaderInstance::kFloat3:
	case MHWRender::MShaderInstance::kFloat4:
	case MHWRender::MShaderInstance::kFloat4x4Row:
	case MHWRender::MShaderInstance::kFloat4x4Col:
		return MUniformParameter::kTypeFloat;
	case MHWRender::MShaderInstance::kTexture1:
		return MUniformParameter::kType1DTexture;
	case MHWRender::MShaderInstance::kTexture2:
		return MUniformParameter::kType2DTexture;
	case MHWRender::MShaderInstance::kTexture3:
		return MUniformParameter::kType3DTexture;
	case MHWRender::MShaderInstance::kTextureCube:
		return MUniformParameter::kTypeCubeTexture;
	case MHWRender::MShaderInstance::kSampler:
		return MUniformParameter::kTypeString;
	default:
		return MUniformParameter::kTypeUnknown;
	}

	return MUniformParameter::kTypeUnknown;
}

static MUniformParameter::DataSemantic convertToUniformSemantic(const char* strSemantic)
{
	MUniformParameter::DataSemantic paramSemantic = MUniformParameter::kSemanticUnknown;

	if(	     !STRICMP( strSemantic, glslShaderSemantic::kWorld))								paramSemantic = MUniformParameter::kSemanticWorldMatrix;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kWorldTranspose))						paramSemantic = MUniformParameter::kSemanticWorldTransposeMatrix;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kWorldInverse))							paramSemantic = MUniformParameter::kSemanticWorldInverseMatrix;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kWorldInverseTranspose))				paramSemantic = MUniformParameter::kSemanticWorldInverseTransposeMatrix;

	else if( !STRICMP( strSemantic, glslShaderSemantic::kView))									paramSemantic = MUniformParameter::kSemanticViewMatrix;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kViewTranspose))						paramSemantic = MUniformParameter::kSemanticViewTransposeMatrix;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kViewInverse))							paramSemantic = MUniformParameter::kSemanticViewInverseMatrix;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kViewInverseTranspose))					paramSemantic = MUniformParameter::kSemanticViewInverseTransposeMatrix;

	else if( !STRICMP( strSemantic, glslShaderSemantic::kProjection))							paramSemantic = MUniformParameter::kSemanticProjectionMatrix;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kProjectionTranspose))					paramSemantic = MUniformParameter::kSemanticProjectionTransposeMatrix;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kProjectionInverse))					paramSemantic = MUniformParameter::kSemanticProjectionInverseMatrix;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kProjectionInverseTranspose))			paramSemantic = MUniformParameter::kSemanticProjectionInverseTransposeMatrix;

	else if( !STRICMP( strSemantic, glslShaderSemantic::kWorldView))							paramSemantic = MUniformParameter::kSemanticWorldViewMatrix;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kWorldViewTranspose))					paramSemantic = MUniformParameter::kSemanticWorldViewTransposeMatrix;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kWorldViewInverse))						paramSemantic = MUniformParameter::kSemanticWorldViewInverseMatrix;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kWorldViewInverseTranspose))			paramSemantic = MUniformParameter::kSemanticWorldViewInverseTransposeMatrix;

	else if( !STRICMP( strSemantic, glslShaderSemantic::kViewProjection))						paramSemantic = MUniformParameter::kSemanticViewProjectionMatrix;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kViewProjectionTranspose))				paramSemantic = MUniformParameter::kSemanticViewProjectionTransposeMatrix;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kViewProjectionInverse))				paramSemantic = MUniformParameter::kSemanticViewProjectionInverseMatrix;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kViewProjectionInverseTranspose))		paramSemantic = MUniformParameter::kSemanticViewProjectionInverseTransposeMatrix;

	else if( !STRICMP( strSemantic, glslShaderSemantic::kWorldViewProjection))					paramSemantic = MUniformParameter::kSemanticWorldViewProjectionMatrix;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kWorldViewProjectionTranspose))			paramSemantic = MUniformParameter::kSemanticWorldViewProjectionTransposeMatrix;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kWorldViewProjectionInverse))			paramSemantic = MUniformParameter::kSemanticWorldViewProjectionInverseMatrix;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kWorldViewProjectionInverseTranspose))	paramSemantic = MUniformParameter::kSemanticWorldViewProjectionInverseTransposeMatrix;

	else if( !STRICMP( strSemantic, glslShaderSemantic::kViewDirection))						paramSemantic = MUniformParameter::kSemanticViewDir;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kViewPosition))							paramSemantic = MUniformParameter::kSemanticViewPos;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kLocalViewer))							paramSemantic = MUniformParameter::kSemanticLocalViewer;

	else if( !STRICMP( strSemantic, glslShaderSemantic::kViewportPixelSize))					paramSemantic = MUniformParameter::kSemanticViewportPixelSize;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kBackgroundColor))						paramSemantic = MUniformParameter::kSemanticBackgroundColor;

	else if( !STRICMP( strSemantic, glslShaderSemantic::kFrame))								paramSemantic = MUniformParameter::kSemanticFrameNumber;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kFrameNumber))							paramSemantic = MUniformParameter::kSemanticFrameNumber;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kAnimationTime))						paramSemantic = MUniformParameter::kSemanticTime;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kTime))									paramSemantic = MUniformParameter::kSemanticTime;

	else if( !STRICMP( strSemantic, glslShaderSemantic::kColor))								paramSemantic = MUniformParameter::kSemanticColor;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kLightColor))							paramSemantic = MUniformParameter::kSemanticColor;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kAmbient))								paramSemantic = MUniformParameter::kSemanticColor;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kLightAmbientColor))					paramSemantic = MUniformParameter::kSemanticColor;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kSpecular))								paramSemantic = MUniformParameter::kSemanticColor;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kLightSpecularColor))					paramSemantic = MUniformParameter::kSemanticColor;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kDiffuse))								paramSemantic = MUniformParameter::kSemanticColor;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kNormal))								paramSemantic = MUniformParameter::kSemanticNormal;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kBump))							    	paramSemantic = MUniformParameter::kSemanticBump;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kEnvironment))							paramSemantic = MUniformParameter::kSemanticEnvironment;

	else if( !STRICMP( strSemantic, glslShaderSemantic::kPosition))								paramSemantic = MUniformParameter::kSemanticWorldPos;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kAreaPosition0))						paramSemantic = MUniformParameter::kSemanticWorldPos;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kAreaPosition1))						paramSemantic = MUniformParameter::kSemanticWorldPos;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kAreaPosition2))						paramSemantic = MUniformParameter::kSemanticWorldPos;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kAreaPosition3))						paramSemantic = MUniformParameter::kSemanticWorldPos;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kDirection))							paramSemantic = MUniformParameter::kSemanticViewDir;

	else if( !STRICMP( strSemantic, glslShaderSemantic::kShadowMap))							paramSemantic = MUniformParameter::kSemanticColorTexture;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kShadowColor))							paramSemantic = MUniformParameter::kSemanticColor;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kShadowFlag))							paramSemantic = MUniformParameter::kSemanticUnknown;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kShadowMapBias))						paramSemantic = MUniformParameter::kSemanticUnknown;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kShadowMapMatrix))						paramSemantic = MUniformParameter::kSemanticUnknown;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kShadowMapXForm))						paramSemantic = MUniformParameter::kSemanticUnknown;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kStandardsGlobal))						paramSemantic = MUniformParameter::kSemanticUnknown;

	else if( !STRICMP( strSemantic, glslShaderSemantic::kTranspDepthTexture))					paramSemantic = MUniformParameter::kSemanticTranspDepthTexture;
	else if( !STRICMP( strSemantic, glslShaderSemantic::kOpaqueDepthTexture))					paramSemantic = MUniformParameter::kSemanticOpaqueDepthTexture;

	return paramSemantic;
}

// Implicit light bindings are done without generating a dirty
// notification that the attribute editor can catch and use to
// update the dropdown menus and text fields used to indicate
// the current state of the light connections. This class
// accumulates refresh requests, and sends a single MEL command
// to refresh the AE when the app becomes idle.
class IdleAttributeEditorImplicitRefresher
{
public:
	static void activate()
	{
		if (sInstance == NULL)
			sInstance = new IdleAttributeEditorImplicitRefresher();
	};

private:
	IdleAttributeEditorImplicitRefresher()
	{
		mIdleCallback = MEventMessage::addEventCallback( "idle", IdleAttributeEditorImplicitRefresher::refresh );
	};

	~IdleAttributeEditorImplicitRefresher()
	{
		MMessage::removeCallback( mIdleCallback );
	}

	static void refresh(void* data)
	{
		if (sInstance)
		{
			MGlobal::executeCommandOnIdle("if (exists(\"AEGLSLShader_lightConnectionUpdateAll\")) AEGLSLShader_lightConnectionUpdateAll;");
			delete sInstance;
			sInstance = NULL;
		}
	}

private:
	MCallbackId mIdleCallback;
	static IdleAttributeEditorImplicitRefresher *sInstance;
};
IdleAttributeEditorImplicitRefresher *IdleAttributeEditorImplicitRefresher::sInstance = NULL;

// Adding and removing attributes while a scene is loading can lead
// to issues, especially if there were connections between the shader
// and a texture. To prevent these issues, we will wait until the scene
// has finished loading before adding or removing the attributes that
// manage connections between a scene light and its corresponding shader
// parameters.
class PostSceneUpdateAttributeRefresher
{
public:
	static void add(GLSLShaderNode* node)
	{
		if (sInstance == NULL)
			sInstance = new PostSceneUpdateAttributeRefresher();
		sInstance->mNodeSet.insert(node);
	};

	static void remove(GLSLShaderNode* node)
	{
		if (sInstance != NULL)
			sInstance->mNodeSet.erase(node);
	}

private:
	PostSceneUpdateAttributeRefresher()
	{
		mSceneUpdateCallback = MSceneMessage::addCallback(MSceneMessage::kSceneUpdate, PostSceneUpdateAttributeRefresher::refresh );
		mAfterCreateReference = MSceneMessage::addCallback(MSceneMessage::kAfterCreateReference , PostSceneUpdateAttributeRefresher::refresh );
		mAfterImport = MSceneMessage::addCallback(MSceneMessage::kAfterImport, PostSceneUpdateAttributeRefresher::refresh );
		mAfterLoadReference = MSceneMessage::addCallback(MSceneMessage::kAfterLoadReference, PostSceneUpdateAttributeRefresher::refresh );
	};

	~PostSceneUpdateAttributeRefresher()
	{
		MSceneMessage::removeCallback( mSceneUpdateCallback );
		MSceneMessage::removeCallback( mAfterCreateReference );
		MSceneMessage::removeCallback( mAfterImport );
		MSceneMessage::removeCallback( mAfterLoadReference );
	}

	static void refresh(void* data)
	{
		if (sInstance)
		{
			for (TNodeSet::iterator itNode = sInstance->mNodeSet.begin();
				itNode != sInstance->mNodeSet.end();
				++itNode )
			{
				(*itNode)->refreshLightConnectionAttributes(true);
			}

			delete sInstance;
			sInstance = NULL;
		}
	}

private:
	typedef std::set<GLSLShaderNode*> TNodeSet;
	TNodeSet mNodeSet;
	MCallbackId mSceneUpdateCallback;
	MCallbackId mAfterCreateReference;
	MCallbackId mAfterImport;
	MCallbackId mAfterLoadReference;
	static PostSceneUpdateAttributeRefresher *sInstance;
};
PostSceneUpdateAttributeRefresher *PostSceneUpdateAttributeRefresher::sInstance = NULL;

GLSLShaderNode::GLSLShaderNode()
:	fEffectLoaded(false)
,	fGLSLShaderInstance(NULL)
,	fTechniqueName("Main")
,	fTechniqueIdx(-1)
,	fTechniqueIsSelectable(false)
,	fTechniqueIsTransparent(false)
,	fTechniqueSupportsAdvancedTransparency(false)
,	fTechniqueOverridesDrawState(false)
,	fTechniqueTextureMipmapLevels(0)
,	fTechniqueBBoxExtraScale(1.0)
,	fTechniqueOverridesNonMaterialItems(false)
,	fTechniquePassCount(0)
,	fTechniquePassSpecs()
,	fLastFrameStamp((MUint64)-1)
{
	static bool s_addResourcePath = true;
	if (s_addResourcePath)
	{
		MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
		if (renderer)
		{
			const MString resourceLocation = MString("${MAYA_LOCATION}/presets/GLSL/examples").expandEnvironmentVariablesAndTilde();

			MHWRender::MTextureManager* textureMgr = renderer->getTextureManager();
			if (textureMgr) {
				textureMgr->addImagePath( resourceLocation );
			}

			const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
			if (shaderMgr) {
				shaderMgr->addShaderPath( resourceLocation );
				shaderMgr->addShaderIncludePath( resourceLocation );
			}
		}

		s_addResourcePath = false;
	}
}

GLSLShaderNode::~GLSLShaderNode()
{
	deleteUniformUserData();
	PostSceneUpdateAttributeRefresher::remove(this);
	if (fGLSLShaderInstance)
	{
		MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
		if (renderer)
		{
			const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
			if (shaderMgr)
			{
				shaderMgr->releaseShader(fGLSLShaderInstance);
				fGLSLShaderInstance = NULL;
			}
		}
	}
}

MStatus GLSLShaderNode::initialize()
{
	MStatus ms = MStatus::kSuccess;

	try
	{
		initializeNodeAttrs();
	}
	catch ( ... )
	{
		//MGlobal::displayError( "GLSLShader internal error: Unhandled exception in initialize" );
		ms = MS::kFailure;
	}

	return ms;
}

void* GLSLShaderNode::creator()
{
	return new GLSLShaderNode();
}

MStatus GLSLShaderNode::render( MGeometryList& iterator)
{
	MStatus result = MStatus::kFailure;

	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
	glPushAttrib(GL_CURRENT_BIT);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glColor4f(0.7f, 0.1f, 0.1f, 1.0f);
	glDisable(GL_LIGHTING);

	for( ; iterator.isDone() == false; iterator.next())
	{
		//MGeometry& geometry = iterator.geometry( MGeometryList::kMatrices );
		MGeometry& geometry = iterator.geometry( MGeometryList::kNone );

		{
			const MGeometryData position = geometry.position();

			GLint size = 0;
			switch (position.elementSize())
			{
			case MGeometryData::kOne:	size = 1; break;
			case MGeometryData::kTwo:	size = 2; break;
			case MGeometryData::kThree:	size = 3; break;
			case MGeometryData::kFour:	size = 4; break;
			default:					continue;
			}
			const GLvoid* data = position.data();

			glVertexPointer(size, GL_FLOAT, 0, data);
		}
		{
			const MGeometryData normal = geometry.normal();

			const GLvoid* data = normal.data();

			glNormalPointer(GL_FLOAT, 0, data);
		}
		for(unsigned int primitiveIdx = 0; primitiveIdx < geometry.primitiveArrayCount(); ++primitiveIdx)
		{
			MGeometryPrimitive primitive = geometry.primitiveArray(primitiveIdx);

			GLenum mode = GL_TRIANGLES;
			switch (primitive.drawPrimitiveType())
			{
			case MGeometryPrimitive::kPoints:			mode = GL_POINTS;			break;
			case MGeometryPrimitive::kLines:			mode = GL_LINES;			break;
			case MGeometryPrimitive::kLineStrip:		mode = GL_LINE_STRIP;		break;
			case MGeometryPrimitive::kLineLoop:			mode = GL_LINE_LOOP;		break;
			case MGeometryPrimitive::kTriangles:		mode = GL_TRIANGLES;		break;
			case MGeometryPrimitive::kTriangleStrip:	mode = GL_TRIANGLE_STRIP;	break;
			case MGeometryPrimitive::kTriangleFan:		mode = GL_TRIANGLE_FAN;		break;
			case MGeometryPrimitive::kQuads:			mode = GL_QUADS;			break;
			case MGeometryPrimitive::kQuadStrip:		mode = GL_QUAD_STRIP;		break;
			case MGeometryPrimitive::kPolygon:			mode = GL_POLYGON;			break;
			default:									continue;
			};
			GLenum format = GL_UNSIGNED_INT;
			switch (primitive.dataType())
			{
			case MGeometryData::kChar:
			case MGeometryData::kUnsignedChar:			format = GL_UNSIGNED_BYTE;	break;
			case MGeometryData::kInt16:
			case MGeometryData::kUnsignedInt16:			format = GL_UNSIGNED_SHORT;	break;
			case MGeometryData::kInt32:
			case MGeometryData::kUnsignedInt32:			format = GL_UNSIGNED_INT;	break;
			default:									continue;
			}
			GLsizei count = primitive.elementCount();
			const GLvoid* indices = primitive.data();
			glDrawElements(mode, count, format, indices);
			result = MStatus::kSuccess; // something drew
		}
	}

	glPopAttrib();
	glPopClientAttrib();

	return result;
}

void GLSLShaderNode::initializeNodeAttrs()
{
	MFnTypedAttribute	typedAttr;
	MFnNumericAttribute numAttr;
	MFnStringData		stringData;
	MFnStringArrayData	stringArrayData;
	MStatus				stat, stat2;

	// The shader attribute holds the name of the effect file that defines
	// the shader
	//
	sShader = typedAttr.create("shader", "s", MFnData::kString, stringData.create(&stat2), &stat);
	M_CHECK( stat );
	typedAttr.setInternal( true);
	typedAttr.setKeyable( false );
	typedAttr.setAffectsAppearance( true );
	typedAttr.setUsedAsFilename( true );
	stat = addAttribute(sShader);
	M_CHECK( stat );

	//
	// Effect Uniform Parameters
	//
	sEffectUniformParameters = typedAttr.create("EffectParameters", "ep", MFnData::kString, stringData.create(&stat2), &stat);
	M_CHECK( stat );
	typedAttr.setInternal( true);
	typedAttr.setKeyable( false);
	typedAttr.setAffectsAppearance( true );
	stat = addAttribute(sEffectUniformParameters);
	M_CHECK( stat );

	//
	// technique
	//
	sTechnique = typedAttr.create("technique", "t", MFnData::kString, stringData.create(&stat2), &stat);
	M_CHECK( stat );
	typedAttr.setInternal( true);
	typedAttr.setKeyable( true);
	typedAttr.setAffectsAppearance( true );
	stat = addAttribute(sTechnique);
	M_CHECK( stat );

	//
	// technique list
	//
	sTechniques = typedAttr.create("techniques", "ts", MFnData::kStringArray, stringArrayData.create(&stat2), &stat);
	M_CHECK( stat );
	typedAttr.setInternal( true);
	typedAttr.setKeyable( false);
	typedAttr.setStorable( false);
	typedAttr.setWritable( false);
	typedAttr.setAffectsAppearance( true );
	stat = addAttribute(sTechniques);
	M_CHECK( stat );

	// The description field where we pass compile errors etc back for the user to see
	//
	sDescription = typedAttr.create("description", "desc", MFnData::kString, stringData.create(&stat2), &stat);
	M_CHECK( stat );
	typedAttr.setKeyable( false);
	typedAttr.setWritable( false);
	typedAttr.setStorable( false);
	stat = addAttribute(sDescription);
	M_CHECK( stat );

	// The feedback field where we pass compile errors etc back for the user to see
	//
	sDiagnostics = typedAttr.create("diagnostics", "diag", MFnData::kString, stringData.create(&stat2), &stat);
	M_CHECK( stat );
	typedAttr.setKeyable( false);
	typedAttr.setWritable( false);
	typedAttr.setStorable( false);
	stat = addAttribute(sDiagnostics);
	M_CHECK( stat );

	// The description field where we pass compile errors etc back for the user to see
	//
	sLightInfo = typedAttr.create("lightInfo", "linfo", MFnData::kString, stringData.create(&stat2), &stat);
	M_CHECK( stat );
	typedAttr.setKeyable( false);
	typedAttr.setWritable( false);
	typedAttr.setStorable( false);
	stat = addAttribute(sLightInfo);
	M_CHECK( stat );

	//
	// Specify our dependencies
	//
	attributeAffects( sShader, sTechniques);
	attributeAffects( sShader, sTechnique);
}

bool GLSLShaderNode::getInternalValueInContext(const MPlug& plug,MDataHandle& handle,MDGContext& context)
{
	bool retVal = true;

	try
	{
		if (plug == sShader)
		{
			handle.set( fEffectName );
		}
		else if (plug == sTechnique)
		{
			handle.set( fTechniqueName );
		}
		else if (plug ==sTechniques)
		{
			const MStringArray* tlist = &fTechniqueNames;
			if (tlist)
				handle.set( MFnStringArrayData().create( *tlist ));
			else
				handle.set( MFnStringArrayData().create() );
		}
		else if (plug == fTechniqueEnumAttr)
		{
			//Todo: Move heavy instructions from here?
			fTechniqueIdx = -1;

			for (int i = 0; i < (int) fTechniqueNames.length(); ++i)
			{
				if (fTechniqueNames[i] == fTechniqueName)
				{
					fTechniqueIdx = i;
					break;
				}
			}

			if (fTechniqueIdx >=0)
			{
				handle.set((short)fTechniqueIdx);
			}
		}
		else
		{
			retVal = MPxHardwareShader::getInternalValueInContext(plug, handle, context);
		}
	}
	catch ( ... )
	{
		retVal = false;
	}

	return retVal;
}

bool GLSLShaderNode::setInternalValueInContext(const MPlug& plug,const MDataHandle& handle,MDGContext& context)
{
	bool retVal = true;
	try
	{
		if (plug == sShader)
		{
			loadEffect ( handle.asString() );
		}
		else if (plug == sTechnique)
		{
			fTechniqueName = handle.asString();
			loadEffect (fEffectName);
		}
		else if (plug == fTechniqueEnumAttr)
		{
			int index = handle.asShort();
			M_CHECK(fTechniqueNames.length() < (unsigned int)std::numeric_limits<int>::max());
			if (index >= 0 && index < (int)fTechniqueNames.length() && index != fTechniqueIdx)
			{
				fTechniqueName = fTechniqueNames[index];
			}
		}
		else
		{
			retVal = MPxHardwareShader::setInternalValueInContext(plug, handle, context);
		}
	}
	catch( ... )
	{
		retVal = false;
	}

	return retVal;
}

const MString& GLSLShaderNode::effectName() const
{
	return fEffectName;
}

bool GLSLShaderNode::loadEffect(const MString& effectName)
{
	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (!renderer)
		return false;

	const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
	if (!shaderMgr)
		return false;

	//In core profile, there used to be the problem where the shader fails to load sometimes.
	//The problem occurs when the OpenGL Device Context is switched  before
	//calling the GLSLShaderNode::loadEffect() function(this switch is performed by Tmodel::selectManip).
	//When that occurs, the shader is loaded in the wrong context instead of the
	//viewport context... so that in the draw phase, after switching to the viewport context,
	//the drawing is erroneous.
	//In order to solve that problem, make the view context current
	if ((renderer->drawAPI() & MHWRender::kOpenGLCoreProfile) != 0)
	{
		MStatus stat;
		
		M3dView view = M3dView::active3dView(&stat);
		if (stat != MStatus::kSuccess)
		{
			return false;
		}
		view.makeSharedContextCurrent();
	}

	if(effectName.length() == 0)
	{
		clearParameters();
		fEffectName.clear();
		fEffectLoaded = false;
		if (fGLSLShaderInstance != NULL) {
			shaderMgr->releaseShader(fGLSLShaderInstance);
			fGLSLShaderInstance = NULL;
		}
		fTechniqueNames.clear();
		return true;
	}

#if 0
	//if (MFileIO::isReadingFile() || MFileIO::isOpeningFile())
	{
		MFileObject file;
		file.setRawFullName( effectName );
		if(!file.exists())
		{
			MString msg = glslShaderStrings::getString( glslShaderStrings::kErrorFileNotFound, effectName );
			fErrorLog += msg;
			displayErrorAndWarnings();
			return false;
		}
	}
#endif

	// Tell Maya that we want access/control to all uniform parameters
	// by default Maya handles parameters with 'system' semantics such as LIGHTCOLOR
	// and these parameters won't be accessible from the plugin
	MHWRender::MShaderCompileMacro macros[] = { { MString("_MAYA_PLUGIN_HANDLES_ALL_UNIFORMS_"), MString("TRUE") } };
	unsigned int nbMacros = 1;
	
	// Get list of techniques
	MStringArray techniqueNames;
	shaderMgr->getEffectsTechniques(effectName, techniqueNames, macros, nbMacros);
	if (techniqueNames.length() == 0)
	{
		//If no techniques available, return false
		MString msg = glslShaderStrings::getString( glslShaderStrings::kErrorLoadingEffect, effectName );
		fErrorLog += msg;
		displayErrorAndWarnings();
		return false;
	}

	// Get preferred technique
	MString techniqueName;
	int techniqueIdx = -1;
	if (fTechniqueName.length() > 0)
	{
		for (unsigned int i = 0; i < techniqueNames.length(); ++i)
		{
			if (techniqueNames[i] == fTechniqueName)
			{
				techniqueName = fTechniqueName;
				techniqueIdx = i;
				break;
			}
		}
	}
	// If not found use first
	if (techniqueName.length() == 0) {
		techniqueName = techniqueNames[0];
		techniqueIdx = 0;
	}

	// Do not use cache here, in case we want to recompiling a shader that has been modified after loading.
	MHWRender::MShaderInstance* newInstance = shaderMgr->getEffectsFileShader(effectName, techniqueName, macros, nbMacros, false);
	if (newInstance)
	{	
		// Reset current light connections,	that will unlock light parameters so that their uniform attributes can be properly removed if not reused
		// Do not refresh AE, it's done on idle and the attribute may not exist anymore. The AE will be refreshed later on anyway
		clearLightConnectionData(false /*refreshAE*/);
		fLightParameters.clear();

		// Effect loaded successfuly, let's replace the previous one
		if (fGLSLShaderInstance != NULL)
			shaderMgr->releaseShader(fGLSLShaderInstance);
		fGLSLShaderInstance = newInstance;

		fEffectName = effectName;
		fTechniqueNames = techniqueNames;
		fTechniqueName = techniqueName;
		fTechniqueIdx = techniqueIdx;

		MPlug descriptionPlug( thisMObject(), sDescription);
		descriptionPlug.setValue( "" );

		MStatus opStatus;

		// Build list of techniques pass specs and determine Selectable status
		fTechniqueIsSelectable = false;
		fTechniquePassCount = 0;
		fTechniquePassSpecs.clear();
		{
			MHWRender::MDrawContext* context = MHWRender::MRenderUtilities::acquireSwatchDrawContext();
			if(context)
			{
				newInstance->bind(*context);

				fTechniquePassCount = newInstance->getPassCount(*context);
				for (unsigned int passIndex = 0; passIndex < fTechniquePassCount; ++passIndex)
				{
					const MString passDrawContext = newInstance->passAnnotationAsString(passIndex, glslShaderAnnotation::kDrawContext, opStatus);
					if (STRICMP(passDrawContext.asChar(), MHWRender::MPassContext::kSelectionPassSemantic.asChar()) == 0)
						fTechniqueIsSelectable = true;

					const MString passPrimitiveFilter = newInstance->passAnnotationAsString(passIndex, glslShaderAnnotation::kPrimitiveFilter, opStatus);
					const bool passIsForFatLine  = (STRICMP(passPrimitiveFilter.asChar(), glslShaderAnnotationValue::kFatLine) == 0);
					const bool passIsForFatPoint = (STRICMP(passPrimitiveFilter.asChar(), glslShaderAnnotationValue::kFatPoint) == 0);

					PassSpec spec = { passDrawContext, passIsForFatLine, passIsForFatPoint };
					fTechniquePassSpecs.insert( std::make_pair(passIndex, spec) );
				}

				newInstance->unbind(*context);
				MHWRender::MRenderUtilities::releaseDrawContext(context);
			}
		}

		// Setup Transparency using technique annotation
		fTechniqueIsTransparent = false;
		const MString transparency = fGLSLShaderInstance->techniqueAnnotationAsString(glslShaderAnnotation::kTransparency, opStatus);
		if (opStatus == MStatus::kSuccess)
		{
			fTechniqueIsTransparent = (STRICMP(transparency.asChar(), glslShaderAnnotationValue::kValueTransparent)==0);
		}

		// Setup Advanced Transparency support using technique annotation
		fTechniqueSupportsAdvancedTransparency = false;
		const MString advancedTransparency = fGLSLShaderInstance->techniqueAnnotationAsString(glslShaderAnnotation::kSupportsAdvancedTransparency, opStatus);
		if (opStatus == MStatus::kSuccess)
		{
			fTechniqueSupportsAdvancedTransparency = (STRICMP(advancedTransparency.asChar(), glslShaderAnnotationValue::kValueTrue)==0);
		}

		// Setup index buffer mutators using annotations
		fTechniqueIndexBufferType = MString();
		const MString indexBufferType = fGLSLShaderInstance->techniqueAnnotationAsString(glslShaderAnnotation::kIndexBufferType, opStatus);
		if (opStatus == MStatus::kSuccess)
		{
			fTechniqueIndexBufferType = indexBufferType;

			// Use our own crack free primitive generators - we know they are registered
			if( fTechniqueIndexBufferType == "PNAEN18" )
				fTechniqueIndexBufferType = "GLSL_PNAEN18";
			else if( fTechniqueIndexBufferType == "PNAEN9" )
				fTechniqueIndexBufferType = "GLSL_PNAEN9";
		}

		// Query technique if it should follow the maya transparent object rendering or is self-managed (multi-passes)
		fTechniqueOverridesDrawState = false;
		const MString overridesDrawState = fGLSLShaderInstance->techniqueAnnotationAsString(glslShaderAnnotation::kOverridesDrawState, opStatus);
		if (opStatus == MStatus::kSuccess)
		{
			fTechniqueOverridesDrawState = (STRICMP(overridesDrawState.asChar(), glslShaderAnnotationValue::kValueTrue)==0);
		}

		// Query technique preference for the mip map level to generate or load for each textures
		fTechniqueTextureMipmapLevels = 0;
		const int textureMipMapLevels = fGLSLShaderInstance->techniqueAnnotationAsInt(glslShaderAnnotation::kTextureMipmaplevels, opStatus);
		if (opStatus == MStatus::kSuccess)
		{
			fTechniqueTextureMipmapLevels = textureMipMapLevels;
		}

		// Query technique bbox extra scale
		fTechniqueBBoxExtraScale = 1.0;
		const double extraScale = (double) fGLSLShaderInstance->techniqueAnnotationAsFloat(glslShaderAnnotation::kExtraScale, opStatus);
		if (opStatus == MStatus::kSuccess)
		{
			fTechniqueBBoxExtraScale = extraScale;
		}

		// Query technique if it overrides non material items items
		fTechniqueOverridesNonMaterialItems = false;
		MString overridesNonMaterialItems = fGLSLShaderInstance->techniqueAnnotationAsString(glslShaderAnnotation::kOverridesNonMaterialItems, opStatus);
		if (opStatus == MStatus::kSuccess)
		{
			fTechniqueOverridesNonMaterialItems = (STRICMP(overridesNonMaterialItems.asChar(), glslShaderAnnotationValue::kValueTrue)==0);
		}
		configureUniforms();
		configureGeometryRequirements();

		fTechniqueEnumAttr = buildTechniqueEnumAttribute(*this);

		fEffectLoaded = true;

		// Refresh any AE that monitors implicit lights:
		IdleAttributeEditorImplicitRefresher::activate();

		refreshView();

		return true;
	}

	// Could not load effect, keep using the previous one ... no change
	MString msg = glslShaderStrings::getString( glslShaderStrings::kErrorLoadingEffect, effectName );
	fErrorLog += msg;
	displayErrorAndWarnings();
	return false;

}

// ***********************************
// ERROR Reporting
// ***********************************
void GLSLShaderNode::displayErrorAndWarnings() const
{
	MPlug diagnosticsPlug( thisMObject(), sDiagnostics);
	MString currentDiagnostic;
	diagnosticsPlug.getValue(currentDiagnostic);
	if(fErrorLog.length())
	{
		currentDiagnostic += glslShaderStrings::getString( glslShaderStrings::kErrorLog, fErrorLog );
		diagnosticsPlug.setValue( currentDiagnostic );

		MGlobal::displayError(fErrorLog);

		fErrorLog.clear();
	}
	if(fWarningLog.length())
	{
		currentDiagnostic += glslShaderStrings::getString( glslShaderStrings::kWarningLog, fWarningLog );
		diagnosticsPlug.setValue( currentDiagnostic );
		MGlobal::displayWarning(fWarningLog);
		fWarningLog.clear();
	}
}

void GLSLShaderNode::clearParameters()
{
	clearLightConnectionData();
	fLightParameters.clear();

	fUniformParameters.setLength(0);
	setUniformParameters( fUniformParameters, false );
	deleteUniformUserData();

	fGeometryRequirements.clear();
	fVaryingParameters.setLength(0);
	setVaryingParameters( fVaryingParameters, false );
	fVaryingParametersUpdateId = 0;

	fTechniqueIndexBufferType.clear();
	fTechniquePassSpecs.clear();
	fTechniqueIdx = -1;

	fUIGroupNames.setLength(0);
	fUIGroupParameters.clear();
}

void GLSLShaderNode::configureUniformUI(const MString& parameterName, MUniformParameter& uniformParam) const
{
	MStatus opStatus;

	const MString uiWidget = fGLSLShaderInstance->uiWidget(parameterName, opStatus);
	if (opStatus == MStatus::kSuccess)
	{
		if (uiWidget == "None")
		{
			uniformParam.setUIHidden(true);
		}
		else
		{
			uniformParam.setUIHidden(false);

			//Set UIMin and UIMax
#define SET_VALUE_FROM_ANNOTATION(shaderAnnotation, parameterFunction) \
			{ \
				const float value = fGLSLShaderInstance->annotationAsFloat(parameterName, glslShaderAnnotation::shaderAnnotation, opStatus); \
				if (opStatus == MStatus::kSuccess) \
				{ \
					uniformParam.parameterFunction((double)value); \
				} \
			}

			SET_VALUE_FROM_ANNOTATION(kUIMin, setRangeMin);
			SET_VALUE_FROM_ANNOTATION(kUIMax, setRangeMax);
			SET_VALUE_FROM_ANNOTATION(kUISoftMin, setSoftRangeMin);
			SET_VALUE_FROM_ANNOTATION(kUISoftMax, setSoftRangeMax);

#undef SET_VALUE_FROM_ANNOTATION
		}
	}

	const MString uiName = fGLSLShaderInstance->uiName(parameterName, opStatus);
	if (opStatus == MStatus::kSuccess)
	{
		uniformParam.setUINiceName(uiName);
	}
}

//
// Convert Shader space into Maya space
//
MUniformParameter::DataSemantic GLSLShaderNode::convertSpace(const MString& parameterName, MUniformParameter::DataSemantic defaultSpace)
{
	MUniformParameter::DataSemantic space = defaultSpace;

	MStatus opStatus;
	if (fGLSLShaderInstance == NULL)
	{
		return space;
	}
	
	MString ann = fGLSLShaderInstance->annotationAsString(parameterName, glslShaderAnnotation::kSpace, opStatus);
	if(opStatus != MStatus::kSuccess)
	{
		return space;
	}


	if( !STRICMP( ann.asChar(), glslShaderAnnotationValue::kObject))		space = defaultSpace >= MUniformParameter::kSemanticObjectPos ? MUniformParameter::kSemanticObjectPos	: MUniformParameter::kSemanticObjectDir;
	else if( !STRICMP( ann.asChar(), glslShaderAnnotationValue::kWorld))	space = defaultSpace >= MUniformParameter::kSemanticObjectPos ? MUniformParameter::kSemanticWorldPos	: MUniformParameter::kSemanticWorldDir;
	else if( !STRICMP( ann.asChar(), glslShaderAnnotationValue::kView))		space = defaultSpace >= MUniformParameter::kSemanticObjectPos ? MUniformParameter::kSemanticViewPos		: MUniformParameter::kSemanticViewDir;
	else if( !STRICMP( ann.asChar(), glslShaderAnnotationValue::kCamera))	space = defaultSpace >= MUniformParameter::kSemanticObjectPos ? MUniformParameter::kSemanticViewPos		: MUniformParameter::kSemanticViewDir;

	return space;
}

MUniformParameter::DataSemantic GLSLShaderNode::guessUnknownSemantics(const MString& parameterName)
{
	MUniformParameter::DataSemantic uniformSemantic = MUniformParameter::kSemanticUnknown;

	MStatus opStatus;
	if (fGLSLShaderInstance == NULL)
	{
		return uniformSemantic;
	}

	MString sasSemantic = fGLSLShaderInstance->annotationAsString(parameterName, glslShaderAnnotation::kSasBindAddress, opStatus);
	if( opStatus == MStatus::kSuccess && sasSemantic.length())
	{
		if(      !MSTRICMP( sasSemantic, glslShaderAnnotationValue::kSas_Skeleton_MeshToJointToWorld_0_))	uniformSemantic = MUniformParameter::kSemanticWorldMatrix;
		else if( !MSTRICMP( sasSemantic, glslShaderAnnotationValue::kSas_Camera_WorldToView))				uniformSemantic = MUniformParameter::kSemanticViewMatrix;
		else if( !MSTRICMP( sasSemantic, glslShaderAnnotationValue::kSas_Camera_Projection))				uniformSemantic = MUniformParameter::kSemanticProjectionMatrix;
		else if( !MSTRICMP( sasSemantic, glslShaderAnnotationValue::kSas_Time_Now))							uniformSemantic = MUniformParameter::kSemanticTime;
		else if( sasSemantic.rindexW( glslShaderAnnotationValue::k_Position) >= 0)									uniformSemantic = convertSpace(parameterName, MUniformParameter::kSemanticWorldPos);
		else if( sasSemantic.rindexW( glslShaderAnnotationValue::k_Direction) >= 0 &&
				 sasSemantic.rindexW( glslShaderAnnotationValue::k_Direction) != sasSemantic.rindexW( glslShaderAnnotationValue::k_Directional))	uniformSemantic = convertSpace(parameterName, MUniformParameter::kSemanticViewDir);
	}

	// Next try control type
	if( uniformSemantic == MUniformParameter::kSemanticUnknown)
	{
		const char* UIAnnotations[2] = { glslShaderAnnotation::kSasUiControl, glslShaderAnnotation::kUIWidget };
		for (int i = 0; i < 2; ++i)
		{
			MString UiControl = fGLSLShaderInstance->annotationAsString(parameterName, UIAnnotations[i], opStatus);
			if (opStatus == MStatus::kSuccess && UiControl.length() && !MSTRICMP( UiControl, glslShaderAnnotationValue::kColorPicker))
			{
				uniformSemantic = MUniformParameter::kSemanticColor;
				break;
			}
		}
	}

	MString semantic = fGLSLShaderInstance->semantic(parameterName);
	MHWRender::MShaderInstance::ParameterType paramType = fGLSLShaderInstance->parameterType(parameterName);

	// As a last ditch effort, look for an obvious parameter name
	if ( uniformSemantic == MUniformParameter::kSemanticUnknown && !semantic.length() &&
		(paramType == MHWRender::MShaderInstance::kFloat3 || paramType == MHWRender::MShaderInstance::kFloat4) )
	{
		if ( parameterName.rindexW( glslShaderAnnotationValue::kPosition) >= 0)
		{
			uniformSemantic = convertSpace(parameterName, MUniformParameter::kSemanticWorldPos);
		}
		else if ( parameterName.rindexW( glslShaderAnnotationValue::kDirection) >= 0 &&
				  parameterName.rindexW( glslShaderAnnotationValue::kDirection) != parameterName.rindexW( glslShaderAnnotationValue::kDirectional))
		{
			uniformSemantic = convertSpace(parameterName,  MUniformParameter::kSemanticWorldDir);
		}
		else if ( parameterName.rindexW( glslShaderAnnotationValue::kColor) >= 0 ||
					parameterName.rindexW( glslShaderAnnotationValue::kColour) >= 0 ||
					parameterName.rindexW( glslShaderAnnotationValue::kDiffuse) >= 0 ||
					parameterName.rindexW( glslShaderAnnotationValue::kSpecular) >= 0 ||
					parameterName.rindexW( glslShaderAnnotationValue::kAmbient) >= 0)
		{
			uniformSemantic = MUniformParameter::kSemanticColor;
		}
	}

	return uniformSemantic;
}

void GLSLShaderNode::configureUniforms()
{
	fUniformParameters.setLength(0);
	deleteUniformUserData();
	fUIGroupNames.setLength(0);
	fUIGroupParameters.clear();

	MStatus opStatus;

	MStringArray unordedParams;
	fGLSLShaderInstance->parameterList(unordedParams);

	// sort parameters by UIOrder annotation
	std::multimap<int, MString> orderedParams;

	const unsigned int numParams = unordedParams.length();
	for (unsigned int i=0; i<numParams; ++i)
	{
		const MString& paramName = unordedParams[i];

		int uiOrder = fGLSLShaderInstance->annotationAsInt(paramName, glslShaderAnnotation::kUIOrder, opStatus);
		if (opStatus != MStatus::kSuccess)
			uiOrder = -1;

		orderedParams.insert( std::make_pair(uiOrder, paramName) );
	}

	// Does the shader want us to use the variable name as maya attribute name (instead of UI name)?
	bool useVariableNameAsAttributeName = true;
	// For now no boolean annotation available use a string annotation instead
	const MString useVariableNameAsAttributeNameValue = fGLSLShaderInstance->techniqueAnnotationAsString(glslShaderAnnotation::kVariableNameAsAttributeName, opStatus);
	if (opStatus == MStatus::kSuccess)
		useVariableNameAsAttributeName = ((STRICMP(useVariableNameAsAttributeNameValue.asChar(), glslShaderAnnotationValue::kValueTrue)==0));

	std::multimap<int, MString>::const_iterator it = orderedParams.begin();
	std::multimap<int, MString>::const_iterator itEnd = orderedParams.end();
	for(; it != itEnd; ++it)
	{
		const MString& paramName = it->second;

		MUniformParameter::DataType uniformDataType = convertToUniformDataType(fGLSLShaderInstance->parameterType(paramName));
		MUniformParameter::DataSemantic uniformSemantic = convertToUniformSemantic(fGLSLShaderInstance->semantic(paramName).asChar());

		//check if the "Space" annotation is defined for uniform, change semantic accordingly.
		uniformSemantic = convertSpace(paramName, uniformSemantic);
		
		// Check for possibilities for unresolved semantics:
		if( uniformSemantic == MUniformParameter::kSemanticUnknown)
		{
			uniformSemantic = guessUnknownSemantics(paramName);
		}

		/*
		The name of the parameter in the attribute editor defaults to the name of the variable associated with the parameter.
		If there is a UIName attribute on the parameter, and the 'kVariableNameAsAttributeName' annotation is not set,
		this name will be used to define all three of the parameter short/long/nice name.
		If the UIName contains spaces or other script unfriendly characters, those will be replaced by underscores in the
		short and long names used in scripting.

		Using UIName as attribute name can lead to ambiguity since UIName annotations are not required to be unique in the effect.
		The MPxHardwareShader class will add numbers at the end of the short/long names as required to make them unique.
		*/
		const MString uiName = fGLSLShaderInstance->uiName(paramName, opStatus);
		const MString uniformName = (useVariableNameAsAttributeName || uiName.length() == 0 ? paramName : sanitizeName(uiName));

		void* uniformUserData = NULL;
		// Since we are using the uiName as uniform name, we won't be able to access the shader parameter using the uniform name
		// save the original shader parameter name as user data of the uniform.
		if( uniformName != paramName ) {
			uniformUserData = createUniformUserData(paramName);
		}

#ifdef _DEBUG_SHADER
		printf("ParamName='%s', ParamType=", paramName.asChar());
#endif

		MUniformParameter uniParam;
		bool validParam = false;

		switch (fGLSLShaderInstance->parameterType(paramName))
		{
		case MHWRender::MShaderInstance::kInvalid:
			{
#ifdef _DEBUG_SHADER
				printf("'Invalid'\n");
#endif
				break;
			}
		case MHWRender::MShaderInstance::kBoolean:
			{
#ifdef _DEBUG_SHADER
				printf("'Boolean'\n");
#endif
				uniParam = MUniformParameter(uniformName, uniformDataType, uniformSemantic, 1, 1, uniformUserData);
				validParam = true;

				void* defaultValue = fGLSLShaderInstance->parameterDefaultValue(paramName, opStatus);
				if (defaultValue != NULL)
				{
					uniParam.setAsBool(static_cast<bool*>(defaultValue)[0]);
				}
				break;
			}
		case MHWRender::MShaderInstance::kInteger:
			{
#ifdef _DEBUG_SHADER
				printf("'Integer'\n");
#endif
				const MString uiFieldNames = fGLSLShaderInstance->annotationAsString(paramName, glslShaderAnnotation::kUIFieldNames, opStatus);
				if (opStatus == MStatus::kSuccess)
				{
					uniformDataType = MUniformParameter::kTypeEnum;
				}

				uniParam = MUniformParameter(uniformName, uniformDataType, uniformSemantic, 1, 1, uniformUserData);
				validParam = true;

				if (uniformDataType == MUniformParameter::kTypeEnum)
				{
					uniParam.setEnumFieldNames(uiFieldNames);
				}
				
				void* defaultValue = fGLSLShaderInstance->parameterDefaultValue(paramName, opStatus);
				if (defaultValue != NULL)
				{
					uniParam.setAsInt(static_cast<int*>(defaultValue)[0]);
				}
				break;
			}
		case MHWRender::MShaderInstance::kFloat:
			{
#ifdef _DEBUG_SHADER
				printf("'Float'\n");
#endif
				uniParam = MUniformParameter(uniformName, uniformDataType, uniformSemantic, 1, 1, uniformUserData);
				validParam = true;

				void* defaultValue = fGLSLShaderInstance->parameterDefaultValue(paramName, opStatus);
				if (defaultValue != NULL)
				{
					uniParam.setAsFloat(static_cast<float*>(defaultValue)[0]);
				}
				break;
			}
		case MHWRender::MShaderInstance::kFloat2:
			{
#ifdef _DEBUG_SHADER
				printf("'Float2'\n");
#endif
				uniParam = MUniformParameter(uniformName, uniformDataType, uniformSemantic, 2, 1, uniformUserData);
				validParam = true;

				void* defaultValue = fGLSLShaderInstance->parameterDefaultValue(paramName, opStatus);
				if (defaultValue != NULL)
				{
					uniParam.setAsFloatArray(static_cast<float*>(defaultValue),2);
				}
				break;
			}
		case MHWRender::MShaderInstance::kFloat3:
			{
#ifdef _DEBUG_SHADER
				printf("'Float3'\n");
#endif
				uniParam = MUniformParameter(uniformName, uniformDataType, uniformSemantic, 3, 1, uniformUserData);
				validParam = true;

				void* defaultValue = fGLSLShaderInstance->parameterDefaultValue(paramName, opStatus);
				if (defaultValue != NULL)
				{
					uniParam.setAsFloatArray(static_cast<float*>(defaultValue),3);
				}
				break;
			}
		case MHWRender::MShaderInstance::kFloat4:
			{
#ifdef _DEBUG_SHADER
				printf("'Float4'\n");
#endif
				uniParam = MUniformParameter(uniformName, uniformDataType, uniformSemantic, 4, 1, uniformUserData);
				validParam = true;

				void* defaultValue = fGLSLShaderInstance->parameterDefaultValue(paramName, opStatus);
				if (defaultValue != NULL)
				{
					uniParam.setAsFloatArray(static_cast<float*>(defaultValue),4);
				}
				break;
			}
		case MHWRender::MShaderInstance::kFloat4x4Row:
			{
#ifdef _DEBUG_SHADER
				printf("'Float4x4Row'\n");
#endif
				uniParam = MUniformParameter(uniformName, uniformDataType, uniformSemantic, 4, 4, uniformUserData);
				validParam = true;

				void* defaultValue = fGLSLShaderInstance->parameterDefaultValue(paramName, opStatus);
				if (defaultValue != NULL)
				{
					uniParam.setAsFloatArray(static_cast<float*>(defaultValue),16);
				}
				break;
			}
		case MHWRender::MShaderInstance::kFloat4x4Col:
			{
#ifdef _DEBUG_SHADER
				printf("'Float4x4Col'\n");
#endif
				uniParam = MUniformParameter(uniformName, uniformDataType, uniformSemantic, 4, 4, uniformUserData);
				validParam = true;

				void* defaultValue = fGLSLShaderInstance->parameterDefaultValue(paramName, opStatus);
				if (defaultValue != NULL)
				{
					uniParam.setAsFloatArray(static_cast<float*>(defaultValue),16);
				}
				break;
			}
		case MHWRender::MShaderInstance::kTexture1:
		case MHWRender::MShaderInstance::kTexture2:
		case MHWRender::MShaderInstance::kTexture3:
		case MHWRender::MShaderInstance::kTextureCube:
			{
#ifdef _DEBUG_SHADER
				printf("'Texture'\n");
#endif
				uniParam = MUniformParameter(uniformName, uniformDataType, uniformSemantic, 1, 1, uniformUserData);
				validParam = true;
				
				const MString resourceName = fGLSLShaderInstance->resourceName(paramName, opStatus);
				if (opStatus == MStatus::kSuccess)
				{
					if( MFileObject::isAbsolutePath(resourceName) )
					{
						//if ResourceName is a full path, retain it as is
						uniParam.setAsString(resourceName);
					}
					else if( MFileObject::isAbsolutePath(fEffectName) )
					{
						MFileObject fileObj;
						fileObj.setRawFullName(fEffectName);

						uniParam.setAsString(fileObj.rawPath() + MString("/") + resourceName);
					}
					else
					{
						uniParam.setAsString(resourceName);
					}
				}
				break;
			}
		case MHWRender::MShaderInstance::kSampler:
			{
#ifdef _DEBUG_SHADER
				printf("'Sampler'\n");
#endif
				MUniformParameter uniParam(uniformName, uniformDataType, uniformSemantic);
				validParam = true;
				break;
			}
		default:
			{
#ifdef _DEBUG_SHADER
				printf("'Unknown'\n");
#endif
				break;
			}
		}

		if(validParam)
		{
			configureUniformUI(paramName, uniParam);

			fUniformParameters.append(uniParam);	
			//check if parameter is lighting param and update light info accordingly
			updateLightInfoFromSemantic(paramName, fUniformParameters.length()-1);
		}
	}

	setUniformParameters(fUniformParameters, true);

	// Build the UI groups
	for(int i = 0; i < fUniformParameters.length(); ++i)
	{
		// Now that the parameters were pushed to the MPxHardwareShader,
		// uniform mapping may have changed the internal names of the parameters.
		// Use the uniform attribute short names to build the UI group lists,
		// this is the names the AE will also use

		MUniformParameter uniformParam = fUniformParameters.getElement(i);
		MPlug uniformPlug(uniformParam.getPlug());
		if (uniformPlug.isNull())
			continue;

		MFnAttribute uniformAttribute(uniformPlug.attribute());
		if (uniformAttribute.isHidden())
			continue;

		MString parameterName = uniformParam.name();
		if( uniformParam.userData() != NULL ) {
			parameterName = uniformUserDataToMString(uniformParam.userData());
		}

		MString uiGroupName = fGLSLShaderInstance->annotationAsString(parameterName, glslShaderAnnotation::kUIGroup, opStatus);
		if (opStatus != MStatus::kSuccess) {
			// UIGroup annotation not found, try again with Object
			uiGroupName = fGLSLShaderInstance->annotationAsString(parameterName, glslShaderAnnotation::kObject, opStatus);
		}

		if(uiGroupName.length() > 0)
		{
			int uiIndex = findInArray(fUIGroupNames, uiGroupName, true /*appendIfNotFound*/);
			if( fUIGroupParameters.size() <= (unsigned int)uiIndex )
				fUIGroupParameters.resize(uiIndex+1);

			const MString uniformName = uniformAttribute.shortName();
			findInArray(fUIGroupParameters[uiIndex], uniformName, true /*appendIfNotFound*/);
		}
	}

	updateImplicitLightParameterCache();
}

void GLSLShaderNode::configureGeometryRequirements()
{
	fVaryingParameters.setLength(0);
	fVaryingParametersUpdateId = 0;

	std::list<MHWRender::MGeometry::Semantic> semanticUsage;

	fGeometryRequirements.clear();
	fGLSLShaderInstance->requiredVertexBuffers( fGeometryRequirements );

	// No set/update available in MVertexBufferDescriptorList :
	// go from top and push a new descriptor while removing the top
	const int nbReq = fGeometryRequirements.length();
	for( int i = 0; i < nbReq; ++i )
	{
		MHWRender::MVertexBufferDescriptor vbDesc;
		fGeometryRequirements.getDescriptor(0, vbDesc);

		const MString semanticName = vbDesc.semanticName();
		const int dimension = vbDesc.dimension();

		MVaryingParameter::MVaryingParameterType dataType = MVaryingParameter::kInvalidParameter;
		switch( vbDesc.dataType() )
		{
			case MHWRender::MGeometry::kFloat:
				dataType = MVaryingParameter::kFloat;
				break;
			case MHWRender::MGeometry::kDouble:
				dataType = MVaryingParameter::kDouble;
				break;
			case MHWRender::MGeometry::kChar:
				dataType = MVaryingParameter::kChar;
				break;
			case MHWRender::MGeometry::kUnsignedChar:
				dataType = MVaryingParameter::kUnsignedChar;
				break;
			case MHWRender::MGeometry::kInt16:
				dataType = MVaryingParameter::kInt16;
				break;
			case MHWRender::MGeometry::kUnsignedInt16:
				dataType = MVaryingParameter::kUnsignedInt16;
				break;
			case MHWRender::MGeometry::kInt32:
				dataType = MVaryingParameter::kInt32;
				break;
			case MHWRender::MGeometry::kUnsignedInt32:
				dataType = MVaryingParameter::kUnsignedInt32;
				break;
			default:
				break;
		}

		const unsigned int usageCount = (unsigned int) std::count(semanticUsage.begin(), semanticUsage.end(), vbDesc.semantic());
		semanticUsage.push_back( vbDesc.semantic() );

		MVaryingParameter::MVaryingParameterSemantic semantic = MVaryingParameter::kNoSemantic;
		MString uiName;
		MString sourceSet;
		switch( vbDesc.semantic() )
		{
			case MHWRender::MGeometry::kPosition:
				semantic = MVaryingParameter::kPosition;
				uiName = glslShaderSemantic::kPosition;
				break;
			case MHWRender::MGeometry::kNormal:
				semantic = MVaryingParameter::kNormal;
				uiName = glslShaderSemantic::kNormal;
				break;
			case MHWRender::MGeometry::kTexture:
				semantic = MVaryingParameter::kTexCoord;
				uiName = glslShaderSemantic::kTexCoord;
				uiName += usageCount;

				sourceSet = "map";
				sourceSet += (usageCount+1);
				break;
			case MHWRender::MGeometry::kColor:
				semantic = MVaryingParameter::kColor;
				uiName = glslShaderSemantic::kColor;
				uiName += usageCount;

				sourceSet = "colorSet";
				if( usageCount > 0 ) {
					sourceSet += usageCount;
				}
				break;
			case MHWRender::MGeometry::kTangent:
				semantic = MVaryingParameter::kTangent;
				uiName = glslShaderSemantic::kTangent;
				break;
			case MHWRender::MGeometry::kBitangent:
				semantic = MVaryingParameter::kBinormal;
				uiName = glslShaderSemantic::kBinormal;
				break;
			default:
				break;
		}

		MVaryingParameter varying(
			uiName,
			dataType,
			dimension, //minDimension,
			dimension, //maxDimension,
			dimension,
			semantic,
			sourceSet,
			false, // invertTexCoords
			semanticName);
		fVaryingParameters.append(varying);

		// Set desired source set as name of the buffer descriptor
		vbDesc.setName(sourceSet);

		// Remove old and append updated descriptor
		fGeometryRequirements.removeAt(0);
		fGeometryRequirements.append(vbDesc);
	}

	setVaryingParameters(fVaryingParameters, true);
}

bool GLSLShaderNode::hasUpdatedVaryingInput() const
{
	// Test if varying parameters have changed
	unsigned int varyingUpdateId = 0;
	for( int i = 0; i < fVaryingParameters.length(); ++i) {
		MVaryingParameter varying = fVaryingParameters.getElement(i);
		varyingUpdateId += varying.getUpdateId();
	}

	return (fVaryingParametersUpdateId != varyingUpdateId);
}

void GLSLShaderNode::updateGeometryRequirements()
{
	unsigned int varyingUpdateId = 0;

	// No set/update available in MVertexBufferDescriptorList :
	// go from top and push a new descriptor while removing the top
	const int nbReq = fGeometryRequirements.length();
	for( int i = 0; i < nbReq; ++i )
	{
		MHWRender::MVertexBufferDescriptor vbDesc;
		fGeometryRequirements.getDescriptor(0, vbDesc);

		MVaryingParameter varying = fVaryingParameters.getElement(i);
		varyingUpdateId += varying.getUpdateId();

		// Update source set
		vbDesc.setName(varying.getSourceSetName());

		// Remove old and append updated descriptor
		fGeometryRequirements.removeAt(0);
		fGeometryRequirements.append(vbDesc);
	}

	fVaryingParametersUpdateId = varyingUpdateId;
}

const MRenderProfile& GLSLShaderNode::profile()
{
	static MRenderProfile sProfile;
	if(sProfile.numberOfRenderers() == 0)
		sProfile.addRenderer(MRenderProfile::kMayaOpenGL);

	return sProfile;
}


MHWRender::MTexture* GLSLShaderNode::loadTexture(const MString& textureName, const MString& layerName, int alphaChannelIdx, int mipmapLevels) const
{
	if(textureName.length() == 0)
		return NULL;

	MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
	if(theRenderer == NULL)
		return NULL;

	MHWRender::MTextureManager*	txtManager = theRenderer->getTextureManager();
	if(txtManager == NULL)
		return NULL;

	// check extension of texture.
	// for HDR EXR files, we tell Maya to skip using exposeControl or it would normalize our RGB values via linear mapping
	// We don't want that for things like Vector Displacement Maps.
	// In the future, other 32bit images can be added, such as TIF, but those currently do not load properly in ATIL and
	// therefor we have to force them to use linear exposure control for them to load at all.
	MString extension;
	int idx = textureName.rindexW(L'.');
	if(idx > 0)
	{
		extension = textureName.substringW( idx+1, textureName.length()-1 );
		extension = extension.toLowerCase();
	}
	bool isEXR = (extension == "exr");

	MHWRender::MTexture* texture = txtManager->acquireTexture( textureName, mipmapLevels, !isEXR, layerName, alphaChannelIdx );

#ifdef _DEBUG_SHADER
	if(texture == NULL)
	{
		printf("-- Texture %s not found.\n", textureName.asChar());
	}
#endif

	return texture;
}

void GLSLShaderNode::updateParameters(const MHWRender::MDrawContext& context, ERenderType renderType) const
{
	if(!fGLSLShaderInstance)
		return;

	// If the render frame stamp did not change, it's likely that this shader is used by multiple objects,
	// and is called more than once in a single frame render.
	// No need to update the light parameters (again) as it's quite costly
	bool updateLightParameters = true;
	bool updateViewParams = false;
	//bool updateTextures = fForceUpdateTexture;
	if(renderType == RENDER_SCENE)
	{
		// We are rendering the scene
		MUint64 currentFrameStamp = context.getFrameStamp();
		updateLightParameters = (currentFrameStamp != fLastFrameStamp);
		updateViewParams = (currentFrameStamp != fLastFrameStamp);
		fLastFrameStamp = currentFrameStamp;
		//fForceUpdateTexture = false;

		const MHWRender::MPassContext & passCtx = context.getPassContext();
		const MStringArray & passSem = passCtx.passSemantics();
		if (passSem.length() == 1 && passSem[0] == MHWRender::MPassContext::kSelectionPassSemantic)
			updateLightParameters = false;
	}
	else if(renderType == RENDER_SWATCH)
	{
		// We are rendering the swatch using current effect
		// Reset the renderId, to be sure that the next updateParameters() will go through
		fLastFrameStamp = (MUint64)-1;
		//fForceUpdateTexture = false;
	}
	else
	{
		// We are rendering the proxy swatch or the uv texture (temporary effect)
		fLastFrameStamp = (MUint64)-1;
		updateLightParameters = false;
		// We need to update the texture when rendering the swatch or uv texture using a custom effect
		//updateTextures = true;
	}

	bool updateTransparencyTextures = false;
	if( renderType == RENDER_SCENE && techniqueIsTransparent() && techniqueSupportsAdvancedTransparency())
	{
		const MHWRender::MFrameContext::TransparencyAlgorithm transAlg = context.getTransparencyAlgorithm();
		if (transAlg == MHWRender::MFrameContext::kDepthPeeling || transAlg == MHWRender::MFrameContext::kWeightedAverage)
		{
			const MHWRender::MPassContext & passCtx = context.getPassContext();
			const MStringArray & passSemantics = passCtx.passSemantics();
			for (unsigned int i = 0; i < passSemantics.length() && !updateTransparencyTextures; ++i)
			{
				const MString& semantic = passSemantics[i];
				if(	semantic == MHWRender::MPassContext::kTransparentPeelSemantic ||
					semantic == MHWRender::MPassContext::kTransparentPeelAndAvgSemantic ||
					semantic == MHWRender::MPassContext::kTransparentWeightedAvgSemantic)
				{
					updateTransparencyTextures = true;
				}
			}
		}
	}

	std::set<int> lightParametersToUpdate;
	if(updateLightParameters)
	{
		getLightParametersToUpdate(lightParametersToUpdate, renderType);
	}

	if(updateLightParameters)
	{
		// Update using draw context properties if light is explicitely connected.
		// Must be done after we have reset lights to their previous values as
		// explicit light connections overrides values stored in shader:
		updateExplicitLightConnections(context, renderType);

		updateImplicitLightConnections(context, renderType);
	}

	for (int i = 0;i <fUniformParameters.length(); ++i)
	{
		MUniformParameter currentUniform = fUniformParameters.getElement(i);
		MString parameterName = currentUniform.name();
		if( currentUniform.userData() != NULL ) {
			parameterName = uniformUserDataToMString(currentUniform.userData());
		}

		if( currentUniform.hasChanged(context) || lightParametersToUpdate.count(i) || (currentUniform.isATexture()) )
		{
			switch (currentUniform.type())
			{
			case MUniformParameter::kTypeFloat:
				if (currentUniform.semantic() == MUniformParameter::kSemanticViewportPixelSize)
				{
					// Temporary patch until GEC-660 is fixed
					{
						static const float resetData[] = { (float)-1, (float)-1 };
						fGLSLShaderInstance->setParameter(parameterName, resetData);
					}
					int width, height;
					context.getRenderTargetSize(width, height);
					const float data[] = { (float)width, (float)height };
					fGLSLShaderInstance->setParameter(parameterName, data);
				}
				else
				{
					const float* data = currentUniform.getAsFloatArray(context);

					if(currentUniform.numElements() == 1)
						fGLSLShaderInstance->setParameter(parameterName, data[0]);
					else
						fGLSLShaderInstance->setParameter(parameterName, data);
				}
				break;
			case MUniformParameter::kTypeInt:
			case MUniformParameter::kTypeEnum:
				fGLSLShaderInstance->setParameter(parameterName, currentUniform.getAsInt(context));
				break;
			case MUniformParameter::kTypeBool:
				fGLSLShaderInstance->setParameter(parameterName, currentUniform.getAsBool(context));
				break;
			case MUniformParameter::kTypeString:
				break;
			default:
				if (currentUniform.isATexture())
				{
					MUniformParameter::DataSemantic sem = currentUniform.semantic();
					if (sem == MUniformParameter::kSemanticTranspDepthTexture) {
						if(updateTransparencyTextures) {
							const MHWRender::MTexture *tex = context.getInternalTexture(MHWRender::MDrawContext::kDepthPeelingTranspDepthTexture);
							MHWRender::MTextureAssignment assignment;
							assignment.texture = (MHWRender::MTexture *)tex;
							fGLSLShaderInstance->setParameter(parameterName, assignment);
						}
					}
					else if (sem == MUniformParameter::kSemanticOpaqueDepthTexture) {
						if(updateTransparencyTextures) {
							const MHWRender::MTexture *tex = context.getInternalTexture(MHWRender::MDrawContext::kDepthPeelingOpaqueDepthTexture);
							MHWRender::MTextureAssignment assignment;
							assignment.texture = (MHWRender::MTexture *)tex;
							fGLSLShaderInstance->setParameter(parameterName, assignment);
						}
					} else {
						MString textureName, layerName;
						int alphaChannelIdx;
						getTextureDesc(context, currentUniform, textureName, layerName, alphaChannelIdx);

						int mipmaplevels = fTechniqueTextureMipmapLevels;
						MStatus opStatus;
						int readMipMapLevels = fGLSLShaderInstance->annotationAsInt(currentUniform.name(), glslShaderAnnotation::kMipmaplevels, opStatus);
						if (opStatus == MStatus::kSuccess)
						{
							mipmaplevels = readMipMapLevels;
						}

						//To have optimal performance for texture creation/load, insert a mipmaplevels value different than 0. The value can be acquired by checking uniform annotation in  shader.
						MHWRender::MTexture* texture = loadTexture(textureName, layerName, alphaChannelIdx, mipmaplevels);

						if (texture)
						{
							MHWRender::MTextureAssignment assignment;
							assignment.texture = texture;
							fGLSLShaderInstance->setParameter(parameterName, assignment);

							MHWRender::MRenderer::theRenderer()->getTextureManager()->releaseTexture(texture);
						}
					}
				}

				break;
			}

		}
	}
}

void GLSLShaderNode::updateOverrideNonMaterialItemParameters(const MHWRender::MDrawContext& context, const MHWRender::MRenderItem* item, RenderItemDesc& renderItemDesc) const
{
	if(!fGLSLShaderInstance)
		return;

	if (!item || item->type() != MHWRender::MRenderItem::OverrideNonMaterialItem)
		return;

	renderItemDesc.isOverrideNonMaterialItem = true;

	unsigned int size;
	{
		static const MString defaultColorParameter("defaultColor");
		const float* defaultColor = item->getShaderFloatArrayParameter(defaultColorParameter, size);
		if(defaultColor && size == 4) {
			static const MString solidColorUniform("gsSolidColor");
			fGLSLShaderInstance->setParameter(solidColorUniform, defaultColor);
		}
	}

	const MHWRender::MGeometry::Primitive primitive = item->primitive();
	if( primitive == MHWRender::MGeometry::kLines || primitive == MHWRender::MGeometry::kLineStrip ) {
		static const MString lineWidthParameter("lineWidth");
		const float* lineWidth = item->getShaderFloatArrayParameter(lineWidthParameter, size);
		if(lineWidth && size == 2 && lineWidth[0] > 1.f && lineWidth[1] > 1.f) {
			static const MString fatLineWidthUniform("gsFatLineWidth");
			fGLSLShaderInstance->setParameter(fatLineWidthUniform, lineWidth);
			renderItemDesc.isFatLine = true;
		}
	}
	else if( primitive == MHWRender::MGeometry::kPoints ) {
		static const MString pointSizeParameter("pointSize");
		const float* pointSize = item->getShaderFloatArrayParameter(pointSizeParameter, size);
		if(pointSize && size == 2 && pointSize[0] > 1.f && pointSize[1] > 1.f) {
			static const MString fatPointSizeUniform("gsFatPointSize");
			fGLSLShaderInstance->setParameter(fatPointSizeUniform, pointSize);
			renderItemDesc.isFatPoint = true;
		}
	}

	fGLSLShaderInstance->updateParameters(context);
}

void GLSLShaderNode::getExternalContent(MExternalContentInfoTable& table) const
{
	addExternalContentForFileAttr(table, sShader);
	MPxHardwareShader::getExternalContent(table);
}

void GLSLShaderNode::setExternalContent(const MExternalContentLocationTable& table)
{
	setExternalContentForFileAttr(sShader, table);
	MPxHardwareShader::setExternalContent(table);
}

MStatus GLSLShaderNode::renderSwatchImage(MImage & image)
{
	//TODO: continue developping to support lighting, ogsfx, cgfx and fx
	if (!fEffectLoaded)
	{
		return MStatus::kSuccess;
	}

	// Let the VP2 renderer do the work for us:
	return MStatus::kNotImplemented;

	// TODO: All things swatch related like disabling displacement and setting up swatch lighting
}

MTypeId GLSLShaderNode::typeId() const
{
	return m_TypeId;
}

MTypeId GLSLShaderNode::TypeID()
{
	return m_TypeId;
}

bool GLSLShaderNode::reload()
{
	return loadEffect(fEffectName);
}

/*
	Here we find light specific semantics on parameters. This will be used
	to properly transfer values from a Maya light to the effect. Parameters
	that have semantics that are not light-like will get the light type
	eNotALight and will not participate in light related code paths.

	We also try to detect the light type that best match this parameter based
	on a substring match for point/spot/directional/ambient strings. We can also
	deduce the light type from extremely specialized semantics like cone angle and
	falloff for a spot light or LP0 for an area light.

	We finally try to group light parameters together into a single logical light
	group using either an "Object" annotation or a substring of the parameter name.

	The light group name is one of:
		- The string value of the "Object" annotation
		- The prefix part of a parameter name that contains either "Light", "light",
		   or a number:
				DirectionalLightColor  ->   DirectionalLight
				scene_light_position   ->   scene_light
				Lamp0Color             ->   Lamp0

	- All light parameters that share a common light group name will be grouped together
		into a single logical light
	- When a logical light is bound to a scene light, all parameter values will be
		transferred in block from the scene light to the logical light
	- The Attribute Editor will show one extra control per logical light that will allow
		to quickly specify how this logical light should be handled by Maya. Options are
		to explicitely bind a scene light, allow automatic binding to any compatible scene
		light, or ignore scene lights and use values stored in the effect parameters.
	- The Attribute Editor will also group all light parameters in separate panels as if
		they were grouped using the UIGroup annotation. See comments on UIGroup annotation
		for more details.
*/
void GLSLShaderNode::updateLightInfoFromSemantic(const MString& parameterName, int uniformParamIndex)
{
	MStatus opStatus;
	if (fGLSLShaderInstance == NULL)
	{
		return;
	}
	//Check for light type from object type
	MString objectAnnotation = fGLSLShaderInstance->annotationAsString(parameterName, glslShaderAnnotation::kObject, opStatus);
	int currentLightIndex = -1;
	ELightType currentLightType = eUndefinedLight;;
	ELightParameterType currentParamType = eUndefined;

	bool hasLightTypeSemantic = false;

	if(opStatus == MStatus::kSuccess)
	{
		currentLightIndex = getIndexForLightName(objectAnnotation, true);
		if(objectAnnotation.rindexW(glslShaderAnnotationValue::kLight) >= 0 || objectAnnotation.rindexW(glslShaderAnnotationValue::kLamp) >= 0)
		{
			currentLightType = eUndefinedLight;
			if(objectAnnotation.rindexW(glslShaderAnnotationValue::kPoint) >= 0)
			{
				currentLightType = ePointLight;
			}
			else if(objectAnnotation.rindexW(glslShaderAnnotationValue::kSpot) >= 0)
			{
				currentLightType = eSpotLight;
			}
			else if(objectAnnotation.rindexW(glslShaderAnnotationValue::kDirectional) >= 0)
			{
				currentLightType = eDirectionalLight;
			}
			else if(objectAnnotation.rindexW(glslShaderAnnotationValue::kAmbient) >= 0)
			{
				currentLightType = eAmbientLight;
			}
		}
		else
		{
			//if object is not a light, return
			return;
		}
	}
	else
	{
		//If parameter doesn't carry an Object annotation, it is not a light
		return;
	}

	MString semanticValueRaw = fGLSLShaderInstance->parameterSemantic(parameterName, opStatus);

	if(opStatus == MStatus::kSuccess)
	{
		const char* semanticValue = semanticValueRaw.asChar();
		
		if( !STRICMP( semanticValue, glslShaderSemantic::kLightColor))
		{
			currentParamType = eLightColor;
		}
		if( !STRICMP( semanticValue, glslShaderSemantic::kLightEnable))
		{
			currentParamType = eLightEnable;
		}
		else if( !STRICMP( semanticValue, glslShaderSemantic::kLightIntensity))
		{
			currentParamType = eLightIntensity;
		}
		else if( !STRICMP( semanticValue, glslShaderSemantic::kLightFalloff) ||
			     !STRICMP( semanticValue, glslShaderSemantic::kFalloff))
		{
			currentLightType = eSpotLight;
			currentParamType = eLightFalloff;
		}
		else if (!STRICMP( semanticValue, glslShaderSemantic::kLightDiffuseColor))
		{
			currentParamType = eLightDiffuseColor;
		}
		else if (!STRICMP( semanticValue, glslShaderSemantic::kLightAmbientColor))
		{
			currentParamType = eLightAmbientColor;
			currentLightType = eAmbientLight;
		}
		else if (!STRICMP( semanticValue, glslShaderSemantic::kLightSpecularColor))
		{
			currentParamType = eLightSpecularColor;
		}
		else if (!STRICMP( semanticValue, glslShaderSemantic::kShadowMap))
		{
			currentParamType = eLightShadowMap;
		}
		else if (!STRICMP( semanticValue, glslShaderSemantic::kShadowMapBias))
		{
			currentParamType = eLightShadowMapBias;
		}
		else if (!STRICMP( semanticValue, glslShaderSemantic::kShadowFlag))
		{
			currentParamType = eLightShadowOn;
		}
		else if (!STRICMP( semanticValue, glslShaderSemantic::kShadowMapMatrix) ||
			     !STRICMP( semanticValue, glslShaderSemantic::kShadowMapXForm))
		{
			//View transformation matrix of the light
			currentParamType = eLightShadowViewProj;
		}
		else if (!STRICMP( semanticValue, glslShaderSemantic::kShadowColor))
		{
			currentParamType = eLightShadowColor;
		}
		else if (!STRICMP( semanticValue, glslShaderSemantic::kHotspot))
		{
			currentParamType = eLightHotspot;
			currentLightType = eSpotLight;
		}
		else if (!STRICMP( semanticValue, glslShaderSemantic::kLightType))
		{
			currentParamType = eLightType;
			hasLightTypeSemantic = true;
		}
		else if (!STRICMP( semanticValue, glslShaderSemantic::kDecayRate))
		{
			currentParamType = eDecayRate;
		}
		else
		{
			bool isLight = (currentLightType != eInvalidLight || findSubstring(parameterName, MString(glslShaderAnnotationValue::kLight)) >= 0);
			if(isLight)
			{
				if( !STRICMP( semanticValue, glslShaderSemantic::kPosition))
				{
					currentParamType = eLightPosition;
				}
				else if( !STRICMP( semanticValue, glslShaderSemantic::kAreaPosition0))
				{
					currentParamType = eLightAreaPosition0;
					currentLightType = eAreaLight;
				}
				else if( !STRICMP( semanticValue, glslShaderSemantic::kAreaPosition1))
				{
					currentParamType = eLightAreaPosition1;
					currentLightType = eAreaLight;
				}
				else if( !STRICMP( semanticValue, glslShaderSemantic::kAreaPosition2))
				{
					currentParamType = eLightAreaPosition2;
					currentLightType = eAreaLight;
				}
				else if( !STRICMP( semanticValue, glslShaderSemantic::kAreaPosition3))
				{
					currentParamType = eLightAreaPosition3;
					currentLightType = eAreaLight;
				}
				else if( !STRICMP( semanticValue, glslShaderSemantic::kDirection))
				{
					currentParamType = eLightDirection;
				}
				else if( !STRICMP( semanticValue, glslShaderSemantic::kColor))
				{
					//
					currentParamType = eLightColor;
				}
				else if( !STRICMP( semanticValue, glslShaderSemantic::kAmbient))
				{
					currentParamType = eLightAmbientColor;
					currentLightType = eAmbientLight;
				}
				else if( !STRICMP( semanticValue, glslShaderSemantic::kDiffuse))
				{
					currentParamType = eLightDiffuseColor;
				}
				else if( !STRICMP( semanticValue, glslShaderSemantic::kSpecular))
				{
					currentParamType = eLightSpecularColor;
				}
			}
		}

		//Compute light index
		if(currentParamType != eUndefined && currentLightIndex ==  -1)
		{
			const char* objectName = parameterName.asChar();
			int truncationPos = -1;

			int lightPos = findSubstring(parameterName, MString(glslShaderAnnotationValue::kLight));
			if (lightPos >= 0)
				truncationPos = lightPos + 5;

			if(truncationPos < 0)
			{
				// last effort, see if there is any digit in the parameter name:
				unsigned int digitPos = 0;
				for ( ; digitPos < parameterName.numChars(); ++digitPos)
					if ( isdigit(objectName[digitPos]) )
						break;
				if ( digitPos < parameterName.numChars() )
					truncationPos = digitPos;
			}
			if (truncationPos >= 0)
			{
				// Need to also skip any digits found after the "light"
				int maxChars = int(parameterName.numChars());
				while (truncationPos < maxChars && isdigit(objectName[truncationPos]))
					++truncationPos;

				currentLightIndex = getIndexForLightName(parameterName.substring(0,truncationPos-1), true);
			}
		}

	}

	//if parameter is not a light or unrecognized semantic, do not add to fLightParameters
	if(/*currentLightType == eUndefinedLight ||*/ currentParamType == eUndefined || currentLightIndex < 0)
		return;
	
	//look for light in fLightParameters to append parameter
	bool parameterFound = false;
	for(int i = 0;i < fLightParameters.size();++i)
	{
		if (fLightParameters[i].mLightIndex == currentLightIndex)
		{
			fLightParameters[i].fConnectableParameters.insert(LightParameterInfo::TConnectableParameters::value_type(uniformParamIndex, currentParamType));
			fLightParameters[i].fHasLightTypeSemantics |= hasLightTypeSemantic;
			parameterFound = true;
			break;
		}
	}

	// If not found, create light parameter and append to fLightParameters
	if (!parameterFound)
	{	
		fLightParameters.push_back(LightParameterInfo(currentLightIndex, currentLightType, hasLightTypeSemantic));
		fLightParameters[fLightParameters.size()-1].fConnectableParameters.insert(LightParameterInfo::TConnectableParameters::value_type(uniformParamIndex, currentParamType));
	}
}

int GLSLShaderNode::getIndexForLightName(const MString& lightName, bool appendLight)
{
	return findInArray(fLightNames, lightName, appendLight);
}

MStringArray GLSLShaderNode::getLightableParameters(int lightIndex, bool showSemantics)
{
	MStringArray retVal;
	if(lightIndex < (int)fLightParameters.size())
	{
		LightParameterInfo& currLight = fLightParameters[lightIndex];
		for (LightParameterInfo::TConnectableParameters::const_iterator idxIter=currLight.fConnectableParameters.begin();
			idxIter != currLight.fConnectableParameters.end();
			++idxIter)
		{
			bool appended = appendParameterNameIfVisible((*idxIter).first, retVal);

			if (appended && showSemantics) {
				int paramType((*idxIter).second);
				retVal.append(getLightParameterSemantic(paramType));
			}
		}
	}
	return retVal;
}

/*
	In the AE we only want to expose visible parameters, so
	test here for parameter visibility:
*/
bool GLSLShaderNode::appendParameterNameIfVisible(int paramIndex, MStringArray& paramArray) const
{
	MUniformParameter uniform = fUniformParameters.getElement(paramIndex);

	MPlug uniformPlug(uniform.getPlug());
	if (uniformPlug.isNull())
		return false;

	MFnAttribute uniformAttribute(uniformPlug.attribute());
	if (uniformAttribute.isHidden())
		return false;

	paramArray.append(uniformAttribute.shortName());
	return true;
}

// Get semantic string back from enum:
MString& GLSLShaderNode::getLightParameterSemantic(int lightParameterType)
{

	if (lightParameterType < 0 || lightParameterType >= eLastParameterType)
		lightParameterType = eUndefined;

	static MStringArray semanticNames;

	if (!semanticNames.length()) {
		semanticNames.append(glslShaderSemantic::kUndefined);
		semanticNames.append(glslShaderSemantic::kPosition);
		semanticNames.append(glslShaderSemantic::kDirection);
		semanticNames.append(glslShaderSemantic::kLightColor);
		semanticNames.append(glslShaderSemantic::kLightSpecularColor);
		semanticNames.append(glslShaderSemantic::kLightAmbientColor);
		semanticNames.append(glslShaderSemantic::kLightDiffuseColor);
		semanticNames.append(glslShaderSemantic::kLightRange);          // Not recognized!
		semanticNames.append(glslShaderSemantic::kFalloff);
		semanticNames.append(glslShaderSemantic::kLightAttenuation0);   // Not recognized!
		semanticNames.append(glslShaderSemantic::kLightAttenuation1);   // Not recognized!
		semanticNames.append(glslShaderSemantic::kLightAttenuation2);   // Not recognized!
		semanticNames.append(glslShaderSemantic::kLightTheta);   // Not recognized!
		semanticNames.append(glslShaderSemantic::kLightPhi);   // Not recognized!
		semanticNames.append(glslShaderSemantic::kShadowMap);
		semanticNames.append(glslShaderSemantic::kShadowMapBias);
		semanticNames.append(glslShaderSemantic::kShadowColor);
		semanticNames.append(glslShaderSemantic::kShadowMapMatrix);
		semanticNames.append(glslShaderSemantic::kShadowFlag);
		semanticNames.append(glslShaderSemantic::kLightIntensity);
		semanticNames.append(glslShaderSemantic::kHotspot);
		semanticNames.append(glslShaderSemantic::kLightEnable);
		semanticNames.append(glslShaderSemantic::kLightType);
		semanticNames.append(glslShaderSemantic::kDecayRate);
		semanticNames.append(glslShaderSemantic::kAreaPosition0);
		semanticNames.append(glslShaderSemantic::kAreaPosition1);
		semanticNames.append(glslShaderSemantic::kAreaPosition2);
		semanticNames.append(glslShaderSemantic::kAreaPosition3);
	}
	return semanticNames[lightParameterType];
}


///////////////////////////////////////////////////////
// This is where we create the light connection attributes
// when a shader is first assigned. When a scene is loaded,
// we only need to retrieve the dynamic attributes that were
// created by the persistence code. The code also handles
// re-creating the attributes if the light group names were
// changed in the effect file.
void GLSLShaderNode::refreshLightConnectionAttributes(bool inSceneUpdateNotification)
{
	if ( inSceneUpdateNotification || (!MFileIO::isReadingFile() && !MFileIO::isOpeningFile()) )
	{
		MFnDependencyNode fnDepThisNode(thisMObject());
		MStatus status;
		for (size_t iLi=0; iLi<fLightParameters.size(); ++iLi)
		{
			LightParameterInfo& currLight(fLightParameters[iLi]);
			MString sanitizedLightGroupName = sanitizeName(fLightNames[(unsigned int)iLi]);

			// If the attributes are not there at this time then create them.
			if (currLight.fAttrUseImplicit.isNull())
				currLight.fAttrUseImplicit = fnDepThisNode.attribute(sanitizedLightGroupName + "_use_implicit_lighting");

			if (currLight.fAttrUseImplicit.isNull())
			{
				// Create:
				MFnNumericAttribute fnAttr;
				MString attrName = sanitizedLightGroupName + "_use_implicit_lighting";
				MObject attrUseImplicit = fnAttr.create(attrName , attrName, MFnNumericData::kBoolean);
				fnAttr.setDefault(true);
				fnAttr.setKeyable(false);
				fnAttr.setStorable(true);
				fnAttr.setAffectsAppearance(true);
				if (!attrUseImplicit.isNull())
				{
					MDGModifier implicitModifier;
					status = implicitModifier.addAttribute(thisMObject(), attrUseImplicit);
					if (status.statusCode() == MStatus::kSuccess)
					{
						status = implicitModifier.doIt();
						if (status.statusCode() == MStatus::kSuccess)
						{
							currLight.fAttrUseImplicit = attrUseImplicit;
						}
					}
				}
			}

			if (currLight.fAttrConnectedLight.isNull())
			{
				currLight.fAttrConnectedLight = fnDepThisNode.attribute(sanitizedLightGroupName + "_connected_light");;
			}
			if (currLight.fAttrConnectedLight.isNull())
			{
				MFnMessageAttribute msgAttr;
				MString attrName = sanitizedLightGroupName + "_connected_light";
				MObject attrConnectedLight = msgAttr.create(attrName, attrName);
				msgAttr.setAffectsAppearance(true);
				if (!attrConnectedLight.isNull())
				{
					MDGModifier implicitModifier;
					status = implicitModifier.addAttribute(thisMObject(), attrConnectedLight);
					if (status.statusCode() == MStatus::kSuccess)
					{
						status = implicitModifier.doIt();
						if (status.statusCode() == MStatus::kSuccess)
						{
							currLight.fAttrConnectedLight = attrConnectedLight;
						}
					}
				}
			}
		}
	}
	else
	{
		// Hmmm. Really not a good idea to start adding parameters while scene is not fullly loaded.
		// Ask to be called back at a later time:
		PostSceneUpdateAttributeRefresher::add(this);
	}
}

/*
	Implicit light connection:
	=========================

	In this function we want to bind the M shader lights to the best
	subset of the N scene lights found in the draw context. For performance
	we keep count of the number of light to connect and short-circuit loops
	when we ran out of lights to bind on either the shader or draw context side.

	This function can be called in 3 different context:

	- Scene: We have multiple lights in the draw context and we need to
	         find a light that is compatible with the shader whenever the
			 cached light is not found and it is not explicitly connected.
	- Default light: The draw context will contain only a single light and
	                 it needs to override light in all three lighting modes.
	- Swatch: Same requirements as "Default Light", but does not override
	          lights in "Use Shader Settings" mode.

	We need to keep track of which lights are implicitly/explicitly bound to
	make sure we do not automatically bind the same light more than once.

	Scene ligths that are part of the scene but cannot be found in the draw
	context are either invisible, disabled, or in any other lighting combination
	(like "Use Selected Light") where we do not want to see the lighting in the
	shader. For these lights we turn the shader lighting "off" by setting
	the shader parameter values to black, with zero intensity.
*/
void GLSLShaderNode::updateImplicitLightConnections(const MHWRender::MDrawContext& context, ERenderType renderType) const
{
	if(renderType != RENDER_SCENE && renderType != RENDER_SWATCH)
		return;

	bool ignoreLightLimit = true;
	MHWRender::MDrawContext::LightFilter lightFilter = MHWRender::MDrawContext::kFilteredToLightLimit;
	if (ignoreLightLimit)
	{
		lightFilter = MHWRender::MDrawContext::kFilteredIgnoreLightLimit;
	}
	unsigned int nbSceneLights = context.numberOfActiveLights(lightFilter);
	unsigned int nbSceneLightsToBind = nbSceneLights;
	bool implicitLightWasRebound = false;

	// Detect headlamp scene rendering mode:
	if(renderType == RENDER_SCENE && nbSceneLights == 1)
	{
		MHWRender::MLightParameterInformation* sceneLightParam = context.getLightParameterInformation( 0 );
		const ELightType sceneLightType = getLightType(sceneLightParam);
		if(sceneLightType == GLSLShaderNode::eDefaultLight )
		{
			// Swatch and headlamp are the same as far as
			// implicit light connection is concerned:
			renderType = RENDER_SCENE_DEFAULT_LIGHT;
		}
	}

	unsigned int nbShaderLights = (unsigned int)fLightParameters.size();
	unsigned int nbShaderLightsToBind = nbShaderLights;
	// Keep track of the shader lights that were treated : binding was successful
	std::vector<bool> shaderLightTreated(nbShaderLights, false);
	std::vector<bool> shaderLightUsesImplicit(nbShaderLights, false);

	MFnDependencyNode depFn( thisMObject() );

	// Keep track of the scene lights that were used : binding was successful
	std::vector<bool> sceneLightUsed(nbSceneLights, false);

	// Upkeep pass.
	//
	// We want to know exactly which shader light will later require implicit
	// connection, and which scene lights are already used. We also remember
	// lights that were previously bound using the cached light parameter of
	// the light group info structure. It the cached light exists, and is
	// still available for automatic binding, we immediately reuse it.
	if(renderType == RENDER_SCENE)
	{
		// Find out all explicitely connected lights and mark them as already
		// bound.
		for(unsigned int shaderLightIndex = 0;
			shaderLightIndex < nbShaderLights && nbShaderLightsToBind && nbSceneLightsToBind;
			++shaderLightIndex )
		{
			const LightParameterInfo& shaderLightInfo = fLightParameters[shaderLightIndex];
			MPlug thisLightConnectionPlug = depFn.findPlug(shaderLightInfo.fAttrConnectedLight, true);
			if (thisLightConnectionPlug.isConnected())
			{
				// Find the light connected as source to this plug:
				MPlugArray srcCnxArray;
				thisLightConnectionPlug.connectedTo(srcCnxArray,true,false);
				if (srcCnxArray.length() > 0)
				{
					MPlug sourcePlug = srcCnxArray[0];
					for(unsigned int sceneLightIndex = 0; sceneLightIndex < nbSceneLights; ++sceneLightIndex)
					{
						MHWRender::MLightParameterInformation* sceneLightParam = context.getLightParameterInformation( sceneLightIndex, lightFilter );
						if(sceneLightParam->lightPath().node() == sourcePlug.node())
						{
							sceneLightUsed[sceneLightIndex] = true;
							nbSceneLightsToBind--;
						}
					}
					if (!shaderLightInfo.fCachedImplicitLight.isNull())
					{
						(const_cast<LightParameterInfo&>(shaderLightInfo)).fCachedImplicitLight = MObject();
						// Light is explicitely connected, so parameters are locked:
						setLightParameterLocking(shaderLightInfo, true);
						implicitLightWasRebound = true;
					}
				}
			}
		}

		// Update cached implicit lights:
		for(unsigned int shaderLightIndex = 0;
			shaderLightIndex < nbShaderLights && nbShaderLightsToBind;
			++shaderLightIndex )
		{
			// See if this light uses implicit connections:
			const LightParameterInfo& shaderLightInfo = fLightParameters[shaderLightIndex];
			MPlug useImplicitPlug = depFn.findPlug( shaderLightInfo.fAttrUseImplicit, false );
			if( !useImplicitPlug.isNull() ) {
				bool useImplicit;
				useImplicitPlug.getValue( useImplicit );
				shaderLightUsesImplicit[shaderLightIndex] = useImplicit;
				if (useImplicit)
				{
					// Make sure cached light is still in model:
					if (!shaderLightInfo.fCachedImplicitLight.isNull())
					{
						MStatus status;
						MFnDagNode lightDagNode(shaderLightInfo.fCachedImplicitLight, &status);
						if (status.statusCode() == MStatus::kSuccess && lightDagNode.inModel() ) {

							// Try to connect to the cached light:
							MHWRender::MLightParameterInformation* matchingSceneLightParam = NULL;
							unsigned int sceneLightIndex = 0;

							for( ; sceneLightIndex < nbSceneLights; ++sceneLightIndex)
							{
								MHWRender::MLightParameterInformation* sceneLightParam = context.getLightParameterInformation( sceneLightIndex, lightFilter );

								if( sceneLightParam->lightPath().node() == shaderLightInfo.fCachedImplicitLight )
								{
									matchingSceneLightParam = sceneLightParam;
									break;
								}
							}

							if (matchingSceneLightParam)
							{
								if (!sceneLightUsed[sceneLightIndex])
								{
									connectLight(shaderLightInfo, matchingSceneLightParam);
									sceneLightUsed[sceneLightIndex] = true;			// mark this scene light as used
									nbSceneLightsToBind--;
									shaderLightTreated[shaderLightIndex] = true;	// mark this shader light as binded
									nbShaderLightsToBind--;
								}
								else
								{
									setLightRequiresShadows(shaderLightInfo.fCachedImplicitLight, false);

									// Light already in use, clear the cache to allow binding at a later stage:
									(const_cast<LightParameterInfo&>(shaderLightInfo)).fCachedImplicitLight = MObject();
									setLightParameterLocking(shaderLightInfo, false);
									implicitLightWasRebound = true;
								}
							}
							else
							{
								// mark this shader light as bound even if not found in DC
								turnOffLight(shaderLightInfo);
								shaderLightTreated[shaderLightIndex] = true;
								nbShaderLightsToBind--;
							}
						}
						else
						{
							// Note that we don't need to clear the requirement for
							// implicit shadow maps here as light deletion is already handled by the renderer
							//
							// Light is not in the model anymore, allow rebinding:
							(const_cast<LightParameterInfo&>(shaderLightInfo)).fCachedImplicitLight = MObject();
							setLightParameterLocking(shaderLightInfo, false);
							implicitLightWasRebound = true;
						}
					}
				}
				else
				{
					// This light is either explicitly bound, or in the
					// "Use Shader Settings" mode, so we have one less
					// shader light to bind:
					nbShaderLightsToBind--;
				}
			}
		}
	}
	else
	{
		// Here we are in swatch or default light mode and must override all light connection
		// by marking them all as available for "Automatic Bind"
		for(unsigned int shaderLightIndex = 0;
			shaderLightIndex < nbShaderLights && nbShaderLightsToBind && nbSceneLightsToBind;
			++shaderLightIndex )
		{
			const LightParameterInfo& shaderLightInfo = fLightParameters[shaderLightIndex];
			MPlug thisLightConnectionPlug = depFn.findPlug(shaderLightInfo.fAttrConnectedLight, true);

			bool useImplicit = true;
			MPlug useImplicitPlug = depFn.findPlug( shaderLightInfo.fAttrUseImplicit, false );
			if( !useImplicitPlug.isNull() ) {
				useImplicitPlug.getValue( useImplicit );
			}

			if (thisLightConnectionPlug.isConnected() || useImplicit || renderType == RENDER_SCENE_DEFAULT_LIGHT )
			{
				shaderLightUsesImplicit[shaderLightIndex] = true;
			}
			else
			{
				// In swatch rendering, lights in the "Use Shader Settings" mode are not
				// overridden:
				nbShaderLightsToBind--;
			}
		}
	}

	// First pass ... try to connect each shader lights with the best scene light possible.
	// This means for each light whose type is explicitly known, we try to find the first
	// draw context light that is of the same type.
	//
	// The type of the shader light is deduced automatically first by looking for a substring
	// match in the light "Object" annotation, then by searching the parameter name, and finally
	// by checking which combination of position/direction semantics the light requires:
	if(renderType == RENDER_SCENE)
		fImplicitAmbientLight = -1;

	for(unsigned int shaderLightIndex = 0;
		shaderLightIndex < nbShaderLights && nbShaderLightsToBind && nbSceneLightsToBind;
		++shaderLightIndex )
	{
		const LightParameterInfo& shaderLightInfo = fLightParameters[shaderLightIndex];
		const ELightType shaderLightType = shaderLightInfo.mLightType;

		if(!shaderLightUsesImplicit[shaderLightIndex] || shaderLightTreated[shaderLightIndex] == true)
			continue;

		for(unsigned int sceneLightIndex = 0; sceneLightIndex < nbSceneLights; ++sceneLightIndex)
		{
			if(sceneLightUsed[sceneLightIndex] == true)
				continue;

			MHWRender::MLightParameterInformation* sceneLightParam = context.getLightParameterInformation( sceneLightIndex, lightFilter );

			const ELightType sceneLightType = getLightType(sceneLightParam);
			if( shaderLightType == sceneLightType || shaderLightInfo.fHasLightTypeSemantics )
			{
				connectLight(shaderLightInfo, sceneLightParam, renderType);

				shaderLightTreated[shaderLightIndex] = true;	// mark this shader light as binded
				nbShaderLightsToBind--;

				// Rendering swatch needs to drive all lights, except if they have a light type semantics,
				// where we only need to drive one:
				if (renderType != RENDER_SWATCH || shaderLightInfo.fHasLightTypeSemantics)
				{
					sceneLightUsed[sceneLightIndex] = true;			// mark this scene light as used
					nbSceneLightsToBind--;
				}

				if(renderType == RENDER_SCENE)
				{
					setLightRequiresShadows(shaderLightInfo.fCachedImplicitLight, true);

					(const_cast<LightParameterInfo&>(shaderLightInfo)).fCachedImplicitLight = sceneLightParam->lightPath().node();
					setLightParameterLocking(shaderLightInfo, true);
					implicitLightWasRebound = true;

					// only update 'fImplicitAmbientLight' if it was not set yet. This allows the user to
					// manually bind an ambient light into the shader and still see any implicit 'Ambient' lighting bound in AE.
					if (sceneLightType == eAmbientLight && fImplicitAmbientLight < 0)
						fImplicitAmbientLight = shaderLightIndex;
				}
				else
				{
					// Will need to refresh defaults on next scene redraw:
					(const_cast<LightParameterInfo&>(shaderLightInfo)).fIsDirty = true;
				}

				break;
			}
		}
	}

	// Second pass ... connect remaining shader lights with scene lights that are not yet connected.
	//
	// In this pass, we consider compatible all lights that possess a superset of the
	// semantics required by the shader light, so a scene spot light can be bound to
	// shader lights requesting only a position, or a direction, and any light can bind
	// to a shader light that only requires a color:
	for(unsigned int shaderLightIndex = 0;
		shaderLightIndex < nbShaderLights && nbShaderLightsToBind && nbSceneLightsToBind;
		++shaderLightIndex )
	{
		if(!shaderLightUsesImplicit[shaderLightIndex] || shaderLightTreated[shaderLightIndex] == true)
			continue;

		const LightParameterInfo& shaderLightInfo = fLightParameters[shaderLightIndex];
		const ELightType shaderLightType = shaderLightInfo.mLightType;

		for(unsigned int sceneLightIndex = 0; sceneLightIndex < nbSceneLights; ++sceneLightIndex)
		{
			if(sceneLightUsed[sceneLightIndex] == true)
				continue;

			MHWRender::MLightParameterInformation* sceneLightParam = context.getLightParameterInformation( sceneLightIndex, lightFilter );

			const ELightType sceneLightType = getLightType(sceneLightParam);
			if( isLightAcceptable(shaderLightType, sceneLightType) )
			{
				connectLight(shaderLightInfo, sceneLightParam, renderType);

				shaderLightTreated[shaderLightIndex] = true;	// mark this shader light as binded
				nbShaderLightsToBind--;

				// Rendering swatch needs to drive all lights, except if they have a light type semantics,
				// where we only need to drive one:
				if (renderType != RENDER_SWATCH || shaderLightInfo.fHasLightTypeSemantics)
				{
					sceneLightUsed[sceneLightIndex] = true;			// mark this scene light as used
					nbSceneLightsToBind--;
				}

				if(renderType == RENDER_SCENE)
				{
					(const_cast<LightParameterInfo&>(shaderLightInfo)).fCachedImplicitLight = sceneLightParam->lightPath().node();
					setLightParameterLocking(shaderLightInfo, true);
					implicitLightWasRebound = true;

					setLightRequiresShadows(shaderLightInfo.fCachedImplicitLight, true);
				}
				else
				{
					// Will need to refresh defaults on next scene redraw:
					(const_cast<LightParameterInfo&>(shaderLightInfo)).fIsDirty = true;
				}

				break;
			}
		}
	}

	// Final pass: shutdown all implicit lights that were not bound
	for(unsigned int shaderLightIndex = 0;
		shaderLightIndex < nbShaderLights && nbShaderLightsToBind;
		++shaderLightIndex )
	{
		if(!shaderLightUsesImplicit[shaderLightIndex] || shaderLightTreated[shaderLightIndex] == true)
			continue;

		const LightParameterInfo& shaderLightInfo = fLightParameters[shaderLightIndex];

		turnOffLight(shaderLightInfo);

		if(renderType != RENDER_SCENE)
		{
			// Will need to refresh defaults on next scene redraw:
			(const_cast<LightParameterInfo&>(shaderLightInfo)).fIsDirty = true;
		}
	}

	// If during this update phase we changed any of the cached implicit light
	// objects, we need to trigger a refresh of the attribute editor light binding
	// information to show the current light connection settings. Multiple requests
	// are pooled by the refresher and only one request is sent to the AE in the next
	// idle window.
	if (implicitLightWasRebound)
		IdleAttributeEditorImplicitRefresher::activate();
}

/*
	This function rebuilds all the shader light information structures:

	fLightParameters: Main struct that contains the frequently use runtime information
		Contains:
			fLightType: What kind of scene light drives this shader light completely
			fHasLightTypeSemantics: Is the shader light code able to adapt to multiple light types?
			fIsDirty: Should we refresh the shader light parameter values at the next redraw?
			fConnectableParameters: Set of indices in the uniform parameter array that define this shader light
			fAttrUseImplicit: Boolean attribute whose value is true when in "Automatic Bind" mode
			fAttrConnectedLight: Message attribute that is connected to a light shape for explicit binds
			fCachedImplicitLight: Reference to the light shape that was automatically bound during last redraw

	 fLightDescriptions: String array containing pairs of (Light Group Name, Light Group Type) returned by
	                     "GLSLShader -listLightInformation" query and used by the AE to create the light
						 connection panel and to filter which scene lights can appear in the dropdowns for
						 explicit connection
*/
void GLSLShaderNode::updateImplicitLightParameterCache()
{
	MFnDependencyNode fnDepThisNode(thisMObject());
	MDGModifier implicitModifier;

	// The attributes for connected lights and implicit binding can be created from
	// the persistence. Try to preserve them if possible.
	bool updateConnectionAttributes = ( !MFileIO::isReadingFile() && !MFileIO::isOpeningFile() );
	if ( updateConnectionAttributes ) {
		// Do not update if the light groups are exactly the same:
		//   (happens a lot when switching from one technique to another)
		if ( fLightParameters.size() == fLightNames.length() )
		{
			updateConnectionAttributes = false;
			for (size_t iLi=0; iLi<fLightParameters.size(); ++iLi) {
				MString newName = sanitizeName(fLightNames[(unsigned int)iLi]) + "_use_implicit_lighting";
				MStatus status;
				MFnAttribute currentAttribute(fLightParameters[iLi].fAttrUseImplicit, &status);
				if (status.statusCode() != MStatus::kSuccess || currentAttribute.name() != newName ) {
					updateConnectionAttributes = true;
					break;
				}
			}
		}
	}

	if ( updateConnectionAttributes ) {
		for (size_t iLi=0; iLi<fLightParameters.size(); ++iLi)
		{
			if(fLightParameters[iLi].fAttrUseImplicit.isNull() == false)
				implicitModifier.removeAttribute(thisMObject(), fLightParameters[iLi].fAttrUseImplicit);
			if(fLightParameters[iLi].fAttrConnectedLight.isNull() == false)
				implicitModifier.removeAttribute(thisMObject(), fLightParameters[iLi].fAttrConnectedLight);
		}
	}
	implicitModifier.doIt();
	refreshLightConnectionAttributes();

	/*
		Once all light group information is found, we can generate
		the light parameter info array for the AE
	*/
	fLightDescriptions.clear();
	LightParameterInfoVec::iterator iterLight = fLightParameters.begin();
	unsigned int lightIndex = 0;
	for(;iterLight != fLightParameters.end();++iterLight, ++lightIndex)
	{
		fLightDescriptions.append(fLightNames[lightIndex]);

		static const MString kInvalid("invalid");
		static const MString kUndefined("undefined");
		static const MString kSpot("spot");
		static const MString kPoint("point");
		static const MString kDirectional("directional");
		static const MString kAmbient("ambient");
		static const MString kArea("area");

		MString lightType = kInvalid;
		switch(iterLight->mLightType)
		{
		case eUndefinedLight:
			lightType = kUndefined;
			break;
		case eSpotLight:
			lightType = kSpot;
			break;
		case ePointLight:
			lightType = kPoint;
			break;
		case eDirectionalLight:
			lightType = kDirectional;
			break;
		case eAmbientLight:
			lightType = kAmbient;
			break;
		case eAreaLight:
			lightType = kArea;
			break;
		default:
			break;
		};
		fLightDescriptions.append(lightType);
	}
}

/*
	Traverse all explicit light connections and refresh the shader data if the light
	is found in the draw context, otherwise turn off the light.

	This is also where we handle the special case of the merged ambient lights by
	refreshing the connected ambient light, but only if we found the merged one
	inside the draw context. Not finding ambient lights in the draw context mean that
	they are all invisible, or disabled, or otherwise not drawn.

*/
void GLSLShaderNode::updateExplicitLightConnections(const MHWRender::MDrawContext& context, ERenderType renderType) const
{
	if(renderType != RENDER_SCENE)
		return;

	unsigned int nbShaderLights = (unsigned int)fLightParameters.size();
	if(nbShaderLights == 0)
		return;

	bool ignoreLightLimit = true;
	MHWRender::MDrawContext::LightFilter lightFilter = MHWRender::MDrawContext::kFilteredToLightLimit;
	if (ignoreLightLimit)
	{
		lightFilter = MHWRender::MDrawContext::kFilteredIgnoreLightLimit;
	}
	unsigned int nbSceneLights = context.numberOfActiveLights(lightFilter);

	MFnDependencyNode thisDependNode;
	thisDependNode.setObject(thisMObject());

	for(size_t shaderLightIndex = 0; shaderLightIndex <nbShaderLights; ++shaderLightIndex )
	{
		const LightParameterInfo& shaderLightInfo = fLightParameters[shaderLightIndex];

		MPlug thisLightConnectionPlug = thisDependNode.findPlug(shaderLightInfo.fAttrConnectedLight, true);
		if (thisLightConnectionPlug.isConnected())
		{
			// Find the light connected as source to this plug:
			MPlugArray srcCnxArray;
			thisLightConnectionPlug.connectedTo(srcCnxArray,true,false);
			if (srcCnxArray.length() > 0)
			{
				MPlug sourcePlug = srcCnxArray[0];
				MObject sourceLight(sourcePlug.node());
				bool bHasAmbient = false;

				bool bLightEnabled = false;
				unsigned int sceneLightIndex = 0;
				for(; sceneLightIndex < nbSceneLights; ++sceneLightIndex)
				{
					MHWRender::MLightParameterInformation* sceneLightParam = context.getLightParameterInformation( sceneLightIndex, lightFilter );
					if(sceneLightParam->lightPath().node() == sourceLight)
					{
						setLightRequiresShadows(sourceLight, true);

						// Use connectLight to transfer all values.
						connectLight(shaderLightInfo, sceneLightParam);

						// Keep light visibility state in case shader cares:
						MFloatArray floatVals;
						static MString kLightOn("lightOn");
						sceneLightParam->getParameter( kLightOn, floatVals );
						bLightEnabled = (floatVals.length() == 0 || floatVals[0] > 0) ? true : false;
						break;
					}

					if (eAmbientLight == getLightType(sceneLightParam))
					{
						bHasAmbient = true;
						bLightEnabled = true;
					}
				}

				if (bHasAmbient && sceneLightIndex == nbSceneLights)
					bLightEnabled = connectExplicitAmbientLight(shaderLightInfo, sourceLight);

				// Adjust LightEnable parameter if it exists based on the presence of the light in the draw context:
				if (!bLightEnabled)
				{
					turnOffLight(shaderLightInfo);
				}
			}
		}
	}
}

bool GLSLShaderNode::connectExplicitAmbientLight(const LightParameterInfo& lightInfo, const MObject& sourceLight) const
{
	bool bDidConnect = false;
	if (sourceLight.hasFn(MFn::kAmbientLight))
	{
		MStatus status;
		MFnAmbientLight ambientLight(sourceLight, &status);

		if (status == MStatus::kSuccess)
		{
			bDidConnect = true;
			LightParameterInfo::TConnectableParameters::const_iterator it    = lightInfo.fConnectableParameters.begin();
			LightParameterInfo::TConnectableParameters::const_iterator itEnd = lightInfo.fConnectableParameters.end();
			for (; it != itEnd; ++it)
			{
				const int parameterIndex = it->first;
				const int parameterType  = it->second;

				switch (parameterType)
				{
				case eLightType:
					//setParameterAsScalar(parameterIndex, (int)eAmbientLight);
					fUniformParameters.getElement(parameterIndex).setAsInt((int)eAmbientLight);
					break;

				case eLightEnable:
					//setParameterAsScalar(parameterIndex, true);
					fUniformParameters.getElement(parameterIndex).setAsBool(true);
					break;

				case eLightColor:
				case eLightAmbientColor:
				case eLightSpecularColor:
				case eLightDiffuseColor:
					{
						//update color
						MColor ambientColor(ambientLight.color());
						float color[3];
						ambientColor.get(color);
						//setParameterAsVector(parameterIndex, color);
						fUniformParameters.getElement(parameterIndex).setAsFloatArray(color, 3);
					}
					break;

				case eLightIntensity:
					//setParameterAsScalar(parameterIndex, ambientLight.intensity());
					fUniformParameters.getElement(parameterIndex).setAsFloat(ambientLight.intensity());
					break;
				}
			}
		}
	}
	return bDidConnect;
}

/*
	This is where we explicitely connect a light selected by the user
	by creating an explicit connection between the "lightData" of the
	light shape and the "*_connected_light" attribute. This connection
	can be traversed by the Attribute Editor to navigate between the
	GLSLShader and the connected light in both directions.
*/
void GLSLShaderNode::connectLight(int lightIndex, MDagPath lightPath)
{
	if(lightIndex < (int)fLightParameters.size())
	{
		MDGModifier	DG;
		LightParameterInfo& currLight = fLightParameters[lightIndex];

		// Connect the light to the connection placeholder:
		MObject lightShapeNode = lightPath.node();
		MFnDependencyNode dependNode;
		dependNode.setObject(lightShapeNode);
		// Connecting to lightData allows backward navigation:
		MPlug otherPlug = dependNode.findPlug("lightData");
		MPlug paramPlug(thisMObject(),currLight.fAttrConnectedLight);
		MStatus status = DG.connect(otherPlug,paramPlug);
		if(status.statusCode() == MStatus::kSuccess)
		{
			DG.doIt();

			currLight.fIsDirty = true;

			// Lock parameters:
			setLightParameterLocking(currLight, true);

			// Flush implicit cache:
			currLight.fCachedImplicitLight = MObject();

			// Mark the light as being explicitly connected:
			MPlug useImplicitPlug(thisMObject(), currLight.fAttrUseImplicit);
			if( !useImplicitPlug.isNull() ) {
				useImplicitPlug.setValue( false );
			}

			// trigger additional refresh of view to make sure shadow maps are updated
			refreshView();
		}
	}
}

/*
	Helper function to trigger a viewport refresh
	This can be used when we need shadow maps calculated for lights outside the default light list
*/
void GLSLShaderNode::refreshView() const
{
	if (MGlobal::mayaState() != MGlobal::kBatch)
	{
		M3dView view = M3dView::active3dView();
		view.refresh( true /*all views*/, false /*force*/ );
	}
}

/*
	When a shader light is driven either by an explicit light connection or has been bound
	once to a scene light while in "Automatic Bind" mode, we need to make all attributes
	uneditable in the attribute editor.

	This function locks and unlocks light parameters as connection come and go:
*/
void GLSLShaderNode::setLightParameterLocking(const LightParameterInfo& lightInfo, bool locked, bool refreshAE) const
{
	for (LightParameterInfo::TConnectableParameters::const_iterator idxIter=lightInfo.fConnectableParameters.begin();
		idxIter != lightInfo.fConnectableParameters.end();
		++idxIter)
	{
		int parameterIndex((*idxIter).first);
		MUniformParameter param = fUniformParameters.getElement(parameterIndex);

		MPlug uniformPlug(param.getPlug());
		if (!uniformPlug.isNull())
		{
			MFnAttribute uniformAttribute(uniformPlug.attribute());
			if (!uniformAttribute.isHidden()) {
				uniformPlug.setLocked(locked);

				if( refreshAE ) {
					// When the locking is done during the render, the AE is not always properly refreshed
					MGlobal::executeCommandOnIdle(MString("setAttr \"") + uniformPlug.name() + MString("\" -lock ") + (locked ? MString("true") : MString("false")) + MString(";") );
				}
			}
		}
	}
}

void GLSLShaderNode::turnOffLight(const LightParameterInfo& lightInfo) const
{
	static const float kOffColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};

	LightParameterInfo::TConnectableParameters::const_iterator it;
	for (it = lightInfo.fConnectableParameters.begin();
		it != lightInfo.fConnectableParameters.end(); ++it)
	{
		const int parameterIndex = it->first;
		const int parameterType  = it->second;
		switch (parameterType)
		{
		case eLightEnable:
			fUniformParameters.getElement(parameterIndex).setAsBool(false);
			//setParameterAsScalar(parameterIndex, false);
			break;

		case eLightColor:
		case eLightAmbientColor:
		case eLightSpecularColor:
		case eLightDiffuseColor:
			//setParameterAsVector(parameterIndex, (float*)kOffColor);
			fUniformParameters.getElement(parameterIndex).setAsFloatArray((float*)kOffColor,4);
			break;

		case eLightIntensity:
			//setParameterAsScalar(parameterIndex, 0.0f);
			fUniformParameters.getElement(parameterIndex).setAsFloat(0.0f);
			break;

		}
	}
}

/*
	Transfer light parameter values from a draw context light info to all shader parameters
	of the specified light group. Uses the drawContextParameterNames acceleration structure
	to iterate quickly through relevant draw context parameters.
*/
void GLSLShaderNode::connectLight(const LightParameterInfo& lightInfo, MHWRender::MLightParameterInformation* lightParam, ERenderType renderType) const
{
	unsigned int positionCount = 0;
	MFloatPoint position;
	MFloatVector direction;
	float intensity = 1.0f;
	float decayRate = 0.0f;
	MColor color(1.0f, 1.0f, 1.0f);
	bool globalShadowsOn = false;
	bool localShadowsOn = false;
	void *shadowResource = NULL;
	MMatrix shadowViewProj;
	MColor shadowColor;
	float shadowBias = 0.0f;
	MAngle hotspot(40.0, MAngle::kDegrees);
	MAngle falloff(0.0);

	ELightType lightType = getLightType(lightParam);

	// Looping on the uniform parameters reduces the processing time by not
	// enumerating light parameters that are not used by the shader.
	LightParameterInfo::TConnectableParameters::const_iterator it    = lightInfo.fConnectableParameters.begin();
	LightParameterInfo::TConnectableParameters::const_iterator itEnd = lightInfo.fConnectableParameters.end();
	for (; it != itEnd; ++it)
	{
		const int parameterIndex = it->first;
		const int parameterType  = it->second;

		if (parameterType == eLightType) {
			fUniformParameters.getElement(parameterIndex).setAsInt(lightType != eDefaultLight? (int)lightType : eDirectionalLight);
			//setParameterAsScalar(parameterIndex, lightType != eDefaultLight? (int)lightType : eDirectionalLight);
			continue;
		}

		if (parameterType == eLightEnable) {
			fUniformParameters.getElement(parameterIndex).setAsBool(true);
			//setParameterAsScalar(parameterIndex, true);
			continue;
		}

		const MStringArray& params(drawContextParameterNames(lightType, parameterType, lightParam));

		if (params.length() == 0)
			continue;

		for (unsigned int p = 0; p < params.length(); ++p)
		{
			MString pname = params[p];

			MHWRender::MLightParameterInformation::StockParameterSemantic semantic = lightParam->parameterSemantic( pname );

			// Pull off values with position, direction, intensity or color
			// semantics
			//
			MFloatArray floatVals;
			MIntArray intVals;

			switch (semantic)
			{
			case MHWRender::MLightParameterInformation::kWorldPosition:
				lightParam->getParameter( pname, floatVals );
				position += MFloatPoint( floatVals[0], floatVals[1], floatVals[2] );
				++positionCount;
				break;
			case MHWRender::MLightParameterInformation::kWorldDirection:
				lightParam->getParameter( pname, floatVals );
				direction = MFloatVector( floatVals[0], floatVals[1], floatVals[2] );
				break;
			case MHWRender::MLightParameterInformation::kIntensity:
				lightParam->getParameter( pname, floatVals );
				intensity = floatVals[0];
				break;
			case MHWRender::MLightParameterInformation::kDecayRate:
				lightParam->getParameter( pname, floatVals );
				decayRate = floatVals[0];
				break;
			case MHWRender::MLightParameterInformation::kColor:
				lightParam->getParameter( pname, floatVals );
				color[0] = floatVals[0];
				color[1] = floatVals[1];
				color[2] = floatVals[2];
				break;
			// Parameter type extraction for shadow maps
			case MHWRender::MLightParameterInformation::kGlobalShadowOn:
				lightParam->getParameter( pname, intVals );
				if (intVals.length())
					globalShadowsOn = (intVals[0] != 0) ? true : false;
				break;
			case MHWRender::MLightParameterInformation::kShadowOn:
				lightParam->getParameter( pname, intVals );
				if (intVals.length())
					localShadowsOn = (intVals[0] != 0) ? true : false;
				break;
			case MHWRender::MLightParameterInformation::kShadowViewProj:
				lightParam->getParameter( pname, shadowViewProj);
				break;
			case MHWRender::MLightParameterInformation::kShadowMap:
				shadowResource = lightParam->getParameterTextureHandle( pname );
				break;
			case MHWRender::MLightParameterInformation::kShadowColor:
				lightParam->getParameter( pname, floatVals );
				shadowColor[0] = floatVals[0];
				shadowColor[1] = floatVals[1];
				shadowColor[2] = floatVals[2];
				break;
			case MHWRender::MLightParameterInformation::kShadowBias:
				lightParam->getParameter(pname,floatVals);
				shadowBias = floatVals[0];
				break;
			case MHWRender::MLightParameterInformation::kCosConeAngle:
				lightParam->getParameter(pname,floatVals);
				hotspot = MAngle(acos(floatVals[0]), MAngle::kRadians);
				falloff = MAngle(acos(floatVals[1]), MAngle::kRadians);
				break;
			default:
				break;
			}
		}

		// Compute an average position in case we connected an area
		// light to a shader light that cannot handle the 4 corners:
		if (positionCount > 1)
		{
			position[0] /= (float)positionCount;
			position[1] /= (float)positionCount;
			position[2] /= (float)positionCount;
		}

		switch (parameterType)
		{
		case eLightColor:
		case eLightAmbientColor:
		case eLightSpecularColor:
		case eLightDiffuseColor:
			{
				// For swatch and headlamp, we need to tone down the color if it is driving an ambient light:
				if (renderType != RENDER_SCENE && lightInfo.mLightType == eAmbientLight)
				{
					color[0] *= 0.15f;
					color[1] *= 0.15f;
					color[2] *= 0.15f;
				}

				//update color
				fUniformParameters.getElement(parameterIndex).setAsFloatArray((float*)&color[0], 3);
			}
			break;

		case eLightPosition:
		case eLightAreaPosition0:
		case eLightAreaPosition1:
		case eLightAreaPosition2:
		case eLightAreaPosition3:
			fUniformParameters.getElement(parameterIndex).setAsFloatArray((float*)&position[0], 3);
			positionCount = 0;
			position = MFloatPoint();
			break;

		case eLightIntensity:
			fUniformParameters.getElement(parameterIndex).setAsFloat(intensity);
			break;

		case eDecayRate:
			fUniformParameters.getElement(parameterIndex).setAsFloat(decayRate);
			break;

		case eLightDirection:
			fUniformParameters.getElement(parameterIndex).setAsFloatArray((float*)&direction[0], 3);
			break;

		case eLightShadowMapBias:
			fUniformParameters.getElement(parameterIndex).setAsFloat(shadowBias);
			break;

		case eLightShadowColor:
			fUniformParameters.getElement(parameterIndex).setAsFloatArray((float*)&shadowColor[0], 3);
			break;

		case eLightShadowOn:
		{
			// Do an extra check to make sure we have an up-to-date shadow map.
			// If not, disable shadows.
			bool localShadowsDirty = false;
			MIntArray intVals;
			lightParam->getParameter(MHWRender::MLightParameterInformation::kShadowDirty, intVals );
			if (intVals.length())
				localShadowsDirty = (intVals[0] != 0) ? true : false;

			fUniformParameters.getElement(parameterIndex).setAsBool(globalShadowsOn && localShadowsOn && shadowResource && !localShadowsDirty);
			break;
		}
		case eLightShadowViewProj:
			float matrix[4][4];
			shadowViewProj.get(matrix);
			fUniformParameters.getElement(parameterIndex).setAsFloatArray((float*)&matrix[0], 16);
			break;

		case eLightShadowMap:
			//TODO: fix this:
			//setParameterAsResource(parameterIndex, shadowResource);
			break;

		case eLightHotspot:
			fUniformParameters.getElement(parameterIndex).setAsFloat(float(hotspot.asRadians()));
			break;

		case eLightFalloff:
			fUniformParameters.getElement(parameterIndex).setAsFloat(float(falloff.asRadians()));
			break;

		default:
			break;
		}
	}
}

/*
	Explicitely disconnect an explicit light connection:
*/
void GLSLShaderNode::disconnectLight(int lightIndex)
{
	if(lightIndex < (int)fLightParameters.size())
	{
		LightParameterInfo& currLight = fLightParameters[lightIndex];
		currLight.fIsDirty = true;

		// Unlock all light parameters:
		setLightParameterLocking(currLight, false);

		// Flush implicit cache:
		setLightRequiresShadows(currLight.fCachedImplicitLight, false);
		currLight.fCachedImplicitLight = MObject();

		// Disconnect the light from the connection placeholder:
		{
			MFnDependencyNode thisDependNode;
			thisDependNode.setObject(thisMObject());
			MPlug thisLightConnectionPlug = thisDependNode.findPlug(currLight.fAttrConnectedLight, true);
			if (thisLightConnectionPlug.isConnected())
			{
				// Find the light connected as source to this plug:
				MPlugArray srcCnxArray;
				thisLightConnectionPlug.connectedTo(srcCnxArray,true,false);
				if (srcCnxArray.length() > 0)
				{
					MPlug sourcePlug = srcCnxArray[0];
					MDGModifier	DG;
					DG.disconnect(sourcePlug, thisLightConnectionPlug);
					DG.doIt();

					setLightRequiresShadows(sourcePlug.node(), false);

					// trigger additional refresh of view to make sure shadow maps are updated
					refreshView();
				}
			}
		}
	}
}


//
//Helper function to set light requires shadow on/off
//
void GLSLShaderNode::setLightRequiresShadows(const MObject& lightObject, bool requiresShadow) const
{
		if (!lightObject.isNull())
		{
			
			//fprintf(stderr, "Clear implicit light path on disconnect light: %s\n", MFnDagNode( lightObject ).fullPathName().asChar());
			
			MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
			theRenderer->setLightRequiresShadows( lightObject, requiresShadow );
		}
}


// Set the dirty flag on a specific shader light when the user changes
// the light connection settings in order to refresh the shader light
// bindings at the next redraw.
MStatus GLSLShaderNode::setDependentsDirty(const MPlug & plugBeingDirtied, MPlugArray & affectedPlugs)
{
	for(size_t shaderLightIndex = 0; shaderLightIndex < fLightParameters.size(); ++shaderLightIndex )
	{
		LightParameterInfo& shaderLightInfo = fLightParameters[shaderLightIndex];

		MPlug implicitLightPlug(thisMObject(), shaderLightInfo.fAttrUseImplicit);
		if ( implicitLightPlug == plugBeingDirtied ) {
			shaderLightInfo.fIsDirty = true;
		}

		MPlug connectedLightPlug(thisMObject(), shaderLightInfo.fAttrConnectedLight);
		if ( connectedLightPlug == plugBeingDirtied ) {
			shaderLightInfo.fIsDirty = true;
		}
	}

	return MPxHardwareShader::setDependentsDirty(plugBeingDirtied, affectedPlugs);
}

/*
	Populates the set of light parameters that need to be refreshed from the shader parameter
	values in this redraw. This includes all parameters in any light group that was marked as
	being dirty, and can also include parameters from clean groups if the rendering context
	is swatch or default light since the light binding can be overridden.

	Light groups will get dirty in the following scenarios:
		- A notification from a connected light shape was received
		- A scene light was explicitely connected or disconnected
		- Last draw was done in swatch or default scene light context
*/
void GLSLShaderNode::getLightParametersToUpdate(std::set<int>& parametersToUpdate, ERenderType renderType) const
{
	MFnDependencyNode thisDependNode;
	thisDependNode.setObject(thisMObject());

	for(size_t shaderLightIndex = 0; shaderLightIndex < fLightParameters.size(); ++shaderLightIndex )
	{
		const LightParameterInfo& shaderLightInfo = fLightParameters[shaderLightIndex];

		bool needUpdate = (shaderLightInfo.fIsDirty || renderType != RENDER_SCENE);
		if(!needUpdate) {
			MPlug thisLightConnectionPlug = thisDependNode.findPlug(shaderLightInfo.fAttrConnectedLight, true);
			needUpdate = thisLightConnectionPlug.isConnected();
		}
		if (needUpdate)
		{
			LightParameterInfo::TConnectableParameters::const_iterator it = shaderLightInfo.fConnectableParameters.begin();
			LightParameterInfo::TConnectableParameters::const_iterator itEnd = shaderLightInfo.fConnectableParameters.end();
			for (; it != itEnd; ++it)
			{
				parametersToUpdate.insert(it->first);
			}

			if (renderType == RENDER_SCENE)
			{
				// If light is implicit, it stays dirty (as we do not control
				// what happens with the lights and need to react quickly)
				MFnDependencyNode depFn( thisMObject() );
				MPlug useImplicitPlug = depFn.findPlug( shaderLightInfo.fAttrUseImplicit, false );
				if( !useImplicitPlug.isNull() ) {
					bool useImplicit;
					useImplicitPlug.getValue( useImplicit );
					if (!useImplicit)
					{
						// Light will be cleaned. And we are not implicit.
						(const_cast<LightParameterInfo&>(shaderLightInfo)).fIsDirty = false;
					}
				}
			}
		}
	}
}



void GLSLShaderNode::clearLightConnectionData(bool refreshAE)
{
	// Unlock all light parameters.
	for (size_t i = 0; i < fLightParameters.size(); ++i) {
		fLightParameters[i].fCachedImplicitLight = MObject();
		setLightParameterLocking(fLightParameters[i], false, refreshAE);
	}

	fLightNames.setLength(0);
	fLightDescriptions.setLength(0);
}

/*
	Helper function used by the AE via the GLSLShader command to
	know which light is currently driving a light group. For
	explicitly connected lights, we follow the connection to the
	light shape. For implicit lights, we check if we have a cached
	light in the light info structure.
*/
MString GLSLShaderNode::getLightConnectionInfo(int lightIndex)
{
	if(lightIndex < (int)fLightParameters.size())
	{
		LightParameterInfo& currLight = fLightParameters[lightIndex];

		MFnDependencyNode thisDependNode;
		thisDependNode.setObject(thisMObject());
		MPlug thisLightConnectionPlug = thisDependNode.findPlug(currLight.fAttrConnectedLight, true);
		if (thisLightConnectionPlug.isConnected())
		{
			// Find the light connected as source to this plug:
			MPlugArray srcCnxArray;
			thisLightConnectionPlug.connectedTo(srcCnxArray,true,false);
			if (srcCnxArray.length() > 0)
			{
				MPlug sourcePlug = srcCnxArray[0];
				MDagPath sourcePath;
				MDagPath::getAPathTo(sourcePlug.node(), sourcePath);
				MFnDependencyNode sourceTransform;
				sourceTransform.setObject(sourcePath.transform());
				return sourceTransform.name();
			}
		}

		// If light is currently cached, also return it:
		MPlug useImplicitPlug = thisDependNode.findPlug( currLight.fAttrUseImplicit, false );
		if( !useImplicitPlug.isNull() ) {
			bool useImplicit;
			useImplicitPlug.getValue( useImplicit );
			if (useImplicit)
			{
				// Make sure cached light is still in model:
				if (!currLight.fCachedImplicitLight.isNull())
				{
					MStatus status;
					MFnDagNode lightDagNode(currLight.fCachedImplicitLight, &status);
					if (status.statusCode() == MStatus::kSuccess && lightDagNode.inModel() ) {
						MDagPath cachedPath;
						MDagPath::getAPathTo(currLight.fCachedImplicitLight, cachedPath);
						MFnDependencyNode cachedTransform;
						cachedTransform.setObject(cachedPath.transform());
						return cachedTransform.name();
					}
				}
				else if (lightIndex == fImplicitAmbientLight)
					return glslShaderStrings::getString( glslShaderStrings::kAmbient );
			}
		}
	}
	return "";
}

bool GLSLShaderNode::techniqueHandlesContext(MHWRender::MDrawContext& context) const
{
	for (unsigned int passIndex = 0; passIndex < fTechniquePassCount; ++passIndex)
	{
		if( passHandlesContext(context, passIndex) )
			return true;
	}
	return false;
}

bool GLSLShaderNode::passHandlesContext(MHWRender::MDrawContext& context, unsigned int passIndex, const RenderItemDesc* renderItemDesc) const
{
	PassSpecMap::const_iterator it = fTechniquePassSpecs.find(passIndex);
	if (it == fTechniquePassSpecs.end())
		return false;
	const PassSpec& passSpec = it->second;

	const MHWRender::MPassContext & passCtx = context.getPassContext();
	const MStringArray & passSemantics = passCtx.passSemantics();

	bool isHandled = false;
	for (unsigned int passSemIdx = 0; passSemIdx < passSemantics.length() && !isHandled; ++passSemIdx)
	{
		const MString& semantic = passSemantics[passSemIdx];

		// For color passes, only handle if there isn't already
		// a global override. This is the same as the default
		// logic for this method in MPxShaderOverride
		//
		const bool isColorPass = (semantic == MHWRender::MPassContext::kColorPassSemantic);
		if (isColorPass)
		{
			if (!passCtx.hasShaderOverride())
			{
				if(renderItemDesc && renderItemDesc->isOverrideNonMaterialItem)
				{
					isHandled = (STRICMP(passSpec.drawContext.asChar(), glslShaderAnnotation::kNonMaterialItemsPass) == 0);
				}
				else
				{
					isHandled = (passSpec.drawContext.length() == 0) || (STRICMP(semantic.asChar(), passSpec.drawContext.asChar()) == 0);
				}
			}
		}
		else
		{
			isHandled = (STRICMP(semantic.asChar(), passSpec.drawContext.asChar()) == 0);
		}

		if (isHandled && renderItemDesc && renderItemDesc->isOverrideNonMaterialItem)
		{
			if (renderItemDesc->isFatLine)
			{
				if (!passSpec.forFatLine)
				{
					// This pass is not meant for fat line,
					// accept only if there is no pass with the same drawContext which handles fat line
					const PassSpec passSpecTest = { passSpec.drawContext, true, false };
					isHandled = (findMatchingPass(context, passSpecTest) == (unsigned int)-1);
				}
			}
			else if (renderItemDesc->isFatPoint)
			{
				if (!passSpec.forFatPoint)
				{
					// This pass is not meant for fat point,
					// accept only if there is no pass with the same drawContext which handles fat point
					const PassSpec passSpecTest = { passSpec.drawContext, false, true };
					isHandled = (findMatchingPass(context, passSpecTest) == (unsigned int)-1);
				}
			}
			else
			{
				isHandled = (!passSpec.forFatLine && !passSpec.forFatPoint);
			}
		}
	}

	return isHandled;
}

unsigned int GLSLShaderNode::findMatchingPass(MHWRender::MDrawContext& context, const PassSpec& passSpecTest) const
{
	PassSpecMap::const_iterator it = fTechniquePassSpecs.begin();
	PassSpecMap::const_iterator itEnd = fTechniquePassSpecs.end();
	for(; it != itEnd; ++it)
	{
		const PassSpec& passSpec = it->second;
		if( passSpec.forFatLine == passSpecTest.forFatLine &&
			passSpec.forFatPoint == passSpecTest.forFatPoint &&
			STRICMP(passSpec.drawContext.asChar(), passSpecTest.drawContext.asChar()) == 0)
		{
			return it->first;
		}
	}

	return (unsigned int) -1;
}

/*
	Returns the list of all parameters that are members of UI group at given index
*/
MStringArray GLSLShaderNode::getUIGroupParameters(int uiGroupIndex) const
{
	if(uiGroupIndex >= 0 && (unsigned int)uiGroupIndex < fUIGroupParameters.size())
		return fUIGroupParameters[(unsigned int)uiGroupIndex];

	return MStringArray();
}

/*
	Returns the index of the given UI group
*/
int GLSLShaderNode::getIndexForUIGroupName(const MString& uiGroupName) const
{
	return findInArray(const_cast<MStringArray&>(fUIGroupNames), uiGroupName, false /*appendIfNotFound*/);
}

void GLSLShaderNode::deleteUniformUserData()
{
	std::vector<MString*>::iterator it = fUniformUserData.begin();
	std::vector<MString*>::iterator itEnd = fUniformUserData.end();
	for(; it != itEnd; ++it)
		delete *it;
	fUniformUserData.clear();
}

void* GLSLShaderNode::createUniformUserData(const MString& value)
{
	MString* newData = new MString(value);
	fUniformUserData.push_back(newData);
	return (void*)newData;
}

MString GLSLShaderNode::uniformUserDataToMString(void* userData) const
{
	return *((MString*)(userData));
}

