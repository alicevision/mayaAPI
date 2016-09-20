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

// Example plugin: customPrimitiveGenerator.cpp
//
// This plug-in is an example of a custom MPxPrimitiveGenerator.
// It provides custom primitives based on shader requirements coming from 
// an MPxShaderOverride.  The name() in the MIndexBufferDescriptor is used 
// to signify a unique identifier for a custom buffer.

// This primitive generator is provided for demonstration purposes only.
// It simply provides a triangle list for mesh objects with no vertex sharing.
// A more sophisticated primitive provider could be used to provide patch primitives
// for GPU tessellation.


// This plugin is meant to be used in conjunction with the d3d11Shader or cgShader or the hwPhongShader plugins.
// The customPrimitiveGeneratorDX11.fx and customPrimitiveGeneratorGL.cgfx files accompanying this sample
// can be loaded using the appropriate shader plugin.
// In any case, the environment variable MAYA_USE_CUSTOMPRIMITIVEGENERATOR needs to be set (any value is fine) for it to be enabled.


#include <maya/MStatus.h>
#include <maya/MFnMesh.h>
#include <maya/MIntArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MPxPrimitiveGenerator.h>
#include <maya/MPxVertexBufferGenerator.h>
#include <maya/MHWGeometry.h>
#include <maya/MDrawRegistry.h>
#include <list>

using namespace MHWRender;

class MyCustomPrimitiveGenerator : public MPxPrimitiveGenerator
{
public:
    MyCustomPrimitiveGenerator() {}
    virtual ~MyCustomPrimitiveGenerator() {}

    virtual unsigned int computeIndexCount(const MObject& object, const MObject& component) const
    {
        // get the mesh from the current path
        // if it is not a mesh we do nothing.
        MStatus status;
        MFnMesh mesh(object, &status);
        if (!status) return 0; // failed

        return mesh.numFaceVertices();
    }

    virtual MGeometry::Primitive generateIndexing(const MObject& object, const MObject& component,
        const MComponentDataIndexingList& /*sourceIndexing*/,
        const MComponentDataIndexingList& targetIndexing,
        MIndexBuffer& indexBuffer,
		int& primitiveStride) const
    {
        // get the mesh from the current path
        // if it is not a mesh we do nothing.
        MStatus status;
        MFnMesh mesh(object, &status);
        if (!status) return MGeometry::kInvalidPrimitive; // failed 

        for (int x = 0; x < targetIndexing.length(); ++x)
        {
            const MComponentDataIndexing* target = targetIndexing[x];
            if (target->componentType() == MComponentDataIndexing::kFaceVertex)
            {
                // Get Triangles
                MIntArray triCounts;
                MIntArray triVertIDs;
                mesh.getTriangleOffsets(triCounts, triVertIDs);
                unsigned int numTriVerts = triVertIDs.length();
  
				unsigned int customNumTriVerts = numTriVerts * 2;
                void* indexData = indexBuffer.acquire(customNumTriVerts, true /*writeOnly - we don't need the current buffer values*/);
                if (indexData == NULL)
                    return MGeometry::kInvalidPrimitive;

				const MUintArray& sharedIndices = target->indices();

				// Crawl the sharedIndices array to find the last
				// The new vertices will be added at the end
				unsigned int nextNewVertexIndex = 0;
				for(unsigned int idx = 0; idx < sharedIndices.length(); ++idx)
					nextNewVertexIndex = std::max(nextNewVertexIndex, sharedIndices[idx]);
				++nextNewVertexIndex;

				unsigned int newTriId = 0;
                for(unsigned int triId = 0; triId < numTriVerts; )
                {
					// split each triangle in two : add new vertex between vertexId1 and vertexId2
					unsigned int vertexId0 = sharedIndices[triVertIDs[triId++]];
					unsigned int vertexId1 = sharedIndices[triVertIDs[triId++]];
					unsigned int vertexId2 = sharedIndices[triVertIDs[triId++]];

					unsigned int newVertexIndex = nextNewVertexIndex++;

					// Triangle 0 1 2 become two triangles : 0 1 X and 0 X 2
					if (indexBuffer.dataType() == MGeometry::kUnsignedInt32) {
						((unsigned int*)indexData)[newTriId++] = vertexId0;
						((unsigned int*)indexData)[newTriId++] = vertexId1;
						((unsigned int*)indexData)[newTriId++] = newVertexIndex;

						((unsigned int*)indexData)[newTriId++] = vertexId0;
						((unsigned int*)indexData)[newTriId++] = newVertexIndex;
						((unsigned int*)indexData)[newTriId++] = vertexId2;
					}
					else if (indexBuffer.dataType() == MGeometry::kUnsignedChar) {
						((unsigned short*)indexData)[newTriId++] = (unsigned short)vertexId0;
						((unsigned short*)indexData)[newTriId++] = (unsigned short)vertexId1;
						((unsigned short*)indexData)[newTriId++] = (unsigned short)newVertexIndex;

						((unsigned short*)indexData)[newTriId++] = (unsigned short)vertexId0;
						((unsigned short*)indexData)[newTriId++] = (unsigned short)newVertexIndex;
						((unsigned short*)indexData)[newTriId++] = (unsigned short)vertexId2;
					}
				}

				indexBuffer.commit(indexData);
				primitiveStride = 3;
                return MGeometry::kTriangles;
            }
        }

        return MGeometry::kInvalidPrimitive;
    }
};

// This is the primitive generator creation function registered with the DrawRegistry.
// Used to initialize a custom primitive generator.
static MPxPrimitiveGenerator* createMyCustomPrimitiveGenerator()
{
    return new MyCustomPrimitiveGenerator();
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

class MyCustomPositionBufferGenerator : public MPxVertexBufferGenerator
{
public:
    MyCustomPositionBufferGenerator() {}
    virtual ~MyCustomPositionBufferGenerator() {}

	virtual bool getSourceIndexing(const MObject& object, 
        MComponentDataIndexing& sourceIndexing) const
	{
        // get the mesh from the current path
        // if it is not a mesh we do nothing.
        MStatus status;
        MFnMesh mesh(object, &status);
        if (!status) return false; // failed

		MIntArray vertexCount, vertexList;
		mesh.getVertices(vertexCount, vertexList);
		unsigned int vertCount = vertexList.length();

		MUintArray& vertices = sourceIndexing.indices();
		vertices.setLength(vertCount);

		for(unsigned int i = 0; i < vertCount; ++i)
			vertices[i] = (unsigned int)vertexList[i];

        // assign the source indexing
        sourceIndexing.setComponentType(MComponentDataIndexing::kFaceVertex);

		return true;
	}

	virtual bool getSourceStreams(const MObject& object,
		MStringArray &sourceStreams) const
	{
		// No source stream needed
		return false;
	}

	virtual void createVertexStream(const MObject& object,
        MVertexBuffer& vertexBuffer, const MComponentDataIndexing& targetIndexing, const MComponentDataIndexing& sharedIndexing, const MVertexBufferArray& /*sourceStreams*/) const
	{
		// get the descriptor from the vertex buffer.  
		// It describes the format and layout of the stream.
		const MVertexBufferDescriptor& descriptor = vertexBuffer.descriptor();
        
		// we are expecting a float stream.
		MGeometry::DataType dataType = descriptor.dataType();
		if (dataType != MGeometry::kFloat)
			return;

		// we are expecting a dimension of 3
		int dimension = descriptor.dimension();
		if (dimension != 3)
			return;

		// we are expecting a position channel
		if (descriptor.semantic() != MGeometry::kPosition)
			return;

		// get the mesh from the current path
		// if it is not a mesh we do nothing.
		MStatus status;
		MFnMesh mesh(object, &status);
		if (!status) return; // failed

        const MUintArray& indices = targetIndexing.indices();

        unsigned int vertexCount = indices.length();
        if (vertexCount <= 0)
            return;

		// Keep track of the vertices that will be used to created a new vertex in-between
		typedef std::list< std::pair< unsigned int, unsigned int > > ExtraVerticesList;
		ExtraVerticesList extraVertices;
		{
			// Get Triangles
			MIntArray triCounts;
			MIntArray triVertIDs;
			mesh.getTriangleOffsets(triCounts, triVertIDs);
			unsigned int numTriVerts = triVertIDs.length();

			const MUintArray& sharedIndices = sharedIndexing.indices();

            for(unsigned int triId = 0; triId < numTriVerts; )
            {
				// split each triangle in two : add new vertex between vertexId1 and vertexId2
				triId++; //unsigned int vertexId0 = sharedIndices[triVertIDs[triId++]];
				unsigned int vertexId1 = sharedIndices[triVertIDs[triId++]];
				unsigned int vertexId2 = sharedIndices[triVertIDs[triId++]];

				extraVertices.push_back( std::make_pair(vertexId1, vertexId2) );
			}
		}

		unsigned int newVertexCount = vertexCount + (unsigned int)extraVertices.size();
		float* customBuffer = (float*)vertexBuffer.acquire(newVertexCount, true /*writeOnly - we don't need the current buffer values*/);
		if(customBuffer)
		{
			float* customBufferStart = customBuffer;

			// Append 'real' vertices position
			unsigned int vertId = 0;
			for(; vertId < vertexCount; ++vertId)
			{
				unsigned int vertexId = indices[vertId];

				MPoint point;
				mesh.getPoint(vertexId, point);

				customBuffer[vertId * dimension] = (float)point.x;
				customBuffer[vertId * dimension + 1] = (float)point.y;
				customBuffer[vertId * dimension + 2] = (float)point.z;
			}

			// Append the new vertices position, interpolated from vert1 and vert2
			ExtraVerticesList::const_iterator it = extraVertices.begin();
			ExtraVerticesList::const_iterator itEnd = extraVertices.end();
			for(; it != itEnd; ++it, ++vertId)
			{
				unsigned int vertexId1 = indices[it->first];
				unsigned int vertexId2 = indices[it->second];

				MPoint point1, point2;
				mesh.getPoint(vertexId1, point1);
				mesh.getPoint(vertexId2, point2);

				MPoint point = (point1 + point2) / 2.0f;

				customBuffer[vertId * dimension] = (float)point.x;
				customBuffer[vertId * dimension + 1] = (float)point.y;
				customBuffer[vertId * dimension + 2] = (float)point.z;
			}

			vertexBuffer.commit(customBufferStart);
		}
	}
};

// This is the buffer generator creation function registered with the DrawRegistry.
// Used to initialize the generator.
static MPxVertexBufferGenerator* createMyCustomPositionBufferGenerator()
{
    return new MyCustomPositionBufferGenerator();
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

class MyCustomNormalBufferGenerator : public MPxVertexBufferGenerator
{
public:
    MyCustomNormalBufferGenerator() {}
    virtual ~MyCustomNormalBufferGenerator() {}

	virtual bool getSourceIndexing(const MObject& object, 
        MComponentDataIndexing& sourceIndexing) const
	{
        // get the mesh from the current path
        // if it is not a mesh we do nothing.
        MStatus status;
        MFnMesh mesh(object, &status);
        if (!status) return false; // failed

		MIntArray vertexCount, vertexList;
		mesh.getVertices(vertexCount, vertexList);
		unsigned int vertCount = vertexList.length();

		MUintArray& vertices = sourceIndexing.indices();
		vertices.setLength(vertCount);

		for(unsigned int i = 0; i < vertCount; ++i)
			vertices[i] = (unsigned int)vertexList[i];

        // assign the source indexing
        sourceIndexing.setComponentType(MComponentDataIndexing::kFaceVertex);
		
		return true;
	}

	virtual bool getSourceStreams(const MObject& object,
		MStringArray &sourceStreams) const
	{
		// No source stream needed
		return false;
	}

	virtual void createVertexStream(const MObject& object,
        MVertexBuffer& vertexBuffer, const MComponentDataIndexing& targetIndexing, const MComponentDataIndexing& sharedIndexing, const MVertexBufferArray& /*sourceStreams*/) const
	{
		// get the descriptor from the vertex buffer.  
		// It describes the format and layout of the stream.
		const MVertexBufferDescriptor& descriptor = vertexBuffer.descriptor();
        
		// we are expecting a float stream.
		MGeometry::DataType dataType = descriptor.dataType();
		if (dataType != MGeometry::kFloat)
			return;

		// we are expecting a dimension of 3
		int dimension = descriptor.dimension();
		if (dimension != 3)
			return;

		// we are expecting a normal channel
		if (descriptor.semantic() != MGeometry::kNormal)
			return;

		// get the mesh from the current path
		// if it is not a mesh we do nothing.
		MStatus status;
		MFnMesh mesh(object, &status);
		if (!status) return; // failed

        const MUintArray& indices = targetIndexing.indices();

        unsigned int vertexCount = indices.length();
        if (vertexCount <= 0)
            return;

		// Keep track of the vertices that will be used to created a new vertex in-between
		typedef std::list< std::pair< unsigned int, unsigned int > > ExtraVerticesList;
		ExtraVerticesList extraVertices;
		{
			// Get Triangles
			MIntArray triCounts;
			MIntArray triVertIDs;
			mesh.getTriangleOffsets(triCounts, triVertIDs);
			unsigned int numTriVerts = triVertIDs.length();

			const MUintArray& sharedIndices = sharedIndexing.indices();

            for(unsigned int triId = 0; triId < numTriVerts;)
            {
				// split each triangle in two : add new vertex between vertexId1 and vertexId2
				triId++; //unsigned int vertexId0 = sharedIndices[triVertIDs[triId++]];
				unsigned int vertexId1 = sharedIndices[triVertIDs[triId++]];
				unsigned int vertexId2 = sharedIndices[triVertIDs[triId++]];

				extraVertices.push_back( std::make_pair(vertexId1, vertexId2) );
			}
		}

		unsigned int newVertexCount = vertexCount + (unsigned int)extraVertices.size();
		float* customBuffer = (float*)vertexBuffer.acquire(newVertexCount, true /*writeOnly - we don't need the current buffer values*/);
		if(customBuffer)
		{
			float* customBufferStart = customBuffer;

			MFloatVectorArray normals;
			mesh.getNormals(normals);

			// Append 'real' vertices normal
			unsigned int vertId = 0;
			for(; vertId < vertexCount; ++vertId)
			{
				const MFloatVector& normal = normals[vertId];

				customBuffer[vertId * dimension] = normal.x;
				customBuffer[vertId * dimension + 1] = normal.y;
				customBuffer[vertId * dimension + 2] = normal.z;
			}

			// Append the new vertices normal, interpolated from vert1 and vert2
			ExtraVerticesList::const_iterator it = extraVertices.begin();
			ExtraVerticesList::const_iterator itEnd = extraVertices.end();
			for(; it != itEnd; ++it, ++vertId)
			{
				unsigned int vertexId1 = it->first;
				unsigned int vertexId2 = it->second;

				const MFloatVector& normal1 = normals[vertexId1];
				const MFloatVector& normal2 = normals[vertexId2];

				MFloatVector normal = (normal1 + normal2) / 2.0f;

				customBuffer[vertId * dimension] = normal.x;
				customBuffer[vertId * dimension + 1] = normal.y;
				customBuffer[vertId * dimension + 2] = normal.z;
			}

			vertexBuffer.commit(customBufferStart);
		}
	}
};

// This is the buffer generator creation function registered with the DrawRegistry.
// Used to initialize the generator.
static MPxVertexBufferGenerator* createMyCustomNormalBufferGenerator()
{
    return new MyCustomNormalBufferGenerator();
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

// The following routines are used to register/unregister
// the vertex mutators with Maya
//
MStatus initializePlugin( MObject obj )
{
	MDrawRegistry::registerPrimitiveGenerator("customPrimitiveTest", createMyCustomPrimitiveGenerator);

	MDrawRegistry::registerVertexBufferGenerator("customPositionStream", createMyCustomPositionBufferGenerator);

	MDrawRegistry::registerVertexBufferGenerator("customNormalStream", createMyCustomNormalBufferGenerator);

	return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj)
{
	MDrawRegistry::deregisterPrimitiveGenerator("customPrimitiveTest");

	MDrawRegistry::deregisterVertexBufferGenerator("customPositionStream");

	MDrawRegistry::deregisterVertexBufferGenerator("customNormalStream");

    return MS::kSuccess;
}
