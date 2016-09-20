#ifndef _assemblyReferenceInitialRep_h_
#define _assemblyReferenceInitialRep_h_

/*==============================================================================
 * This is a utility class that manages the initial representation 
 * configuration information for an assembly references:
 *   -formatting of active representation configuration for output before the 
 *    assembly reference is saved
 *   -interpreting the previously saved configuration data when assembly reference 
 *    is initialized
 *   -querying the initial representation settings
 *   -clearing the data when it is no longer required 
 *
 * Most of the implementation is actually written in python, this class is a wrapper
 * around calls into that class - see assemblyReferenceInitialRep.py
 * 
 *============================================================================*/

class MString;
class MPxAssembly;
class MObject;

class assemblyReferenceInitialRep {

  public:
	assemblyReferenceInitialRep();
	virtual ~assemblyReferenceInitialRep();

	bool     reader(const MObject &rootAssembly);
	bool	 writer(const MObject &rootAssembly) const;
	MString  getInitialRep(const MObject &targetAssembly, bool &hasInitialRep) const;
	bool	 clear(const MObject &rootAssembly) const;

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
