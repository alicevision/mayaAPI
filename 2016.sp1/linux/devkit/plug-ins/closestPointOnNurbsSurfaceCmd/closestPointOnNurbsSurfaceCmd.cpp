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

// Description:
//		A command which exercises the various NURBS closestPoint methods
//	available in the API. The command expects all data it needs to work
//	with to be on the selection list.
//
// Usage:
//		Before calling this command, the selection list needs to have
//		the following nodes selected in order:
//
//		o  A NURBS surface, such as nurbsPlaneShape1. The surface may
//		   be transformed in the DAG, if desired.
//		o  The transform node of a locator. The user should place the
//		   locator at the point in 3-space for which they want to find
//		   the closest point on the NURBS surface.
//
//		When invoked with the above items in order on the selection list,
//		the command proceeds to calculate the closest point on the NURBS
//		surface, moving "locator2" to the computed closest point to allow
//		the user to visually see what closest point was calculated.
//
// Example:
//		1. Compile and load this plug-in into Maya.
//		2. Create a NURBS surface, such as a NURBS plane. Move some CVs
//		   to obtain a wavy surface.
//		3. Create two locators, locator1 and locator2, (both should be
//		   children of the world because the plug-in translates locator2
//		   to the calculated closest point value for display purposes).
//		4. Position the first locator somewhere in 3D space over the
//		   surface. Note that the second locator will be automatically
//		   moved to the closest point on the surface by the command.
//		5. Select the two objects:
//				MEL> select nurbsPlaneShape1 locator1 locator2;
//		6. Invoke this command:
//				MEL> closestPointOnNurbsSurface;
//		7. You should see locator2 move to the point on the NURBS
//		   surface which is closest to locator1.
//		8. Move locator1 to different positions and invoke this
//		   command again. You should see locator2 move to the correct
//		   closest location each time.
//		9. Try rotating, scaling and translating the NURBS surface's
//		   transform node and you should see the closest point being
//		   correctly computed.
//

#include <math.h>

// MAYA HEADERS
#include <maya/MIOStream.h>
#include <maya/MArgList.h>
#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MPoint.h>
#include <maya/MNurbsIntersector.h>
#include <maya/MDagPath.h>
#include <maya/MMatrix.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnTransform.h>
#include <maya/MVector.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnNurbsSurface.h>

#include "closestPointOnNurbsSurfaceCmd.h"

// CONSTRUCTOR:
closestPointOnNurbsSurfaceCmd::closestPointOnNurbsSurfaceCmd()
{
}

// DESTRUCTOR:
closestPointOnNurbsSurfaceCmd::~closestPointOnNurbsSurfaceCmd()
{
}

// FOR CREATING AN INSTANCE OF THIS COMMAND:
void* closestPointOnNurbsSurfaceCmd::creator()
{
   return new closestPointOnNurbsSurfaceCmd;
}

// MAKE THIS COMMAND NOT UNDOABLE:
bool closestPointOnNurbsSurfaceCmd::isUndoable() const
{
   return false;
}


MStatus closestPointOnNurbsSurfaceCmd::doIt(const MArgList& args)
// Description:
// See the command usage at the top of this file for details
// on how to use this command.
//
{
	bool debug = false;	
	bool treeBased = true;

	MStatus stat = MStatus::kSuccess;

	if(debug) cout << "closestPointOnNurbsSurfaceCmd::doIt\n";
	
	MSelectionList list;
	stat = MGlobal::getActiveSelectionList(list);
	if(!stat) {
		if(debug) cout << "getActiveSelectionList FAILED\n";
		return( stat );
	}

	MDagPath path;

	MObject nurbsObject;
	stat = list.getDependNode(0,nurbsObject);
	if(!stat)
		if(debug) cout << "getDependNode FAILED\n";

	MFnDagNode nodeFn(nurbsObject);

	// don't use the transform, use the shape
	if(nodeFn.childCount() > 0) {
		MObject child = nodeFn.child(0);
		nodeFn.setObject(child);
	}

	list.getDagPath(0,path);
	if(debug) cout << "Working with: " << path.partialPathName() << endl;

	MMatrix mat = path.inclusiveMatrixInverse();
	if(debug) cout << mat << endl;

	MObject loc1Object;
	stat = list.getDependNode(1, loc1Object); // use the transform, not the shape
	if(!stat) {
		if(debug) cout << "FAILED grabbing locator1\n";
		return( stat );
	}

	MFnTransform loc1Fn(loc1Object);
	MVector t = loc1Fn.getTranslation(MSpace::kObject);
	
	MPoint pt(t[0], t[1], t[2]);
	if(debug) cout << "test point: " << pt << endl;
	if(debug) cout << "transformed:" << pt * mat << endl;
	MPoint resultPoint;
	double u, v;

	if ( treeBased ) {
		// Use the tree-based NURBS closest point algorithm.
		// The idea is to call create() once, then reuse for later calls
		// to getClosestPoint(). In our example, we'll just do one
		// getClosestPoint() call.
		//
		if(debug) cout << "tree-based NURBS closestPoint (MNurbsIntersector)\n";
		MNurbsIntersector nurbIntersect;
		stat = nurbIntersect.create(nurbsObject, mat);
		if(!stat) {
			if(debug) cout << "MNurbsIntersector::create FAILED\n";
			return( stat );
		}

		MPointOnNurbs ptON;
		stat = nurbIntersect.getClosestPoint(pt, ptON);
		if(!stat) {
			if(debug) cout << "getClosestPoint FAILED!\n";
			return( stat );
		}
		resultPoint = ptON.getPoint();
		MPoint UV = ptON.getUV();
		u = UV.x;
		v = UV.y;
	} else {
		// Use the non-tree NURBS closest point algorithm from MFnNurbsSurface.
		//
		MFnNurbsSurface ns( nurbsObject );
		pt *= mat;	// Need to transform into object space ourselves
		resultPoint = ns.closestPoint( pt, false, &u, &v );
	}

	// As a check, grab the world space point that corresponds to the
	// UVs returned from getClosestPoint.
	//
	if(debug) cout << "result UV: " << u << ", " << v << endl;
	MString cmd = "pointOnSurface -u ";
	cmd += u;
	cmd += " -v ";
	cmd += v;
	cmd += " ";
	cmd += path.partialPathName();
	MDoubleArray arr;
	MGlobal::executeCommand(cmd, arr); 
	if(debug) cout << "check results:  result UV corresponds to world point: " << arr << endl;

	MPoint worldResultPoint = resultPoint * path.inclusiveMatrix();
	if(debug) cout << "local space result point: " << resultPoint << endl;
	if(debug) cout << "world space result point: " << worldResultPoint << endl;

	if ( fabs( arr[0] - worldResultPoint.x ) > 0.0001
			|| fabs( arr[1] - worldResultPoint.y ) > 0.0001
			|| fabs( arr[2] - worldResultPoint.z ) > 0.0001 ) {
		cout << "check results: pointOnSurface does not match world point: " << arr << endl;
		return( MS::kFailure );
	}

	// Move the second locator to the returned world-space point
	// This should always be on the nurbs surface.
	// Note: we are assuming with both locators that they are children of
	// the world.
	//
	MObject loc2Object;
	stat = list.getDependNode(2, loc2Object); // use the transform, not the shape
	if(!stat) {
		if(debug) cout << "FAILED grabbing locator2\n";
		return( stat );
	}

	MFnTransform loc2Fn(loc2Object);
	stat = loc2Fn.setTranslation(worldResultPoint, MSpace::kTransform);

	return stat;
}

// UNDO THE COMMAND
MStatus closestPointOnNurbsSurfaceCmd::undoIt()
{
	MStatus status;
	// undo not implemented
	return status;
}

//
// The following routines are used to register/unregister
// the command we are creating within Maya
//
MStatus initializePlugin( MObject obj )
{
    MStatus   status;
    MFnPlugin plugin( obj, PLUGIN_COMPANY, "8.5", "Any");

    status = plugin.registerCommand( "closestPointOnNurbsSurface",
			closestPointOnNurbsSurfaceCmd::creator );
    if (!status) {
        status.perror("registerCommand");
        return status;
    }

    return status;
}

MStatus uninitializePlugin( MObject obj)
{
    MStatus   status;
    MFnPlugin plugin( obj );

    status = plugin.deregisterCommand( "closestPointOnNurbsSurface" );
    if (!status) {
        status.perror("deregisterCommand");
        return status;
    }

    return status;
}
