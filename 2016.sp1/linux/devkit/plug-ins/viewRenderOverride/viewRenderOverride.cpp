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

#include "viewRenderOverride.h"

/*
	Sample custom render override class.

	By default the plugin will perform a number of operations
	in order to:

	1) Draw a procedurally generated background
	2) Draw the non-UI parts of the scene
	3) Threshold the scene
	4) Blur the thresholded output
	5) Combine the thresholded output with the original scene (resulting
	   in a "glow")
	6) Draw the UI parts of the scene.
	7) Draw the 2D HUD
	8) 'Present' the final output

	A number of intermediate render targets are created to hold contents
	which are passed from operation to operation.
*/

/*
	Constructor
*/
viewRenderOverride::viewRenderOverride( const MString & name )
	: MRenderOverride( name )
	// This is the UI name which will appear in the "Renderer" menu
	// in a 3D viewport panel. Any valid ascii string name can be
	// used here.
	, mUIName("Sample VP2 Renderer Override")
	, mRendererChangeCB(0)
	, mRenderOverrideChangeCB(0)
{
	for (unsigned int i=0; i<kNumberOfOps; i++)
		mRenderOperations[i] = NULL;
	mCurrentOperation = -1;

	// Init target information for the override
	unsigned int sampleCount = 1; // no multi-sampling
	MHWRender::MRasterFormat colorFormat = MHWRender::kR8G8B8A8_UNORM;
	MHWRender::MRasterFormat depthFormat = MHWRender::kD24S8;

	// There are 3 render targets used for the entire override:
	// 1. Color
	// 2. Depth
	// 3. Intermediate target to perform target blurs
	//
	mTargetOverrideNames[kMyColorTarget] = MString("__viewRenderOverrideCustomColorTarget__");
	mTargetDescriptions	[kMyColorTarget] = new MHWRender::MRenderTargetDescription(mTargetOverrideNames[kMyColorTarget], 256, 256, sampleCount, colorFormat, 0, false);
	mTargets			[kMyColorTarget] = NULL;
	mTargetSupportsSRGBWrite[kMyColorTarget] = false;

	mTargetOverrideNames[kMyDepthTarget] = MString("__viewRenderOverrideCustomDepthTarget__");
	mTargetDescriptions [kMyDepthTarget] = new MHWRender::MRenderTargetDescription(mTargetOverrideNames[kMyDepthTarget], 256, 256, sampleCount, depthFormat, 0, false);
	mTargets			[kMyDepthTarget] = NULL;
	mTargetSupportsSRGBWrite[kMyDepthTarget] = false;

	mTargetOverrideNames[kMyBlurTarget] = MString("__viewRenderOverrideBlurTarget__");
	mTargetDescriptions [kMyBlurTarget]= new MHWRender::MRenderTargetDescription(mTargetOverrideNames[kMyBlurTarget], 256, 256, sampleCount, colorFormat, 0, false);
	mTargets			[kMyBlurTarget] = NULL;
	mTargetSupportsSRGBWrite[kMyBlurTarget] = false;

	// Set to true to split UI and non-UI draw
	mSplitUIDraw = false;

	// For debugging
	mDebugOverride = false;

	// Default do full effects
	mSimpleRendering = false;

	mPanelName.clear();
}

/*
	Destructor. Make sure to clean up any resources allocated for this override
*/
viewRenderOverride::~viewRenderOverride()
{
    MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
	const MHWRender::MRenderTargetManager* targetManager = theRenderer ? theRenderer->getRenderTargetManager() : NULL;

	// Delete any targets created
	for(unsigned int targetId = 0; targetId < kTargetCount; ++targetId)
	{
		if (mTargetDescriptions[targetId])
		{
			delete mTargetDescriptions[targetId];
			mTargetDescriptions[targetId] = NULL;
		}

		if (mTargets[targetId])
		{
			if (targetManager)
			{
				targetManager->releaseRenderTarget(mTargets[targetId]);
			}
			mTargets[targetId] = NULL;
		}
	}

	cleanup();

	// Delete all the operations. This will release any
	// references to other resources user per operation
	//
	for (unsigned int i=0; i<kNumberOfOps; i++)
	{
		if (mRenderOperations[i])
		{
			delete mRenderOperations[i];
			mRenderOperations[i] = NULL;
		}
	}

	// Clean up callbacks
	//
	if (mRendererChangeCB)
	{
		MMessage::removeCallback(mRendererChangeCB);
	}
	if (mRenderOverrideChangeCB)
	{
		MMessage::removeCallback(mRenderOverrideChangeCB);
	}
}

/*
	Return that this plugin supports both GL and DX draw APIs
*/
MHWRender::DrawAPI
viewRenderOverride::supportedDrawAPIs() const
{
	return ( MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile );
}

/*
	Initialize "iterator". We keep a list of operations indexed
	by mCurrentOperation. Set to 0 to point to the first operation.
*/
bool
viewRenderOverride::startOperationIterator()
{
	mCurrentOperation = 0;
	return true;
}

/*
	Return an operation indicated by mCurrentOperation
*/
MHWRender::MRenderOperation *
viewRenderOverride::renderOperation()
{
	if (mCurrentOperation >= 0 && mCurrentOperation < kNumberOfOps)
	{
		while(!mRenderOperations[mCurrentOperation])
		{
			mCurrentOperation++;
			if (mCurrentOperation >= kNumberOfOps)
			{
				return NULL;
			}
		}

		if (mRenderOperations[mCurrentOperation])
		{
			if (mDebugOverride)
			{
				printf("\t%s : Queue render operation[%d] = (%s)\n",
					mName.asChar(),
					mCurrentOperation,
					mRenderOperations[mCurrentOperation]->name().asChar());
			}
			return mRenderOperations[mCurrentOperation];
		}
	}
	return NULL;
}

/*
	Advance "iterator" to next operation
*/
bool
viewRenderOverride::nextRenderOperation()
{
	mCurrentOperation++;
	if (mCurrentOperation < kNumberOfOps)
	{
		return true;
	}
	return false;
}

/*
	Update the render targets that are required for the entire override.
	References to these targets are set on the individual operations as
	required so that they will send their output to the appropriate location.
*/
bool viewRenderOverride::updateRenderTargets()
{
	if (mDebugOverride)
	{
		printf("\t%s : Set output render target overrides: color=%s, depth=%s\n",
			mName.asChar(),
			mTargetDescriptions[0]->name().asChar(), mTargetDescriptions[1]->name().asChar());
	}

	MHWRender::MRenderer *theRenderer = MHWRender::MRenderer::theRenderer();

	// Get the current output target size as specified by the
	// renderer. If it has changed then the targets need to be
	// resized to match.
	unsigned int targetWidth = 0;
	unsigned int targetHeight = 0;
	if( theRenderer )
	{
		theRenderer->outputTargetSize( targetWidth, targetHeight );
	}

	if (mTargetDescriptions[0]->width() != targetWidth ||
		mTargetDescriptions[1]->height() != targetHeight)
	{
		// A resize occured
	}

	// Note that the render target sizes could be set to be
	// smaller than the size used by the renderer. In this case
	// a final present will generally stretch the output.

	// Update size value for all target descriptions kept
	for(unsigned int targetId = 0; targetId < kTargetCount; ++targetId)
	{
		mTargetDescriptions[targetId]->setWidth( targetWidth );
		mTargetDescriptions[targetId]->setHeight( targetHeight );
	}

	// Keep track of whether the main color target can support sRGB write
	bool colorTargetSupportsSGRBWrite = false;
	// Uncomment this to debug if targets support sRGB write.
	static const bool sDebugSRGBWrite = false;
	// Enable to testing unordered write access
	static const bool testUnorderedWriteAccess = false;

	// Either acquire a new target if it didn't exist before, resize
	// the current target.
	const MHWRender::MRenderTargetManager *targetManager = theRenderer ? theRenderer->getRenderTargetManager() : NULL;
	if (targetManager)
	{
		if (sDebugSRGBWrite)
		{
			if ( theRenderer->drawAPI() != MHWRender::kOpenGL)
			{
				// Sample code to scan all available targetgs for sRGB write support
				for (unsigned int i=0; i<MHWRender::kNumberOfRasterFormats; i++)
				{
					if (targetManager->formatSupportsSRGBWrite( (MHWRender::MRasterFormat) i ))
						printf("Format %d supports SRGBwrite\n", i);
				}
			}
		}

		for(unsigned int targetId = 0; targetId < kTargetCount; ++targetId)
		{
			// Check to see if the format supports sRGB write.
			// Set unordered write access flag if test enabled.
			bool supportsSRGBWrite = false;
			if ( theRenderer->drawAPI() != MHWRender::kOpenGL)
			{
				mTargetSupportsSRGBWrite[targetId] = supportsSRGBWrite =
					targetManager->formatSupportsSRGBWrite( mTargetDescriptions[targetId]->rasterFormat() );
			}
			mTargetDescriptions[targetId]->setAllowsUnorderedAccess( testUnorderedWriteAccess );

			// Keep track of whether the main color target can support sRGB write
			if (targetId == kMyColorTarget)
				colorTargetSupportsSGRBWrite = supportsSRGBWrite;

			if (sDebugSRGBWrite)
			{
				if (targetId == kMyColorTarget || targetId == kMyBlurTarget)
				{
					printf("Color target %d supports sRGB write = %d\n", targetId, supportsSRGBWrite );
				}
				// This would be expected to fail.
				if (targetId == kMyDepthTarget)
				{
					printf("Depth target supports sRGB write = %d\n", supportsSRGBWrite );
				}
			}


			// Create a new target
			if (!mTargets[targetId])
			{
				mTargets[targetId] =  targetManager->acquireRenderTarget( *( mTargetDescriptions[targetId]) );
			}
			// "Update" using a description will resize as necessary
			else
			{
				mTargets[targetId]->updateDescription( *( mTargetDescriptions[targetId]) );
			}

			if (testUnorderedWriteAccess && mTargets[targetId])
			{
				MHWRender::MRenderTargetDescription returnDesc;
				(mTargets[targetId])->targetDescription(returnDesc);
				mTargetDescriptions[targetId]->setAllowsUnorderedAccess( returnDesc.allowsUnorderedAccess() );
				printf("Acquire target[%s] with unordered access = %d. Should fail if attempting with depth target = %d\n",
					returnDesc.name().asChar(), returnDesc.allowsUnorderedAccess(),
					targetId == kMyDepthTarget);
			}

		}
	}

	// Update the render targets on the individual operations
	//
	// Set the targets on the operations. For simplicity just
	// passing over the set of all targets used for the frame
	// to each operation.
	//
	viewRenderQuadRender *quadOp = (viewRenderQuadRender *)mRenderOperations[kBackgroundBlit];
	if (quadOp)
		quadOp->setRenderTargets(mTargets);
	viewRenderSceneRender *sceneOp = (viewRenderSceneRender *)mRenderOperations[kMaya3dSceneRender];
	if (sceneOp)
	{
		sceneOp->setRenderTargets(mTargets);
		sceneOp->setEnableSRGBWriteFlag( colorTargetSupportsSGRBWrite );
	}
	viewRenderSceneRender *opaqueSceneOp = (viewRenderSceneRender *)mRenderOperations[kMaya3dSceneRenderOpaque];
	if (opaqueSceneOp)
	{
		opaqueSceneOp->setRenderTargets(mTargets);
		opaqueSceneOp->setEnableSRGBWriteFlag( colorTargetSupportsSGRBWrite );
	}
	viewRenderSceneRender *transparentSceneOp = (viewRenderSceneRender *)mRenderOperations[kMaya3dSceneRenderTransparent];
	if (transparentSceneOp)
	{
		transparentSceneOp->setRenderTargets(mTargets);
		transparentSceneOp->setEnableSRGBWriteFlag( colorTargetSupportsSGRBWrite );
	}
	viewRenderSceneRender *uiSceneOp = (viewRenderSceneRender *)mRenderOperations[kMaya3dSceneRenderUI];
	if (uiSceneOp)
	{
		uiSceneOp->setRenderTargets(mTargets);
		uiSceneOp->setEnableSRGBWriteFlag( false ); // Don't enable sRGB write for UI
	}
	viewRenderQuadRender *quadOp2 = (viewRenderQuadRender *)mRenderOperations[kPostOperation1];
	if (quadOp2)
		quadOp2->setRenderTargets(mTargets);
	viewRenderQuadRender *quadOp3 = (viewRenderQuadRender *)mRenderOperations[kPostOperation2];
	if (quadOp3)
		quadOp3->setRenderTargets(mTargets);
	viewRenderUserOperation *userOp = (viewRenderUserOperation *) mRenderOperations[kUserOpNumber];
	if (userOp)
	{
		userOp->setRenderTargets(mTargets);
		userOp->setEnableSRGBWriteFlag( colorTargetSupportsSGRBWrite ); // Enable sRGB write for user ops
	}
	viewRenderPresentTarget *presentOp = (viewRenderPresentTarget *) mRenderOperations[kPresentOp];
	if (presentOp)
		presentOp->setRenderTargets(mTargets);
	viewRenderQuadRender *thresholdOp = (viewRenderQuadRender *)mRenderOperations[kThresholdOp];
	if (thresholdOp)
		thresholdOp->setRenderTargets(mTargets);
	viewRenderQuadRender *horizBlur = (viewRenderQuadRender *)mRenderOperations[kHorizBlurOp];
	if (horizBlur)
		horizBlur->setRenderTargets(mTargets);
	viewRenderQuadRender *vertBlur = (viewRenderQuadRender *)mRenderOperations[kVertBlurOp];
	if (vertBlur)
		vertBlur->setRenderTargets(mTargets);
	viewRenderQuadRender *blendOp = (viewRenderQuadRender *)mRenderOperations[kBlendOp];
	if (blendOp)
		blendOp->setRenderTargets(mTargets);
	viewRenderHUDOperation *hudOp = (viewRenderHUDOperation *)mRenderOperations[kHUDBlit];
	if (hudOp)
		hudOp->setRenderTargets(mTargets);

	return (mTargets[kMyColorTarget] && mTargets[kMyDepthTarget] && mTargets[kMyBlurTarget]);
}

/*
	"Setup" will be called for each frame update.

	Here we set up the render loop logic and allocate any necessary resources.
	The render loop logic setup is done by setting up a list of
	render operations which will be returned by the "iterator" calls.
*/
MStatus
viewRenderOverride::setup( const MString & destination )
{
	if (mDebugOverride)
	{
		printf("%s : Perform setup with panel [%s]\n",
			mName.asChar(), destination.asChar() );
	}

	// As an example, we keep track of the active 3d viewport panel
	// if any exists. This information is passed to the operations
	// in case they require accessing the current 3d view (M3dView).
	mPanelName.set( destination.asChar() );

	// Track changes to the renderer and override for this viewport (nothing
	// will be printed unless mDebugOverride is true)
	if (!mRendererChangeCB)
	{
		mRendererChangeCB = MUiMessage::add3dViewRendererChangedCallback(destination, sRendererChangeFunc, (void*)mDebugOverride);
	}
	if (!mRenderOverrideChangeCB)
	{
		mRenderOverrideChangeCB = MUiMessage::add3dViewRenderOverrideChangedCallback(destination, sRenderOverrideChangeFunc, (void*)mDebugOverride);
	}

	if (mRenderOperations[kPresentOp] == NULL)
	{
		// Sample of a "simple" render loop.
		// "Simple" means a scene draw + HUD + present to viewport
		if (mSimpleRendering)
		{
			mSplitUIDraw = false;

			mRenderOperations[kBackgroundBlit] = NULL;

			mRenderOperationNames[kMaya3dSceneRender] = "__MySimpleSceneRender";
			simpleViewRenderSceneRender *sceneOp = new simpleViewRenderSceneRender( mRenderOperationNames[kMaya3dSceneRender] );
			mRenderOperations[kMaya3dSceneRender] = sceneOp;

			// NULL out any additional opertions used for the "complex" render looop
			mRenderOperations[kMaya3dSceneRenderOpaque] = NULL;
			mRenderOperations[kMaya3dSceneRenderTransparent] = NULL;
			mRenderOperations[kThresholdOp] = NULL;
			mRenderOperations[kHorizBlurOp] = NULL;
			mRenderOperations[kVertBlurOp] = NULL;
			mRenderOperations[kPostOperation1] = NULL;
			mRenderOperations[kPostOperation2] = NULL;
			mRenderOperations[kMaya3dSceneRenderUI] = NULL;
			mRenderOperations[kUserOpNumber] = NULL;

			mRenderOperations[kHUDBlit] = new viewRenderHUDOperation();
			mRenderOperationNames[kHUDBlit] = (mRenderOperations[kHUDBlit])->name();

			mRenderOperationNames[kPresentOp] = "__MyPresentTarget";
			mRenderOperations[kPresentOp] = new viewRenderPresentTarget( mRenderOperationNames[kPresentOp] );
			mRenderOperationNames[kPresentOp] = (mRenderOperations[kPresentOp])->name();
		}

		// Sample which performs the full "complex" render loop
		//
		else
		{
			MFloatPoint rect;
			rect[0] = 0.0f;
			rect[1] = 0.0f;
			rect[2] = 1.0f;
			rect[3] = 1.0f;

			// Pre scene quad render to render a procedurally drawn background
			//
			mRenderOperationNames[kBackgroundBlit] = "__MyPreQuadRender";
			viewRenderQuadRender *quadOp = new viewRenderQuadRender( mRenderOperationNames[kBackgroundBlit] );
			quadOp->setShader( viewRenderQuadRender::kPre_MandelBrot ); // We use a shader override to render the background
			quadOp->setViewRectangle(rect);
			mRenderOperations[kBackgroundBlit] = quadOp;

			// Set up scene draw operations
			//
			// This flag indicates that we wish to split up the scene draw into
			// opaque, transparent, and UI passes.
			//
			// When we don't split up the UI from the opaque and transparent,
			// the UI will have the "glow" effect applied to it. Instead
			// splitting up will allow the UI to draw after the "glow" effect
			// has been applied.
			//
			mSplitUIDraw = true;
			mRenderOperations[kMaya3dSceneRender] = NULL;
			mRenderOperations[kMaya3dSceneRenderOpaque] = NULL;
			mRenderOperations[kMaya3dSceneRenderTransparent] = NULL;
			mRenderOperations[kMaya3dSceneRenderUI] = NULL;
			if (mSplitUIDraw)
			{
				// opaque
				viewRenderSceneRender* sceneOp = NULL;
				static const bool sDrawOpaque = true; // can disable if desired
				if (sDrawOpaque)
				{
					mRenderOperationNames[kMaya3dSceneRenderOpaque] = "__MyStdSceneRenderOpaque";
					sceneOp = new viewRenderSceneRender(
						mRenderOperationNames[kMaya3dSceneRenderOpaque],
						MHWRender::MSceneRender::kRenderOpaqueShadedItems,
						(unsigned int)(MHWRender::MClearOperation::kClearDepth | MHWRender::MClearOperation::kClearStencil));
					sceneOp->setViewRectangle(rect);
					mRenderOperations[kMaya3dSceneRenderOpaque] = sceneOp;
				}

				// transparent, clear nothing since needs to draw on top of opaque
				static const bool sDrawTransparent = true; // can disable if desired
				if (sDrawTransparent)
				{
					mRenderOperationNames[kMaya3dSceneRenderTransparent] = "__MyStdSceneRenderTransparent";
					sceneOp = new viewRenderSceneRender(
						mRenderOperationNames[kMaya3dSceneRenderTransparent],
						MHWRender::MSceneRender::kRenderTransparentShadedItems,
						sDrawOpaque
							? (unsigned int)(MHWRender::MClearOperation::kClearNone)
							: (unsigned int)(MHWRender::MClearOperation::kClearDepth | MHWRender::MClearOperation::kClearStencil));
					sceneOp->setViewRectangle(rect);
					mRenderOperations[kMaya3dSceneRenderTransparent] = sceneOp;
				}

				// ui, don't clear depth since we need it for drawing ui correctly
				mRenderOperationNames[kMaya3dSceneRenderUI] = "__MyStdSceneRenderUI";
				sceneOp = new viewRenderSceneRender(
					mRenderOperationNames[kMaya3dSceneRenderUI],
					MHWRender::MSceneRender::kRenderUIItems,
					sDrawOpaque || sDrawTransparent
						? (unsigned int)(MHWRender::MClearOperation::kClearStencil)
						: (unsigned int)(MHWRender::MClearOperation::kClearDepth | MHWRender::MClearOperation::kClearStencil));
				sceneOp->setViewRectangle(rect);
				mRenderOperations[kMaya3dSceneRenderUI] = sceneOp;
			}
			else
			{
				// will draw all of opaque, transparent and ui at once
				mRenderOperationNames[kMaya3dSceneRender] = "__MyStdSceneRender";
				viewRenderSceneRender* sceneOp = new viewRenderSceneRender(
					mRenderOperationNames[kMaya3dSceneRender],
					MHWRender::MSceneRender::kNoSceneFilterOverride,
					(unsigned int)(MHWRender::MClearOperation::kClearDepth | MHWRender::MClearOperation::kClearStencil));
				sceneOp->setViewRectangle(rect);
				mRenderOperations[kMaya3dSceneRender] = sceneOp;
			}

			// Set up operations which will perform a threshold and a blur on the thresholded
			// render target. Also included is an operation to blend the non-UI scene
			// render target with the output of this set of operations (thresholded blurred scene)
			//
			mRenderOperationNames[kThresholdOp] = "__ThresholdColor";
			viewRenderQuadRender *quadThreshold  = new viewRenderQuadRender( mRenderOperationNames[kThresholdOp] );
			quadThreshold->setShader( viewRenderQuadRender::kScene_Threshold ); // Use threshold shader
			quadThreshold->setViewRectangle(rect);
			mRenderOperations[kThresholdOp] = quadThreshold;

			mRenderOperationNames[kHorizBlurOp] = "__HorizontalBlur";
			viewRenderQuadRender *quadHBlur  = new viewRenderQuadRender( mRenderOperationNames[kHorizBlurOp] );
			quadHBlur->setShader( viewRenderQuadRender::kScene_BlurHoriz ); // Use horizontal blur shader
			quadHBlur->setViewRectangle(rect);
			mRenderOperations[kHorizBlurOp] = quadHBlur;

			mRenderOperationNames[kVertBlurOp] = "__VerticalBlur";
			viewRenderQuadRender *quadVBlur  = new viewRenderQuadRender( mRenderOperationNames[kVertBlurOp] );
			quadVBlur->setShader( viewRenderQuadRender::kScene_BlurVert ); // Use vertical blur shader
			quadVBlur->setViewRectangle(rect);
			mRenderOperations[kVertBlurOp] = quadVBlur;

			mRenderOperationNames[kBlendOp] = "__SceneBlurBlend";
			viewRenderQuadRender *quadBlend  = new viewRenderQuadRender( mRenderOperationNames[kBlendOp] );
			quadBlend->setShader( viewRenderQuadRender::kSceneBlur_Blend ); // Use color blend shader
			quadBlend->setViewRectangle(rect);
			mRenderOperations[kBlendOp] = quadBlend;

			// Sample custom operation which will peform a custom "scene render"
			//
			mRenderOperationNames[kUserOpNumber] = "__MyCustomSceneRender";
			viewRenderUserOperation *userOp = new viewRenderUserOperation( mRenderOperationNames[kUserOpNumber] );
			userOp->setViewRectangle(rect);
			mRenderOperations[kUserOpNumber] = userOp;

			bool wantPostQuadOps = false;

			// Some sample post scene quad render operations
			// a. Monochrome quad render with custom shader
			mRenderOperationNames[kPostOperation1] = "__PostOperation1";
			viewRenderQuadRender *quadOp2  = new viewRenderQuadRender( mRenderOperationNames[kPostOperation1] );
			quadOp2->setShader( viewRenderQuadRender::kPost_EffectMonochrome );
			quadOp2->setViewRectangle(rect);
			if (wantPostQuadOps)
				mRenderOperations[kPostOperation1] = quadOp2;
			else
				mRenderOperations[kPostOperation1] = NULL;

			// b. Invert quad render with custom shader
			mRenderOperationNames[kPostOperation2] = "__PostOperation2";
			viewRenderQuadRender *quadOp3  = new viewRenderQuadRender( mRenderOperationNames[kPostOperation2] );
			quadOp3->setShader( viewRenderQuadRender::kPost_EffectInvert );
			quadOp3->setViewRectangle(rect);
			if (wantPostQuadOps)
				mRenderOperations[kPostOperation2] = quadOp3;
			else
				mRenderOperations[kPostOperation2] = NULL;

			// "Present" opertion which will display the target for viewports.
			// Operation is a no-op for batch rendering as there is no on-screen
			// buffer to send the result to.
			mRenderOperationNames[kPresentOp] = "__MyPresentTarget";
			mRenderOperations[kPresentOp] = new viewRenderPresentTarget( mRenderOperationNames[kPresentOp] );
			mRenderOperationNames[kPresentOp] = (mRenderOperations[kPresentOp])->name();

			// A preset 2D HUD render operation
			mRenderOperations[kHUDBlit] = new viewRenderHUDOperation();
			mRenderOperationNames[kHUDBlit] = (mRenderOperations[kHUDBlit])->name();
		}
	}

	bool gotTargets = true;
	if (!mSimpleRendering)
	{
		// Update any of the render targets which will be required
		gotTargets = updateRenderTargets();

		// Set the name of the panel on operations which may use the panel
		// name to find out the associated M3dView.
		if (mRenderOperations[kMaya3dSceneRender])
			((viewRenderSceneRender * )mRenderOperations[kMaya3dSceneRender])->setPanelName( mPanelName );
		if (mRenderOperations[kMaya3dSceneRenderOpaque])
			((viewRenderSceneRender * )mRenderOperations[kMaya3dSceneRenderOpaque])->setPanelName( mPanelName );
		if (mRenderOperations[kMaya3dSceneRenderTransparent])
			((viewRenderSceneRender * )mRenderOperations[kMaya3dSceneRenderTransparent])->setPanelName( mPanelName );
		if (mRenderOperations[kMaya3dSceneRenderUI])
			((viewRenderSceneRender * )mRenderOperations[kMaya3dSceneRenderUI])->setPanelName( mPanelName );
		if (mRenderOperations[kUserOpNumber])
			((viewRenderUserOperation *)mRenderOperations[kUserOpNumber])->setPanelName( mPanelName );
	}
	mCurrentOperation = -1;

	return (gotTargets ? MStatus::kSuccess : MStatus::kFailure );
}

/*
	End of frame cleanup. For now just clears out any data on operations which may
	change from frame to frame (render target, output panel name etc)
*/
MStatus
viewRenderOverride::cleanup()
{
	if (mDebugOverride)
	{
		printf("%s : Perform cleanup. panelname=%s\n", mName.asChar(), mPanelName.asChar());
	}

	viewRenderQuadRender *quadOp = (viewRenderQuadRender *) mRenderOperations[kPostOperation1];
	if (quadOp)
	{
		quadOp->setRenderTargets(NULL);
	}
	quadOp = (viewRenderQuadRender *) mRenderOperations[kPostOperation2];
	if (quadOp)
	{
		quadOp->setRenderTargets(NULL);
	}

	// Reset the active view
	mPanelName.clear();
	// Reset current operation
	mCurrentOperation = -1;

	return MStatus::kSuccess;
}

// Callback for tracking renderer changes
void viewRenderOverride::sRendererChangeFunc(
	const MString& panelName,
	const MString& oldRenderer,
	const MString& newRenderer,
	void* clientData)
{
	if (clientData)
	{
		printf("Renderer changed for panel '%s'. New renderer is '%s', old was '%s'.\n",
			panelName.asChar(), newRenderer.asChar(), oldRenderer.asChar());
	}
}

// Callback for tracking render override changes
void viewRenderOverride::sRenderOverrideChangeFunc(
	const MString& panelName,
	const MString& oldOverride,
	const MString& newOverride,
	void* clientData)
{
	if (clientData)
	{
		printf("Render override changed for panel '%s'. New override is '%s', old was '%s'.\n",
			panelName.asChar(), newOverride.asChar(), oldOverride.asChar());
	}
}
