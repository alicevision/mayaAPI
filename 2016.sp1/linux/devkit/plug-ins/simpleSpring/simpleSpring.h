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

#include <maya/MIOStream.h>
#include <maya/MTime.h>
#include <maya/MVector.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MFnPlugin.h>
#include <maya/MPxSpringNode.h>


class MIntArray;
class MVectorArray;

#define McheckErr(stat, msg)		\
	if ( MS::kSuccess != stat )		\
	{								\
		cerr << msg;				\
		return MS::kFailure;		\
	}

class simpleSpring: public MPxSpringNode
{
public:
	simpleSpring();
	virtual ~simpleSpring();

	static void		*creator();
	static MStatus	initialize();

	// will be used to get user defined attributes
	//
	virtual MStatus	compute( const MPlug& plug, MDataBlock& block );

	// Override applySpringLaw.
	//
	virtual MStatus	applySpringLaw( double stiffness,
						double damping, double restLength,
						double endMass1, double endMass2,
						const MVector &endP1, const MVector &endP2,
						const MVector &endV1, const MVector &endV2,
						MVector &forceV1, MVector &forceV2 );


	//=================================================================
	// If you need new attributes, add them here. Below is an example.
	//=================================================================

	// Your numeric attribute
	//
	static MObject	aSpringFactor;

	// Other data members
	//
	static MTypeId	id;

    // INLINEs to get attribute value.
    //
    double  end1WeightValue( MDataBlock& block );
    double  end2WeightValue( MDataBlock& block );

private:

	// methods to get attribute value.
	//
	double	springFactor( MDataBlock& block );

	// A parameter for this simple spring node.
	//
	double	factor;
};

// inlines
//
inline double simpleSpring::springFactor( MDataBlock& block )
{
	MStatus status;

	MDataHandle hValue = block.inputValue( aSpringFactor, &status );

	double value = 0.0;
	if( status == MS::kSuccess )
		value = hValue.asDouble();

	return( value );
}


// INLINE
//
inline double simpleSpring::end1WeightValue( MDataBlock& block )
{
	MStatus status;
	MDataHandle hValue = block.inputValue( mEnd1Weight, &status );

	double value = 0.0;
	if( status == MS::kSuccess )
		value = hValue.asDouble();

	return( value );
}

inline double simpleSpring::end2WeightValue( MDataBlock& block )
{
	MStatus status;
	MDataHandle hValue = block.inputValue( mEnd2Weight, &status );

	double value = 0.0;
	if( status == MS::kSuccess )
		value = hValue.asDouble();

	return( value );
}
