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
#include <maya/MRenderTargetManager.h>
#include <maya/MTextureManager.h>
#include <maya/MDrawContext.h>

#include "viewRenderOverrideTargets.h"


viewRenderOverrideTargets::viewRenderOverrideTargets(const MString& name)
: MRenderOverride(name)
, mUIName("VP2 Targets Copy Target Test")
{
	unsigned int i = 0;
	for (i=0; i<kOperationCount; i++)
	{
		mRenderOperations[i] = NULL;
	}
	mCurrentOperation = -1;

	for (i=0; i<kShaderCount; i++)
	{
		mShaderInstances[i] = NULL;
	}
}

viewRenderOverrideTargets::~viewRenderOverrideTargets()
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
		for (unsigned int i=0; i<kShaderCount; i++)
		{
			if (mShaderInstances[i])
			{
				if (shaderMgr)
					shaderMgr->releaseShader(mShaderInstances[i]);
				mShaderInstances[i] = NULL;
			}
		}
	}
}

MHWRender::DrawAPI viewRenderOverrideTargets::supportedDrawAPIs() const
{
	return (MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile);
}

bool viewRenderOverrideTargets::startOperationIterator()
{
	mCurrentOperation = 0;
	return true;
}

MHWRender::MRenderOperation* viewRenderOverrideTargets::renderOperation()
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

bool viewRenderOverrideTargets::nextRenderOperation()
{
	mCurrentOperation++;
	if (mCurrentOperation < kOperationCount)
	{
		return true;
	}
	return false;
}

//
// Update list of operations to perform:
//
// 1. Clear 1 color target, 1 depth target. 
// 2. Render the scene 
// 4. Preview the colour + depth targets as sub-regions in a 3rd target
// 5. Present 3rd target
//
// Operations before the preview can be enabled / disable to change
// what is shown by the preview operation.
//
MStatus viewRenderOverrideTargets::updateRenderOperations()
{
	bool initOperations = true;
	for (unsigned int i=0; i<kOperationCount; i++)
	{
		if (mRenderOperations[i])
			initOperations = false;
	}

	if (initOperations)
	{
		mRenderOperationNames[kMaya3dSceneRender] = "_viewRenderOverrideTargets_SceneRenderTargets";
		sceneRenderTargets * sceneOp = new sceneRenderTargets (mRenderOperationNames[kMaya3dSceneRender], this);
		mRenderOperations[kMaya3dSceneRender] = sceneOp;
		mRenderOperationEnabled[kMaya3dSceneRender] = true;

		mRenderOperationNames[kTargetPreview] = "_viewRenderOverrideTargets_TargetPreview";
		quadRenderTargets * quadOp2 = new quadRenderTargets (mRenderOperationNames[kTargetPreview], this);
		mRenderOperations[kTargetPreview] = quadOp2;
		mRenderOperationEnabled[kTargetPreview] = true;

		mRenderOperationNames[kPresentOp] = "_viewRenderOverrideTargets_PresentTargetTargets";
		mRenderOperations[kPresentOp] = new presentTargetTargets(mRenderOperationNames[kPresentOp]);
		mRenderOperationEnabled[kPresentOp] = true;
	}
	mCurrentOperation = -1;

	MStatus haveOperations = MStatus::kFailure;
	for (unsigned int i=0; i<kOperationCount; i++)
	{
		if (mRenderOperations[i])
			haveOperations = MStatus::kSuccess;
	}
	return haveOperations;
}

//
// Update all shaders used for rendering
//
MStatus viewRenderOverrideTargets::updateShaders(const MHWRender::MShaderManager* shaderMgr)
{
	// Set up a preview target shader (Targets as input)
	//
	MHWRender::MShaderInstance *shaderInstance = mShaderInstances[kTargetPreviewShader];
	if (!shaderInstance)
	{
		shaderInstance = shaderMgr->getEffectsFileShader( "FreeView", "" );
		mShaderInstances[kTargetPreviewShader] = shaderInstance;

		// Set constant parmaeters
		if (shaderInstance)
		{
			const float borderClr[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			const float backGroundClr[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
			shaderInstance->setParameter("gBorderColor", borderClr );
			shaderInstance->setParameter("gBackgroundColor", backGroundClr );
		}
	}
	// Update shader's per frame parameters
	if (shaderInstance)
	{
		unsigned int targetWidth = 0;
		unsigned int targetHeight = 0;
		MHWRender::MRenderer *theRenderer = MHWRender::MRenderer::theRenderer();
		if( theRenderer )
			theRenderer->outputTargetSize( targetWidth, targetHeight );

		float vpSize[2] = { (float)targetWidth,  (float)targetHeight };
		shaderInstance->setParameter("gViewportSizePixels", vpSize );

		float sourceSize[2] = { (float)targetWidth,  (float)targetHeight };
		shaderInstance->setParameter("gSourceSizePixels", sourceSize );

		/// Could use 0.0125 * width / 2
		shaderInstance->setParameter("gBorderSizePixels", 0.00625f * targetWidth );

		
	}
	// Update shader on quad operation
	quadRenderTargets * quadOp = (quadRenderTargets * )mRenderOperations[kTargetPreview];
	if (quadOp)
		quadOp->setShader( mShaderInstances[kTargetPreviewShader] );

	if (quadOp && shaderInstance)
		return MStatus::kSuccess;
	return MStatus::kFailure;
}

//
// Update override for the current frame
//
MStatus viewRenderOverrideTargets::setup(const MString& destination)
{
	// Firewall checks
	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (!renderer) return MStatus::kFailure;

	const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
	if (!shaderMgr) return MStatus::kFailure;

	const MHWRender::MRenderTargetManager *targetManager = renderer->getRenderTargetManager();
	if (!targetManager) return MStatus::kFailure;

	// Update render operations
	MStatus status = updateRenderOperations();
	if (status != MStatus::kSuccess)
		return status;

	// Update shaders
	status = updateShaders( shaderMgr );

	return status;
}

MStatus viewRenderOverrideTargets::cleanup()
{
	mCurrentOperation = -1;

	// Clear targets on quad render
	quadRenderTargets *quadOp = (quadRenderTargets *)mRenderOperations[kTargetPreview];
	if (quadOp)
		quadOp->updateTargets( NULL, NULL );

	// Release the targets
	sceneRenderTargets *sceneOp = (sceneRenderTargets *) mRenderOperations[kMaya3dSceneRender];
	if (sceneOp)
		sceneOp->releaseTargets();

	return MStatus::kSuccess;	
}

///////////////////////////////////////////////////////////////////

sceneRenderTargets ::sceneRenderTargets (const MString& name, viewRenderOverrideTargets *theOverride)
: MSceneRender(name)
, mOverride(theOverride)
{
	float val[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	mClearOperation.setClearColor(val);
}

sceneRenderTargets ::~sceneRenderTargets ()
{
	mOverride = NULL;
}

/* virtual */
MHWRender::MClearOperation &
sceneRenderTargets::clearOperation()
{
	mClearOperation.setMask( (unsigned int)( MHWRender::MClearOperation::kClearAll ) );
	return mClearOperation;
}


/* virtual */
// We only care about the opaque objects
MHWRender::MSceneRender::MSceneFilterOption sceneRenderTargets::renderFilterOverride()
{
	return MHWRender::MSceneRender::kRenderOpaqueShadedItems;
}

/* virtual */
void sceneRenderTargets::postSceneRender(const MHWRender::MDrawContext & context)
{
	const MHWRender::MPassContext & passCtx = context.getPassContext();
	const MStringArray & passSem = passCtx.passSemantics();
	bool inColorPass = false;
	bool inDisallowedPass = false;
	for (unsigned int i=0; i<passSem.length(); i++)
	{
		if (passSem[i] ==  MHWRender::MPassContext::kColorPassSemantic)
			inColorPass = true;
		else if ((passSem[i] == MHWRender::MPassContext::kShadowPassSemantic) ||
				 (passSem[i] == MHWRender::MPassContext::kDepthPassSemantic) ||
				 (passSem[i] == MHWRender::MPassContext::kNormalDepthPassSemantic))
		{
			inDisallowedPass = true;
		}
	}
	if (!inColorPass || inDisallowedPass)
	{
		return;
	}

	MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
	if (!theRenderer)
		return;

	// Make a copy of the current color and depth target
	mTempColourTarget = context.copyCurrentColorRenderTargetToTexture();
	mTempDepthTarget = context.copyCurrentDepthRenderTargetToTexture();

	// Set the targets on quad render
	quadRenderTargets *quadOp = NULL;
	if (mOverride)
	{
		quadOp = (quadRenderTargets *)mOverride->getOperation( viewRenderOverrideTargets::kTargetPreview );
	}
	if (quadOp)
		quadOp->updateTargets( mTempColourTarget, mTempDepthTarget );
}

void sceneRenderTargets::releaseTargets()
{
	// Release temporary target copies
	MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
	if (!theRenderer)
		return;
	MHWRender::MTextureManager* textureManager = theRenderer->getTextureManager();
	if (!textureManager)
		return;

	if (mTempColourTarget)
	{
		textureManager->releaseTexture( mTempColourTarget );
		mTempColourTarget = NULL;
	}
	if (mTempDepthTarget)
	{
		textureManager->releaseTexture( mTempDepthTarget );
		mTempDepthTarget = NULL;
	}
}


///////////////////////////////////////////////////////////////////

presentTargetTargets ::presentTargetTargets (const MString& name)
: MPresentTarget(name)
{
}

presentTargetTargets ::~presentTargetTargets ()
{
}

///////////////////////////////////////////////////////////////////

void quadRenderTargets::updateTargets( MHWRender::MTexture *colorTarget, MHWRender::MTexture *depthTarget )
{
	if (!mShaderInstance)
		return;

	// Bind two input target parameters
	//
	MHWRender::MTextureAssignment texAssignment;
 	texAssignment.texture = (MHWRender::MTexture*) colorTarget;
	mShaderInstance->setParameter("gSourceTex", texAssignment);

 	MHWRender::MTextureAssignment texAssignment2;
 	texAssignment2.texture = (MHWRender::MTexture*) depthTarget;
	mShaderInstance->setParameter("gSourceTex2", texAssignment2);
}

quadRenderTargets::quadRenderTargets(const MString &name, viewRenderOverrideTargets *theOverride)
	: MQuadRender( name )
	, mShaderInstance(NULL)
	, mOverride(theOverride)
{
}

quadRenderTargets::~quadRenderTargets()
{
	mShaderInstance = NULL;
	mOverride = NULL;
}

const MHWRender::MShaderInstance *
quadRenderTargets::shader()
{
	return mShaderInstance;
}

MHWRender::MClearOperation &
quadRenderTargets::clearOperation()
{
	mClearOperation.setMask( (unsigned int) MHWRender::MClearOperation::kClearAll );
	return mClearOperation;
}

///////////////////////////////////////////////////////////////////



