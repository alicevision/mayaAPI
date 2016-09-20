//-
// ==========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

//
//

//polyExporter.cpp
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MObject.h>
#include <maya/MGlobal.h>
#include <maya/MItDag.h>
#include <maya/MDagPath.h>
#include <maya/MItSelectionList.h>
#include <maya/MPlug.h>

#include <maya/MIOStream.h>
#include <maya/MFStream.h>

#include "polyExporter.h"
#include "polyWriter.h"


polyExporter::polyExporter()
{
//Summary:  constructor method; does nothing
}


polyExporter::~polyExporter() 
{ 
//Summary:  destructor method; does nothing
//
}


MStatus polyExporter::writer(const MFileObject& file,
							 const MString& /*options*/,
							 MPxFileTranslator::FileAccessMode mode) 
//Summary:	saves a file of a type supported by this translator by traversing
//			the all or selected objects (depending on mode) in the current
//			Maya scene, and writing a representation to the given file
//Args   :	file - object containing the pathname of the file to be written to
//			options - a string representation of any file options 
//			mode - the method used to write the file - export, or export active
//				   are valid values; method will fail for any other values 
//Returns:	MStatus::kSuccess if the export was successful;
//			MStatus::kFailure otherwise
{
	const MString fileName = file.fullName();

	ofstream newFile(fileName.asChar(), ios::out);
	if (!newFile) {
		MGlobal::displayError(fileName + ": could not be opened for reading");
		return MS::kFailure;
	}
	newFile.setf(ios::unitbuf);

	writeHeader(newFile);

	//check which objects are to be exported, and invoke the corresponding
	//methods; only 'export all' and 'export selection' are allowed
	//
	if (MPxFileTranslator::kExportAccessMode == mode) {
		if (MStatus::kFailure == exportAll(newFile)) {
			return MStatus::kFailure;
		}
	} else if (MPxFileTranslator::kExportActiveAccessMode == mode) {
		if (MStatus::kFailure == exportSelection(newFile)) {
			return MStatus::kFailure;
		}
	} else {
		return MStatus::kFailure;
	}

	writeFooter(newFile);
	newFile.flush();
	newFile.close();

	MGlobal::displayInfo("Export to " + fileName + " successful!");
	return MS::kSuccess;
}


bool polyExporter::haveWriteMethod() const 
//Summary:	returns true if the writer() method of the class is implemented;
//			false otherwise
//Returns:  true since writer() is implemented in this class
{
	return true;
}


bool polyExporter::haveReadMethod() const 
//Summary:	returns true if the reader() method of the class is implemented;
//			false otherwise
//Returns:  false since reader() is not implemented in this class
{
	return false;
}


bool polyExporter::canBeOpened() const 
//Summary:	returns true if the translator can open and import files;
//			false if it can only import files
//Returns:  true
{
	return true;
}


MStatus polyExporter::exportAll(ostream& os) 
//Summary:	finds and outputs all polygonal meshes in the DAG
//Args   :	os - an output stream to write to
//Returns:  MStatus::kSuccess if the method succeeds
//			MStatus::kFailure if the method fails
{
	MStatus status;

	//create an iterator for only the mesh components of the DAG
	//
	MItDag itDag(MItDag::kDepthFirst, MFn::kMesh, &status);

	if (MStatus::kFailure == status) {
		MGlobal::displayError("MItDag::MItDag");
		return MStatus::kFailure;
	}

	for(;!itDag.isDone();itDag.next()) {
		//get the current DAG path
		//
		MDagPath dagPath;
		if (MStatus::kFailure == itDag.getPath(dagPath)) {
			MGlobal::displayError("MDagPath::getPath");
			return MStatus::kFailure;
		}

		MFnDagNode visTester(dagPath);

		//if this node is visible, then process the poly mesh it represents
		//
		if(isVisible(visTester, status) && MStatus::kSuccess == status) {
			if (MStatus::kFailure == processPolyMesh(dagPath, os)) {
				return MStatus::kFailure;
			}
		}
	}
	return MStatus::kSuccess;
}


MStatus polyExporter::exportSelection(ostream& os) 
//Summary:	finds and outputs all selected polygonal meshes in the DAG
//Args   :	os - an output stream to write to
//Returns:  MStatus::kSuccess if the method succeeds
//			MStatus::kFailure if the method fails
{
	MStatus status;

	//create an iterator for the selected mesh components of the DAG
	//
	MSelectionList selectionList;
	if (MStatus::kFailure == MGlobal::getActiveSelectionList(selectionList)) {
		MGlobal::displayError("MGlobal::getActiveSelectionList");
		return MStatus::kFailure;
	}

	MItSelectionList itSelectionList(selectionList, MFn::kMesh, &status);	
	if (MStatus::kFailure == status) {
		return MStatus::kFailure;
	}

	for (itSelectionList.reset(); !itSelectionList.isDone(); itSelectionList.next()) {
		MDagPath dagPath;

		//get the current dag path and process the poly mesh on it
		//
		if (MStatus::kFailure == itSelectionList.getDagPath(dagPath)) {
			MGlobal::displayError("MItSelectionList::getDagPath");
			return MStatus::kFailure;
		}

		if (MStatus::kFailure == processPolyMesh(dagPath, os)) {
			return MStatus::kFailure;
		}
	}
	return MStatus::kSuccess;
}


MStatus polyExporter::processPolyMesh(const MDagPath dagPath, ostream& os) 
//Summary:	processes the mesh on the given dag path by extracting its geometry
//			and writing this data to file
//Args   :	dagPath - the current dag path whose poly mesh is to be processed
//			os - an output stream to write to
//Returns:	MStatus::kSuccess if the polygonal mesh data was processed fully;
//			MStatus::kFailure otherwise
{
	MStatus status;
	polyWriter* pWriter = createPolyWriter(dagPath, status);
	if (MStatus::kFailure == status) {
		delete pWriter;
		return MStatus::kFailure;
	}
	if (MStatus::kFailure == pWriter->extractGeometry()) {
		delete pWriter;
		return MStatus::kFailure;
	}
	if (MStatus::kFailure == pWriter->writeToFile(os)) {
		delete pWriter;
		return MStatus::kFailure;
	}
	delete pWriter;
	return MStatus::kSuccess;
}


bool polyExporter::isVisible(MFnDagNode & fnDag, MStatus& status) 
//Summary:	determines if the given DAG node is currently visible
//Args   :	fnDag - the DAG node to check
//Returns:	true if the node is visible;		
//			false otherwise
{
	if(fnDag.isIntermediateObject())
		return false;

	MPlug visPlug = fnDag.findPlug("visibility", &status);
	if (MStatus::kFailure == status) {
		MGlobal::displayError("MPlug::findPlug");
		return false;
	} else {
		bool visible;
		status = visPlug.getValue(visible);
		if (MStatus::kFailure == status) {
			MGlobal::displayError("MPlug::getValue");
		}
		return visible;
	}
}


void polyExporter::writeHeader(ostream& os) 
//Summary:	outputs information that needs to appear before the main data
//Args   :	os - an output stream to write to
{
	os << "";
}


void polyExporter::writeFooter(ostream& os) 
//Summary:	outputs information that needs to appear after the main data
//Args   :	os - an output stream to write to
{
	os << "";
}

