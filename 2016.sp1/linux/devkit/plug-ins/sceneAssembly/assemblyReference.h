#ifndef _assemblyReference_h_
#define _assemblyReference_h_

///////////////////////////////////////////////////////////////////////////////
//
// AssemblyReference
//
// Autodesk reference implementation scene assembly reference node.
//
// Assembly references use assembly definitions to provide them with
// their list of representations, and representation attributes.  This
// allows a single assembly definition to be used by many assembly
// reference nodes, with a change in representations in the assembly
// definition being reflected in all assembly references that refer to
// it.  Fundamentally, multiple assembly references of the same
// assembly definition can be thought of logically "instancing" the
// assembly definition multiple times (though this mechanism is 
// different from Maya DAG instancing).  See AssemblyDefinition for
// more details.
//
// Assembly references cannot add or remove representations from the
// list given to them by their assembly definition.
//
// When an assembly definition file path is set onto an assembly reference,
// the assembly reference will automatically call its own postLoad() method
// to initialize itself.
//
////////////////////////////////////////////////////////////////////////////////

#include "assemblyDefinition.h"
#include "assemblyDefinitionFileCache.h"

class MStringResourceId;
class assemblyReferenceInitialRep;

class AssemblyReference : public AssemblyDefinition
{
public:

   /*----- static member functions -----*/

   static void* creator();
   static MStatus initialize();
   static MStatus uninitialize();

   static const MTypeId id;
   static const MString typeName;

   /*----- member functions -----*/
      
   AssemblyReference();
   ~AssemblyReference();

   // See base class.  Only getExternalContent is overridden: no need to
   // override setExternalContent as it already does the right thing, which is
   // to assume keys are plug names.
   virtual void getExternalContent( MExternalContentInfoTable& table ) const;
   virtual void setExternalContent(
      const MExternalContentLocationTable& table
   );

   // Returns failure: can't create representations through an
   // assembly reference.
   virtual MString createRepresentation(
      const MString& input,
      const MString& type,
      const MString& representation,
      MDagModifier*  undoRedo,
      MStatus*       status
   );

   // Representation creation query for existing assemblies.  Since
   // assembly references cannot create representations, returns an
   // empty array.
   virtual MStringArray repTypes() const;

   // MPxAssembly overrides.  The representation namespace is stored in
   // our representation namespace attribute.
   virtual MString   getRepNamespace() const;
   virtual void      updateRepNamespace(const MString& repNamespace);
   
   // Returns failure: can't delete representations through an
   // assembly reference.
   virtual MStatus      deleteRepresentation(const MString& representation);
   virtual MStatus      deleteAllRepresentations();

   virtual bool			supportsEdits() const;

private:

   /*----- types and enumerations ----*/

   typedef AssemblyDefinition BaseNode;

   /*----- member functions -----*/
      
   // Listen to writes to our definition file attribute.
   virtual bool setInternalValueInContext(
      const MPlug& plug, const MDataHandle& dataHandle, MDGContext& ctx);

   // Invoked when an assembly reference node is duplicated. 
   virtual void copyInternalData(MPxNode* srcNode);

   // Called to perform any processing before the assembly is saved.
   // The beforeSave() in this class saves the state of the active 
   // representations for all subassemblies in an attribute on the 
   // assembly node (these will be read back and used as the initial 
   // representation when the assembly is next loaded)
   void beforeSave();

   // Called to initialize node with file path of assembly definition file.
   // The postLoad() initialization in this class does the following:
   // 1) Clear the existing representation list of the assembly reference.
   // 2) Import the assembly definition file as a member of the assembly
   //    reference, and look for an assembly definition inside it.  If there
   //    is none, or more than one, an error is reported.
   // 3) Copy the list of representations from the assembly definition
   //    node to the assembly reference.
   // 4) Delete the assembly definition node.
   // 5) Activate the initial representation
   void postLoad();

   // Called on top level assemblies, to determine the initial representation 
   // for the assembly specified
   virtual MString      getInitialRep(const MObject &assembly, bool& hasInitialRep, MStatus* status = NULL) const;

   // Return the name of the default icon for the node.
   virtual MString      getDefaultIcon() const;

   // Return the name of the definition file for this assembly reference.
   MString getDefinitionFile() const;

   // Error handler when assembly definition error occurs.  Display
   // the error string corresponding to the argument id, and clear the
   // container.
   void definitionError(const MStringResourceId& id);

   // Prohibited and unimplemented.
   AssemblyReference(const AssemblyReference& obj);
   const AssemblyReference& operator=(const AssemblyReference& obj);

   /*----- static data members -----*/

   // File path to assembly definition file.
   static MObject aDefnFile;

   // String attribute for the assembly's representation namespace.
   static MObject aRepNamespace;
   
      // Array of compound attributes describing the representations.
   static MObject aRepresentations;
   
   // Representation name.
   static MObject aRepName;
   
   // String attribute for the representation label.
   static MObject aRepLabel;
   
   // String attribute for the representation type.
   static MObject aRepType;

   // String attribute for the representation data.
   static MObject aRepData;

   // String attribute for the assembly's initial representation configuration
   static MObject aInitialRep;

   /*----- data members -----*/

   // Definition file transiently set in setInternalValueInContext().
   MString           fDefnFile;
   
   // Use the definition file from the plug attribute if true, otherwise
   // use definition file from fDefnFile. 
   bool 			 fUseDefnFileAttrib;	
   
   // Pointer to an entry in the assembly definition file cache. This
   // keeps the entry alive in case another assembly reference node
   // refers to the same definition file.
   AssemblyDefinitionFileCache::EntryPtr fDefnFileCacheEntry;

   // Initial representation information, transiently set in postLoad
   assemblyReferenceInitialRep *fInitialRep;

	// Used to tell if we're in the process of updating the rep namespace.
	// Used to distinguish between cases where the NS change was initiated
	// by Maya or via the NS editor (so fUpdatingRepNamespace == true)
	// and when the repNamespace attribute was edited directly 
	// (fUpdatingRepNamespace == false).
   bool 			fUpdatingRepNamespace;
};

#endif

//-
// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//+
