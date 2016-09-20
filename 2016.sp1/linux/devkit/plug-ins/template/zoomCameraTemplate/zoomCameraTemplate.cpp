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

////////////////////////////////////////////////////////////////////////
//
//  Description:
//      - doubles the focal length for the camera of the
//        current 3d view
//
////////////////////////////////////////////////////////////////////////

#include <maya/MTemplateCommand.h>

#include <maya/MString.h>
#include <maya/MArgList.h>

#include <maya/MFnCamera.h>
#include <maya/MGlobal.h>
#include <maya/M3dView.h>
#include <maya/MDagPath.h>

#include <maya/MIOStream.h>

class zoomCameraTemplate;
char cmdName[] = "zoomCameraTemplate";

class zoomCameraTemplate : public MTemplateCommand<zoomCameraTemplate,cmdName, MTemplateCommand_nullSyntax >
{
public:
	//
	zoomCameraTemplate()
	{}

	//
	virtual ~zoomCameraTemplate()
	{}

	//
	virtual MStatus doIt( const MArgList& )
	{
		MStatus stat = M3dView::active3dView().getCamera( camera );			
		if ( MS::kSuccess == stat ) {
			redoIt();
		}
		else {
 			cerr << "Error getting camera" << endl;
 		}
		return stat;
	}

	//
	virtual MStatus redoIt()
	{
		MFnCamera fnCamera( camera );
		double fl = fnCamera.focalLength();
		fnCamera.setFocalLength( fl * 2.0 );
		return MS::kSuccess;
	}

	//
	virtual MStatus undoIt()
	{
		MFnCamera fnCamera( camera );
		double fl = fnCamera.focalLength();
		fnCamera.setFocalLength( fl / 2.0 );
		return MS::kSuccess;
	}

private:
	MDagPath camera;
};

static zoomCameraTemplate _zoom;

//
//	Entry points
//

MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "2009", "Any");

	status = _zoom.registerCommand( obj );
	if (!status) 
	{
		status.perror("registerCommand");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = _zoom.deregisterCommand( obj );
	if (!status) 
	{
		status.perror("deregisterCommand");
		return status;
	}

	return status;
}
