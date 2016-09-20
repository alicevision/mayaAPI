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

//
//  File: sseDeformer.cc
//
//  Description:
// 		Example implementation of a deformer. This node
//		offsets vertices according to the CV's weights.
//		The weights are set using the set editor or the
//		percent command.
//

#include <string.h>
#include <float.h> // for FLT_MAX
#include <maya/MIOStream.h>
#include <math.h>

#include <maya/MPxGeometryFilter.h> 
#include <maya/MItGeometry.h>
#include <maya/MPxLocatorNode.h> 

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnMatrixData.h>

#include <maya/MFnPlugin.h>
#include <maya/MFnDependencyNode.h>

#include <maya/MTypeId.h> 
#include <maya/MPlug.h>

#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MArrayDataHandle.h>

#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MMatrix.h>
#include <maya/MTimer.h>

#include <maya/MDagModifier.h>
#include <maya/MFnMesh.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnMeshData.h>
#include <maya/MFloatVectorArray.h>

// Macros
//
#define MCheckStatus(status,message)	\
	if( MStatus::kSuccess != status ) {	\
		cerr << message << "\n";		\
		return status;					\
	}

class sseDeformer : public MPxGeometryFilter
{
public:
						sseDeformer();
	virtual				~sseDeformer();

	static  void*		creator();
	static  MStatus		initialize();

	// deformation function
	//
	virtual MStatus compute(const MPlug& plug, MDataBlock& dataBlock);

public:
	// local node attributes
	static  MObject sseEnabled;

	static  MTypeId		id;

private:
};

MTypeId     sseDeformer::id( 0x8104E );

// local attributes
MObject sseDeformer::sseEnabled;

sseDeformer::sseDeformer() {}
sseDeformer::~sseDeformer() {}

void* sseDeformer::creator()
{
	return new sseDeformer();
}

MStatus sseDeformer::initialize()
{
	// local attribute initialization
	MStatus status;
	MFnNumericAttribute mSSEAttr;
 	sseEnabled=mSSEAttr.create( "enableSSE", "sse", MFnNumericData::kBoolean, 0, &status);
	mSSEAttr.setStorable(true);

 	//  deformation attributes
 	status = addAttribute( sseEnabled );
	MCheckStatus(status, "ERROR in addAttribute\n");

 	status = attributeAffects( sseEnabled, outputGeom );
	MCheckStatus(status, "ERROR in attributeAffects\n");

	return MStatus::kSuccess;
}

MStatus sseDeformer::compute(const MPlug& plug, MDataBlock& data)
{
	MStatus status;
 	if (plug.attribute() != outputGeom) {
		printf("Ignoring requested plug\n");
		return status;
	}
	unsigned int index = plug.logicalIndex();
	MObject thisNode = this->thisMObject();

	// get input value
	MPlug inPlug(thisNode,input);
	inPlug.selectAncestorLogicalIndex(index,input);
	MDataHandle hInput = data.inputValue(inPlug, &status);
	MCheckStatus(status, "ERROR getting input mesh\n");
	
	// get the input geometry
	MDataHandle inputData = hInput.child(inputGeom);
	if (inputData.type() != MFnData::kMesh) {
 		printf("Incorrect input geometry type\n");
		return MStatus::kFailure;
 	}

  	MObject iSurf = inputData.asMesh() ;
  	MFnMesh inMesh;
  	inMesh.setObject( iSurf ) ;

	MDataHandle outputData = data.outputValue(plug);
	outputData.copy(inputData);
 	if (outputData.type() != MFnData::kMesh) {
		printf("Incorrect output mesh type\n");
		return MStatus::kFailure;
	}
	
  	MObject oSurf = outputData.asMesh() ;
 	if(oSurf.isNull()) {
		printf("Output surface is NULL\n");
		return MStatus::kFailure;
	}

  	MFnMesh outMesh;
  	outMesh.setObject( oSurf ) ;
	MCheckStatus(status, "ERROR setting points\n");

	// get all points at once for demo purposes. Really should get points from the current group using iterator
	MFloatPointArray pts;
	outMesh.getPoints(pts);

 	int nPoints = pts.length();

	MDataHandle envData = data.inputValue(envelope, &status);
	float env = envData.asFloat();	

	MDataHandle sseData = data.inputValue(sseEnabled, &status);
	bool sseEnabled = (bool) sseData.asBool();	

	// NOTE: Using MTimer and possibly other classes disables
	// autovectorization with Intel <=10.1 compiler on OSX and Linux!!
	// Must compile this function with -fno-exceptions on OSX and
	// Linux to guarantee autovectorization is done. Use -fvec_report2
	// to check for vectorization status messages with Intel compiler.
 	MTimer timer; timer.beginTimer();

	if(sseEnabled) {

		// Innter loop will autovectorize. Around 3x faster than the
		// loop below it. It would be faster if first element was
		// guaranteed to be aligned on 16 byte boundary.
		for(int i=0; i<nPoints; i++) {
			float* ptPtr = &pts[i].x;
			for(int j=0; j<4; j++) {
				ptPtr[j] = env * (cosf(ptPtr[j]) * sinf(ptPtr[j]) * tanf(ptPtr[j]));
			}
		}

	} else {

		// This inner loop will not autovectorize.
		for(int i=0; i<nPoints; i++) {
			MFloatPoint& pt = pts[i];
			for(int j=0; j<3; j++) {
				pt[j] = env * (cosf(pt[j]) * sinf(pt[j]) * tanf(pt[j]));
			}

		}
	}

 	timer.endTimer(); 
	if(sseEnabled) {
		printf("SSE enabled, runtime %f\n", timer.elapsedTime());
	} else {
		printf("SSE disabled, runtime %f\n", timer.elapsedTime());
	}

	outMesh.setPoints(pts);

	return status;
}


// standard initialization procedures
//

MStatus initializePlugin( MObject obj )
{
	MStatus result;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "1.0", "Any");
	result = plugin.registerNode( "sseDeformer", sseDeformer::id, sseDeformer::creator, 
								  sseDeformer::initialize, MPxNode::kDeformerNode );

	return result;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus result;
	MFnPlugin plugin( obj );
	result = plugin.deregisterNode( sseDeformer::id );
	return result;
}
