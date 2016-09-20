#ifndef _gpuCacheIsectUtil_h_
#define _gpuCacheIsectUtil_h_
// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.

#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MBoundingBox.h>

namespace GPUCache {
	class  gpuCacheIsectUtil
	{
	public:
		static int intersectRayWithBox(
			MPoint minPoint,
			MPoint maxPoint,
			const MPoint&	rayOrigin,
			const MVector&	rayDirection,
			double			isectParams[2]
		);

		static bool firstRayIntersection( 
			MPoint bboxMin,
			MPoint bboxMax,
			const MPoint&		rayOrigin,
			const MVector&		rayDirection,
			double*			isectParam,
			MPoint *		isectPoint
			);

		static bool intersectPlane(
			const MPoint &planePoint, const MVector &planeNormal, 
			const MPoint& rayPoint, const MVector &rayDirection, double &t);
			
		static bool getClosestPointOnTri(
			const MPoint &toThisPoint, 
			const MPoint &pt1, const MPoint &pt2, const MPoint &pt3, 
			MPoint &theClosestPoint, double &currDist);

		static double getClosestPointOnBox(const MPoint& point, const MBoundingBox& bbox, MPoint& closestPoint);
		static double getEdgeSnapPointOnBox(const MPoint& raySource, const MVector& rayDirection, const MBoundingBox& bbox, MPoint& snapPoint);
		static double getEdgeSnapPointOnTriangle(const MPoint& raySource, const MVector& rayDirection, const MPoint& vert1, const MPoint& vert2, const MPoint& vert3, MPoint& snapPoint);

		static double getClosestPointOnLine(const MPoint& queryPoint, const MPoint& pt1, const MPoint& pt2, MPoint& closestPoint);

		static void getClosestPointToRayOnLine(const MPoint& vert1, const MPoint& vert2, const MPoint& raySource, const MVector& rayDirection, MPoint& closestPoint, double& percent);

	private:

	};

}
#endif
