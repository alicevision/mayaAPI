//-
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+

#include <stdio.h>

#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#include <maya/MString.h>

#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>

#include <maya/M3dView.h>
#include <maya/MPoint.h>
#include <maya/MImage.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MRenderTargetManager.h>
#include <maya/MStateManager.h>
#include <maya/MShaderManager.h>
#include <maya/MTextureManager.h>
#include <maya/MDrawContext.h>

#include "viewRenderOverride.h"

/*
	 Utilty to print out lighting information from a draw context
*/
void viewRenderOverrideUtilities::printDrawContextLightInfo( const MHWRender::MDrawContext & drawContext )
{
	// Get all the lighting information in the scene
	MHWRender::MDrawContext::LightFilter considerAllSceneLights = MHWRender::MDrawContext::kFilteredIgnoreLightLimit;
	unsigned int lightCount = drawContext.numberOfActiveLights(considerAllSceneLights);
	if (!lightCount)
		return;

	MFloatPointArray positions;
	MFloatPoint position;
	MFloatVector direction;
	MColor color;
	unsigned int positionCount = 0;
	position[0] = position[1] = position[2] = 0.0f;

	for (unsigned int i=0; i<lightCount; i++)
	{
		MHWRender::MLightParameterInformation *lightParam = drawContext.getLightParameterInformation( i, considerAllSceneLights );
		if (lightParam)
		{
			printf("\tLight %d\n\t{\n", i);

			MStringArray params;
			lightParam->parameterList(params);
			for (unsigned int p=0; p<params.length(); p++)
			{
				MString pname = params[p];
				MHWRender::MLightParameterInformation::ParameterType ptype = lightParam->parameterType( pname );
				MFloatArray floatVals;
				MIntArray intVals;
				MMatrix matrixVal;
				MHWRender::MSamplerStateDesc samplerDesc;
				switch (ptype)
				{
				case MHWRender::MLightParameterInformation::kBoolean:
					lightParam->getParameter( pname, intVals );
					printf("\t\tLight parameter %s. Bool[%d]\n", pname.asChar(),
						intVals[0]);
					break;
				case MHWRender::MLightParameterInformation::kInteger:
					lightParam->getParameter( pname, intVals );
					printf("\t\tLight parameter %s. Integer[%d]\n", pname.asChar(),
						intVals[0]);
					break;
				case MHWRender::MLightParameterInformation::kFloat:
					lightParam->getParameter( pname, floatVals );
					printf("\t\tLight parameter %s. Float[%g]\n", pname.asChar(),
						floatVals[0]);
					break;
				case MHWRender::MLightParameterInformation::kFloat2:
					lightParam->getParameter( pname, floatVals );
					printf("\t\tLight parameter %s. Float[%g,%g]\n", pname.asChar(),
						floatVals[0], floatVals[1]);
					break;
				case MHWRender::MLightParameterInformation::kFloat3:
					lightParam->getParameter( pname, floatVals );
					printf("\t\tLight parameter %s. Float3[%g,%g,%g]\n", pname.asChar(),
						floatVals[0], floatVals[1], floatVals[2]);
					break;
				case MHWRender::MLightParameterInformation::kFloat4:
					lightParam->getParameter( pname, floatVals );
					printf("\t\tLight parameter %s. Float4[%g,%g,%g,%g]\n", pname.asChar(),
						floatVals[0], floatVals[1], floatVals[2], floatVals[3]);
					break;
				case MHWRender::MLightParameterInformation::kFloat4x4Row:
					lightParam->getParameter( pname, matrixVal );
					printf("\t\tLight parameter %s. Float4x4Row [%g,%g,%g,%g]\n\t\t[%g,%g,%g,%g]\n\t\t[%g,%g,%g,%g]\n\t\t[%g,%g,%g,%g]\n",
						pname.asChar(),
						matrixVal[0][0], matrixVal[0][1], matrixVal[0][2], matrixVal[0][3],
						matrixVal[1][0], matrixVal[1][1], matrixVal[1][2], matrixVal[1][3],
						matrixVal[2][0], matrixVal[2][1], matrixVal[2][2], matrixVal[2][3],
						matrixVal[3][0], matrixVal[3][1], matrixVal[3][2], matrixVal[3][3]
					);
					break;
				case MHWRender::MLightParameterInformation::kFloat4x4Col:
					lightParam->getParameter( pname, matrixVal );
					printf("\t\tLight parameter %s. Float4x4Row\n", pname.asChar() );
					break;

				case MHWRender::MLightParameterInformation::kTexture2:
					{
						// Get shadow map as a resource handle directly in OpenGL
						void *handle = lightParam->getParameterTextureHandle( pname );
						printf("\t\tLight texture parameter %s. OpenGL texture id = %d\n", pname.asChar(),
							*((int *)handle));
						// Similar access for DX would look something like this:
						// (ID3D11ShaderResourceView *) lightParam->getParameterTextureHandle( pname );
						break;
					}
				case MHWRender::MLightParameterInformation::kSampler:
					lightParam->getParameter( pname, samplerDesc );
					printf("\t\tLight sampler parameter %s. filter = %d\n", pname.asChar(),
						samplerDesc.filter );
					break;
				default:
					break;
				}

				// Do some discovery to map stock parameters to usable values
				// based on the semantic
				//
				MHWRender::MLightParameterInformation::StockParameterSemantic semantic = lightParam->parameterSemantic( pname );
				switch (semantic)
				{
				case MHWRender::MLightParameterInformation::kLightEnabled:
					printf("\t\t- Parameter semantic : light enabled\n");
					break;
				case MHWRender::MLightParameterInformation::kWorldPosition:
					printf("\t\t- Parameter semantic : world position\n");
					position += MFloatPoint( floatVals[0], floatVals[1], floatVals[2] );
					positionCount++;
					break;
				case MHWRender::MLightParameterInformation::kWorldDirection:
					printf("\t\t- Parameter semantic : world direction\n");
					direction = MFloatVector( floatVals[0], floatVals[1], floatVals[2] );
					break;
				case MHWRender::MLightParameterInformation::kIntensity:
					printf("\t\t- Parameter semantic : intensity\n");
					break;
				case MHWRender::MLightParameterInformation::kColor:
					printf("\t\t- Parameter semantic : color\n");
					color = MColor( floatVals[0], floatVals[1], floatVals[2] );
					break;
				case MHWRender::MLightParameterInformation::kEmitsDiffuse:
					printf("\t\t- Parameter semantic : emits-diffuse\n");
					break;
				case MHWRender::MLightParameterInformation::kEmitsSpecular:
					printf("\t\t- Parameter semantic : emits-specular\n");
					break;
				case MHWRender::MLightParameterInformation::kDecayRate:
					printf("\t\t- Parameter semantic : decay rate\n");
					break;
				case MHWRender::MLightParameterInformation::kDropoff:
					printf("\t\t- Parameter semantic : drop-off\n");
					break;
				case MHWRender::MLightParameterInformation::kCosConeAngle:
					printf("\t\t- Parameter semantic : cosine cone angle\n");
					break;
				case MHWRender::MLightParameterInformation::kShadowMap:
					printf("\t\t- Parameter semantic : shadow map\n");
					break;
				case MHWRender::MLightParameterInformation::kShadowSamp:
					printf("\t\t- Parameter semantic : shadow map sampler\n");
					break;
				case MHWRender::MLightParameterInformation::kShadowBias:
					printf("\t\t- Parameter semantic : shadow map bias\n");
					break;
				case MHWRender::MLightParameterInformation::kShadowMapSize:
					printf("\t\t- Parameter semantic : shadow map size\n");
					break;
				case MHWRender::MLightParameterInformation::kShadowViewProj:
					printf("\t\t- Parameter semantic : shadow map view projection matrix\n");
					break;
				case MHWRender::MLightParameterInformation::kShadowColor:
					printf("\t\t- Parameter semantic : shadow color\n");
					break;
				case MHWRender::MLightParameterInformation::kGlobalShadowOn:
					printf("\t\t- Parameter semantic : global shadows on \n");
					break;
				case MHWRender::MLightParameterInformation::kShadowOn:
					printf("\t\t- Parameter semantic : local shadows on\n");
					break;
				default:
					break;
				}

			}

			// Compute an average position
			if (positionCount > 1)
			{
				position[0] /= (float)positionCount;
				position[1] /= (float)positionCount;
				position[2] /= (float)positionCount;
				printf("\t\tCompute average position [%g,%g,%g]\n", position[0],
					position[1], position[2]);
			}

			// Print by semantic
			printf("\t\tSemantic -> Parameter Name Lookups\n");
			MStringArray paramNames;
			lightParam->parameterNames(MHWRender::MLightParameterInformation::kLightEnabled, paramNames);
			printf("\t\t\tkLightEnabled -> ");
			for (unsigned int i=0; i<paramNames.length(); i++)
			{
				printf("%s ", paramNames[i].asChar());
			}
			MFloatArray floatVals;
			lightParam->getParameter(MHWRender::MLightParameterInformation::kLightEnabled, floatVals);
			if (floatVals.length() > 0) printf("(%f)", floatVals[0]);
			printf("\n");
			lightParam->parameterNames(MHWRender::MLightParameterInformation::kWorldPosition, paramNames);
			printf("\t\t\tkWorldPosition -> ");
			for (unsigned int i=0; i<paramNames.length(); i++)
			{
				printf("%s ", paramNames[i].asChar());
			}
			printf("\n");
			lightParam->parameterNames(MHWRender::MLightParameterInformation::kWorldDirection, paramNames);
			printf("\t\t\tkWorldDirection -> ");
			for (unsigned int i=0; i<paramNames.length(); i++)
			{
				printf("%s ", paramNames[i].asChar());
			}
			printf("\n");
			lightParam->parameterNames(MHWRender::MLightParameterInformation::kIntensity, paramNames);
			printf("\t\t\tkIntensity -> ");
			for (unsigned int i=0; i<paramNames.length(); i++)
			{
				printf("%s ", paramNames[i].asChar());
			}
			printf("\n");
			lightParam->parameterNames(MHWRender::MLightParameterInformation::kColor, paramNames);
			printf("\t\t\tkColor -> ");
			for (unsigned int i=0; i<paramNames.length(); i++)
			{
				printf("%s ", paramNames[i].asChar());
			}
			printf("\n");
			lightParam->parameterNames(MHWRender::MLightParameterInformation::kEmitsDiffuse, paramNames);
			printf("\t\t\tkEmitsDiffuse -> ");
			for (unsigned int i=0; i<paramNames.length(); i++)
			{
				printf("%s ", paramNames[i].asChar());
			}
			printf("\n");
			lightParam->parameterNames(MHWRender::MLightParameterInformation::kEmitsSpecular, paramNames);
			printf("\t\t\tkEmitsSpecular -> ");
			for (unsigned int i=0; i<paramNames.length(); i++)
			{
				printf("%s ", paramNames[i].asChar());
			}
			printf("\n");
			lightParam->parameterNames(MHWRender::MLightParameterInformation::kDecayRate, paramNames);
			printf("\t\t\tkDecayRate -> ");
			for (unsigned int i=0; i<paramNames.length(); i++)
			{
				printf("%s ", paramNames[i].asChar());
			}
			printf("\n");
			lightParam->parameterNames(MHWRender::MLightParameterInformation::kDropoff, paramNames);
			printf("\t\t\tkDropoff -> ");
			for (unsigned int i=0; i<paramNames.length(); i++)
			{
				printf("%s ", paramNames[i].asChar());
			}
			printf("\n");
			lightParam->parameterNames(MHWRender::MLightParameterInformation::kCosConeAngle, paramNames);
			printf("\t\t\tkCosConeAngle -> ");
			for (unsigned int i=0; i<paramNames.length(); i++)
			{
				printf("%s ", paramNames[i].asChar());
			}
			printf("\n");
			lightParam->parameterNames(MHWRender::MLightParameterInformation::kIrradianceIn, paramNames);
			printf("\t\t\tkIrradianceIn -> ");
			for (unsigned int i=0; i<paramNames.length(); i++)
			{
				printf("%s ", paramNames[i].asChar());
			}
			printf("\n");
			lightParam->parameterNames(MHWRender::MLightParameterInformation::kShadowMap, paramNames);
			printf("\t\t\tkShadowMap -> ");
			for (unsigned int i=0; i<paramNames.length(); i++)
			{
				printf("%s ", paramNames[i].asChar());
			}
			printf("\n");
			lightParam->parameterNames(MHWRender::MLightParameterInformation::kShadowSamp, paramNames);
			printf("\t\t\tkShadowSamp -> ");
			for (unsigned int i=0; i<paramNames.length(); i++)
			{
				printf("%s ", paramNames[i].asChar());
			}
			printf("\n");
			lightParam->parameterNames(MHWRender::MLightParameterInformation::kShadowBias, paramNames);
			printf("\t\t\tkShadowBias -> ");
			for (unsigned int i=0; i<paramNames.length(); i++)
			{
				printf("%s ", paramNames[i].asChar());
			}
			printf("\n");
			lightParam->parameterNames(MHWRender::MLightParameterInformation::kShadowMapSize, paramNames);
			printf("\t\t\tkShadowMapSize -> ");
			for (unsigned int i=0; i<paramNames.length(); i++)
			{
				printf("%s ", paramNames[i].asChar());
			}
			printf("\n");
			lightParam->parameterNames(MHWRender::MLightParameterInformation::kShadowColor, paramNames);
			printf("\t\t\tkShadowColor -> ");
			for (unsigned int i=0; i<paramNames.length(); i++)
			{
				printf("%s ", paramNames[i].asChar());
			}
			printf("\n");
			lightParam->parameterNames(MHWRender::MLightParameterInformation::kGlobalShadowOn, paramNames);
			printf("\t\t\tkGlobalShadowOn -> ");
			for (unsigned int i=0; i<paramNames.length(); i++)
			{
				printf("%s ", paramNames[i].asChar());
			}
			printf("\n");
			lightParam->parameterNames(MHWRender::MLightParameterInformation::kShadowOn, paramNames);
			printf("\t\t\tkShadowOn -> ");
			for (unsigned int i=0; i<paramNames.length(); i++)
			{
				printf("%s ", paramNames[i].asChar());
			}
			printf("\n");

			printf("\t}\n");
		}
	}
}

//------------------------------------------------------------------------
// Custom present target operation
//
// There is not much in this operation except to override which targets
// will be presented.
//
// This differs from scene and quad operations which generally
// use targets as the place to render into.
//
viewRenderPresentTarget::viewRenderPresentTarget(const MString &name)
	: MPresentTarget( name )
{
	mTargets = NULL;
}

viewRenderPresentTarget::~viewRenderPresentTarget()
{
	mTargets = NULL;
}

void
viewRenderPresentTarget::setRenderTargets(MHWRender::MRenderTarget **targets)
{
	mTargets = targets;
}

MHWRender::MRenderTarget* const*
viewRenderPresentTarget::targetOverrideList(unsigned int &listSize)
{
	if (mTargets)
	{
		listSize = 2;
		return &mTargets[kMyColorTarget];
	}
	return NULL;
}


//------------------------------------------------------------------------
// Custom HUD operation
//
void viewRenderHUDOperation::addUIDrawables( MHWRender::MUIDrawManager& drawManager2D, const MHWRender::MFrameContext& frameContext )
{
	// Start draw UI
	drawManager2D.beginDrawable();
	// Set font color
	drawManager2D.setColor( MColor( 0.455f, 0.212f, 0.596f ) );
	// Set font size
	drawManager2D.setFontSize( MHWRender::MUIDrawManager::kSmallFontSize );

	// Draw renderer name
	int x=0, y=0, w=0, h=0;
	frameContext.getViewportDimensions( x, y, w, h );
	drawManager2D.text( MPoint(w*0.5f, h*0.91f), MString("Sample VP2 Renderer Override"), MHWRender::MUIDrawManager::kCenter );

	// Draw viewport information
	MString viewportInfoText( "Viewport information: x= " );
	viewportInfoText += x;
	viewportInfoText += ", y= ";
	viewportInfoText += y;
	viewportInfoText += ", w= ";
	viewportInfoText += w;
	viewportInfoText += ", h= ";
	viewportInfoText += h;
	drawManager2D.text( MPoint(w*0.5f, h*0.885f), viewportInfoText, MHWRender::MUIDrawManager::kCenter );

	// End draw UI
	drawManager2D.endDrawable();
}

//------------------------------------------------------------------------
// Custom quad operation
//
// Instances of this class are used to provide different
// shaders to be applied to a full screen quad.
//
viewRenderQuadRender::viewRenderQuadRender(const MString &name)
	: MQuadRender( name )
	, mShaderInstance(NULL)
	, mShader(kEffectNone)
{
	mTargets = NULL;
}

viewRenderQuadRender::~viewRenderQuadRender()
{
	mTargets = NULL;
	if (mShaderInstance)
	{
		MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
		if (renderer)
		{
			const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
			if (shaderMgr)
			{
				shaderMgr->releaseShader(mShaderInstance);
			}
		}
		mShaderInstance = NULL;
	}
}

/*
	Return the appropriate shader instance based on the what
	we want the quad operation to perform
*/
const MHWRender::MShaderInstance *
viewRenderQuadRender::shader()
{
	// Create a new shader instance for this quad render instance
	//
	if (mShaderInstance == NULL)
	{
		MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
		if (renderer)
		{
			const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
			if (shaderMgr)
			{
				// Note in the following code that we are not specifying the
				// full file name, but relying on the getEffectFileShader() logic
				// to determine the correct file name extension based on the shading language
				// which is appropriate for the drawing API (DirectX or OpenGL).
				//
				// Refer to the documentation for this method to review how the
				// final name on disk is derived.
				//
				// The second argument here is the technique. If desired
				// and effect on disk can hold different techniques. For each unique
				// effect + technique a different shader instance is created.
				//
				switch (mShader)
				{
				case kPre_MandelBrot:
					mShaderInstance = shaderMgr->getEffectsFileShader( "MandelBrot", "" );
					break;
				case kPost_EffectMonochrome:
					mShaderInstance = shaderMgr->getEffectsFileShader( "FilterMonochrome", "" );
					break;
				case kPost_EffectEdgeDetect:
					mShaderInstance = shaderMgr->getEffectsFileShader( "FilterEdgeDetect", "" );
					break;
				case kPost_EffectInvert:
					mShaderInstance = shaderMgr->getEffectsFileShader( "Invert", "" );
					break;
				case kScene_Threshold:
					mShaderInstance = shaderMgr->getEffectsFileShader( "Threshold", "" );
					break;
				case kScene_BlurHoriz:
					mShaderInstance = shaderMgr->getEffectsFileShader( "Blur", "BlurHoriz" );
					break;
				case kScene_BlurVert:
					mShaderInstance = shaderMgr->getEffectsFileShader( "Blur", "BlurVert" );
					break;
				case kSceneBlur_Blend:
					mShaderInstance = shaderMgr->getEffectsFileShader( "Blend", "Add" );
					break;
				default:
					break;
				}
			}
		}
	}

	// Set parameters on the shader instance.
	//
	// This is where the input render targets can be specified by binding
	// a render target to the appropriate parameter on the shader instance.
	//
	if (mShaderInstance)
	{
		switch (mShader)
		{
		case kPre_MandelBrot:
			{
				// Example of a simple float parameter setting.
				MStatus status = mShaderInstance->setParameter("gIterate", 50);
				if (status != MStatus::kSuccess)
				{
					printf("Could not change mandelbrot parameter\n");
					return NULL;
				}
			}
			break;

		case kPost_EffectInvert:
			{
				// Set the input texture parameter 'gInputTex' to use
				// a given color target
				MHWRender::MRenderTargetAssignment assignment;
				assignment.target = mTargets[kMyColorTarget];
				MStatus status = mShaderInstance->setParameter("gInputTex", assignment);
				if (status != MStatus::kSuccess)
				{
					printf("Could not set input render target / texture parameter on invert shader\n");
					return NULL;
				}
			}
			break;

		case kScene_Threshold:
			{
				// Set the input texture parameter 'gSourceTex' to use
				// a given color target
				MHWRender::MRenderTargetAssignment assignment;
				assignment.target = mTargets[kMyColorTarget];
				MStatus status = mShaderInstance->setParameter("gSourceTex", assignment);
				if (status != MStatus::kSuccess)
				{
					printf("Could not set input render target / texture parameter on threshold shader\n");
					return NULL;
				}
				mShaderInstance->setParameter("gBrightThreshold", 0.7f );
			}
			break;

		case kScene_BlurHoriz:
			{
				// Set the input texture parameter 'gSourceTex' to use
				// a given color target
				MHWRender::MRenderTargetAssignment assignment;
				assignment.target = mTargets[kMyBlurTarget];
				MStatus status = mShaderInstance->setParameter("gSourceTex", assignment);
				if (status != MStatus::kSuccess)
				{
					printf("Could not set input render target / texture parameter on hblur shader\n");
					return NULL;
				}
			}
			break;

		case kScene_BlurVert:
			{
				// Set the input texture parameter 'gSourceTex' to use
				// a given color target
				MHWRender::MRenderTargetAssignment assignment;
				assignment.target = mTargets[kMyBlurTarget];
				MStatus status = mShaderInstance->setParameter("gSourceTex", assignment);
				if (status != MStatus::kSuccess)
				{
					printf("Could not set input render target / texture parameter on vblur shader\n");
					return NULL;
				}
			}
			break;

		case kSceneBlur_Blend:
			{
				// Set the first input texture parameter 'gSourceTex' to use
				// one color target.
				MHWRender::MRenderTargetAssignment assignment;
				assignment.target = mTargets[kMyColorTarget];
				MStatus status = mShaderInstance->setParameter("gSourceTex", assignment);
				if (status != MStatus::kSuccess)
				{
					return NULL;
				}
				// Set the second input texture parameter 'gSourceTex2' to use
				// a second color target.
				MHWRender::MRenderTargetAssignment assignment2;
				assignment2.target = mTargets[kMyBlurTarget];
				status = mShaderInstance->setParameter("gSourceTex2", assignment2);
				if (status != MStatus::kSuccess)
				{
					return NULL;
				}
				mShaderInstance->setParameter("gBlendSrc", 0.3f );
			}
			break;

		case kPost_EffectMonochrome:
			{
				// Set the input texture parameter 'gInputTex' to use
				// a given color target
				MHWRender::MRenderTargetAssignment assignment;
				assignment.target = mTargets[kMyColorTarget];
				MStatus status = mShaderInstance->setParameter("gInputTex", assignment);
				if (status != MStatus::kSuccess)
				{
					printf("Could not set input render target / texture parameter on monochrome shader\n");
					return NULL;
				}
			}
			break;

		case kPost_EffectEdgeDetect:
			{
				// Set the input texture parameter 'gInputTex' to use
				// a given color target
				MHWRender::MRenderTargetAssignment assignment;
				assignment.target = mTargets[kMyColorTarget];
				MStatus status = mShaderInstance->setParameter("gInputTex", assignment);
				if (status != MStatus::kSuccess)
				{
					printf("Could not set input render target / texture parameter on edge detect shader\n");
					return NULL;
				}
				mShaderInstance->setParameter("gThickness", 1.0f );
				mShaderInstance->setParameter("gThreshold", 0.1f );
			}
			break;

		default:
			break;
		}
	}
	return mShaderInstance;
}

/*
	Based on which shader is being used for the quad render
	we want to render to different targets. For the
	threshold and two blur shaders the temporary 'blur'
	target is used. Otherwise rendering should be directed
	to the custom color and depth target.
*/
MHWRender::MRenderTarget* const*
viewRenderQuadRender::targetOverrideList(unsigned int &listSize)
{
	if (mTargets)
	{
		// Render to blur target for blur operations
		if (mShader == kScene_Threshold ||
			mShader == kScene_BlurHoriz ||
			mShader == kScene_BlurVert )
		{
			listSize = 1; // Only to color target
			return &mTargets[kMyBlurTarget];
		}
		// Render to final otherwise
		else
		{
			listSize = 2; // 2nd target is depth
			return &mTargets[kMyColorTarget];
		}
	}
	return NULL;
}

/*
	Set the clear override to use.
*/
MHWRender::MClearOperation &
viewRenderQuadRender::clearOperation()
{
	// Want to clear everything since the quad render is the first operation.
	if (mShader == kPre_MandelBrot)
	{
		mClearOperation.setClearGradient( false );
		mClearOperation.setMask( (unsigned int) MHWRender::MClearOperation::kClearAll );
	}
	// This is a post processing operation, so we don't want to clear anything
	else
	{
		mClearOperation.setClearGradient( false );
		mClearOperation.setMask( (unsigned int) MHWRender::MClearOperation::kClearNone );
	}
	return mClearOperation;
}

//------------------------------------------------------------------------
//
//  Simple scene operation
//
//	Example of just overriding a few options on the scene render.
//
simpleViewRenderSceneRender::simpleViewRenderSceneRender(const MString &name)
	: MSceneRender( name )
{
	// 100 % of target size
	mViewRectangle[0] = 0.0f;
	mViewRectangle[1] = 0.0f;
	mViewRectangle[2] = 1.0f;
	mViewRectangle[3] = 1.0f;
}

const MFloatPoint *
simpleViewRenderSceneRender::viewportRectangleOverride()
{
	// Enable this flag to use viewport sizing
	bool testRectangleSize = false;
	if (testRectangleSize)
	{
		// 1/3 to the right and 10 % up. 1/2 the target size.
		mViewRectangle[0] = 0.33f;
		mViewRectangle[1] = 0.10f;
		mViewRectangle[2] = 0.50f;
		mViewRectangle[3] = 0.50f;
	}
	return &mViewRectangle;
}

MHWRender::MClearOperation &
simpleViewRenderSceneRender::clearOperation()
{
	// Override to clear to these gradient colors
	float val1[4] = { 0.0f, 0.2f, 0.8f, 1.0f };
	float val2[4] = { 0.5f, 0.4f, 0.1f, 1.0f };
	mClearOperation.setClearColor( val1 );
	mClearOperation.setClearColor2( val2 );
	mClearOperation.setClearGradient( true );
	return mClearOperation;
}


//------------------------------------------------------------------------
// Custom scene operation
//
// Some example things that can be done with the operation are
// included here but disabled. They are here as examples only
// and not all are used for the overrall render loop logic.
//
viewRenderSceneRender::viewRenderSceneRender(
	const MString &name,
	MHWRender::MSceneRender::MSceneFilterOption sceneFilter,
	unsigned int clearMask)
: MSceneRender(name)
, mSceneFilter(sceneFilter)
, mClearMask(clearMask)
, mEnableSRGBWrite(false)
{
	mPrevDisplayStyle = M3dView::kGouraudShaded;

	// 100 % of target size
	mViewRectangle[0] = 0.0f;
	mViewRectangle[1] = 0.0f;
	mViewRectangle[2] = 1.0f;
	mViewRectangle[3] = 1.0f;

	mTargets = NULL;

	mShaderOverride = NULL;

	mUseShaderOverride = false;
	mUseStockShaderOverride = false;
	mAttachPrePostShaderCallback = false;
	mUseShadowShader = false;
	mOverrideDisplayMode = true;
	mOverrideLightingMode = false;
	mOverrideCullingMode = false;
	mOverrrideM3dViewDisplayMode = false;
	mDebugTargetResourceHandle = false;
	mFilterDrawNothing = false;
	mFilterDrawSelected = false;
}

viewRenderSceneRender::~viewRenderSceneRender()
{
	mTargets = NULL;

	if (mShaderOverride)
	{
		MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
		if (renderer)
		{
			const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
			if (shaderMgr)
			{
				shaderMgr->releaseShader(mShaderOverride);
			}
		}
		mShaderOverride = NULL;
	}
}

/*
	Pre UI draw
*/
void viewRenderSceneRender::addPreUIDrawables( MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext& frameContext  )
{
	drawManager.beginDrawable();
	drawManager.setColor( MColor( 0.1f, 0.5f, 0.95f ) );
	drawManager.setFontSize( MHWRender::MUIDrawManager::kSmallFontSize );
	drawManager.text( MPoint( -2, 2, -2 ), MString("Pre UI draw test in Scene operation"), MHWRender::MUIDrawManager::kRight );
	drawManager.line( MPoint( -2, 0, -2 ), MPoint( -2, 2, -2 ) );
	drawManager.setColor( MColor( 1.0f, 1.0f, 1.0f ) );
	drawManager.sphere(  MPoint( -2, 2, -2 ), 0.8, false );
	drawManager.setColor( MColor( 0.1f, 0.5f, 0.95f, 0.4f ) );
	drawManager.sphere(  MPoint( -2, 2, -2 ), 0.8, true );
	drawManager.endDrawable();
}

/*
	Post UI draw
*/
void viewRenderSceneRender::addPostUIDrawables( MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext& frameContext  )
{
	drawManager.beginDrawable();
	drawManager.setColor( MColor( 0.05f, 0.95f, 0.34f ) );
	drawManager.setFontSize( MHWRender::MUIDrawManager::kSmallFontSize );
	drawManager.text( MPoint( 2, 2, 2 ), MString("Post UI draw test in Scene operation"), MHWRender::MUIDrawManager::kLeft );
	drawManager.line( MPoint( 2, 0, 2), MPoint( 2, 2, 2 ) );
	drawManager.setColor( MColor( 1.0f, 1.0f, 1.0f ) );
	drawManager.sphere(  MPoint( 2, 2, 2 ), 0.8, false );
	drawManager.setColor( MColor( 0.05f, 0.95f, 0.34f, 0.4f ) );
	drawManager.sphere(  MPoint( 2, 2, 2 ), 0.8, true );
	drawManager.endDrawable();
}

/*
	Keep a reference of per-frame render targets on the operation
*/
void
viewRenderSceneRender::setRenderTargets(MHWRender::MRenderTarget **targets)
{
	mTargets = targets;
}

/*
	Offscreen target override.

	For this render loop the scene render will always render to
	an offscreen color and depth target (listSize returned = 2).
*/
MHWRender::MRenderTarget* const*
viewRenderSceneRender::targetOverrideList(unsigned int &listSize)
{
	if (mTargets)
	{
		listSize = 2;
		return &mTargets[kMyColorTarget];
	}
	return NULL;
}

/*
	Indicate whether to enable SRGB write
*/
bool viewRenderSceneRender::enableSRGBWrite()
{
	return mEnableSRGBWrite;
}

/*
	Sample of accessing the view to get a camera path and using that as
	the camera override. Other camera paths or direct matrix setting
*/
const MHWRender::MCameraOverride *
viewRenderSceneRender::cameraOverride()
{
	M3dView mView;
	if (mPanelName.length() &&
		(M3dView::getM3dViewFromModelPanel(mPanelName, mView) == MStatus::kSuccess))
	{
		mView.getCamera( mCameraOverride.mCameraPath );
		return &mCameraOverride;
	}

	printf("\t%s : Query custom scene camera override -- no override set\n", mName.asChar());
	return NULL;
}

/*
	Depending on what is required either the scene filter will return whether
	to draw the opaque, transparent of non-shaded (UI) items.
*/
MHWRender::MSceneRender::MSceneFilterOption
viewRenderSceneRender::renderFilterOverride()
{
	return mSceneFilter;
}

/*
	Example display mode override. In this example we override so that
	the scene will always be drawn in "flat shade selected" mode and in bounding box
	mode (bounding boxes will also be drawn). This is fact not a
	'regular' viewport display mode available from the viewport menus.
*/
MHWRender::MSceneRender::MDisplayMode
viewRenderSceneRender::displayModeOverride()
{
	if (mOverrideDisplayMode)
	{
		return (MHWRender::MSceneRender::MDisplayMode)
			( MHWRender::MSceneRender::kBoundingBox |
			MHWRender::MSceneRender::kFlatShaded | 
			MHWRender::MSceneRender::kShadeActiveOnly);
	}
	return MHWRender::MSceneRender::kNoDisplayModeOverride;
}

/*
	Example Lighting mode override. In this example
	the override would set to draw with only selected lights.
*/
MHWRender::MSceneRender::MLightingMode
viewRenderSceneRender::lightModeOverride()
{
	if (mOverrideLightingMode)
	{
		return MHWRender::MSceneRender::kSelectedLights;
	}
	return MHWRender::MSceneRender::kNoLightingModeOverride;
}

/*
	Example culling mode override. When enable
	this example would force to cull backfacing
	polygons.
*/
MHWRender::MSceneRender::MCullingOption
viewRenderSceneRender::cullingOverride()
{
	if (mOverrideCullingMode)
	{
		return MHWRender::MSceneRender::kCullBackFaces;
	}
	return MHWRender::MSceneRender::kNoCullingOverride;
}

/*
	Per scene operation pre-render.

	In this example the display style for the given panel / view
	M3dView is set to be consistent with the draw override
	for the scene operation.
*/
void
viewRenderSceneRender::preRender()
{
	if ( mOverrrideM3dViewDisplayMode )
	{
		M3dView mView;
		if (mPanelName.length() &&
			(M3dView::getM3dViewFromModelPanel(mPanelName, mView) == MStatus::kSuccess))
		{
			mPrevDisplayStyle = mView.displayStyle();
			mView.setDisplayStyle( M3dView::kGouraudShaded );
		}
	}
}

/*
	Post-render example.

	In this example we can debug the resource handle of the active render target
	after this operation. The matching for for the pre-render M3dView override
	also resides here to restore the M3dView state.
*/
void
viewRenderSceneRender::postRender()
{
	if (mDebugTargetResourceHandle)
	{
		// Get the id's for the textures which are used as the color and
		// depth render targets. These id's could arbitrarily change
		// so they should not be held on to.
		void * colorResourceHandle = (mTargets[kMyColorTarget]) ? mTargets[kMyColorTarget]->resourceHandle() : NULL;
		if (colorResourceHandle)
			printf("\t - Color target resource handle = %d\n", *( (int *)colorResourceHandle) );
		void * depthStencilResourceHandle = (mTargets[kMyDepthTarget]) ? mTargets[kMyDepthTarget]->resourceHandle() : NULL;
		if (depthStencilResourceHandle)
			printf("\t - Depth target resource handle = %d\n", *( (int *)depthStencilResourceHandle) );
	}

	// Example of set the display style for the given panel / view
	// via M3dView vs using the scene operation override
	//
	if ( mOverrrideM3dViewDisplayMode )
	{
		M3dView mView;
		if (mPanelName.length() &&
			(M3dView::getM3dViewFromModelPanel(mPanelName, mView) == MStatus::kSuccess))
		{
			// Simple example of restoring display style
			mView.setDisplayStyle( mPrevDisplayStyle );
		}
	}
}

/*
	Object type exclusions example.
	In this example we want to hide cameras and the grid (ground plane)
*/
MHWRender::MSceneRender::MObjectTypeExclusions
viewRenderSceneRender::objectTypeExclusions()
{
	// Example of hiding by type.
	return (MHWRender::MSceneRender::MObjectTypeExclusions )
		( MHWRender::MSceneRender::kExcludeCameras );
}

/*
	Example scene override logic.

	In this example, the scene to draw can be filtered by a returned
	selection list. If an empty selection list is returned then we can
	essentially disable scene drawing. The other option coded here
	is to look at the current active selection list and return that.
	This results in only rendering what has been selected by the user
	when this operation is executed.

	If this filtering is required across more than one operation it
	is better to precompute these values in the setup phase of
	override and cache the information per operation as required.
*/
const MSelectionList *
viewRenderSceneRender::objectSetOverride()
{
	mSelectionList.clear();

	// If you set this to true you can make the
	// scene draw draw no part of the scene, only the
	// additional UI elements
	//
	if (mFilterDrawNothing)
		return &mSelectionList;

	// Turn this on to query the active list and only
	// use that for drawing
	//
	if (mFilterDrawSelected)
	{
		MSelectionList selList;
		MGlobal::getActiveSelectionList( selList );
		if (selList.length())
		{
			MItSelectionList iter( selList );
			for ( ; !iter.isDone(); iter.next() )
			{
				MDagPath item;
				MObject component;
				iter.getDagPath( item, component );
				/* MStatus sStatus = */ mSelectionList.add( item, component );
				/*
				if (sStatus == MStatus::kSuccess)
				{
				printf("Add selection item to other list\n");
				}
				else if (sStatus == MStatus::kInvalidParameter)
				{
				printf("Can't Add invalid selection item to other list\n");
				}
				else
				printf("Can't Add selection item to other list\n");
				*/
			}
		}

		if (mSelectionList.length())
		{
			printf("\t%s : Filtering render with active object list\n",
				mName.asChar());
			return &mSelectionList;
		}
	}
	return NULL;
}

/*
	Custom clear override.

	Depending on whether we are drawing the "UI" or "non-UI"
	parts of the scene we will clear different channels.
	Color is never cleared since there is a separate operation
	to clear the background.
*/
MHWRender::MClearOperation &
viewRenderSceneRender::clearOperation()
{
	if (	mSceneFilter &
		(	MHWRender::MSceneRender::kRenderOpaqueShadedItems | 
			MHWRender::MSceneRender::kRenderTransparentShadedItems |
			MHWRender::MSceneRender::kRenderUIItems) ){

			mClearOperation.setClearGradient(false);
	} else{
		// Force a gradient clear with some sample colors.
		//
		float val1[4] = { 0.0f, 0.2f, 0.8f, 1.0f };
		float val2[4] = { 0.5f, 0.4f, 0.1f, 1.0f };
		mClearOperation.setClearColor(val1);
		mClearOperation.setClearColor2(val2);
		mClearOperation.setClearGradient(true);
	}

	mClearOperation.setMask(mClearMask);

	return mClearOperation;
}

/*
	Return shadow override. For the UI pass we don't want to compute shadows.
*/
const bool* viewRenderSceneRender::shadowEnableOverride()
{
	if ( (mSceneFilter & MHWRender::MSceneRender::kRenderShadedItems) == 0 )
	{
		static const bool noShadowsForUI = false;
		return &noShadowsForUI; // UI doesn't need shadows
	}

	// For all other cases, just use whatever is currently set
	return NULL;
}

// Shader override helpers:
// As part of a shader override it is possible to attach callbacks which
// are invoked when the shader is to be used. The following are some examples
// of what could be performed.
/*
	Example utility used by a callback to:

	1. Print out the shader parameters for a give MShaderInsrtance
	2. Examine the list of render items which will be rendered with this MShaderInstance
	3. Examine the pass context and print out information in the context.
*/
static void callbackDataPrint(
	MHWRender::MDrawContext& context,
	const MHWRender::MRenderItemList& renderItemList,
	MHWRender::MShaderInstance *shaderInstance)
{
	if (shaderInstance)
	{
		MStringArray paramNames;
		shaderInstance->parameterList( paramNames );
		unsigned int paramCount = paramNames.length();
		printf("\tSHADER: # of parameters = %d\n", paramCount );
		for (unsigned int i=0; i<paramCount; i++)
		{
			printf("\t\tPARAM[%s]\n", paramNames[i].asChar());
		}
	}
	int numItems = renderItemList.length();
	const MHWRender::MRenderItem* item = NULL;
	for (int i=0; i<numItems; i++)
	{
		item = renderItemList.itemAt(i);
		if (item)
		{
			MDagPath path = item->sourceDagPath();
			printf("\tRENDER ITEM: '%s' -- SOURCE: '%s'\n", item->name().asChar(), path.fullPathName().asChar());
		}
	}

	const MHWRender::MPassContext & passCtx = context.getPassContext();
	const MString & passId = passCtx.passIdentifier();
	const MStringArray & passSem = passCtx.passSemantics();
	printf("PASS ID[%s], PASS SEMANTICS[", passId.asChar());
	for (unsigned int i=0; i<passSem.length(); i++)
		printf(" %s", passSem[i].asChar());
	printf("\n");
}

/*
	Example utility used by callback to bind lighting information to a shader instance.

	This callback works specific with the MayaBlinnDirectionLightShadow shader example.
	It will explicitly binding lighting and shadowing information to the shader instance.
*/
static void shaderOverrideCallbackBindLightingInfo(
	MHWRender::MDrawContext& drawContext,
	const MHWRender::MRenderItemList& renderItemList,
	MHWRender::MShaderInstance *shaderInstance)
{
	if (!shaderInstance)
		return;

	// Defaults in case there are no lights
	//
	bool globalShadowsOn = false;
	bool localShadowsOn = false;
	MFloatVector direction(0.0f, 0.0f, 1.0f);
	float lightIntensity = 0.0f; // If no lights then black out the light
	float lightColor[3] = { 0.0f, 0.0f, 0.0f };

	MStatus status;

	// Scan to find the first light that has a direction component in it
	// It's possible we find no lights.
	//
	MHWRender::MDrawContext::LightFilter considerAllSceneLights = MHWRender::MDrawContext::kFilteredIgnoreLightLimit;
	unsigned int lightCount = drawContext.numberOfActiveLights(considerAllSceneLights);
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
			globalShadowsOn = false;
			localShadowsOn = false;
			direction = MFloatVector(0.0f, 0.0f, 1.0f);
			lightIntensity = 0.0f;
			lightColor[0] = lightColor[1] = lightColor[2] = 0.0f;

			MHWRender::MLightParameterInformation *lightParam = drawContext.getLightParameterInformation( i, considerAllSceneLights );
			if (lightParam)
			{
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

			// Set shadow map and projection if shadows are turned on.
			//
			if (foundDirectional && globalShadowsOn && localShadowsOn && shadowResource.texture)
			{
				void *resourceHandle = shadowResource.texture->resourceHandle();
				if (resourceHandle)
				{
					bool debugShadowBindings = false;
					status  = shaderInstance->setParameter("mayaShadowPCF1_shadowMap", shadowResource );
					if (status == MStatus::kSuccess && debugShadowBindings)
						printf("Bound shadow map to shader param mayaShadowPCF1_shadowMap\n");
					status  = shaderInstance->setParameter("mayaShadowPCF1_shadowViewProj", shadowViewProj );
					if (status == MStatus::kSuccess && debugShadowBindings)
						printf("Bound shadow map transform to shader param mayaShadowPCF1_shadowViewProj\n");
					status  = shaderInstance->setParameter("mayaShadowPCF1_shadowColor", &shadowColor[0] );
					if (status == MStatus::kSuccess && debugShadowBindings)
						printf("Bound shadow map color to shader param mayaShadowPCF1_shadowColor\n");
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

/*
	Example pre-render callback attached to a shader instance
*/
static void shaderOverridePreDrawCallback(
	MHWRender::MDrawContext& context,
	const MHWRender::MRenderItemList& renderItemList,
	MHWRender::MShaderInstance *shaderInstance)
{
	printf("PRE-draw callback triggered for render item list with data:\n");
	callbackDataPrint(context, renderItemList, shaderInstance);
	printf("\n");

	printf("\tLIGHTS\n");
	viewRenderOverrideUtilities::printDrawContextLightInfo( context );
	printf("\n");
}

/*
	Example post-render callback attached to a shader instance
*/
static void shaderOverridePostDrawCallback(
	MHWRender::MDrawContext& context,
	const MHWRender::MRenderItemList& renderItemList,
	MHWRender::MShaderInstance *shaderInstance)
{
	printf("POST-draw callback triggered for render item list with data:\n");
	callbackDataPrint(context, renderItemList, shaderInstance);
	printf("\n");
}

/*
	Example of setting a shader override.

	Some variations are presented based on some member flags:
	- Use a stock shader or not
	- Attach pre and post shader instance callbacks
	- Use a shadow shader
*/
const MHWRender::MShaderInstance*
viewRenderSceneRender::shaderOverride()
{
	if (mUseShaderOverride)
	{
		if (!mShaderOverride)
		{
			MHWRender::MRenderer *theRenderer = MHWRender::MRenderer::theRenderer();
			const MHWRender::MShaderManager *shaderManager = theRenderer ? theRenderer->getShaderManager() : NULL;
			if (shaderManager)
			{
				if (!mUseStockShaderOverride)
				{
					if (mUseShadowShader)
					{
						// This shader has parameters which can be updated
						// by the attached pre-callback.
						mShaderOverride = shaderManager->getEffectsFileShader(
							"MayaBlinnDirectionalLightShadow", "", 0, 0, true,
							shaderOverrideCallbackBindLightingInfo,
							NULL);
					}
					else
					{
						// Use a sample Gooch shader
						mShaderOverride = shaderManager->getEffectsFileShader(
							"Gooch", "", 0, 0, true,
							mAttachPrePostShaderCallback ? shaderOverridePreDrawCallback : NULL,
							mAttachPrePostShaderCallback ? shaderOverridePostDrawCallback : NULL);
					}
				}
				else
				{
					// Use a stock shader available from the shader manager
					// In this case the stock Blinn shader.
					mShaderOverride = shaderManager->getStockShader(
						MHWRender::MShaderManager::k3dBlinnShader,
						mAttachPrePostShaderCallback ? shaderOverridePreDrawCallback : NULL,
						mAttachPrePostShaderCallback ? shaderOverridePostDrawCallback : NULL);
					if (mShaderOverride)
					{
						printf("\t%s : Set stock shader override %d\n", mName.asChar(), MHWRender::MShaderManager::k3dBlinnShader );
						static const float diffColor[] = {0.0f, 0.4f, 1.0f, 1.0f};
						MStatus status = mShaderOverride->setParameter("diffuseColor", diffColor);
						if (status != MStatus::kSuccess)
						{
							printf("Could not set diffuseColor on shader\n");
						}
					}
				}
			}
		}
		return mShaderOverride;
	}
	// No override so return NULL
	return NULL;
}





