#ifndef _gpuCacheSpatialSubdivision
#define _gpuCacheSpatialSubdivision
// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.

//
// Description:  
//
//		The gpuCacheSpatialSubdivision class represents a spatial subdivision 
//		structure that subdivides the bounding box for a gpuCache into cells.  
//		Each cell stores a list of the triangles of the gpuCache that at least 
//		partially intersects the cell.  An iterator is provided that will walk 
//		through the cells intersected by a ray.  This structure can be used to 
//		perform fast intersection tests between a ray and the gpuCache.
//
//		The gpuCacheIsectAccelParams class encapsulates the parameters of the
//		intersection acceleration structure, including how the cells are
//		organized, and how many cells are used to fill the mesh bounding
//		box.  Currently, the only option is a uniform grid, with a variable
//		number of grid cells along the X, Y, and Z axes.  In the future, other
//		schemes such as octrees could potentially be incorporated.
//

#include "gpuCacheSample.h"

#include "gpuCacheSpatialGrid.h" 
#include "gpuCacheSpatialGridWalker.h" 
#include "gpuCacheIsectUtil.h"

namespace GPUCache {

typedef IndexBuffer::index_t index_t;
class ShapeNode;
class gpuCacheVoxelGrid;

//=============================================================================
//
//	Class: gpuCacheIsectAccelParams
//	
//	Purpose: Encapsulates parameters describing the configuration of a
//			 spatial acceleration structure.  Passed as a parameter to 
//			 gpuCacheSpatialSubdivision to control construction of the 
//			 acceleration structure.
//
//=============================================================================

class gpuCacheIsectAccelParams
{
public:

	//	default constructor for arrays
	//
	gpuCacheIsectAccelParams();

	//	comparing acceleration structure configurations
	//	(necessary to know if an acceleration structure needs
	//	to be rebuilt)
	//
	int operator==( const gpuCacheIsectAccelParams& rhs );
	int operator!=( const gpuCacheIsectAccelParams& rhs );

	//	Use the *Params methods to create acceleration param structures.
	//	There are currently two algorithms available:
	//
	//	1) uniformGrid: triangles are organized into a uniform grid.
	//					with the user specifying the number of grid
	//					divisions in x, y, and z
	//
	//	2) autoUniformGrid: also a uniform grid strategy, but
	//						the number of divisions is chosen automatically
	//						based on the average triangle area of the
	//						mesh, and using some heuristics.
	//

	//	create a uniform grid configuration object
	static gpuCacheIsectAccelParams uniformGridParams( int divX = 10,
		int divY = 10,
		int divZ = 10 );

	//	create an auto uniform grid configuration object
	static gpuCacheIsectAccelParams autoUniformGridParams();

	friend class gpuCacheSpatialSubdivision;

	// types of acceleration structures
	enum
	{
		kUniformGrid,
		kAutoUniformGrid,
		kInvalid

	};

private:

	//	constructor is intentionally private, to force clients to go through
	//	the *Param methods above.  Seems like a better way to leave room
	//	for future extensions.
	//
	gpuCacheIsectAccelParams( int	 alg, 
		int 	 divX,
		int 	 divY,
		int 	 divZ );

	int		fAlgorithm;	// type of acceleration structure
	int 		fDivX;		// number of grid cells along X axis
	int 		fDivY;		// number of grid cells along Y axis
	int 		fDivZ;		// number of grid cells along Z axis
};

//=============================================================================
//
//	Class: gpuCacheSpatialSubdivision
//	
//	Purpose: Organizes the triangles of a poly mesh into the cells of a 3d
//			 spatial subdivision of the mesh bounding box.  Provides a routine
//			 for intersecting a ray with the mesh.  This intersection operation
//			 only considers intersections with triangles that intersect
//			 cells that lie along the ray's path, therefore it can be much
//			 faster than testing the ray against each triangle.
//
//			 The gpuCacheIsectAccelParams class contains the parameters that
//			 describe the spatial subdivision.  Currently, we only support
//			 a uniform Nx by Ny by Nz uniform grid, but the class could be
//			 generalized later to use other subdivision schemes such as 
//			 octrees or BSP trees.
//
//=============================================================================

class gpuCacheSpatialSubdivision
{
public:

	//	The subdivision doesn't actually get built until the
	//	first call to closestIntersection()
	//
	gpuCacheSpatialSubdivision( unsigned int numTriangles, const index_t* srcTriangleVertIndices, const float* srcPositions,
		const MBoundingBox bounds, const gpuCacheIsectAccelParams& accelParams );

	//	frees memory for the subdivision structure
	//
	~gpuCacheSpatialSubdivision();

	//	find closest intersection, if any, of a ray within the triangles of triArray
	//
	MStatus closestIntersection( 
		const unsigned int numTriangles, 
		const index_t*	srcTriangleVertIndices, 
		const float*	srcPositions,	
		const MPoint& 	origin,
		const MVector& 	direction,
		const MIntArray&triArray,
		float 			maxParam,
		MPoint&			closestIsect,
		MVector&		isectNormal );
	
	//	find closest intersection of a ray with entire grid contents
	//
	MStatus closestIntersection( 
		const unsigned int numTriangles, 
		const index_t*	srcTriangleVertIndices, 
		const float*	srcPositions,	
		const MPoint& 	origin,
		const MVector& 	direction,
		float 			maxParam,
		MPoint&			closestIsect,
		MVector&		isectNormal );

	//	find closest point to a point on a set of triangles
	//
	bool closestPointToPoint(const unsigned int numTriangles, 
		const index_t*	srcTriangleVertIndices, 
		const float*	srcPositions,	
		const MPoint& 	queryPoint,
		MIntArray&		triArray,
		MPoint& closestPoint);
	//	find closest point to a point on the entire surface
	//
	void closestPointToPoint(const unsigned int numTriangles, 
		const index_t*	srcTriangleVertIndices, 
		const float*	srcPositions,	
		const MPoint& 	queryPoint,
		MPoint& closestPoint);

	//	find edge snap point on a set of triangles
	//
	double getEdgeSnapPoint(const unsigned int numTriangles, 
		const index_t*	srcTriangleVertIndices, 
		const float*	srcPositions,
		const MPoint& 	rayPoint,
		const MVector& 	rayDirection,
		MIntArray&		triArray,
		MPoint& closestPoint);

	//	find edge snap point on the entire surface
	//
	double getEdgeSnapPoint(const unsigned int numTriangles, 
		const index_t*	srcTriangleVertIndices, 
		const float*	srcPositions,	
		const MPoint& 	rayPoint,
		const MVector& 	rayDirection,
		MPoint& closestPoint);

	//	determines if the grid was created with parameters compatible with
	//	the given params
	//	
	bool matchesParams( const gpuCacheIsectAccelParams& accelParams );

	//	returns the total amount of memory (in KB) used by the spatial
	//	subdivision structure
	//
	float getMemoryFootprint();

	//	retrieves the amount of time that was used to build the structure
	//	(in seconds)
	//
	float getBuildTime();

	//	returns a string describing the structure and its parameters
	//
	MString getDescription( bool includeStats );

	//	diagnostic stats for all acceleration structures in the system
	//	(how many, total memory footprint, total time required to build them)
	//
	static int totalNumActive();
	static int totalNumCreated();
	static float totalFootprints();
	static float totalBuildTimes();

	//	returns a string describing the total resource usage for all
	//	gpuCacheSpatialSubdivisions in the system.
	//
	static MString systemStats(); 

	//	resets the count of number of spatial subdivisions created (but doesn't
	//	lose track of ones currently allocated), as well as the peak memory
	//	usage and build times.
	//
	static void resetSystemStats(); 
private:

	//	deletes the grid, does appropriate accounting
	//
	void deleteVoxelGrid();

	// 	poly object on which we are doing the lookups
	//
	gpuCacheIsectAccelParams 	fAccelParams;
	gpuCacheVoxelGrid*			fVoxelGrid;

	//	describes the structure
	//
	MString 	fDescription;

	//	time that was used to construct the structure (in seconds)
	//
	float		fMemoryFootprint;
	float		fBuildTime;

	//	static data for accounting purposes
	//
	static int 			fsTotalNumActiveSpatialSubdivisions;	
	static int 			fsTotalNumCreatedSpatialSubdivisions;	
	static float			fsTotalMemoryFootprint;	
	static float			fsTotalBuildTime; 
	static float			fsPeakMemoryFootprint;
};

}
#endif
