#include "assemblyReferenceInitialRep.h"
#include <maya/MGlobal.h>
#include <maya/MFnAssembly.h>


namespace {

//==============================================================================
// LOCAL DECLARATIONS
//==============================================================================

	// Preamble added to python calls
	const MString pyPreamble1("import maya.app.sceneAssembly.assemblyReferenceInitialRep as iRep\n");
	const MString pyPreamble2("ir=iRep.assemblyReferenceInitialRep()\n");

    // Global enable/disable of the feature
    // TODO: could add an env variable or other external mechanism to control this
    bool fsEnable = true;

}


//------------------------------------------------------------------------------
//
assemblyReferenceInitialRep::assemblyReferenceInitialRep()
{
}

//------------------------------------------------------------------------------
//
assemblyReferenceInitialRep::~assemblyReferenceInitialRep()
{
}


//------------------------------------------------------------------------------
// Read in the initial representation data from a top level assembly node
// Note: it is expected that reader() will be called before calls to 
// getInitialRep attempt to acces the data.
//
bool assemblyReferenceInitialRep::reader(const  MObject &rootAssembly) 
{

	if (!fsEnable)
		return true;

	MFnAssembly aFn(rootAssembly);
	MStatus status;
	MString pyCmd;

	// Invoke: assemblyReferenceInitialRep.reader(rootAssemblyName)
	pyCmd += pyPreamble1;
	pyCmd += pyPreamble2;
	pyCmd += "ir.reader(\'";
	pyCmd += aFn.name();
	pyCmd += "\')\n";

	status = MGlobal::executePythonCommand(pyCmd);
	return status == MStatus::kSuccess;
}

//------------------------------------------------------------------------------
// Write out the initial representation for a top level assembly node
//
bool assemblyReferenceInitialRep::writer(const MObject &rootAssembly) const
{
	if (!fsEnable)
		return true;

	MFnAssembly aFn(rootAssembly);
	MStatus status;
	MString pyCmd;

	// Invoke: assemblyReferenceInitialRep.writer(rootAssemblyName)
	pyCmd += pyPreamble1;
	pyCmd += pyPreamble2;
	pyCmd += "ir.writer(\'";
	pyCmd += aFn.name();
	pyCmd += "\')\n";

	status = MGlobal::executePythonCommand(pyCmd);
	return status == MStatus::kSuccess;
}

//------------------------------------------------------------------------------
//
// Get the initial representation for an assembly node
// Note: it is expected that reader() will have been previously called
// to initialize the initialRep data.

MString assemblyReferenceInitialRep::getInitialRep(const MObject &targetAssembly, bool &hasInitialRep) const
{

	if (!fsEnable)
		return MString();

	MFnAssembly aFn(targetAssembly);
	MString result;
    MStringArray stringArrayResult;

	// Invoke: assemblyReferenceInitialRep.getInitialRep(targetAssemblyName)

	// Note: to get string result from python, it must execute a single command,
	// so wrap it in a proc

	MString pyCmd1;
	pyCmd1 = "def tempGetInitialRepProc():\n";
	pyCmd1 += "\t" + pyPreamble1;
	pyCmd1 += "\t" + pyPreamble2;
	pyCmd1 += "\treturn ir.getInitialRep(\'";
	pyCmd1 += aFn.name();
	pyCmd1 += "\')\n";
	MGlobal::executePythonCommand(pyCmd1);

	MString pyCmd2;
	pyCmd2 = "tempGetInitialRepProc()";
	MGlobal::executePythonCommand(pyCmd2, stringArrayResult);
    // The Python boolean value has been converted to a string since it's returned in a MStringArray.
    hasInitialRep = stringArrayResult[1] == "True";
        
	return stringArrayResult[0];
}

//------------------------------------------------------------------------------
// Clear the initial representation data that for a top level assembly node
// Note: The data would have been previously read in by the reader() method.
// This method can be called when the data is not longer required (subsequent
// calls to getInitialRep will not return any data).
//
bool assemblyReferenceInitialRep::clear(const MObject &rootAssembly) const
{
	if (!fsEnable)
		return true;

	MFnAssembly aFn(rootAssembly);
	MStatus status;
	MString pyCmd;

	// Invoke: assemblyReferenceInitialRep.clear(rootAssemblyName)
	pyCmd += pyPreamble1;
	pyCmd += pyPreamble2;
	pyCmd += "ir.clear(\'";
	pyCmd += aFn.name();
	pyCmd += "\')\n";

	status = MGlobal::executePythonCommand(pyCmd);
	return status == MStatus::kSuccess;
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
