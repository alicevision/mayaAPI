//-
// ==========================================================================
// Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

//
//

//polyRawExporter.cpp
#include <maya/MFnPlugin.h>
#include <maya/MDagPath.h>
#include <maya/MIOStream.h>
#include <maya/MFStream.h>

#include "polyRawExporter.h"
#include "polyRawWriter.h"

polyRawExporter::~polyRawExporter() 
{ 
//Summary:  destructor method; does nothing
//
}

     
void* polyRawExporter::creator() 
//Summary:  allows Maya to allocate an instance of this object
{
	return new polyRawExporter();
}


MString polyRawExporter::defaultExtension () const 
//Summary:	called when Maya needs to know the preferred extension of this file
//			format.  For example, if the user tries to save a file called 
//			"test" using the Save As dialog, Maya will call this method and 
//			actually save it as "test.x3d". Note that the period should *not* 
//			be included in the extension.
//Returns:  "raw"
{
	return MString("raw");
}


MStatus initializePlugin(MObject obj)
//Summary:	registers the commands, tools, devices, and so on, defined by the 
//			plug-in with Maya
//Returns:	MStatus::kSuccess if the registration was successful;
//			MStatus::kFailure otherwise
{
	MStatus status;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "4.5", "Any");

	// Register the translator with the system
	//
	status =  plugin.registerFileTranslator("RawText",
											"",
											polyRawExporter::creator,
											"",
											"option1=1",
											true);
	if (!status) {
		status.perror("registerFileTranslator");
		return status;
	}

	return status;
}


MStatus uninitializePlugin(MObject obj) 
//Summary:	deregisters the commands, tools, devices, and so on, defined by the 
//			plug-in
//Returns:	MStatus::kSuccess if the deregistration was successful;
//			MStatus::kFailure otherwise
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status =  plugin.deregisterFileTranslator("RawText");
	if (!status) {
		status.perror("deregisterFileTranslator");
		return status;
	}

	return status;
}


void polyRawExporter::writeHeader(ostream& os) 
//Summary:	outputs legend information before the main data
//Args   :	os - an output stream to write to
{
	os << "Legend:\n"
	   << "Delimiter = TAB\n"
	   << "() = coordinates\n"
	   << "[] = vector\n"
	   << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n";
}


polyWriter* polyRawExporter::createPolyWriter(const MDagPath dagPath, MStatus& status) 
//Summary:	creates a polyWriter for the raw export file type
//Args   :	dagPath - the current polygon dag path
//			status - will be set to MStatus::kSuccess if the polyWriter was 
//					 created successfully;  MStatus::kFailure otherwise
//Returns:	pointer to the new polyWriter object
{
	return new polyRawWriter(dagPath, status);
}
