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

// threadedBoundingBox.cpp
// 
// This plugin demonstrates the hazards of false sharing in
// multithreaded code. The plugin computes the min X element of the
// bounding box of a selected mesh. The element is computed two ways:
//
//   - Allocating an array of elements, one per thread, and building up
//   one value in each thread.
//
//   - Allocating an array of elements, more than one per thread, and
//   building up one value in each thread. Extra intermediate array
//   elements are allocated to ensure that each value used by a thread
//   is on a separate cache line.
//
//   In both cases, the values computed in each thread are finally
//   merged into a single min X value.
//
// The observed result is that the second computation is significantly
// faster than the first, at the cost of a small amount of extra
// memory usage. What is happening is that in the first case most (if
// not all) of the points being accumulated are on the same cache
// line, which causes the array of points to ping pong between
// processor caches as elements are computed by different threads
// running on different cores and written into the array. This
// degrades performance significantly (around 30x slower on a dual
// quad core Clovertown system.)
//
// Note that cache lines in current processors are usually 64 bytes,
// but may grow in future. To get the exact value we call an API
// method that returns the cache line size for the current processor
// on which Maya is being run.

#include <float.h>
#include <limits.h>

#ifdef _OPENMP
#include <omp.h>    // OpenMP header for omp_get_max_threads
#endif

#include <maya/MIOStream.h>
#include <maya/MSimple.h>
#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>
#include <maya/MFnMesh.h>
#include <maya/MDagPath.h>
#include <maya/MTimer.h>
#include <maya/MFloatPointArray.h>
#include <maya/MThreadUtils.h>

DeclareSimpleCommand( threadedBoundingBox, PLUGIN_COMPANY, "1.0");

float computeMinX(const MFloatPointArray& vertexArray, bool padding)
{
	// compute minimum X value using multiple evaluators, one per thread.

	const int floatSize = sizeof(float); // 4 bytes per float value

	const int cacheLineSize = MThreadUtils::getCacheLineSize();

	// spacing is the number of float values that must be adjacent in
	// memory to ensure one cache line is filled. We add an extra two
	// to ensure separation regardless of alignment.
	int spacing = 1;
	if(padding) {
		spacing = 2+(cacheLineSize/floatSize);
	}

	// NOTE - omp_get_num_threads() only returns >1 inside threaded code!
	// Use omp_get_max_threads() to get actual number of active threads.
	int numThreads = 1;
#ifdef _OPENMP
	numThreads = omp_get_max_threads();
#endif

	// allocate one or more boxes per thread, depending on the spacing computed above.
	float* pointArray = new float[numThreads*spacing];

	const int nb = vertexArray.length();
	const int nbStep = nb / numThreads;
	
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for(int i=0; i<numThreads; i++) {
		float& minX = pointArray[i*spacing];
		minX = FLT_MAX;
		const int n1 = i*nbStep;
		const int n2 = (i<(numThreads-1)) ? (i+1)*nbStep : nb;
		for  (int n=n1; n<n2; n++) {
			const MFloatPoint& p = vertexArray[n];
			if(p.x < minX) minX = p.x;
		}
	}

	float finalMinX = FLT_MAX;
	
	// accumulate separate boxes
	for(int i=0; i<numThreads; i++) {
		if(pointArray[i*spacing] < finalMinX) finalMinX = pointArray[i*spacing];
	}
	
	// clean up temporary boxes
	delete [] pointArray;

	return finalMinX;
}


MStatus threadedBoundingBox::doIt( const MArgList& args )
//
//	Description:
//		Implements the MEL threadedBoundingBox command.  This command computes a
//      bounding box for the currently selected mesh objects.

//      It is a demonstration of the problems of false sharing.
//		
//	Arguments:
//		args - the argument list that was passes to the command from MEL.  This
//			   command takes no arguments.
//
//	Return Value:
//		MS::kSuccess - command succeeded
//		MS::kFailure - command failed (returning this value will cause the 
//                     MEL script that is being run to terminate unless the
//                     error is caught using a "catch" statement.
//
{
	MStatus stat = MS::kSuccess;

	MSelectionList curSel;
	MGlobal::getActiveSelectionList( curSel );

	// iterate through the selection list, and find bounding boxes for any selected
	// polygons.
	//
	int numSelected = curSel.length();
	for( int s = 0; s < numSelected; s++ )
	{

		// get the selected item, figure out if it's a polymesh or not
		//
		MDagPath dagPath;
		curSel.getDagPath( s, dagPath );
		if( dagPath.extendToShape() != MS::kSuccess )
		{
			// selection does not correspond to a DAG shape
			//
			cout<<"	Error - object is not a polymesh"<<endl;
			stat = MS::kFailure;
			return stat;
		}

		MObject node = dagPath.node();
		MFnDependencyNode fnNode( node );
		
		MFnMesh fnMesh( node, &stat );
		if( stat != MS::kSuccess )
		{
			cout<<"	Error - unable to create MFnMesh object"<<endl;
			return stat;
		}

		// Retrieve the list of vertices on the polymesh.
		MFloatPointArray vertexArray;
		
		// get the vertices
		//
		stat = fnMesh.getPoints(vertexArray );
		if( stat != MS::kSuccess ) {
			cout<<"	Error - unable to retrieve vertices"<<endl;
			return stat;
		}

		float minX1 = 0.0f;
		float minX2 = 0.0f;

		cout<<"    Poly has "<< vertexArray.length()<<" vertices"<<endl;
		bool padding = false;
		int numIterations = 100;

		MTimer timer; 
		timer.beginTimer();
		for(int i=0; i<numIterations; i++) {
			minX1 = computeMinX(vertexArray, padding);
		}
		timer.endTimer(); 
		printf("Runtime without padding %f\n", timer.elapsedTime());

		padding = true;
		timer.beginTimer();
		for(int i=0; i<numIterations; i++) {
			minX2 = computeMinX(vertexArray, padding);
		}
		timer.endTimer(); 
		printf("Runtime with padding %f\n", timer.elapsedTime());
		

		if(fabs(minX1-minX2)<1e-10) {
			cout << "Boxes match" << endl;
		} else {
			cout << "Boxes do not match" << endl;
			stat = MS::kFailure;
			return stat;
		}
	}
	
	setResult( "threadedBoundingBox completed." );
	return stat;
}




