#ifndef _GLSLSHADER_GLSLSHADERSTRINGS_H
#define _GLSLSHADER_GLSLSHADERSTRINGS_H

#include <maya/MStringResourceId.h>

class MString;
class MStringArray;

namespace glslShaderStrings
{

	extern const MStringResourceId kErrorLog;
	extern const MStringResourceId kWarningLog;

	extern const MStringResourceId kErrorFileNotFound;
	extern const MStringResourceId kErrorLoadingEffect;


	// GLSLShader_initUI.mel
	extern const MStringResourceId kReloadTool;			
	extern const MStringResourceId kReloadAnnotation;	
	extern const MStringResourceId kEditTool;			
	extern const MStringResourceId kEditAnnotationWIN;	
	extern const MStringResourceId kEditAnnotationMAC;	
	extern const MStringResourceId kEditAnnotationLNX;	
	extern const MStringResourceId kEditCommandLNX;		
	extern const MStringResourceId kNiceNodeName;		

	// AEGLSLShaderTemplate.mel
	extern const MStringResourceId kShaderFile;
	extern const MStringResourceId kShader;
	extern const MStringResourceId kTechnique;
	extern const MStringResourceId kTechniqueTip;
	extern const MStringResourceId kLightBinding;
	extern const MStringResourceId kLightConnectionNone;
	extern const MStringResourceId kLightConnectionImplicit;
	extern const MStringResourceId kLightConnectionImplicitNone;
	extern const MStringResourceId kLightConnectionNoneTip;
	extern const MStringResourceId kLightConnectionImplicitTip;
	extern const MStringResourceId kLightConnectionExplicitTip;
	extern const MStringResourceId kShaderParameters;
	extern const MStringResourceId kSurfaceData;
	extern const MStringResourceId kDiagnostics;
	extern const MStringResourceId kNoneDefined;
	extern const MStringResourceId kEffectFiles;
	extern const MStringResourceId kAmbient;	

	extern const MStringResourceId kActionChoose;
	extern const MStringResourceId kActionEdit;
	extern const MStringResourceId kActionNew;
	extern const MStringResourceId kActionNewAnnotation;
	extern const MStringResourceId kActionNothing;
	extern const MStringResourceId kActionNothingAnnotation;
	extern const MStringResourceId kActionNotAssigned;
	extern const MStringResourceId kActionEmptyCommand;

	extern const MStringResourceId kActionEditorTitle;
	extern const MStringResourceId kActionEditorName;
	extern const MStringResourceId kActionEditorImageFile;
	extern const MStringResourceId kActionEditorDescription;
	extern const MStringResourceId kActionEditorCommands;
	extern const MStringResourceId kActionEditorInsertVariable;
	extern const MStringResourceId kActionEditorSave;
	extern const MStringResourceId kActionEditorCreate;
	extern const MStringResourceId kActionEditorDelete;
	extern const MStringResourceId kActionEditorClose;

	//GLSLShader
	extern const MStringResourceId kErrorRegisterNodeType;

	//GLSLShaderCmd
	extern const MStringResourceId kInvalidGLSLShader;
	extern const MStringResourceId kUnknownSceneObject;
	extern const MStringResourceId kUnknownUIGroup;
	extern const MStringResourceId kNotALight;
	extern const MStringResourceId kUnknownConnectableLight;

	//GLSLShaderCreateUI.mel
	extern const MStringResourceId kPrefSavePrefsOrNotMsg;
	extern const MStringResourceId kPrefSaveMsg;
	extern const MStringResourceId kPrefFileOpen;
	extern const MStringResourceId kPrefDefaultEffectFile;
	extern const MStringResourceId kPrefGLSLShaderPanel;
	extern const MStringResourceId kPrefGLSLShaderTab;

	// Register all strings
	MStatus registerMStringResources(void);

	MString getString(const MStringResourceId &stringId);
	MString getString(const MStringResourceId &stringId, const MString& arg);
	MString getString(const MStringResourceId &stringId, const MStringArray& args);
}

#endif

//-
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+



