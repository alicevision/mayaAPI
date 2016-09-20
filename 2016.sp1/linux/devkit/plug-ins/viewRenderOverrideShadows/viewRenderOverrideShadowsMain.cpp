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

#include "viewRenderOverrideShadows.h"

static viewRenderOverrideShadows* viewRenderOverrideShadowsInstance = NULL;

MStatus initializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "1.0", "Any");

	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (renderer)
	{
		if (!viewRenderOverrideShadowsInstance)
		{
			viewRenderOverrideShadowsInstance = new viewRenderOverrideShadows("my_viewRenderOverrideShadows");
			renderer->registerOverride(viewRenderOverrideShadowsInstance);
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
		if (viewRenderOverrideShadowsInstance)
		{
			renderer->deregisterOverride(viewRenderOverrideShadowsInstance);
			delete viewRenderOverrideShadowsInstance;
		}
		viewRenderOverrideShadowsInstance = NULL;
	}

	return status;
}

