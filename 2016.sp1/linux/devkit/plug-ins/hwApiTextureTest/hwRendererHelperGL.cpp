#include "hwRendererHelperGL.h"
#include "hwApiTextureTestStrings.h"

#include <maya/MTextureManager.h>
#include <maya/MRenderTargetManager.h>
#include <maya/MGLFunctionTable.h>
#include <maya/MHardwareRenderer.h>


hwRendererHelperGL::hwRendererHelperGL(MHWRender::MRenderer* renderer)
: hwRendererHelper(renderer)
{
}

hwRendererHelperGL::~hwRendererHelperGL()
{
}

bool hwRendererHelperGL::renderTextureToTarget(MHWRender::MTexture* texture, MHWRender::MRenderTarget *target)
{
	static MGLFunctionTable *gGLFT = MHardwareRenderer::theRenderer()->glFunctionTable();
	if (gGLFT == NULL || gGLFT->extensionExists( kMGLext_frame_buffer_object ) == false)
		return false;

	if (texture == NULL || target == NULL)
		return false;

	MGLuint *pTextureId = (MGLuint *)texture->resourceHandle();
	MGLuint *pTargetId  = (MGLuint *)target->resourceHandle();
	if (pTextureId == NULL || pTargetId == NULL)
		return false;

	MHWRender::MTextureDescription textureDesc;
	texture->textureDescription(textureDesc);

	MHWRender::MRenderTargetDescription targetDesc;
	target->targetDescription(targetDesc);

	if (textureDesc.fFormat != targetDesc.rasterFormat())
		return false;

	MGLint srcX0 = 0;
	MGLint srcY0 = textureDesc.fHeight;
	MGLint srcX1 = textureDesc.fWidth;
	MGLint srcY1 = 0;

	MGLint dstX0 = 0;
	MGLint dstY0 = 0;
	MGLint dstX1 = targetDesc.width();
	MGLint dstY1 = targetDesc.height();

    // Generates 2 framebuffers, one as READ_FRAMEBUFFER and the other as DRAW_FRAMEBFFER.
    MGLuint framebuffers[2] = {0, 0};
	gGLFT->glGenFramebuffersEXT(2, framebuffers);
	if (framebuffers[0] == 0 || framebuffers[1] == 0)
		return false;

    // Bind the first FBO as the read framebuffer, and attach texture
    gGLFT->glBindFramebufferEXT(MGL_READ_FRAMEBUFFER, framebuffers[0]);
	gGLFT->glFramebufferTexture2DEXT(MGL_READ_FRAMEBUFFER, MGL_COLOR_ATTACHMENT0, MGL_TEXTURE_2D, *pTextureId, 0);

    // Bind the 2nd FBO as the draw frambuffer, and attach the target
    gGLFT->glBindFramebufferEXT(MGL_DRAW_FRAMEBUFFER, framebuffers[1]);
	gGLFT->glFramebufferTexture2DEXT(MGL_DRAW_FRAMEBUFFER, MGL_COLOR_ATTACHMENT0, MGL_TEXTURE_2D, *pTargetId, 0);

	// Copy data from read buffer (the texture) to draw buffer (the target)
	gGLFT->glBlitFramebufferEXT(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, MGL_COLOR_BUFFER_BIT, MGL_LINEAR);

	// Restore the framebuffer binding.
	gGLFT->glBindFramebufferEXT(MGL_FRAMEBUFFER, 0);

	// Deletes the 2 framebuffers.
	gGLFT->glDeleteFramebuffersEXT(2, framebuffers);

	return true;
}

//-
// Copyright 2012 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+
