//-
// ==========================================================================
// Copyright 2012 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

// Example plugin: vertexBufferMutator.cpp
//
// This plug-in is an example of a custom MPxVertexBufferMutator.
// It provides custom vertex streams based on shader requirements coming from 
// an MPxShaderOverride.  The semanticName() in the MVertexBufferDescriptor is used 
// to signify a unique identifier for a custom stream.

// This plugin is meant to be used in conjunction with the hwPhongShader
// where the position geometry is defined with the semantic "swizzlePosition"

#include <maya/MStatus.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnMesh.h>
#include <maya/MDrawRegistry.h>
#include <maya/MPxVertexBufferMutator.h> 
#include <maya/MHWGeometry.h>

using namespace MHWRender;

class MyCustomBufferMutator : public MHWRender::MPxVertexBufferMutator
{
public:
    MyCustomBufferMutator() {}
    virtual ~MyCustomBufferMutator() {}

    virtual void modifyVertexStream(const MObject& object,
		MVertexBuffer& vertexBuffer, const MComponentDataIndexing& targetIndexing) const
    {
        // get the descriptor from the vertex buffer.  
        // It describes the format and layout of the stream.
        const MVertexBufferDescriptor& descriptor = vertexBuffer.descriptor();
        
        // we are expecting a float stream.
        if (descriptor.dataType() != MGeometry::kFloat) 
            return;

        // we are expecting a float3
        if (descriptor.dimension() != 3)
            return;

        // we are expecting a position channel
        if (descriptor.semantic() != MGeometry::kPosition)
            return;

        // get the mesh from the current path
        // if it is not a mesh we do nothing.
        MStatus status;
        MFnMesh mesh(object, &status);
        if (!status) return; // failed

        unsigned int vertexCount = vertexBuffer.vertexCount();
        if (vertexCount <= 0)
            return;

        // acquire the buffer to fill with data.
        float* buffer = (float*)vertexBuffer.acquire(vertexCount, false /*writeOnly - we want the current buffer values*/);
        float* start = buffer;

        for(unsigned int i = 0; i < vertexCount; ++i)
        {
            // Here we swap the x, y and z values
			float x = buffer[0];
			buffer[0] = buffer[1];	// y --> x
			buffer[1] = buffer[2];	// z --> y
			buffer[2] = x;			// x --> z
			buffer += 3;
        }

        // commit the buffer to signal completion.
        vertexBuffer.commit(start);
    }
};

// This is the buffer generator creation function registered with the DrawRegistry.
// Used to initialize the generator.
static MPxVertexBufferMutator* createMyCustomBufferMutator()
{
    return new MyCustomBufferMutator();
}

// The following routines are used to register/unregister
// the vertex mutators with Maya
//
MStatus initializePlugin( MObject obj )
{
    MDrawRegistry::registerVertexBufferMutator("swizzlePosition", createMyCustomBufferMutator);
    return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj)
{
    MDrawRegistry::deregisterVertexBufferMutator("swizzlePosition");
    return MS::kSuccess;
}
