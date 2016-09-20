#include "assemblyReference.h"
#include "sceneAssemblyStrings.h"
#include "assemblyReferenceInitialRep.h"

#include <maya/MPxRepresentation.h>
#include <maya/MFnAssembly.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnContainerNode.h>
#include <maya/MObjectArray.h>
#include <maya/MFileIO.h>
#include <maya/MGlobal.h>
#include <maya/MNamespace.h>

#include <maya/MExternalContentInfoTable.h>
#include <maya/MExternalContentLocationTable.h>

#include <cassert>
#include <iostream>             // For CHECK_MSTATUS_AND_RETURN_IT.
using namespace std;            // For CHECK_MSTATUS macros.

namespace {

//==============================================================================
// LOCAL DECLARATIONS
//==============================================================================

/*----- constants -----*/

const char* const ICON_NAME = "out_assemblyReference.png";

const MString DEFINITION_FILE_ATTR_UINAME( "definition" );
const MString DEFINITION_FILE_ATTR_SHORTNAME( "def" );

//==============================================================================
// LOCAL FUNCTIONS
//==============================================================================

bool isAssemblyDefinition(MObject& obj)
{
   // Require exact type match for assembly definition.  Precludes
   // having a class derived from AssemblyDefinition to be used as an
   // assembly definition, but this is an acceptable restriction.
   MFnDependencyNode fn(obj);
   return fn.typeId() == AssemblyDefinition::id;
}

}

//==============================================================================
// CLASS AssemblyReference
//==============================================================================

const MTypeId AssemblyReference::id(0x580000b1);

const MString AssemblyReference::typeName("assemblyReference");

MObject AssemblyReference::aDefnFile;
MObject AssemblyReference::aRepNamespace;
MObject AssemblyReference::aRepresentations;
MObject AssemblyReference::aRepName;
MObject AssemblyReference::aRepLabel;
MObject AssemblyReference::aRepType;
MObject AssemblyReference::aRepData;
MObject AssemblyReference::aInitialRep;

//------------------------------------------------------------------------------
//
void* AssemblyReference::creator()
{
   return new AssemblyReference;
}

//------------------------------------------------------------------------------
//
MStatus AssemblyReference::initialize()
{
   // We are not using the inheritAttributesFrom(BaseNode::typeName)
   // because we need to set them "not storable".  Representation
   // attributes are not stored because they are obtained from the
   // assembly definition.
   MStatus stat = initRepresentations(
      kNotStorable, aRepresentations, aRepName, aRepLabel, aRepType, aRepData);
   CHECK_MSTATUS_AND_RETURN_IT(stat);

   MFnTypedAttribute defnFileAttrFn;
   aDefnFile = defnFileAttrFn.create(
      DEFINITION_FILE_ATTR_UINAME, DEFINITION_FILE_ATTR_SHORTNAME,
      MFnData::kString);
   // Set the attribute as "internal" not to store it ourselves, but
   // to get setInternalValueInContext() to be called.
   CHECK_MSTATUS(defnFileAttrFn.setInternal(true));
   CHECK_MSTATUS(defnFileAttrFn.setUsedAsFilename(true));
   stat = MPxNode::addAttribute(aDefnFile);
   CHECK_MSTATUS_AND_RETURN_IT(stat);

   // Namespace for assembly representations.
   MFnTypedAttribute repNamespaceAttrFn;
   aRepNamespace = repNamespaceAttrFn.create("repNamespace", "rns", MFnData::kString);
   CHECK_MSTATUS(repNamespaceAttrFn.setInternal(true));
   stat = MPxNode::addAttribute(aRepNamespace);
   CHECK_MSTATUS_AND_RETURN_IT(stat);

   // Initial configuration (may be empty, only used for top level assemblies) 
   MFnTypedAttribute initialRepAttrFn;
   aInitialRep = initialRepAttrFn.create("initialRep", "irp", MFnData::kString);
   stat = MPxNode::addAttribute(aInitialRep);
   CHECK_MSTATUS_AND_RETURN_IT(stat);

   return stat;
}

//------------------------------------------------------------------------------
//
MStatus AssemblyReference::uninitialize()
{
   return MStatus::kSuccess;
}

//------------------------------------------------------------------------------
//
AssemblyReference::AssemblyReference() 
	: BaseNode()
	, fDefnFile()
	, fUseDefnFileAttrib(true)
	, fInitialRep(NULL)
	, fUpdatingRepNamespace(false)
{}

//------------------------------------------------------------------------------
//
AssemblyReference::~AssemblyReference()
{
	// TODO: MAYA-15544 - either restructure to remove the need for this member 
	// variable, or make it an auto_ptr. 
	assert(NULL == fInitialRep);
}

//------------------------------------------------------------------------------
//
MString AssemblyReference::createRepresentation(
   const MString& /* input */,
   const MString& /* type */,
   const MString& /* representation */,
   MDagModifier*  /* undoRedo */,
   MStatus*       status
)
{
    // Can't create a representation through an assembly reference.
    if (status != NULL) {
        *status = MStatus::kFailure;
    }

    return MString();
}


//------------------------------------------------------------------------------
//
void AssemblyReference::getExternalContent(
   MExternalContentInfoTable& table
) const
{
   // Not invoking the base class version of the method: AssemblyReference
   // exposes representations from the definition it points at, but these are
   // not direct dependencies of the reference.

   table.addUnresolvedEntry( DEFINITION_FILE_ATTR_UINAME, getDefinitionFile(), name() );
}

//------------------------------------------------------------------------------
//
void AssemblyReference::setExternalContent(
   const MExternalContentLocationTable& table
)
{
   // Could call the default implementation in MPxNode, but this will end up
   // looking up the attribute by name in order to set its value.  Here, since
   // we already have the aDefnFile attribute, it will be slightly more
   // efficient to use it, thus bypassing the search altogether.
   MString location;
   if ( !table.getLocation( DEFINITION_FILE_ATTR_UINAME, location ) ) {
      return;
   }

   MPlug defnPlug(thisMObject(), aDefnFile);
   defnPlug.setValue(location);
}


//------------------------------------------------------------------------------
//
MStringArray AssemblyReference::repTypes() const
{
   // This is the representation creation query for existing assembly
   // references.  
   // An assembly reference cannot create new representations, only use
   // those provided by its assembly definition, so return an empty
   // array.
   return MStringArray();
}

//------------------------------------------------------------------------------
//
void AssemblyReference::copyInternalData(MPxNode* srcNode)
{
    assert(dynamic_cast<AssemblyReference*>(srcNode) != 0);
    AssemblyReference* srcAssembly = static_cast<AssemblyReference*>(srcNode);

    fDefnFile           = srcAssembly->fDefnFile;
    fUseDefnFileAttrib  = srcAssembly->fUseDefnFileAttrib;
    fDefnFileCacheEntry = srcAssembly->fDefnFileCacheEntry;
}

//------------------------------------------------------------------------------
//
bool AssemblyReference::setInternalValueInContext(
   const MPlug& plug, const MDataHandle& dataHandle, MDGContext& /* ctx */
)
{
    if (plug == aDefnFile ) {

        // Skip any setAttrs done as part of file IO. If we call
        // postLoad() when we import the assembly definition 
        // container into our own container while it's not fully-defined
        // yet, it can cause havoc
        if( MFileIO::isOpeningFile() || MFileIO::isReadingFile() ) {
            return false;
        }

        // At this point the plug value has not been set yet, and we
        // can't set it here, so save a transient copy from the data handle.
        fDefnFile = dataHandle.asString();
        fUseDefnFileAttrib = false;

        // Call the MFn version of postLoad so that it goes through
        // Maya's virtual function calls rather than calling
        // AssemblyReference::postLoad directly
        MFnAssembly aFn(thisMObject());
        aFn.postLoad();    

        fUseDefnFileAttrib = true;      
    }
    else if( plug == aRepNamespace && !fUpdatingRepNamespace) {
        // Rename the Maya namespace associated to the assembly with the new repNamespace.
        // Correct the repNamespace if needed.

        // To rename the Maya namespace, there are 2 cases to get the oldNS to rename:
        // 1- If the assembly NS attribute is changed directly (i.e. someone
        // did a setAttr directly, or modified it via the AE), we get the
        // oldNS (namespace to be renamed) using the plug value, which has
        // not been set yet.
        // So query the old NS name from current state of the datablock, and 
        // the new one from the the data handle that is passed into this method.
        //
        // 2- If we are in IO, the plug value has already been set, but the 
        // namespace still has the default value given by MPxAssembly::getRepNamespace().

        MString oldNS;
        plug.getValue(oldNS);
        // Early-out if the plug value is empty: the namespace has not been created yet.
        if (0 == oldNS.numChars())
        {
            return false;
        }

        // Get the default namespace to rename.
        if( MFileIO::isOpeningFile() || MFileIO::isReadingFile() ) {
            oldNS = MPxAssembly::getRepNamespace();
        }

        MString newNS = dataHandle.asString();
        // Validate the name and only use it if valid (not "").
        // If the name is not valid, or if the user entered "" as repNamespace,
        // use the default name (assembly name +  "_NS").
        MStatus status;
        MString validNewNS = MNamespace::validateName(newNS, &status);
        if (status != MStatus::kSuccess) {
            return false;
        }

        if( validNewNS.numChars() == 0 ) {
            // defaults to the MPxAssembly implementation
            validNewNS = MPxAssembly::getRepNamespace();
        }
        if (validNewNS != newNS)
        {
            // update the value of newNS and of the data-handle
            newNS = validNewNS;
            MDataHandle* nonConstHandle  = (MDataHandle*) &dataHandle;
            nonConstHandle->set(newNS);
        }

        // Finally, tell Maya to rename namespaces.
        if( oldNS.numChars() > 0 && newNS.numChars() > 0 && oldNS != newNS) {
            status = MNamespace::renameNamespace(oldNS, newNS);
            if (status != MStatus::kSuccess) {
                // Rename failed. Set back old value.
                // Note: if rename failed, it is probably because the namespace
                // newNS already existed. But it is the responsibility of
                // the user to provide a name that does not exist.
                MDataHandle* nonConstHandle  = (MDataHandle*) &dataHandle;
                nonConstHandle->set(oldNS);
            }
        }
   }

   return false;
}


//------------------------------------------------------------------------------
//
// Handle required processing before file save 
// Maya will call this on top level assemblies only. 
//
void AssemblyReference::beforeSave()
{

   // Invoke assemblyReferenceInitialRep to store the initialRep configuration
   // onto the top level assembly. 
   // TODO: MAYA-15544 reorganize assemblyReferenceInitialRep wrapper class
   // into more a more polished interface, e.g. given its current use, 
   // writer() could be a static method. 
   assemblyReferenceInitialRep saveRep;
   saveRep.writer(thisMObject()); 
}

//------------------------------------------------------------------------------
//
// Determine the intial representation to use for the given assembly
// Maya guarantees this will be called on top level references only. 
// If an initial configuration cannot be determined, an empty string
// is returned.

MString AssemblyReference::getInitialRep(const MObject &assembly, bool& hasInitialRep, MStatus* status) const
{

	// If we have an initial representation object to query, use it to 
	// try and get the initial value.     
	if (fInitialRep)
	{
		return fInitialRep->getInitialRep(assembly, hasInitialRep);
	}  
	// Otherwise, we simply return an empty string.
	return MString();
}


//------------------------------------------------------------------------------
//
void AssemblyReference::postLoad()
{
   // Clear out our representations.
   clearRepresentationList();
   
   MFnAssembly aFn(thisMObject());

   // If no definition file is provided, we can stop here.
   MString defnFile = getDefinitionFile();
   if (defnFile.numChars() == 0) {
	  // Activate to "none" or else, on reload, the new active representation 
	  // will be the same as the old one and we won't load it.
	  aFn.activate("");
      return;
   }

   // Have we previously read that assembly definition file ? Can we
   // simply reuse its content ?
   //
   // Note that we have to take a non-const reference since the get()
   // member function is non-const!
   AssemblyDefinitionFileCache& cache =
       AssemblyDefinitionFileCache::getInstance();
   fDefnFileCacheEntry = cache.get(defnFile);

   if (!fDefnFileCacheEntry) {
       const bool fileIgnoreVersion =
           MGlobal::optionVarIntValue("fileIgnoreVersion") == 1;

       // Next, import the file into the container.  
       MStatus status = aFn.importFile(
           defnFile, NULL /*type*/, false /*preserveReferences*/,
           NULL /*nameSpace*/, fileIgnoreVersion);
       if (status != MStatus::kSuccess) {
           definitionError(rAssemblyDefnImportError);
           return;
       }

       // Clear out our representations some more.  This should be
       // completely unnecessary, since we've already cleared out the
       // representation list, which covers the case where we set the
       // definition file to the empty string and therefore exit before
       // performing the import.  Inexplicably, the representation array
       // is somehow resized back to its initial size by the call to
       // import, though it is left empty.  Entered as JIRA-10452.
       // PPT, 3-Feb-2012.
       clearRepresentationList();

       // Loop through the imported nodes and try to find an assembly
       // definition node.  The supported workflow is to have a single
       // assembly definition node in the file.
       MFnContainerNode contFn(thisMObject());
       MObjectArray members;
       status = contFn.getMembers(members);
       if (status != MStatus::kSuccess) {
           definitionError(rAssemblyDefnNotFoundError);
           return;
       }

       // Prefer safety over performance and keep looking even if we've
       // found an assembly definition node, to make sure there isn't
       // another one in the file.  Otherwise, can stop at found!=nbMembers.
       const int nbMembers = static_cast<int>(members.length());
       int found = nbMembers;
       int nbFound = 0;
       for (int i=0; i<nbMembers; ++i) {
           if (isAssemblyDefinition(members[i])) {
               found = i;
               ++nbFound;
           }
       }

       if (found==nbMembers) {
           definitionError(rAssemblyDefnNotFoundError);
           return;
       }
       else if (nbFound > 1) {
           definitionError(rMultAssemblyDefnFoundError);
           return;
       }

       // Found an assembly definition.  Copy over its attributes, which
       // at time of writing (3-May-2012) is its list of representations.
       MFnAssembly defnFn(members[found]);
       MStringArray defnRepresentations = defnFn.getRepresentations(&status);
       const int nbReps = defnRepresentations.length();
       if (status == MS::kSuccess) {
           const AssemblyDefinition* defn =
               dynamic_cast<AssemblyDefinition*>(defnFn.userNode());
           // Will succeed because of isAssemblyDefinition().
           assert(defn != 0);

           AssemblyDefinitionFileCache::RepCreationArgsList repCreationArgsList;
           for (int i=0; i<nbReps; ++i) {
               const MString repName = defnRepresentations[i];
               repCreationArgsList.push_back(
                   AssemblyDefinitionFileCache::RepresentationCreationArgs(
                       repName,
                       defn->getRepType(repName),
                       defn->getRepLabel(repName),
                       defn->getRepData(repName)));
           }
           fDefnFileCacheEntry = cache.insert(defnFile, repCreationArgsList);
       }
   
       // Get rid of imported asset definition.
       status = contFn.clear();
       if (status != MStatus::kSuccess) {
           return;
       }
   }
   
   // Found an assembly definition.  Copy over its attributes, which
   // at time of writing (3-May-2012) is its list of representations.
   const AssemblyDefinitionFileCache::RepCreationArgsList& repCreationArgslist =
       fDefnFileCacheEntry->getRepCreationArgsList();
   {
       AssemblyDefinitionFileCache::RepCreationArgsList::const_iterator it  =
           repCreationArgslist.begin();
       AssemblyDefinitionFileCache::RepCreationArgsList::const_iterator end =
           repCreationArgslist.end();
       for (;it != end; ++it) {
           performCreateRepresentation(
               it->getName(), it->getType(), it->getLabel(), it->getData());
       }
   }
   
   // If this is not a top-level assembly, lock the repNamespace attrib.
   // User should not be able to change this attribute on nested assembly
   // because otherwise it won't match the info stored in nested file.
   if(!aFn.isTopLevel())
   {
       MPlug repNamespacePlug(thisMObject(), aRepNamespace);
       repNamespacePlug.setLocked(true);
   }

   // If we can't activate a representation (because our parent assembly is
   // being called with activateNonRecursive()), stop now.
   if (!aFn.canActivate()) {
      return;
   }

   // If this is a top level assembly, initialize the 
   // initialRep configuration. This object is destroyed
   // on exit from this postLoad routine.
   // The initialRep configuration from this topLevel assembly
   // will be accessed by this assembly, and each nested subAssembly 
   // as they are activated and call getInitialRep() from within their 
   // own postLoad. 
   // TODO: MAYA-15544 reorganize assemblyReferenceInitialRep wrapper class
   // to have a clearer interface (see other comments in this file and 
   // in earlier code reviews on MAYA-15544). 
   assert(fInitialRep == NULL);
   if (aFn.isTopLevel())
   {
	   fInitialRep = new assemblyReferenceInitialRep();
	   fInitialRep->reader(thisMObject());
   }

   // Activate the initial representation
   if (!repCreationArgslist.empty()) {
	   
	   // Check if initial representation is specified
	   // Use hasInitialRep to know if it has initial representation
       // since we can have an empty string representation. 
	   bool hasInitialRep = false;
	   MString initialRep = aFn.getInitialRep(hasInitialRep);       
	   if (!hasInitialRep)
	   {
		   // No initial representation has been found, 
		   // use the default (which is the first one).
		   initialRep = repCreationArgslist.front().getName();
	   }
	   // MFnAssembly::activate() must be called to benefit from scene
	   // assembly infrastructure activation services.
	   aFn.activate(initialRep);
   }

   // We no longer need the initial representation information. 
   if (fInitialRep)
   {
	   fInitialRep->clear(thisMObject());
	   delete fInitialRep;
	   fInitialRep = NULL;
   }
}

//------------------------------------------------------------------------------
//
MString AssemblyReference::getDefaultIcon() const
{
   return MString(ICON_NAME);
}

//------------------------------------------------------------------------------
//
MString AssemblyReference::getDefinitionFile() const
{
   MString defnFile = fDefnFile;
   if (fUseDefnFileAttrib) {
      MPlug defnPlug(thisMObject(), aDefnFile);
      defnPlug.getValue(defnFile);
   }
   return defnFile;
}

//------------------------------------------------------------------------------
//
void AssemblyReference::definitionError(const MStringResourceId& id)
{
   // The following relies on the error message being formatted with
   // URI first, assembly reference name second.
   MStatus status;
   const MString defnFile = getDefinitionFile();
   MFnContainerNode contFn(thisMObject());
   MString errorString = MStringResource::getString(id, status);
   errorString.format(errorString, defnFile, contFn.name());
   MGlobal::displayError(errorString);
   contFn.clear();
}

//------------------------------------------------------------------------------
//
MStatus AssemblyReference::deleteRepresentation(const MString& repName)
{    
   // can't delete representations through an assembly reference.
   return MStatus::kFailure;
}

//------------------------------------------------------------------------------
//
MStatus AssemblyReference::deleteAllRepresentations()
{  
   // can't delete representations through an assembly reference.
   return MStatus::kFailure;
}

//------------------------------------------------------------------------------
//
MString AssemblyReference::getRepNamespace() const
{

	MString repNamespaceStr;

	MPlug repNamespacePlug(thisMObject(), aRepNamespace);
	repNamespacePlug.getValue(repNamespaceStr);
	   
	if ( !repNamespaceStr.length() ) {
		// defaults to the MPxAssembly implementation
		repNamespaceStr = MPxAssembly::getRepNamespace();
			
		// update attribute if we're reading from it
		repNamespacePlug.setValue(repNamespaceStr);
	}

   // This assembly does not support nodes in the 
   // rootNS. So we should never set the repNamespace
   // attribute to an empty string
   assert ( repNamespaceStr.length() > 0 );

   return repNamespaceStr;
}

//------------------------------------------------------------------------------
//
void AssemblyReference::updateRepNamespace( const MString& repNamespace )
{
   MPlug repNamespacePlug(thisMObject(), aRepNamespace);

   MString repCurrentNamespaceStr;
   repNamespacePlug.getValue(repCurrentNamespaceStr);

   bool prevVal = fUpdatingRepNamespace;
   fUpdatingRepNamespace = true;

   // update attribute 
   repNamespacePlug.setValue(repNamespace);
   
   fUpdatingRepNamespace = prevVal;
}

bool AssemblyReference::supportsEdits() const
{
	// Opt into Maya's edit tracking system
	return true;
}

//-
// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//+
