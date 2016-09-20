#ifndef helixQtCmd_h
#define helixQtCmd_h
// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.

//	Must ensure that at least one Qt header is included before anything
//	else.
#include <QtCore/QPointer>
#include <QtGui/QPushButton>
#include <maya/MPxCommand.h>
#include <vector>

class HelixButton : public QPushButton
{
	Q_OBJECT

public:
			HelixButton(const QString& text, QWidget* parent = 0);
	virtual	~HelixButton();

public slots:
	void	createHelix(bool checked);

private:
	float	fOffset;
};


class HelixQtCmd : public MPxCommand
{
public:
	static void		cleanup();
	static void*	creator()		{ return new HelixQtCmd(); }

	MStatus			doIt(const MArgList& args);

	static QPointer<HelixButton>	button;
	static const MString			commandName;
};

#endif
