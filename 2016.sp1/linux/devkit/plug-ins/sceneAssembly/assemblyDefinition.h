#ifndef _assemblyDefinition_h_
#define _assemblyDefinition_h_

///////////////////////////////////////////////////////////////////////////////
//
// AssemblyDefinition
//
// Autodesk reference implementation scene assembly definition node.
//
// Objects of this type can be used by assembly reference nodes to
// define their representations and initial attribute values for
// renderable representation, etc.  They provide a single, sharable
// point where representations and their properties can be defined, in
// order to potentially share them through many assembly references.
//
// Alternately, if no re-use through assembly references is desired,
// assembly definitions can be used by themselves to describe a scene
// element with multiple representations.  They can then be used to
// hierarchically build a scene with assembly definitions containing
// other assembly definitions.
// 
// This class supports a registry mechanism to register factories for
// representations.  These factory objects create a representation of
// the appropriate type when a representation is activated.  See the
// representation factory base class, AdskRepresentationFactory, for
// more details.
//
////////////////////////////////////////////////////////////////////////////////

#include <maya/MPxAssembly.h>

#include <memory>               // for auto_ptr

class MPxRepresentation;
class AdskRepresentationFactory;

class AssemblyDefinition : public MPxAssembly
{
public:

   /*----- types and enumerations -----*/

   typedef std::auto_ptr<MPxRepresentation> RepresentationPtr;

   /*----- static member functions -----*/

   static void* creator();
   static MStatus initialize();
   static MStatus uninitialize();

   // Register a representation factory.  On successful registration,
   // returns true, and ownership of the representation factory is
   // transferred to the registry.  The representation type is
   // obtained from the factory object.  If a factory for the type
   // already existed, or if the argument pointer is null, will return
   // false, and ownership is not transferred to the registry.
   static bool registerRepresentationFactory(AdskRepresentationFactory* f);

   // Deregister and delete a representation factory of the given
   // type.  Return whether the type was found in the registry.
   static bool deregisterRepresentationFactory(const MString& type);

   // Return a string array of representation types for which 
   // there are registered factories. 
   static MStringArray registeredTypes();

   static const MTypeId id;
   static const MString typeName;

   /*----- member functions -----*/
      
   AssemblyDefinition();
   ~AssemblyDefinition();

   virtual void getExternalContent( MExternalContentInfoTable& table ) const;
   virtual void setExternalContent(
      const MExternalContentLocationTable& table
   );

   // Create representation.  The input to the method is representation type-
   // specific, and is used by the representation type-specific representation
   // factory to create the representation name, label, and data.  The undo /
   // redo pointer is unused.
   virtual MString createRepresentation(
      const MString& input,
      const MString& type,
      const MString& representation,
      MDagModifier*  undoRedo,
      MStatus*       status
   );
   // Activate and inactivate a representation.  The implementation in
   // this class forwards the call to the corresponding
   // MPxRepresentation method in our active representation object.
   virtual bool         activateRep(const MString& representation);
   virtual bool         inactivateRep();
   
   virtual MString      getActive() const;
   
   // For each representation, returns the name of the representation.
   virtual MStringArray getRepresentations(MStatus* status = NULL) const;
   virtual MString      getRepType(const MString& representation) const;
   virtual MString      getRepLabel(const MString& representation) const;
   virtual MStringArray repTypes() const;  
   virtual MStatus      deleteRepresentation(const MString& representation);
   virtual MStatus      deleteAllRepresentations();
   virtual MString      getRepNamespace() const;

   virtual MString      setRepName(
      const MString& representation,
      const MString& newName,
      MStatus*       status
   );
   virtual MStatus      setRepLabel(
      const MString& representation,
      const MString& label
   );

   virtual bool         canRepApplyEdits(const MString& representation) const;

   MString getRepData(const MString& representation) const;
   void setRepData(
      const MString& representation,
      const MString& data
   );
   
protected:

   /*----- types and enumerations ----*/

   enum eStorable {kNotStorable = 0, kStorable};

   /*----- static member functions -----*/

   // Initialize representations attributes.  The first argument is 
   // whether representations are storable or not.
   static MStatus initRepresentations(
      eStorable storable,
      MObject&  aRepresentations,
      MObject&  aRepName,
      MObject&  aRepLabel,
      MObject&  aRepType,
      MObject&  aRepData
   );

   /*----- member functions -----*/

   // Add a representation to the representations multi-attribute.
   void performCreateRepresentation(
      const MString& name,
      const MString& type,
      const MString& label,
      const MString& data
   );

   // Create a representation object to manage representation behavior
   // while it's active.
   MPxRepresentation* representationFactory(const MString& name) const;

   // Return the name of the default icon for the node.
   virtual MString      getDefaultIcon() const;

   // Clears out representation list, without performing
   // representation inactivation.
   void clearRepresentationList();

   // Post-construction initialization, called by the Maya infrastructure.
   virtual void postConstructor();

private:

   /*----- member functions -----*/
      
   // Prohibited and unimplemented.
   AssemblyDefinition(const AssemblyDefinition& obj);
   const AssemblyDefinition& operator=(const AssemblyDefinition& obj);

   // Called to initialize node by activating default representation.
   void postLoad();
  
   // Utility to get representation label, type, or data attribute
   // values, given name.  Second argument is chosen attribute.
   MString getRepAttrValue(
      const MString& representation,
      MObject&       repAttr
   ) const;

   // Utility to set representation name or label, given name.  Second
   // argument is chosen attribute.
   MStatus setRepAttrValue(
      const MString& representation,
      MObject&       repAttr,
      const MString& data
   );

   /*----- static data members -----*/

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

   /*----- data members -----*/

   // Active representation object.
   RepresentationPtr fActiveRep;  
   };

#endif

//-
//*****************************************************************************
// Copyright 2013 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy
// form.
//*****************************************************************************
//+
