#include "sceneAssemblyStrings.h"
#include "adskRepresentationCmd.h"
#include "assemblyDefinition.h"

#include <maya/MSyntax.h>
#include <maya/MGlobal.h>

#include <iostream>
#include <string>

#include <ciso646>
#if defined( _WIN32 ) || defined( _LIBCPP_VERSION )
#include <unordered_map>
#define ADSTD std
#else
// Note:
// We found tr1::unordered_map::begin() is O(n) on Linux. 
// The C++11 standard clearly states that begin() must be O(1) 
// for all std containers This means that the platform 
// implementation of tr1::unordered_map is not C++11 compliant yet.
// If meet performance issue with tr1::unordered_map, suggest to 
// use boost library, which is cross-platform and boost::unordered_map
// is C++11 compliant.
#include <tr1/unordered_map>
#define ADSTD std::tr1
#endif

using namespace std;

namespace {

//==============================================================================
// LOCAL DECLARATIONS
//==============================================================================

/*----- classes -----*/

class RegistryEntry {

public:

   /*----- types and enumerations -----*/

   // Copy ctor and assignment optor not defined, as
   // compiler defaults (memberwise default ctor, memberwise copy,
   // memberwise assignment, respectively) are fine.
   RegistryEntry() : fTypeLabel() {}  
   ~RegistryEntry() {}
  
   MString typeLabel() const { return fTypeLabel; }
   MString aeRepresentationProc() const { return fAERepresentationProc; }
   
   void    setTypeLabel(const MString& typeLabel) {
      fTypeLabel = typeLabel;
   }  
   
   void    setAERepresentationProc(const MString& proc) {
      fAERepresentationProc = proc;
   }  

private: 
   MString fTypeLabel;
   MString fAERepresentationProc;    
};

/*----- types and enumerations -----*/

typedef ADSTD::unordered_map< std::string, RegistryEntry > AdskRepresentationRegistry;

/*----- variables -----*/

AdskRepresentationRegistry fRegistry;       // Representation manager registry.

}

//==============================================================================
// CLASS AdskRepresentationCmd
//==============================================================================


//------------------------------------------------------------------------------
//
AdskRepresentationCmd::AdskRepresentationCmd()
   : fMode(kEdit)
{}

//------------------------------------------------------------------------------
//
AdskRepresentationCmd::~AdskRepresentationCmd()
{}

//------------------------------------------------------------------------------
//
void* AdskRepresentationCmd::creator()
{
    return new AdskRepresentationCmd();
}

//------------------------------------------------------------------------------
//
MSyntax AdskRepresentationCmd::cmdSyntax()
{
	MSyntax syntax;
    
    syntax.addFlag("-tl",  "-typeLabel",      MSyntax::kString);   
	syntax.addFlag("-rcp", "-updateAERepresentationProc",      MSyntax::kString);   
	syntax.addFlag("-lrt", "-listRepTypes",      MSyntax::kString);
  
	syntax.setObjectType(MSyntax::kStringObjects);
    syntax.enableQuery(true);
    syntax.enableEdit(true);

    return syntax;
}

//------------------------------------------------------------------------------
//
const char* AdskRepresentationCmd::name()
{
	return "adskRepresentation";
}

//------------------------------------------------------------------------------
//
MStatus AdskRepresentationCmd::doIt(const MArgList& args)
{
   MStatus status;
    
   MArgDatabase argsDb(syntax(), args, &status);
   if (!status) return status;

   if (argsDb.isEdit()) {
      if (argsDb.isQuery()) {
         displayError( MStringResource::getString( rEditQueryError, status));
         return MS::kFailure;
      }
      fMode = kEdit;
   }
   else if (argsDb.isQuery()) {
      fMode = kQuery;
   }

   fTypeLabelFlag.parse( argsDb, "-typeLabel");   
   fAERepresentationProcFlag.parse( argsDb, "-updateAERepresentationProc"); 
   
   fListRepTypesFlag.parse( argsDb, "-listRepTypes");  
   if (!fListRepTypesFlag.isModeValid(fMode)) {
      displayError( MStringResource::getString( rListRepTypesFlagError, status));
      return MS::kFailure;
   }

   MStringArray objs;   
   status = argsDb.getObjects(objs);
   CHECK_MSTATUS_AND_RETURN_IT(status);
   
   MString repType;
   
   if (objs.length() == 0) {
      if(needObjectArg()) {
         displayError( MStringResource::getString( rRepTypeObjArgError, status));
         return MS::kFailure;
      }
   } else {
      repType = objs[0];
   }
   
    
   switch (fMode) {     
      case kEdit:     status = doEdit(repType);  break;
      case kQuery:    status = doQuery(repType); break;
   }
    
   return status;
}

//------------------------------------------------------------------------------
//
MStatus AdskRepresentationCmd::doEdit(const MString& repType)
{
   MStatus status;  
   
   if (fTypeLabelFlag.isSet()) {
      assert(fTypeLabelFlag.isArgValid());
      
      AdskRepresentationRegistry::iterator found = fRegistry.find(repType.asChar());
      RegistryEntry entry = (found == fRegistry.end()) ? RegistryEntry() : found->second;
      entry.setTypeLabel(fTypeLabelFlag.arg());
      fRegistry[string(repType.asChar())] = entry;
      setResult(fTypeLabelFlag.arg());
   }
   else if (fAERepresentationProcFlag.isSet()) {
      assert(fAERepresentationProcFlag.isArgValid());
      
      AdskRepresentationRegistry::iterator found = fRegistry.find(repType.asChar());
      RegistryEntry entry = (found == fRegistry.end()) ? RegistryEntry() : found->second;
      entry.setAERepresentationProc(fAERepresentationProcFlag.arg());
      fRegistry[string(repType.asChar())] = entry;
      setResult(fAERepresentationProcFlag.arg());
   }   

   return MS::kSuccess;
}

//------------------------------------------------------------------------------
//
MStatus AdskRepresentationCmd::doQuery(const MString& repType)
{    
    if (fTypeLabelFlag.isSet()) {   

       // Get the representation type label from the registry. 
       AdskRepresentationRegistry::const_iterator found = fRegistry.find(repType.asChar());
       if (found == fRegistry.end()) {
          return MS::kFailure;
       }            
       setResult(found->second.typeLabel());
    }else if (fAERepresentationProcFlag.isSet()) {   

       // Get the representation procedure from the registry. 
       AdskRepresentationRegistry::const_iterator found = fRegistry.find(repType.asChar());
       if (found == fRegistry.end()) {
          return MS::kFailure;
       }             
       setResult(found->second.aeRepresentationProc());     
	}else if (fListRepTypesFlag.isSet()) {            
        MStringArray representations = AssemblyDefinition::registeredTypes();
		setResult(representations);
    } 
	   
    return MS::kSuccess;
}

//------------------------------------------------------------------------------
//
bool AdskRepresentationCmd::needObjectArg() const
{
   return !fListRepTypesFlag.isSet();
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
