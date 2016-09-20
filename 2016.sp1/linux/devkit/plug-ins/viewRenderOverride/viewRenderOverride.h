//-
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+

#include <stdio.h>

#include <maya/MString.h>
#include <maya/MDrawTraversal.h>
#include <maya/MSelectionList.h>
#include <maya/M3dView.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MDrawContext.h>
#include <maya/MFrameContext.h>
#include <maya/MUiMessage.h>

//////////////////////////////////////////////////////////////////////////
/*
	Sample custom render override class.

	Is responsible for setting up the render loop operations and
	updating resources for each frame render as well as any
	rendering options.

	By default the plugin will perform a number of operations
	in order to:

	1) Draw a procedurally generated background
	2) Draw the non-UI parts of the scene using internal logic.
	3) Threshold the scene
	4) Blur the thresholded output
	5) Combine the thresholded output with the original scene (resulting
	   in a "glow")
	6a) Draw the UI parts of the scene using internal logic.
	6b) Perform an option custom user operation for additional UI.
	7) Draw the 2D HUD
	8) 'Present' the final output

	A number of intermediate render targets are created to hold contents
	which are passed from operation to operation.
*/
enum ETargetId // Helper to enumerate the target indexing
{
	kMyColorTarget,
	kMyDepthTarget,
	kMyBlurTarget,

	kTargetCount
};
class viewRenderOverride : public MHWRender::MRenderOverride
{
public:
	// Enumerations to identify an operation within
	// a list of operations.
	enum {
		kBackgroundBlit = 0,
		// 3d scene render to target 1
		kMaya3dSceneRender,
		// 3d opaque scene render to target 1
		kMaya3dSceneRenderOpaque,
		// 3d transparent scene render to target 1
		kMaya3dSceneRenderTransparent,
		// Brightness threshold
		kThresholdOp,
		// Down sample to target 2
		kHorizBlurOp,
		kVertBlurOp,
		// Blend target 1 and 2 back to target 1
		kBlendOp,
		// Post ops on target 1
		kPostOperation1,
		kPostOperation2,
		// Post ui draw to target 1
		kMaya3dSceneRenderUI,
		// User op draw to target 1
		kUserOpNumber,
		// Draw HUD on top
		kHUDBlit,
		// Present
		kPresentOp,
		kNumberOfOps
	};

	viewRenderOverride( const MString & name );
	virtual ~viewRenderOverride();
	virtual MHWRender::DrawAPI supportedDrawAPIs() const;

	// Virtual overrides to return operations in this override
	virtual bool startOperationIterator();
	virtual MHWRender::MRenderOperation * renderOperation();
	virtual bool nextRenderOperation();

	// Override setup and cleanup for this override
	//
	virtual MStatus setup( const MString & destination );
	virtual MStatus cleanup();

	// Unique name for this override
	static MString gsVROName;

	const MString & panelName() const
	{
		return mPanelName;
	}
	void setSimpleRendering( bool flag )
	{
		mSimpleRendering = flag;
	}
	virtual MString uiName() const
	{
		return mUIName;
	}

protected:
	bool updateRenderTargets();

	// Operation lists
	MHWRender::MRenderOperation * mRenderOperations[kNumberOfOps];
	MString mRenderOperationNames[kNumberOfOps];
	int mCurrentOperation;

	// Shared render target list
	MString mTargetOverrideNames[kTargetCount];
	MHWRender::MRenderTargetDescription* mTargetDescriptions[kTargetCount];
	MHWRender::MRenderTarget* mTargets[kTargetCount];
	bool mTargetSupportsSRGBWrite[kTargetCount];

	// Override is for this panel
	MString mPanelName;

	// Split UI / nonUI draw
	bool mSplitUIDraw;

	// Simple rendering
	bool mSimpleRendering;

	// UI name which will show up in places
	// like the viewport 'Renderer' menu
	MString mUIName;

	bool mDebugOverride;

	// Callback IDs for tracking viewport changes
	MCallbackId mRendererChangeCB;
	MCallbackId mRenderOverrideChangeCB;
	static void sRendererChangeFunc(
		const MString& panelName,
		const MString& oldRenderer,
		const MString& newRenderer,
		void* clientData);
	static void sRenderOverrideChangeFunc(
		const MString& panelName,
		const MString& oldRenderer,
		const MString& newRenderer,
		void* clientData);
};

//////////////////////////////////////////////////////////////////////////
//
// Render override support classes
//
//------------------------------------------------------------------------
//	Custom present target operation
//
//	Only overrides the targets to present
//
class viewRenderPresentTarget : public MHWRender::MPresentTarget
{
public:
	viewRenderPresentTarget(const MString &name);
	~viewRenderPresentTarget();

	virtual MHWRender::MRenderTarget* const* targetOverrideList(unsigned int &listSize);

	void setRenderTargets(MHWRender::MRenderTarget **targets);
protected:
	// Targets used as input parameters to mShaderInstance;
	MHWRender::MRenderTarget **mTargets;
};

//------------------------------------------------------------------------
// Custom quad operation
//
//	General quad operation which can be instantiated with a few
//	different shaders.
//
class viewRenderQuadRender : public MHWRender::MQuadRender
{
public:
	// Shader list
	typedef enum {
		kEffectNone,
		kPost_EffectMonochrome, // Mono color shader
		kPost_EffectEdgeDetect,	// Edge detect shader
		kPost_EffectInvert,		// Invert color shader
		kScene_Threshold,		// Color threshold shader
		kScene_BlurHoriz,		// Horizontal blur shader
		kScene_BlurVert,		// Vertical blur shader
		kSceneBlur_Blend,		// Blend shader
		kPre_MandelBrot,		// Mandelbrot shader
	} MQuadShader ;

	viewRenderQuadRender(const MString &name);
	~viewRenderQuadRender();

	virtual const MHWRender::MShaderInstance * shader();
	virtual MHWRender::MRenderTarget* const* targetOverrideList(unsigned int &listSize);
	virtual MHWRender::MClearOperation & clearOperation();

	void setRenderTargets(MHWRender::MRenderTarget **targets)
	{
		mTargets = targets;
	}
	void setShader( MQuadShader shader )
	{
		mShader = shader;
	}
	MQuadShader shader() const
	{
		return mShader;
	}
	const MFloatPoint & viewRectangle() const
	{
		return mViewRectangle;
	}
	void setViewRectangle(const MFloatPoint & rect)
	{
		mViewRectangle = rect;
	}

protected:
	// Shader to use for the quad render
	MHWRender::MShaderInstance *mShaderInstance;
	// Targets used as input parameters to mShaderInstance;
	MHWRender::MRenderTarget** mTargets;
	// View rectangle
	MFloatPoint mViewRectangle;

	// Shader to use for quad rendering
	MQuadShader mShader;
};

//------------------------------------------------------------------------
// Custom hud operation
//
class viewRenderHUDOperation : public MHWRender::MHUDRender
{
public:
	viewRenderHUDOperation()
	{
		mTargets = NULL;
	}

	// Target override
	virtual MHWRender::MRenderTarget* const* targetOverrideList(unsigned int &listSize)
	{
		if (mTargets)
		{
			listSize = 2;
			return &mTargets[0];
		}
		listSize = 0;
		return NULL;
	}

	virtual bool hasUIDrawables() const
	{
		return true;
	}

	virtual void addUIDrawables( MHWRender::MUIDrawManager& drawManager2D, const MHWRender::MFrameContext& frameContext );

	void setRenderTargets(MHWRender::MRenderTarget **targets)
	{
		mTargets = targets;
	}

	~viewRenderHUDOperation()
	{
		mTargets = NULL;
	}

protected:
	MHWRender::MRenderTarget** mTargets;
};


//------------------------------------------------------------------------
//
// Simple scene operation that just overrides the clear operation
// and the viewport rectange.
//
class simpleViewRenderSceneRender : public MHWRender::MSceneRender
{
public:
    simpleViewRenderSceneRender(const MString &name);
	virtual const MFloatPoint * viewportRectangleOverride();
	virtual MHWRender::MClearOperation & clearOperation();

	MFloatPoint mViewRectangle;
};

//------------------------------------------------------------------------
/* Custom scene operation

	A scene render which is reused as necessary with different
	override parameters
*/
class viewRenderSceneRender : public MHWRender::MSceneRender
{
public:
	viewRenderSceneRender(
		const MString &name,
		MHWRender::MSceneRender::MSceneFilterOption sceneFilter,
		unsigned int clearMask);
	virtual ~viewRenderSceneRender();

	void setRenderTargets(MHWRender::MRenderTarget **targets);

	virtual MHWRender::MRenderTarget* const* targetOverrideList(unsigned int &listSize);
	virtual bool enableSRGBWrite();
	virtual const MHWRender::MCameraOverride * cameraOverride();
	virtual MHWRender::MSceneRender::MSceneFilterOption renderFilterOverride();
	virtual MHWRender::MSceneRender::MDisplayMode displayModeOverride();
	virtual MHWRender::MSceneRender::MLightingMode lightModeOverride();
	virtual const bool* shadowEnableOverride();
	virtual MHWRender::MSceneRender::MCullingOption cullingOverride();

	virtual void preRender();
	virtual void postRender();
	virtual MHWRender::MSceneRender::MObjectTypeExclusions objectTypeExclusions();
	virtual const MSelectionList * objectSetOverride();
	virtual MHWRender::MClearOperation & clearOperation();
	virtual const MHWRender::MShaderInstance* shaderOverride();
	virtual bool hasUIDrawables() const
	{
		return true;
	}

	virtual void addPreUIDrawables( MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext& frameContext );
	virtual void addPostUIDrawables( MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext& frameContext );

	const MString & panelName() const
	{
		return mPanelName;
	}
	void setPanelName( const MString & name )
	{
		mPanelName.set( name.asChar() );
	}
	const MFloatPoint & viewRectangle() const
	{
		return mViewRectangle;
	}
	void setViewRectangle(const MFloatPoint & rect)
	{
		mViewRectangle = rect;
	}

	MHWRender::MRenderTarget* colorTarget() const
	{
		return mTargets ? mTargets[kMyColorTarget] : NULL;
	}
	MHWRender::MRenderTarget* depthTarget() const
	{
		return mTargets ? mTargets[kMyDepthTarget] : NULL;
	}
	void setEnableSRGBWriteFlag( bool val )
	{
		mEnableSRGBWrite = val;
	}
	bool enableSRGBWriteFlag() const
	{
		return mEnableSRGBWrite;
	}

protected:
	MSelectionList mSelectionList;

	// 3D viewport panel name, if available
	MString mPanelName;
	// Camera override
	MHWRender::MCameraOverride mCameraOverride;
	// Viewport rectangle override
	MFloatPoint mViewRectangle;
	// Available render targets
	MHWRender::MRenderTarget** mTargets;
	// Shader override for surfaces
	MHWRender::MShaderInstance *mShaderOverride;
	// Scene draw filter override
	MHWRender::MSceneRender::MSceneFilterOption mSceneFilter;
	// Mask for clear override
	unsigned int mClearMask;

	// Some sample override flags
	bool	mUseShaderOverride;
	bool	mUseStockShaderOverride;
	bool	mAttachPrePostShaderCallback;
	bool	mUseShadowShader;
	bool	mOverrideDisplayMode;
	bool	mOverrideLightingMode;
	bool	mOverrideCullingMode;
	bool	mDebugTargetResourceHandle;
	bool	mOverrrideM3dViewDisplayMode;
	M3dView::DisplayStyle mPrevDisplayStyle; // Track previous display style of override set
	bool	mFilterDrawNothing;
	bool	mFilterDrawSelected;
	bool	mEnableSRGBWrite;
};

//------------------------------------------------------------------------
/*
	Custom user operation. One approach to adding a pre and
	post scene callback. In this approach only 1 operation
	is reused twice with internal state as to when it is being
	used. Another approach which may be more suitable for when
	global state is changed is to create 2 instances of this
	operation and keep global state on the override instead of
	locally here.

	The cost of an override is very small so creating more instances
	can provide a clearer and cleaner render loop logic.
*/
class viewRenderUserOperation : public MHWRender::MUserRenderOperation
{
public:
	viewRenderUserOperation(const MString &name);
	virtual ~viewRenderUserOperation();

	virtual MStatus execute( const MHWRender::MDrawContext & drawContext );
	virtual const MHWRender::MCameraOverride * cameraOverride();
	virtual MHWRender::MRenderTarget* const* targetOverrideList(unsigned int &listSize);
	virtual bool enableSRGBWrite();
	virtual bool hasUIDrawables() const;
	virtual void addUIDrawables(
		MHWRender::MUIDrawManager& drawManager,
		const MHWRender::MFrameContext& frameContext );

	void setRenderTargets(MHWRender::MRenderTarget **targets)
	{
		mTargets = targets;
	}
	void setEnableSRGBWriteFlag( bool val )
	{
		fEnableSRGBWriteFlag = val;
	}

	const MString & panelName() const
	{
		return mPanelName;
	}
	void setPanelName( const MString & name )
	{
		mPanelName.set( name.asChar() );
	}
	const MFloatPoint & viewRectangle() const
	{
		return mViewRectangle;
	}
	void setViewRectangle(const MFloatPoint & rect)
	{
		mViewRectangle = rect;
	}

protected:
	// 3D viewport panel name, if any
	MString mPanelName;
	// Camera override
	MHWRender::MCameraOverride mCameraOverride;
	// Viewport rectangle override
	MFloatPoint mViewRectangle;
	// Available targets
	MHWRender::MRenderTarget** mTargets;
	// sRGB write flag
	bool fEnableSRGBWriteFlag;
	// Draw an extra label
	bool fDrawLabel;
	// Use camera override
	bool fUserCameraOverride;
	// Draw colored bounding boxes
	bool fDrawBoundingBoxes;
	// Debugging flags
	bool fDebugDrawContext;
	bool fDebugLightingInfo;
};

/*
	Draw traversal utility used by
	the custom user operation (viewRenderUserOperation)
*/
class MSurfaceDrawTraversal : public MDrawTraversal
{
	// Override to filter out some node types.
	virtual bool filterNode( const MDagPath &traversalItem );
};

/*
	A very simplistic custom scene draw example which just draws
	coloured bounding boxes for surface types.

	This code is similiar to the example code found in the
	plugin example OpenGLViewportRenderer and just
	traverses the scene to perform a draw.

	Used by the custom user operation (viewRenderUserOperation)
*/
class MCustomSceneDraw
{
public:
	MCustomSceneDraw();
	~MCustomSceneDraw();

	// Draw a bounding box
	void drawBounds( const MDagPath &dagPath,
					 const MBoundingBox &box);

	// Draw a scene full of bounding boxes
	bool draw(MDagPath &cameraPath, unsigned int width, unsigned int height);
};




//////////////////////////////////////////////////////////////////////////
/*
	Some utility routines
*/
class viewRenderOverrideUtilities
{
public:
	static void printDrawContextLightInfo( const MHWRender::MDrawContext & drawContext );
};


