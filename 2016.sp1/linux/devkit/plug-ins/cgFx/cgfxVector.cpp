//
// Copyright (C) 2002 NVIDIA 
// 
// File: cgfxVector.cpp
//
// Dependency Graph Node: cgfxVector
//
// Description:
//	The cgfxVector node is used to convert a vector in the scene to
//	world coordinates.  The inputs are a vector in local coordinates,
//	a flag indicating whether the vector is a position or a direction,
//	and a matrix that will transoform the vector to world coordinates.
//	This matrix is generally the worldInverseMatrix of the vector.
//
// Author: Jim Atkinson
//
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
#include "cgfxShaderCommon.h"

#include "cgfxVector.h"

#include <maya/MMatrix.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MPlug.h>

// The typeid is a unique 32bit indentifier that describes this node.
// It is used to save and retrieve nodes of this type from the binary
// file format.  If it is not unique, it will cause file IO problems.
//
#ifdef _WIN32
MTypeId cgfxVector::sId( 4084862001 );
#else
MTypeId cgfxVector::sId( 0xF37A0C31 );
#endif

// There needs to be a MObject handle declared for each attribute that
// the node will have.  These handles are needed for getting and setting
// the values later.
//
// Input vector attribute
//
MObject	cgfxVector::sVector;

MObject cgfxVector::sVectorX;
MObject cgfxVector::sVectorY;
MObject cgfxVector::sVectorZ;

// Input position/direction flag.  If isDirection is set then
// the vector represents a direction and the W coordinate is
// 0.0.  If it is not set then W is 1.0.
//
MObject	cgfxVector::sIsDirection;

// Input matrix attribute
//
MObject cgfxVector::sMatrix;

// Output world coordinate vector attribute
//
MObject	cgfxVector::sWorldVector;
MObject	cgfxVector::sWorldVectorX;
MObject	cgfxVector::sWorldVectorY;
MObject	cgfxVector::sWorldVectorZ;
MObject	cgfxVector::sWorldVectorW;

cgfxVector::cgfxVector()
{
	// Nothing to construct
}

/* virtual */
cgfxVector::~cgfxVector()
{
	// Nothing to destruct
}

#ifdef CGFX_DEBUG
#include <stdarg.h>

#ifdef _WIN32
// Disable in Linux build, because
// our Linux compiler setting makes it to complain for unused vars and methods
//
/*
static void dprintf(char* format, ...)
{
	char buffer[1024];

	va_list args;
	va_start(args, format);

	vsprintf(buffer, format, args);
	OutputDebugString(buffer);

	va_end(args);
}
*/
#endif


#else
static inline void dprintf(char* format, ...)
{
	// nothing
}
#endif
/* virtual */
MStatus cgfxVector::compute( const MPlug& plug, MDataBlock& data )
{
	MStatus status;

	if( plug == sWorldVector ||
		plug == sWorldVectorX ||
		plug == sWorldVectorY ||
		plug == sWorldVectorZ ||
		plug == sWorldVectorW)
	{
		// We do isDirection first simply because if there is an
		// error, the isDirection error is more legible than the
		// vector or matrix error.
		//
		MDataHandle dhIsDirection = data.inputValue(sIsDirection, &status);
		if (!status)
		{
			status.perror("cgfxVector: isDirection handle");
			return status;
		}

		MDataHandle dhVector = data.inputValue(sVector, &status);
		if (!status)
		{
			status.perror("cgfxVector: vector handle");
			return status;
		}

		MMatrix matrix;

		MPlug matrixPlug(thisMObject(), sMatrix);
		if (matrixPlug.isNull())
		{
			OutputDebugString("matrixPlug is NULL!\n");
		}

		// TODO: Fix this kludge.
		//
		// We should not have to do this but for some reason, 
		// using data.inputValue() fails for the sMatrix attribute.
		// Instead, we get a plug to the attribute and then get
		// the value directly.
		//
		MObject oMatrix;

		matrixPlug.getValue(oMatrix);

		MFnMatrixData fndMatrix(oMatrix, &status);
		if (!status)
		{
			status.perror("cgfxVector: matrix data");
		}

		matrix= fndMatrix.matrix(&status);
		if (!status)
		{
			status.perror("cgfxVector: get matrix");
		}

#if 0
		// TODO: This is how we are supposed to do it.  (I think).
		//
		MDataHandle dhMatrix = data.inputValue(sMatrix, &status);
		if (!status)
		{
			status.perror("cgfxVector: matrix handle");
		}

		oMatrix			= dhMatrix.data();
		MFnMatrixData fnMatrix(oMatrix, &status);
		if (!status)
		{
			status.perror("cgfxVector: matrix function set");
		}

		matrix = fnMatrix.matrix();
#endif /* 0 */

		bool	 isDirection	= dhIsDirection.asBool();
		double3& vector			= dhVector.asDouble3();

		double mat[4][4];
		matrix.get(mat);

		double ix, iy, iz, iw;	// Input vector
		float  ox, oy, oz, ow;	// Output vector

		ix = vector[0];
		iy = vector[1];
		iz = vector[2];
		iw = isDirection ? 0.0 : 1.0;

		ox = (float)(mat[0][0] * ix +
					 mat[1][0] * iy +
					 mat[2][0] * iz +
					 mat[3][0] * iw);

		oy = (float)(mat[0][1] * ix +
					 mat[1][1] * iy +
					 mat[2][1] * iz +
					 mat[3][1] * iw);

		oz = (float)(mat[0][2] * ix +
					 mat[1][2] * iy +
					 mat[2][2] * iz +
					 mat[3][2] * iw);

		ow = (float)(mat[0][3] * ix +
					 mat[1][3] * iy +
					 mat[2][3] * iz +
					 mat[3][3] * iw);

		MDataHandle dhWVector = data.outputValue(sWorldVector, &status);
		if (!status)
		{
			status.perror("cgfxVector: worldVector handle");
			return status;
		}

		MDataHandle dhWVectorW = data.outputValue(sWorldVectorW, &status);
		if (!status)
		{
			status.perror("cgfxVector: worldVectorW handle");
			return status;
		}

		dhWVector.set(ox, oy, oz);
		dhWVectorW.set(ow);
		data.setClean(sWorldVector);
		data.setClean(sWorldVectorW);
	}
	else
	{
		return MS::kUnknownParameter;
	}

	return MS::kSuccess;
}

/* static */
void* cgfxVector::creator()
{
	return new cgfxVector;
}

/* static */
MStatus cgfxVector::initialize()
{
	MStatus status;

	MFnNumericAttribute nAttr;
	MFnMatrixAttribute  mAttr;

	sVectorX = nAttr.create("vectorX", "vx",
							MFnNumericData::kDouble, 0.0, &status);
	if (!status)
	{
		status.perror("cgfxVector: create vectorX");
		return status;
	}
	nAttr.setKeyable(true);

	sVectorY = nAttr.create("vectorY", "vy",
							MFnNumericData::kDouble, 0.0, &status);
	if (!status)
	{
		status.perror("cgfxVector: create vectorY");
		return status;
	}
	nAttr.setKeyable(true);

	sVectorZ = nAttr.create("vectorZ", "vz",
							MFnNumericData::kDouble, 0.0, &status);
	if (!status)
	{
		status.perror("cgfxVector: create vectorZ");
		return status;
	}
	nAttr.setKeyable(true);

	sVector = nAttr.create("vector", "v",
						   sVectorX, sVectorY, sVectorZ, &status);
	if (!status)
	{
		status.perror("cgfxVector: create vector");
		return status;
	}
	nAttr.setKeyable(true);

	sIsDirection = nAttr.create("isDirection", "id",
								MFnNumericData::kBoolean, 0.0, &status);
	if (!status)
	{
		status.perror("cgfxVector: create isDirection");
		return status;
	}
	nAttr.setKeyable(true);
	nAttr.setDefault(false);

	sMatrix = mAttr.create("matrix", "m",
						   MFnMatrixAttribute::kDouble, &status);
	if (!status)
	{
		status.perror("cgfxVector: create matrix");
		return status;
	}
	mAttr.setWritable(true);
	mAttr.setStorable(true);

	sWorldVectorX = nAttr.create("worldVectorX", "wvx",
								 MFnNumericData::kFloat, 0.0, &status);
	if (!status)
	{
		status.perror("cgfxVector: create worldVectorX");
		return status;
	}
	nAttr.setWritable(false);
	nAttr.setStorable(false);

	sWorldVectorY = nAttr.create("worldVectorY", "wvy",
								 MFnNumericData::kFloat, 0.0, &status);
	if (!status)
	{
		status.perror("cgfxVector: create worldVectorY");
		return status;
	}
	nAttr.setWritable(false);
	nAttr.setStorable(false);

	sWorldVectorZ = nAttr.create("worldVectorZ", "wvz",
								 MFnNumericData::kFloat, 0.0, &status);
	if (!status)
	{
		status.perror("cgfxVector: create worldVectorZ");
		return status;
	}
	nAttr.setWritable(false);
	nAttr.setStorable(false);

	sWorldVectorW = nAttr.create("worldVectorW", "wvw",
								 MFnNumericData::kFloat, 0.0, &status);
	if (!status)
	{
		status.perror("cgfxVector: create worldVectorW");
		return status;
	}
	nAttr.setWritable(false);
	nAttr.setStorable(false);

	sWorldVector = nAttr.create("worldVector", "wv",
								sWorldVectorX, sWorldVectorY, sWorldVectorZ,
								&status);
	if( !status )
	{
		status.perror("cgfxVector: create worldVector");
		return status;
	}
	nAttr.setWritable(false);
	nAttr.setStorable(false);

	status = addAttribute(sVector);
	if (!status)
	{
		status.perror("cgfxVector: addAttribute vector");
		return status;
	}

	status = addAttribute(sIsDirection);
	if (!status)
	{
		status.perror("cgfxVector: addAttribute isDirection");
		return status;
	}

	status = addAttribute(sMatrix);
	if (!status)
	{
		status.perror("cgfxVector: addAttribute matrix");
		return status;
	}

	status = addAttribute(sWorldVector);
	if (!status)
	{
		status.perror("cgfxVector: addAttribute worldVector");
		return status;
	}

	status = addAttribute(sWorldVectorW);
	if (!status)
	{
		status.perror("cgfxVector: addAttribute worldVectorW");
		return status;
	}

	status = attributeAffects(sVector, sWorldVector);
	if (!status)
	{
		status.perror("cgfxVector: attributeAffects vector -> worldVector");
		return status;
	}

	status = attributeAffects(sIsDirection, sWorldVector);
	if (!status)
	{
		status.perror("cgfxVector: attributeAffects isDirection -> worldVector");
		return status;
	}

	status = attributeAffects(sMatrix, sWorldVector);
	if (!status)
	{
		status.perror("cgfxVector: attributeAffects matrix -> worldVector");
		return status;
	}

	status = attributeAffects(sVector, sWorldVectorW);
	if (!status)
	{
		status.perror("cgfxVector: attributeAffects vector -> worldVectorW");
		return status;
	}

	status = attributeAffects(sIsDirection, sWorldVectorW);
	if (!status)
	{
		status.perror("cgfxVector: attributeAffects isDirection -> worldVectorW");
		return status;
	}

	status = attributeAffects(sMatrix, sWorldVectorW);
	if (!status)
	{
		status.perror("cgfxVector: attributeAffects matrix -> worldVectorW");
		return status;
	}

	return MS::kSuccess;
}

