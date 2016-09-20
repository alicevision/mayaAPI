#ifndef viewImageBlitOverride_h_
#define viewImageBlitOverride_h_

//-
// Copyright 2012 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+


#include <maya/MString.h>
#include <maya/M3dView.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MShaderManager.h>
#include <maya/MTextureManager.h>
#include <maya/MUiMessage.h>
#include <maya/MStateManager.h>

//
// Sample plugin which will blit an image as the scene and rely on
// Maya's internal rendering for the UI only
// 
namespace viewImageBlitOverride
{
class RenderOverride : public MHWRender::MRenderOverride
{
public:
	RenderOverride( const MString & name );
	virtual ~RenderOverride();
	virtual MHWRender::DrawAPI supportedDrawAPIs() const;

	virtual MStatus setup( const MString & destination );
	virtual bool startOperationIterator();
	virtual MHWRender::MRenderOperation * renderOperation();
	virtual bool nextRenderOperation();
	virtual MStatus cleanup();

	virtual MString uiName() const
	{
		return mUIName;
	}
	
	// Global override instance
	static RenderOverride* sViewImageBlitOverrideInstance;

protected:
	bool updateTextures(MHWRender::MRenderer *theRenderer,
						MHWRender::MTextureManager* textureManager);

	// UI name 
	MString mUIName;

	// Operations + names
	MHWRender::MRenderOperation* mOperations[3];
	MString mOperationNames[3];

	// Texture(s) used for the quad render
	MHWRender::MTextureDescription mColorTextureDesc;
	MHWRender::MTextureDescription mDepthTextureDesc;
	MHWRender::MTextureAssignment mColorTexture;
	MHWRender::MTextureAssignment mDepthTexture;

	int mCurrentOperation;

	// Options
	bool mLoadImagesFromDisk;
};

//
// Image blit used to perform the 'scene render'
//
class SceneBlit : public MHWRender::MQuadRender
{
public:
	SceneBlit(const MString &name);
	~SceneBlit();

	virtual const MHWRender::MShaderInstance * shader();
	virtual MHWRender::MClearOperation & clearOperation();
	virtual const MHWRender::MDepthStencilState *depthStencilStateOverride();

	inline void setColorTexture( const MHWRender::MTextureAssignment &val )
	{
		mColorTexture.texture = val.texture;
		mColorTextureChanged = true;
	}
	inline void setDepthTexture( const MHWRender::MTextureAssignment &val )
	{
		mDepthTexture.texture = val.texture;
		mDepthTextureChanged = true;
	}

protected:
	// Shader to use for the quad render
	MHWRender::MShaderInstance *mShaderInstance;
	// Texture(s) used for the quad render. Not owned by operation.
	MHWRender::MTextureAssignment mColorTexture;
	MHWRender::MTextureAssignment mDepthTexture;
	bool mColorTextureChanged;
	bool mDepthTextureChanged;

	const MHWRender::MDepthStencilState *mDepthStencilState;
};

//
// UI pass
//
class UIDraw : MHWRender::MSceneRender
{
public:
	UIDraw(const MString& name);
	~UIDraw();

	virtual MHWRender::MSceneRender::MSceneFilterOption		renderFilterOverride();
	virtual MHWRender::MSceneRender::MObjectTypeExclusions	objectTypeExclusions();
	virtual MHWRender::MClearOperation &					clearOperation();
};


} //namespace

#endif
