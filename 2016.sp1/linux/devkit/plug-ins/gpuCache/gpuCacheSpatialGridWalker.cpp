// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.

#include "gpuCacheSpatialGrid.h" 
#include "gpuCacheSpatialGridWalker.h" 
#include <maya/MIntArray.h> 
#include "gpuCacheIsectUtil.h"

SpatialGridWalker::SpatialGridWalker( 
	const MPoint& origin,
	const MVector& direction,
	SpatialGrid *grid )
	//
	//	Description:
	//
	//		Initializes the ray for its walk through the voxel grid.
	//		We must do the following:
	//
	//		- if the ray is outside the voxel grid bounding box, we must 
	//		  snap it to its closest intersection with the box.  
	//
	//		- compute the initial fDistance values, which give the parametric
	//		  distances to the x,y, and z axes along the ray
	//
	//		- compute the initial fNextAxis value, which tells us which
	//		  axis we will next advance along
	//
	:
fVoxelGrid(grid),
	fOrigin(origin),
	fDirection(direction),
	fDone(false)
{ 
	//	First, snap the ray to the bounding box if necessary
	//
	if( !grid->bounds().contains( origin ) )
	{
		MPoint boxIntersectionPt;
		if(GPUCache::gpuCacheIsectUtil::firstRayIntersection((grid->bounds().min()), (grid->bounds().max()), origin, direction, NULL, &boxIntersectionPt))
		{
			//	ray intersects bounding box, so snap the origin to the
			//	closest hit on the outside of the box
			//
			fOrigin = MPoint( boxIntersectionPt.x, 
				boxIntersectionPt.y, 
				boxIntersectionPt.z );

			float dist = (float) boxIntersectionPt.distanceTo( origin );
			float length = (float) direction.length();
			fCurVoxelStartRayParam = dist/ length;

			fDone = false;
		}
		else
		{
			//	ray doesn't hit box, so it can't hit anything inside the
			//	voxel grid, thus the iterator is done
			//
			fDone = true;
			return;
		}
	}
	else
	{
		fCurVoxelStartRayParam = 0;
	}

	fNextAxis = 0;
	fCurDistances = gridPoint3<float>( 1.0e8, 1.0e8, 1.0e8 );

	//	figure out which grid cell we are in, and how far we are from
	//	the lower corner of that cell
	//
	MPoint residual;
	grid->getVoxelCoords( fOrigin, fCurVoxelCoords, &residual );

	//	for each axis, figure out how far we need to follow the ray before
	//	we hit the next grid line in that axis.  the parametric value to 
	//	the grid line is just the actual distance to the line divided by
	//	the ray's component along the axis.
	//
	for( int axis = 0; axis < 3; axis++ )
	{
		//	take into account that we may be heading towards the next-lowest
		//	grid line or the next-highest, depending on the sign of the ray
		//	direction coordinate for this axis
		//
		if( fDirection[axis] > 0 )
		{
			fCurDistances[axis] = (grid->fVoxelSizes[axis]-
				residual[axis])/fDirection[axis];
		}
		else if( fDirection[axis] < 0 )
		{
			fCurDistances[axis] = -(residual[axis]/fDirection[axis]);
		}
		else
		{
			//	ray direction component along this axis is 0, meaning ray will
			//	never reach the grid cell adjacent along this axis.  just set
			//	the distance to huge value to make sure that we never advance
			//	along that axis
			//
			fCurDistances[axis] = 1.0e8;
		}

		//	store which axis has the smallest distance
		//
		if( fCurDistances[axis] < fCurDistances[fNextAxis] )
		{
			fNextAxis = axis;
		}
	}

	//	figure out total parametric distance from ray origin to end of
	//	this voxel
	//
	fCurVoxelEndRayParam = fCurVoxelStartRayParam + fCurDistances[fNextAxis];
}

void SpatialGridWalker::next()
	//
	//	Description:
	//
	//		Walks the iterator to the voxel adjacent to the current voxel
	//		that the ray will hit next.	
	//
	//		The axis specified by fNextAxis (x=0, y=1, z=2) tells us which
	//		is closest, so we just advance along that axis, and update
	//		the fDistance distances and the fNextAxis value.  We also need
	//		to watch for when the ray leaves the grid, in which case the
	//		fDone member is set to true to indicate that all voxels have
	//		been traversed.
	//
{
	//	axes are represented by indices x=0, y=1, z=2, which makes it
	//	easy to write code that operates on any axis, rather than
	//	having to explicitly code cases for x, y, and z.  
	//
	int curAxis = fNextAxis;
	int otherAxis1 = (curAxis+1)%3;
	int otherAxis2 = (curAxis+2)%3;

	//	we are going to go to the voxel that is adjacent to the current one
	//	along the fNextAxis axis.  Figure out if we are going to a higher
	//	or lower voxel, and figure out if we are leaving the grid.
	//	
	if( fDirection[curAxis] >= 0.0 )
	{
		fCurVoxelCoords[curAxis] += 1;
		if( fCurVoxelCoords[curAxis] >= fVoxelGrid->fNumVoxels[curAxis] )
		{
			fDone = true;
		}
	}
	else
	{
		fCurVoxelCoords[curAxis] -= 1;
		if( fCurVoxelCoords[curAxis] < 0 )
		{
			fDone = true;
		}
	}

	fCurVoxelStartRayParam += fCurDistances[curAxis];

	//	update the fCurDistances, the parametric distances to the closest
	//	adjacent voxels in the x,y,z directions.  We know that
	//	fCurDistances[curAxis] is the smallest, and we are moving that far,
	//	so just subtract that value from the distances for the other axes
	//	
	fCurDistances[otherAxis1] -= fCurDistances[curAxis];
	fCurDistances[otherAxis2] -= fCurDistances[curAxis];

	//	update the distance for the current axis.  Since we have advanced to
	//	the boundary of a voxel along that axis, the new required distance
	//	is a full voxel width in the specified axis.  Make sure to get the
	//	sign right - the distance must always be positive.
	//
	fCurDistances[curAxis] = fVoxelGrid->fVoxelSizes[curAxis] / 
		fabs(fDirection[curAxis]);

	//	figure out which axis now has the smallest distance.  It could be
	//	x, y, or z
	//
	if( fCurDistances[otherAxis1] < fCurDistances[otherAxis2] )
	{
		if( fCurDistances[otherAxis1] < fCurDistances[fNextAxis] )
		{
			fNextAxis = otherAxis1;
		}
	}
	else
	{
		if( fCurDistances[otherAxis2] < fCurDistances[fNextAxis] )
		{
			fNextAxis = otherAxis2;
		}
	}

	//	recompute ray-parametric distance to end of new voxel
	//
	fCurVoxelEndRayParam = fCurVoxelStartRayParam + fCurDistances[fNextAxis];
}

bool SpatialGridWalker::isDone()
	//
	//	Description:
	//
	//		Returns true when the ray has traversed all voxel grid cells
	//		that it intersects.
	//
{
	return fDone;
}

float SpatialGridWalker::curVoxelStartRayParam()
	//
	//	Description:
	//
	//		Returns ray-parametric distance to the start of the current
	//		voxel.  This is useful for determining whether all the voxel
	//		contents lie beyond a particular distance from the ray origin.
	//
{
	return fCurVoxelStartRayParam;
}

float SpatialGridWalker::curVoxelEndRayParam()
	//
	//	Description:
	//
	//		Returns ray-parametric distance to the end of the current
	//		voxel.  
	//
{
	return fCurVoxelEndRayParam;
}

MUintArray *SpatialGridWalker::voxelContents()
	//
	//	Description:
	//
	//		Returns the contents of the voxel in which the iterator
	//		currently resides.  This is a list of triangles that
	//		can be NULL if no triangles are found in the voxel.
	//
{
	return fVoxelGrid->getVoxelContents( fCurVoxelCoords );
}

gridPoint3<int> 
	SpatialGridWalker::gridLocation()
	//
	// Description: 
	//  Returns the current location within the grid. 
	//
{
	return fCurVoxelCoords; 
}
