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

//	Must ensure that at least one Qt header is included before anything
//	else.
#include <QtGui/QComboBox>
#include <QtGui/QDoubleValidator>
#include <QtCore/QFile>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtCore/QLocale>
#include <QtGui/QPushButton>
#include <QtUiTools/QtUiTools>
#include <QtGui/QVBoxLayout>

#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include <maya/MObject.h>
#include <maya/MStringArray.h>

#include <qtForms.h>

// ==========================================================================
//
//			CubeCreator class
//
//	This is an example of a dialog which was created using Qt Designer and
//	then compiled into the application at build time.
//
// ==========================================================================

CubeCreator::CubeCreator(QWidget* parent)
:	QDialog(parent)
,	fCurValue(0.0)
{
	//	Initialize the form and let it know that we are its parent.
	setupUi(this);

	//	Destroy the dialog when it is closed.
	setAttribute(Qt::WA_DeleteOnClose, true);

	//	We only want the size field to accept floating point numbers.
	sizeField->setValidator(new QDoubleValidator(-10.0, 10.0, 2, sizeField));

	//	When the form's slider changes we'll need to update the field, and
	//	vice versa.
	connect(
		sizeSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int))
	);
	connect(
		sizeField, SIGNAL(textEdited(const QString&)),
		this, SLOT(fieldChanged(const QString&))
	);

	show();
}


//	The form will automatically connect its Ok button to our accept() slot.
//	We use the slot to create the cube.
void CubeCreator::accept()
{
	MStringArray	names;

	//	Execute a 'polyCube' command using the specified size for all three
	//	dimensions.
	MGlobal::executeCommand(
		MString("polyCube -w ") + fCurValue + " -h " + fCurValue
			+ " -d " + fCurValue,
		names
	);

	//	Let everyone know that we've created a new object.
	emit objectCreated(names[0].asChar());
}


void CubeCreator::fieldChanged(const QString& newValue)
{
	QLocale	defaultLocale;

	//	Convert the value from the size field, which is a double from -10.0
	//	to +10.0, into a value for the slider, which is an integer from
	//	-1000 to +1000.
	fCurValue = defaultLocale.toDouble(newValue);
	sizeSlider->setValue((int)(fCurValue * 100.0));
}


void CubeCreator::sliderChanged(int newValue)
{
	QLocale	defaultLocale;

	//	Convert the value from the slider, which is an integer from -1000
	//	to +1000, into a value for the size field, which is a double from
	//	-10.0 to +10.0.
	fCurValue = ((double)newValue) / 100.0;
	sizeField->setText(defaultLocale.toString(fCurValue));
}


// ==========================================================================
//
//			SphereCreator class
//
//	This is an example of a dialog which was created using Qt Designer and
//	is loaded into the application from a resource at run time.
//
// ==========================================================================

SphereCreator::SphereCreator(QWidget* parent)
:	QWidget(parent)
,	fCurValue(0.0)
{
	//	Load the form from its resource.
	QUiLoader	loader;
	QFile		file(":/sphereForm.ui");

	file.open(QFile::ReadOnly);
	fForm = loader.load(&file, this);
	file.close();

	if (fForm) {
		//	Destroy the dialog when it is closed.
		fForm->setAttribute(Qt::WA_DeleteOnClose, true);

		//	Locate the various widgets inside the form.
		fButtonBox = fForm->findChild<QDialogButtonBox*>("buttonBox");
		fField = fForm->findChild<QLineEdit*>("sizeField");
		fSlider = fForm->findChild<QSlider*>("sizeSlider");
	
		//	Connect to the buttonBox's 'accepted' signal, which indicates
		//	that the Ok button has been clicked.
		connect(fButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
	
		//	We want the size field to only accept floating point numbers.
		fField->setValidator(new QDoubleValidator(-10.0, 10.0, 2, fField));
	
		//	When the form's slider changes we'll need to update the field, and
		//	vice versa.
		connect(
			fSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int))
		);
		connect(
			fField, SIGNAL(textEdited(const QString&)),
			this, SLOT(fieldChanged(const QString&))
		);

		//	When the form is destroyed, destroy us as well.
		connect(
			fForm, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater())
		);

		fForm->show();
	}
}


void SphereCreator::accept()
{
	MStringArray	names;
	MGlobal::executeCommand(MString("polySphere -r ") + fCurValue, names);

	QString		objectName(names[0].asChar());
	emit objectCreated(objectName);
}


void SphereCreator::fieldChanged(const QString& newValue)
{
	QLocale	defaultLocale;

	//	Convert the value from the size field, which is a double from -10.0
	//	to +10.0, into a value for the slider, which is an integer from
	//	-1000 to +1000.
	fCurValue = defaultLocale.toDouble(newValue);
	fSlider->setValue((int)(fCurValue * 100.0));
}


void SphereCreator::sliderChanged(int newValue)
{
	QLocale	defaultLocale;

	//	Convert the value from the slider, which is an integer from -1000
	//	to +1000, into a value for the size field, which is a double from
	//	-10.0 to +10.0.
	fCurValue = ((double)newValue) / 100.0;
	fField->setText(defaultLocale.toString(fCurValue));
}


// ==========================================================================
//
//			ObjectTypeDialog class
//
//	This is an example of a dialog which is completely created at run time
//	using Qt calls.
//
// ==========================================================================

ObjectTypeDialog::ObjectTypeDialog(QWidget* parent)
:	QDialog(parent)
,	fSelectObjList(0)
{
	//	Create the form's various components.
	QLabel*	selectObjLabel = new QLabel("Object Type");
	selectObjLabel->setAlignment(Qt::AlignRight);

	fSelectObjList = new QComboBox();
	fSelectObjList->addItem("None");
	fSelectObjList->addItem("Cube");
	fSelectObjList->addItem("Sphere");
	selectObjLabel->setBuddy(fSelectObjList);

	QLabel*	mostRecentLabel = new QLabel("Most recently created");
	mostRecentLabel->setAlignment(Qt::AlignRight);

	fMostRecentField = new QLineEdit();
	fMostRecentField->setReadOnly(true);
	mostRecentLabel->setBuddy(fMostRecentField);

	QPushButton*	closeButton = new QPushButton("Close");

	//	Put the labels and controls into a 2x2 grid layout.
	QGridLayout*	gridLayout = new QGridLayout();
	gridLayout->addWidget(selectObjLabel, 0, 0, Qt::AlignRight);
	gridLayout->addWidget(fSelectObjList, 0, 1, Qt::AlignLeft);
	gridLayout->addWidget(mostRecentLabel, 1, 0, Qt::AlignRight);
	gridLayout->addWidget(fMostRecentField, 1, 1, Qt::AlignLeft);

	//	Use a vertical layout to place the grid above the close button.
	QVBoxLayout*	mainLayout = new QVBoxLayout();
	mainLayout->addLayout(gridLayout);
	mainLayout->addWidget(closeButton, 0, Qt::AlignHCenter);

	//	Make the vertical layout the top layout of this window.
	setLayout(mainLayout);

	//	Whenever the user selects a new object type from the combo box, we
	//	need to display the appropriate dialog.
	connect(
		fSelectObjList, SIGNAL(currentIndexChanged(const QString&)),
		this, SLOT(displayObjectDialog(const QString&))
	);

	//	Delete this dialog when the Close button is clicked.
	setAttribute(Qt::WA_DeleteOnClose, true);
	connect(
		closeButton, SIGNAL(clicked()), this, SLOT(close())
	);

	//	Display this dialog.
	show();
}


ObjectTypeDialog::~ObjectTypeDialog()
{
	//	If there is an object dialog displayed, get rid of it.
	if (!fCurrentDialog.isNull()) delete fCurrentDialog;
}


void ObjectTypeDialog::displayObjectDialog(const QString& item)
{
	//	If there is already an object dialog displayed, get rid of it.
	if (!fCurrentDialog.isNull()) {
		delete fCurrentDialog;
	}

	if (item == "Cube") {
		fCurrentDialog = new CubeCreator();
	}
	else if (item == "Sphere") {
		fCurrentDialog = new SphereCreator();
	}

	if (!fCurrentDialog.isNull()) {
		//	Whenever the object dialog creates a new object, we display its
		//	name in our 'Most Recent' field.
		connect(
			fCurrentDialog, SIGNAL(objectCreated(const QString&)),
			fMostRecentField, SLOT(setText(const QString&))
		);
	
		//	Reset the object selector if the object dialog is destroyed.
		connect(
			fCurrentDialog, SIGNAL(destroyed(QObject*)),
			this, SLOT(resetSelector())
		);
	}
}


void ObjectTypeDialog::resetSelector()
{
	//	Reset the object selector to 'None'.
	fSelectObjList->setCurrentIndex(0);
}


// ==========================================================================
//
//			qtFormsCmd class
//
// ==========================================================================

//	We store a pointer to the button window in a static QPointer so that we
//	can destroy it if the plugin is unloaded. The QPointer will
//	automatically set itself to zero if the button window is destroyed
//	for any reason.
QPointer<ObjectTypeDialog>	qtFormsCmd::objectCreator;

const MString			qtFormsCmd::commandName("qtForms");


//	Destroy the Object Creator window, if it still exists.
void qtFormsCmd::cleanup()
{
	if (!objectCreator.isNull()) delete objectCreator;
}


MStatus qtFormsCmd::doIt(const MArgList& /* args */)
{
	//	Create the Object Creator window, if it does not already exist
	//	Otherwise just make sure that the existing window is visible.
	if (objectCreator.isNull()) {
		objectCreator = new ObjectTypeDialog();
	}
	else {
		objectCreator->showNormal();
		objectCreator->raise();
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
			MString("qtForms - could not initialize plugin: ")
			+ st.errorString()
		);
		return st;
	}

	//	Register the command.
	st = pluginFn.registerCommand(qtFormsCmd::commandName, qtFormsCmd::creator);

	if (!st) {
		MGlobal::displayError(
			MString("qtForms - could not register '")
			+ qtFormsCmd::commandName + "' command: "
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
			MString("qtForms - could not uninitialize plugin: ")
			+ st.errorString()
		);
		return st;
	}

	//	Make sure that there is no UI left hanging around.
	qtFormsCmd::cleanup();

	//	Deregister the command.
	st = pluginFn.deregisterCommand(qtFormsCmd::commandName);

	if (!st) {
		MGlobal::displayError(
			MString("qtForms - could not deregister '")
			+ qtFormsCmd::commandName + "' command: "
			+ st.errorString()
		);
		return st;
	}

	return st;
}



