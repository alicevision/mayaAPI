//
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


//
//  File: weightListNode.cpp
//
//  Description:
// 		Example implementation of a node which read and write a 
//              multi of multi of float attibute in the compute() method. The
//              definition of this multi of multi of float attibute is 
//              the same as the weightList attribute for deformer. 
//        
//              To test the node, use the following MEL commands
/*
				loadPlugin weightListNode;
				createNode weightList;
				setAttr weightList1.bias 1;
				getAttr -type weightList1.weightsList;
*/

#include <string.h>
#include <maya/MIOStream.h>
#include <math.h>

#include <maya/MPxNode.h> 
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MArrayDataBuilder.h>

#define McheckErr(stat,msg)		\
	if ( MS::kSuccess != stat ) {	\
		cerr << msg;		\
		return MS::kFailure;	\
	}

class weightList : public MPxNode
{
public:
				weightList();
	virtual			~weightList();

	static  void*		creator();
	static  MStatus		initialize();

	// deformation function
	//
        virtual MStatus      	compute( const MPlug&, MDataBlock&);

public:
	// local node attributes

	static  MObject         aWeightsList;
	static  MObject         aWeights;
	static  MObject         aBias;
	static  MTypeId		id;

private:
};

MTypeId weightList::id( 0x81035 );

// local attributes
//
MObject	weightList::aWeightsList;
MObject	weightList::aWeights;
MObject weightList::aBias;

weightList::weightList() {}
weightList::~weightList() {}

void* weightList::creator()
{
	return new weightList();
}

MStatus weightList::initialize()
{
        MStatus status;
	MFnNumericAttribute numAtt; 

	aBias = numAtt.create( "bias", "b", MFnNumericData::kFloat);
	addAttribute(aBias);

	aWeights = numAtt.create("weights", "w", MFnNumericData::kFloat, -1000.0, &status); 
	numAtt.setKeyable(true); 
	numAtt.setArray(true); 
	numAtt.setReadable(true); 
	numAtt.setUsesArrayDataBuilder(true); 
	// setIndexMatters() will only affect array attributes with setReadable set to false, 
	// i.e. destination attributes. We have set the default value to an unlikely value 
	// to guarantee an entry is created regardless of its value. 
	// numAtt.setIndexMatters(true);
	addAttribute(aWeights); 
	
	MFnCompoundAttribute cmpAttr; 
	aWeightsList = cmpAttr.create("weightsList", "wl", &status); 
	cmpAttr.setArray(true); 
	cmpAttr.addChild(aWeights); 
	cmpAttr.setReadable(true); 
	cmpAttr.setUsesArrayDataBuilder(true); 
	// cmpAttr.setIndexMatters(true);
	addAttribute(aWeightsList); 

	attributeAffects(aBias, aWeightsList);
	
	return MStatus::kSuccess;
}

MStatus weightList::compute( const MPlug& plug, MDataBlock& block)
{
    MStatus status = MS::kSuccess;

	unsigned i, j;
	MObject thisNode = thisMObject();
	MPlug wPlug(thisNode, aWeights); 

	// Write into aWeightList
	for( i = 0; i < 3; i++) {
	    status = wPlug.selectAncestorLogicalIndex( i, aWeightsList );
	    MDataHandle wHandle = wPlug.constructHandle(block);
	    MArrayDataHandle arrayHandle(wHandle, &status);
	    McheckErr(status, "arrayHandle construction failed\n");
	    MArrayDataBuilder arrayBuilder = arrayHandle.builder(&status);
	    McheckErr(status, "arrayBuilder accessing/construction failed\n");
	    for( j = 0; j < i+2; j++) {
	        MDataHandle handle = arrayBuilder.addElement(j,&status);
			McheckErr(status, "addElement to arrayBuilder failed\n");
			float val = (float)(1.0f*(i+j)); 
			handle.set(val);
	    }
	    status = arrayHandle.set(arrayBuilder);
	    McheckErr(status, "set arrayBuilder failed\n");
	    wPlug.setValue(wHandle);
	    wPlug.destructHandle(wHandle);
	}

	// Read from aWeightList and print out result
	MArrayDataHandle arrayHandle = block.outputArrayValue(aWeightsList, &status);
	McheckErr(status, "arrayHandle construction for aWeightsList failed\n");
	unsigned count = arrayHandle.elementCount();
	for( i = 0; i < count; i++) {
	    arrayHandle.jumpToElement(i);
	    MDataHandle eHandle = arrayHandle.outputValue(&status).child(aWeights);
	    McheckErr(status, "handle evaluation failed\n");
	    MArrayDataHandle eArrayHandle(eHandle, &status);
	    McheckErr(status, "arrayHandle construction for aWeights failed\n");
	    unsigned eCount = eArrayHandle.elementCount();
	    for( j = 0; j < eCount; j++) {
	        eArrayHandle.jumpToElement(j);
			float weight = eArrayHandle.outputValue(&status).asFloat();
			McheckErr(status, "weight evaluation error\n");
			fprintf(stderr, "weightList[%u][%u] = %g\n",i,j,weight);
	    }
	}

	// Read from aWeightList and print out result using the more
	// efficient jumpToArrayElement() call
	arrayHandle = block.outputArrayValue(aWeightsList, &status);
	McheckErr(status, "arrayHandle construction for aWeightsList failed\n");
	count = arrayHandle.elementCount();
	for( i = 0; i < count; i++) {
	    arrayHandle.jumpToArrayElement(i);
	    MDataHandle eHandle = arrayHandle.outputValue(&status).child(aWeights);
	    McheckErr(status, "handle evaluation failed\n");
	    MArrayDataHandle eArrayHandle(eHandle, &status);
	    McheckErr(status, "arrayHandle construction for aWeights failed\n");
	    unsigned eCount = eArrayHandle.elementCount();
	    for( j = 0; j < eCount; j++) {
	        eArrayHandle.jumpToArrayElement(j);
			float weight = eArrayHandle.outputValue(&status).asFloat();
			McheckErr(status, "weight evaluation error\n");
			fprintf(stderr, "weightList[%d][%d] = %g\n",i,j,weight);
	    }
	}

	return status;
}

// standard initialization procedures
//

MStatus initializePlugin( MObject obj )
{
	MStatus result;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");
	result = plugin.registerNode( "weightList", 
				      weightList::id, 
				      weightList::creator, 
				      weightList::initialize);

	return result;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus result;
	MFnPlugin plugin( obj );
	result = plugin.deregisterNode( weightList::id );
	return result;
}
