//-
// ==========================================================================
// Copyright 2010 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

////////////////////////////////////////////////////////////////////////
//
// multiPlugInfo.cpp
//
// This plugin prints out the child plug information for a multiPlug.
// If the -index flag is used, the logical index values used by the plug
// will be returned.  Otherwise, the plug values will be returned.
//
////////////////////////////////////////////////////////////////////////

#include <maya/MFnPlugin.h>
#include <maya/MPxCommand.h>
#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MPlug.h>
#include <maya/MSelectionList.h>
#include <maya/MSyntax.h>

#define kIndexFlag		"-i"
#define kIndexFlagLong	"-index"

///////////////////////////////////////////////////
//
// Command class declaration
//
///////////////////////////////////////////////////
class multiPlugInfo : public MPxCommand
{
public:
					multiPlugInfo();
	virtual			~multiPlugInfo(); 

	MStatus			doIt( const MArgList& args );

	virtual bool	hasSyntax();
	static MSyntax	cmdSyntax();
	static void*	creator();

private:
	MStatus			parseArgs( const MArgList& args );
	MPlug			fPlug;
	bool			isIndex;
};



///////////////////////////////////////////////////
//
// Command class implementation
//
///////////////////////////////////////////////////

multiPlugInfo::multiPlugInfo()
	: isIndex (false)
{}

multiPlugInfo::~multiPlugInfo() {}

void* multiPlugInfo::creator()
{
	return new multiPlugInfo;
}

MSyntax multiPlugInfo::cmdSyntax()
{
	MSyntax syntax;
	syntax.addFlag(kIndexFlag, kIndexFlagLong, MSyntax::kNoArg);
	syntax.setObjectType(MSyntax::kSelectionList, 1, 1);
	syntax.enableQuery( false );
	syntax.enableEdit( false );
	return syntax;
}

bool multiPlugInfo::hasSyntax()
{
	return true;
}

MStatus multiPlugInfo::parseArgs( const MArgList& args )
{
	MStatus status = MS::kSuccess;
	MArgDatabase	argData(syntax(), args, &status);
	if (status != MS::kSuccess)
		return status;

	if (argData.isFlagSet(kIndexFlag))
		isIndex = true;

	// Get the plug specified on the command line.
	MSelectionList slist;
	argData.getObjects(slist);
	if ((slist.length() == 0) ||
		(slist.getPlug(0, fPlug) != MS::kSuccess))
	{
		displayError("Must specify an array plug in the form <nodeName>.<multiPlugName>.", false);
		return MS::kFailure;
	}

	return MS::kSuccess;
}

MStatus multiPlugInfo::doIt( const MArgList& args )
//
// Description
//     This method performs the action of the command.
//
//     This method gets the data stored in the multi attribute
//		and prints it out.
//
{
	if (parseArgs(args) != MS::kSuccess)
		return MS::kFailure;

	// Construct a data handle containing the data stored in the plug.
	MDataHandle dh;
	MStatus stat = fPlug.getValue(dh);
	if (stat != MS::kSuccess)
	{
		displayError("Could not get the plug value.", false);
		return MS::kFailure;
	}
	MArrayDataHandle adh(dh, &stat);
	if (stat != MS::kSuccess)
	{
		displayError("Could not create the array data handle.", false);
		fPlug.destructHandle(dh);
		return MS::kFailure;
	}

	// Iterate over the values in the multiPlug.  If the index flag has been used, just return
	// the logical indices of the child plugs.  Otherwise, return the plug values.
	unsigned int i;
	for (i=0; i<adh.elementCount(); i++, adh.next())
	{
		unsigned int indx = adh.elementIndex(&stat);
		if (stat != MS::kSuccess)
			continue;

		if (isIndex)
			appendToResult((int)indx);
		else
		{
			MDataHandle h = adh.outputValue();
			if (h.isNumeric())
			{
				switch(h.numericType())
				{
					case MFnNumericData::kBoolean:
						appendToResult(h.asBool());
						break;
					case MFnNumericData::kShort:
						appendToResult((int)(h.asShort()));
						break;
					case MFnNumericData::kInt:
						appendToResult(h.asInt());
						break;
					case MFnNumericData::kFloat:
						appendToResult((double)(h.asFloat()));
						break;
					case MFnNumericData::kDouble:
						appendToResult(h.asDouble());
						break;
					default:
						displayError("This sample command only supports boolean, integer, and floating point values.", false);
						stat = MS::kFailure;
						break;
				}
			}
 		}
	}

	fPlug.destructHandle(dh);
	return stat;

}

/////////////////////////////////////////////////////////////////
//
// The following routines are used to register/unregister
// the command we are creating within Maya
//
/////////////////////////////////////////////////////////////////
MStatus initializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "1.0", "Any");

	status = plugin.registerCommand( "multiPlugInfo",
									  multiPlugInfo::creator,
									  multiPlugInfo::cmdSyntax);
	if (!status) {
		status.perror("registerCommand");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus	  status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterCommand( "multiPlugInfo" );
	if (!status) {
		status.perror("deregisterCommand");
		return status;
	}

	return status;
}
