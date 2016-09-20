//-
// ==========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

//  Description
//	The dynExprField node implements a uniform field, which allows 
//      the per particle attributes to drive the field's attributes.
//

#include <maya/MIOStream.h>
#include <maya/MVector.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MFnPlugin.h>
#include <maya/MPxFieldNode.h>
#include <maya/MGL.h>

#define McheckErr(stat, msg)		\
	if ( MS::kSuccess != stat )    	\
	{			      	\
		cerr << msg;		\
		return MS::kFailure;	\
	}

class dynExprField: public MPxFieldNode
{
public:
	dynExprField() {};
	virtual ~dynExprField() {};

	static void    *creator();
	static MStatus	initialize();

	// will compute output force.
	//
	virtual MStatus	compute( const MPlug& plug, MDataBlock& block );

	///
	virtual MStatus iconSizeAndOrigin(	GLuint& width,
						GLuint& height,
						GLuint& xbo,
						GLuint& ybo   );
	///
	virtual MStatus iconBitmap(GLubyte* bitmap);

	//
	// attributes.
	//

	// direction of the force
	//
	static MObject	mDirection;

	// Other data members
	//
        static MTypeId	id;

private:

	// methods to get attribute value.
	//
	double	magnitude(MDataBlock& block);

	MVector direction(MDataBlock& block);

	// Compute the force to apply to each particle
	void apply(MDataBlock         &block,
		   int                 receptorSize,
		   const MDoubleArray &magnitudeArray,
		   const MDoubleArray &magnitudeOwnerArray,
		   const MVectorArray &directionArray,
		   const MVectorArray &directionOwnerArray,
		   MVectorArray       &outputForce);
};

