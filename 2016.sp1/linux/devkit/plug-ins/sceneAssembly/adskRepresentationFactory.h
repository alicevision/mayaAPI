#ifndef _adskRepresentationFactory_h_
#define _adskRepresentationFactory_h_

#include <maya/MString.h>

class MPxAssembly;
class MPxRepresentation;

/*==============================================================================
 * CLASS AdskRepresentationFactory
 *============================================================================*/

// <summary>Base class for representation factory objects.</summary>
//
// This class provides a uniform interface for objects that can create
// representations.

class AdskRepresentationFactory {

public: 

   /*----- member functions -----*/
      
   // Create a representation object of the given name, with the given
   // data, and set its assembly to be the argument assembly.  The
   // type of the created representation is determined by the derived
   // factory class.  The factory object does not keep ownership of
   // the returned representation; it is up to the caller to delete
   // it.  If the assembly pointer is null, a null representation
   // pointer should be returned.
   virtual MPxRepresentation* create(
      MPxAssembly* assembly, const MString& name, const MString& data
   ) const = 0;

   // Given representation type-specific creation input, return a name
   // for a representation of the type supported by this factory.
   virtual MString creationName(
      MPxAssembly* assembly, const MString& input
   ) const = 0;

   // Given representation type-specific creation input, return a UI
   // label for a representation of the type supported by this factory.
   virtual MString creationLabel(
      MPxAssembly* assembly, const MString& input
   ) const = 0;

   // Given representation type-specific creation input, return
   // persistent data the assembly should initially store for a
   // representation of the type supported by this factory.
   virtual MString creationData(
      MPxAssembly* assembly, const MString& input
   ) const = 0;

   // Return the type name of the representation created by this factory.
   MString getType() const { return fType; }

   virtual ~AdskRepresentationFactory() {}

protected: 

   /*----- member functions -----*/

   AdskRepresentationFactory(const MString& type) : fType(type) {}

private: 

   /*----- member functions -----*/

   // Don't copy or assign.
   AdskRepresentationFactory(const AdskRepresentationFactory&);
   AdskRepresentationFactory& operator=(const AdskRepresentationFactory&);

   /*----- data members -----*/

   const MString fType;
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
