#include "hwRendererHelper.h"
#include "hwApiTextureTestStrings.h"

#include <maya/MViewport2Renderer.h>
#include <maya/MTextureManager.h>
#include <maya/MRenderTargetManager.h>
#include <maya/MGlobal.h>

#include "hwRendererHelperGL.h"

#ifdef _WIN32
#include "hwRendererHelperDX.h"
#endif

hwRendererHelper::hwRendererHelper(MHWRender::MRenderer* renderer)
: fRenderer(renderer)
{
}

hwRendererHelper::~hwRendererHelper()
{
}

/*static*/
hwRendererHelper* hwRendererHelper::create(MHWRender::MRenderer* renderer)
{
	hwRendererHelper* helper = NULL;

	if(renderer->drawAPIIsOpenGL())
	{
		helper = new hwRendererHelperGL(renderer);
	}
#ifdef _WIN32
	else
	{
		ID3D11Device* dxDevice = (ID3D11Device*) renderer->GPUDeviceHandle();
		helper = new hwRendererHelperDX(renderer, dxDevice);
	}
#endif

	return helper;
}

MHWRender::MTexture* hwRendererHelper::createTextureFromScreen()
{
	const MHWRender::MRenderTargetManager* targetManager = fRenderer->getRenderTargetManager();
	if(targetManager == NULL)
	{
		MGlobal::displayWarning( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kErrorTargetManager ) );
		return NULL;
	}

	MHWRender::MTextureManager* textureManager = fRenderer->getTextureManager();
	if(textureManager == NULL)
	{
		MGlobal::displayWarning( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kErrorTextureManager ) );
		return NULL;
	}

	// Acquire a render target initialized with current on-screen target (size, format and data)
	MHWRender::MRenderTarget* onScreenRenderTarget = targetManager->acquireRenderTargetFromScreen( MString("hwApiTextureTest") );
	if(onScreenRenderTarget == NULL)
	{
		MGlobal::displayWarning( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kErrorSaveGrabArg ) );
		return NULL;
	}

	MHWRender::MRenderTargetDescription targetDesc;
	onScreenRenderTarget->targetDescription(targetDesc);

	// Get the render target data (the screen pixels)
	int rowPitch, slicePitch;
	char* targetData = (char*)onScreenRenderTarget->rawData(rowPitch, slicePitch);

	targetManager->releaseRenderTarget(onScreenRenderTarget);

	// Create a texture with same size and format as the render target
	MHWRender::MTexture* texture = NULL;
	if(targetData != NULL)
	{
		MHWRender::MTextureDescription textureDesc;
		textureDesc.fWidth = targetDesc.width();
		textureDesc.fHeight = targetDesc.height();
		textureDesc.fDepth = 1;
		textureDesc.fBytesPerRow = rowPitch;
		textureDesc.fBytesPerSlice = slicePitch;
		textureDesc.fMipmaps = 1;
		textureDesc.fArraySlices = targetDesc.arraySliceCount();
		textureDesc.fFormat = targetDesc.rasterFormat();
		textureDesc.fTextureType = MHWRender::kImage2D;
		textureDesc.fEnvMapType = MHWRender::kEnvNone;

		// Construct the texture with the screen pixels
		texture = textureManager->acquireTexture( MString("hwApiTextureTest"), textureDesc, targetData );
		delete targetData;
	}

	return texture;
}

bool hwRendererHelper::renderTextureToScreen(MHWRender::MTexture* texture)
{
	const MHWRender::MRenderTargetManager* targetManager = fRenderer->getRenderTargetManager();
	if(targetManager == NULL)
	{
		//MGlobal::displayWarning( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kErrorTargetManager ) );
		return false;
	}

	MHWRender::MTextureDescription textureDesc;
	texture->textureDescription(textureDesc);

	MHWRender::MRenderTargetDescription targetDesc(
		MString("hwApiTextureTest"),
		textureDesc.fWidth,
		textureDesc.fHeight,
		1 /*multiSampleCount*/,
		textureDesc.fFormat,
		textureDesc.fArraySlices,
		false /*isCubeMap*/);

	// Create a render target with same size and format as texture
	MHWRender::MRenderTarget* renderTarget = targetManager->acquireRenderTarget( targetDesc );
	if(renderTarget == NULL)
		return false;

	bool result = false;

	// Render texture to target
	if( renderTextureToTarget(texture, renderTarget) )
	{
		// Copy target to screen
		result = fRenderer->copyTargetToScreen( renderTarget );
	}

	targetManager->releaseRenderTarget(renderTarget);

	return result;
}

//-
// Copyright 2012 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+
