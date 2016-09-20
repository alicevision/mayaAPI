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

#include <maya/MPoint.h>
#include <maya/MVector.h>

#include "manipulatorMath.h"

//
// Utility classes + functions
//

//
//	planeMath
//

planeMath::planeMath()
{
	a = b = c = d = 0.0;
}

bool planeMath::setPlane( const MPoint& pointOnPlane, const MVector& normalToPlane )
{
	MVector _normalToPlane = normalToPlane;
	_normalToPlane.normalize();

	// Calculate a,b,c,d based on input
	a = _normalToPlane.x;
	b = _normalToPlane.y;
	c = _normalToPlane.z;
	d = -(a*pointOnPlane.x + b*pointOnPlane.y + c*pointOnPlane.z);
	return true;
}

bool planeMath::intersect( const MPoint& linePoint, const MVector& lineDirection, MPoint& intersectionPoint )
{
	double denominator = a*lineDirection.x + b*lineDirection.y + c*lineDirection.z;

	// Verify that the vector and the plane are not parallel.
	if(denominator < .00001)
	{
		return false;
	}

	double t = -(d + a*linePoint.x + b*linePoint.y + c*linePoint.z) / denominator;

	// Calculate the intersection point.
	intersectionPoint = linePoint + t * lineDirection;

	return true;
}

//
//	lineMath
//

lineMath::lineMath()
{
	// No-op
}

bool lineMath::setLine( const MPoint& linePoint, const MVector& lineDirection )
{
	point = linePoint;
	direction = lineDirection;
	direction.normalize();
	return true;
}

bool lineMath::closestPoint( const MPoint& toPoint, MPoint& closest )
{
	double t = direction * ( toPoint - point );
	closest = point + ( direction * t );
	return true;
}

//
// Utility class
//

double DegreeRadianConverter::degreesToRadians( double degrees )
{
	 return degrees * ( M_PI/ 180.0 );
}

double DegreeRadianConverter::radiansToDegrees( double radians )
{
	return radians * (180.0/M_PI);
}	

