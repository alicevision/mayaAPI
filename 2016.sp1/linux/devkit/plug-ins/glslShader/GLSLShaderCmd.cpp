// ==========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

#if _MSC_VER >= 1700
#pragma warning( disable: 4005 )
#endif

#include "GLSLShaderCmd.h"
#include "GLSLShader.h"
#include "GLSLShaderStrings.h"
#include <maya/MGlobal.h>
#include <maya/MArgDatabase.h>
#include <maya/MCommandResult.h>
#include <maya/MDagPath.h>
#include <maya/MFeedbackLine.h>
#include <maya/MFnDagNode.h>
#include <maya/MSyntax.h>
#include <maya/MArgParser.h>
#include <maya/MSelectionList.h>


// Forces all GLSLShader nodes that share the same effect name to reload the effect.
#define kReloadFlag								"-r"
#define kReloadFlagLong							"-reload"


// Retrives the effect file name. Functionnally equivalent to "getAttr GLSLShader1.shader"
//
//  example:
//		GLSLShader GLSLShader1 -q -fx;
//		Result: MayaUberShader.fxo // 
#define kFXFileFlag								"-fx"
#define kFXFileFlagLong							"-fxFile"

// Retrieves a string array containing all techniques supported by the shader
//
//  example:
//		GLSLShader GLSLShader1 -q -lt;
//		Result: TessellationOFF TessellationON WireFrame // 
#define kListTechniquesFlag						"-lt"
#define kListTechniquesFlagLong					"-listTechniques"

// Clears all parameters of the GLSLShader node
#define kClearParametersFlag					"-c"
#define kClearParametersFlagLong				"-clearParameters"

// Returns a string array containing a pair (Light Group Name, Light Group Type) for
// all logical light groups found in the effect
//
//  example:
//		GLSLShader GLSLShader1 -q -li;
//		Result: Light 0 undefined Light 1 undefined Light 2 undefined // 
#define kListLightInformationFlag				"-li"
#define kListLightInformationFlagLong			"-listLightInformation"

// Lists all the parameter names that are member of a logical light. The
// light group name must be provided:
//
//  example:
//		GLSLShader GLSLShader1 -lp "Light 0";
//		Result: Enable_Light_0 Light_0_Position Light_0_Color Light_0_Intensity... 
#define kListLightParametersFlag				"-lp"
#define kListLightParametersFlagLong			"-listLightParameters"

// Used together with the -listLightParameters flag, this returns the semantics
// of each light parameter:
//
//  example:
//		GLSLShader GLSLShader1 -lp "Light 0" -sem;
//		Result: Enable_Light_0 LightEnable Light_0_Position Position Light_0_Color LightColor...
#define kListLightParameterSemanticsFlag		"-sem"
#define kListLightParameterSemanticsFlagLong	"-semantics"

// List all UI groups names:
//
//  example:
//		GLSLShader GLSLShader1 -q -lg;
//		Result: Lighting Light 0 Light 1 Light 2 Ambient and Emissive Diffuse Opacity...
#define kListUIGroupInformationFlag				"-lg"
#define kListUIGroupInformationFlagLong			"-listUIGroupInformation"

// Lists all the parameter names that are member of a UI group. The
// group name must be provided:
//
//  example:
//		GLSLShader GLSLShader1 -lgp "Diffuse";
//		Result: Diffuse_Map Diffuse_Map_Alpha Diffuse_Map_1 Diffuse_Color Lightmap_Map...
#define kListUIGroupParametersFlag				"-lgp"
#define kListUIGroupParametersFlagLong			"-listUIGroupParameters"

// Connects a scene light to a logical light:
//
//  example:
//		GLSLShader GLSLShader1 -cl Light_0 pointLight1;
#define kConnectLightFlag						"-cl"
#define kConnectLightFlagLong					"-connectLight"

// Returns the scene light that is currently driving a light group:
//
//  example:
//		GLSLShader GLSLShader1 -lcs Light_0;
//		Result: pointLight1
//		GLSLShader GLSLShader1 -lcs Light_1;
//		<No result>
#define kLightConnectionStatusFlag				"-lcs"
#define kLightConnectionStatusFlagLong			"-lightConnectionStatus"

// Explicitly disconnect a scene light from a light group. This will put the
// light into the "Use Shader Settings" mode. To go back to "Automatic Bind",
// you must also set the value of the implicit bind attribute:
//
//  example:
//		GLSLShader GLSLShader1 -d Light_0;
//		setAttr GLSLShader1.Light_0_use_implicit_lighting 1;
#define kDisconnectLightFlag					"-d"
#define kDisconnectLightFlagLong				"-disconnectLight"


GLSLShaderCmd::GLSLShaderCmd()
{}

GLSLShaderCmd::~GLSLShaderCmd()
{}

MStatus GLSLShaderCmd::doIt( const MArgList& args )
{
	MStatus status;

	// Parse the shader node
	//
	MArgParser parser( syntax(), args );
	MString nodeName;
	parser.getCommandArgument( 0, nodeName );


	MSelectionList list;
	status = list.add( nodeName );
	MObject shaderNode;
	status = list.getDependNode( 0, shaderNode );
	if( status != MS::kSuccess )
	{
		MStringArray args;
		args.append(nodeName);

		MString msg = glslShaderStrings::getString( glslShaderStrings::kInvalidGLSLShader, args );
		displayError( msg );
		return MS::kFailure;
	}

	MArgDatabase argData( syntax(), args, &status );
	if ( !status )
		return status;

	bool fIsQuery = argData.isQuery();

	MFnDependencyNode depFn( shaderNode );
	if( depFn.typeId() != GLSLShaderNode::TypeID() )
	{
		MString msg = glslShaderStrings::getString( glslShaderStrings::kInvalidGLSLShader, nodeName );
		displayError( msg );
		return MS::kFailure;
	}

	GLSLShaderNode* shader = (GLSLShaderNode*) depFn.userNode();
	if( !shader )
	{
		MString msg = glslShaderStrings::getString( glslShaderStrings::kInvalidGLSLShader, nodeName );
		displayError( msg );
		return MS::kFailure;
	}

	if ( fIsQuery ) 
	{
		if( parser.isFlagSet(kFXFileFlag) )
		{
			MString path = shader->effectName();
			setResult( path );
			return MS::kSuccess;
		}
		else if( parser.isFlagSet( kListTechniquesFlag ) )
		{
			setResult( shader->techniqueNames() );
			return MS::kSuccess;
		}
		else if( parser.isFlagSet( kListLightInformationFlag ) )
		{
			setResult( shader->lightInfoDescription() );
			return MS::kSuccess;
		}
		else if( parser.isFlagSet( kListUIGroupInformationFlag ) )
		{
			setResult( shader->getUIGroups() );
			return MS::kSuccess;
		}
	}
	else
	{
		// Process flag arguments
		//
		if( parser.isFlagSet(kReloadFlag) )
		{

			//GLSLShader::reloadAll( shader->effectName() );
			//the new effect name and technique name must have been reloaded
			shader->reload( );
		}
		else if( parser.isFlagSet(kClearParametersFlag))
		{
			shader->clearParameters();
		}
		else if( parser.isFlagSet(kConnectLightFlag))
		{
			MString connectableLightName;
			argData.getFlagArgument(kConnectLightFlag, 0, connectableLightName);
			int lightIndex = shader->getIndexForLightName(connectableLightName);
			if (lightIndex < 0)
			{
				MString msg = glslShaderStrings::getString( glslShaderStrings::kUnknownConnectableLight, connectableLightName );
				displayError( msg );
				return MS::kFailure;
			}
			MString lightName;
			argData.getFlagArgument(kConnectLightFlag, 1, lightName);
			MDagPath lightPath;
			list.clear();
			list.add( lightName );
			status = list.getDagPath(0, lightPath);
			if( status != MS::kSuccess )
			{
				MString msg = glslShaderStrings::getString( glslShaderStrings::kUnknownSceneObject, lightName );
				displayError( msg );
				return MS::kFailure;
			}
			// Make sure it is a light:
			MDagPath lightShapeDagPath;
			unsigned int numShapes = 0;
			lightPath.numberOfShapesDirectlyBelow(numShapes);
			for (unsigned int i = 0; i < numShapes; ++i)
			{
				MDagPath shapePath(lightPath);
				status = shapePath.extendToShapeDirectlyBelow(i);
				if( status == MS::kSuccess && shapePath.hasFn(MFn::kLight))
				{
					lightShapeDagPath = shapePath;
					break;
				}
			}

			if (!lightShapeDagPath.isValid())
			{
				MString msg = glslShaderStrings::getString( glslShaderStrings::kNotALight, lightName );
				displayError( msg );
				return MS::kFailure;
			}

			shader->disconnectLight(lightIndex);
			shader->connectLight(lightIndex,lightShapeDagPath);
			return MS::kSuccess;
		}
		else if( parser.isFlagSet(kLightConnectionStatusFlag))
		{
			MString connectableLightName;
			argData.getFlagArgument(kLightConnectionStatusFlag, 0, connectableLightName);
			int lightIndex = shader->getIndexForLightName(connectableLightName);
			if (lightIndex < 0)
			{
				MString msg = glslShaderStrings::getString( glslShaderStrings::kUnknownConnectableLight, connectableLightName );
				displayError( msg );
				return MS::kFailure;
			}
			setResult( shader->getLightConnectionInfo(lightIndex));
			return MS::kSuccess;
		}
		else if(parser.isFlagSet(kListLightParametersFlag))
		{
			MString connectableLightName;
			argData.getFlagArgument(kListLightParametersFlag, 0, connectableLightName);
			int lightIndex = shader->getIndexForLightName(connectableLightName);
			if (lightIndex < 0)
			{
				MString msg = glslShaderStrings::getString( glslShaderStrings::kUnknownConnectableLight, connectableLightName );
				displayError( msg );
				return MS::kFailure;
			}
			setResult( shader->getLightableParameters(lightIndex, parser.isFlagSet(kListLightParameterSemanticsFlag) ));
			return MS::kSuccess;
		}
		else if(parser.isFlagSet(kListUIGroupParametersFlag))
		{
			MString connectableUIGroupName;
			argData.getFlagArgument(kListUIGroupParametersFlag, 0, connectableUIGroupName);
			int uiGroupIndex = shader->getIndexForUIGroupName(connectableUIGroupName);
			if (uiGroupIndex < 0)
			{
				MString msg = glslShaderStrings::getString( glslShaderStrings::kUnknownUIGroup, connectableUIGroupName );
				displayError( msg );
				return MS::kFailure;
			}
			setResult( shader->getUIGroupParameters(uiGroupIndex) );
			return MS::kSuccess;
		}
		else if( parser.isFlagSet(kDisconnectLightFlag))
		{
			MString lightName;
			argData.getFlagArgument(kDisconnectLightFlag, 0, lightName);
			int lightIndex = shader->getIndexForLightName(lightName);
			if (lightIndex < 0)
			{
				MString msg = glslShaderStrings::getString( glslShaderStrings::kUnknownConnectableLight, lightName );
				displayError( msg );
				return MS::kFailure;
			}
			shader->disconnectLight(lightIndex);
			return MS::kSuccess;
		}
	}


	return MS::kSuccess;
}

MSyntax GLSLShaderCmd::newSyntax()
{
	MSyntax syntax;
	syntax.enableQuery();
	syntax.addFlag( kReloadFlag, kReloadFlagLong);
	syntax.addFlag( kFXFileFlag, kFXFileFlagLong, MSyntax::kString );
	syntax.addFlag( kListTechniquesFlag, kListTechniquesFlagLong);
	syntax.addFlag( kClearParametersFlag, kClearParametersFlagLong);
	syntax.addFlag( kListLightInformationFlag, kListLightInformationFlagLong);
	syntax.addFlag( kConnectLightFlag, kConnectLightFlagLong, MSyntax::kString, MSyntax::kString );
	syntax.addFlag( kLightConnectionStatusFlag, kLightConnectionStatusFlagLong, MSyntax::kString );
	syntax.addFlag( kListLightParametersFlag, kListLightParametersFlagLong, MSyntax::kString );
	syntax.addFlag( kListLightParameterSemanticsFlag, kListLightParameterSemanticsFlagLong );
	syntax.addFlag( kListUIGroupInformationFlag, kListUIGroupInformationFlagLong);
	syntax.addFlag( kListUIGroupParametersFlag, kListUIGroupParametersFlagLong, MSyntax::kString );
	syntax.addFlag( kDisconnectLightFlag, kDisconnectLightFlagLong, MSyntax::kString);
	syntax.addArg( MSyntax::kString );	
	return syntax;
}

void* GLSLShaderCmd::creator()
{
	return new GLSLShaderCmd;
}

