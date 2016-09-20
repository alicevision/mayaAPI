//-
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+

#ifndef GLSLSHADER_GLSLSHADER_H
#define GLSLSHADER_GLSLSHADER_H

#include <maya/MHWGeometry.h>
#include <maya/MMessage.h>
#include <maya/MPxHardwareShader.h>
#include <maya/MUniformParameter.h>
#include <maya/MUniformParameterList.h>
#include <maya/MVaryingParameter.h>
#include <maya/MVaryingParameterList.h>
#include <maya/MShaderManager.h>
#include <maya/MStringArray.h>
#include <maya/MDrawContext.h>
#include <vector>
#include <map>
#include <set>

class GLSLShaderNode : public MPxHardwareShader
{
public:
	enum ERenderType
	{
		RENDER_SCENE,				// Render the scene to the viewport 2.0
		RENDER_SWATCH,				// Render the swatch that represents the current selected technique
		RENDER_SWATCH_PROXY,		// Render a dummy swatch when no effect or no valid technique selected
		RENDER_UVTEXTURE,			// Render a texture for the UV editor
		RENDER_SCENE_DEFAULT_LIGHT	// Render the scene using a default light
	};

	struct RenderItemDesc
	{
		bool isOverrideNonMaterialItem;
		bool isFatLine;
		bool isFatPoint;
	};

public:
	GLSLShaderNode();
	~GLSLShaderNode();

	static void* creator();
	static MStatus initialize();
	static void initializeNodeAttrs();
	virtual MStatus render(MGeometryList& iterator);

	virtual bool getInternalValueInContext( const MPlug&,MDataHandle&,MDGContext&);
	virtual bool setInternalValueInContext( const MPlug&,const MDataHandle&,MDGContext&);

	virtual const MRenderProfile & profile();
	const MString& effectName() const;

	void clearParameters();
	void updateParameters(const MHWRender::MDrawContext& context, ERenderType renderType = RENDER_SCENE) const;
	void updateOverrideNonMaterialItemParameters(const MHWRender::MDrawContext& context, const MHWRender::MRenderItem* item, RenderItemDesc& renderItemDesc) const;

	MHWRender::MShaderInstance* GetGLSLShaderInstance() const { return fGLSLShaderInstance; }
	const MHWRender::MVertexBufferDescriptorList& geometryRequirements() const { return fGeometryRequirements; }

	bool hasUpdatedVaryingInput() const;
	void updateGeometryRequirements();

	MTypeId	typeId() const;
	static MTypeId TypeID();

	static const MTypeId m_TypeId;
	static const MString m_TypeName;
	static const MString m_RegistrantId;
	static const MString m_drawDbClassification;

	MHWRender::MTexture* loadTexture(const MString& textureName, const MString& layerName, int alphaChannelIdx, int mipmapLevels) const;

	virtual MStatus renderSwatchImage( MImage & image );

	bool reload();

	MString techniqueName() const { return fTechniqueName; }
	MStringArray techniqueNames() const { return fTechniqueNames; }

	bool techniqueIsSelectable() const { return fTechniqueIsSelectable; }
	bool techniqueIsTransparent() const { return fTechniqueIsTransparent; }
	bool techniqueSupportsAdvancedTransparency() const { return fTechniqueSupportsAdvancedTransparency; }
	bool techniqueOverridesDrawState() const { return fTechniqueOverridesDrawState; }
	double techniqueBBoxExtraScale() const { return fTechniqueBBoxExtraScale; }
	bool techniqueOverridesNonMaterialItems() const { return fTechniqueOverridesNonMaterialItems; }

	const MString& techniqueIndexBufferType() const { return fTechniqueIndexBufferType; }

	MUniformParameter::DataSemantic convertSpace(const MString& parameterName, MUniformParameter::DataSemantic defaultSpace);
	MUniformParameter::DataSemantic guessUnknownSemantics(const MString& parameterName);

	//Lighting

	enum ELightType
	{
		eInvalidLight,
		eUndefinedLight,
		eSpotLight,
		ePointLight,	
		eDirectionalLight,
		eAmbientLight,
		eVolumeLight,
		eAreaLight,
		eDefaultLight,

		eLightCount
	};

	enum ELightParameterType
	{
		eUndefined, // 0
		eLightPosition,
		eLightDirection,
		eLightColor,
		eLightSpecularColor,
		eLightAmbientColor, // 5
		eLightDiffuseColor,
		eLightRange,
		eLightFalloff,
		eLightAttenuation0,
		eLightAttenuation1, // 10
		eLightAttenuation2,
		eLightTheta,
		eLightPhi,
		eLightShadowMap,
		eLightShadowMapBias, // 15
		eLightShadowColor,
		eLightShadowViewProj,
		eLightShadowOn,
		eLightIntensity,
		eLightHotspot, // 20
		eLightEnable,
		eLightType,
		eDecayRate,
		eLightAreaPosition0,
		eLightAreaPosition1, // 25
		eLightAreaPosition2,
		eLightAreaPosition3,

		// When updating this array, please keep the 
		// strings in getLightParameterSemantic in sync. 
		//    Thanks!
		eLastParameterType,
	};

	class LightParameterInfo 
	{
	public:
		LightParameterInfo(
			int lightIndex,
			ELightType lightType = GLSLShaderNode::eInvalidLight,
			bool hasLightTypeSemantics = false)
		: mLightIndex(lightIndex)
		, mLightType(lightType)
		, fIsDirty(true)
		, fHasLightTypeSemantics(hasLightTypeSemantics)
		{

		}

		int mLightIndex;
		ELightType mLightType;
		bool fIsDirty;
		bool fHasLightTypeSemantics;

		// This is a map<MUniformParameterList->index, ELightParameterType>
		typedef std::map<int, int> TConnectableParameters;
		TConnectableParameters fConnectableParameters;

		MObject		fAttrUseImplicit;
		MObject		fAttrConnectedLight;
		MObject		fCachedImplicitLight;
	};

	MStringArray getLightableParameters(int lightIndex, bool showSemantics);
	MString& getLightParameterSemantic(int lightParameterType);
	void refreshLightConnectionAttributes(bool inSceneUpdateNotification = false);

	void updateLightInfoFromSemantic(const MString& parameterName, int uniformParamIndex);
	int getIndexForLightName(const MString& lightName, bool appendLight = false);

	const MStringArray& lightInfoDescription() const { return fLightDescriptions; }
	void clearLightConnectionData(bool refreshAE=true);
	MString getLightConnectionInfo(int lightIndex);
	void connectLight(int lightIndex, MDagPath lightPath);

	bool techniqueHandlesContext(MHWRender::MDrawContext& context) const;
	bool passHandlesContext(MHWRender::MDrawContext& context, unsigned int passIndex, const RenderItemDesc* renderItemDesc = NULL) const;

private:
	void configureUniforms();
	void configureGeometryRequirements();

	void updateImplicitLightConnections(const MHWRender::MDrawContext& context, ERenderType renderType) const;
	void updateExplicitLightConnections(const MHWRender::MDrawContext& context, ERenderType renderType) const;
	void connectLight(const LightParameterInfo& lightInfo, MHWRender::MLightParameterInformation* lightParam, ERenderType renderType = RENDER_SCENE) const;
	bool connectExplicitAmbientLight(const LightParameterInfo& lightInfo, const MObject& sourceLight) const;
	void turnOffLight(const LightParameterInfo& lightInfo) const;
	void setLightRequiresShadows(const MObject& lightObject, bool requiresShadow) const;
	void updateImplicitLightParameterCache();

	void refreshView() const;
	void setLightParameterLocking(const LightParameterInfo& lightInfo, bool locked, bool refreshAE=true) const;
	void getLightParametersToUpdate(std::set<int>& parametersToUpdate, ERenderType renderType) const;

	bool appendParameterNameIfVisible(int paramIndex, MStringArray& paramArray) const;

public:
	void disconnectLight(int lightIndex);

	virtual MStatus setDependentsDirty(const MPlug& plugBeingDirtied, MPlugArray& affectedPlugs);

	/////////////////////////////////
	// Attibute Editor
	const MStringArray& getUIGroups() const { return fUIGroupNames; }
	MStringArray getUIGroupParameters(int uiGroupIndex) const;
	int getIndexForUIGroupName(const MString& uiGroupName) const;

private:

	bool loadEffect( const MString& effectName );
	void displayErrorAndWarnings() const;
	void configureUniformUI(const MString& parameterName, MUniformParameter& uniformParam) const;

	/////////////////////////////////
	// External content management	
	virtual	void getExternalContent(MExternalContentInfoTable& table) const;
	virtual	void setExternalContent(const MExternalContentLocationTable& table);

private:
	void deleteUniformUserData();
	void* createUniformUserData(const MString& value);
	MString uniformUserDataToMString(void* userData) const;

private:
	struct PassSpec
	{
		MString drawContext;
		bool forFatLine;
		bool forFatPoint;
	};
	unsigned int findMatchingPass(MHWRender::MDrawContext& context, const PassSpec& passSpecTest) const;

private:

	bool							fEffectLoaded;

	MString							fEffectName;

	MUniformParameterList			fUniformParameters;
	std::vector<MString*>			fUniformUserData;

	MHWRender::MVertexBufferDescriptorList	fGeometryRequirements;
	MVaryingParameterList			fVaryingParameters;
	unsigned int					fVaryingParametersUpdateId;

	MHWRender::MShaderInstance *	fGLSLShaderInstance;

	//Diagnostics strings
	mutable MString					fErrorLog;
	mutable MString					fWarningLog;
	mutable unsigned int			fErrorCount;


	// Active technique name
	MString							fTechniqueName;
	MStringArray                    fTechniqueNames;

	MObject							fTechniqueEnumAttr;
	int								fTechniqueIdx;

	bool							fTechniqueIsSelectable;
	bool							fTechniqueIsTransparent;
	bool							fTechniqueSupportsAdvancedTransparency;
	MString							fTechniqueIndexBufferType;
	bool							fTechniqueOverridesDrawState;
	int								fTechniqueTextureMipmapLevels;
	double							fTechniqueBBoxExtraScale;
	bool							fTechniqueOverridesNonMaterialItems;

	unsigned int					fTechniquePassCount;
	typedef std::map<unsigned int, PassSpec> PassSpecMap;
	PassSpecMap						fTechniquePassSpecs;

	//Lighting
	typedef std::vector<LightParameterInfo> LightParameterInfoVec;
	LightParameterInfoVec			fLightParameters;
	MStringArray					fLightNames;
	MStringArray					fLightDescriptions;
	mutable int						fImplicitAmbientLight; 

	// Identifier to track scene-render-frame in order to optimize the updateParameter routine.
	mutable MUint64					fLastFrameStamp;

	///////////// Attibute Editor
	MStringArray					fUIGroupNames;
	std::vector<MStringArray>		fUIGroupParameters;
};

#endif
