//
// Copyright (C) 2002-2004 NVIDIA
//
// File: pluginMain.cpp
//
// Author: Jim Atkinson
//
// Changes:
//  10/2003  Kurt Harriman - www.octopusgraphics.com +1-415-893-1023
//           - "-pluginPath/pp" flag of cgfxShader command returns the
//             full path of the "cgfxShader" subdirectory beneath the
//             directory from which the plug-in binary was loaded.
//             Supporting files such as MEL scripts can be loaded from
//             this directory to avoid inadvertently picking up wrong
//             versions from random directories on the search path.
//           - The plug-in executes the cgfxShader_initUI.mel script
//             from this directory at the time the plug-in is loaded.
//           - The MEL command `pluginInfo -q -version cgfxShader`
//             returns the plug-in version and cgfxShaderNode.cpp
//             compile date.
//  11/2003  Kurt Harriman - www.octopusgraphics.com +1-415-893-1023
//           - To load or reload a CgFX file, use the cgfxShader command
//             "-fx/fxFile <filename>" flag.  Setting the cgfxShader
//             node's "s/shader" attribute no longer loads the file.
//
//-
// ==========================================================================
// Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+
#include "cgfxShaderCommon.h"

#include "cgfxShaderNode.h"
#include "cgfxProfile.h"
#include "cgfxShaderCmd.h"
#include "cgfxTextureCache.h"
#include "cgfxVector.h"

#include <maya/MFnPlugin.h>
#include <maya/MIOStream.h>
#include <maya/MSceneMessage.h>
#include <maya/MNodeMessage.h>
#include <maya/MGlobal.h>
#include <maya/MFileIO.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MDGMessage.h>

#if defined(_SWATCH_RENDERING_SUPPORTED_)
	#include <maya/MHWShaderSwatchGenerator.h>
	#include <maya/MHardwareRenderer.h>
#endif


// viewport 2.0
#include <maya/MDrawRegistry.h>


// callbackIds is an array of callback identifiers which need to be
// cancelled when the plug-in is unloaded.
//
static MIntArray callbackIds;

static void cgfxShaderFileSaveCB(void* clientData );

MStatus initializePlugin( MObject obj )
//
//	Description:
//		this method is called when the plug-in is loaded into Maya.  It
//		registers all of the services that this plug-in provides with
//		Maya.
//
//	Arguments:
//		obj - a handle to the plug-in object (use MFnPlugin to access it)
//
{
	MString   sVer = cgfxShaderNode::getPluginVersion();

#if defined(_SWATCH_RENDERING_SUPPORTED_)
    	// Allow an environment variable to override usage of swatch rendering.
	// Set the environment variable to a value other than 0 for it to take effect.
	const char *cgfxEnvVar = getenv("CGFX_SWATCH_RENDERING");

	MString UserClassify = MString( "shader/surface/utility:drawdb/shader/surface/cgfxShader" );

	// Don't initialize swatches in batch mode
	if (MGlobal::mayaState() != MGlobal::kBatch)
	{
		const MString& swatchName =	MHWShaderSwatchGenerator::initialize();

#ifdef _WIN32
	    if (!cgfxEnvVar)
			UserClassify = MString( "shader/surface/utility/:drawdb/shader/surface/cgfxShader:swatch/"+swatchName );
	    else
	    {
	        if ( 0 != strcmp(cgfxEnvVar,"0") )
				UserClassify = MString( "shader/surface/utility/:drawdb/shader/surface/cgfxShader:swatch/"+swatchName );
	    }
#else
	    if ( cgfxEnvVar && ( 0 != strcmp(cgfxEnvVar,"0") ) )
	    {
			UserClassify = MString( "shader/surface/utility/:drawdb/shader/surface/cgfxShader:swatch/"+swatchName );
	    }
#endif
	}

#else
	const MString UserClassify( "shader/surface/utility:drawdb/shader/surface/cgfxShader" );
#endif

	MFnPlugin plugin( obj, "NVIDIA", sVer.asChar(), MApiVersion );

	// Register/initialize localized string resources
	CHECK_MSTATUS( plugin.registerUIStrings(NULL,
				   "cgfxShaderPluginInitStrings" ));

	// Create Cg Context & register the Cg error callback

#if defined(_SWATCH_RENDERING_SUPPORTED_)
	// The following code is only used on Maya versions 7.0 and
    // later.
	MStatus status = MStatus::kFailure;
#if !defined(LINUX)
	// Always initialize a context on Linux since it is not predictable as to when
	// we may get a valid context.
	if (MGlobal::mayaState() != MGlobal::kInteractive)
#endif
	{
		MHardwareRenderer *pRenderer = MHardwareRenderer::theRenderer();

		if (pRenderer) {
			const MString & backEndStr = pRenderer->backEndString();
			unsigned int width = 64, height = 64;
			status = pRenderer->makeSwatchContextCurrent( backEndStr,
				width,
				height );
		}
		if (status != MStatus::kSuccess) {
			MGlobal::displayError(MString("Unqualified video card : Offscreen contexts not supported. CgFx plugin will not be loaded."));
			return MStatus::kFailure;
		}
	}
#else
#error "CgFx requires the Maya version 7 or greater"
#endif

    cgfxTextureCache::initialize();
    
	cgfxShaderNode::sCgContext = cgCreateContext();
	cgSetErrorCallback(cgfxShaderNode::cgErrorCallBack);
	cgSetErrorHandler(cgfxShaderNode::cgErrorHandler, 0);
	cgGLRegisterStates(cgfxShaderNode::sCgContext);
	cgGLSetManageTextureParameters(cgfxShaderNode::sCgContext, CG_TRUE);

    cgSetAutoCompile(cgfxShaderNode::sCgContext, CG_COMPILE_LAZY);      
    cgSetLockingPolicy(CG_NO_LOCKS_POLICY);
    cgGLSetDebugMode(CG_FALSE);

    cgfxProfile::initialize();
    if (cgfxProfile::getBestProfile() == NULL) {
        MGlobal::displayError(
            MString("No supported Cg profiles were found. CgFx plugin will not be loaded.")
        );
        return MStatus::kFailure;
    }

    {
      // display banner
      MString s;
      s += sVer;
      MGlobal::displayInfo(s);
    }
    
    CHECK_MSTATUS( plugin.registerNode("cgfxShader",
                                       cgfxShaderNode::sId,
                                       cgfxShaderNode::creator,
                                       cgfxShaderNode::initialize,
                                       MPxNode::kHwShaderNode,
                                       &UserClassify));

	CHECK_MSTATUS( plugin.registerNode("cgfxVector",
		cgfxVector::sId,
		cgfxVector::creator,
		cgfxVector::initialize));

	CHECK_MSTATUS( plugin.registerCommand("cgfxShader",
		cgfxShaderCmd::creator,
		cgfxShaderCmd::newSyntax));

	// Register a shader override for this node
	CHECK_MSTATUS(MHWRender::MDrawRegistry::registerShaderOverrideCreator(
		cgfxShaderOverride::drawDbClassification,
		cgfxShaderOverride::drawRegistrantId,
		cgfxShaderOverride::Creator));

	// Where are my MEL scripts?
	cgfxShaderCmd::sPluginPath = plugin.loadPath();

	// Run MEL script for user interface initialization.
	if (MGlobal::mayaState() == MGlobal::kInteractive)
	{
		MString sCmd = "evalDeferred \"source \\\"cgfxShader_initUI.mel\\\"\"";
		MGlobal::executeCommand( sCmd );
	}

	// Skip the status checking on the addCallback calls since the only
	// way that they can really fail is if Maya is out of memory and then
	// everything is going to fall apart anyway.
	//
	// call backs for "source directory" removal...
	MCallbackId id = MSceneMessage::addCallback(MSceneMessage::kBeforeSave,
		cgfxShaderFileSaveCB, NULL, &status);
	CHECK_MSTATUS(status);
	callbackIds.append((int)id);

	id = MSceneMessage::addCallback(MSceneMessage::kBeforeExport,
		cgfxShaderFileSaveCB, NULL, &status);
	CHECK_MSTATUS(status);
	callbackIds.append((int)id);

	return MStatus::kSuccess;
}

MStatus uninitializePlugin( MObject obj)
//
//	Description:
//		this method is called when the plug-in is unloaded from Maya. It
//		deregisters all of the services that it was providing.
//
//	Arguments:
//		obj - a handle to the plug-in object (use MFnPlugin to access it)
//
{
	MStatus   status;
	MFnPlugin plugin( obj );

	cgDestroyContext(cgfxShaderNode::sCgContext);
    cgfxProfile::uninitialize();
    cgfxTextureCache::uninitialize();
    
	// Remove all the callbacks that we registered.
	//
	MMessage::removeCallbacks(callbackIds);

	// Deregister our node types.
	//
	CHECK_MSTATUS(plugin.deregisterNode( cgfxShaderNode::sId ));
	CHECK_MSTATUS(plugin.deregisterNode( cgfxVector::sId ));

	// Deregister our commands.
	//
	CHECK_MSTATUS(plugin.deregisterCommand( "cgfxShader" ));

	// Deregister the override
	CHECK_MSTATUS(MHWRender::MDrawRegistry::deregisterShaderOverrideCreator(
		cgfxShaderOverride::drawDbClassification, cgfxShaderOverride::drawRegistrantId));

	return MStatus::kSuccess;
}

static void cgfxShaderFileSaveCB(void* clientData )
{
	// Look through the scene for cgfxShader nodes whose effect is NULL
	// but whose shader attribute is not empty.
	//
	MStatus status;

	MString workspace;
	status = MGlobal::executeCommand(MString("workspace -q -rd;"),
		workspace);
	if (!status)
	{
		workspace.clear();
	}

	MItDependencyNodes nodeIt;

	for (nodeIt.reset(MFn::kPluginHwShaderNode);
		!nodeIt.isDone();
		nodeIt.next())
	{
		MObject oNode = nodeIt.item();

		MFnDependencyNode fnNode(oNode);
		if (fnNode.typeId() == cgfxShaderNode::sId)
		{
			// We've got a winner.
			//
			cgfxShaderNode* pNode = (cgfxShaderNode*)fnNode.userNode();
			MString ShaderFxFile = pNode->shaderFxFile();
			if (strncmp(ShaderFxFile.asChar(),workspace.asChar(),workspace.length()) == 0)
			{
				ShaderFxFile = ShaderFxFile.substring(workspace.length(),ShaderFxFile.length() - 1);
				MPlug plShader = fnNode.findPlug( pNode->sShader );
				plShader.setValue( ShaderFxFile );
				OutputDebugString("CGFX shader pathname saved as: ");
				OutputDebugString(ShaderFxFile.asChar());
				OutputDebugString("\n");
			}

			if( pNode->getTexturesByName())
			{
				cgfxAttrDefList::iterator it(pNode->attrDefList());

				while (it)
				{
					cgfxAttrDef* aDef = (*it);

					MObject oNode = pNode->thisMObject();

					switch (aDef->fType)
					{
					case cgfxAttrDef::kAttrTypeColor1DTexture:
					case cgfxAttrDef::kAttrTypeColor2DTexture:
					case cgfxAttrDef::kAttrTypeColor3DTexture:
					case cgfxAttrDef::kAttrTypeColor2DRectTexture:
					case cgfxAttrDef::kAttrTypeNormalTexture:
					case cgfxAttrDef::kAttrTypeBumpTexture:
					case cgfxAttrDef::kAttrTypeCubeTexture:
					case cgfxAttrDef::kAttrTypeEnvTexture:
					case cgfxAttrDef::kAttrTypeNormalizationTexture:
						{
							MString pathname;
							aDef->getValue(oNode, pathname);
							if (strncmp(pathname.asChar(),workspace.asChar(),workspace.length()) == 0)
							{
								pathname = pathname.substring(workspace.length(),pathname.length());
								aDef->setValue(oNode, pathname);
								OutputDebugString("CGFX texture pathname saved as: ");
								OutputDebugString(pathname.asChar());
								OutputDebugString("\n");
							}
						}
						break;
					default:
						break;
					}
					++it;
				}
			}
		}
	}
}

