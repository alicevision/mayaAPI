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
#include <maya/MPxFluidEmitterNode.h>


class MIntArray;
class MVectorArray;
class MFnArrayAttrsData;

#define McheckErr(stat, msg)		\
	if ( MS::kSuccess != stat )		\
	{								\
		cerr << msg;				\
		return MS::kFailure;		\
	}

class simpleFluidEmitter: public MPxFluidEmitterNode
{
public:
	simpleFluidEmitter();
	virtual ~simpleFluidEmitter();

	static void		*creator();
	static MStatus	initialize();
	virtual MStatus	compute( const MPlug& plug, MDataBlock& block );
	static MTypeId	id;

	virtual MStatus fluidEmitter( const MObject& fluid, const MMatrix& worldMatrix, int plugIndex );
	void  omniFluidEmitter(  MFnFluid&, const MMatrix&, int, MDataBlock&, double, double, double );
	void volumeFluidEmitter(  MFnFluid&, const MMatrix&, int, MDataBlock&, double, double, double );
	void surfaceFluidEmitter( MFnFluid&, const MMatrix&, int, MDataBlock&, double, double, double );
	void ownerFluidEmitter( MFnFluid&, const MMatrix&, int, MDataBlock&, double, double, double );

private:

};



