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

//polyX3DExporter.cpp

#include <maya/MIOStream.h>
#include <maya/MDagPath.h>
#include <maya/MStatus.h>
#include <maya/MGlobal.h>
#include <maya/MFnPlugin.h>

#include "polyX3DExporter.h"
#include "polyX3DWriter.h"

//Macros
//
//XML header related info
#define XMLVERSION 1.0
#define XMLENCODING "UTF-8"


polyX3DExporter::~polyX3DExporter() 
{ 
//Summary:  destructor method - does nothing
//
}

     
void* polyX3DExporter::creator() 
//Summary:  allows Maya to allocate an instance of this object
{
	return new polyX3DExporter();
}


MString polyX3DExporter::defaultExtension () const 
//Summary:	called when Maya needs to know the preferred extension of this file
//			format.  For example, if the user tries to save a file called 
//			"test" using the Save As dialog, Maya will call this method and 
//			actually save it as "test.x3d". Note that the period should *not* 
//			be included in the extension.
//Returns:  "x3d"
{
	return MString("x3d");
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
	status =  plugin.registerFileTranslator("X3D",
											"",
											polyX3DExporter::creator,
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

	status =  plugin.deregisterFileTranslator("X3D");
	if (!status) {
		status.perror("deregisterFileTranslator");
		return status;
	}

	return status;
}


void polyX3DExporter::writeHeader(ostream& os) 
//Summary:	outputs the required opening X3D tags
//Args   :	os - an output stream to write to
{
	//output required tags:  XML, X3D, and Scene
	//
	os << "<?xml version=\"" << XMLVERSION 
	   << "\" encoding=\"" << XMLENCODING 
	   << "\"?>\n"
	   << "<!DOCTYPE X3D PUBLIC \"http://www.web3D.org/TaskGroups/x3d/translation/x3d-compact.dtd\" "
	   << "\"/www.web3d.org/TaskGroups/x3d/translation/x3d-compact.dtd\">\n"
	   << "<X3D>\n"
	   << "\t<Scene>\n";
}


void polyX3DExporter::writeFooter(ostream& os)
//Summary:	outputs the required closing X3D tags
//Args   :	os - an output stream to write to
{
	os << "\t</Scene>\n"
	   << "</X3D>\n";
}


polyWriter* polyX3DExporter::createPolyWriter(const MDagPath dagPath, MStatus& status) 
//Summary:	creates a polyWriter for the X3D export file type
//Args   :	dagPath - the current polygon dag path
//			status - will be set to MStatus::kSuccess if the polyWriter was 
//					 created successfully;  MStatus::kFailure otherwise
//Returns:	pointer to the new polyWriter object
{
	return new polyX3DWriter(dagPath, status);
}
