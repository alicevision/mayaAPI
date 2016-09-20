#include "GLSLShaderStrings.h"

#include <maya/MStringResource.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>

#ifdef __LINUX__ // Linux
#include <strings.h>
#endif

namespace glslShaderStrings
{
#define kPluginId  "glslShader"
	const MStringResourceId kErrorLog					( kPluginId, "kErrorLog", 					MString( "errors :\n^1s" ) );
	const MStringResourceId kWarningLog					( kPluginId, "kWarningLog", 				MString( "warnings :\n^1s" ) );
	
	const MStringResourceId kErrorFileNotFound			( kPluginId, "kErrorFileNotFound", 			MString( "Effect file \"^1s\" not found." ) );
	const MStringResourceId kErrorLoadingEffect         ( kPluginId, "kErrorLoadingEffect", 		MString( "Error Loading Effect file \"^1s\"" ) );
	
	// GLSLShader_initUI.mel
	const MStringResourceId kReloadTool					( kPluginId, "kReloadTool", 				MString( "Reload" ) );
	const MStringResourceId kReloadAnnotation			( kPluginId, "kReloadAnnotation", 			MString( "Reload shader file" ) );
	const MStringResourceId kEditTool					( kPluginId, "kEditTool", 					MString( "Edit" ) );
	const MStringResourceId kEditAnnotationWIN			( kPluginId, "kEditAnnotationWIN", 			MString( "Edit with application associated with shader file" ) );
	const MStringResourceId kEditAnnotationMAC			( kPluginId, "kEditAnnotationMAC", 			MString( "Edit shader file with TextEdit" ) );
	const MStringResourceId kEditAnnotationLNX			( kPluginId, "kEditAnnotationLNX", 			MString( "Edit shader file with your editor" ) );
	const MStringResourceId kEditCommandLNX				( kPluginId, "kEditCommandLNX", 			MString( "Please set the command before using this feature." ) );
	const MStringResourceId kNiceNodeName				( kPluginId, "kNiceNodeName", 				MString( "GLSL Shader" ) );

	//GLSLShaderNode
	const MStringResourceId kErrorRegisterNodeType		( kPluginId, "kErrorRegisterNodeType",		MString( "Failed to register ^1s to filePathEditor" ) );

	//GLSLShaderCreateUI.mel
	const MStringResourceId kPrefSavePrefsOrNotMsg		( kPluginId, "kPrefSavePrefsOrNotMsg", 		MString( "The GLSLShader plug-in is about to refresh preferences windows. Save your changes in the preferences window?" ) );
	const MStringResourceId kPrefSaveMsg				( kPluginId, "kPrefSaveMsg", 				MString( "Save Preferences" ) );
	const MStringResourceId kPrefFileOpen				( kPluginId, "kPrefFileOpen", 				MString( "Open" ) );
	const MStringResourceId kPrefDefaultEffectFile		( kPluginId, "kPrefDefaultEffectFile", 		MString( "Default effects file" ) );
	const MStringResourceId kPrefGLSLShaderPanel		( kPluginId, "kPrefGLSLShaderPanel", 		MString( "GLSL Shader Preferences" ) );
	const MStringResourceId kPrefGLSLShaderTab			( kPluginId, "kPrefGLSLShaderTab", 			MString( "    GLSL Shader" ) );

	// GLSLShaderTemplate.mel
	const MStringResourceId kShaderFile					( kPluginId, "kShaderFile", 				MString( "Shader File" ) );
	const MStringResourceId kShader						( kPluginId, "kShader", 					MString( "Shader" ) );
	const MStringResourceId kTechnique					( kPluginId, "kTechnique", 					MString( "Technique" ) );
	const MStringResourceId kTechniqueTip				( kPluginId, "kTechniqueTip", 				MString( "Select among the rendering techniques defined in the fx file." ) );
	const MStringResourceId kLightBinding				( kPluginId, "kLightBinding",				MString( "Light Binding" ) );
	const MStringResourceId kLightConnectionNone		( kPluginId, "kLightConnectionNone", 		MString( "Use Shader Settings" ) );
	const MStringResourceId kLightConnectionImplicit	( kPluginId, "kLightConnectionImplicit", 	MString( "Automatic Bind" ) );
	const MStringResourceId kLightConnectionImplicitNone( kPluginId, "kLightConnectionImplicitNone",MString( "none" ) );
	const MStringResourceId kLightConnectionNoneTip		( kPluginId, "kLightConnectionNoneTip", 	MString( "Ignores Maya lights and uses the settings in the effect file." ) );
	const MStringResourceId kLightConnectionImplicitTip	( kPluginId, "kLightConnectionImplicitTip", MString( "Maya will automatically bind scene lights and parameters to the lights defined in the effect file." ) );
	const MStringResourceId kLightConnectionExplicitTip	( kPluginId, "kLightConnectionExplicitTip",	MString( "Explicitly binds a Maya scene light and it's parameters to a light defined in the effect file." ) );
	const MStringResourceId kShaderParameters			( kPluginId, "kShaderParameters", 			MString( "Parameters" ) );
	const MStringResourceId kSurfaceData				( kPluginId, "kSurfaceData", 				MString( "Surface Data" ) );
	const MStringResourceId kDiagnostics				( kPluginId, "kDiagnostics", 				MString( "Diagnostics" ) );
	const MStringResourceId kNoneDefined				( kPluginId, "kNoneDefined", 				MString( "None defined" ) );
	const MStringResourceId kEffectFiles				( kPluginId, "kEffectFiles", 				MString( "Effect Files" ) );
	const MStringResourceId kAmbient					( kPluginId, "kAmbient", 					MString( "Ambient" ) );

	const MStringResourceId kActionChoose				( kPluginId, "kActionChoose",				MString( "Choose button action:" ) );
	const MStringResourceId kActionEdit					( kPluginId, "kActionEdit", 				MString( "Edit the action definition" ) );
	const MStringResourceId kActionNew					( kPluginId, "kActionNew", 					MString( "New action..." ) );
	const MStringResourceId kActionNewAnnotation		( kPluginId, "kActionNewAnnotation", 		MString( "Add a new action to this menu" ) );
	const MStringResourceId kActionNothing				( kPluginId, "kActionNothing", 				MString( "(nothing)" ) );
	const MStringResourceId kActionNothingAnnotation	( kPluginId, "kActionNothingAnnotation", 	MString( "Button does nothing; not assigned." ) );
	const MStringResourceId kActionNotAssigned			( kPluginId, "kActionNotAssigned", 			MString( "LMB: Does nothing; not assigned.  RMB: Choose what this button should do." ) );
	const MStringResourceId kActionEmptyCommand			( kPluginId, "kActionEmptyCommand", 		MString( "LMB: Does nothing; empty command.  RMB: Choose what this button should do." ) );

	const MStringResourceId kActionEditorTitle			( kPluginId, "kActionEditorTitle", 			MString( "GLSLShader Tool Button Editor" ) );
	const MStringResourceId kActionEditorName			( kPluginId, "kActionEditorName", 			MString( "Name" ) );
	const MStringResourceId kActionEditorImageFile		( kPluginId, "kActionEditorImageFile", 		MString( "Image File" ) );
	const MStringResourceId kActionEditorDescription	( kPluginId, "kActionEditorDescription", 	MString( "Description" ) );
	const MStringResourceId kActionEditorCommands		( kPluginId, "kActionEditorCommands", 		MString( "Commands" ) );
	const MStringResourceId kActionEditorInsertVariable	( kPluginId, "kActionEditorInsertVariable", MString( "Insert variable:" ) );
	const MStringResourceId kActionEditorSave			( kPluginId, "kActionEditorSave", 			MString( "Save" ) );
	const MStringResourceId kActionEditorCreate			( kPluginId, "kActionEditorCreate", 		MString( "Create" ) );
	const MStringResourceId kActionEditorDelete			( kPluginId, "kActionEditorDelete", 		MString( "Delete" ) );
	const MStringResourceId kActionEditorClose			( kPluginId, "kActionEditorClose", 			MString( "Close" ) );

	//GLSLShaderCmd
	const MStringResourceId kInvalidGLSLShader			( kPluginId, "kInvalidGLSLShader",			MString( "Invalid GLSLShader node: ^1s" ) );
	const MStringResourceId kUnknownSceneObject			( kPluginId, "kUnknownSceneObject",			MString( "Unknown scene object: ^1s" ) );
	const MStringResourceId kUnknownUIGroup				( kPluginId, "kUnknownUIGroup",				MString( "Unknown UI group: ^1s" ) );
	const MStringResourceId kNotALight					( kPluginId, "kNotALight",					MString( "Not a light: ^1s" ) );
	const MStringResourceId kUnknownConnectableLight	( kPluginId, "kUnknownConnectableLight",	MString( "Unknown connectable light: ^1s" ) );
}


// Register all strings
//
MStatus glslShaderStrings::registerMStringResources(void)
{
	MStringResource::registerString( kErrorLog );
	MStringResource::registerString( kWarningLog );
	MStringResource::registerString( kErrorFileNotFound );

	MStringResource::registerString( kReloadAnnotation );

	MStringResource::registerString( kReloadTool	);	
	MStringResource::registerString( kReloadAnnotation );
	MStringResource::registerString( kEditTool );
	MStringResource::registerString( kEditAnnotationWIN );	
	MStringResource::registerString( kEditAnnotationMAC );	
	MStringResource::registerString( kEditAnnotationLNX );	
	MStringResource::registerString( kEditCommandLNX	);	
	MStringResource::registerString( kNiceNodeName );	

	MStringResource::registerString( kShaderFile );
	MStringResource::registerString( kShader );
	MStringResource::registerString( kTechnique );
	MStringResource::registerString( kTechniqueTip );
	MStringResource::registerString( kLightBinding );
	MStringResource::registerString( kLightConnectionNone );
	MStringResource::registerString( kLightConnectionImplicit );
	MStringResource::registerString( kLightConnectionImplicitNone );
	MStringResource::registerString( kLightConnectionNoneTip );
	MStringResource::registerString( kLightConnectionImplicitTip );
	MStringResource::registerString( kLightConnectionExplicitTip );
	MStringResource::registerString( kShaderParameters );
	MStringResource::registerString( kSurfaceData );
	MStringResource::registerString( kDiagnostics );
	MStringResource::registerString( kNoneDefined );
	MStringResource::registerString( kEffectFiles );
	MStringResource::registerString( kAmbient );

	MStringResource::registerString( kActionChoose );
	MStringResource::registerString( kActionEdit );
	MStringResource::registerString( kActionNew );
	MStringResource::registerString( kActionNewAnnotation );
	MStringResource::registerString( kActionNothing );
	MStringResource::registerString( kActionNothingAnnotation );
	MStringResource::registerString( kActionNotAssigned );
	MStringResource::registerString( kActionEmptyCommand );

	MStringResource::registerString( kActionEditorTitle );
	MStringResource::registerString( kActionEditorName );
	MStringResource::registerString( kActionEditorImageFile );
	MStringResource::registerString( kActionEditorDescription );
	MStringResource::registerString( kActionEditorCommands );
	MStringResource::registerString( kActionEditorInsertVariable );
	MStringResource::registerString( kActionEditorSave );
	MStringResource::registerString( kActionEditorCreate );
	MStringResource::registerString( kActionEditorDelete );
	MStringResource::registerString( kActionEditorClose );

	//GLSLShaderCreateUI.mel
	MStringResource::registerString( kPrefSavePrefsOrNotMsg );
	MStringResource::registerString( kPrefSaveMsg );
	MStringResource::registerString( kPrefFileOpen );
	MStringResource::registerString( kPrefDefaultEffectFile );
	MStringResource::registerString( kPrefGLSLShaderPanel );
	MStringResource::registerString( kPrefGLSLShaderTab );

	//GLSLShaderNode
	MStringResource::registerString( kErrorRegisterNodeType );

	//GLSLShaderCmd
	MStringResource::registerString( kInvalidGLSLShader );
	MStringResource::registerString( kUnknownSceneObject );
	MStringResource::registerString( kUnknownUIGroup );
	MStringResource::registerString( kNotALight );
	MStringResource::registerString( kUnknownConnectableLight );

	return MS::kSuccess;
}

MString glslShaderStrings::getString(const MStringResourceId &stringId)
{
	MStatus status;
	return MStringResource::getString(stringId, status);
}

MString glslShaderStrings::getString(const MStringResourceId &stringId, const MString& arg)
{
	MStringArray args;
	args.append(arg);
	return glslShaderStrings::getString(stringId, args);
}

MString glslShaderStrings::getString(const MStringResourceId &stringId, const MStringArray& args)
{
	MString string;
	string.format( glslShaderStrings::getString(stringId), args);
	return string;
}

//-
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+
