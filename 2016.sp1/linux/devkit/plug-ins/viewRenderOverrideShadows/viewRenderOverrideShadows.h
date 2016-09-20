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
#include <maya/MSelectionList.h>

class viewRenderOverrideShadows : public MHWRender::MRenderOverride
{
public:
	enum
	{
		kShadowPrePass,
		kMaya3dSceneRender,
		kPresentOp,
		kOperationCount
	};

	viewRenderOverrideShadows(const MString& name);
	virtual ~viewRenderOverrideShadows();

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

protected:
	MStatus updateRenderOperations();
	MStatus updateShaders(const MHWRender::MShaderManager* shaderMgr);
	MStatus updateLightList();

	MString mUIName;
	MColor mClearColor;

	MHWRender::MRenderOperation * mRenderOperations[kOperationCount];
	MString mRenderOperationNames[kOperationCount];
	bool mRenderOperationEnabled[kOperationCount];
	int mCurrentOperation;

	MHWRender::MShaderInstance * mLightShader;
	MSelectionList mLightList;
};

// Scene render to output to targets
class sceneRender : public MHWRender::MSceneRender
{
public:
    sceneRender(const MString &name);
    virtual ~sceneRender();

	virtual const MHWRender::MShaderInstance* shaderOverride();
	virtual void preSceneRender(const MHWRender::MDrawContext & context);

	void setShader( MHWRender::MShaderInstance *shader)
	{
		mLightShader = shader;
	}
	void setLightList( MSelectionList *val )
	{
		mLightList = val;
	}

protected:
	MHWRender::MShaderInstance *mLightShader;
	MSelectionList *mLightList;
};

// Shadow prepass operation
class shadowPrepass : public MHWRender::MUserRenderOperation
{
public:
	shadowPrepass(const MString &name);
	virtual ~shadowPrepass();

	virtual MStatus execute( const MHWRender::MDrawContext & drawContext );
	virtual bool requiresLightData() const
	{
		return true;
	}
	void setLightList( MSelectionList *val )
	{
		mLightList = val;
	}

	MSelectionList *mLightList;
};



