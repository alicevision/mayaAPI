//-
// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//+
//	File Name: exampleCameraSetViewCmd.cpp
//	Author: Dave Immel
//
//	Description:
//		A command used for testing cameraSet based drawing into a single view
//		See exampleCameraSetViewCmd.h for a description.
//
//	Date:	May 01 2008
//
//	Description:
//

#include "exampleCameraSetViewCmd.h"
#include "exampleCameraSetView.h"

#include <maya/MItDag.h>
#include <maya/MGlobal.h>
#include <maya/MSyntax.h>
#include <maya/MArgParser.h>
#include <maya/MArgList.h>
#include <maya/MDagPath.h>
#include <maya/MFnCameraSet.h>
#include <maya/MObject.h>
#include <maya/MSelectionList.h>
#include <maya/MFnCamera.h>
#include <maya/MIOStream.h> 
#include <maya/MPx3dModelView.h> 

#include <iostream>

exampleCameraSetViewCmd::exampleCameraSetViewCmd()
: 	MPxModelEditorCommand()
//
//	Description:
//		Class constructor.
//
{
}

exampleCameraSetViewCmd::~exampleCameraSetViewCmd()
//
//	Description:
//		Class destructor.
//
{
}

void* exampleCameraSetViewCmd::creator()
//
//	Description:
//		Create the command.
//
{
    return new exampleCameraSetViewCmd();
}

MPx3dModelView *exampleCameraSetViewCmd::userView()
//
//	Description:
//		Create the MPx3dModelPanel used by this command.
//
{
    return new exampleCameraSetView();
}

MStatus exampleCameraSetViewCmd::appendSyntax()
//
//	Description:
//		Add syntax to the command. All of the parent syntax is added
//		before this call is made.
//
{
	MStatus ReturnStatus;

	MSyntax theSyntax = syntax(&ReturnStatus);
	if (MS::kSuccess != ReturnStatus) {
		MGlobal::displayError("Could not get the parent's syntax");
		return ReturnStatus;
	}

	theSyntax.addFlag(kTestMultiPackInitFlag, 
					  kTestMultiPackInitFlagLong);

	theSyntax.addFlag(kTestMultiPackResultsFlag, 
					  kTestMultiPackResultsFlagLong);

	theSyntax.addFlag(kTestMultiPackClearFlag, 
					  kTestMultiPackClearFlagLong);

	return ReturnStatus;
}

MStatus exampleCameraSetViewCmd::doEditFlags()
//
//	Description:
//		Handle edits for flags added by this class.
//		If the flag is unknown, return MS::kSuccess and the parent class
//		will attempt to process the flag. Returning MS::kUnknownParameter
//		will cause the parent class to process the flag.
//
{
	MPx3dModelView *user3dModelView = modelView();
	if (NULL == user3dModelView) {
		MGlobal::displayError("NULL == user3dModelView!");
		return MS::kFailure;
	}

	//	This is now safe to do, since the above test passed.
	//
	exampleCameraSetView *dView = (exampleCameraSetView *)user3dModelView;

	MArgParser argData = parser();
	if (argData.isFlagSet(kTestMultiPackInitFlag)) {
		return initTests(*dView);
	} else if (argData.isFlagSet(kTestMultiPackResultsFlag)) {
		return testResults(*dView);
	} else if (argData.isFlagSet(kTestMultiPackClearFlag)) {
		return clearResults(*dView);
	}

	return MS::kUnknownParameter;
}

MStatus exampleCameraSetViewCmd::clearResults(MPx3dModelView &view)
//
//	Description:
//
{
	MObject cstObj = MObject::kNullObj;
	MStatus stat = view.getCameraSet(cstObj);
	if (stat == MS::kSuccess)
	{
		view.setCameraSet(MObject::kNullObj);
		MGlobal::deleteNode(cstObj);
	}
	fCameraList.clear();
	return MS::kSuccess;
}

MStatus exampleCameraSetViewCmd::initTests(MPx3dModelView &view)
//
//	Description:
//
{
	MGlobal::displayInfo("exampleCameraSetViewCmd::initTests");

	clearResults(view);

	//	Add every camera into the scene. Don't change the main camera,
	//	it is OK that it gets reused.
	//
	MFnCameraSet cstFn;
	MObject cstObj = cstFn.create();
	MDagPath cameraPath;
	MStatus status = MS::kSuccess;
	MItDag dagIterator(MItDag::kDepthFirst, MFn::kCamera);
	for (; !dagIterator.isDone(); dagIterator.next()) {
		if (!dagIterator.getPath(cameraPath)) {
			continue;
		}

		MFnCamera camera(cameraPath, &status);
		if (MS::kSuccess != status) {
			continue;
		}

		fCameraList.append(cameraPath);
		if (cstFn.appendLayer(cameraPath, MObject::kNullObj) != MS::kSuccess)
			MGlobal::displayError("Could not add camera layer!");
		MGlobal::displayInfo(camera.fullPathName());
	}

	if (MS::kSuccess !=
		view.setCameraSet(cstObj)) {
		MGlobal::displayError("Could not set the cameraSet");
		return MS::kFailure;
	}

	view.refresh();
	return MS::kSuccess;
}

MStatus exampleCameraSetViewCmd::testResults(MPx3dModelView &view)
{
	MObject cstObj = MObject::kNullObj;

	MStatus stat = view.getCameraSet(cstObj);
	if (stat != MS::kSuccess)
		return stat;

	cout << "fCameraList.length() = " << fCameraList.length() << endl;
	cout << "fCameraList = " << fCameraList << endl;

	MFnCameraSet cstFn(cstObj);
	unsigned int numLayers = cstFn.getNumLayers();
	cout << "view.cameraSet.numLayers = " << numLayers << endl;
	cout << "Cameras:" << endl;
	for (unsigned int i=0; i<numLayers; i++)
	{
		MDagPath camPath;
		cstFn.getLayerCamera(i, camPath);
		camPath.extendToShape();
		cout << "    " << camPath.fullPathName() << endl;
	}

	return MS::kSuccess;
}

