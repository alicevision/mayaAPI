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

// This plug-in demonstrates how to use the MPxNode::preEvaluation() method
// in conjunction with running Maya in Serial or Parallel evaluation mode.
// When Maya is running in Serial or Parallel evaluation mode additional
// code in the preEvaluation() method handles special cases.
// In this example an optimization is being made for a heavy calculation
// as simulated with the doExpensiveCalculation() method below.  There is
// a method variable cachedValueIsValid that controls if the cachedValue
// is up to date or needs to be computed.  The method setDependentsDirty()
// is used to control the cachedValueIsValid variable in the normal DG case.
// When Maya is switched to Serial or Parallel evaluation modes, an evaluation
// graph is built from the dirty state of the scene and dirty propagation is
// turned off until it is required again.  This means that setDependentsDirty
// is no longer called when dirty propagation is off.  For the evaluation
// manager to handle this case, the preEvaluation() method is implemented
// to handle the normal context.  Depending on which plugs/attributes are 
// dirty we reset the cachedValueIsValid state forcing a compute of the
// output when the evaluation manager invokes this call.
//
// To run this example, execute the MEL code below.  If you are in normal DG
// evaluation mode then clicking on the timeline will move the poly sphere.
// Switch to Serial or Parallel evaluation modes and then click on the time
// line.  You will notice that the sphere will not move.  This is because the
// DO_PRE_EVAL is turned off by default.  Turn this define on and recompile
// and run the same test again to see the sphere moving in Serial or
// Parallel evaluation modes.
//

/*
    MEL:
    loadPlugin simpleEvaluationNode;

    file -f -new;
    createNode simpleEvaluationNode;
    connectAttr time1.outTime simpleEvaluationNode1.inputTime;
    setAttr simpleEvaluationNode1.input .25;

    polySphere -ch on -o on -r 3.0;
    connectAttr simpleEvaluationNode1.output pSphere1.translateX;
*/

#include <string.h>
#include <maya/MIOStream.h>
#include <math.h>

#include <maya/MPxNode.h> 

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnUnitAttribute.h>

#include <maya/MString.h> 
#include <maya/MTypeId.h> 
#include <maya/MTime.h> 
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MEvaluationNode.h>
#include <maya/MEvaluationManager.h>

#include <maya/MFnPlugin.h>

class simpleEvaluationNode : public MPxNode
{
public:
						simpleEvaluationNode();
	virtual				~simpleEvaluationNode(); 

	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );

    virtual MStatus     setDependentsDirty( const MPlug& plug, MPlugArray& plugArray);

    virtual MStatus     preEvaluation( const  MDGContext& context, const MEvaluationNode& evaluationNode );

	static  void*		creator();
	static  MStatus		initialize();

public:
	static  MObject		input;         // The input value.
	static  MObject     aTimeInput;    // The time input
	static  MObject		output;        // The output value.
	static	MTypeId		id;

private:
    bool cachedValueIsValid;
    float cachedValue;
    float doExpensiveCalculation( float a, float b );
};

// Statics
MTypeId     simpleEvaluationNode::id( 0x0008002B );
MObject     simpleEvaluationNode::input;        
MObject     simpleEvaluationNode::aTimeInput;        
MObject     simpleEvaluationNode::output;       

// Class implementation
//
simpleEvaluationNode::simpleEvaluationNode()
    : cachedValueIsValid( false )
    , cachedValue( 0.0 )
{
}

simpleEvaluationNode::~simpleEvaluationNode()
{
}

float simpleEvaluationNode::doExpensiveCalculation( float a, float b )
{
    return a * b ;
}

MStatus simpleEvaluationNode::compute( const MPlug& plug, MDataBlock& data )
{
	
	MStatus returnStatus;
 
	if( plug == output )
	{
		MDataHandle inputData = data.inputValue( input, &returnStatus );

		if( returnStatus != MS::kSuccess )
        {
			cerr << "ERROR getting data" << endl;
        }
		else
		{
            MDataHandle inputTimeData = data.inputValue( aTimeInput, &returnStatus );
		    if( returnStatus != MS::kSuccess )
            {
			    cerr << "ERROR getting data" << endl;
            }
            else
            {
                if ( ! cachedValueIsValid )
                {
                    MTime time = inputTimeData.asTime();
			        cachedValue = doExpensiveCalculation( inputData.asFloat() , (float) time.value() );
                    cachedValueIsValid = true;
                }
			    MDataHandle outputHandle = data.outputValue( simpleEvaluationNode::output );
			    outputHandle.set( cachedValue );
			    data.setClean(plug);
            }
		}
	} else {
		return MS::kUnknownParameter;
	}

	return MS::kSuccess;
}

MStatus simpleEvaluationNode::setDependentsDirty( const MPlug& plug, MPlugArray& plugArray)
{
	if (plug == input || plug == aTimeInput )
	{
		if(MEvaluationManager::graphConstructionActive())
		{
			cout << "Evaluation Graph is being constructed" << endl;
		}
		else if(MEvaluationManager::evaluationManagerActive(MDGContext::fsNormal))
		{
			cout << "Evaluation Manager is active, but not in construction" << endl;
		}  

		cachedValueIsValid = false;
	}
	return MPxNode::setDependentsDirty(plug, plugArray);
}

// Turn this define on to see the evaluation work in Serial/Parallel modes
// #define DO_PRE_EVAL
MStatus simpleEvaluationNode::preEvaluation( const  MDGContext& context, const MEvaluationNode& evaluationNode )
{
#ifdef DO_PRE_EVAL
    MStatus status;

    // We use m_CachedValueIsValid only for normal context
    if( !context.isNormal() ) 
        return MStatus::kFailure;
                
    if( ( evaluationNode.dirtyPlugExists(input, &status) && status ) || 
            ( evaluationNode.dirtyPlugExists(aTimeInput, &status) && status ) )
    {
        cachedValueIsValid = false;
    }
#endif
    return MS::kSuccess;
}

void* simpleEvaluationNode::creator()
{
	return new simpleEvaluationNode();
}

MStatus simpleEvaluationNode::initialize()
{
	MFnNumericAttribute nAttr;
	MFnUnitAttribute    uAttr;
	MStatus				status;

	input = nAttr.create( "input", "in", MFnNumericData::kFloat, 2.0 );
 	nAttr.setStorable(true);

	aTimeInput = uAttr.create( "inputTime", "itm", MFnUnitAttribute::kTime, 0.0 );
	uAttr.setWritable(true);
	uAttr.setStorable(true);
    uAttr.setReadable(true);
    uAttr.setKeyable(true);

	output = nAttr.create( "output", "out", MFnNumericData::kFloat, 0.0 );
	nAttr.setWritable(false);
	nAttr.setStorable(false);

	status = addAttribute( input );
		if (!status) { status.perror("addAttribute"); return status;}
	status = addAttribute( aTimeInput );
		if (!status) { status.perror("addAttribute"); return status;}
	status = addAttribute( output );
		if (!status) { status.perror("addAttribute"); return status;}

	status = attributeAffects( input, output );
		if (!status) { status.perror("attributeAffects"); return status;}
	status = attributeAffects( aTimeInput, output );
		if (!status) { status.perror("attributeAffects"); return status;}

	return MS::kSuccess;
}

// Plug-in entry points
//
MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "1.0", "Any");

	status = plugin.registerNode( "simpleEvaluationNode", simpleEvaluationNode::id, simpleEvaluationNode::creator,
								  simpleEvaluationNode::initialize );
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

	status = plugin.deregisterNode( simpleEvaluationNode::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	return status;
}
