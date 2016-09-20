//-
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+

#include <stdio.h>
#include <stdlib.h>			// for getenv

#include <maya/MString.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MRenderTargetManager.h>
#include <maya/MTextureManager.h>
#include <maya/MDrawContext.h>
#include <maya/MAnimControl.h>
#include "viewRenderOverrideFrameCache.h"

// Raw reading, and dump to disk
#include <maya/MHardwareRenderer.h>
#include <maya/MGLFunctionTable.h>
#include <maya/MImage.h>

viewRenderOverrideFrameCache::viewRenderOverrideFrameCache(const MString& name)
: MRenderOverride(name)
, mUIName("VP2 Frame Caching Override")
, mAllowCaching(false)
, mCacheToDisk(false)
, mPerformCapture(false)
, mCachedTexture(NULL)
, mCurrentTime(0)
, mSubFrameSamples(10.0) // Allow room for up to 10 samples between integer frames
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

viewRenderOverrideFrameCache::~viewRenderOverrideFrameCache()
{
	// Release any store textures
	releaseCachedTextures();

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

void viewRenderOverrideFrameCache::releaseCachedTextures()
{
	printf("viewRenderOverrideFrameCache : Reset frame cache.\n");

	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (!renderer) return;
	MHWRender::MTextureManager* textureManager = renderer->getTextureManager();
	if (!textureManager) return;

	for (std::map<unsigned int,MHWRender::MTexture*>::iterator it=mCachedTargets.begin(); it!=mCachedTargets.end(); ++it)
	{
		MHWRender::MTexture *texture = it->second;
		if (NULL != texture)
		{
			textureManager->releaseTexture(texture);
			it->second = NULL;
		}
	}
	mCachedTargets.clear();
}

MHWRender::DrawAPI viewRenderOverrideFrameCache::supportedDrawAPIs() const
{
	return (MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile);
}

bool viewRenderOverrideFrameCache::startOperationIterator()
{
	mCurrentOperation = 0;
	return true;
}

MHWRender::MRenderOperation* viewRenderOverrideFrameCache::renderOperation()
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

bool viewRenderOverrideFrameCache::nextRenderOperation()
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
//	A. For caching:
//		1. Render scene to new target
//		2. Cache the target at the given time.
//		3. Blit on-screen
//	B. For playing cache:
//		1. Find target at time
//		2. If found blit. Otherwise cache.
//
// Operations before the preview can be enabled / disable to change
// what is shown by the preview operation.
//
MStatus viewRenderOverrideFrameCache::updateRenderOperations()
{
	bool initOperations = true;
	for (unsigned int i=0; i<kOperationCount; i++)
	{
		if (mRenderOperations[i])
			initOperations = false;
	}

	if (initOperations)
	{
		// 1. These ops are for capture
		// Regular scene render
		mRenderOperationNames[kMaya3dSceneRender] = "_viewRenderOverrideFrameCache_SceneRender";
		SceneRenderOperation * sceneOp = new SceneRenderOperation (mRenderOperationNames[kMaya3dSceneRender], this);
		mRenderOperations[kMaya3dSceneRender] = sceneOp;
		mRenderOperationEnabled[kMaya3dSceneRender] = false;

		// User operation capture a target
		mRenderOperationNames[kTargetCapture] = "_viewRenderOverrideFrameCache_TargetCapture";
		CaptureTargetsOperation* captureOp = new CaptureTargetsOperation(mRenderOperationNames[kTargetCapture], this);
		mRenderOperations[kTargetCapture] = captureOp;
		mRenderOperationEnabled[kTargetCapture] = false;

		// 2. There ops are for playback
		// A quad blit to preview a target
		mRenderOperationNames[kTargetPreview] = "_viewRenderOverrideFrameCache_TargetPreview";
		PreviewTargetsOperation * previewOp = new PreviewTargetsOperation (mRenderOperationNames[kTargetPreview], this);
		mRenderOperations[kTargetPreview] = previewOp;
		mRenderOperationEnabled[kTargetPreview] = false;

		// Generic screen blit - always want to do this
		mRenderOperationNames[kPresentOp] = "_viewRenderOverrideFrameCache_PresentTarget";
		mRenderOperations[kPresentOp] = new presentTargets(mRenderOperationNames[kPresentOp]);
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
	if (haveOperations != MStatus::kSuccess)
		return haveOperations;

	// Get the current time. See if we have a frame cached. If not then
	// enable operations to perform a "capture".
	// Otherwise retrieve the previous texture and perform a "preview"
	//
	mCachedTexture = NULL;

	// Store a unsigned int value = mSubFrameSamples * MTime double value
	mCurrentTime = (unsigned int)(mSubFrameSamples * MAnimControl::currentTime().value());
	std::map<unsigned int,MHWRender::MTexture*>::iterator iter = mCachedTargets.find( mCurrentTime );

	// Check to see if we have a cached frame
	if (iter != mCachedTargets.end())
	{
		mCachedTexture = iter->second;
	}

	mPerformCapture = (NULL == mCachedTexture) && mAllowCaching;

	CaptureTargetsOperation* captureOp = (CaptureTargetsOperation *)mRenderOperations[kTargetCapture];
	captureOp->setTexture(NULL);
	captureOp->setCurrentTime( mCurrentTime );
	captureOp->setDumpImageToDisk( mCacheToDisk );

	// Capture a new image
	if (mPerformCapture)
	{
		printf("viewRenderOverrideFrameCache : Mode = capturing texture at time %g\n", (double)mCurrentTime / mSubFrameSamples );
		mRenderOperationEnabled[kMaya3dSceneRender] = true;
		mRenderOperationEnabled[kTargetCapture] = true;
		mRenderOperationEnabled[kTargetPreview] = false;
	}
	else
	{
		// Blit from cached image
		if (mCachedTexture)
		{
			printf("viewRenderOverrideFrameCache : Mode = preview cached texture at time %g\n", (double)mCurrentTime / mSubFrameSamples );
			mRenderOperationEnabled[kMaya3dSceneRender] = false;
			mRenderOperationEnabled[kTargetCapture] = false;
			mRenderOperationEnabled[kTargetPreview] = true;
		}
		// Just a regular scene render only
		else
		{
			printf("viewRenderOverrideFrameCache : Caching disabled and no frame to draw. Use regular refresh at time %g\n", (double)mCurrentTime / mSubFrameSamples );
			mRenderOperationEnabled[kMaya3dSceneRender] = true;
			mRenderOperationEnabled[kTargetCapture] = false;
			mRenderOperationEnabled[kTargetPreview] = false;
		}
	}

	return haveOperations;
}

//
// Update all shaders used for rendering
//
MStatus viewRenderOverrideFrameCache::updateShaders(const MHWRender::MShaderManager* shaderMgr)
{
	// Set up a preview target shader (Targets as input)
	//
	MHWRender::MShaderInstance *shaderInstance = mShaderInstances[kTargetPreviewShader];
	if (!shaderInstance)
	{
		shaderInstance = shaderMgr->getEffectsFileShader( "Copy", "" );
		mShaderInstances[kTargetPreviewShader] = shaderInstance;

		// Set constant parameters
		if (shaderInstance)
		{
			// We want to make sure to reblit back alpha as well as RGB
			shaderInstance->setParameter("gDisableAlpha", false );
			shaderInstance->setParameter("gVerticalFlip", false );
		}
	}
	// Make sure to update the texture to use
 	MHWRender::MTextureAssignment texAssignment;
 	texAssignment.texture = mCachedTexture;
	shaderInstance->setParameter("gInputTex", texAssignment );

	// Update shader and texture on quad operation
	PreviewTargetsOperation * quadOp = (PreviewTargetsOperation * )mRenderOperations[kTargetPreview];
	if (quadOp)
	{
		quadOp->setShader( mShaderInstances[kTargetPreviewShader] );
	}

	if (quadOp && shaderInstance)
		return MStatus::kSuccess;
	return MStatus::kFailure;
}

//
// Update override for the current frame
//
MStatus viewRenderOverrideFrameCache::setup(const MString& destination)
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

MStatus viewRenderOverrideFrameCache::cleanup()
{
	// If we captured a new target then put it into the target cache
	CaptureTargetsOperation* captureOp = (CaptureTargetsOperation* ) mRenderOperations[kTargetCapture];
	if (captureOp)
	{
		MHWRender::MTexture* texture = captureOp->getTexture();
		if (texture)
		{
			printf("viewRenderOverrideFrameCache : Cache a new texture at time %g\n", (double)mCurrentTime / mSubFrameSamples);
			std::pair<std::map<unsigned int,MHWRender::MTexture*>::iterator,bool> val;
			val = mCachedTargets.insert( std::pair<unsigned int,MHWRender::MTexture*>(mCurrentTime, texture) );
			if (val.second == false)
			{
				printf("viewRenderOverrideFrameCache : Failed to insert texture into cache. Already have element in cache !\n");
			}
			captureOp->setTexture( NULL );
		}
	}

	mCurrentOperation = -1;
	mPerformCapture = true;
	mCachedTexture = NULL;

	return MStatus::kSuccess;
}

///////////////////////////////////////////////////////////////////

SceneRenderOperation ::SceneRenderOperation (const MString& name, viewRenderOverrideFrameCache *theOverride)
: MSceneRender(name)
, mOverride(theOverride)
{
}

SceneRenderOperation ::~SceneRenderOperation ()
{
	mOverride = NULL;
}

///////////////////////////////////////////////////////////////////

presentTargets ::presentTargets (const MString& name)
: MPresentTarget(name)
{
}

presentTargets ::~presentTargets ()
{
}

///////////////////////////////////////////////////////////////////

PreviewTargetsOperation::PreviewTargetsOperation(const MString &name, viewRenderOverrideFrameCache *theOverride)
	: MQuadRender( name )
	, mShaderInstance(NULL)
	, mTexture(NULL)
	, mOverride(theOverride)
{
}

PreviewTargetsOperation::~PreviewTargetsOperation()
{
	mShaderInstance = NULL;
	mTexture = NULL;
	mOverride = NULL;
}

const MHWRender::MShaderInstance* PreviewTargetsOperation::shader()
{
	return mShaderInstance;
}

MHWRender::MClearOperation &
PreviewTargetsOperation::clearOperation()
{
	mClearOperation.setMask( (unsigned int) MHWRender::MClearOperation::kClearAll );
	return mClearOperation;
}

///////////////////////////////////////////////////////////////////

CaptureTargetsOperation::CaptureTargetsOperation(const MString &name, viewRenderOverrideFrameCache *override)
: MHWRender::MUserRenderOperation(name)
, mOverride(override)
, mTexture(NULL)
, mDumpImageToDisk(false)
{
}

CaptureTargetsOperation::~CaptureTargetsOperation()
{
	mOverride = NULL;
	mTexture = NULL;
}

MStatus CaptureTargetsOperation::execute( const MHWRender::MDrawContext & drawContext )
{
	// Get a copy of the current target
	mTexture = drawContext.copyCurrentColorRenderTargetToTexture();
	if (mTexture)
	{
		// File name to dump cached frames to disk with
		const char *tmpDir = getenv("TMPDIR");
		if (mDumpImageToDisk && tmpDir)
		{
			MString outputName;
			outputName = tmpDir;
			outputName += "/viewCachedImage.";
			outputName += ((float)mCurrentTime / 10.0f);
			outputName += ".exr";

			MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
			MHWRender::MTextureManager* textureMgr = renderer ? renderer->getTextureManager() : NULL;
			if (textureMgr)
			{
				MStatus status = textureMgr->saveTexture( mTexture, outputName );
				printf("viewRenderOverrideFrameCache : Saved copied cached image to disk (%s) = %d\n", outputName.asChar(), (status == MStatus::kSuccess));
			}
		}
	}

	return MStatus::kSuccess;
}

///////////////////////////////////////////////////////////////////
