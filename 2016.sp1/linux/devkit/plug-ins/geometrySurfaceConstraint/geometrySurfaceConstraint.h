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

#include <string.h>
#include <maya/MIOStream.h>
#include <math.h>

#include <maya/MPxTransform.h>
#include <maya/MPxConstraint.h> 
#include <maya/MPxConstraintCommand.h> 

#include <maya/MArgDatabase.h>

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnPlugin.h>

#include <maya/MGlobal.h>

#include <maya/MString.h> 
#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MVector.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnTransform.h>
#include <maya/MVector.h>
#include <maya/MTypes.h>
#include <maya/MFnNumericData.h>
#include <maya/MDGModifier.h>

#include <math.h>
#include <float.h>

//
//	Macros
//


#define kConstrainToLargestWeightFlag "-lw"
#define kConstrainToLargestWeightFlagLong "-largestWeight"
#define kConstrainToSmallestWeightFlag "-sw"
#define kConstrainToSmallestWeightFlagLong "-smallestWeight"

//
// Class definitions
//

class geometrySurfaceConstraintCommand : public MPxConstraintCommand
{
public:

	enum ConstraintType
	{
		kLargestWeight,
		kSmallestWeight,
	};

	geometrySurfaceConstraintCommand();
	~geometrySurfaceConstraintCommand();

	virtual MStatus		doIt(const MArgList &argList);
	virtual MStatus		appendSyntax();

	virtual MTypeId constraintTypeId() const;
	virtual MPxConstraintCommand::TargetType targetType() const;

	virtual const MObject& constraintInstancedAttribute() const;
	virtual const MObject& constraintOutputAttribute() const;
	virtual const MObject& constraintTargetInstancedAttribute() const;
	virtual const MObject& constraintTargetAttribute() const;
	virtual const MObject& constraintTargetWeightAttribute() const;
	virtual const MObject& objectAttribute() const;

	virtual MStatus connectTarget(void *opaqueTarget, int index);
	virtual MStatus connectObjectAndConstraint( MDGModifier& modifier );

	virtual void createdConstraint(MPxConstraint *constraint);

	static  void* creator();

protected:
		virtual MStatus			parseArgs(const MArgList &argList);

	geometrySurfaceConstraintCommand::ConstraintType weightType;
};

class geometrySurfaceConstraint : public MPxConstraint
{
public:
						geometrySurfaceConstraint();
	virtual				~geometrySurfaceConstraint(); 

	virtual void		postConstructor();
	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );

	virtual const MObject weightAttribute() const;
    virtual const MObject targetAttribute() const;
	virtual void getOutputAttributes(MObjectArray& attributeArray);

	static  void*		creator();
	static  MStatus		initialize();

public:
	static MObject		compoundTarget;
	static MObject		targetGeometry;
	static MObject		targetWeight;

	static MObject		constraintParentInverseMatrix;
	static MObject		constraintGeometry;

	static	MTypeId		id;

	geometrySurfaceConstraintCommand::ConstraintType weightType;
};

// Useful inline method
inline bool
equivalent(double a, double b  )
{
	return fabs( a - b ) < .000001 ;
}

