//-
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+
#include <stdio.h>

#include <maya/MViewport2Renderer.h>
#include <maya/MShaderManager.h>

#include <maya/MFnPlugin.h>
#include <maya/MSelectionList.h>
#include <maya/MFnSet.h>

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>

/*
	The following is an example plug-in which shows the various options available for
	the operations in an MRenderOverride (render override) for VP2.

	The render override will appear as an available renderer under the "Renderer" menu
	for 3d viewports. When chosen the renderer will be activated and will use it's default
	options.

	The viewMRenderOverride MEL command can be used to modify the options for the renderer.

	For code clarity, error status checking is minimal.
*/

/*
	Custom scene operation. Currrent options which are shown:

	1) Object set filterting:  Use a set name to filter which objects will be drawn with the operation.
	2) Clear mask setting: Set the clear parameters for the operation. By default both the color and depth are cleared.
	3) Scene element filtering : Set which types of objects to draw based on the MSceneFilterOption value. (e.g.
		setting to draw opaque objects only).
*/
class MSceneRenderTester : public MHWRender::MSceneRender
{
public:
	MSceneRenderTester( const MString& name ) 
	: MHWRender::MSceneRender( name )
	, mClearMask( (unsigned int)MHWRender::MClearOperation::kClearAll )
	, mShaderInstance( NULL )
	, mSceneFilterOperation( MHWRender::MSceneRender::kNoSceneFilterOverride )
	, mSceneUIDrawables(false)
	, mOverrideViewRectangle(false)
	, mDebugTrace(false)
	{
	}

	/*
		Object set to draw. Returning NULL indicates we are not using an set override.
	*/
	virtual const MSelectionList* objectSetOverride()
	{
		// If not name set then don't return an override
		if (mSetName.length())
		{
			MSelectionList list; 
			list.add( mSetName );
			
			MObject obj; 
			list.getDependNode( 0, obj );
			
			MFnSet set( obj ); 
			set.getMembers( mFilterSet, true );
			
			if (mFilterSet.length())
			{
				if (mDebugTrace)
					printf(" %s : Enable set filter = %s\n", mName.asChar(), mSetName.asChar() );
				return &mFilterSet;
			}
			return NULL;
		}
		return NULL;
	}

	/*
		Clear operation control. Will attempt to use the existing values
		found in the renderer for background color.
	*/
	virtual MHWRender::MClearOperation & clearOperation()
	{
		MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
		bool gradient = renderer->useGradient();
		MColor color1 = renderer->clearColor();
		MColor color2 = renderer->clearColor2();

		float c1[4] = { color1[0], color1[1], color1[2], 1.0f };
		float c2[4] = { color2[0], color2[1], color2[2], 1.0f };

		mClearOperation.setClearColor( c1 );
		mClearOperation.setClearGradient( gradient);
		mClearOperation.setClearColor2( c2 );
		mClearOperation.setMask( mClearMask );
		return mClearOperation;
	}

	/*
		Scene element filter. Seem MSceneRender::MSceneFilterOption for description
		of available options.
	*/
	virtual MHWRender::MSceneRender::MSceneFilterOption renderFilterOverride()
	{
		if (mDebugTrace)
		{
			if (mSceneFilterOperation != MHWRender::MSceneRender::kNoSceneFilterOverride &&
				mSceneFilterOperation != MHWRender::MSceneRender::kRenderAllItems)
			{
				printf(" %s : Set scene filter = %d\n", mName.asChar(), mSceneFilterOperation);
			}
		}
		return mSceneFilterOperation;
	}
	
	/* 
		Shader override used for objects which are surfaces.
		Returning NULL will indicate to not use a shader override.
	*/ 
	virtual const MHWRender::MShaderInstance* shaderOverride()
	{
		if (mDebugTrace && mShaderInstance)
			printf(" %s : Enable shader override\n", mName.asChar() );
		return mShaderInstance;
	}

	virtual void preRender() 
	{
		if (mDebugTrace)
			printf(" %s : preRender callback\n", mName.asChar());
	}

	virtual void postRender()
	{
		if (mDebugTrace)
			printf(" %s : postRender callback\n", mName.asChar());
	}
	virtual void preSceneRender(const MHWRender::MDrawContext & context)
	{
		if (mDebugTrace)
			printf(" %s : preScene callback\n", mName.asChar());
	}
	virtual void postSceneRender(const MHWRender::MDrawContext & context)
	{
		if (mDebugTrace)
			printf(" %s : postScene callback\n", mName.asChar());
	}

	virtual bool hasUIDrawables() const
	{
		return mSceneUIDrawables;
	}

	virtual void addPreUIDrawables( MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext& frameContext )
	{
		if (mDebugTrace)
			printf(" %s : add pre-UI drawables\n", mName.asChar());

		drawManager.beginDrawable();

		drawManager.setColor( MColor( 0.1f, 0.5f, 0.95f ) );
		drawManager.setFontSize( MHWRender::MUIDrawManager::kSmallFontSize );
		drawManager.text( MPoint( -2, 2, -2 ), MString("Pre UI draw test in scene operation"), MHWRender::MUIDrawManager::kRight );
		drawManager.line( MPoint( -2, 0, -2 ), MPoint( -2, 2, -2 ) );
		drawManager.setColor( MColor( 1.0f, 1.0f, 1.0f ) );
		drawManager.sphere(  MPoint( -2, 2, -2 ), 0.8, false );
		drawManager.setColor( MColor( 0.1f, 0.5f, 0.95f, 0.4f ) );
		drawManager.sphere( MPoint( -2, 2, -2 ), 0.8, true );

		drawManager.endDrawable();
	}

	virtual void addPostUIDrawables( MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext& frameContext )
	{
		if (mDebugTrace)
			printf(" %s : add post-UI drawables\n", mName.asChar());

		drawManager.beginDrawable();

		drawManager.setColor( MColor( 0.05f, 0.95f, 0.34f ) );
		drawManager.setFontSize( MHWRender::MUIDrawManager::kSmallFontSize );
		drawManager.text( MPoint( 2, 2, 2 ), MString("Post UI draw test in scene operation"), MHWRender::MUIDrawManager::kLeft );
		drawManager.line( MPoint( 2, 0, 2), MPoint( 2, 2, 2 ) );
		drawManager.setColor( MColor( 1.0f, 1.0f, 1.0f ) );
		drawManager.sphere(  MPoint( 2, 2, 2 ), 0.8, false );
		drawManager.setColor( MColor( 0.05f, 0.95f, 0.34f, 0.4f ) );
		drawManager.sphere( MPoint( 2, 2, 2 ), 0.8, true );

		drawManager.endDrawable();
	}


	virtual const MFloatPoint *viewportRectangleOverride()
	{
		if (mOverrideViewRectangle)
		{
			if (mDebugTrace)
				printf("%s : override viewport rectangle\n", mName.asChar()); 

			// 1/4 to the right and 1/4 up. 3/4 of the target size.
			mViewRectangle[0] = 0.25f;
			mViewRectangle[1] = 0.25f;
			mViewRectangle[2] = 0.75f;
			mViewRectangle[3] = 0.75f;
			return &mViewRectangle;
		}
		return NULL;
	}

	// Options
	void setClearMask( unsigned int val )
	{
		mClearMask = val;
	}
	void setObjectSetOverride( const MString & val )
	{
		mSetName = val;
	}
	void setRenderFilterOverride(MHWRender::MSceneRender::MSceneFilterOption val)
	{
		mSceneFilterOperation = val;
	}
	void setShaderOverride(MHWRender::MShaderInstance* val)
	{
		mShaderInstance = val;
	}
	void setSceneUIDrawables(bool val)
	{
		mSceneUIDrawables = val;
	}
	void setOverrideViewRectangle(bool val)
	{
		mOverrideViewRectangle = val;
	}
	void setDebugTrace( bool val )
	{
		mDebugTrace = val;
	}

protected:
	unsigned int mClearMask;
	MString mSetName;
	MSelectionList mFilterSet;
	MHWRender::MSceneRender::MSceneFilterOption mSceneFilterOperation;
	MHWRender::MShaderInstance* mShaderInstance;
	MFloatPoint mViewRectangle;
	bool mSceneUIDrawables;
	bool mOverrideViewRectangle;
	bool mDebugTrace;
};

/*
	Class for testing user render operation options
*/
class MUserRenderOperationTester : public MHWRender::MUserRenderOperation
{
public:
	MUserRenderOperationTester(const MString &name)
	: MHWRender::MUserRenderOperation(name)
	, mUserUIDrawables(false)
	, mUserUILightData(false)
	, mOverrideViewRectangle(false)
	, mDebugTrace(false)
	{
	}

	virtual MStatus execute(const MHWRender::MDrawContext & drawContext )
	{
		if (mDebugTrace)
			printf("%s : call execute\n", mName.asChar());

		return MStatus::kSuccess;
	}

	virtual bool hasUIDrawables() const
	{
		return mUserUIDrawables;
	}
	
	virtual void addUIDrawables( MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext& frameContext )
	{
		if (mDebugTrace)
			printf("%s : add ui drawables\n", mName.asChar()); 

		drawManager.beginDrawable();

		drawManager.setColor( MColor( 0.95f, 0.5f, 0.1f ) );
		drawManager.setFontSize( MHWRender::MUIDrawManager::kSmallFontSize );
		drawManager.text( MPoint( 0, 2, 0 ), MString("UI draw test in user operation") );
		drawManager.line( MPoint( 0, 0, 0), MPoint( 0, 2, 0 ) );
		drawManager.setColor( MColor( 1.0f, 1.0f, 1.0f ) );
		drawManager.sphere(  MPoint( 0, 2, 0 ), 0.8, false );
		drawManager.setColor( MColor( 0.95f, 0.5f, 0.1f, 0.4f ) );
		drawManager.sphere(  MPoint( 0, 2, 0 ), 0.8, true );

		drawManager.endDrawable();
	}

	virtual const MFloatPoint *viewportRectangleOverride()
	{
		if (mOverrideViewRectangle)
		{
			if (mDebugTrace)
				printf("%s : override viewport rectangle\n", mName.asChar()); 

			// 1/4 to the right and 1/4 up. 3/4 of the target size.
			mViewRectangle[0] = 0.25f;
			mViewRectangle[1] = 0.25f;
			mViewRectangle[2] = 0.75f;
			mViewRectangle[3] = 0.75f;
			return &mViewRectangle;
		}
		return NULL;
	}

	virtual bool requiresLightData() const
	{
		return mUserUILightData;
	}

	// Options
	void setUserUIDrawables(bool val)
	{
		mUserUIDrawables = val;
	}
	void setUserUILightData(bool val)
	{
		mUserUILightData = val;
	}
	void setOverrideViewRectangle(bool val)
	{
		mOverrideViewRectangle = val;
	}
	void setDebugTrace( bool val )
	{
		mDebugTrace = val;
	}

protected:
	bool mUserUIDrawables;
	bool mUserUILightData;
	bool mOverrideViewRectangle;
	MFloatPoint mViewRectangle;
	bool mDebugTrace;
};

/*
	Class for testing HUD operation options
*/
class MHUDRenderTester : public MHWRender::MHUDRender
{
public:
	MHUDRenderTester(const MString &name)
	: MHWRender::MHUDRender()
	, mUserUIDrawables(false)
	, mOverrideViewRectangle(false)
	, mDebugTrace(false)
	, mName(name)
	{
	}

	virtual bool hasUIDrawables() const
	{
		return mUserUIDrawables;
	}

	virtual void addUIDrawables( MHWRender::MUIDrawManager& drawManager2D, const MHWRender::MFrameContext& frameContext )
	{
		if (mUserUIDrawables)
		{
			if (mDebugTrace)
				printf("%s : add ui drawables\n", mName.asChar()); 

			// Start draw UI
			drawManager2D.beginDrawable();
			// Set font color
			drawManager2D.setColor( MColor( 0.455f, 0.212f, 0.596f ) );
			// Set font size
			drawManager2D.setFontSize( MHWRender::MUIDrawManager::kSmallFontSize );

			// Draw renderer name
			int x=0, y=0, w=0, h=0;
			frameContext.getViewportDimensions( x, y, w, h );
			drawManager2D.text( MPoint(w*0.5f, h*0.91f), MString("Renderer Override Options Tester"), MHWRender::MUIDrawManager::kCenter );

			// Draw viewport information
			MString viewportInfoText( "Viewport information: x= " );
			viewportInfoText += x;
			viewportInfoText += ", y= ";
			viewportInfoText += y;
			viewportInfoText += ", w= ";
			viewportInfoText += w;
			viewportInfoText += ", h= ";
			viewportInfoText += h;
			drawManager2D.text( MPoint(w*0.5f, h*0.885f), viewportInfoText, MHWRender::MUIDrawManager::kCenter );

			// End draw UI
			drawManager2D.endDrawable();
		}
	}

	virtual const MFloatPoint *viewportRectangleOverride()
	{
		if (mOverrideViewRectangle)
		{
			if (mDebugTrace)
				printf("%s : override viewport rectangle\n", mName.asChar()); 

			// 1/4 to the right and 1/4 up. 3/4 of the target size.
			mViewRectangle[0] = 0.25f;
			mViewRectangle[1] = 0.25f;
			mViewRectangle[2] = 0.75f;
			mViewRectangle[3] = 0.75f;
			return &mViewRectangle;
		}
		return NULL;
	}

	// Options
	void setUserUIDrawables(bool val)
	{
		mUserUIDrawables = val;
	}
	void setOverrideViewRectangle(bool val)
	{
		mOverrideViewRectangle = val;
	}
	void setDebugTrace( bool val )
	{
		mDebugTrace = val;
	}

protected:
	bool mUserUIDrawables;
	bool mOverrideViewRectangle;
	MFloatPoint mViewRectangle;
	bool mDebugTrace;
	MString mName;
};

/*
	MRenderOverride test override which draws the following:

	1) Scene operation with it's options. See MSceneRenderTester class for available options.
	2) HUD operation
	3) Present operation with it's options. The only option is whether to blit back depth in
		addition to blitting color.
*/
class MRenderOverrideTester : public MHWRender::MRenderOverride
{
public:
	MRenderOverrideTester( const MString& name ) 
	: MHWRender::MRenderOverride( name ) 
	, mUIName("Render Override Options Renderer")
	, mOperation(0)
	, mUseSceneShaderInstance(false)
	, mSceneShaderInstance(NULL)
	, mSceneFilterOperation( MHWRender::MSceneRender::kNoSceneFilterOverride )
	, mSceneClearMask( (unsigned int)MHWRender::MClearOperation::kClearAll )
	, mSceneUIDrawables(false)
	, mUserUIDrawables(false)
	, mUserUILightData(false)
	, mPresentDepth(false)
	, mOverrideViewRectangle(false)
	, mDebugTrace(false)
	{
		const MString render1Name("Scene Render 1");
		const MString presentName("Present Target");
		const MString userOperationName("User Operation 1");
		const MString hudOperationName("HUD Operation");

		// Clear + render set 1
		mSceneRender1 = new MSceneRenderTester( render1Name );
		
		mUserOperation1 = new MUserRenderOperationTester( userOperationName );

		mHUDRender = new MHUDRenderTester( hudOperationName );	

		mPresentTarget = new MHWRender::MPresentTarget( presentName ); 
	}

	~MRenderOverrideTester()
	{
		MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
		const MHWRender::MShaderManager* shaderMgr = renderer ? renderer->getShaderManager() : NULL;
		if (shaderMgr)
		{
			if (mSceneShaderInstance)
			{
				shaderMgr->releaseShader(mSceneShaderInstance);
				mSceneShaderInstance = NULL;
			}
		}
		delete mSceneRender1; mSceneRender1 = NULL;
		delete mUserOperation1; mUserOperation1 = NULL;
		delete mHUDRender; mHUDRender = NULL;
		delete mPresentTarget; mPresentTarget = NULL;
	}

	virtual MHWRender::DrawAPI supportedDrawAPIs() const
	{
		// this plugin supports both GL and DX
		return (MHWRender::kOpenGL | MHWRender::kDirectX11);
	}

	virtual bool startOperationIterator()
	{
		mOperation = 0; 
		return true;
	}
		
	virtual MHWRender::MRenderOperation* renderOperation()
	{
		switch( mOperation )
		{
		case 0 : return mSceneRender1;
		case 1 : return mUserOperation1;
		case 2 : return mHUDRender;
		case 3 : return mPresentTarget;
		}
		return NULL;
	}

	virtual bool nextRenderOperation()
	{
		mOperation++; 
		return mOperation < 4 ? true : false;
	}
		
	/*
		UI name to appear as renderer 
	*/
	virtual MString uiName() const
	{
		return mUIName;
	}

	/*
		Setup will set up options on operations.
		Any resources which are required are also allocated at this point.
	*/
	virtual MStatus setup( const MString & destination )
	{
		if (mDebugTrace)
			printf("In setup of renderer: %s. Rendering to destination: %s\n", mName.asChar(), destination.asChar());

		MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();

		//
		// 1. Scene operation options:
		//
		// Shader override option. Acquire a stock shader and set it as an
		// override if enabled.
		//
		if (mSceneRender1)
		{
			const MHWRender::MShaderManager* shaderMgr = renderer ? renderer->getShaderManager() : NULL;
			if (shaderMgr && !mSceneShaderInstance)
			{
				mSceneShaderInstance = shaderMgr->getStockShader( MHWRender::MShaderManager::k3dBlinnShader );
				float diffuse[4] = { 1.0f, 0.0, 0.5f, 1.0f };
				mSceneShaderInstance->setParameter("diffuseColor", &diffuse[0]); 
			}

			if (mUseSceneShaderInstance && mSceneShaderInstance)
			{
				mSceneRender1->setShaderOverride(mSceneShaderInstance);
			}
			else
			{
				mSceneRender1->setShaderOverride(NULL);
			}
		
			// Set the clear mask
			mSceneRender1->setClearMask( mSceneClearMask );
			// Set scene filtering
			mSceneRender1->setRenderFilterOverride( mSceneFilterOperation );
			// Set object set filtering
			mSceneRender1->setObjectSetOverride( mSceneSetFilterName );
			// Set to use ui drawabls
			mSceneRender1->setSceneUIDrawables( mSceneUIDrawables );
			// Set viewport rectangle
			mSceneRender1->setOverrideViewRectangle( mOverrideViewRectangle );
			// Set debugging
			mSceneRender1->setDebugTrace( mDebugTrace );
		}

		// 
		// 2. User operation options:
		//
		if (mUserOperation1)
		{
			// Set to use ui drawables
			mUserOperation1->setUserUIDrawables( mUserUIDrawables );
			// Set to require light data
			mUserOperation1->setUserUILightData( mUserUILightData );
			// Set viewport rectangle
			mUserOperation1->setOverrideViewRectangle( mOverrideViewRectangle );
			// Set debugging
			mUserOperation1->setDebugTrace( mDebugTrace );
		}

		//
		// 3. HUD operation options:
		//
		if (mHUDRender)
		{
			// Set viewport rectangle
			mHUDRender->setOverrideViewRectangle( mOverrideViewRectangle );
			// Set to use ui drawables
			mHUDRender->setUserUIDrawables( mUserUIDrawables );
			// Set debugging
			mHUDRender->setDebugTrace( mDebugTrace );
		}

		// 
		// 4. Present operation options:
		//
		// Set depth target options
		mPresentTarget->setPresentDepth( mPresentDepth );

		return MStatus::kSuccess;
	}

	virtual MStatus cleanup()
	{
		if (mDebugTrace)
			printf("In cleanup %s\n", mName.asChar());
		return MStatus::kSuccess;
	}

	// Options
	void setUseSceneShaderInstance(bool val)
	{
		mUseSceneShaderInstance = val;
	}
	void setSceneFilterOperation(MHWRender::MSceneRender::MSceneFilterOption val)
	{
		mSceneFilterOperation = val;
	}
	void setSceneSetFilterName( const MString &val )
	{
		mSceneSetFilterName = val;
	}
	void setSceneUIDrawables( bool val )
	{
		mSceneUIDrawables = val;
	}
	void setPresentDepth( bool val )
	{
		mPresentDepth = val;
	}
	void setUserUIDrawables(bool val)
	{
		mUserUIDrawables = val;
	}
	void setUserUILightData(bool val)
	{
		mUserUILightData = val;
	}
	void setOverrideViewRectangle(bool val)
	{
		mOverrideViewRectangle = val;
	}
	void setDebugTrace( bool val )
	{
		mDebugTrace = val;
	}

protected:
	MSceneRenderTester*			mSceneRender1;
	MUserRenderOperationTester*	mUserOperation1;
	MHUDRenderTester*			mHUDRender;
	MHWRender::MPresentTarget*	mPresentTarget;
	int							mOperation;
	MString						mUIName;

	// Scene operation options
	bool mUseSceneShaderInstance;
	MHWRender::MShaderInstance* mSceneShaderInstance;
	unsigned int mSceneClearMask;
	MString mSceneSetFilterName;
	MHWRender::MSceneRender::MSceneFilterOption mSceneFilterOperation;
	bool mSceneUIDrawables;

	// User operation options
	bool mUserUIDrawables;
	bool mUserUILightData;

	bool mOverrideViewRectangle;

	// Present options
	bool mPresentDepth;

	// For execution tracing
	bool mDebugTrace;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Command to control render override options
//
// Syntax : 
// 
//	viewMRenderOverrideCmd 
//
//		-shaderOverride {on,off,0,1}		// Enable / disable using a shader override on surface objects
//		-objectSet <setName>				// Use named object set to filter what to draw during scene operation
//		-sceneFilter <#>,					// Scene elements filtering
//			where #=0 = MHWRender::MSceneRender::kNoSceneFilterOverride 
//					1 = MHWRender::MSceneRender::kRenderPreSceneUIItems		
//					2 = MHWRender::MSceneRender::kRenderOpaqueShadedItems			
//					3 = MHWRender::MSceneRender::kRenderTransparentShadedItems	
//					4 = MHWRender::MSceneRender::kRenderShadedItems						
//					5 = MHWRender::MSceneRender::kRenderPostSceneUIItems				
//					6 = MHWRender::MSceneRender::kRenderUIItems		
//		-sceneUIDrawables {on,off,0,1}		// Enable / disable user ui drawables for scene operation
//
//		-userUIDrawables {on,off,0,1}		// Enable / disable user ui drawables for user operation
//		-userUILightData {on,off,0,1}		// Enable / disable light data requirement for user operation
//		-presentDepth {on,off,0,1}			// Present depth when presenting color
//
//		-debugTrace {on,off,0,1}			// Output debug messages to stdout
// 
class viewMRenderOverrideCmd : public MPxCommand
{
public:
	viewMRenderOverrideCmd();
	virtual                 ~viewMRenderOverrideCmd(); 

	MStatus                 doIt( const MArgList& args );

	static MSyntax			newSyntax();
	static void*			creator();

private:
	MStatus                 parseArgs( const MArgList& args );

	bool mEnableShaderOverride;
	MString mObjectSetFilterName;
	int mSceneFilter;
	bool mSceneUIDrawables;
	bool mUserUIDrawables;
	bool mUserUILightData;
	bool mPresentDepth;
	bool mOverrideViewRectangle;
	bool mDebugTrace;

};

viewMRenderOverrideCmd::viewMRenderOverrideCmd()
: mEnableShaderOverride(false)
, mSceneFilter(0)
, mSceneUIDrawables(false)
, mUserUIDrawables(false)
, mUserUILightData(false)
, mPresentDepth(false)
, mOverrideViewRectangle(false)
, mDebugTrace(false)
{

}
viewMRenderOverrideCmd::~viewMRenderOverrideCmd()
{
}
void* viewMRenderOverrideCmd::creator()
{
	return (void *) (new viewMRenderOverrideCmd);
}

// Argument strings
const char *shaderOverrideShortName = "-so";
const char *shaderOverrideLongName = "-shaderOverride";
const char *objectSetFilterShortName = "-os";
const char *objectSetFilterLongName = "-objectSet";
const char *scenefilterShortName = "-sf";
const char *scenefilterLongName = "-sceneFilter";
const char *sceneUIDrawablesShortName = "-su";
const char *sceneUIDrawablesLongName = "-sceneUIDrawables";

const char *userUIDrawablesShortName = "-uu";
const char *userUIDrawablesLongName = "-userUIDrawables";
const char *userUILightDataShortName = "-ul";
const char *userUILightDataLongName = "-userUILightData";

const char *viewportRectangleShortName = "-vr";
const char *viewportRectangleLongName = "-viewportRectangle";

const char *presentDepthShortName = "-pd";
const char *presentDepthLongName = "-presentDepth";

const char *debugTraceShortName = "-db";
const char *debugTraceLongName = "-debug";

MSyntax viewMRenderOverrideCmd::newSyntax()
{
	MSyntax syntax;

	// Use shader override
	syntax.addFlag(shaderOverrideShortName, shaderOverrideLongName, MSyntax::kBoolean);

	// Object set filter name
	syntax.addFlag(objectSetFilterShortName, objectSetFilterLongName, MSyntax::kString);

	// Scene filter
	syntax.addFlag(scenefilterShortName, scenefilterLongName, MSyntax::kUnsigned);

	// Scene ui drawables
	syntax.addFlag(sceneUIDrawablesShortName, sceneUIDrawablesLongName, MSyntax::kBoolean);

	// User ui drawables
	syntax.addFlag(userUIDrawablesShortName, userUIDrawablesLongName, MSyntax::kBoolean);

	// User light data
	syntax.addFlag(userUILightDataShortName, userUILightDataLongName, MSyntax::kBoolean);

	// Present depth 
	syntax.addFlag(presentDepthShortName, presentDepthLongName, MSyntax::kBoolean);

	// Present depth 
	syntax.addFlag(viewportRectangleShortName, viewportRectangleLongName, MSyntax::kBoolean);

	// Debug output
	syntax.addFlag(debugTraceShortName, debugTraceLongName, MSyntax::kBoolean);

	return syntax;
}

// parseArgs
//
MStatus viewMRenderOverrideCmd::parseArgs(const MArgList& args)
{
	MStatus			status;
	MArgDatabase	argData(syntax(), args);

	mEnableShaderOverride = false;
	mObjectSetFilterName.clear();
	mSceneFilter = 0;
	mUserUIDrawables = false;
	mUserUILightData = false;
	mOverrideViewRectangle = false;
	mPresentDepth = false;

	MString     	arg;
	for ( unsigned int i = 0; i < args.length(); i++ ) 
	{
		arg = args.asString( i, &status );
		if (!status)              
			continue;

		// Check for shader override flag. 
		if ( arg == MString(shaderOverrideShortName) || arg == MString(shaderOverrideLongName) ) 
		{
			if (i == args.length()-1) {
				arg += ": must specify a value for ";
				arg += shaderOverrideLongName;
				displayError(arg);
				return MS::kFailure;
			}			
			i++;
			args.get(i, mEnableShaderOverride);
		}

		// Check for object filter flag
		//
		if ( arg == MString(objectSetFilterShortName) || arg == MString(objectSetFilterLongName) ) 
		{
			if (i == args.length()-1) {
				arg += ": must specify a value for ";
				arg += objectSetFilterLongName;
				displayError(arg);
				return MS::kFailure;
			}
			i++;
			args.get(i, mObjectSetFilterName);
		}

		// Check for scene filter flag
		//
		if ( arg == MString(scenefilterShortName) || arg == MString(scenefilterLongName) ) 
		{
			if (i == args.length()-1) {
				arg += ": must specify a value for ";
				arg += scenefilterLongName;
				displayError(arg);
				return MS::kFailure;
			}
			i++;
			args.get(i, mSceneFilter);
		}

		if ( arg == MString(sceneUIDrawablesShortName ) || arg == MString(sceneUIDrawablesLongName) ) 
		{
			if (i == args.length()-1) {
				arg += ": must specify a value for ";
				arg += sceneUIDrawablesLongName;
				displayError(arg);
				return MS::kFailure;
			}
			i++;
			args.get(i, mSceneUIDrawables);
		}

		if ( arg == MString(userUIDrawablesShortName) || arg == MString(userUIDrawablesLongName) ) 
		{
			if (i == args.length()-1) {
				arg += ": must specify a value for ";
				arg += userUIDrawablesLongName;
				displayError(arg);
				return MS::kFailure;
			}
			i++;
			args.get(i, mUserUIDrawables);
		}

		// User light data
		if ( arg == MString(userUILightDataShortName) || arg == MString(userUILightDataLongName) ) 
		{
			if (i == args.length()-1) {
				arg += ": must specify a value for ";
				arg += userUILightDataLongName;
				displayError(arg);
				return MS::kFailure;
			}
			i++;
			args.get(i, mUserUILightData);
		}


		// Viewport rectangle
		if ( arg == MString(viewportRectangleShortName) || arg == MString(viewportRectangleLongName) ) 
		{
			if (i == args.length()-1) {
				arg += ": must specify a value for ";
				arg += viewportRectangleLongName;
				displayError(arg);
				return MS::kFailure;
			}
			i++;
			args.get(i, mOverrideViewRectangle);
		}

		// Check for present depth flag
		//
		if ( arg == MString(presentDepthShortName) || arg == MString(presentDepthLongName) ) 
		{
			if (i == args.length()-1) {
				arg += ": must specify a value for ";
				arg += presentDepthLongName;
				displayError(arg);
				return MS::kFailure;
			}
			i++;
			args.get(i, mPresentDepth);
		}

		// Check for debug output flag
		//
		if ( arg == MString(debugTraceShortName) || arg == MString(debugTraceLongName) ) 
		{
			if (i == args.length()-1) {
				arg += ": must specify a value for ";
				arg += debugTraceLongName;
				displayError(arg);
				return MS::kFailure;
			}
			i++;
			args.get(i, mDebugTrace);
		}
	}
	return MS::kSuccess;
}

MStatus viewMRenderOverrideCmd::doIt(const MArgList& args)
{
	// Find the render override
    //
	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer(); 
	MRenderOverrideTester *renderOverride = NULL;
	if (renderer)
	{
		renderOverride = (MRenderOverrideTester *) renderer->findRenderOverride( "viewMRenderOverride" );
	}
	if (!renderOverride)
		return MStatus::kFailure;

	// Get command options
	//
	MStatus status = parseArgs(args);
	if (status != MStatus::kSuccess)
	{
		return status;
	}

	// Set options on scene operation
	//
	renderOverride->setUseSceneShaderInstance( mEnableShaderOverride );
	renderOverride->setSceneSetFilterName( mObjectSetFilterName );
	renderOverride->setSceneUIDrawables( mSceneUIDrawables );
	MHWRender::MSceneRender::MSceneFilterOption filters[7] = {
		MHWRender::MSceneRender::kNoSceneFilterOverride,
		MHWRender::MSceneRender::kRenderPreSceneUIItems,				
		MHWRender::MSceneRender::kRenderOpaqueShadedItems,			
		MHWRender::MSceneRender::kRenderTransparentShadedItems,	
		MHWRender::MSceneRender::kRenderShadedItems,						
		MHWRender::MSceneRender::kRenderPostSceneUIItems,		
		MHWRender::MSceneRender::kRenderUIItems				
	};
	if (mSceneFilter >=0 && mSceneFilter < 7)
	{
		renderOverride->setSceneFilterOperation( filters[mSceneFilter] );
	}

	// Set options on user operation
	//
	renderOverride->setUserUIDrawables( mUserUIDrawables );
	renderOverride->setUserUILightData( mUserUILightData );

	// Set options on present operation
	//
	renderOverride->setPresentDepth( mPresentDepth );

	renderOverride->setOverrideViewRectangle( mOverrideViewRectangle );

	renderOverride->setDebugTrace( mDebugTrace );

	// Cause a refresh to occur so that viewports will update
	MGlobal::executeCommandOnIdle("refresh");

	return status;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Plugin registration
//
MStatus 
initializePlugin( MObject obj )
{
    MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer(); 
	MStatus status = MStatus::kFailure;
	if (renderer)
	{
		status = renderer->registerOverride( new MRenderOverrideTester( "viewMRenderOverride" ));
	}

	if (status == MStatus::kSuccess)
	{
		MFnPlugin plugin(obj);
		status = plugin.registerCommand("viewMRenderOverride",
										viewMRenderOverrideCmd::creator,
										viewMRenderOverrideCmd::newSyntax);
	}
    return status;
}

//
// Plugin deregistration
//
MStatus 
uninitializePlugin( MObject obj )
{
    MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer(); 
	MStatus status = MStatus::kFailure;
	if (renderer)
	{
		status = renderer->deregisterOverride( renderer->findRenderOverride( "viewMRenderOverride" ));
	}

	MFnPlugin plugin(obj);
	status = plugin.deregisterCommand( "viewMRenderOverride" );

    return status;
}
