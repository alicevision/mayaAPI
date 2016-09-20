//-
// ==========================================================================
// Copyright 2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

//	Must ensure that at least one Qt header is included before anything
//	else.
#include <helixQtCmd.h>

#include <math.h>
#include <maya/MDoubleArray.h> 
#include <maya/MFnDependencyNode.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include <maya/MObject.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>

// ==========================================================================
//
//			HelixButton class
//
// ==========================================================================

HelixButton::HelixButton(const QString& text, QWidget* parent)
:	QPushButton(text, parent)
{}

HelixButton::~HelixButton()
{}


//	This is a slot which is called whenever the button is pushed.
//	It creates a helical NURBS curve within Maya.
void HelixButton::createHelix(bool /* checked */)
{
	MStatus st;

    const unsigned deg    = 3;             // Curve Degree
    const unsigned ncvs   = 20;            // Number of CVs
    const unsigned spans  = ncvs - deg;    // Number of spans
    const unsigned nknots = spans+2*deg-1; // Number of knots
    double         radius = 4.0;           // Helix radius
    double         pitch  = 0.5;           // Helix pitch
    unsigned       i;

    MPointArray controlVertices;
    MDoubleArray knotSequences;

    // Set up cvs and knots for the helix
    for (i = 0; i < ncvs; i++) {
        controlVertices.append(
			MPoint(
				radius * cos((double) i),
				pitch * (double) i,
				radius * sin((double) i)
			)
		);
	}
	
    for (i = 0; i < nknots; i++)
        knotSequences.append((double) i);

    // Now create the curve
    MFnNurbsCurve curveFn;

    MObject curve = curveFn.create(
		controlVertices,
		knotSequences,
		deg,
		MFnNurbsCurve::kOpen,
		false,
		false,
		MObject::kNullObj,
		&st
	);

	if (!st) {
        MGlobal::displayError(
			HelixQtCmd::commandName + " - could not create helix: "
			+ st.errorString()
		);
	}
}


// ==========================================================================
//
//			HelixQtCmd class
//
// ==========================================================================

//	We store a pointer to the button window in a static QPointer so that we
//	can destroy it if the plugin is unloaded. The QPointer will
//	automatically set itself to zero if the button window is destroyed
//	for any reason.
QPointer<HelixButton>	HelixQtCmd::button;

const MString			HelixQtCmd::commandName("helixQt");


//	Destroy the button window, if it still exists.
void HelixQtCmd::cleanup()
{
	if (!button.isNull()) delete button;
}


MStatus HelixQtCmd::doIt(const MArgList& /* args */)
{
	//	Create a window containing a HelixButton, if one does not already
	//	exist. Otherwise just make sure that the existing window is visible.
	if (button.isNull()) {
		button = new HelixButton("Create Helix");
		button->connect(button, SIGNAL(clicked(bool)), button, SLOT(createHelix(bool)));
		button->show();
	}
	else {
		button->showNormal();
		button->raise();
	}


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
			MString("helixQtCmd - could not initialize plugin: ")
			+ st.errorString()
		);
		return st;
	}

	//	Register the command.
	st = pluginFn.registerCommand(HelixQtCmd::commandName, HelixQtCmd::creator);

	if (!st) {
		MGlobal::displayError(
			MString("helixQtCmd - could not register '")
			+ HelixQtCmd::commandName + "' command: "
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
			MString("helixQtCmd - could not uninitialize plugin: ")
			+ st.errorString()
		);
		return st;
	}

	//	Make sure that there is no UI left hanging around.
	HelixQtCmd::cleanup();

	//	Deregister the command.
	st = pluginFn.deregisterCommand(HelixQtCmd::commandName);

	if (!st) {
		MGlobal::displayError(
			MString("helixQtCmd - could not deregister '")
			+ HelixQtCmd::commandName + "' command: "
			+ st.errorString()
		);
		return st;
	}

	return st;
}



