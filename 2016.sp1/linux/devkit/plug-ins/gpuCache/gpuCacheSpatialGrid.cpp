// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.

#include "gpuCacheSpatialGrid.h" 
#include "gpuCacheSpatialGridWalker.h" 
#include "gpuCacheIsectUtil.h"

#include <maya/MPointArray.h>

SpatialGrid::SpatialGrid( 
	const MBoundingBox& boundingBox, 
	const gridPoint3<int>& numVoxels 
	)
	//
	//	Description:
	//
	//		Constructure just initializes bounding box sizes and sets
	//		all voxel entries to NULL pointers.
	//
	:	fBounds(boundingBox),
	fNumVoxels(numVoxels)
{
	//	artificially expand the bounding box if is too small along one or more
	//	axes
	//
	double minSize[3] = { 0.01, 0.01, 0.01 };
	expandBBoxByPercentage( fBounds, 1.0, minSize );
	
	//	figure out voxel sizes
	//
	fVoxelSizes[0] = fBounds.width() / fNumVoxels[0];
	fVoxelSizes[1] = fBounds.height() / fNumVoxels[1];
	fVoxelSizes[2] = fBounds.depth() / fNumVoxels[2];

	//	make sure none of the voxels are tiny along one dimension
	//
	if( fVoxelSizes[0] < 0.01 )
	{
		fVoxelSizes[0] = fBounds.width();
		fNumVoxels[0] = 1;
	}

	if( fVoxelSizes[1] < 0.01 )
	{
		fVoxelSizes[1] = fBounds.height();
		fNumVoxels[1] = 1;
	}

	if( fVoxelSizes[2] < 0.01 )
	{
		fVoxelSizes[2] = fBounds.depth();
		fNumVoxels[2] = 1;
	}

	//	NULL out all voxel contents
	//
	fVoxels.clear();
	unsigned int totalVoxels = fNumVoxels[0] * fNumVoxels[1] * fNumVoxels[2];
	for( unsigned int i = 0; i < totalVoxels; i++ )
	{
		fVoxels.push_back(NULL);
	}
}

SpatialGrid::~SpatialGrid()
	//
	//	Description:
	//
	//		Just delete non-empty voxel grid entries.
	//
{
	int numVoxels = fNumVoxels[0] * fNumVoxels[1] * fNumVoxels[2];
	for( int v = 0; v < numVoxels; v++ )
	{
		if( fVoxels[v] != NULL )
		{
			delete fVoxels[v];
		}
	}
}

//
//	Some simple accessors
//
const gridPoint3<int>& SpatialGrid::getNumVoxels()
{
	return fNumVoxels;
}

void SpatialGrid::bounds( MPoint& lowerCorner, MPoint& upperCorner )
{
	lowerCorner = fBounds.min();
	upperCorner = fBounds.max();
}

const MBoundingBox& SpatialGrid::bounds()
{
	return fBounds;
}

int SpatialGrid::getLinearVoxelIndex( const gridPoint3<int>& index ) const
	//
	//	Description:
	//
	//		Figures out which linear array element represents the
	//		voxel with the given x,y,z indices.  Remember, voxels
	//		are stored by increasing order of X, then Y, then Z coordinate
	//		indices.
	//
{
	return index[2]*(fNumVoxels[0]*fNumVoxels[1]) 
		+ index[1]*fNumVoxels[0] 
	+ index[0];
}

void 
	SpatialGrid::getVoxelRange(
	const MBoundingBox& box,
	gridPoint3<int>& minIndices,
	gridPoint3<int>& maxIndices 
	) const
	//
	//	Description:
	//
	//		Given a bounding box, compute the min and max voxel
	//		indices (in x, y, z) of the cells that intersect the box.
	//
{
	//	get bbox corners
	//
	const MPoint& minPt = box.min();
	const MPoint& maxPt = box.max();

	//	return indices for min/max corners
	//
	getVoxelCoords( minPt, minIndices, NULL );
	getVoxelCoords( maxPt, maxIndices, NULL );
}

void SpatialGrid::getVoxelCoords( 
	const MPoint& point, 
	gridPoint3<int>& coords,
	MPoint *residuals ) const
	//
	//	Description:
	//
	//		Given a point, compute the x,y,z indices of the voxel grid
	//		in which it resides.  Optionally, return residual which gives
	//		distance from point to next-lowest grid line value in each
	//		dimension.
	//
{
	//	get point relative to voxel grid lower corner
	//
	MPoint relPoint = point - fBounds.min();

	//	get indices for each axis
	//
	for( int axis = 0; axis < 3; axis++ )
	{
		//	figure out which cell point resides in
		//
		float voxSpace = relPoint[axis] / fVoxelSizes[axis];
		coords[axis] = (int)floor( voxSpace );

		//	clamp coordinate to valid range
		//
		if( coords[axis] < 0 ) 
		{
			coords[axis] = 0;
		}
		else if( coords[axis] >= fNumVoxels[axis] )
		{
			coords[axis] = fNumVoxels[axis]-1;
		}

		//	compute residual
		//		
		if( residuals != NULL )
		{
			(*residuals)[axis] = fVoxelSizes[axis]*(voxSpace-coords[axis]);
		}
	}
}

void SpatialGrid::expandBBoxByPercentage( 
	MBoundingBox& bbox,
	double percentage,
	double min[3] 
)
	//
	//	Description:
	//
	//		Expands the given bounding box by the given percentage
	//		in all dimensions.  Percentage should be a value between
	//		0 and 1, representing 0% to 100%.
	//
	//		The optional 3 "min" values specify minimum sizes along each
	//		axis that the bounding box size will be expanded to.  This
	//		is useful for situations where one of the box axes is so small
	//		that a percentagewise increase will not be meaningful.
	//
{
	percentage += 1.0;

	MPoint c = bbox.center();

	float w = bbox.width();
	float h = bbox.height();
	float d = bbox.depth();

	//	clamp the box sizes to the minimums, if given
	//
	if( min != NULL )
	{	
		if( w < min[0] )
		{
			w = min[0];
		}

		if( h < min[1] ) 
		{
			h = min[1];
		}

		if( d < min[2] )
		{
			d = min[2];
		}
	}

	//	increase the box size
	//
	MVector offset(w, h, d);
	offset *= (0.5f * percentage);

	bbox.expand( c + offset);
	bbox.expand( c - offset);
}

void SpatialGrid::getClosestVoxelCoords( 
	const MPoint& point, 
	gridPoint3<int>& coords) const
	//
	//	Description:
	//
	//		Given a point, compute the x,y,z indices of the voxel grid
	//		that it is closest to.   
	//
{	
	MPoint c1 = fBounds.min() + MVector(fBounds.width()/4, fBounds.height()/4, fBounds.depth()/4);
	MPoint c2 = fBounds.max() - MVector(fBounds.width()/4, fBounds.height()/4, fBounds.depth()/4);
	MBoundingBox bounds(c1,c2);
	MPoint relPoint;
	if(bounds.contains(point)){
		relPoint = point - fBounds.min();
	} else {
		//snap to bbox
		MPoint closestPoint = point;
		GPUCache::gpuCacheIsectUtil::getClosestPointOnBox(point, bounds, closestPoint);
		relPoint = closestPoint - fBounds.min();
	} 

	for( int axis = 0; axis < 3; axis++ )
	{
		//	figure out which cell point resides in
		//
		float voxSpace = relPoint[axis] / fVoxelSizes[axis];
		coords[axis] = (int)floor( voxSpace );
	}
	
}

bool SpatialGrid::isValidVoxel(gridPoint3<int>& vox){
	if(vox[0]>=0 && vox[0]<fNumVoxels[0] && vox[1]>=0 && vox[1]<fNumVoxels[1] && vox[2]>=0 && vox[2]<fNumVoxels[2]) 
		return true;
	return false;
}

MUintArray *
	SpatialGrid::getVoxelContents( 
	const gridPoint3<int>& index 
	)
	//
	//	Description:
	//
	//		Returns list of triangles for given voxel.  Allocates the array
	//		if it doesn't exist already
	//
{
	int linearIndex = getLinearVoxelIndex( index );
	if( fVoxels[linearIndex] == NULL )
	{
		fVoxels[linearIndex] = new MUintArray;
	}

	return fVoxels[linearIndex];
}

float SpatialGrid::getMemoryFootprint()
	//
	//	Description:
	//
	//		Returns total amount of memory used by acceleration
	//		structure.  This is the sum of the physical sizes of
	//		all the voxel entries, plus the physical size of the 
	//		linear voxel array as well.
	//
	//		The result is returned in KB.
	//
{
	int totalSize = 0;

	//	total up grid cell contents
	//
	int numVoxels = fNumVoxels[0]*fNumVoxels[1]*fNumVoxels[2];
	for( int v = 0; v < numVoxels; v++ )
	{
		if( fVoxels[v] != NULL )
		{
			totalSize += fVoxels[v]->length()*sizeof(unsigned int);
		}
	}

	//	also add space required for linear array of voxels
	//
	totalSize += fVoxels.size()*sizeof(MUintArray*);

	return ((float)totalSize)/1024.0f;
}

SpatialGridWalker SpatialGrid::getRayIterator( 
	const MPoint& origin,
	const MVector& direction )
	//
	//	Description:
	//
	//		Returns an iterator that will walk through every voxel 
	//		intersected by the ray from 'origin' along direction
	//		'direction'.  The iterator starts off in a valid grid 
	//		cell on initialization.
	//
{
	return SpatialGridWalker( origin, direction, this );
}
