#include "assemblyDefinition.h"
#include "adskRepresentationFactory.h"

#include <maya/MPxRepresentation.h>
#include <maya/MFnAssembly.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnContainerNode.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MStringArray.h>
#include <maya/MNamespace.h>
#include <maya/MGlobal.h>

#include <maya/MExternalContentInfoTable.h>
#include <maya/MExternalContentLocationTable.h>

#include <string>
#include <map>
#include <iostream>             // For CHECK_MSTATUS_AND_RETURN_IT.
using namespace std;            // For CHECK_MSTATUS macros.

#define DIMOF(array) (sizeof(array)/sizeof((array)[0]))

namespace {

//==============================================================================
// LOCAL DECLARATIONS
//==============================================================================

/*----- constants -----*/

const char* const ICON_NAME = "out_assemblyDefinition.png";

const MString REPRESENTATIONS_ATTR_UINAME( "representations" );
const MString REPRESENTATIONS_ATTR_SHORTNAME( "rep" );
const MString REPRESENTATION_DATA_ATTR_UINAME( "repData" );
const MString REPRESENTATION_DATA_ATTR_SHORTNAME( "rda" );

/*----- types and enumerations -----*/

typedef std::map<std::string, AdskRepresentationFactory*>
RepresentationFactories;

/*----- variables -----*/

RepresentationFactories repFactories;
}

//==============================================================================
// CLASS AssemblyDefinition
//==============================================================================

const MTypeId AssemblyDefinition::id(0x580000b2);

const MString AssemblyDefinition::typeName("assemblyDefinition");

MObject AssemblyDefinition::aRepresentations;
MObject AssemblyDefinition::aRepName;
MObject AssemblyDefinition::aRepLabel;
MObject AssemblyDefinition::aRepType;
MObject AssemblyDefinition::aRepData;

//------------------------------------------------------------------------------
//
void* AssemblyDefinition::creator()
{
   return new AssemblyDefinition;
}

//------------------------------------------------------------------------------
//
MStatus AssemblyDefinition::initRepresentations(
   eStorable storable,
   MObject&  aRepresentations,
   MObject&  aRepName,
   MObject&  aRepLabel,
   MObject&  aRepType,
   MObject&  aRepData
)
{
   MStatus stat;
   MFnTypedAttribute stringAttrFn;
   aRepName = stringAttrFn.create("repName", "rna", MFnData::kString);
   stat = MPxNode::addAttribute(aRepName);
   CHECK_MSTATUS_AND_RETURN_IT(stat);

   aRepLabel = stringAttrFn.create("repLabel", "rla", MFnData::kString);
   stat = MPxNode::addAttribute(aRepLabel);
   CHECK_MSTATUS_AND_RETURN_IT(stat);

   aRepType = stringAttrFn.create("repType", "rty", MFnData::kString);
   stat = MPxNode::addAttribute(aRepType);
   CHECK_MSTATUS_AND_RETURN_IT(stat);

   aRepData = stringAttrFn.create(
      REPRESENTATION_DATA_ATTR_UINAME, REPRESENTATION_DATA_ATTR_SHORTNAME,
      MFnData::kString);   
   stat = MPxNode::addAttribute(aRepData);
   CHECK_MSTATUS_AND_RETURN_IT(stat);

   // List of representations.
   MFnCompoundAttribute representationsAttrFn;
   aRepresentations = representationsAttrFn.create(
      REPRESENTATIONS_ATTR_UINAME, REPRESENTATIONS_ATTR_SHORTNAME);  
   if (storable == kNotStorable) {
      CHECK_MSTATUS(representationsAttrFn.setStorable(false));
   }
   CHECK_MSTATUS(representationsAttrFn.setArray(true));
   representationsAttrFn.addChild(aRepName);
   representationsAttrFn.addChild(aRepLabel);
   representationsAttrFn.addChild(aRepType);
   representationsAttrFn.addChild(aRepData);
   stat = MPxNode::addAttribute(aRepresentations);
   CHECK_MSTATUS_AND_RETURN_IT(stat);
   return stat;
}

//------------------------------------------------------------------------------
//
MStatus AssemblyDefinition::initialize()
{
   // Initialize our storable list of representations.
   MStatus stat = initRepresentations(
      kStorable, aRepresentations, aRepName, aRepLabel, aRepType, aRepData);
   CHECK_MSTATUS_AND_RETURN_IT(stat);

   return stat;
}

//------------------------------------------------------------------------------
//
MStatus AssemblyDefinition::uninitialize()
{
   // Delete and remove any representation factories left behind.
   RepresentationFactories::iterator i = repFactories.begin();
   for (; i != repFactories.end(); ++i) {
      delete i->second;
   }
   repFactories.clear();

   return MStatus::kSuccess;
}

//------------------------------------------------------------------------------
//
MStringArray AssemblyDefinition::registeredTypes()
{
   MStringArray repTypes;
   RepresentationFactories::const_iterator i = repFactories.begin();
   for (; i != repFactories.end(); ++i) {
      repTypes.append(MString(i->first.c_str()));
   }
   return repTypes;
}

//------------------------------------------------------------------------------
//
bool AssemblyDefinition::registerRepresentationFactory(
   AdskRepresentationFactory* f
)
{
   if (f == 0 ||
       repFactories.find(f->getType().asChar()) != repFactories.end()) {
      return false;
   }

   repFactories[f->getType().asChar()] = f;
   return true;
}

//------------------------------------------------------------------------------
//
bool AssemblyDefinition::deregisterRepresentationFactory(
   const MString& type
)
{
   RepresentationFactories::iterator found = repFactories.find(type.asChar());
   if (found == repFactories.end()) {
      return false;
   }

   delete found->second;
   repFactories.erase(found);

   return true;
}

 //------------------------------------------------------------------------------
//
AssemblyDefinition::AssemblyDefinition() : fActiveRep(0)
{}

//------------------------------------------------------------------------------
//
AssemblyDefinition::~AssemblyDefinition()
{}


//------------------------------------------------------------------------------
//
void AssemblyDefinition::getExternalContent(
   MExternalContentInfoTable& table
) const
{
   MStatus status;
   MStringArray repNames( getRepresentations( &status ) );
   const unsigned int nbReps = repNames.length();
   if (status != MS::kSuccess || nbReps == 0) {
      return;
   }

   // To be friendly to the file path editor, which assumes all paths are stored
   // verbatim in plugs, we provide the external content entries with the plug
   // name as the key.  This simplified approach can be changed if the filepath
   // editor supports a more generic interface in the future.
   //
   // This is based on assumptions (true as of this writing) that
   // representations will have, at most, a single external content item, and
   // that the key that representations give for the item is irrelevant (they
   // will read the path from the first item we pass back in setExternalContent,
   // no matter what its name is).
   const MString entryFormat( "^1s[^2s].^3s" );
   MString entryName;

   for (unsigned int i = 0; i < nbReps; ++i) {
      RepresentationPtr rep( representationFactory( repNames[ i ] ) );
      if ( rep.get() == 0 ) {
         continue;
      }

      MExternalContentInfoTable repTable;
      rep->getExternalContent( repTable );

      if ( repTable.length() == 0 ) {
         continue;
      }

      MString unusedKey;
      MString unresolvedLocation;
      MString resolvedLocation;
	  MString contextNodeFullName;
      MStringArray roles;

      // As explained above, we are only reading the first item in the table, as
      // we know that the currently implemented representations only have one,
      // at most.  The case where there is no external content has already been
      // filtered out at this point.
      if ( repTable.getEntry(
              0, unusedKey, unresolvedLocation, resolvedLocation, contextNodeFullName, roles ) !=
           MStatus::kSuccess ) {
         continue;
      }

      MString plugIndex;
      plugIndex += i;
      entryName.format( entryFormat, REPRESENTATIONS_ATTR_UINAME,
                        plugIndex, REPRESENTATION_DATA_ATTR_UINAME );

      table.addResolvedEntry(
         entryName, unresolvedLocation, resolvedLocation, contextNodeFullName, roles );
   }
}

//------------------------------------------------------------------------------
//
void AssemblyDefinition::setExternalContent(
   const MExternalContentLocationTable& table
)
{
   MStatus status;
   MStringArray repNames( getRepresentations( &status ) );
   const unsigned int nbReps = repNames.length();
   if (status != MS::kSuccess || nbReps == 0) {
      return;
   }
	
   const unsigned int nbEntries = table.length();
   MString oldActive = getActive();
   for (unsigned int i = 0; i < nbEntries; ++i) {
      MString key;
      MString location;
      table.getEntry( i, key, location );

      // The key name is set in getExternalContent, and will be along the
      // lines of
      //
      //    representations[0].repData
      //
      // The only information that is relevant for this implementation is the
      // index as the rest is implicit.  So just extract the index by finding
      // what lies between brackets.
      const int openingBracketIdx = key.index( '[' );
      const int closingBracketIdx = key.index( ']' );

      if ( openingBracketIdx < 0 || closingBracketIdx < 0 ) {
         // Key syntax got garbled, not much we can do.
         continue;
      }

      const MString repIdxStr(
         key.substring( openingBracketIdx + 1, closingBracketIdx - 1 ) );
      
      const unsigned int repIdx = repIdxStr.asUnsigned();

      if ( repIdx >= nbReps ) {
         continue;
      }

      std::auto_ptr< MPxRepresentation > rep(
         representationFactory( repNames[ repIdx ] ) );
      if ( rep.get() == 0 ) {
         continue;
      }

      // Create a table just for the representation.  The entry name is not
      // really important in this limited implementation: we know in advance
      // that the currently existing representations will only use the first
      // item, whatever its name.
      MExternalContentLocationTable repTable;
      repTable.addEntry( "Data", location );
      rep->setExternalContent( repTable );
	  
	  // Since the file has been changed,
	  // refresh representation if it is active
	  if(repNames[ repIdx ] == oldActive)
	  {
		activate(oldActive);
	  }
   }
}


//------------------------------------------------------------------------------
//
MString AssemblyDefinition::createRepresentation(
   const MString& input,
   const MString& type,
   const MString& representation,
   MDagModifier*  /* undoRedo */,
   MStatus*       status
)
{
   // Early out: unknown representation type.
   RepresentationFactories::const_iterator found =
      repFactories.find(type.asChar());
   if (found == repFactories.end()) {
       if (status) *status = MStatus::kFailure;
       return MString();
   }

   const AdskRepresentationFactory* const repFactory = found->second;

   // If it wasn't given to us, set the return representation name.
   MString newRepName = 
       (representation.numChars() > 0) ?
       representation :
       repFactory->creationName(this, input);

   // If the factory couldn't create a name for the representation,
   // report failure.
   if (newRepName.numChars() == 0) {
       if (status) *status = MStatus::kFailure;
       return MString();
   }
   
   performCreateRepresentation(
      newRepName,                             // Name
      type,                                   // Type
      repFactory->creationLabel(this, input), // Label
      repFactory->creationData(this, input)   // Data
   );
   
   if (status) *status = MStatus::kSuccess;

   return newRepName;
}

//------------------------------------------------------------------------------
//
bool AssemblyDefinition::inactivateRep()
{
   // Unload the previously-active representation (if any).
   MString oldActive = getActive();
   if (oldActive.numChars() != 0) {
      bool returnVal = fActiveRep.get() ? fActiveRep->inactivate(): false;      
      if (!returnVal) {
         return false;
      }
	  // Null out the active representation only if inactivation succeeds.
	  fActiveRep = RepresentationPtr();
   }
   return true;
}

//------------------------------------------------------------------------------
//
bool AssemblyDefinition::activateRep(const MString& representation)
{
   // Activation of an empty string is a no-op.
   if (representation.numChars() != 0) {
      fActiveRep = RepresentationPtr(representationFactory(representation));
      return fActiveRep.get() ?  fActiveRep->activate() : false;
   }
   return true;
}
   
//------------------------------------------------------------------------------
//
MString AssemblyDefinition::getActive() const
{
   return fActiveRep.get() ? fActiveRep->getName() : MString();
}

//------------------------------------------------------------------------------
//

MStringArray AssemblyDefinition::getRepresentations(MStatus* status) const
{
   MStringArray representations;
   MPlug representationsPlug(thisMObject(), aRepresentations);
   for (int i=0; i<static_cast<int>(representationsPlug.numElements()); ++i) {
      MPlug representationPlug = representationsPlug[i];
      MPlug namePlug = representationPlug.child(aRepName);

      MString name;
      MStatus status_ = namePlug.getValue(name);
      if (status_ != MS::kSuccess) {
          // Retrieving the plug name should never fail. If it did,
          // something went terribly wrong, so let's report it.
          CHECK_MSTATUS(status_);
          break;
      }
      
      representations.append(name);
   }

   // There exists no documented reason that would cause this function
   // to fail. We therefore unconditionally return success.
   if (status != 0) {
       *status = MS::kSuccess;
   }

   return representations;
}

//------------------------------------------------------------------------------
//
MString AssemblyDefinition::getRepType(const MString& repName) const
{
   return getRepAttrValue(repName, aRepType);
}

//------------------------------------------------------------------------------
//
MString AssemblyDefinition::getRepLabel(const MString& repName) const
{
   return getRepAttrValue(repName, aRepLabel);
}

//------------------------------------------------------------------------------
//
MString AssemblyDefinition::getRepData(const MString& repName) const
{
   return getRepAttrValue(repName, aRepData);
}

//------------------------------------------------------------------------------
//
void AssemblyDefinition::setRepData(
   const MString& repName,
   const MString& data
)
{
   setRepAttrValue( repName, aRepData, data );
}

//------------------------------------------------------------------------------
//
MString AssemblyDefinition::getRepNamespace() const
{
   // returning an empty string will make sure that no namespace is created for this assembly
   return MString();   
}

//------------------------------------------------------------------------------
//
MStringArray AssemblyDefinition::repTypes() const
{
   // This is the representation creation query for existing assembly
   // definitions.  Previously-created representations impose no
   // constraints to new representation creation, so simply return the
   // full list of representation types.
   return registeredTypes();
}

//------------------------------------------------------------------------------
//
MStatus AssemblyDefinition::deleteRepresentation(const MString& repName)
{   
   MStatus status;
   
   MPlug representationsPlug(thisMObject(), aRepresentations);
   const int numElements = static_cast<int>(representationsPlug.numElements());
   static MObject attribs[] = {aRepName, aRepData, aRepLabel, aRepType};

   for (int i=0; i<numElements; ++i) 
   {
      MPlug representationPlug = representationsPlug[i];
      MPlug namePlug = representationPlug.child(aRepName);
      MString value;    
      namePlug.getValue(value);
      if (value == repName) {		
		 // Keep the representation list compacted.
		 // Overwrite the item data that we want to delete by copying 
		 // all the data items -1 position where they are.
		 // Delete the last item of the list.		
         for (int j = i + 1 ; j < numElements; j++)
		 {			
			for (int k = 0; k < DIMOF(attribs); ++k){
               representationsPlug[j].child(attribs[k]).getValue(value);
			   representationsPlug[j-1].child(attribs[k]).setValue(value);				 
			}			
         }	

		 // remove the numElements-1 item.
	     MString cmd("removeMultiInstance -b true ");
         cmd += representationsPlug[numElements-1].name();
         MGlobal::executeCommand(cmd);	
		        
         break;
      }
   }
   return status;
}

//------------------------------------------------------------------------------
//
MStatus AssemblyDefinition::deleteAllRepresentations()
{  
   clearRepresentationList();

   MPlug representationsPlug(thisMObject(), aRepresentations);
   return representationsPlug.numElements() == 0 ? MStatus::kSuccess : MStatus::kFailure;  

}

//------------------------------------------------------------------------------
//
MPxRepresentation* AssemblyDefinition::representationFactory(
   const MString& name
) const
{
   MString repType = getRepType(name);

   if (repType.numChars() > 0) {
      MString repData = getRepData(name);
      RepresentationFactories::const_iterator found =
         repFactories.find(repType.asChar());
      if (found != repFactories.end()) {
         // Representations require a non-const pointer to their
         // assembly, which they will then use in a const-correct way.
         return found->second->create(
            const_cast<AssemblyDefinition*>(this), name, repData);
      }
   }
   return 0;
}

//------------------------------------------------------------------------------
//
void AssemblyDefinition::postLoad()
{
   // Activate the default representation, which is representation 0.
   MStatus status;
   MStringArray representations = getRepresentations(&status);
   const int len = representations.length();
   if (status != MS::kSuccess || len == 0) {
      return;
   }

   // If we can't activate a representation (because our parent assembly is
   // being called with activateNonRecursive()), stop now.  As of
   // 20-Nov-2012, nesting assembly definitions is not a recommended
   // workflow, but it is not prohibited.
   MFnAssembly aFn(thisMObject());
   if (!aFn.canActivate()) {
      return;
   }

   // MFnAssembly::activate() must be called to benefit from scene
   // assembly infrastructure activation services.
   aFn.activate(representations[0]);
}

//------------------------------------------------------------------------------
//
MString AssemblyDefinition::getDefaultIcon() const
{
   return MString(ICON_NAME);
}

//------------------------------------------------------------------------------
//
void AssemblyDefinition::postConstructor()
{
   // Set the assembly default icon.
   MFnDependencyNode self(thisMObject());
   MStatus status;
   MPlug iconName = self.findPlug(MString("iconName"), true, &status);
   if (status != MStatus::kSuccess) {
      return;
   }
   iconName.setValue(getDefaultIcon());
}

//------------------------------------------------------------------------------
//
void AssemblyDefinition::clearRepresentationList()
{
   MStringArray representations;
   MPlug representationsPlug(thisMObject(), aRepresentations);
   for (int i=0; i<static_cast<int>(representationsPlug.numElements()); ++i) {
      representations.append(representationsPlug[i].name());
   }   
   for (int i=0; i<static_cast<int>(representations.length()); ++i) {
      MString cmd("removeMultiInstance -b true ");
      cmd += representations[i];
      MGlobal::executeCommand(cmd);
   }   
}

//------------------------------------------------------------------------------
//
void AssemblyDefinition::performCreateRepresentation(
   const MString& name,
   const MString& type,
   const MString& label,
   const MString& data
)
{
   MPlug representationsPlug(thisMObject(), aRepresentations);
   int nbElements = representationsPlug.numElements();
   representationsPlug.selectAncestorLogicalIndex(nbElements, aRepresentations);
   MPlug namePlug = representationsPlug.child(aRepName);
   namePlug.setValue(name);

   MPlug labelPlug = representationsPlug.child(aRepLabel);
   labelPlug.setValue(label);

   // In the future, could consider making representation type as not editable.
   MPlug typePlug = representationsPlug.child(aRepType);
   typePlug.setValue(type);

   MPlug dataPlug = representationsPlug.child(aRepData);
   dataPlug.setValue(data);
}

//------------------------------------------------------------------------------
//
MString AssemblyDefinition::getRepAttrValue(
   const MString& repName,
   MObject&       repAttr
) const
{
   if (repName.numChars() == 0) {
      return MString();
   }

   MString data;
   MPlug representationsPlug(thisMObject(), aRepresentations);
   // O(n) search over all representations should not matter because
   // the number of representations is small.
   const int numElements = static_cast<int>(representationsPlug.numElements());
   for (int i=0; i<numElements; ++i) {
      MPlug representationPlug = representationsPlug[i];
      MPlug namePlug = representationPlug.child(aRepName);
      MString name;
      namePlug.getValue(name);
      if (name == repName) {
         MPlug dataPlug = representationPlug.child(repAttr);
         dataPlug.getValue(data);
         break;
      }
   }
   return data;
}

//------------------------------------------------------------------------------
//
MStatus AssemblyDefinition::setRepAttrValue(
   const MString& repName,
   MObject&       repAttr,
   const MString& data
)
{
   if (repName.numChars() == 0) {
      return MStatus::kFailure;
   }

   MPlug representationsPlug(thisMObject(), aRepresentations);
   // O(n) search over all representations should not matter because
   // the number of representations is small.
   const int numElements = static_cast<int>(representationsPlug.numElements());
   for (int i=0; i<numElements; ++i) {
      MPlug representationPlug = representationsPlug[i];
      MPlug namePlug = representationPlug.child(aRepName);
      MString name;
      namePlug.getValue(name);
      if (name == repName) {
         MPlug dataPlug = representationPlug.child(repAttr);
         dataPlug.setValue(data);
         break;
      }
   }
   return MStatus::kSuccess;
}

//------------------------------------------------------------------------------
//
MString AssemblyDefinition::setRepName(
   const MString& repName,
   const MString& newName,
   MStatus*       status
)
{
    MStatus s = setRepAttrValue(repName, aRepName, newName);
    if (status) *status = s;
    return newName;
}

//------------------------------------------------------------------------------
//
MStatus AssemblyDefinition::setRepLabel(
   const MString& repName,
   const MString& label
)
{
   return setRepAttrValue(repName, aRepLabel, label);
}

//------------------------------------------------------------------------------
//
bool AssemblyDefinition::canRepApplyEdits(const MString& representation) const
{
   if (representation.numChars() == 0) {
      return false;
   }

   // If the argument isn't the active representation, need to build a
   // temporary transient representation to ask it.
   if (getActive() != representation || fActiveRep.get() == 0) {
      RepresentationPtr ptr(representationFactory(representation));
      return (ptr.get() == 0 ? false : ptr->canApplyEdits());
   }

   return fActiveRep->canApplyEdits();
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
