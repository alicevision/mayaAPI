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

//	This plugin provides the 'saveSwatches' command which takes a list of
//	render nodes and saves out their swatches as 64x64 PNG images.
//
//	The plugin demonstrates the following:
//
//	- how to find a Maya control's QWidget from its name
//	- how to grab a snapshot of a control and save it to an image file

//	Must ensure that at least one Qt header is included before anything
//	else.
#include <QtGui/QPixmap>
#include <QtCore/QString>
#include <QtGui/QWidget>

#include <maya/MArgList.h>
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include <maya/MQtUtil.h>
#include <maya/MString.h>

#include <saveSwatchesCmd.h>

const MString  SaveSwatchesCmd::commandName("saveSwatches");


MStatus SaveSwatchesCmd::doIt(const MArgList& args)
{
	//	If we weren't passed any arguments, display a help string.
	if (args.length() == 0) {
		MGlobal::displayError(
			MString("Usage: ")
			+ SaveSwatchesCmd::commandName
			+ " renderNode [renderNode ...]"
		);
		return MS::kInvalidParameter;
	}

	//	Create a window with a swatchDisplayPort in it.
	MString	swatchPortName = MGlobal::executeCommandStringResult(
		"window -h 64 -w 64 swatchGrabber;"
		"rowLayout -nc 1;"
		"string $swatchPort = `swatchDisplayPort -w 64 -h 64 -rs 64`;"
		"showWindow;"
		"$swatchPort = $swatchPort;"
	);

	//	Find the swatchDisplayPort's QWidget.
	QWidget*  swatchPort = MQtUtil::findControl(swatchPortName);

	if (swatchPort) {
		//	Step through each render node and take its snapshot.
		for (unsigned int i = 0; i < args.length(); ++i) {
			MString	renderNode = args.asString(i);

			//	Assign the material node to the swatchDisplayPort which
			//	will automatically render its swatch.
			MGlobal::executeCommand(
				MString("swatchDisplayPort -e -sn ") + renderNode + " " + swatchPortName
			);

			//	Swatch rendering takes place during idle events, so let's
			//	give it a chance to run.
			MGlobal::executeCommand("flushIdleQueue");

			//	Take a snapshot of the widget.
			QPixmap	swatch = QPixmap::grabWidget(swatchPort);

			//	Save the snapshot as a PNG file.
			QString	fileName = renderNode.asChar();
			fileName += ".png";
			swatch.save(fileName, "png", 100);
		}
	} else {
		MGlobal::displayError(
			MString("Could not find swatchDisplayPort '") + swatchPortName + "'"
		);
		return MS::kFailure;
	}

	//	Get rid of the window.
	MGlobal::executeCommand("deleteUI swatchGrabber");

	return MS::kSuccess;
}


// ==========================================================================
//
//			Plugin load/unload
//
// ==========================================================================

MStatus initializePlugin(MObject plugin)
{
	MStatus		st;
	MFnPlugin	pluginFn(plugin, "Autodesk, Inc.", "1.0", "Any", &st);

	if (!st) {
		MGlobal::displayError(
			MString("saveSwatchesCmd - could not initialize plugin: ")
			+ st.errorString()
		);
		return st;
	}

	//	Register the command.
	st = pluginFn.registerCommand(SaveSwatchesCmd::commandName, SaveSwatchesCmd::creator);

	if (!st) {
		MGlobal::displayError(
			MString("saveSwatchesCmd - could not register '")
			+ SaveSwatchesCmd::commandName + "' command: "
			+ st.errorString()
		);
		return st;
	}

	return st;
}


MStatus uninitializePlugin(MObject plugin)
{
	MStatus		st;
	MFnPlugin	pluginFn(plugin, "Autodesk, Inc.", "1.0", "Any", &st);

	if (!st) {
		MGlobal::displayError(
			MString("saveSwatchesCmd - could not uninitialize plugin: ")
			+ st.errorString()
		);
		return st;
	}

	//	Deregister the command.
	st = pluginFn.deregisterCommand(SaveSwatchesCmd::commandName);

	if (!st) {
		MGlobal::displayError(
			MString("saveSwatchesCmd - could not deregister '")
			+ SaveSwatchesCmd::commandName + "' command: "
			+ st.errorString()
		);
		return st;
	}

	return st;
}



