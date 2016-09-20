//-
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+

#include <stdio.h>

#include <maya/MString.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MDrawContext.h>
#include <maya/MTextureManager.h>
#include <maya/MStateManager.h>
#include <maya/MGlobal.h>
#include <maya/MDagPath.h>

#include "viewRenderOverrideShadows.h"

///////////////////////////////////////////////////////////////////////////////
// 
// Description:
//
// This is an example plugin which will override the viewport 2.0 rendering
// for the purposes of showing how shadow maps may be requested "on-demand" 
// and used for selective lighting (and shadowing) in a scene render.
//
// The basic logic is:
//
// 1. Perform an operation to queue requests for shadow map computation for specific lights. The
//		sample logic will either send requests for all lights, or only lights on the selection list,
//		if the list is not empty.
// 2. Perform an operation to invoke shadow map update and color pass rendering. The color pass
//		uses the shadow maps computed during the operation execution.
//		For simplicitly a single override shader is used for the entire scene. This shader will
//		update shadow maps (bind textures) as apprpropriate just before the the color pass in invoked.
//
///////////////////////////////////////////////////////////////////////////////
/*
	Constructor for override
*/
viewRenderOverrideShadows::viewRenderOverrideShadows(const MString& name)
: MRenderOverride(name)
, mUIName("Sample VP2 Shadow Requester")
{
	unsigned int i = 0;
	for (i=0; i<kOperationCount; i++)
	{
		mRenderOperations[i] = NULL;
	}
	mCurrentOperation = -1;

	mLightShader = NULL;
}

/*
	Desctructor

	Make sure to release the operations and any shaders acquired
	via the shader manager
*/
viewRenderOverrideShadows::~viewRenderOverrideShadows()
{
	for (unsigned int i=0; i<kOperationCount; i++)
	{
		delete mRenderOperations[i];
		mRenderOperations[i] = NULL;
	}

    MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
	if (theRenderer)
	{
		// Release shaders
		const MHWRender::MShaderManager* shaderMgr = theRenderer->getShaderManager();
		if (shaderMgr && mLightShader)
		{
			shaderMgr->releaseShader(mLightShader);
			mLightShader = NULL;
		}
	}
}

/*
	Can draw in DX11 and OpenGL
*/
MHWRender::DrawAPI viewRenderOverrideShadows::supportedDrawAPIs() const
{
	return (MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile);
}

//
// Basic operation iterators
//
bool viewRenderOverrideShadows::startOperationIterator()
{
	mCurrentOperation = 0;
	return true;
}

MHWRender::MRenderOperation* viewRenderOverrideShadows::renderOperation()
{
	if (mCurrentOperation >= 0 && mCurrentOperation < kOperationCount)
	{
		// Skip empty and disabled operations
		//
		while(!mRenderOperations[mCurrentOperation] || !mRenderOperationEnabled[mCurrentOperation])
		{
			mCurrentOperation++;
			if (mCurrentOperation >= kOperationCount)
			{
				return NULL;
			}
		}

		if (mRenderOperations[mCurrentOperation])
		{
			return mRenderOperations[mCurrentOperation];
		}
	}
	return NULL;
}

bool viewRenderOverrideShadows::nextRenderOperation()
{
	mCurrentOperation++;
	if (mCurrentOperation < kOperationCount)
	{
		return true;
	}
	return false;
}

//
// Update list of operations to perform
//
MStatus viewRenderOverrideShadows::updateRenderOperations()
{
	bool initOperations = true;
	for (unsigned int i=0; i<kOperationCount; i++)
	{
		if (mRenderOperations[i])
		{
			initOperations = false;
			break;
		}
	}

	// We want 3 basic operations:
	// 1. A prepass which will scan the available lights, and queue selected
	//		ones as requiring up-to-date shadow maps. A custom user operation is used.
	// 2. A basic scene render which will extract out the shaow maps requested in
	//		step 1. and bind as appropirate to a scene level shader override (MShaderInstance).
	// 3. A basic "present" operation to display to screen.
	//
	if (initOperations)
	{
		mRenderOperationNames[kShadowPrePass] = "_viewRenderOverrideShadows_ShadowPrepass";
		shadowPrepass * shadowOp = new shadowPrepass (mRenderOperationNames[kShadowPrePass]);
		mRenderOperations[kShadowPrePass] = shadowOp;
		mRenderOperationEnabled[kShadowPrePass] = true;

		mRenderOperationNames[kMaya3dSceneRender] = "_viewRenderOverrideShadows_SceneRender";
		sceneRender * sceneOp = new sceneRender (mRenderOperationNames[kMaya3dSceneRender]);
		mRenderOperations[kMaya3dSceneRender] = sceneOp;
		mRenderOperationEnabled[kMaya3dSceneRender] = true;

		mRenderOperationNames[kPresentOp] = "_viewRenderOverrideShadows_PresentTarget";
		mRenderOperations[kPresentOp] = new MHWRender::MPresentTarget(mRenderOperationNames[kPresentOp]);
		mRenderOperationEnabled[kPresentOp] = true;
	}
	mCurrentOperation = -1;

	MStatus haveOperations = MStatus::kSuccess;
	for (unsigned int i=0; i<kOperationCount; i++)
	{
		if (!mRenderOperations[i])
		{
			haveOperations = MStatus::kFailure;	
			break;
		}
	}
	return haveOperations;
}

//
// Method to add in a light "prune" list. Only selected
// lights will be have their shadows requested, and
// be used for the scene render shader override.
//
MStatus viewRenderOverrideShadows::updateLightList()
{
	mLightList.clear();

	shadowPrepass* shadowOp = (shadowPrepass*)mRenderOperations[kShadowPrePass];
	sceneRender* sceneOp = (sceneRender*)mRenderOperations[kMaya3dSceneRender];
	if (!shadowOp || !sceneOp)
		return MStatus::kFailure;

	// Scan selection list for active lights
	//
	MSelectionList selectList;
	MDagPath dagPath;
	MObject component;
	MGlobal::getActiveSelectionList( selectList );
	for (unsigned int i=0; i<selectList.length(); i++)
	{
		selectList.getDagPath( i, dagPath, component );
		dagPath.extendToShape();
		if ( dagPath.hasFn( MFn::kLight ) )
		{
			mLightList.add( dagPath );
		}
	}
	
	// Set light list to prune which lights to request shadows for
	//
	if (mLightList.length())
		shadowOp->setLightList( &mLightList );
	else
		shadowOp->setLightList( NULL );

	// Set light list to prune which lights to bind for scene shader
	//
	if (mLightList.length())
		sceneOp->setLightList( &mLightList );
	else
		sceneOp->setLightList( NULL );

	return MStatus::kSuccess;
}

/*
	Utility method to update shader parameters based on available
	lighting information.
*/
static void updateLightShader( MHWRender::MShaderInstance *shaderInstance,
						const MHWRender::MDrawContext & context,
						const MSelectionList * lightList )
{
	if (!shaderInstance)
		return;

	// Check pass context information to see if we are in a shadow 
	// map update pass. If so do nothing.
	//
	const MHWRender::MPassContext & passCtx = context.getPassContext();
	const MStringArray & passSem = passCtx.passSemantics();
	bool handlePass = true;
	for (unsigned int i=0; i<passSem.length() && handlePass; i++)
	{
		// Handle special pass drawing.
		//
		if (passSem[i] == MHWRender::MPassContext::kShadowPassSemantic)
		{
			handlePass = false;
		}
	}
	if (!handlePass) return;

	//
	// Perform light shader update with lighting information
	// If the light list is not empty then use that light's information.
	// Otherwise choose the first appropriate light which can cast shadows.
	//

	// Defaults in case there are no lights
	//
	bool globalShadowsOn = false;
	bool localShadowsOn = false;
	bool shadowDirty = false;
	MFloatVector direction(0.0f, 0.0f, 1.0f);
	float lightIntensity = 0.0f; // If no lights then black out the light
	float lightColor[3] = { 0.0f, 0.0f, 0.0f };

	MStatus status;

	// Scan to find the first N lights that has a direction component in it
	// It's possible we find no lights.
	//
	MHWRender::MDrawContext::LightFilter considerAllSceneLights = MHWRender::MDrawContext::kFilteredIgnoreLightLimit;
	unsigned int lightCount = context.numberOfActiveLights(considerAllSceneLights);
	if (lightCount)
	{
		MFloatArray floatVals;
		MIntArray intVals;
		MHWRender::MTextureAssignment shadowResource;
		shadowResource.texture = NULL;
		MHWRender::MSamplerStateDesc samplerDesc;
		MMatrix shadowViewProj;
		float shadowColor[3] = { 0.0f, 0.0f, 0.0f };

		unsigned int i=0;
		bool foundDirectional = false;
		for (i=0; i<lightCount && !foundDirectional ; i++)
		{
			MHWRender::MLightParameterInformation *lightParam = context.getLightParameterInformation( i, considerAllSceneLights );
			if (lightParam)
			{
				// Prune against light list if any.
				if (lightList && lightList->length())
				{
					if (!lightList->hasItem(lightParam->lightPath()))
						continue;
				}

				MStringArray params;
				lightParam->parameterList(params);
				for (unsigned int p=0; p<params.length(); p++)
				{
					MString pname = params[p];

					MHWRender::MLightParameterInformation::StockParameterSemantic semantic = lightParam->parameterSemantic( pname );
					switch (semantic)
					{
						// Pick a few light parameters to pick up as an example
					case MHWRender::MLightParameterInformation::kWorldDirection:
						lightParam->getParameter( pname, floatVals );
						direction = MFloatVector( floatVals[0], floatVals[1], floatVals[2] );
						foundDirectional = true;
						break;
					case MHWRender::MLightParameterInformation::kIntensity:
						lightParam->getParameter( pname, floatVals );
						lightIntensity = floatVals[0];
						break;
					case MHWRender::MLightParameterInformation::kColor:
						lightParam->getParameter( pname, floatVals );
						lightColor[0] = floatVals[0];
						lightColor[1] = floatVals[1];
						lightColor[2] = floatVals[2];
						break;

						// Pick up shadowing parameters
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
						lightParam->getParameter( pname, shadowResource );
						break;
					case MHWRender::MLightParameterInformation::kShadowDirty:
						if (intVals.length())
							shadowDirty = (intVals[0] != 0) ? true : false;
						break;
					case MHWRender::MLightParameterInformation::kShadowSamp:
						lightParam->getParameter( pname, samplerDesc );
						break;
					case MHWRender::MLightParameterInformation::kShadowColor:
						lightParam->getParameter( pname, floatVals );
						shadowColor[0] = floatVals[0];
						shadowColor[1] = floatVals[1];
						shadowColor[2] = floatVals[2];
						break;
					default:
						break;
					}
				} /* for params */
			}

			if (foundDirectional && globalShadowsOn && localShadowsOn && shadowResource.texture)
			{
				void *resourceHandle = shadowResource.texture->resourceHandle();
				if (resourceHandle)
				{
					static bool debugShadowBindings = false;
					status  = shaderInstance->setParameter("mayaShadowPCF1_shadowMap", shadowResource );
					if (status == MStatus::kSuccess && debugShadowBindings)
						fprintf(stderr, "Bound shadow map to shader param mayaShadowPCF1_shadowMap\n");
					status  = shaderInstance->setParameter("mayaShadowPCF1_shadowViewProj", shadowViewProj );
					if (status == MStatus::kSuccess && debugShadowBindings)
						fprintf(stderr, "Bound shadow map transform to shader param mayaShadowPCF1_shadowViewProj\n");
					status  = shaderInstance->setParameter("mayaShadowPCF1_shadowColor", &shadowColor[0] );
					if (status == MStatus::kSuccess && debugShadowBindings)
						fprintf(stderr, "Bound shadow map color to shader param mayaShadowPCF1_shadowColor\n");
				}

				MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
				if (renderer)
				{
					MHWRender::MTextureManager* textureManager = renderer->getTextureManager();
					if (textureManager)
					{
						textureManager->releaseTexture(shadowResource.texture);
					}
				}
				shadowResource.texture = NULL;
			}
		}
	}

	// Set up parameters which should be set regardless of light existence.
	status = shaderInstance->setParameter("mayaDirectionalLight_direction", &( direction[0] ));
	status = shaderInstance->setParameter("mayaDirectionalLight_intensity", lightIntensity );
	status = shaderInstance->setParameter("mayaDirectionalLight_color", &( lightColor[0] ));
	status = shaderInstance->setParameter("mayaShadowPCF1_mayaGlobalShadowOn", globalShadowsOn);
	status = shaderInstance->setParameter("mayaShadowPCF1_mayaShadowOn", localShadowsOn);
}

//
// Update the light shader override used for rendering the scene with.
//
MStatus viewRenderOverrideShadows::updateShaders(const MHWRender::MShaderManager* shaderMgr)
{
	// Create a light shader and assign it to the scene render.
	// We don't update the parameters (shadow maps)
	// here since they may not be available or up-to-date at this point.
	//
	const MString shaderName("MayaBlinnDirectionalLightShadow");
	const MString techniqueName("");
	if (!mLightShader)
	{
		mLightShader = shaderMgr->getEffectsFileShader( shaderName, techniqueName );
		if (!mLightShader)
		{
			return MStatus::kFailure;
		}
		static const float blinnColor[4] = { 0.85f, 1.0f, 0.7f, 1.0f };
		mLightShader->setParameter("blinn1color", blinnColor); 
	}	

	sceneRender *sceneOp = (sceneRender *)mRenderOperations[kMaya3dSceneRender];
	if (sceneOp && mLightShader)
	{
		sceneOp->setShader( mLightShader );
		return MStatus::kSuccess;
	}
	return MStatus::kFailure;
}

//
// Update override for the current frame.
//
// Make sure we have a proper set of operations. If so then update
// shaders and light pruning information.
//
MStatus viewRenderOverrideShadows::setup(const MString& destination)
{
	// Firewall checks
	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (!renderer) return MStatus::kFailure;

	const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
	if (!shaderMgr) return MStatus::kFailure;

	// Update render operations
	//
	MStatus status = updateRenderOperations();
	if (status != MStatus::kSuccess)
		return status;

	// Update shaders
	status = updateShaders( shaderMgr );
	if (status != MStatus::kSuccess)
		return status;

	// Update light list
	status = updateLightList();

	return status;
}

/*
	End of frame cleanup
*/
MStatus viewRenderOverrideShadows::cleanup()
{
	mCurrentOperation = -1;

	// Reset light list used for pruning at end of each invocation. 
	// Also clear memobers on scene and shadow pre-pass operations
	mLightList.clear();

	shadowPrepass * shadowOp = (shadowPrepass *)mRenderOperations[kShadowPrePass];
	if (shadowOp)
		shadowOp->setLightList( NULL ); 	

	sceneRender *sceneOp = (sceneRender *)mRenderOperations[kMaya3dSceneRender];
	if (sceneOp)
	{
		// Clear the light shader
		sceneOp->setShader( NULL );
		sceneOp->setLightList( NULL ); 	
	}
	return MStatus::kSuccess;
}

///////////////////////////////////////////////////////////////////

sceneRender::sceneRender (const MString& name)
: MSceneRender(name)
{
	mLightShader = NULL;
}

sceneRender::~sceneRender ()
{
	mLightShader = NULL;
}

/* virtual */
const MHWRender::MShaderInstance* sceneRender::shaderOverride()
{
	return mLightShader;
}

/*
	After shadows and lighting have been updated we need to
	update this information on the override shader used to 
	render the scene, before it is rendered.

	We don't want to perform any updates during shadow map
	update so we exit early if this condition is detected.
*/
void sceneRender::preSceneRender(const MHWRender::MDrawContext & context)
{
	updateLightShader( mLightShader, context, mLightList );
}

///////////////////////////////////////////////////////////////////

shadowPrepass::shadowPrepass(const MString &name)
: MUserRenderOperation( name )
{
}

shadowPrepass::~shadowPrepass()
{
}

/*
	From the draw context, get the list of lights and queue the ones we are interested in
	into the "desired list"
*/
MStatus shadowPrepass::execute( const MHWRender::MDrawContext & context )
{
	MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
	if (!theRenderer)
		return MStatus::kSuccess;

	// Skip lighting modes where there are no lights which can
	// cast shadows
	MHWRender::MDrawContext::LightingMode lightingMode = context.getLightingMode();
	if (lightingMode != MHWRender::MDrawContext::kSelectedLights &&
		lightingMode != MHWRender::MDrawContext::kSceneLights)
	{
		return MStatus::kSuccess;
	}

	MHWRender::MDrawContext::LightFilter lightFilter = 
		MHWRender::MDrawContext::kFilteredIgnoreLightLimit;
	unsigned int nbSceneLights = context.numberOfActiveLights(lightFilter);

	for (unsigned int i=0; i<nbSceneLights; i++)
	{
		MHWRender::MLightParameterInformation* lightInfo = 
			context.getLightParameterInformation( i, lightFilter );
		if (!lightInfo)
			continue;

		// Get the actually Maya light node
		MStatus status = MStatus::kFailure;
		MDagPath lightPath = lightInfo->lightPath(&status);
		if (status != MStatus::kSuccess || !lightPath.isValid())
			continue;

		// Would be good to have an API method here to indicate if it
		// casts shadows
		MIntArray intVals;

		// Check if light is enabled, and emits any lighting
		MHWRender::MLightParameterInformation::StockParameterSemantic semantic =
			MHWRender::MLightParameterInformation::kLightEnabled;
		if (MStatus::kSuccess == lightInfo->getParameter( semantic, intVals ))
		{
			if (intVals.length())
			{
				if (intVals[0] == 0)
					continue;
			}
		}

		semantic = MHWRender::MLightParameterInformation::kEmitsDiffuse;
		if (MStatus::kSuccess == lightInfo->getParameter( semantic, intVals ))
		{
			if (intVals.length())
			{
				if (intVals[0] == 0)
					continue;
			}
		}

		semantic = MHWRender::MLightParameterInformation::kEmitsSpecular;
		if (MStatus::kSuccess == lightInfo->getParameter( semantic, intVals ))
		{
			if (intVals.length())
			{
				if (intVals[0] == 0)
					continue;
			}
		}

		// Check if local shadows are enabled.
		semantic = MHWRender::MLightParameterInformation::kShadowOn;
		if (MStatus::kSuccess == lightInfo->getParameter( semantic, intVals ))
		{
			if (intVals.length())
			{
				if (intVals[0] == 0)
					continue;
			}
		}

		// Check if the shadow is "dirty"
		bool shadowIsDirty = false;
		semantic = MHWRender::MLightParameterInformation::kShadowDirty;
		if (MStatus::kSuccess == lightInfo->getParameter( semantic, intVals ))
		{
			if (intVals.length())
			{
				if (intVals[0] == 0)
				//	continue;

				shadowIsDirty = intVals[0] != 0 ? true : false ;
			}
		}

		// Check the light list to prune, if not already pruned
		bool prune = false;
		if (lightingMode != MHWRender::MDrawContext::kSelectedLights)
		{
			if (mLightList && mLightList->length())
			{
				prune = !mLightList->hasItem(lightInfo->lightPath());
			}
		}

		static bool debugShadowRequests = false;
		// Set that we require shadows for this light
		if (!prune)
		{
			if (debugShadowRequests)
				fprintf(stderr, "QUEUE light shadows for %s, shadowDirty = %d\n", 
					lightPath.fullPathName().asChar(), shadowIsDirty);

			theRenderer->setLightRequiresShadows( lightPath.node(), true );
		}
		
		// Set that we DON'T require shadows for this light
		else
		{
			if (debugShadowRequests)
				fprintf(stderr, "DEQUEUE light shadows for %s, shadowDirty = %d\n", 
					lightPath.fullPathName().asChar(), shadowIsDirty);
			theRenderer->setLightRequiresShadows( lightPath.node(), false );
		}
	}

	return MStatus::kSuccess;
}



