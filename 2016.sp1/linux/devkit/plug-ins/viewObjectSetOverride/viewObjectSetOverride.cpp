//-
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+
#include <maya/MViewport2Renderer.h>
#include <maya/MSelectionList.h>
#include <maya/MFnSet.h>

namespace MHWRender
{
	//
	// Class which filters what to render in a scene draw by
	// returning the objects in a named set as the object set filter.
	//
	// Has the option of what to do as the clear operation.
	// Usage can be to clear on first set draw, and and not clear
	// on subsequent draws.
	//
	class ObjectSetSceneRender : public MSceneRender
	{
	public:
		ObjectSetSceneRender( const MString& name, const MString setName, unsigned int clearMask ) 
		: MSceneRender( name )
		, mSetName( setName ) 
		, mClearMask( clearMask )
		{}

		// Return filtered list of items to draw
		virtual const MSelectionList* objectSetOverride()
		{
			MSelectionList list; 
			list.add( mSetName );
			
			MObject obj; 
			list.getDependNode( 0, obj );
			
			MFnSet set( obj ); 
			set.getMembers( mFilterSet, true );
			
			return &mFilterSet;
		}

		// Return clear operation to perform
		virtual MHWRender::MClearOperation & clearOperation()
		{
			mClearOperation.setMask( mClearMask );
			return mClearOperation;
		}

	protected:
		MSelectionList mFilterSet;
		MString mSetName;
		unsigned int mClearMask;

	};

	//
	// Render override which draws 2 sets of objects in multiple "passes" (multiple scene renders)
	// by using a filtered draw for each pass.
	//
	class viewObjectSetOverride : public MRenderOverride
	{
	public:
		viewObjectSetOverride( const MString& name ) 
		: MHWRender::MRenderOverride( name ) 
		, mUIName("Multi-pass filtered object-set renderer")
		, mOperation(0)
		{
			const MString render1Name("Render Set 1");
			const MString render2Name("Render Set 2");
			const MString set1Name("set1");
			const MString set2Name("set2");
			const MString presentName("Present Target");

			// Clear + render set 1
			mRenderSet1 = new ObjectSetSceneRender( render1Name, set1Name,  (unsigned int)MHWRender::MClearOperation::kClearAll );
			// Don't clear and render set 2
			mRenderSet2 = new ObjectSetSceneRender( render2Name, set2Name,  (unsigned int)MHWRender::MClearOperation::kClearNone );
			// Present results
			mPresentTarget = new MPresentTarget( presentName ); 
		}

		~viewObjectSetOverride()
		{
			delete mRenderSet1; mRenderSet1 = NULL;
			delete mRenderSet2; mRenderSet2 = NULL;
			delete mPresentTarget; mPresentTarget = NULL;
		}

		virtual MHWRender::DrawAPI supportedDrawAPIs() const
		{
			// this plugin supports both GL and DX
			return (MHWRender::kOpenGL | MHWRender::kOpenGLCoreProfile | MHWRender::kDirectX11);
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
			case 0 : return mRenderSet1;
			case 1 : return mRenderSet2;
			case 2 : return mPresentTarget;
			}
			return NULL;
		}

		virtual bool nextRenderOperation()
		{
			mOperation++; 
			return mOperation < 3 ? true : false;
		}
		
		// UI name to appear as renderer 
		virtual MString uiName() const
		{
			return mUIName;
		}

	protected:
		ObjectSetSceneRender*   mRenderSet1;
		ObjectSetSceneRender*   mRenderSet2;
		MPresentTarget*			mPresentTarget;
		int						mOperation;
		MString					mUIName;
	};
}

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
		status = renderer->registerOverride( new MHWRender::viewObjectSetOverride( "viewObjectSetOverride" ));
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
		status = renderer->deregisterOverride( renderer->findRenderOverride( "viewObjectSetOverride" ));
	}
    return status;
}
