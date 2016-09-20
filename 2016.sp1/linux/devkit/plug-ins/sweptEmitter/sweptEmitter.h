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
#include <maya/MPxEmitterNode.h>


class MIntArray;
class MVectorArray;
class MFnArrayAttrsData;

#define McheckErr(stat, msg)		\
	if ( MS::kSuccess != stat )		\
	{								\
		cerr << msg;				\
		return MS::kFailure;		\
	}

class sweptEmitter: public MPxEmitterNode
{
public:
	sweptEmitter();
	virtual ~sweptEmitter();

	static void		*creator();
	static MStatus	initialize();
	virtual MStatus	compute( const MPlug& plug, MDataBlock& block );
	static MTypeId	id;


private:

	void	emit( const MVectorArray &inPosAry, const MVectorArray &inVelAry,
					const MIntArray &countAry, double dt, double speed,
					double inheritFactor, MVector dirV,
					MVectorArray &outPos, MVectorArray &outVel );

	MVector	useRotation ( MVector &direction );

	MStatus	emitCountPerPoint( const MPlug& plug, MDataBlock& block,
								int length, MIntArray &countAry );

	// methods to get attribute value.
	//
	double	rateValue( MDataBlock& block );
	double	speedValue( MDataBlock& block );
	MVector	directionVector( MDataBlock& block );

	bool	isFullValue( int plugIndex, MDataBlock& block );
	double	inheritFactorValue( int plugIndex, MDataBlock& block );

	MTime	currentTimeValue( MDataBlock& block );
	MTime	startTimeValue( int plugIndex, MDataBlock& block );
	MTime	deltaTimeValue( int plugIndex, MDataBlock& block );

	// the previous position in the world space.
	//
	MPoint	lastWorldPoint;
};

// inlines
//
inline double sweptEmitter::rateValue( MDataBlock& block )
{
	MStatus status;

	MDataHandle hValue = block.inputValue( mRate, &status );

	double value = 0.0;
	if( status == MS::kSuccess )
		value = hValue.asDouble();

	return( value );
}

inline double sweptEmitter::speedValue( MDataBlock& block )
{
	MStatus status;

	MDataHandle hValue = block.inputValue( mSpeed, &status );

	double value = 0.0;
	if( status == MS::kSuccess )
		value = hValue.asDouble();

	return( value );
}

inline MVector sweptEmitter::directionVector( MDataBlock& block )
{
	MStatus status;
	MVector dirV(0.0, 0.0, 0.0);

	MDataHandle hValue = block.inputValue( mDirection, &status );

	if( status == MS::kSuccess )
	{
		double3 &value = hValue.asDouble3();

		dirV[0] = value[0];
		dirV[1] = value[1];
		dirV[2] = value[2];
	}

	return( dirV );
}

inline bool sweptEmitter::isFullValue( int plugIndex, MDataBlock& block )
{
	MStatus status;
	bool value = true;

	MArrayDataHandle mhValue = block.inputArrayValue( mIsFull, &status );
	if( status == MS::kSuccess )
	{
		status = mhValue.jumpToElement( plugIndex );
		if( status == MS::kSuccess )
		{
			MDataHandle hValue = mhValue.inputValue( &status );
			if( status == MS::kSuccess )
				value = hValue.asBool();
		}
	}

	return( value );
}

inline double sweptEmitter::inheritFactorValue(int plugIndex,MDataBlock& block)
{
	MStatus status;
	double value = 0.0;

	MArrayDataHandle mhValue = block.inputArrayValue( mInheritFactor, &status );
	if( status == MS::kSuccess )
	{
		status = mhValue.jumpToElement( plugIndex );
		if( status == MS::kSuccess )
		{
			MDataHandle hValue = mhValue.inputValue( &status );
			if( status == MS::kSuccess )
				value = hValue.asDouble();
		}
	}

	return( value );
}

inline MTime sweptEmitter::currentTimeValue( MDataBlock& block )
{
	MStatus status;

	MDataHandle hValue = block.inputValue( mCurrentTime, &status );

	MTime value(0.0);
	if( status == MS::kSuccess )
		value = hValue.asTime();

	return( value );
}

inline MTime sweptEmitter::startTimeValue( int plugIndex, MDataBlock& block )
{
	MStatus status;
	MTime value(0.0);

	MArrayDataHandle mhValue = block.inputArrayValue( mStartTime, &status );
	if( status == MS::kSuccess )
	{
		status = mhValue.jumpToElement( plugIndex );
		if( status == MS::kSuccess )
		{
			MDataHandle hValue = mhValue.inputValue( &status );
			if( status == MS::kSuccess )
				value = hValue.asTime();
		}
	}

	return( value );
}

inline MTime sweptEmitter::deltaTimeValue( int plugIndex, MDataBlock& block )
{
	MStatus status;
	MTime value(0.0);

	MArrayDataHandle mhValue = block.inputArrayValue( mDeltaTime, &status );
	if( status == MS::kSuccess )
	{
		status = mhValue.jumpToElement( plugIndex );
		if( status == MS::kSuccess )
		{
			MDataHandle hValue = mhValue.inputValue( &status );
			if( status == MS::kSuccess )
				value = hValue.asTime();
		}
	}

	return( value );
}

