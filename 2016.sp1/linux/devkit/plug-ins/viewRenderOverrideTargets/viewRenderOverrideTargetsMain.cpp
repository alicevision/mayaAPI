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

#include "viewRenderOverrideTargets.h"

static viewRenderOverrideTargets* viewRenderOverrideTargetsInstance = NULL;

MStatus initializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "1.0", "Any");

	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (renderer)
	{
		if (!viewRenderOverrideTargetsInstance)
		{
			viewRenderOverrideTargetsInstance = new viewRenderOverrideTargets("my_viewRenderOverrideTargets");
			renderer->registerOverride(viewRenderOverrideTargetsInstance);
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
		if (viewRenderOverrideTargetsInstance)
		{
			renderer->deregisterOverride(viewRenderOverrideTargetsInstance);
			delete viewRenderOverrideTargetsInstance;
		}
		viewRenderOverrideTargetsInstance = NULL;
	}

	return status;
}

