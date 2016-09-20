// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.

#include "gpuCacheIsectUtil.h"
#include <maya/MMatrix.h>
#include <maya/MPointArray.h>
#include <limits>

namespace GPUCache {

	// used when ray does not intersect object
	// to account for perspective, all edges are flattened onto 
	// a plane defined by the raySource and rayDirection
	double gpuCacheIsectUtil::getClosestPointOnLine(const MPoint& queryPoint, const MPoint& pt1, const MPoint& pt2, MPoint& closestPoint){
		MVector edgeVec = pt2-pt1;
		double t = ( (queryPoint - pt1) * edgeVec ) / (edgeVec * edgeVec);
		if(t<0) t=0;
		if(t>1) t=1;
		closestPoint = (1-t) * pt1 + t * pt2;
		return t;
	}
	

	// used when ray does not intersect object
	// to account for perspective, all edges are flattened onto 
	// a plane defined by the raySource and rayDirection
	double gpuCacheIsectUtil::getEdgeSnapPointOnBox(const MPoint& raySource, const MVector& rayDirection, const MBoundingBox& bbox, MPoint& snapPoint){
		//if ray intersects bbox
		MPoint boxIntersectionPt;
		if(firstRayIntersection(bbox.min(), bbox.max(), raySource, rayDirection, NULL, &boxIntersectionPt))
		{
			//	ray intersects bounding box, so snapPoint is
			//	closest hit on the outside of the box
			//  and distance to box is 0 (for snapping purposes)
			snapPoint = boxIntersectionPt;
			return 0.0;
		}

		MPointArray verts;
		MPoint vmin = bbox.min();
		MPoint vmax = bbox.max();

		verts.insert(vmin,0);
		verts.insert(MPoint(vmax[0],vmin[1],vmin[2]),1);
		verts.insert(MPoint(vmax[0],vmax[1],vmin[2]),2);
		verts.insert(MPoint(vmin[0],vmax[1],vmin[2]),3);
		verts.insert(MPoint(vmin[0],vmin[1],vmax[2]),4);
		verts.insert(MPoint(vmax[0],vmin[1],vmax[2]),5);
		verts.insert(vmax,6);
		verts.insert(MPoint(vmin[0],vmax[1],vmax[2]),7);

		int edgeIndices[12][2]={{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{3,7},{2,6}};

		double minDistRect = std::numeric_limits<double>::max();
		
		for(int edge=0; edge<12;edge++){
			MPoint vertex1Org = verts[edgeIndices[edge][0]];
			MPoint vertex2Org = verts[edgeIndices[edge][1]];
			
			double coef_plane = rayDirection * raySource;
			double d = coef_plane - rayDirection * vertex1Org;
			MPoint vertex1 = vertex1Org + rayDirection * d;
			d = coef_plane - rayDirection * vertex2Org;
			MPoint vertex2 = vertex2Org + rayDirection * d;

			MVector edgeDir = vertex2 - vertex1;

			if (edgeDir.length()<0.0000001){
				double dist = vertex1.distanceTo(raySource);
				if (dist < minDistRect) {
					minDistRect = dist;
					snapPoint = vertex1Org;
				}
			} else {
				MPoint edgePt;
				// Compute the closest point from the edge to cursor Ray.
				double percent = gpuCacheIsectUtil::getClosestPointOnLine(raySource, vertex1, vertex2, edgePt);
				double dist = edgePt.distanceTo(raySource);
				if (dist < minDistRect) {
					minDistRect = dist;
					snapPoint =  (vertex1Org + percent * (vertex2Org - vertex1Org));
				}
			}
		}
		
		return minDistRect;
	}
	// used when ray does not intersect object
	// to account for perspective, all edges are flattened onto 
	// a plane defined by the raySource and rayDirection
	double gpuCacheIsectUtil::getEdgeSnapPointOnTriangle(const MPoint& raySource, const MVector& rayDirection, const MPoint& vert1, const MPoint& vert2, const MPoint& vert3, MPoint& snapPoint){
		MPointArray verts;
		verts.insert(vert1,0);
		verts.insert(vert2,1);
		verts.insert(vert3,2);

		int edgeIndices[3][2]={{0,1},{1,2},{2,0}};

		double minDistTri = std::numeric_limits<double>::max();

		for(int edge=0; edge<3;edge++){
			MPoint vertex1Org = verts[edgeIndices[edge][0]];
			MPoint vertex2Org = verts[edgeIndices[edge][1]];

			double coef_plane = rayDirection * raySource;
			double d = coef_plane - rayDirection * vertex1Org;
			MPoint vertex1 = vertex1Org + rayDirection * d;
			d = coef_plane - rayDirection * vertex2Org;
			MPoint vertex2 = vertex2Org + rayDirection * d;

			MVector edgeDir = vertex2 - vertex1;
			
			if (edgeDir.length()<0.0000001){
				double dist = vertex1.distanceTo(raySource);
				if (dist < minDistTri) {
					minDistTri = dist;
					snapPoint = vertex1Org;
				}
			} else {
				MPoint edgePt;
				// Compute the closest point from the edge to cursor Ray.
				double percent = gpuCacheIsectUtil::getClosestPointOnLine(raySource, vertex1, vertex2, edgePt);
				double dist = edgePt.distanceTo(raySource);                                                            
				if (dist < minDistTri) {
					minDistTri = dist;
					snapPoint =  (vertex1Org + percent * (vertex2Org - vertex1Org));
				}
			}
		}

		return minDistTri;
	}

	void gpuCacheIsectUtil::getClosestPointToRayOnLine(const MPoint& vertex1, const MPoint& vertex2, const MPoint& raySource, const MVector& rayDirection, MPoint& closestPoint, double& percent){
		MPoint clsPoint;
		MVector edgeDir = vertex2 - vertex1;
		double len = edgeDir.length();

		if(len < 0.0000001 )
		{
			percent = 0.0;
			closestPoint = vertex1;
			return;
		}

		edgeDir.normalize();
		
		//if line is parallel to ray
		double dotPrd = fabs(edgeDir * rayDirection);
		if(dotPrd > 0.9999){
			percent = 0.0;
			closestPoint = vertex1;
			return;
		}

		// Vector connecting two closest points.
		//
		MVector crossProd = edgeDir ^ rayDirection;

		// Normal to the plane defined by that vector and the 'otherLine'.
		//
		MVector planeNormal = rayDirection ^ crossProd;
		//intersectionPlane is raySource,planeNormal
		double t;
		if(intersectPlane(raySource, planeNormal, vertex1,edgeDir,t)){
			clsPoint = vertex1 + t * edgeDir;

			// Find percent, where
			// vertex1 + percent * (edgeDir) == closestPoint
			//
			percent = edgeDir * (clsPoint - vertex1) / len;

			// The closest point may not be on the segment. Find the closest
			// point on the segment using t.
			//
			if (percent < 0)
			{
				closestPoint = vertex1;
				percent = 0.0;
			}
			else if (percent > 1.0)
			{
				closestPoint = vertex2;
				percent = 1.0;
			}
			else
			{
				closestPoint = clsPoint;
			}
		} else
		{
			closestPoint = vertex1;
			percent = 0.0;
		}
	}

	double gpuCacheIsectUtil::getClosestPointOnBox(const MPoint& point, const MBoundingBox& bbox, MPoint& closestPoint){
		MVector diff = point - bbox.center();

		MVector axis[3] = { MVector(1.0,0.0,0.0),
			MVector(0.0,1.0,0.0),
			MVector(0.0,0.0,1.0)};

		double dimensions[3] = {0.5*bbox.width(),0.5*bbox.height(),0.5*bbox.depth()};

		double sqrDistance = 0.0;
		double delta;
		MPoint closest;

		for (int i = 0; i < 3; ++i)
		{
			closest[i] = diff * axis[i];
			if (closest[i] < -dimensions[i])
			{
				delta = closest[i] + dimensions[i];
				sqrDistance += delta*delta;
				closest[i] = -dimensions[i];
			}
			else if (closest[i] > dimensions[i])
			{
				delta = closest[i] - dimensions[i];  
				sqrDistance += delta*delta;
				closest[i] = dimensions[i];
			}
		}

		closestPoint = bbox.center();
		for (int i = 0; i < 3; ++i)
		{
			closestPoint += closest[i]*axis[i];
		}

		return sqrt(sqrDistance);
	}


	int gpuCacheIsectUtil::intersectRayWithBox(
		MPoint minPoint,
		MPoint maxPoint,
		const MPoint&	rayOrigin,
		const MVector&	rayDirection,
		double			isectParams[2]
	)
		//
		//	Description:
		//
		//		Utility function that finds parametric values of all
		//		intersections of a ray with the bounding box whose
		//		lower and upper bounds along each axis are defined
		//		by xBound, yBound, zBound.
		//
		//		Returns number of hits found (should always be <= 2),
		//		and returns parametric values of hits (sorted by increasing
		//		distance from the ray) through isectParams array.
		//		These parameters are the "t" values, if the ray is 
		//		expressed parametrically as P(t) = rayOrigin + t*rayDirection.
		//
	{
		//	small tolerance necessary when ray passes almost exactly through
		//	corners of the bounding box.
		//
		const double isectTol = 1.0e-6;

		//	how many hits have we found so far
		//
		int numFound = 0;

		//	put bounds in an array to let us index them by axis
		//
		double boundsMin[3]={minPoint[0],minPoint[1],minPoint[2]};
		double boundsMax[3]={maxPoint[0],maxPoint[1],maxPoint[2]};

		//	for each side of the voxel grid (+X, -X, +Y, -Y, +Z, -Z), we will
		//	intersect the ray with that side's plane, then check the intersection
		//	point to see if it lies within 
		//
		for( int axis = 0; axis < 3; axis++ )
		{
			//	ray can't intersect faces that it is parallel to 
			//
			if( rayDirection[axis] != 0 )
			{
				//	We are intersecting the ray with faces perp. to one axis 
				//	(X, Y, or Z).  Figure out what the other two axes are, as we're 
				//	going to have to test whether the intersect points are "inside"
				//	those faces.
				//
				int otherAxis1 = (axis+1)%3;
				int otherAxis2 = (axis+2)%3;

				//	figure out high and low "axis" coordinate values for the faces
				//	(x values of the -X and +X faces, for example)
				//			
				double sides[2] = { boundsMin[axis], boundsMax[axis] };

				//	find the ray intersection with the high and low faces for this
				//	axis, and determine if the hit points lie within the bounds
				//	for the other two axes.  For example, if the ray hits the plane
				//	defined by the +X face, does the hit point lie within the Y and Z
				//	ranges of the box.  If so, then the ray intersects the box at
				//	that point.
				//
				//	"side" is just 0 or 1: 0 for the low face (-X, for example),
				//	1 for the high face (1, for example).
				//
				for( int side = 0; side < 2; side++ )
				{
					//	find parametric distance to this face
					//	
					double tSide = (sides[side]-rayOrigin[axis])/rayDirection[axis];
					if( tSide > 0.0 )
					{
						//	find first other coordinate value of hit point (hit x
						//	axis, we figure out the y value, for example)
						//
						double newPointOtherAxis1 = rayOrigin[otherAxis1] + 
							tSide*rayDirection[otherAxis1];

						//	see if the bounding box for the first other axis 
						//	contains the hit point.  If not, the ray can't intersect
						//	this face of the bounding box
						//

						if( (newPointOtherAxis1 >= (boundsMin[otherAxis1]-isectTol)) && (newPointOtherAxis1 <= (boundsMax[otherAxis1]+isectTol)) )
						{
							//	now, test the hit point for the second other coordinate
							//	value, to see if it is inside the box bounds
							//
							double newPointOtherAxis2 = rayOrigin[otherAxis2] + 
								tSide*rayDirection[otherAxis2];


							if( (newPointOtherAxis2 >= (boundsMin[otherAxis2]-isectTol)) && (newPointOtherAxis2 <= (boundsMax[otherAxis2]+isectTol)) )
							{
								//	Point is on one face, inside the box bounds for
								//	the other two axes, so it's a hit.  
								//	Now, insert its parametric value into the hit
								//	param array, maintaining the array in
								//	sorted order of ascending t.  Note that since
								//	we are intersecting a ray against a convex 
								//	object, we should never have more than 2 
								//	intersections, so we'll just assume that the
								//	array currently has size 0 or 1, which makes
								//	the sorting trivial. 
								//
								//	The only time that this may not be the case
								//	is if a ray goes exactly through an edge or
								//	a corner.  In that case, all the intersections
								//	correspond to the same point, so we should only
								//	report it once.  We will achieve this by 
								//	discarding intersection param values that are
								//	"equivalent" (equal with a small numerical 
								//	tolerance). 
								//
								if( numFound == 0 )
								{
									isectParams[0] = tSide;
									numFound++;
								}
								else if( numFound == 1 )
								{
									//	add the hit param in the appropriate
									//	position in the array
									//
									if( tSide >= (isectParams[0]+1.0e-10) )
									{
										isectParams[1] = tSide;
										numFound++;
									}
									else if( tSide <= (isectParams[0]-1.0e-10) )
									{
										isectParams[1] = isectParams[0];
										isectParams[0] = tSide;
										numFound++;
									}
								}
							}
						}
					}
				}
			}
		}

		return numFound;
	}

	bool gpuCacheIsectUtil::firstRayIntersection ( 
		MPoint bboxMin,
		MPoint bboxMax,
		const MPoint&		rayOrigin,
		const MVector&		rayDirection,
		double*			isectParam,
		MPoint *		isectPoint
		) 
		//
		//	Description:
		//
		//		Finds first hit of ray against the outside of the bounding box.
		//		Returns true if the ray hits the box, false otherwise.
		//		"isectParam", if non-NULL, receives parametric distance along ray
		//		to the intersection point (ie the "t" value if the ray is expressed
		//		parametrically as P(t) = rayOrigin + t * rayDirection).
		//		"isectPoint", if non-NULL, receives the intersection point coordinates.
		//
	{
		//	get all the hits with the bounding box
		//
		double allIsectParams[4];

		if( !intersectRayWithBox( bboxMin, bboxMax, rayOrigin, rayDirection, allIsectParams ) )
		{
			return false;
		}
		else
		{
			//	Found hits, so closest one is first in array.  Return
			//	parametric value and point coordinates, if required.
			//
			if( isectParam != NULL )
			{
				*isectParam = allIsectParams[0];
			}

			if( isectPoint != NULL )
			{
				*isectPoint = rayOrigin + allIsectParams[0]*rayDirection;
			}

			return true;
		}
	}

	bool gpuCacheIsectUtil::intersectPlane(const MPoint &planePoint, const MVector &planeNormal, const MPoint& rayPoint, const MVector &rayDirection, double &t)
	{
		// assuming vectors are all normalized
		double denom = planeNormal * rayDirection;
		if (denom > 0.0000001) {
			MVector p0l0 = planePoint - rayPoint;
			t = (p0l0 * planeNormal) / denom; 
			return (t >= 0);
		}
		return false;
	}

	bool gpuCacheIsectUtil::getClosestPointOnTri(const MPoint &toThisPoint, const MPoint &pt1, const MPoint &pt2, const MPoint &pt3, MPoint &theClosestPoint, double &currDist) 
	{
		double		sum, a, b, c, len, dist;
		MMatrix mat;
		mat.setToIdentity();
		mat[2][0] = mat[2][1] = mat[2][2] = 1.;

		MVector v = toThisPoint - pt1;
		MVector v12 = pt2 - pt1;
		MVector v13 = pt3 - pt1;
		MVector norm = v12 ^ v13;
		len = norm * norm;
		if (len < 1.175494351e-38F) return false;
		len = ( norm * v ) / len;

		MPoint pnt = toThisPoint - len * norm;

		// Do a quick test first
		if (pnt.distanceTo(toThisPoint) >= currDist)
			return false;

		int i, j;				// Find best plane to project to
		if (fabs(norm[0]) > fabs(norm[1]))
		{
			if (fabs(norm[0]) > fabs(norm[2]))
			{
				i = 1; j = 2;
			}
			else
			{
				i = 0; j = 1;
			}
		}
		else
		{
			if (fabs(norm[1]) > fabs(norm[2]))
			{
				i = 0; j = 2;
				// i = 2; j = 0;
			}
			else
			{
				i = 0; j = 1;
			}
		}

		mat[0][0] = pt1[i]; mat[0][1] = pt2[i]; mat[0][2] = pt3[i]; 
		mat[1][0] = pt1[j]; mat[1][1] = pt2[j]; mat[1][2] = pt3[j]; 

		MMatrix	matInv = mat.inverse();
		MPoint abc(pnt[i], pnt[j], 1, 0);

		abc = matInv * abc;
		// Now abc is the barycentric coordinates of pnt
		// clip to inside triangle

		if (abc[0]<0) { // a < 0
			if (abc[1]<0) { // b < 0
				a = b = 0;
				c = 1;
			} else if (abc[2]<0) { // c < 0
				a = c = 0;
				b = 1;
			} else {
				a = 0;
				// c = BP dot BC / BC square;
				MVector v23 = pt3 - pt2; // BC
				MVector vp =  toThisPoint - pt2;  // BP

				c = ( vp * v23 ) / ( v23[0]*v23[0] + v23[1]*v23[1] + v23[2]*v23[2] );
				if (c<0) c = 0; else if (c>1) c = 1;
				b = 1 - c;
			}
		} else if (abc[1]<0) { // b < 0
			if (abc[2]<0) { // c < 0
				b = c = 0;
				a = 1;
				//} else if (abc[0]<0) { // a < 0
				//	b = a = 0;	// commented-code for optimization
				//	c = 1;		// leaving it in for readability (cyclic variations)
			} else {
				b = 0;
				// a = CP dot CA / CA square;
				MVector v31 = pt1 - pt3; // CA
				MVector vp =  toThisPoint - pt3;  // CP

				a = ( vp * v31 ) / ( v31[0]*v31[0] + v31[1]*v31[1] +v31[2]*v31[2] );
				if (a<0) a = 0; else if (a>1) a = 1;
				c = 1 - a;
			} 
		} else if (abc[2]<0) { // c < 0
			//if (abc[1]<0) { // b < 0
			//	c = b = 0;
			//	a = 1;
			//} else if (abc[0]<0) { // a < 0
			//	c = a = 0;
			//	b = 1;	// commented-code for optimization
			//} else {	// leaving it in for readability (cyclic variations)
			c = 0;
			// b = AP dot AB / AB square;
			//DIFF(v23, pt3, pt2); // AB
			MVector vp =  toThisPoint - pt1;  // AP

			b = ( vp * v12 ) / ( v12[0]*v12[0] + v12[1]*v12[1] + v12[2]*v12[2] );
			if (b<0) b = 0; else if (b>1) b = 1;
			a = 1 - b;
			//}
		} else {
			if (abc[0]>0) a = abc[0]; else a = 0;
			if (abc[1]>0) b = abc[1]; else b = 0;
			if (abc[2]>0) c = abc[2]; else c = 0;
		}
		sum = a+b+c;
		a /= sum ; b /= sum ; c /= sum ; 
		pnt = a * pt1 + b * pt2 + c * pt3;
		dist = pnt.distanceTo(toThisPoint);
		if ( dist < currDist)
		{			
			// Now it's really closer, keep it
			currDist = dist;
			theClosestPoint = pnt;
			return true;
		}
		return false;
	}
}
