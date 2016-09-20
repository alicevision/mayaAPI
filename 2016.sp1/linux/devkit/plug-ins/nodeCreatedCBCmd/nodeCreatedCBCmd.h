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
#ifndef _nodeCreatedCBCmd_h
#define _nodeCreatedCBCmd_h

#include <maya/MPxCommand.h>
#include <maya/MMessage.h>

class MObject;

class nodeCreatedCB : public MPxCommand {
public:
					nodeCreatedCB() {};
	virtual MStatus	doIt ( const MArgList& );					

	static void		sCallbackFunc( MObject& node, void * clientData );
	
	static void*	creator();
	static MSyntax	newSyntax();

	static MCallbackId	sId;
	static MStringArray	sMelProcs;
	static MIntArray	sFullDagPath;

private:
	MStatus			registerMelProc( MString melProc, bool fullDagPath );
	MStatus			unregisterMelProc( MString melProc );
	MStatus			changeFilter( MString filter );
};

#endif
