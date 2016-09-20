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

//
// Utillity classes
//

class planeMath
{
public:
	planeMath();
	bool setPlane( const MPoint& pointOnPlane, const MVector& normalToPlane );
	bool intersect( const MPoint& linePoint, const MVector& lineDirection, MPoint& intersectPoint );
	double a,b,c,d;
};

class lineMath
{
public:
	lineMath();
	bool setLine( const MPoint& linePoint, const MVector& lineDirection );
	bool closestPoint( const MPoint& toPoint, MPoint& closest );
	MPoint point;
	MVector direction;
};

//
// Utility methods
//

inline double minOfThree( double a, double b, double c )
{
	if ( a < b && a < c )
		return a;
	if ( b < a && b < c )
		return b;
	return c;
}

inline double maxOfThree( double a, double b, double c )
{
	if ( a > b && a > c )
		return a;
	if ( b > a && b > c )
		return b;
	return c;
}

//
// Utility class
//

class DegreeRadianConverter
{
	public:
		double degreesToRadians( double degrees );
		double radiansToDegrees( double radians );
};
