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

#include <maya/MStatus.h>
#include <maya/MObject.h>
#include <maya/MString.h> 
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MItSurfaceCV.h>
#include <maya/MItMeshVertex.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MMatrix.h>
#include <maya/MDagPath.h>
#include <maya/MFileIO.h>
#include <maya/MLibrary.h>
#include <maya/MIOStream.h>
#include <math.h> 

MStatus twistSurf();

int main(int /*argc*/, char **argv)
{
	MStatus	stat;

	stat = MLibrary::initialize (argv[0]);
	if (!stat) {
		stat.perror("MLibrary::initialize");
		return 1;
	}

	MString	fileName("surf1.ma");

	cout << ">>>> Attempting to load surf1.ma <<<<\n";
	stat = MFileIO::open(fileName, "mayaAscii", true);
	if ( stat )
		cout << ">>>> Load Successfull <<<<\n";
	else {
		cout << ">>>> Load Failed <<<<\n";
		stat.perror("MFileIO::open");
		return 1;
	}

	stat = twistSurf();
	if (!stat)
		return 1;

	cout << ">>>> Attempting save as surf2.ma <<<<\n";
	stat = MFileIO::exportAll("surf2.ma", "mayaAscii");
	if (stat)
		cout << ">>>> Save Successfull <<<<\n";
	else {
		cout << ">>>> Save Failed <<<<\n";
		stat.perror("MFileIO::exportAll");
	}

	MLibrary::cleanup();
	return 0;
}

#define NUM_SPANS        30
#define WIDTH            10.0
#define VERTICAL_SCALING 4.0

static MStatus twistNurbsSurface(MDagPath& objectPath, MObject& component)
{
	MStatus status;

	MPoint  center;
	MVector toCenter( -center.x, 0.0, -center.y );
	double  rotFactor = 0.5;

	// We have a nurbs surface or component
	//
	MItSurfaceCV cvIter( objectPath, component, true, &status );

	if ( status ) {
		// We successfully created a nurbs surface iterator
		//
		for ( ; !cvIter.isDone(); cvIter.nextRow() ) {
			for ( ; !cvIter.isRowDone(); cvIter.next() ) {
				// Get the location of the CV
				//
				MPoint pnt = cvIter.position( MSpace::kWorld );
				pnt = pnt + toCenter;
				// Calculate rotation in radians about the y-axis
				//
				double rotation = pnt.y * rotFactor;
				MMatrix rotMatrix;
				// Set matrix to a rotation about the y axis
				//
				rotMatrix(0,0) = cos( rotation );
				rotMatrix(0,2) = sin( rotation );
				rotMatrix(2,0) = -sin( rotation );
				rotMatrix(2,2) = cos( rotation );
				pnt = ( pnt * rotMatrix ) - toCenter;

				status = cvIter.setPosition( pnt, MSpace::kWorld );
				if ( !status ) {
					status.perror("MItSurfaceCV::setPosition");
					break;
				}
			}
		}
		// Tell maya to redraw the surface with all of our changes
		//
		cvIter.updateSurface();
	} else
		status.perror("MItSurfaceCV::MItSurfaceCV");
	return status;
}

static MStatus twistPolygon(MDagPath& objectPath, MObject& component)
{
	MStatus status;

	MPoint  center;
	MVector toCenter( -center.x, 0.0, -center.y );
	double  rotFactor = 0.5;

	MItMeshVertex vertIter( objectPath, component, &status );

	if ( status ) {
		// We successfully created a polygon vertex iterator
		//

		for ( ; !vertIter.isDone(); vertIter.next() ) {
			// Get the location of the vertex
			//
			MPoint pnt = vertIter.position( MSpace::kWorld );
			pnt = pnt + toCenter;
			// Calculate rotation in radians about the y-axis
			//
			double rotation = pnt.y * rotFactor;
			MMatrix rotMatrix;
			// Set matrix to a rotation about the y axis
			//
			rotMatrix(0,0) = cos( rotation );
			rotMatrix(0,2) = sin( rotation );
			rotMatrix(2,0) = -sin( rotation );
			rotMatrix(2,2) = cos( rotation );
			pnt = ( pnt * rotMatrix ) - toCenter;

			status = vertIter.setPosition( pnt, MSpace::kWorld );
			if ( !status ) {
				status.perror("MItMeshVertex::MItMeshVertex");
				break;
			}
		}
		// Tell maya to redraw the surface with all of our changes
		//
		vertIter.updateSurface();
	} else
		status.perror("MItSurfaceCV::MItSurfaceCV");
	return status;
}

MStatus twistSurf()
{
	MStatus status;

	cout << ">>>> Start twist routine <<<<" << endl;

	MString	surface1("surface1");
	MGlobal::selectByName(surface1, MGlobal::kReplaceList);

	// Create an iterator for the active selection list
	//
	MSelectionList slist;
	MGlobal::getActiveSelectionList( slist );
	MItSelectionList iter( slist );

	if (iter.isDone()) {
		cerr << "Nothing selected\n";
		return MS::kFailure;
	}

	MDagPath objectPath;
	MObject component;

	for ( ; !iter.isDone(); iter.next() ) {
		status = iter.getDagPath( objectPath, component );

		if (objectPath.hasFn(MFn::kNurbsSurface))
			status = twistNurbsSurface(objectPath, component);
		else if (objectPath.hasFn(MFn::kMesh))
			status = twistPolygon(objectPath, component);
		else {
			cerr << "Selected object is not a NURBS surface or a polygon\n";
			return MS::kFailure;
		}
	}

	if ( status ) {
		cout << ">>>> Twist Successfull <<<<\n";
	} else {
		cout << ">>>> Twist Failed <<<<\n";
	}

	return status;
}
