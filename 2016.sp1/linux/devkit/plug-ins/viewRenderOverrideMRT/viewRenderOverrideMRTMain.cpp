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

#include "viewRenderOverrideMRT.h"

static viewRenderOverrideMRT* viewRenderOverrideMRTInstance = NULL;

MStatus initializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "1.0", "Any");

	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (renderer)
	{
		if (!viewRenderOverrideMRTInstance)
		{
			viewRenderOverrideMRTInstance = new viewRenderOverrideMRT("my_viewRenderOverrideMRT");
			renderer->registerOverride(viewRenderOverrideMRTInstance);
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
		if (viewRenderOverrideMRTInstance)
		{
			renderer->deregisterOverride(viewRenderOverrideMRTInstance);
			delete viewRenderOverrideMRTInstance;
		}
		viewRenderOverrideMRTInstance = NULL;
	}

	return status;
}

