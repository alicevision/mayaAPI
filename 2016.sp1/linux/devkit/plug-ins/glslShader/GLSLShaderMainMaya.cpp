//-
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+

#include <maya/MFnPlugin.h>
#include <maya/MDGMessage.h>
#include <maya/MDrawRegistry.h>
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>

#include <maya/MHWShaderSwatchGenerator.h>

// viewport 2.0
#include <maya/MDrawRegistry.h>

#include "GLSLShaderStrings.h"
#include "GLSLShaderCmd.h"

#include "GLSLShader.h"
#include "GLSLShaderOverride.h"

#include "crackFreePrimitiveGenerator.h"



// Plugin configuration
//
MStatus initializePlugin( MObject obj )
{
	MStatus status;
	MFnPlugin plugin(obj, "Autodesk", "1.0", "Any");

	MString UserClassify = MString("shader/surface/utility:");
	UserClassify += GLSLShaderNode::m_drawDbClassification;

	// Register string resources
	//
	CHECK_MSTATUS( plugin.registerUIStrings( glslShaderStrings::registerMStringResources, "GLSLShaderInitStrings" ) );

	CHECK_MSTATUS( plugin.registerCommand("GLSLShader",
		GLSLShaderCmd::creator,
		GLSLShaderCmd::newSyntax));

	// Run MEL script for user interface initialization.
	if (MGlobal::mayaState() == MGlobal::kInteractive)
	{
		MString sCmd = "evalDeferred \"source \\\"GLSLShader_initUI.mel\\\"\"";
		MGlobal::executeCommand( sCmd );
	}

	// Don't initialize swatches in batch mode
	if (MGlobal::mayaState() != MGlobal::kBatch)
	{
		static MString swatchName = MHWShaderSwatchGenerator::initialize();
		UserClassify = MString( "shader/surface/utility/:drawdb/shader/surface/GLSLShader:swatch/"+swatchName );
	}
	
	// Register node
	status = plugin.registerNode(
		GLSLShaderNode::m_TypeName,
		GLSLShaderNode::m_TypeId,
		GLSLShaderNode::creator,
		GLSLShaderNode::initialize,
		MPxNode::kHardwareShader,
		&UserClassify);

	if (status != MS::kSuccess)
	{
		status.perror("registerNode");
		return status;
	}


	// Register a shader override for this node
	status = MHWRender::MDrawRegistry::registerShaderOverrideCreator(
		GLSLShaderNode::m_drawDbClassification,
		GLSLShaderNode::m_RegistrantId,
		GLSLShaderOverride::Creator);

	if (status != MS::kSuccess)
	{
		status.perror("registerShaderOverrideCreator");
		return status;
	}
	

	// Register the vertex mutators with Maya
	//
	CHECK_MSTATUS( MHWRender::MDrawRegistry::registerIndexBufferMutator("GLSL_PNAEN18", CrackFreePrimitiveGenerator::createCrackFreePrimitiveGenerator18));
	CHECK_MSTATUS( MHWRender::MDrawRegistry::registerIndexBufferMutator("GLSL_PNAEN9", CrackFreePrimitiveGenerator::createCrackFreePrimitiveGenerator9));
		
	// Add and manage default plugin user pref:
	MGlobal::executeCommandOnIdle("GLSLShaderCreateUI");

	// Register GLSLShader to filePathEditor
	status = MGlobal::executeCommand("filePathEditor -registerType \"GLSLShader.shader\" -typeLabel \"GLSLShader\" -temporary");
	if (!status) {
		MString nodeAttr("GLSLShader.shader");
		MString errorString = glslShaderStrings::getString( glslShaderStrings::kErrorRegisterNodeType, nodeAttr );
		MGlobal::displayWarning( errorString );
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MFnPlugin plugin(obj);
	MStatus status;

	status = MHWRender::MDrawRegistry::deregisterShaderOverrideCreator(
		GLSLShaderNode::m_drawDbClassification,
		GLSLShaderNode::m_RegistrantId);
	if (status != MS::kSuccess)
	{
		status.perror("deregisterShaderOverrideCreator");
		return status;
	}

	// Deregister the vertex mutators
	//
	MHWRender::MDrawRegistry::deregisterIndexBufferMutator("GLSL_PNAEN18");
	MHWRender::MDrawRegistry::deregisterIndexBufferMutator("GLSL_PNAEN9");

	status = plugin.deregisterNode(GLSLShaderNode::m_TypeId);
	if (status != MS::kSuccess)
	{
		status.perror("deregisterNode");
		return status;
	}

	CHECK_MSTATUS( plugin.deregisterCommand("GLSLShader") );

	status = MGlobal::executeCommand("filePathEditor -deregisterType \"GLSLShader.shader\" -temporary");

	return status;
}
