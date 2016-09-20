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
#include <maya/MTime.h>
#include <map>

class viewRenderOverrideFrameCache : public MHWRender::MRenderOverride
{
public:
	enum
	{
		kMaya3dSceneRender, // Scene render
		kTargetCapture,		// Capture target
		kTargetPreview,		// Preview target
		kPresentOp,			// Present target
		kOperationCount
	};

	enum {
		kTargetPreviewShader = 0,	// To preview targets
		kShaderCount
	};

	viewRenderOverrideFrameCache(const MString& name);
	virtual ~viewRenderOverrideFrameCache();

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
	void setAllowCaching( bool val )
	{
		mAllowCaching = val;
	}
	void setCacheToDisk( bool val )
	{
		mCacheToDisk = val;
	}
	// Exposed to allow cache to be cleared
	void releaseCachedTextures();

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
	
	// Simple cache of <time,texture> pairs. The texture
	// is a snapshot of the target rendered at a given time.
	//
	std::map<unsigned int, MHWRender::MTexture*> mCachedTargets;

	// Current "mode" to perform (capture or playback)
	bool mPerformCapture;
	// Current texture to blit or captured
	MHWRender::MTexture *mCachedTexture;
	// Current time of rendering
	unsigned int mCurrentTime;
	// # of non-integer sub-frame samples allowed
	double mSubFrameSamples;

	bool mAllowCaching;
	bool mCacheToDisk ;
};

////////////////////////////////////////////////////////////////////////////
// Scene render 
////////////////////////////////////////////////////////////////////////////
class SceneRenderOperation : public MHWRender::MSceneRender
{
public:
    SceneRenderOperation(const MString &name, viewRenderOverrideFrameCache* theOverride);
    virtual ~SceneRenderOperation();
protected:
	viewRenderOverrideFrameCache *mOverride;
};

////////////////////////////////////////////////////////////////////////////
// Target preview render
////////////////////////////////////////////////////////////////////////////
class PreviewTargetsOperation : public MHWRender::MQuadRender
{
public:
	PreviewTargetsOperation(const MString &name, viewRenderOverrideFrameCache* theOverride);
	~PreviewTargetsOperation();

	virtual const MHWRender::MShaderInstance * shader();
	virtual MHWRender::MClearOperation & clearOperation();

	void setShader( MHWRender::MShaderInstance *shader)
	{
		mShaderInstance = shader;
	}
	void setTexture( MHWRender::MTexture *texture )
	{
		mTexture = texture;
	}

protected:
	viewRenderOverrideFrameCache *mOverride;

	// Shader and texture used for quad render
	MHWRender::MShaderInstance *mShaderInstance;
	MHWRender::MTexture *mTexture;
};

////////////////////////////////////////////////////////////////////////////
// Capture targets
////////////////////////////////////////////////////////////////////////////
class CaptureTargetsOperation : public MHWRender::MUserRenderOperation
{
public:
	CaptureTargetsOperation(const MString &name, viewRenderOverrideFrameCache *override);
	virtual ~CaptureTargetsOperation();

	virtual MStatus execute( const MHWRender::MDrawContext & drawContext );

	void setTexture( MHWRender::MTexture* texture)
	{
		mTexture = texture;
	}
	MHWRender::MTexture* getTexture() const
	{
		return mTexture;
	}
	void setCurrentTime(unsigned int val)
	{
		mCurrentTime = val;
	}
	unsigned int getCurrentTime() const
	{
		return mCurrentTime;
	}
	void setDumpImageToDisk(bool val)
	{
		mDumpImageToDisk = val;
	}
	bool dumpImageToDisk() const
	{
		return mDumpImageToDisk;
	}
protected:
	viewRenderOverrideFrameCache *mOverride;
	MHWRender::MTexture *mTexture;
	unsigned int mCurrentTime;
	bool mDumpImageToDisk;
};

////////////////////////////////////////////////////////////////////////////
// Present operation to present to screen
////////////////////////////////////////////////////////////////////////////
class presentTargets : public MHWRender::MPresentTarget
{
public:
	presentTargets(const MString &name);
	virtual ~presentTargets();
};





