//-
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+

#include <stdio.h>
#include <exception>

#include <maya/MString.h>
#include <maya/MFnPlugin.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MCommandResult.h>
#include <maya/MGlobal.h>

#include "viewImageBlitOverride.h"


MStatus initializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "1.0", "Any");

	// ************************ MAYA-25818 PART 1 of 2 *************************
	// Workaround for avoiding dirtying the scene until there's a way to
	// register overrides without causing dirty.
	bool sceneDirty = true; // assume true since that's the safest
	try
	{
		// is the scene currently dirty?
		MCommandResult sceneDirtyResult(&status);
		if (status != MStatus::kSuccess) throw std::exception();
		status = MGlobal::executeCommand("file -query -modified", sceneDirtyResult);
		if (status != MStatus::kSuccess) throw std::exception();
		int commandResult;
		status = sceneDirtyResult.getResult(commandResult);
		if (status != MStatus::kSuccess) throw std::exception();
		sceneDirty = commandResult != 0;
	}
	catch (std::exception&)
	{
		// if we got here, assume the scene is dirty
		sceneDirty = true;
	}
	// ************************ MAYA-25818 PART 1 of 2 *********************

	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (renderer)
	{
		if (!viewImageBlitOverride::RenderOverride::sViewImageBlitOverrideInstance)
		{
			viewImageBlitOverride::RenderOverride::sViewImageBlitOverrideInstance = new viewImageBlitOverride::RenderOverride("my_viewImageBlitOverride");
			renderer->registerOverride(viewImageBlitOverride::RenderOverride::sViewImageBlitOverrideInstance);
		}
	}

	// ************************ MAYA-25818 PART 2 of 2 *************************
	// If the scene was previously unmodified, return it to that state since
	// we haven't done anything that needs to be saved.
	if (sceneDirty == false)
	{
		MGlobal::executeCommand("file -modified 0");
	}
	// ************************ END MAYA-25818 PART 2 of 2 *********************

	return status;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj);

	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (renderer)
	{
		if (viewImageBlitOverride::RenderOverride::sViewImageBlitOverrideInstance)
		{
			renderer->deregisterOverride(viewImageBlitOverride::RenderOverride::sViewImageBlitOverrideInstance);
			delete viewImageBlitOverride::RenderOverride::sViewImageBlitOverrideInstance;
		}
		viewImageBlitOverride::RenderOverride::sViewImageBlitOverrideInstance = NULL;
	}

	return status;
}

