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

#include <maya/MPxCommand.h>
#include <maya/MStatus.h>
#include <maya/MArgList.h>
#include <maya/MFnPlugin.h>
#include <maya/MObject.h>
#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MPoint.h>
#include "iffreader.h"
#ifndef OSMac_
#include <maya/MIOStream.h>
#endif
#define IFFCHECKERR(stat, call) \
if (!stat) { \
	MString string = reader.errorString(); \
    string += " in method "; \
	string += #call; \
    displayError (string); \
	return MS::kFailure; \
}

class iffInfo : public MPxCommand
{
public:
                iffInfo();
    virtual     ~iffInfo();

    MStatus     doIt ( const MArgList& args );
    MStatus     redoIt ();
    MStatus     undoIt ();
    bool        isUndoable() const;

    static      void* creator();

private:
	MString     result;
};

iffInfo::iffInfo()
{
}

iffInfo::~iffInfo() {}

void* iffInfo::creator()
{
    return new iffInfo;
}

bool iffInfo::isUndoable() const
{
    return true;
}

MStatus iffInfo::undoIt()
{
    return MS::kSuccess;
}

MString itoa (int n)
{
	char buffer [256];
	sprintf (buffer, "%d", n);
	return MString (buffer);
}

MStatus iffInfo::doIt( const MArgList& args )
{
    MString componentName;
	if (args.length () != 1 ) {
		displayError ("Syntax: iffInfo file");
		return MS::kFailure;
	}

	MString fileName;

	args.get (0, fileName);

	IFFimageReader reader;
	MStatus stat;

	stat = reader.open (fileName);
	IFFCHECKERR (stat, open);

	int bytesPerChannel = reader.getBytesPerChannel ();
	int w,h;

	stat = reader.getSize (w, h);
	IFFCHECKERR (stat, getSize);

	result = "\nResolution: ";
	result += itoa (w);
	result += "x";
	result += itoa (h);
	result += "\n";
	if (reader.isRGB () || reader.isGrayscale ()) {
		if (reader.isRGB ()) {
			result += "RGB";
			if (reader.hasAlpha ())
				result += "A";
		}
		else
			result += "Grayscale";
		result += " data with ";
		if (bytesPerChannel == 2)
			result += "16";
		else
			result += "8";
		result += " bits per channel\n";
	}
	if (reader.hasDepthMap ()) {
		result += "Image has a depth map\n";
	} else {
		result += "Image does not have a depth map\n";
	}

	stat = reader.close ();
	IFFCHECKERR (stat, close);

    return redoIt();
}

MStatus iffInfo::redoIt()
{
    clearResult();
	appendToResult (result);

    return MS::kSuccess;
}

MStatus initializePlugin( MObject obj )
{
	MStatus status;
    MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");
    status = plugin.registerCommand( "iffInfo", iffInfo::creator );

	if (!status)
		status.perror("registerCommand");

    return status;
}

MStatus uninitializePlugin( MObject obj )
{
	MStatus status;
    MFnPlugin plugin( obj );
    status = plugin.deregisterCommand( "iffInfo" );

	if (!status)
		status.perror("deregisterCommand");

    return status;
}
