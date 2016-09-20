#ifndef _GLSLShaderCmd_h_
#define _GLSLShaderCmd_h_
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

#include <maya/MPxCommand.h>
#include <maya/MPxHardwareShader.h>

class GLSLShaderCmd : MPxCommand
{
public:
	GLSLShaderCmd();
	virtual				~GLSLShaderCmd();

	MStatus				doIt( const MArgList& );
	bool				isUndoable() { return false; }

	static MSyntax		newSyntax();
	static void*		creator();
};

#endif /* _GLSLShaderCmd_h_ */