//-
// Copyright 2012 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+

#include <stdio.h>

#include <maya/MString.h>
#include <maya/MColor.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MRenderTargetManager.h>
#include <maya/MShaderManager.h>

class viewRenderOverrideTargets : public MHWRender::MRenderOverride
{
public:
	enum
	{
		kMaya3dSceneRender,
		kTargetPreview,
		kPresentOp,
		kOperationCount
	};

	enum {
		kTargetPreviewShader = 0,	// To preview targets
		kShaderCount
	};

	viewRenderOverrideTargets(const MString& name);
	virtual ~viewRenderOverrideTargets();

	virtual MHWRender::DrawAPI supportedDrawAPIs() const;
	virtual bool startOperationIterator();
	virtual MHWRender::MRenderOperation * renderOperation();
	virtual bool nextRenderOperation();
	virtual MStatus setup(const MString& destination);
	virtual MStatus cleanup();
	virtual MString uiName() const
	{
		return mUIName;
	}
	MHWRender::MRenderOperation * getOperation( unsigned int i)
	{
		if (i < kOperationCount)
			return mRenderOperations[i];
		return NULL;
	}

protected:
	MStatus updateRenderOperations();
	MStatus updateShaders(const MHWRender::MShaderManager* shaderMgr);

	MString mUIName;
	MColor mClearColor;

	MHWRender::MRenderOperation * mRenderOperations[kOperationCount];
	MString mRenderOperationNames[kOperationCount];
	bool mRenderOperationEnabled[kOperationCount];
	int mCurrentOperation;

	MHWRender::MShaderInstance * mShaderInstances[kShaderCount];
};

// Scene render to output to targets
class sceneRenderTargets : public MHWRender::MSceneRender
{
public:
    sceneRenderTargets(const MString &name, viewRenderOverrideTargets *override);
    virtual ~sceneRenderTargets();

	virtual MHWRender::MClearOperation & clearOperation();
	virtual MHWRender::MSceneRender::MSceneFilterOption renderFilterOverride();

	virtual void postSceneRender(const MHWRender::MDrawContext & context);
	MHWRender::MTexture *tempColourTarget() const { return mTempColourTarget; }
	MHWRender::MTexture *tempDepthTarget() const { return mTempDepthTarget; }
	void releaseTargets();

protected:
	MHWRender::MTexture *mTempColourTarget;
	MHWRender::MTexture *mTempDepthTarget;
	viewRenderOverrideTargets *mOverride;
};

// Target preview render
class quadRenderTargets : public MHWRender::MQuadRender
{
public:
	quadRenderTargets(const MString &name, viewRenderOverrideTargets *theOverride);
	~quadRenderTargets();

	virtual const MHWRender::MShaderInstance * shader();
	virtual MHWRender::MClearOperation & clearOperation();

	void setShader( MHWRender::MShaderInstance *shader)
	{
		mShaderInstance = shader;
	}

	void updateTargets( MHWRender::MTexture *colorTarget, MHWRender::MTexture *depthTarget );

protected:
	// Shader to use for the quad render
	MHWRender::MShaderInstance *mShaderInstance;

	viewRenderOverrideTargets *mOverride;
};

// Present operation to present to screen
class presentTargetTargets : public MHWRender::MPresentTarget
{
public:
	presentTargetTargets(const MString &name);
	virtual ~presentTargetTargets();
};





