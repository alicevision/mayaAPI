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
//  File: identityNode.cpp
//
//  Description:
//      Empty implementation of a deformer. This node
//      performs no deformation and is basically an empty
//      shell that can be used to create actual deformers.
//
//
//      Use this script to create a simple example with the identity node.
//      
//      loadPlugin identityNode;
//      
//      polyTorus -r 1 -sr 0.5 -tw 0 -sx 50 -sy 50 -ax 0 1 0 -cuv 1 -ch 1;
//      deformer -type "identity";
//      setKeyframe -v 0 -at weightList[0].weights[0] -t 1 identity1;
//      setKeyframe -v 1 -at weightList[0].weights[0] -t 60 identity1;
//      select -cl;

#include <maya/MFnPlugin.h>
#include <maya/MTypeId.h> 

#include <maya/MStringArray.h>

#include <maya/MPxDeformerNode.h> 
#include <maya/MItGeometry.h>
#include <maya/MPoint.h>

#include <maya/MPxGPUDeformer.h>
#include <maya/MGPUDeformerRegistry.h>
#include <maya/MOpenCLInfo.h>
#include <clew/clew_cl.h>



class identityNode : public MPxDeformerNode
{
public:
    static  void*   creator();
    static  MStatus initialize();

    // Deformation function
    //
    virtual MStatus deform(MDataBlock&    block,
                           MItGeometry&   iter,
                           const MMatrix& mat,
                           unsigned int multiIndex);

    static const MTypeId id;
};

const MTypeId identityNode::id( 0x8000d );


void* identityNode::creator()
{
    return new identityNode();
}

MStatus identityNode::initialize()
{
    return MStatus::kSuccess;
}


MStatus
identityNode::deform( MDataBlock& block,
                      MItGeometry& iter,
                      const MMatrix& /*m*/,
                      unsigned int multiIndex)
//
// Method: deform
//
// Description:   "Deforms" the point with an identity transformation
//
// Arguments:
//   block      : the datablock of the node
//   iter       : an iterator for the geometry to be deformed
//   m          : matrix to transform the point into world space
//   multiIndex : the index of the geometry that we are deforming
//
//
{
    MStatus returnStatus;
    
    // Iterate through each point in the geometry.
    //
    for ( ; !iter.isDone(); iter.next()) {
        MPoint pt = iter.position();
        
        // Perform some calculation on pt.
        // ...
        
        // Set the final position.
        iter.setPosition(pt);
    }
    return returnStatus;
}


// The GPU override implementation of the identityNode
//
class identityGPUDeformer : public MPxGPUDeformer
{
public:
    static MGPUDeformerRegistrationInfo* getGPUDeformerInfo();
    static bool validateNode(MDataBlock& block, const MEvaluationNode&, const MPlug& plug, MStringArray* messages);

    // Virtual methods from MPxGPUDeformer
    identityGPUDeformer();
    virtual ~identityGPUDeformer();

    // Implementation of MPxGPUDeformer.
    virtual MPxGPUDeformer::DeformerStatus evaluate(MDataBlock&, const MEvaluationNode&, const MPlug& plug, unsigned int, const MAutoCLMem, const MAutoCLEvent, MAutoCLMem, MAutoCLEvent&);
    virtual void terminate();

private:
    // Kernel
    MAutoCLKernel fKernel;
    size_t fLocalWorkSize;
    size_t fGlobalWorkSize;
};

// registration information for the identity GPU deformer.
class identityGPUDeformerInfo : public MGPUDeformerRegistrationInfo
{
public:
	identityGPUDeformerInfo(){}
	virtual ~identityGPUDeformerInfo(){}

	virtual MPxGPUDeformer* createGPUDeformer()
	{
		return new identityGPUDeformer();
	}

	virtual bool validateNode(MDataBlock& block, const MEvaluationNode& evaluationNode, const MPlug& plug, MStringArray* messages)
	{
		return identityGPUDeformer::validateNode(block, evaluationNode, plug, messages);
	}
};

MGPUDeformerRegistrationInfo* identityGPUDeformer::getGPUDeformerInfo()
{
    static identityGPUDeformerInfo theOne;
	return &theOne;
}

identityGPUDeformer::identityGPUDeformer()
{
}

identityGPUDeformer::~identityGPUDeformer()
{
    terminate();
}

bool identityGPUDeformer::validateNode(MDataBlock& block, const MEvaluationNode& evaluationNode, const MPlug& plug, MStringArray* messages)
{
    // Support everything.
    return true;
}

MPxGPUDeformer::DeformerStatus identityGPUDeformer::evaluate(
    MDataBlock& block,
    const MEvaluationNode& evaluationNode,
    const MPlug& plug,
    unsigned int numElements,
    const MAutoCLMem inputBuffer,
    const MAutoCLEvent inputEvent,
    MAutoCLMem outputBuffer,
    MAutoCLEvent& outputEvent
    )
{
    cl_int err = CL_SUCCESS;    
    
    // Setup OpenCL kernel.
    if ( !fKernel.get() )
    {
        // Get and compile the kernel.
        const char* mayaLocation = getenv( "MAYA_LOCATION" );

        MString openCLKernelFile( mayaLocation );
        openCLKernelFile +="/devkit/plug-ins/identityNode/identity.cl";

        MString openCLKernelName("identity");

        MAutoCLKernel kernel = MOpenCLInfo::getOpenCLKernel( openCLKernelFile, openCLKernelName );
        if ( kernel.isNull() )
        {
            return MPxGPUDeformer::kDeformerFailure;
        }

        fKernel = kernel;
        
        // Figure out a good work group size for our kernel.
        fLocalWorkSize = 0;
        fGlobalWorkSize = 0;
        size_t retSize = 0;
        err = clGetKernelWorkGroupInfo(
            fKernel.get(),
            MOpenCLInfo::getOpenCLDeviceId(),
            CL_KERNEL_WORK_GROUP_SIZE,
            sizeof(size_t),
            &fLocalWorkSize,
            &retSize
            );
        MOpenCLInfo::checkCLErrorStatus(err);
        if ( err != CL_SUCCESS || retSize == 0 || fLocalWorkSize == 0)
        {
            return MPxGPUDeformer::kDeformerFailure;
        }

        // Global work size must be a multiple of local work size.
        const size_t remain = numElements % fLocalWorkSize;
        if ( remain )
        {
            fGlobalWorkSize = numElements + ( fLocalWorkSize - remain );
        }
        else
        {
            fGlobalWorkSize = numElements;
        }
    }

    // Set all of our kernel parameters.  Input buffer and output buffer may be changing every frame
    // so always set them.
    unsigned int parameterId = 0;
    err = clSetKernelArg(fKernel.get(), parameterId++, sizeof(cl_mem), (void*)outputBuffer.getReadOnlyRef());
    MOpenCLInfo::checkCLErrorStatus(err);
    err = clSetKernelArg(fKernel.get(), parameterId++, sizeof(cl_mem), (void*)inputBuffer.getReadOnlyRef());
    MOpenCLInfo::checkCLErrorStatus(err);
    err = clSetKernelArg(fKernel.get(), parameterId++, sizeof(cl_uint), (void*)&numElements);
    MOpenCLInfo::checkCLErrorStatus(err);

    // Set up our input events.  The input event could be NULL, in that case we need to pass
    // slightly different parameters into clEnqueueNDRangeKernel.
    cl_event events[ 1 ] = { 0 };
    cl_uint eventCount = 0;
    if ( inputEvent.get() )
    {
        events[ eventCount++ ] = inputEvent.get();
    }

    // Run the kernel
    err = clEnqueueNDRangeKernel(
        MOpenCLInfo::getOpenCLCommandQueue() ,
        fKernel.get() ,
        1 ,
        NULL ,
        &fGlobalWorkSize ,
        &fLocalWorkSize ,
        eventCount ,
        events ,
        outputEvent.getReferenceForAssignment()
        );
    MOpenCLInfo::checkCLErrorStatus(err);
    if ( err != CL_SUCCESS )
    {
        return MPxGPUDeformer::kDeformerFailure;
    }

    return MPxGPUDeformer::kDeformerSuccess;
}

void identityGPUDeformer::terminate()
{
    MOpenCLInfo::releaseOpenCLKernel(fKernel);
    fKernel.reset();
}


// standard initialization procedures
//

MStatus initializePlugin( MObject obj )
{
    MStatus result;

    MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");
    result = plugin.registerNode(
        "identity" ,
        identityNode::id ,
        &identityNode::creator ,
        &identityNode::initialize ,
        MPxNode::kDeformerNode
        );

    MString nodeClassName("identity");
    MString registrantId("mayaPluginExample");
    MGPUDeformerRegistry::registerGPUDeformerCreator(
        nodeClassName,
        registrantId,
        identityGPUDeformer::getGPUDeformerInfo()
        );

    return result;
}

MStatus uninitializePlugin( MObject obj )
{
    MStatus result;

    MString nodeClassName("identity");
    MString registrantId("mayaPluginExample");
    MGPUDeformerRegistry::deregisterGPUDeformerCreator(
        nodeClassName,
        registrantId
        );

    MFnPlugin plugin( obj );
    result = plugin.deregisterNode( identityNode::id );

    return result;
}
