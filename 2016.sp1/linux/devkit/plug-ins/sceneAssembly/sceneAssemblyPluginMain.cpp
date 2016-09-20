#include "assemblyReference.h"
#include "assemblyDefinition.h"
#include "adskPrepareRenderGlobals.h"
#include "sceneAssemblyStrings.h"
#include "adskRepresentations.h"
#include "adskRepresentationCmd.h"
#include "adskSceneMetadataCmd.h"

#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>

#include <vector>

//Register all strings
static MStatus registerMStringRes()
{
	MStringResource::registerString(rRegisterUIStringError);
	MStringResource::registerString(rRegisterNodeError);
	MStringResource::registerString(rDeregisterNodeError);
	MStringResource::registerString(rRegisterAssembliesError);
	MStringResource::registerString(rRegisterRepresentationsError);
	MStringResource::registerString(rRegisterCmdError);			  
	MStringResource::registerString(rAssemblyDefnImportError);    
	MStringResource::registerString(rAssemblyDefnNotFoundError);  
	MStringResource::registerString(rMultAssemblyDefnFoundError);
	MStringResource::registerString(rRegisterRepFactoryError);	
	MStringResource::registerString(rDeregisterRepFactoryError);
	MStringResource::registerString(rCreateGPUCacheNodeError);
	MStringResource::registerString(rEditQueryError);		
	MStringResource::registerString(rRepTypeObjArgError);	
	MStringResource::registerString(rListRepTypesFlagError);
	
	return MStatus::kSuccess;
}

namespace {

//==============================================================================
// LOCAL FUNCTIONS
//==============================================================================

//------------------------------------------------------------------------------
//
template< class T >
MStatus registerNode(MFnPlugin& plugin, MPxNode::Type nodeType, const MString* classification = NULL)
{    
   MStatus status = plugin.registerNode(
      T::typeName, T::id, T::creator, T::initialize, nodeType, classification
   );
    
   if (!status) {
       MStatus resStatus;
	   MString errorString =
          MStringResource::getString( rRegisterNodeError, resStatus);
	   errorString.format(errorString, T::typeName, status.errorString());
	   MGlobal::displayError(errorString);
   }
   return status;
}

//------------------------------------------------------------------------------
//
template< class T >
MStatus registerAssemblyNode(MFnPlugin& plugin, MPxNode::Type nodeType)
{    
   // Classify the scene assembly node as a transform.  This causes Viewport
   // 2.0 to treat the scene assembly node the same way it treats a regular
   // transform node, including support for drawing the handle and local axis.
   MString assemblyClassification = "drawdb/geometry/transform";
   
   MStatus status = registerNode<T>(plugin, nodeType, &assemblyClassification);  
   return status;
}

//------------------------------------------------------------------------------
//
template< class T >
MStatus deregisterNode(MFnPlugin& plugin)
{
   MStatus status = plugin.deregisterNode(T::id);
   if (!status) {
       MStatus resStatus;
	   MString errorString =
          MStringResource::getString( rDeregisterNodeError, resStatus);
	   errorString.format(errorString, T::typeName, status.errorString());
	   MGlobal::displayError(errorString);
   }   
   return status;
}

//------------------------------------------------------------------------------
//
template< class T >
MStatus deregisterAssemblyNode(MFnPlugin& plugin)
{   
   MStatus status = deregisterNode<T>(plugin);
   MGlobal::executeCommand("assembly -e -deregister " + T::typeName);
   return status;
}

//------------------------------------------------------------------------------
//
void displayError(const MStringResourceId& id, const MString& fmtString)
{
   // Display an error message with a single string in its format.
   MStatus resStatus;
   MString errorString = MStringResource::getString(id, resStatus);
   errorString.format(errorString, fmtString);
   MGlobal::displayError(errorString);
}

}

//==============================================================================
// PLUGIN INITIALIZATION
//==============================================================================

//------------------------------------------------------------------------------
//
MStatus initializePlugin(MObject obj)
{
    MStatus   status;
    MFnPlugin plugin( obj, PLUGIN_COMPANY, "1.0", "Any");
    
   

	//register strings
	status = plugin.registerUIStrings(registerMStringRes,
		"sceneAssemblyInitStrings");
	if (!status) {
       displayError(rRegisterUIStringError, status.errorString());
       return status;
	}
	
    if (!(status = registerAssemblyNode<AssemblyDefinition>(
             plugin, MPxNode::kAssembly))) {
       return status;
    }

	if (!(status = registerAssemblyNode<AssemblyReference>(
		plugin, MPxNode::kAssembly))) {
			return status;
	}
    
    if (!(status = registerNode<AdskPrepareRenderGlobals>(
        plugin, MPxNode::kDependNode))) {
            return status;
    }

    MGlobal::sourceFile ( "assemblyReferenceUtil.mel" ) ;
    MGlobal::sourceFile ( "assemblyDefinitionUtil.mel" ) ;
    // Following MEL file contains code that is UI-only.
    if (MGlobal::mayaState() == MGlobal::kInteractive) {
       MGlobal::sourceFile( "AEassemblyNamespaceUtil.mel" ) ;
    }
	
    if (!(status =
          MGlobal::executePythonCommand("import maya.app.sceneAssembly"))) {
       return status;
    }

	int	isLoaded = false;
	// Validate that the gpuCache plugin is loaded.
    MGlobal::executeCommand( "pluginInfo -query -loaded gpuCache", isLoaded );	
	// Load the gpuCache plugin if it's not loaded.		
    if (!isLoaded){
		MGlobal::executeCommand("loadPlugin -quiet gpuCache");		
	}
	
    status = MGlobal::executeCommand("registerAssemblies");
    if (!status) {
       displayError(rRegisterAssembliesError, status.errorString());
    }
	
    // Register representation factories.  Ownership of the factory is
    // transferred to the registry.
    std::vector<AdskRepresentationFactory*> factories;
    factories.push_back(new CacheRepresentation::Factory());
    factories.push_back(new SceneRepresentation::Factory());
    factories.push_back(new LocatorRepresentation::Factory());
    std::vector<AdskRepresentationFactory*>::const_iterator f;
    for (f = factories.begin(); f != factories.end(); ++f) {
       if (!AssemblyDefinition::registerRepresentationFactory(*f)) {
          displayError(rRegisterRepFactoryError, (*f)->getType());
          delete (*f);
       }	  
    }
	
	status = plugin.registerCommand(AdskRepresentationCmd::name(), AdskRepresentationCmd::creator, AdskRepresentationCmd::cmdSyntax );
    if (!status) {
		MString errorString = MStringResource::getString( rRegisterCmdError, status);
		errorString.format(errorString, AdskRepresentationCmd::name(), status.errorString());
		MGlobal::displayError( errorString);
        return status;
    }
	
	status = MGlobal::executeCommand("registerRepresentations");
    if (!status) {
       displayError(rRegisterRepresentationsError, status.errorString());
    }
	
	// register assemblyReference and assemblyDefinition to filePathEditor
	status = MGlobal::executeCommand("filePathEditor -registerType \"assemblyReference\" -typeLabel \"AssemblyReference\" -temporary");
    if (!status) {
		MString errorString = MStringResource::getString( rRegisterFilePathEditorError, status );
		errorString.format(errorString, "assemblyReference", status.errorString());
		MGlobal::displayWarning( errorString );
    }
	status = MGlobal::executeCommand("filePathEditor -registerType \"assemblyDefinition\" -typeLabel \"AssemblyDefinition\" -temporary");
    if (!status) {
       	MString errorString = MStringResource::getString( rRegisterFilePathEditorError, status );
		errorString.format(errorString, "assemblyDefinition", status.errorString());
		MGlobal::displayWarning( errorString );
    }

	// Register the scene metadata command
	status = plugin.registerCommand(AdskSceneMetadataCmd::name(), AdskSceneMetadataCmd::creator, AdskSceneMetadataCmd::cmdSyntax );
	if (!status) {
		MString errorString = MStringResource::getString( rRegisterCmdError, status);
		errorString.format(errorString, AdskRepresentationCmd::name(), status.errorString());
		MGlobal::displayError( errorString);
		return status;
	}

    return MS::kSuccess;
}

//------------------------------------------------------------------------------
//
MStatus uninitializePlugin(MObject obj)
{
    MStatus   status;
    MFnPlugin plugin(obj);
	
    if (!(status = plugin.deregisterCommand(AdskSceneMetadataCmd::name()))) {
        return status;
    } 

	if (!(status = plugin.deregisterCommand(AdskRepresentationCmd::name()))) {
	   return status;
    } 

    // Unregister and delete representation factories.
    std::vector<MString> factoryTypes;
    factoryTypes.push_back(CacheRepresentation::type());
    factoryTypes.push_back(SceneRepresentation::type());
    factoryTypes.push_back(LocatorRepresentation::type());
    std::vector<MString>::const_iterator fType;
    for (fType = factoryTypes.begin(); fType != factoryTypes.end(); ++fType) {
       if (!AssemblyDefinition::deregisterRepresentationFactory(*fType)) {
          displayError(rDeregisterRepFactoryError, *fType);
       }
    }

 	if (!(status = deregisterNode<AdskPrepareRenderGlobals>(plugin))) {
       return status;
    }

    if (!(status = deregisterAssemblyNode<AssemblyReference>(plugin))) {
       return status;
    }

    if (!(status = deregisterAssemblyNode<AssemblyDefinition>(plugin))) {
       return status;
    }   
	
	// deregister assemblyReference and assemblyDefinition
	status = MGlobal::executeCommand("filePathEditor -deregisterType \"assemblyReference\" -temporary");
    if (!status) {
		MString errorString = MStringResource::getString( rDeregisterFilePathEditorError, status);
		errorString.format(errorString, "assemblyReference", status.errorString());
		MGlobal::displayWarning( errorString );
    }
	status = MGlobal::executeCommand("filePathEditor -deregisterType \"assemblyDefinition\" -temporary");
    if (!status) {
       	MString errorString = MStringResource::getString( rDeregisterFilePathEditorError, status);
		errorString.format(errorString, "assemblyDefinition", status.errorString());
		MGlobal::displayWarning( errorString );
    }
    
    return MS::kSuccess;
}

//-
//*****************************************************************************
// Copyright 2015 Autodesk, Inc.  All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy
// form.
//*****************************************************************************
//+
