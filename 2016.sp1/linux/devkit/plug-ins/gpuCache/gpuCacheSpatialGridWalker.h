#ifndef __SpatialGridWalker_h 
#define __SpatialGridWalker_h 

//-----------------------------------------------------------------------------
//
//	Class: SpatialGridWalker
//	
//	Purpose: This class can be used to walk through all the voxels that
//			 are intersected by a particular ray.  Voxel cells will be
//			 visited in order of increasing distance from the ray origin.
//			 If the origin of the ray lies outside the voxel grid, it will
//			 be advanced to its nearest intersection with the grid, if it
//			 does intersect the grid.  If the ray does not intersect the
//			 grid at all, the isDone() method will return true immediately
//			 when the iterator is created.
//
//			 Loops with this iterator should look like:
//
//				SpatialGridWalker it = voxelGrid->getRayIterator(origin,dir);
//				while( !it.isDone() )
//				{
//					// do stuff
//					it.next();
//				}
//
//	Algorithm:
//
//		The ray is traveling through an axis-aligned grid of voxels.  For
//		each step of the iterator, we advance the ray to the next voxel, which
//		is adjacent to the current voxel along either the x, y, or z axis.
//		Thus, the decision at each step is which axis to advance along.
//
//		At all times, the ray stores the parametric distances (t-values) from 
//		its current position to its x,y,z neighbors.  Whichever of these
//		distances is lowest dictates which voxel we move to next.
//
//		When we move to the next voxel, we must update the parametric distances.
//		Say we decided to advance along x by the parametric value t.  Now we
//		just update the y and z distances by subtracting t from them.  To 
//		compute the new x distance, we just divide the voxel size in x by the 
//		ray direction's x component - this gives the parametric distance 
//		required to walk across the voxel in that dimension.
//
//		After updating the distances, we also must choose which is the next
//		axis we will advance along - this is just a matter of figuring out
//		which axis' parametric distance value is now the lowest.
// 
// 
//-----------------------------------------------------------------------------

class SpatialGrid; 

class SpatialGridWalker { 
public:

	//	constructor specifies ray details and which grid to walk
	//
	SpatialGridWalker( const MPoint& origin,
		const MVector& direction,
		SpatialGrid *grid );

	//	contents of current voxel. 
	//
	MUintArray *		voxelContents();
	gridPoint3<int>				gridLocation(); 

	//	ray-parametric distances to the start and end of the current
	//	voxel.
	//
	float	curVoxelStartRayParam();
	float	curVoxelEndRayParam();

	//	advances to next voxel along ray
	//
	void 	 next();

	//	returns true if ray has left the voxel grid after the last
	//	next() call, or if the ray never intersects the voxel grid
	//
	bool isDone();

private:

	//	grid being walked through
	// 
	SpatialGrid *		fVoxelGrid;

	//	ray parameters
	//
	MPoint			fOrigin;
	MVector		fDirection;

	//	indices for current voxel in the traversal
	//
	gridPoint3<int>				fCurVoxelCoords;

	//	parametric distances along the ray to the
	//	next voxel grid cells in the x, y, and z directions
	//
	gridPoint3<float>				fCurDistances;

	//	decides which axis the ray will hit next (0=x, 1=y, 2=z)
	//
	int				fNextAxis;

	//	current distance along the ray to the first intersection point
	//	with the current voxel
	//
	float				fCurVoxelStartRayParam;
	float				fCurVoxelEndRayParam;
	//	false if the ray is currently in a valid voxel, true otherwise
	//
	bool			fDone;
};

#endif 
