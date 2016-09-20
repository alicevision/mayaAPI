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
#include "viewOverrideSimple.h"

//
// On plug-in initialization we register a new override
//
MStatus initializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "1.0", "Any");

	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (renderer)
	{
		// We register with a given name
		viewOverrideSimple *overridePtr = new viewOverrideSimple("viewOverrideSimple");
		if (overridePtr)
		{
			renderer->registerOverride(overridePtr);
		}
	}
	return status;
}

//
// On plug-in de-initialization we deregister a new override
//
MStatus uninitializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj);

	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (renderer)
	{
		// Find override with the given name and deregister
		const MHWRender::MRenderOverride* overridePtr = renderer->findRenderOverride("viewOverrideSimple");
		if (overridePtr)
		{
			renderer->deregisterOverride( overridePtr );
			delete overridePtr;
		}
	}

	return status;
}
