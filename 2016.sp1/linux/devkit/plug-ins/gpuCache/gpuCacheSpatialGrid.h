#ifndef __SpatialGrid_h 
#define __SpatialGrid_h 
//-----------------------------------------------------------------------------
//
//	Class: SpatialGrid
//	
//	Purpose: 
//			 This class just provides a bunch of grid cells containing
//			 lists of index data, and some indexing functions to find
//			 grid cells corresponding to bounding boxes in space.  The
//			 voxel grid is axis-aligned.
//
//			 The SpatialGridWalker iterator knows how to walk through
//			 the grid cells that are intersected by a ray.
//
//			 The voxels are organized into a flat array, in X-Y-Z order
//			 (moving linearly through array, X coordinate grows fastest,
//			 then Y, then Z).
// 
//			 Data in the spatial grid is blind, i.e. the underlying
//			 class does not understand anything about the data (other
//			 than it is a uint). The ideal way to store data in
//			 the table would be to create your own array of data then
//			 use the indices of that array to map the contents of the
//			 grid to your domain specific data.
// 
//
//-----------------------------------------------------------------------------

#include <maya/MBoundingBox.h>
#include <maya/MIntArray.h>
#include <maya/MUintArray.h>

#include <vector>

// Class represents a 3-integer index to the spatial grid.
// Contains a comparison value to be used in 
// sorting grid points (used along with std::set)
template <class T>
class gridPoint3 {
public:
	gridPoint3(){}
	gridPoint3(T _1, T _2, T _3){
		fData[0] = _1;
		fData[1] = _2;
		fData[2] = _3;
		fCompareVal = fData[0]+3083*fData[1]+7919*fData[2]; //using two prime numbers
	}

	operator T *(){return fData;}
	operator const T *() const{return fData;}
	double compareVal()const { return fCompareVal; }
	
	gridPoint3 operator+ (const gridPoint3& x) const{gridPoint3 y(x); y.fData[0]+=fData[0]; y.fData[1]+=fData[1]; y.fData[2]+=fData[2]; return y; }
	gridPoint3 operator+ (const T& x) const{ gridPoint3 y(*this); y.fData[0]+=x; y.fData[1]+=x; y.fData[2]+=x; return y; }
	gridPoint3 operator- (const T& x) const{ gridPoint3 y(*this); y.fData[0]-=x; y.fData[1]-=x; y.fData[2]-=x; return y; }
	gridPoint3 operator* (const gridPoint3& x) const{gridPoint3 y(x); y.fData[0]*=fData[0]; y.fData[1]*=fData[1]; y.fData[2]*=fData[2]; return y;}
	gridPoint3 operator* (const T& x)  const{ gridPoint3 y(*this); y.fData[0]*=x; y.fData[1]*=x; y.fData[2]*=x; return y; }

	gridPoint3& operator+= (const gridPoint3& x){ fData[0]+=x[0]; fData[1]+=x[1]; fData[2]+=x[2]; return *this;}
	gridPoint3& operator*= (const gridPoint3& x){fData[0]*=x[0]; fData[1]*=x[1]; fData[2]*=x[2]; return *this;}
	gridPoint3& operator*= (const T& x){fData[0]*=x;    fData[1]*=x;    fData[2]*=x;    return *this;}

	bool operator< (const gridPoint3& y) const{
		return fCompareVal < y.compareVal();
	}

private:
	T fData[3];
	double fCompareVal;
};

class SpatialGridWalker;

class SpatialGrid { 
public:
	//	constructor specifies region of space to be gridded, and number of
	//	grid cells along each dimension
	//
	SpatialGrid( const MBoundingBox& bound, const gridPoint3<int>& numVoxels );

	virtual ~SpatialGrid();

	//	retrieves an iterator that walks through grid cells intersected
	//	by the ray from "origin" along "direction"
	//
	SpatialGridWalker getRayIterator( const MPoint& origin,
		const MVector& direction );

	//	computes x,y,z voxel coords for given 3d point (assumed to be 
	//	inside bounding box).  Optionally returns residual, which is
	//	distances from point to next-lowest grid lines in x, y, z
	//
	void getVoxelCoords( const MPoint& point, 
		gridPoint3<int>& coords, 
		MPoint *residual = NULL ) const;

	void getClosestVoxelCoords( const MPoint& point, 
		gridPoint3<int>& coords) const;

	bool isValidVoxel(gridPoint3<int>& vox);

	//	returns x, y, z index ranges of voxels at least partially
	//	intersected by given bounding box
	//
	void getVoxelRange( const MBoundingBox& box, 
		gridPoint3<int>& minIndices, 
		gridPoint3<int>& maxIndices ) const;

	//	gets triangle list for given voxel
	//						
	MUintArray *			getVoxelContents( const gridPoint3<int>& index );

	//	accessors useful for debugging output
	//
	const gridPoint3<int>& 			getNumVoxels();
	virtual float			getMemoryFootprint();

	MBoundingBox getBounds(){ return fBounds;}
	//	iterator gets internal access
	//
	friend class SpatialGridWalker;	

private:
	//	accessors for bounding box, bounding box corners
	//
	const MBoundingBox&	bounds();
	void bounds( MPoint& lowCorner, 
		MPoint& highCorner );

	//	converts from x,y,z index to linear index into 
	//	voxel array
	//	
	int getLinearVoxelIndex( const gridPoint3<int>& index ) const;

	//	bounding box for the entire grid
	//
	MBoundingBox				fBounds;

	//	number of grid cells along each axis
	//
	gridPoint3<int>						fNumVoxels;

	//	dimensions of each voxel in x, y, z
	//
	gridPoint3<float>						fVoxelSizes;

	//	The actual voxel grid contents, one array entry for each voxel.
	//	Each voxel stores a pointer to a index array, pointers are
	//	all NULL originally. The index array is intended to be data blind.
	//  That is, we don't know what the indices they refer to.  
	//
	std::vector<MUintArray*>		fVoxels;

	void expandBBoxByPercentage( 
		MBoundingBox& bbox,
		double percentage,
		double min[3] 
	);
	
};
#endif
