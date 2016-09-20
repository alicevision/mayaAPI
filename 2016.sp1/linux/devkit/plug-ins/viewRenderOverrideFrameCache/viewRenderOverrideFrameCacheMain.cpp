//-
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+

#include <stdio.h>

#include <maya/MString.h>
#include <maya/MFnPlugin.h>
#include <maya/MViewport2Renderer.h>

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/M3dView.h>

#include "viewRenderOverrideFrameCache.h"

static viewRenderOverrideFrameCache* viewRenderOverrideFrameCacheInstance = NULL;

// Command to control frame caching
//
class viewFrameCache : public MPxCommand
{
public:
	viewFrameCache();
	virtual                 ~viewFrameCache(); 

	MStatus                 doIt( const MArgList& args );

	static MSyntax			newSyntax();
	static void*			creator();

private:
	MStatus                 parseArgs( const MArgList& args );

	MString					mPanelName;		// Name of panel to capture
	bool					mAllowCapture;	// Turn capture on / off
	bool					mResetCapture;	// Reset cache
	bool					mCaptureToDisk; // Write capture to disk
};

viewFrameCache::viewFrameCache()
: mAllowCapture(false)
, mResetCapture(false)
, mCaptureToDisk(false)
{
}
viewFrameCache::~viewFrameCache()
{
}
void* viewFrameCache::creator()
{
	return (void *) (new viewFrameCache);
}

// 
// Syntax : viewFrameCache -capture {on,off,0,1} -todisk {on,off,0,1} -reset <modelPanelName>;
// 
const char *captureShortName = "-ca";
const char *captureLongName = "-capture";
const char *toDiskShortName = "-td";
const char *toDiskLongName = "-todisk";
const char *resetShortName = "-r";
const char *resetLongName = "-reset";
MSyntax viewFrameCache::newSyntax()
{
	MSyntax syntax;

	syntax.addFlag(captureShortName, captureLongName, MSyntax::kBoolean);
	syntax.addFlag(toDiskShortName, toDiskLongName, MSyntax::kBoolean);
	syntax.addFlag(resetShortName, resetLongName, MSyntax::kNoArg);

	// Name of model panel affected
	//
	syntax.addArg(MSyntax::kString);

	return syntax;
}

// parseArgs
//
MStatus viewFrameCache::parseArgs(const MArgList& args)
{
	MStatus			status;
	MArgDatabase	argData(syntax(), args);

	mAllowCapture = false;
	mResetCapture = false;
	mCaptureToDisk = false;

	MString     	arg;
	for ( unsigned int i = 0; i < args.length(); i++ ) 
	{
		arg = args.asString( i, &status );
		if (!status)              
			continue;

		// Check for reset flag. 
		if ( arg == MString(resetShortName) || arg == MString(resetLongName) ) 
		{
			mResetCapture = true;
		}

		// Check for capture flag
		//
		if ( arg == MString(captureShortName) || arg == MString(captureLongName) ) 
		{
			if (i == args.length()-1) {
				arg += ": must specify a value for -capture.";
				displayError(arg);
				return MS::kFailure;
			}
			i++;
			args.get(i, mAllowCapture);
		}

		// Check for "todisk" flag
		//
		if ( arg == MString(toDiskShortName) || arg == MString(toDiskLongName) ) 
		{
			if (i == args.length()-1) {
				arg += ": must specify a value for -todisk.";
				displayError(arg);
				return MS::kFailure;
			}
			i++;
			args.get(i, mCaptureToDisk);
		}

	}

	// Read off the panel name
	status = argData.getCommandArgument(0, mPanelName);
	if (status != MStatus::kSuccess)
	{
		status.perror("No panel name specified as command argument");
		return status;
	}
	return MS::kSuccess;
}

MStatus viewFrameCache::doIt(const MArgList& args)
{
	if (!viewRenderOverrideFrameCacheInstance)
		return MStatus::kFailure;

	MStatus status;

	status = parseArgs(args);
	if (!status)
	{
		return status;
	}

	// Set the panel to use the viewport renderer if capture is on.
	M3dView view;
	status = M3dView::getM3dViewFromModelPanel(mPanelName, view);
	if (status == MStatus::kSuccess)
	{
		view.setRenderOverrideName("viewRenderOverrideFrameCache");
	}
	if (mResetCapture)
	{
		viewRenderOverrideFrameCacheInstance->releaseCachedTextures();
		mAllowCapture = false;
	}
	viewRenderOverrideFrameCacheInstance->setAllowCaching( mAllowCapture );
	viewRenderOverrideFrameCacheInstance->setCacheToDisk( mAllowCapture ? mCaptureToDisk  : false );

	return status;
}

//////////////////////////////////////////////////////////////////////////////////

MStatus initializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "1.0", "Any");

	// Register command
	status = plugin.registerCommand("viewFrameCache",
									viewFrameCache::creator,
									viewFrameCache::newSyntax);
	if (!status)
	{
		status.perror("registerCommand");
	}

	// Register override
	if (status == MStatus::kSuccess)
	{
		MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
		if (renderer)
		{
			if (!viewRenderOverrideFrameCacheInstance)
			{
				viewRenderOverrideFrameCacheInstance = new viewRenderOverrideFrameCache("viewRenderOverrideFrameCache");
				renderer->registerOverride(viewRenderOverrideFrameCacheInstance);
			}
		}
	}
	return status;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj);

	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (renderer)
	{
		if (viewRenderOverrideFrameCacheInstance)
		{
			renderer->deregisterOverride(viewRenderOverrideFrameCacheInstance);
			delete viewRenderOverrideFrameCacheInstance;
		}
		viewRenderOverrideFrameCacheInstance = NULL;
	}

	status = plugin.deregisterCommand( "viewFrameCache" );
	if (!status) {
		status.perror("deregisterCommand");
		return status;
	}

	return status;
}

