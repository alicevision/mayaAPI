#include "sceneAssemblyStrings.h"
#include "adskRepresentations.h"
#include "assemblyDefinition.h"

#include <maya/MGlobal.h>
#include <maya/MDagModifier.h>
#include <maya/MFnContainerNode.h>
#include <maya/MFnAssembly.h>

#include <maya/MExternalContentInfoTable.h>
#include <maya/MExternalContentLocationTable.h>

#include <cassert>

namespace {

//==============================================================================
// LOCAL DECLARATIONS
//==============================================================================

/*----- constants -----*/

const char CACHE_TYPE[]          = "Cache";
const char SCENE_TYPE[]          = "Scene";
const char LOCATOR_TYPE[]        = "Locator";

//==============================================================================
// LOCAL FUNCTIONS
//==============================================================================

//------------------------------------------------------------------------------
//
MString pathSep(const MString& path, const char sep)
{
   if (path.numChars() == 0) {
      return MString();
   }
   MStringArray components;
   MStatus status = path.split(sep, components);
   if (status != MS::kSuccess) {
      return MString();
   }
   return components[components.length()-1];
}

//------------------------------------------------------------------------------
//
MString pathTail(const MString& path)
{
   return pathSep(path, '/');
}

}

//==============================================================================
// CLASS CacheRepresentation::Factory
//==============================================================================

//------------------------------------------------------------------------------
//
MPxRepresentation* CacheRepresentation::Factory::create(
   MPxAssembly* assembly, const MString& name, const MString&
) const
{
   return new CacheRepresentation(assembly, name);
}

//------------------------------------------------------------------------------
//
MString CacheRepresentation::Factory::creationName(
   MPxAssembly* /* assembly */, const MString& input
) const
{
   return pathTail(input);
}

//------------------------------------------------------------------------------
//
MString CacheRepresentation::Factory::creationLabel(
   MPxAssembly* assembly, const MString& input
) const
{
   return creationName(assembly, input);
}

//------------------------------------------------------------------------------
//
MString CacheRepresentation::Factory::creationData(
   MPxAssembly* /* assembly */, const MString& input
) const
{
   return input;
}

//==============================================================================
// CLASS SceneRepresentation::Factory
//==============================================================================

//------------------------------------------------------------------------------
//
MPxRepresentation* SceneRepresentation::Factory::create(
   MPxAssembly* assembly, const MString& name, const MString&
) const
{
   return new SceneRepresentation(assembly, name);
}

//------------------------------------------------------------------------------
//
MString SceneRepresentation::Factory::creationName(
   MPxAssembly* /* assembly */, const MString& input
) const
{
   return pathTail(input);
}

//------------------------------------------------------------------------------
//
MString SceneRepresentation::Factory::creationLabel(
   MPxAssembly* assembly, const MString& input
) const
{
   return creationName(assembly, input);
}

//------------------------------------------------------------------------------
//
MString SceneRepresentation::Factory::creationData(
   MPxAssembly* assembly, const MString& input
) const
{
   return input;
}

//==============================================================================
// CLASS LocatorRepresentation::Factory
//==============================================================================

//------------------------------------------------------------------------------
//
MPxRepresentation* LocatorRepresentation::Factory::create(
   MPxAssembly* assembly, const MString& name, const MString& data
) const
{
   return new LocatorRepresentation(assembly, name, data);
}

//------------------------------------------------------------------------------
//
MString LocatorRepresentation::Factory::creationName(
   MPxAssembly* /* assembly */, const MString& input
) const
{
   return MString("Locator");
}

//------------------------------------------------------------------------------
//
MString LocatorRepresentation::Factory::creationLabel(
   MPxAssembly* assembly, const MString& input
) const
{
   return creationName(assembly, input);
}

//------------------------------------------------------------------------------
//
MString LocatorRepresentation::Factory::creationData(
   MPxAssembly* assembly, const MString& input
) const
{
   return input;
}

//==============================================================================
// CLASS CacheRepresentation
//==============================================================================

//------------------------------------------------------------------------------
//
CacheRepresentation::CacheRepresentation(
   MPxAssembly* assembly, const MString& name
) : BaseClass(assembly, name)
{
   assert( dynamic_cast< AssemblyDefinition* >( getAssembly() ) != 0 );
}

//------------------------------------------------------------------------------
//
CacheRepresentation::~CacheRepresentation()
{}

//------------------------------------------------------------------------------
//
bool CacheRepresentation::activate()
{
   AssemblyDefinition* const assembly =
      dynamic_cast< AssemblyDefinition* >( getAssembly() );
   if ( assembly == 0 ) {
      return false;
   }

   // Create a gpuCache node, and parent it to our container.
   MDagModifier dagMod;   
   MStatus status;
   MObject cacheObj = dagMod.createNode(
      MString("gpuCache"), assembly->thisMObject(), &status);

   if (status != MStatus::kSuccess) {
      int	isLoaded = false;
      // Validate that the gpuCache plugin is loaded.
      MGlobal::executeCommand( "pluginInfo -query -loaded gpuCache", isLoaded );
	  if(!isLoaded){	     
	     MString errorString = MStringResource::getString(rCreateGPUCacheNodeError, status);	     
	     MGlobal::displayError(errorString);	    
      }
      return false;
   }
   status = dagMod.doIt();
   if (status != MStatus::kSuccess) {
      return false;
   }
      
   // Set the cache attribute to point to our Alembic file.
   MFnDependencyNode cache(cacheObj);
   MPlug fileName = cache.findPlug(MString("cacheFileName"), true, &status);
   if (status != MStatus::kSuccess) {
      return false;
   }
   fileName.setValue(assembly->getRepData(getName()));

   return status == MStatus::kSuccess;
}

//------------------------------------------------------------------------------
//
MString CacheRepresentation::type()
{
   return CACHE_TYPE;
}

//------------------------------------------------------------------------------
//
MString CacheRepresentation::getType() const
{
   return type();
}


//------------------------------------------------------------------------------
//
void CacheRepresentation::getExternalContent(
   MExternalContentInfoTable& table
) const
{
   const AssemblyDefinition* const assembly =
      dynamic_cast< AssemblyDefinition* >( getAssembly() );
   if ( assembly == 0 ) {
      return;
   }

   table.addUnresolvedEntry( "Data", assembly->getRepData( getName() ), assembly->name() );
}

//------------------------------------------------------------------------------
//
void CacheRepresentation::setExternalContent(
   const MExternalContentLocationTable& table
)
{
   AssemblyDefinition* const assembly =
      dynamic_cast< AssemblyDefinition* >( getAssembly() );
   if ( assembly == 0 ) {
      return;
   }

   MString path;
   table.getLocation( "Data", path );
   assembly->setRepData( getName(), path );
}


//==============================================================================
// CLASS SceneRepresentation
//==============================================================================

//------------------------------------------------------------------------------
//
SceneRepresentation::SceneRepresentation(
   MPxAssembly* assembly, const MString& name
) : BaseClass(assembly, name)
{
   assert( dynamic_cast< AssemblyDefinition* >( getAssembly() ) != 0 );
}

//------------------------------------------------------------------------------
//
SceneRepresentation::~SceneRepresentation()
{}

//------------------------------------------------------------------------------
//
bool SceneRepresentation::activate()
{
   AssemblyDefinition* const assembly =
      dynamic_cast< AssemblyDefinition* >( getAssembly() );
   if ( assembly == 0 ) {
      return false;
   }

   MFnAssembly aFn(assembly->thisMObject());

   bool fileIgnoreVersion = (MGlobal::optionVarIntValue("fileIgnoreVersion") == 1);

   MStatus status = aFn.importFile(
      assembly->getRepData(getName()) /*fileName*/,
      NULL /*type*/, true /*preserveReferences*/, NULL /*nameSpace*/,
      fileIgnoreVersion);

   return (status == MStatus::kSuccess);
}

//------------------------------------------------------------------------------
//
MString SceneRepresentation::type()
{
   return SCENE_TYPE;
}

//------------------------------------------------------------------------------
//
MString SceneRepresentation::getType() const
{
   return type();
}

//------------------------------------------------------------------------------
//
bool SceneRepresentation::canApplyEdits() const
{
   return true;
}


//------------------------------------------------------------------------------
//
void SceneRepresentation::getExternalContent(
   MExternalContentInfoTable& table
) const
{
   const AssemblyDefinition* const assembly =
      dynamic_cast< AssemblyDefinition* >( getAssembly() );
   if ( assembly == 0 ) {
      return;
   }

   table.addUnresolvedEntry( "Data", assembly->getRepData( getName() ), assembly->name() );
}

//------------------------------------------------------------------------------
//
void SceneRepresentation::setExternalContent(
   const MExternalContentLocationTable& table
)
{
   AssemblyDefinition* const assembly =
      dynamic_cast< AssemblyDefinition* >( getAssembly() );
   if ( assembly == 0 ) {
      return;
   }

   MString path;
   table.getLocation( "Data", path );
   assembly->setRepData( getName(), path );
}


//==============================================================================
// CLASS LocatorRepresentation
//==============================================================================

//------------------------------------------------------------------------------
//
LocatorRepresentation::LocatorRepresentation(
   MPxAssembly* assembly, const MString& name, const MString& data
) : BaseClass(assembly, name), fAnnotation(data)
{}

//------------------------------------------------------------------------------
//
LocatorRepresentation::~LocatorRepresentation()
{}

//------------------------------------------------------------------------------
//
bool LocatorRepresentation::activate()
{
   MPxAssembly* const assembly = getAssembly();

   // Create a locator node, and parent it to our container.
   MDagModifier dagMod;
   MStatus status;
   dagMod.createNode(MString("locator"), assembly->thisMObject(), &status);   
  
   if (status != MStatus::kSuccess) {
      return false;
   }
   status = dagMod.doIt();
   if (status != MStatus::kSuccess) {
      return false;
   }

   // If we have annotation text, create an annotation shape, and a
   // transform for it.  Parent the annotation transform to the assembly.
   if (fAnnotation.numChars() > 0) {
      MObject transformObj = dagMod.createNode(
         MString("transform"), assembly->thisMObject(), &status);

      if (status != MStatus::kSuccess) {
         return false;
      }

	  MString  annotationName =  "annotation";
	  // the + "#" forces Maya to rename using integers for unique names
	  MString  transformName = annotationName + "#";
      dagMod.renameNode(transformObj, transformName);
 
      status = dagMod.doIt();
      if (status != MStatus::kSuccess) {
         return false;
      }

      MObject annotationObj = dagMod.createNode(
         MString("annotationShape"), transformObj, &status);

      if (status != MStatus::kSuccess) {
         return false;
      }
      status = dagMod.doIt();
      if (status != MStatus::kSuccess) {
         return false;
      }

      // Set the annotation text.
      MFnDependencyNode annotation(annotationObj);
      MPlug text = annotation.findPlug(MString("text"), true, &status);
      if (status != MStatus::kSuccess) {
         return false;
      }
      text.setValue(fAnnotation);

      // Get rid of the arrow: our annotation doesn't need to be
      // offset from the locator for readability, since the locator
      // has no volume.  Therefore, we don't need an arrow to point
      // from the annotation back to the object.
      MPlug displayArrow =
         annotation.findPlug(MString("displayArrow"), true, &status);
      if (status != MStatus::kSuccess) {
         return false;
      }
      displayArrow.setValue(false);
   }
   
   return status == MStatus::kSuccess;
}

//------------------------------------------------------------------------------
//
MString LocatorRepresentation::type()
{
   return LOCATOR_TYPE;
}

//------------------------------------------------------------------------------
//
MString LocatorRepresentation::getType() const
{
   return type();
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
