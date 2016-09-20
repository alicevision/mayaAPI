#ifndef qtForms_h
#define qtForms_h
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

#include <QtGui/QDialog>
#include <QtCore/QPointer>
#include <maya/MPxCommand.h>

#include <ui_cubeForm.h>

class QComboBox;


//	This class uses a form which was built in Qt Designer and compiled at
//	build time.
class CubeCreator : public QDialog, private Ui::CubeForm
{
	Q_OBJECT

public:
			CubeCreator(QWidget* parent = 0);

signals:
	void	dialogDismissed();
	void	objectCreated(const QString& objectName);

private slots:
	void	accept();
	void	fieldChanged(const QString& newValue);
	void	sliderChanged(int newValue);

private:
	double				fCurValue;
};


//	This class uses a form which was built in Qt Designer and is loaded
//	from a resource at run time.
class SphereCreator : public QWidget
{
	Q_OBJECT

public:
			SphereCreator(QWidget* parent = 0);

signals:
	void	dialogDismissed();
	void	objectCreated(const QString& objectName);

private slots:
	void	accept();
	void	fieldChanged(const QString& newValue);
	void	sliderChanged(int newValue);

private:
	QDialogButtonBox*	fButtonBox;
	double				fCurValue;
	QLineEdit*			fField;
	QPointer<QWidget>	fForm;
	QSlider*			fSlider;
};


//	This class uses embedded C++ code to build the form at run time.
class ObjectTypeDialog : public QDialog
{
	Q_OBJECT

public:
			ObjectTypeDialog(QWidget* parent = 0);
	virtual	~ObjectTypeDialog();

public slots:
	void	displayObjectDialog(const QString& item);
	void	resetSelector();

private:
	QPointer<QObject>	fCurrentDialog;
	QLineEdit*			fMostRecentField;
	QComboBox*			fSelectObjList;
	float				fOffset;
};


class qtFormsCmd: public MPxCommand
{
public:
	static void		cleanup();
	static void*	creator()		{ return new qtFormsCmd(); }

	MStatus			doIt(const MArgList& args);

	static QPointer<ObjectTypeDialog>	objectCreator;
	static const MString			commandName;
};

#endif
