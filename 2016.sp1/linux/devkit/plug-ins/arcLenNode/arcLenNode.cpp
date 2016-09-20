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

#include <maya/MPxNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MString.h> 
#include <maya/MTypeId.h> 
#include <maya/MPlug.h> 
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h> 

class arcLen : public MPxNode
{
public:
						arcLen();
	virtual				~arcLen(); 

    virtual MStatus		compute( const MPlug& plug, MDataBlock& data );
	static  void*		creator();
	static  MStatus		initialize();

public:
	static  MObject     inputCurve;  // The input curve.
	static  MObject     output;      // The output value.
	static	MTypeId		id;          // The IFF type id
};

MTypeId     arcLen::id( 0x80001  );
MObject     arcLen::inputCurve;        
MObject     arcLen::output;       

arcLen::arcLen() {}
arcLen::~arcLen() {}

void* arcLen::creator()
{
	return new arcLen();
}

MStatus arcLen::compute( const MPlug& plug, MDataBlock& data )
{
	
	MStatus status;
 
	if( plug == output )
	{
		MDataHandle inputData = data.inputValue( inputCurve, &status );

		if( !status ) {
			status.perror("ERROR getting data");
		} else {
			MObject curve = inputData.asNurbsCurveTransformed();
			MFnNurbsCurve curveFn( curve, &status ); 
			if( !status ) {
				status.perror("ERROR creating curve function set");
			} else {
				double result = curveFn.length();
				MDataHandle outputHandle = data.outputValue( arcLen::output );
				outputHandle.set( result );
				data.setClean(plug);
			}
		}
	} else {
		return MS::kUnknownParameter;
	}

	return MS::kSuccess;
}

MStatus arcLen::initialize()
{
	MFnNumericAttribute numericAttr;
	MFnTypedAttribute   typedAttr;
	MStatus status;

	inputCurve = typedAttr.create( "inputCurve", "in", 
									MFnData::kNurbsCurve, 
									&status );

	if( !status ) {
		status.perror("ERROR creating arcLen curve attribute");
		return status;
	}

	output = numericAttr.create( "output", "out", 
									MFnNumericData::kDouble, 0.0, 
									&status );
	if( !status ) {
		status.perror("ERROR creating arcLen output attribute");
		return status;
	}
	numericAttr.setWritable(false);

	status = addAttribute( inputCurve );
	if( !status ) {
		status.perror("addAttribute(inputCurve)");
		return status;
	}

	status = addAttribute( output );
	if( !status ) {
		status.perror("addAttribute(output)");
		return status;
	}

	status = attributeAffects( inputCurve, output );
	if( !status ) {
		status.perror("attributeAffects(inputCurve, output)");
		return status;
	}

	return MS::kSuccess;
}

MStatus initializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");

	status = plugin.registerNode( "arcLen", arcLen::id, arcLen::creator,
											   arcLen::initialize);
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterNode( arcLen::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	return status;
}
