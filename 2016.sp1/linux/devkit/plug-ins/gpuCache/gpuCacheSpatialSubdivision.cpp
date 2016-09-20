// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.

//
// Description:  
//
//		This file contains classes that implement the functionality
//		exposed by the gpuCacheSpatialSubdivision class.  The stuff that is
//		accomplished here is:
//
//		1) Load a SpatialGrid data structure with our list of triangles.
//
//		2) Intelligently walk through the grid using to find cells
//		   that lie along a particular ray. This is done using
//		   SpatialGridWalker.
//
//		3) use all this to provide a ray-mesh intersection 
//
//		The file is organized into several parts:
//
//
//		Part 1: Definition of gpuCacheAccelIsectParams, which encapsulates the
//				creation parameters for the spatial subdivision structure.
//
//		Part 2: Definition of gpuCacheSpatialGrid, derived from SpatialGrid. 
//				This class loads spatial grid with face/triangle data. 
//
//		Part 3: Definition of gpuCacheSpatialSubdivision, which finally
//				implements the various intersection methods.
//
//
#include <sys/timeb.h>

#include "gpuCacheSpatialSubdivision.h"
#include <maya/MMatrix.h>
#include <stdlib.h>

#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>

#include <maya/MGlobal.h>
#include <set>

//=============================================================================
//=============================================================================
//
//
//	PART 1:
//
//	- defines gpuCacheAccelIsectParams class, which encapsulates parameters
//	  for creating the spatial subdivision structure.
//
//
//=============================================================================
//=============================================================================

namespace GPUCache {

	gpuCacheIsectAccelParams
		gpuCacheIsectAccelParams::uniformGridParams(
		int divX,
		int divY,
		int divZ
		)
	{
		return gpuCacheIsectAccelParams( gpuCacheIsectAccelParams::kUniformGrid,
			divX, divY, divZ );
	}

	gpuCacheIsectAccelParams
		gpuCacheIsectAccelParams::autoUniformGridParams()
	{
		return gpuCacheIsectAccelParams( gpuCacheIsectAccelParams::kAutoUniformGrid,
			-1, -1, -1 );
	}

	gpuCacheIsectAccelParams::gpuCacheIsectAccelParams()
		: fAlgorithm( gpuCacheIsectAccelParams::kUniformGrid ),
		fDivX(10),
		fDivY(10),
		fDivZ(10)
	{
	}

	gpuCacheIsectAccelParams::gpuCacheIsectAccelParams( 
		int alg, 
		int divX, 
		int divY, 
		int divZ 
		)
		: fAlgorithm(alg),
		fDivX(divX),
		fDivY(divY),
		fDivZ(divZ)
	{
	}

	int gpuCacheIsectAccelParams::operator==( const gpuCacheIsectAccelParams& rhs )
		//
		//	Description:
		//
		//		Compares two acceleration parameter settings to see if they
		//		are the same.  We need to know this in order to determine
		//		whether the acceleration structure needs to be rebuilt or not.
		//	
		//		We don't compare the verbosity field, as it doesn't actually
		//		affect the acceleration structure.
		//
	{
		if( (fAlgorithm == rhs.fAlgorithm) &&
			(fDivX == rhs.fDivX) &&
			(fDivY == rhs.fDivY) &&
			(fDivZ == rhs.fDivZ) )
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int gpuCacheIsectAccelParams::operator!=( const gpuCacheIsectAccelParams& rhs )
		//	
		//	Description:
		//
		//		Opposite of ==
		//
	{
		return !((*this)==rhs);
	}

	//=============================================================================
	//=============================================================================
	//
	// PART 2: 
	// 
	//  - Derive from SpatialGrid to support the data & accessors that we need. 
	// 
	//  This derive class holds on to the list of triangle indices.
	//	SpatialGrid is a data blind structure. Even though we only use it to store 
	//	triangle indices in this case, we can store much more complex data by storing it in a
	//  gpuCacheVoxelGrid and storing its index in SpatialGrid if needed.
	//
	//=============================================================================
	//=============================================================================

	class gpuCacheVoxelGrid : public SpatialGrid { 
	public: 
		typedef SpatialGrid ParentClass; 

		const unsigned int numTriangles;
		const index_t* srcTriangleVertIndices;
		const float* srcPositions;
		gridPoint3<int>* indexArrayRange;

		gpuCacheVoxelGrid( const MBoundingBox &bound, const gridPoint3<int> &numVoxels, unsigned int thisNumTriangles, const index_t* thisSrcTriangleVertIndices, const float* thisSrcPositions):
			SpatialGrid( bound, numVoxels ),
			numTriangles(thisNumTriangles),
			srcTriangleVertIndices(thisSrcTriangleVertIndices),
			srcPositions(thisSrcPositions)
		{
			addTrianglesToGrid();
		}

		virtual ~gpuCacheVoxelGrid(); 

		void operator()( const tbb::blocked_range<unsigned int> &br ) const;
		void				getTris( MIntArray &triArray, const gridPoint3<int> &grid); 
		virtual float		getMemoryFootprint(); 
	private: 
		void addTrianglesToGrid();
	}; 

	gpuCacheVoxelGrid::~gpuCacheVoxelGrid()
	{
	}

	struct TbbBuildVoxelGrid {
		const gpuCacheVoxelGrid *gpuGrid;

		void operator()( const tbb::blocked_range<unsigned int>& br ) const {
			for (unsigned int j = br.begin(); j != br.end(); j++) {
				index_t idx0=gpuGrid->srcTriangleVertIndices[3*j]*3;
				index_t idx1=gpuGrid->srcTriangleVertIndices[3*j+1]*3;
				index_t idx2=gpuGrid->srcTriangleVertIndices[3*j+2]*3;

				MPoint vertex1(gpuGrid->srcPositions[idx0],gpuGrid->srcPositions[idx0+1],gpuGrid->srcPositions[idx0+2]);
				MPoint vertex2(gpuGrid->srcPositions[idx1],gpuGrid->srcPositions[idx1+1],gpuGrid->srcPositions[idx1+2]);
				MPoint vertex3(gpuGrid->srcPositions[idx2],gpuGrid->srcPositions[idx2+1],gpuGrid->srcPositions[idx2+2]);

				//create bbox for this tri
				MBoundingBox bbox; 
				bbox.expand(vertex1);
				bbox.expand(vertex2);
				bbox.expand(vertex3);

				//expand bbox by 1%
				MPoint expandAmount(0.01 * bbox.width(),0.01 * bbox.height(),0.01 * bbox.depth());
				MBoundingBox bbox2(bbox.min()-expandAmount,bbox.max()+expandAmount);

				gpuGrid->getVoxelRange( bbox2, gpuGrid->indexArrayRange[j*2], gpuGrid->indexArrayRange[j*2+1] );
			}
		}

		TbbBuildVoxelGrid( const gpuCacheVoxelGrid *thisGpuGrid ) : gpuGrid(thisGpuGrid) {}
		TbbBuildVoxelGrid( const TbbBuildVoxelGrid &thisTbbVoxelGrid ) : gpuGrid(thisTbbVoxelGrid.gpuGrid) {}
		~TbbBuildVoxelGrid() {}
	};

	void gpuCacheVoxelGrid::addTrianglesToGrid() {
		indexArrayRange = new gridPoint3<int>[2*numTriangles];

		TbbBuildVoxelGrid tbbBVG(this);
		tbb::parallel_for(tbb::blocked_range<unsigned int>(0, numTriangles, 100), tbbBVG, tbb::auto_partitioner());

		for (unsigned int j = 0; j < numTriangles; j++) {
			const gridPoint3<int>& minIndices = indexArrayRange[j*2];
			const gridPoint3<int>& maxIndices = indexArrayRange[j*2+1];

			//	add current triangle to all these voxels
			for( int x = minIndices[0]; x <= maxIndices[0]; x++ ) { 
				for( int y = minIndices[1]; y <= maxIndices[1]; y++ ) { 
					for( int z = minIndices[2]; z <= maxIndices[2]; z++ ) { 
						MUintArray *indices = getVoxelContents( gridPoint3<int>(x,y,z) );
						indices->append( j ); 
					}
				}
			}
		}
		delete [] indexArrayRange;
	}

void gpuCacheVoxelGrid::getTris( MIntArray &triArray,
	const gridPoint3<int> &gridLocation)
	//
	// Description: 
	//  Get the triangles in the specified grid location. 
	//
{
	MUintArray *values = getVoxelContents( gridLocation );
	unsigned int numTriangles = values->length(); 

	// preallocate max possible size to avoid continual reallocs in loop below
	if(triArray.length() < numTriangles){
		triArray.setLength(numTriangles);
	}

	unsigned int nAdded = 0;
	for( unsigned int i = 0; i < numTriangles; i++ ) { 
		unsigned int index = (*values)[i]; 
		triArray[nAdded] = index;
		nAdded++;
	}

	// shrink logical size down to actual size
	if(triArray.length() > nAdded){ 
		triArray.setLength(nAdded);
	}	
}

float 
	gpuCacheVoxelGrid::getMemoryFootprint() 
	// 
	// Description: 
	//  Get the memory footprint for this derived class. This value is the size
	//  of this class(which is 0 i nthis case) plus the size of the base class. 
	//
{
	float totalClassSize = ParentClass::getMemoryFootprint();
	return totalClassSize;
}

//=============================================================================
//=============================================================================
//
//
//	PART 4:
//
//	- finally, defines the gpuCacheSpatialSubdivision class, the top-level
//	  class for accessing the accelerated ray intersect functionality.
//
//	- class also provides some performance tracking and reporting 
//	  statistics, so users can tell how much they are paying in terms
//	  of time and space for the accelerated intersections
//
//=============================================================================
//=============================================================================

class SimpleTimer
{
public:
	SimpleTimer() {};

	void startTimer()
	{
		fStartTime = GetMilliCount();
	}

	double elapsedTime()
	{
		return GetMilliSpan();
	}

private:
	int GetMilliCount()
	{
		timeb tb;
		ftime( &tb );
		int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
		return nCount;
	}

	int GetMilliSpan()
	{
		int nSpan = GetMilliCount() - fStartTime;
		if ( nSpan < 0 )
			nSpan += 0x100000 * 1000;
		return nSpan;
	}
	int fStartTime;
}; 

//	performance counters
//

//	total number of spatial subdivisions currently in existence in Maya
//
int gpuCacheSpatialSubdivision::fsTotalNumActiveSpatialSubdivisions = 0;

//	total number of spatial subdivisions that have been created
//	during this Maya session
//
int gpuCacheSpatialSubdivision::fsTotalNumCreatedSpatialSubdivisions = 0;

//	total amount of memory used for the currently existing spatial 
//	subdivisions
//
float gpuCacheSpatialSubdivision::fsTotalMemoryFootprint = 0.0;

//	peak memory footprint of all active subdivisions at any time
//
float gpuCacheSpatialSubdivision::fsPeakMemoryFootprint = 0.0;

//	total amount of time that has been spent building spatial acceleration
//	structures since Maya was started.  This counter is never reset during
//	a Maya session.
//
float gpuCacheSpatialSubdivision::fsTotalBuildTime = 0.0;

gridPoint3<int> 
	computeBoundsFromTriangleDensity( 
	unsigned int numTriangles, const index_t* srcTriangleVertIndices, const float* srcPositions,
	const MBoundingBox& bounds/*bbox for the whole mesh*/,
	int trianglesPerVoxel,
	const gridPoint3<int>& minVoxels,
	const gridPoint3<int>& maxVoxels
	)
	//
	//	Description:
	//
	//		Comes up with an estimate of the number of grid cells in 
	//		x, y, and z necessary to subdivide the given poly in order
	//		that each grid cell contain roughly "trianglesPerVoxel" triangles.
	//
	//		The number of voxel subdivisions returned will be clamped to the
	//		specified minVoxels and maxVoxels value.  
	//
	//	Notes:
	//
	//		We have found that a trianglesPerVoxel value around 10 works well,
	//		and that subdividing more than 100x100x100 rarely increases performance,
	//		as the cost of walking the voxel structure overwhelms the ray 
	//		intersection cost.
	//
	//		The algorithm analyzes average triangle bounding box sizes along
	//		the x, y, and z axes to decide how big to make the voxels in order
	//		to contain the specified number of triangles, on average.
	//
{
	gridPoint3<int> res;

	//	take the cube root of the desired number of triangles to figure
	//	out roughly how many to place along each axis
	//	
	float trianglesAlongAxis = powf( float(trianglesPerVoxel), 0.33f );

	//	compute the average sizes of triangle bounding boxes along each
	//	dimension
	//	
	float totalSize[3] = { 0.0, 0.0, 0.0 };
	for (unsigned int j = 0; j < numTriangles; j++) {
		index_t idx0=srcTriangleVertIndices[3*j]*3;
		index_t idx1=srcTriangleVertIndices[3*j+1]*3;
		index_t idx2=srcTriangleVertIndices[3*j+2]*3;

		MPoint vertex1(srcPositions[idx0],srcPositions[idx0+1],srcPositions[idx0+2]);
		MPoint vertex2(srcPositions[idx1],srcPositions[idx1+1],srcPositions[idx1+2]);
		MPoint vertex3(srcPositions[idx2],srcPositions[idx2+1],srcPositions[idx2+2]);

		//	get bounding box for triangle
		//	
		MBoundingBox triBound;
		triBound.expand( vertex1 );
		triBound.expand( vertex2 );
		triBound.expand( vertex3 );

		totalSize[0] += triBound.width();	
		totalSize[1] += triBound.height();
		totalSize[2] += triBound.depth();
	}


	float boundSize[3] = {
        (float)bounds.width(),
        (float)bounds.height(),
        (float)bounds.depth()
    };

	//	for each dimension...
	//
	for( int i = 0; i < 3; i++ )
	{
		//	average triangle size along that dimension...
		//
		float avgSize = totalSize[i] / numTriangles;

		//	size of required number of triangles in each voxel
		//
		float voxelSize = avgSize * trianglesAlongAxis;

		//	number of voxels that should result in the proper distribution
		//	along this dimension
		//
		float numVoxels = boundSize[i] / voxelSize;

		//	clamp to provided min/max values
		//
		int iNumVoxels;
		if( numVoxels < minVoxels[i] )
		{
			iNumVoxels = minVoxels[i];
		}
		else if( numVoxels > maxVoxels[i] )
		{
			iNumVoxels = maxVoxels[i];
		}
		else
		{
			iNumVoxels = (int)ceil(numVoxels);
		}

		res[i] = iNumVoxels;
	}

	return res;
}

gpuCacheSpatialSubdivision::gpuCacheSpatialSubdivision( 
	const unsigned int numTriangles, 
	const index_t* srcTriangleVertIndices, 
	const float* srcPositions,	
	const MBoundingBox bounds,
	const gpuCacheIsectAccelParams& accelParams
	)
	: fAccelParams(accelParams)
	//
	//	Description:
	//
	//		This constructor builds an acceleration structure for the 
	//		given gpuCache, organized by the given acceleration parameters.
	//		Currently, the only type of grid supported is a uniform grid.
	//
	//		To avoid numerical problems, expand each triangle's bounding
	//		box by 1% before adding it to the grid.  This ensures that
	//		we won't miss intersections where the triangle lies exactly
	//		on a voxel boundary.
	//
{
	//	timing probe
	//
	SimpleTimer myTimer;
	myTimer.startTimer();
	if( (accelParams.fAlgorithm == gpuCacheIsectAccelParams::kUniformGrid) ||
		(accelParams.fAlgorithm == gpuCacheIsectAccelParams::kAutoUniformGrid) )
	{
		gridPoint3<int> numSub;

		//	for the straight uniform grid, just use the number of subdivisions
		//	passed in, but for the auto uniform grid compute the number of
		//	subdivisions based on average triangle density
		//
		if( fAccelParams.fAlgorithm == gpuCacheIsectAccelParams::kAutoUniformGrid )
		{
			//
			//	we use 12 triangles/voxel, as this seems to produce 
			//	a good number of voxels from an efficiency standpoint.
			//	Any subdivisions past 100x100x100 are usually not helpful
			//
			numSub = computeBoundsFromTriangleDensity( numTriangles, srcTriangleVertIndices, srcPositions,
				bounds, 
				12, 
				gridPoint3<int>(1,1,1), 
				gridPoint3<int>(100,100,100) );
		}
		else
		{
			numSub = gridPoint3<int>( fAccelParams.fDivX, 
				fAccelParams.fDivY, 
				fAccelParams.fDivZ );
		}

		//	Create the voxel grid and load it with our triangle data. 
		//
		fVoxelGrid = new gpuCacheVoxelGrid( bounds, numSub, numTriangles, srcTriangleVertIndices, srcPositions);
	}

	//	update performance counters.  We need to do this regardless of
	//	the verbosity setting.  The user can turn verbosity on/off, so
	//	we need to make sure that the stats are always correct.
	//
	fMemoryFootprint = fVoxelGrid->getMemoryFootprint();
	fBuildTime = (float)myTimer.elapsedTime();
	fsTotalMemoryFootprint += fMemoryFootprint;
	if( fsTotalMemoryFootprint > fsPeakMemoryFootprint )
	{
		fsPeakMemoryFootprint = fsTotalMemoryFootprint;
	}

	fsTotalBuildTime += fBuildTime;
	fsTotalNumActiveSpatialSubdivisions++;
	fsTotalNumCreatedSpatialSubdivisions++;
}

gpuCacheSpatialSubdivision::~gpuCacheSpatialSubdivision()
	//
	//	Description:
	//
	//		Frees the voxel grid.  The grid can also be freed at other times,
	//		such as when it needs to be rebuilt due to frame change,
	//		or a change in acceleration parameters.
	//
{
	deleteVoxelGrid();
}

void gpuCacheSpatialSubdivision::deleteVoxelGrid()
	//
	//	Description:
	//
	//		Frees the voxel grid.  
	//
{
	if( fVoxelGrid != NULL )
	{
		//	update global stats to reflect removal of this structure
		//
		fsTotalNumActiveSpatialSubdivisions--;
		fsTotalMemoryFootprint -= fMemoryFootprint; 

		//	free the grid
		//
		delete fVoxelGrid;
		fVoxelGrid = NULL;
	}			
}

struct TbbFindClosestEdgePoint {
	MPoint closestPoint;
	double minDist;

	const index_t* srcTriangleVertIndices;
	const float* srcPositions;
	const MIntArray &triArray;

	const MPoint &rayPoint;
	const MVector &rayDirection;

	void reset() {
		closestPoint = MPoint::origin;
		minDist = std::numeric_limits<double>::max();
	}

	TbbFindClosestEdgePoint( const index_t * thisSrcTriangleVertIndices, const float *thisSrcPositions, const MPoint &thisRayPoint, const MVector &thisRayDirection, const MIntArray &thisTriArray) : 
				srcTriangleVertIndices(thisSrcTriangleVertIndices), srcPositions(thisSrcPositions), rayPoint(thisRayPoint), rayDirection(thisRayDirection), triArray(thisTriArray) {
		reset();
	}

	TbbFindClosestEdgePoint(const TbbFindClosestEdgePoint& fCEP, tbb::split) : 
				srcTriangleVertIndices(fCEP.srcTriangleVertIndices), srcPositions(fCEP.srcPositions), rayPoint(fCEP.rayPoint), rayDirection(fCEP.rayDirection), triArray(fCEP.triArray) {
		reset();
	}

	void operator()(tbb::blocked_range<size_t> r) {
		int end=r.end();

		for( size_t j=r.begin(); j!=end; ++j ) {
				int triIndex = triArray[j];
				index_t idx0=srcTriangleVertIndices[3*triIndex]*3;
				index_t idx1=srcTriangleVertIndices[3*triIndex+1]*3;
				index_t idx2=srcTriangleVertIndices[3*triIndex+2]*3;

				MPoint vertex1(srcPositions[idx0],srcPositions[idx0+1],srcPositions[idx0+2]);
				MPoint vertex2(srcPositions[idx1],srcPositions[idx1+1],srcPositions[idx1+2]);
				MPoint vertex3(srcPositions[idx2],srcPositions[idx2+1],srcPositions[idx2+2]);

				MPoint clsPoint;
				double dist = gpuCacheIsectUtil::getEdgeSnapPointOnTriangle(rayPoint,rayDirection,vertex1,vertex2,vertex3,clsPoint);
				if(dist<minDist){
					minDist = dist;
					closestPoint = clsPoint;
				}   
			}
	}

	void join( TbbFindClosestEdgePoint &other ) {
		if(other.minDist < minDist ) {
			minDist = other.minDist;
			closestPoint = other.closestPoint;
		}
	}
};

//	find closest point to a ray on a set of triangles
//
double gpuCacheSpatialSubdivision::getEdgeSnapPoint(const unsigned int numTriangles, 
												   const index_t*	srcTriangleVertIndices, 
												   const float*	srcPositions,	
												   const MPoint& 	rayPoint,
												   const MVector& 	rayDirection,
												   MIntArray&		triArray,
												   MPoint& closestPoint)
{

	TbbFindClosestEdgePoint fCP( srcTriangleVertIndices, srcPositions, rayPoint, rayDirection, triArray );
	tbb::parallel_reduce(tbb::blocked_range<size_t>(0, triArray.length()), fCP );
	closestPoint = fCP.closestPoint;
		return fCP.minDist;
}

//	find closest point to a ray on the entire surface
//
double gpuCacheSpatialSubdivision::getEdgeSnapPoint(const unsigned int numTriangles, 
												   const index_t*	srcTriangleVertIndices, 
												   const float*	srcPositions,	
												   const MPoint& 	rayPoint,
												   const MVector& 	rayDirection,
												   MPoint& closestPoint)
{
	MBoundingBox bbox = fVoxelGrid->getBounds();
	std::set< gridPoint3<int> > potentialVoxels;
	gridPoint3<int> numVoxelsByAxis = fVoxelGrid->getNumVoxels();
	int numVoxels = numVoxelsByAxis[0] * numVoxelsByAxis[1] * numVoxelsByAxis[2];
	MPoint voxSizes(bbox.width()/numVoxelsByAxis[0],bbox.height()/numVoxelsByAxis[1],bbox.depth()/numVoxelsByAxis[2]);
	MPoint expandAmount = 0.1*voxSizes;
	double minDist = std::numeric_limits<double>::max();
	bool *checkedBox = new bool[numVoxels];
	double *allDists = new double[numVoxels];
	gridPoint3<int> closestGridPoint;
	for (int i=0;i<numVoxelsByAxis[0];i++) {
		for (int j=0;j<numVoxelsByAxis[1];j++) {
			for (int k=0;k<numVoxelsByAxis[2];k++) {
				gridPoint3<int> gridLocation = gridPoint3<int>(i,j,k);
				MUintArray *values = fVoxelGrid->getVoxelContents( gridLocation );
				int linearIndex = k * (numVoxelsByAxis[0] * numVoxelsByAxis[1]) + j * numVoxelsByAxis[0] + i;
				checkedBox[linearIndex] = false;
				if(values->length()>0){
					MPoint c1 = bbox.min() + MPoint(i*voxSizes[0],j*voxSizes[1],k*voxSizes[2]);
					MPoint c2 = c1 + voxSizes;
					MBoundingBox voxBox(c1-expandAmount, c2+expandAmount);
					MPoint queryPoint;
					allDists[linearIndex] = gpuCacheIsectUtil::getEdgeSnapPointOnBox(rayPoint,rayDirection,voxBox,queryPoint);
					if(allDists[linearIndex] < minDist){
						minDist = allDists[linearIndex];
						closestGridPoint = gridLocation;
					}
				}
				else {
					allDists[linearIndex] = std::numeric_limits<double>::max();
				}
			}
		}
	}

	for (int i=0;i<numVoxelsByAxis[0];i++) {
		for (int j=0;j<numVoxelsByAxis[1];j++) {
			for (int k=0;k<numVoxelsByAxis[2];k++) {
				int linearIndex = k * (numVoxelsByAxis[0] * numVoxelsByAxis[1]) + j * numVoxelsByAxis[0] + i;
				if(allDists[linearIndex]<=minDist){
					potentialVoxels.insert(gridPoint3<int>(i,j,k));
					checkedBox[linearIndex]=true;
				}
			}
		}
	}

	minDist = std::numeric_limits<double>::max();
	while(potentialVoxels.size()>0){
		std::set< gridPoint3<int> >::iterator voxelIt = potentialVoxels.begin();
		gridPoint3<int> gridLoc = *voxelIt;
		potentialVoxels.erase(voxelIt);

		int linearIndex = gridLoc[2] * (numVoxelsByAxis[0] * numVoxelsByAxis[1]) + gridLoc[1] * numVoxelsByAxis[0] + gridLoc[0];
		if(allDists[linearIndex]>minDist) continue;

		MIntArray triArray;
		fVoxelGrid->getTris( triArray, gridLoc ); 

		MPoint clsPoint;
		double dist = getEdgeSnapPoint(numTriangles,srcTriangleVertIndices,srcPositions,rayPoint, rayDirection,triArray,clsPoint);
		if(dist<minDist){
			minDist = dist;
			closestPoint = clsPoint;

			for (int i=0;i<numVoxelsByAxis[0];i++){
				for (int j=0;j<numVoxelsByAxis[1];j++){
					for (int k=0;k<numVoxelsByAxis[2];k++) {
						int linearIndex = k * (numVoxelsByAxis[0] * numVoxelsByAxis[1]) + j * numVoxelsByAxis[0] + i;
						if(!checkedBox[linearIndex] && allDists[linearIndex]<=minDist){
							potentialVoxels.insert(gridPoint3<int>(i,j,k));
							checkedBox[linearIndex]=true;
						}
					}
				}
			}
		} 
	}
	delete[] checkedBox;
	delete[] allDists;
	return minDist;
}

struct TbbFindClosestPoint {
	bool foundPoint;

	MPoint closestPoint;
	double minDist;

	const index_t *srcTriangleVertIndices;
	const float *srcPositions;
	const MIntArray &triArray;
	const MPoint &queryPoint;

	void reset() {
		foundPoint = false;
		closestPoint = MPoint::origin;
		minDist = std::numeric_limits<double>::max();
	}

	TbbFindClosestPoint( const index_t * thisSrcTriangleVertIndices, const float *thisSrcPositions, const MIntArray &thisTriArray, const MPoint &thisQueryPoint ) :
		srcTriangleVertIndices(thisSrcTriangleVertIndices), srcPositions(thisSrcPositions), triArray(thisTriArray), queryPoint(thisQueryPoint) {
		reset();
	}

	TbbFindClosestPoint(const TbbFindClosestPoint& fCP, tbb::split) :
		srcTriangleVertIndices(fCP.srcTriangleVertIndices), srcPositions(fCP.srcPositions), triArray(fCP.triArray), queryPoint(fCP.queryPoint) {
		reset();
	}

	void operator()(tbb::blocked_range<size_t> r) {
		int end=r.end();

		for( int j=r.begin(); j!=end; ++j ) {
			int triIndex = triArray[j];
			index_t idx0=srcTriangleVertIndices[3*triIndex]*3;
			index_t idx1=srcTriangleVertIndices[3*triIndex+1]*3;
			index_t idx2=srcTriangleVertIndices[3*triIndex+2]*3;

			MPoint vertex1(srcPositions[idx0],srcPositions[idx0+1],srcPositions[idx0+2]);
			MPoint vertex2(srcPositions[idx1],srcPositions[idx1+1],srcPositions[idx1+2]);
			MPoint vertex3(srcPositions[idx2],srcPositions[idx2+1],srcPositions[idx2+2]);

			MPoint clsPoint;
			if(gpuCacheIsectUtil::getClosestPointOnTri(queryPoint, vertex1, vertex2, vertex3, clsPoint, minDist)){
				closestPoint = clsPoint;
				foundPoint = true;
			}
		}
	}

	void join( TbbFindClosestPoint &other ) {
		if( other.foundPoint && other.minDist < minDist ) {
			foundPoint = true;
			minDist = other.minDist;
			closestPoint = other.closestPoint;
		}
	}
};

bool gpuCacheSpatialSubdivision::closestPointToPoint(const unsigned int numTriangles, 
													 const index_t*	srcTriangleVertIndices, 
													 const float*	srcPositions,	
													 const MPoint& 	queryPoint,
													 MIntArray&		triArray,
													 MPoint& closestPoint)
{
	TbbFindClosestPoint fCP( srcTriangleVertIndices, srcPositions, triArray, queryPoint );
	tbb::parallel_reduce(tbb::blocked_range<size_t>(0, triArray.length()), fCP );
	if( fCP.foundPoint ) {
		closestPoint = fCP.closestPoint;
		return true;
	}
	return false;
}

void gpuCacheSpatialSubdivision::closestPointToPoint(const unsigned int numTriangles, 
													 const index_t*	srcTriangleVertIndices, 
													 const float*	srcPositions,	
													 const MPoint& 	queryPoint,
													 MPoint& closestPoint)
{
	double minDist = std::numeric_limits<double>::max();
	//Find voxel you are in
	std::set< gridPoint3<int> > potentialVoxels;
	std::set< gridPoint3<int> > checkedVoxels;
	gridPoint3<int> gridLocOrg;

	fVoxelGrid->getClosestVoxelCoords(queryPoint,gridLocOrg);
	potentialVoxels.insert(gridLocOrg);

	bool foundPoint = false;
	int expandVox = 0;
	while (!foundPoint)
	{
		while(potentialVoxels.size()>0){
			std::set< gridPoint3<int> >::iterator voxelIt = potentialVoxels.begin();
			gridPoint3<int> gridLoc = *voxelIt;
			MIntArray triArray;
			fVoxelGrid->getTris( triArray, gridLoc ); 

			checkedVoxels.insert(gridLoc);
			potentialVoxels.erase(voxelIt);

			MPoint clsPoint;
			if(closestPointToPoint(numTriangles,srcTriangleVertIndices,srcPositions,queryPoint,triArray,clsPoint)){
				double dist = queryPoint.distanceTo(clsPoint);

				if(dist<minDist){
					minDist = dist;
					closestPoint = clsPoint;
					foundPoint = true;

					gridPoint3<int> gridLocMin;
					gridPoint3<int> gridLocMax;
					fVoxelGrid->getClosestVoxelCoords(MPoint(queryPoint[0] - dist, queryPoint[1] - dist, queryPoint[2] - dist),gridLocMin);
					fVoxelGrid->getClosestVoxelCoords(MPoint(queryPoint[0] + dist, queryPoint[1] + dist, queryPoint[2] + dist),gridLocMax);

					for(int i=gridLocMin[0]; i<=gridLocMax[0]; i++) {
						for(int j=gridLocMin[1]; j<=gridLocMax[1]; j++) {
							for(int k=gridLocMin[2]; k<=gridLocMax[2]; k++) {
								gridPoint3<int> gridLocNew = gridPoint3<int>(i,j,k);
								if(fVoxelGrid->isValidVoxel(gridLocNew) && checkedVoxels.find(gridLocNew) == checkedVoxels.end()) {
									potentialVoxels.insert(gridLocNew);
								}
							}
						}
					}
				}
			} 
		}
		expandVox++;

		if(!foundPoint){
			for(int i=-expandVox; i<=expandVox; i++) {
				for(int j=-expandVox; j<=expandVox; j++) {
					for(int k=-expandVox; k<=expandVox; k++) {
						gridPoint3<int> gridLocNew = gridLocOrg + gridPoint3<int>(i,j,k);
						if(fVoxelGrid->isValidVoxel(gridLocNew) && checkedVoxels.find(gridLocNew) == checkedVoxels.end()) {
							potentialVoxels.insert(gridLocNew);
						}
					}
				}
			}
		}
	}
}

struct TbbFindClosestIntersection {
	bool foundIntersection;

	MPoint closestIntersection;
	MVector closestNormal;
	double minDist;

	const index_t *srcTriangleVertIndices;
	const float *srcPositions;
	const MIntArray &triArray;
	const MPoint &raySource;
	const MVector &rayDirection;

	void reset() {
		foundIntersection = false;
		closestIntersection = MPoint::origin;
		closestNormal = MVector::zero;
		minDist = std::numeric_limits<double>::max();
	}

	TbbFindClosestIntersection( const index_t * thisSrcTriangleVertIndices, const float *thisSrcPositions, const MIntArray &thisTriArray, const MPoint &thisRaySource, const MVector &thisRayDirection ) :
		srcTriangleVertIndices(thisSrcTriangleVertIndices), srcPositions(thisSrcPositions), triArray(thisTriArray), raySource(thisRaySource), rayDirection(thisRayDirection) {
		reset();
	}

	TbbFindClosestIntersection(const TbbFindClosestIntersection& fCIS, tbb::split) :
		srcTriangleVertIndices(fCIS.srcTriangleVertIndices), srcPositions(fCIS.srcPositions), triArray(fCIS.triArray), raySource(fCIS.raySource), rayDirection(fCIS.rayDirection) {
		reset();
	}

	void operator()(tbb::blocked_range<size_t> r) {
		int end=r.end();

		for( int i=r.begin(); i!=end; ++i ) {
			int triIndex = triArray[i];
			index_t idx0=srcTriangleVertIndices[3*triIndex]*3;
			index_t idx1=srcTriangleVertIndices[3*triIndex+1]*3;
			index_t idx2=srcTriangleVertIndices[3*triIndex+2]*3;

			MPoint vertex1(srcPositions[idx0],srcPositions[idx0+1],srcPositions[idx0+2]);
			MPoint vertex2(srcPositions[idx1],srcPositions[idx1+1],srcPositions[idx1+2]);
			MPoint vertex3(srcPositions[idx2],srcPositions[idx2+1],srcPositions[idx2+2]);

			MVector c0, c1, rhs, crossc1c2, crossc0rhs;
			double beta, gamm, t, M;

			c0 = vertex1 - vertex2;
			c1 = vertex1 - vertex3;
			rhs = vertex1 - MVector(raySource);

			crossc1c2 = c1 ^ rayDirection;
			crossc0rhs = c0 ^ rhs;
			M = c0 * crossc1c2;
			if (M==0) continue;

			t = -(c1 * crossc0rhs)/M; 
			if (t < 0.0 || t > minDist) continue;

			beta = (rhs * crossc1c2)/M;  
			if (beta < 0  || beta > 1) continue;

			gamm = (rayDirection * crossc0rhs)/M;
			if (gamm < 0 || gamm > 1 - beta) continue;

			//Passed all tests
			minDist = t;
			closestIntersection = raySource + t * rayDirection;
			closestNormal = (c0 ^ c1).normal();
			foundIntersection = true;
		}
	}

	void join( TbbFindClosestIntersection &other ) {
		if( other.foundIntersection && other.minDist < minDist ) {
			foundIntersection = true;
			minDist = other.minDist;
			closestIntersection = other.closestIntersection;
			closestNormal = other.closestNormal;
		}
	}
};

MStatus gpuCacheSpatialSubdivision::closestIntersection( 
	const unsigned int numTriangles, 
	const index_t*	srcTriangleVertIndices, 
	const float*	srcPositions,	
	const MPoint& 	origin,
	const MVector& 	direction,
	const MIntArray& triArray,
	float 			maxParam,
	MPoint&			closestIsect,
	MVector&		isectNormal
	)
{
	TbbFindClosestIntersection fCIS( srcTriangleVertIndices, srcPositions, triArray, origin, direction );
	tbb::parallel_reduce(tbb::blocked_range<size_t>(0, triArray.length()), fCIS );
	if( fCIS.foundIntersection ) {
		closestIsect = fCIS.closestIntersection;
		isectNormal = fCIS.closestNormal;
		return MStatus::kSuccess;
	}
	return MStatus::kFailure;
}

MStatus gpuCacheSpatialSubdivision::closestIntersection( 
	const unsigned int numTriangles, 
	const index_t*	srcTriangleVertIndices, 
	const float*	srcPositions,	
	const MPoint& 	origin,
	const MVector& 	direction,
	float 			maxParam,
	MPoint&			closestIsect,
	MVector&		isectNormal
	)
	//-----------------------------------------------------------------------------
	//
	//	Purpose:	Returns the closest intersection of the given ray with the
	//				contents of the intersection structure (which is assumed
	//				to be triangles from the given ShapeNode). 
	//
	//	Parameters:
	//
	//		numTriangles			- number of triangles for the model
	//		srcTriangleVertIndices	- pointer to the index buffer that has triangle indices
	//		srcPositions			- pointer to the vertex buffer that has vertex positions
	//
	//		origin			- origin of the ray
	//		direction	 	- direction of the ray
	//		maxParam		- maximum parametric distance along the ray at which
	//						  an intersection will be considered valid.
	//		closestIsect	- receives the closest valid intersection, if one is
	//						  found.
	//		isectNormal		- receives the surface normal at the closest valid intersection, 
	//						  if one is found.
	//
	//	Returns:
	//
	//		MStatus::kSuccess if a valid hit was found
	//		MStatus::kFailure otherwise.
	//
	//		If a hit was found, closestIsect and isectNormal will be set to the 
	//		position and surface normal at the intersection.
	//
	//-----------------------------------------------------------------------------
{
	//	walks the grid voxels
	//
	SpatialGridWalker it = fVoxelGrid->getRayIterator( origin, direction );

	maxParam = fabs(maxParam);

	while( !it.isDone() )
	{
		//	exit if the current voxel is past the maximum distance for hits
		//
		if( it.curVoxelStartRayParam() > maxParam )
		{
			break;
		}

		//	consider the current voxel's contents
		//
		gridPoint3<int> gridLoc = it.gridLocation(); 
		MIntArray triArray;
		fVoxelGrid->getTris( triArray, gridLoc); 
		if ( triArray.length() > 0 ) { 
			//	make sure we only consider hits that lie within this voxel,
			//	otherwise we might get an incorrect result for the closest 
			//	hit 
			//
			float voxelMaxParam = std::min( it.curVoxelEndRayParam(), maxParam );

			//	intersect the ray with the current voxel's triangles
			//
			if( MStatus::kSuccess == closestIntersection( numTriangles, srcTriangleVertIndices, srcPositions, origin, direction, 
				triArray, voxelMaxParam, closestIsect, isectNormal ) )
			{
				return MStatus::kSuccess;
			}
		}

		it.next();
	}
	return MStatus::kFailure;
}

float gpuCacheSpatialSubdivision::getMemoryFootprint()
	//
	//	Description:
	//
	//		Returns the total amount of memory used by this structure.
	//
{
	return fMemoryFootprint;
}	

float gpuCacheSpatialSubdivision::getBuildTime()
	//
	//	Description:
	//
	//		Returns the total number of seconds used to build this structure
	//
{
	return fBuildTime;
}

MString gpuCacheSpatialSubdivision::getDescription( bool includeStats )
	//
	//	Description:
	//
	//		Returns a string describing the structure.  The description will
	//		look something like:
	//
	//		10x10x10 Uniform Grid
	//		
	//		or
	//
	//		10x11x23 Auto-Configured Uniform Grid
	//
	//		If includeStats is true, the memory footprint and build time (in 
	//		seconds) will be appended to the description string.
	//
{

	gridPoint3<int> numVoxels = fVoxelGrid->getNumVoxels();

	char buf[512];
	if( fAccelParams.fAlgorithm == gpuCacheIsectAccelParams::kUniformGrid )
	{
		sprintf( buf, "%dx%dx%d Uniform Grid", numVoxels[0], 
			numVoxels[1], 
			numVoxels[2] );
	}
	else if( fAccelParams.fAlgorithm == gpuCacheIsectAccelParams::kAutoUniformGrid )
	{
		sprintf( buf, "%dx%dx%d Auto-Configured Uniform Grid", 
			numVoxels[0], numVoxels[1], numVoxels[2] );
	}

	MString resultStr( buf );

	if( includeStats )
	{
		char buf2[512];
		sprintf( buf2, "build time %.2fs", fBuildTime );
		MString buildTimeStr( buf2 );

		sprintf( buf2, "memory footprint %.2fKB", fMemoryFootprint );
		MString footprintStr( buf2 );

		resultStr += MString(", (") + buildTimeStr + MString("), (") + 
			footprintStr + MString(")");
	}

	return resultStr;
}

MString gpuCacheSpatialSubdivision::systemStats()
	//
	//	Description:
	//
	//		Returns an informative string describing the total resource
	//		usage for all spatial subdivisions in the system.  The string
	//		looks something like:
	//
	//		total 10 isect accelerators, total build time = 5.13s, total memory = 1510.6KB
	//
{
	char buf[1024];
	sprintf( buf, "total %d isect accelerators created (%d currently active - "
		"total current memory = %.2f KB), total build time = %f ms, "
		"peak memory = %.2f KB\n",
		fsTotalNumCreatedSpatialSubdivisions, 
		fsTotalNumActiveSpatialSubdivisions, 
		fsTotalMemoryFootprint, 
		fsTotalBuildTime, 
		fsPeakMemoryFootprint );

	return MString(buf);
}

void gpuCacheSpatialSubdivision::resetSystemStats()
	//
	//	Description:
	//
	//		Resets the global statistics counters for the following:
	//
	//		- total number of spatial subdivisions created so far
	//		- peak memory usage of all spatial subdivisions
	//		- total build time for all spatial subdivisions
	//
{
	fsTotalNumCreatedSpatialSubdivisions = 0;
	fsTotalBuildTime = 0.0f;
	fsPeakMemoryFootprint = 0.0f;
}

bool
	gpuCacheSpatialSubdivision::matchesParams( 
	const gpuCacheIsectAccelParams& accelParams 
	)
	//
	//	Description:
	//
	//		Determines whether this accelerator was built with parameters
	//		identical to the given ones.
	//
{
	if( fVoxelGrid != NULL )
	{
		return (fAccelParams == accelParams) ? true : false;
	}
	else
	{
		return false;
	}
}
}
