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

// Example plugin: crackFreePrimitiveGenerator.cpp
//
// This plug-in is an example of a custom MPxIndexBufferMutator.
// It provides custom primitives based on shader requirements coming from 
// an MPxShaderOverride.  The name() in the MIndexBufferDescriptor is used 
// to signify a unique identifier for a custom buffer.

#include "crackFreePrimitiveGenerator.h"
#include <maya/MStatus.h>
#include <maya/MFnMesh.h>
#include <maya/MIntArray.h>
#include <maya/MFloatArray.h>
#include <maya/MPxVertexBufferGenerator.h>
#include <maya/MHWGeometry.h>
#include <maya/MDrawRegistry.h>
#include <maya/MComponentDataIndexing.h>
#include <map>

namespace
{
	struct Edge
	{
		Edge(unsigned int v0_ = 0, unsigned int v1_ = 0) : v0(v0_), v1(v1_) {}

		Edge reversed() const
		{
			return Edge(v1, v0);
		}

		bool isEqual(const Edge &rhs) const
		{
			return (v0 == rhs.v0 && v1 == rhs.v1);
		}

		bool isReverse(const Edge &rhs) const
		{
			return (v0 == rhs.v1 && v1 == rhs.v0);
		}

		unsigned int v0;
		unsigned int v1;
	};

	bool operator< (const Edge& lhs, const Edge& rhs)
	{
		return (lhs.v0 < rhs.v0) || (lhs.v0 == rhs.v0 && lhs.v1 < rhs.v1);
	}

	class EdgeMapping
	{
	public:
		EdgeMapping();

		void addTriangle(unsigned int faceVertexId0, unsigned int faceVertexId1, unsigned int faceVertexId2, unsigned int polyVertexId0, unsigned int polyVertexId1, unsigned int polyVertexId2);
		void addEdge(const Edge& faceEdge, const Edge& polyEdge);

		void addPositionUV(unsigned int faceVertexId, unsigned int polyVertexId, float u, float v);

		bool adjacentEdge(const Edge& faceEdge, Edge& adjacentEdge) const;

		bool dominantEdge(const Edge& faceEdge, Edge& dominantEdge) const;

		bool dominantPosition(unsigned int faceVertexId, unsigned int& dominantVertexId) const;

	private:
		// Map each face edge to its polygon edge.
		typedef std::map< Edge, Edge > FaceEdge2PolyEdgeMap;
		FaceEdge2PolyEdgeMap fFaceEdge2PolyEdgeMap;

		// Map each poly edge to both face edges that match.
		typedef std::map< Edge, std::pair< Edge, Edge > > PolyEdge2FaceEdgeMap;
		PolyEdge2FaceEdgeMap fPolyEdge2FaceEdgesMap;

		// Map a single face vertex id to its polygon vertex id.
		typedef std::map< unsigned int, unsigned int > FaceVertex2PolyVertexMap;
		FaceVertex2PolyVertexMap fFaceVertex2PolyVertexMap;

		// Map dominant vertex position via lowest uv coords.
		typedef std::pair< unsigned int, std::pair< float, float > > VertexUVPair;
		typedef std::map< unsigned int, VertexUVPair > PolyVertex2FaceVertexUVMap;
		PolyVertex2FaceVertexUVMap fPolyVertex2FaceVertexUVMap;
	};

	EdgeMapping::EdgeMapping()
	{
	}

	void EdgeMapping::addTriangle(unsigned int faceVertexId0, unsigned int faceVertexId1, unsigned int faceVertexId2, unsigned int polyVertexId0, unsigned int polyVertexId1, unsigned int polyVertexId2)
	{
		addEdge( Edge(faceVertexId0, faceVertexId1), Edge(polyVertexId0, polyVertexId1) );
		addEdge( Edge(faceVertexId1, faceVertexId2), Edge(polyVertexId1, polyVertexId2) );
		addEdge( Edge(faceVertexId2, faceVertexId0), Edge(polyVertexId2, polyVertexId0) );
	}

	// Add a new edge
	// The edge is represented by two associated vertex ids pairs : one in face vertices array space and a second in polygon face vertices array space.
	void EdgeMapping::addEdge(const Edge& faceEdge, const Edge& polyEdge)
	{
		if(polyEdge.v1 < polyEdge.v0)
		{
			// Revert edges
			Edge faceEdgeR = faceEdge.reversed();
			Edge polyEdgeR = polyEdge.reversed();
			addEdge(faceEdgeR, polyEdgeR);
			return;
		}

		fFaceEdge2PolyEdgeMap[faceEdge] = polyEdge;
		
		PolyEdge2FaceEdgeMap::iterator itP2F = fPolyEdge2FaceEdgesMap.find( polyEdge );
		if(itP2F == fPolyEdge2FaceEdgesMap.end())
		{
			fPolyEdge2FaceEdgesMap[polyEdge] = std::make_pair( faceEdge, faceEdge );
		}
		else
		{
			itP2F->second.second = faceEdge;
		}
	}

	void EdgeMapping::addPositionUV(unsigned int faceVertexId, unsigned int polyVertexId, float u, float v)
	{
		fFaceVertex2PolyVertexMap[faceVertexId] = polyVertexId;

		PolyVertex2FaceVertexUVMap::iterator it = fPolyVertex2FaceVertexUVMap.find(polyVertexId);
		if(it == fPolyVertex2FaceVertexUVMap.end())
		{
			fPolyVertex2FaceVertexUVMap[polyVertexId] = std::make_pair( faceVertexId, std::make_pair(u,v) );
		}
		else
		{
			VertexUVPair& vertexUVPair = it->second;
			float lastU = vertexUVPair.second.first;
			float lastV = vertexUVPair.second.second;
			if( u < lastU || ( u == lastU && v < lastV ) )
			{
				vertexUVPair.first = faceVertexId;
				vertexUVPair.second = std::make_pair(u,v);
			}
		}
	}

	// Find the adjacent edge that is shared between two faces.
	// The matching is done through the polygon vertex ids.
	// The returning edge have vertices in face space.
	bool EdgeMapping::adjacentEdge(const Edge& faceEdge, Edge& adjacentEdge) const
	{
		FaceEdge2PolyEdgeMap::const_iterator itF2P = fFaceEdge2PolyEdgeMap.find( faceEdge );
		if(itF2P == fFaceEdge2PolyEdgeMap.end())
		{
			Edge faceEdgeR = faceEdge.reversed();
			itF2P = fFaceEdge2PolyEdgeMap.find( faceEdgeR );
			if(itF2P == fFaceEdge2PolyEdgeMap.end())
				return false;
		}

		const Edge& polyEdge = itF2P->second;

		PolyEdge2FaceEdgeMap::const_iterator itP2F = fPolyEdge2FaceEdgesMap.find( polyEdge );
		if(itP2F == fPolyEdge2FaceEdgesMap.end())
			return false;

		const Edge& faceEdge0 = itP2F->second.first;
		const Edge& faceEdge1 = itP2F->second.second;

		bool foundMatch = false;
		if(faceEdge.isEqual(faceEdge0))
		{
			adjacentEdge = faceEdge1;
			foundMatch = true;
		}
		else if(faceEdge.isReverse(faceEdge0))
		{
			adjacentEdge = faceEdge1.reversed();
			foundMatch = true;
		}
		else if(faceEdge.isEqual(faceEdge1))
		{
			adjacentEdge = faceEdge0;
			foundMatch = true;
		}
		else if(faceEdge.isReverse(faceEdge1))
		{
			adjacentEdge = faceEdge0.reversed();
			foundMatch = true;
		}

		return foundMatch;
	}

	bool EdgeMapping::dominantEdge(const Edge& faceEdge, Edge& dominantEdge) const
	{
		bool returnReversed = true;
		FaceEdge2PolyEdgeMap::const_iterator itF2P = fFaceEdge2PolyEdgeMap.find( faceEdge );
		if(itF2P == fFaceEdge2PolyEdgeMap.end())
		{
			Edge faceEdgeR = faceEdge.reversed();
			itF2P = fFaceEdge2PolyEdgeMap.find( faceEdgeR );
			if(itF2P == fFaceEdge2PolyEdgeMap.end())
				return false;
			returnReversed = false;
		}

		const Edge& polyEdge = itF2P->second;

		PolyEdge2FaceEdgeMap::const_iterator itP2F = fPolyEdge2FaceEdgesMap.find( polyEdge );
		if(itP2F == fPolyEdge2FaceEdgesMap.end())
			return false;

		const Edge& faceEdge0 = itP2F->second.first;
		const Edge& faceEdge1 = itP2F->second.second;

		if(faceEdge0 < faceEdge1)
		{
			dominantEdge = (returnReversed ? faceEdge0.reversed() : faceEdge0);
		}
		else
		{
			dominantEdge = (returnReversed ? faceEdge1.reversed() : faceEdge1);
		}

		return true;
	}

	bool EdgeMapping::dominantPosition(unsigned int faceVertexId, unsigned int& dominantVertexId) const
	{
		FaceVertex2PolyVertexMap::const_iterator itF2P = fFaceVertex2PolyVertexMap.find(faceVertexId);
		if(itF2P == fFaceVertex2PolyVertexMap.end())
			return false;

		unsigned polyVertexId = itF2P->second;

		PolyVertex2FaceVertexUVMap::const_iterator itP2FUV = fPolyVertex2FaceVertexUVMap.find(polyVertexId);
		if(itP2FUV == fPolyVertex2FaceVertexUVMap.end())
			return false;

		dominantVertexId = itP2FUV->second.first;
		return true;
	}

	struct VertexF
	{
		static const float kTolerance;
		VertexF(const float* buffer, unsigned int index)
		{
			unsigned int bufferPos = index * 3;
			x = buffer[bufferPos++];
			y = buffer[bufferPos++];
			z = buffer[bufferPos++];
		}

		bool isEqual(const VertexF &rhs) const
		{
			return (fabs(x - rhs.x) < kTolerance && fabs(y - rhs.y) < kTolerance && fabs(z - rhs.z) < kTolerance);
		}

		float x, y, z;
	};

	const float VertexF::kTolerance = 1e-5f;

	bool operator< (const VertexF& lhs, const VertexF& rhs)
	{
		return ((lhs.x - rhs.x) < -VertexF::kTolerance) || 
			   (fabs(lhs.x - rhs.x) < VertexF::kTolerance && (lhs.y - rhs.y) < -VertexF::kTolerance) || 
			   (fabs(lhs.x - rhs.x) < VertexF::kTolerance && fabs(lhs.y - rhs.y) < VertexF::kTolerance && (lhs.z - rhs.z) < -VertexF::kTolerance);
	}

	struct VertexFMap
	{
		unsigned int getVertexId( const VertexF& v );

		typedef std::map<VertexF, unsigned int> TVtxMap;
		TVtxMap vertexMap;
	};

	unsigned int VertexFMap::getVertexId( const VertexF& v )
	{
		VertexFMap::TVtxMap::const_iterator itVtx = vertexMap.find(v);
		if (itVtx != vertexMap.end())
			return itVtx->second;
		unsigned int nextId = (unsigned int)vertexMap.size();
		vertexMap.insert(TVtxMap::value_type(v,nextId));
		return nextId;
	}
}

// Mode 1 : PN Triangles; no divergent normals and no displacement crack fix
// Mode 2 : PN AEN, divergent normals crack fix; no displacement UV seam crack fix
// Mode 3 : PN AEN, crack fix for divergent normals and UV seam displacement


CrackFreePrimitiveGenerator::CrackFreePrimitiveGenerator(bool addAdjacentEdges, bool addDominantEdges, bool addDominantPosition)
: fAddAdjacentEdges(addAdjacentEdges)
, fAddDominantEdges(addDominantEdges)
, fAddDominantPosition(addDominantPosition)
{
}

CrackFreePrimitiveGenerator::~CrackFreePrimitiveGenerator() {}

unsigned int CrackFreePrimitiveGenerator::computeTriangleSize(bool bAddAdjacentEdges,
						bool bAddDominantEdges,
						bool bAddDominantPosition)
{
	return 3								/* triangles vertices */
		+ (bAddAdjacentEdges ? 3 * 2 : 0)	/* adjacent edges */
		+ (bAddDominantEdges ? 3 * 2 : 0)	/* dominant edges */
		+ (bAddDominantPosition ? 3 : 0);	/* dominant position */
}

void CrackFreePrimitiveGenerator::mutateIndexBuffer( const MUintArray& originalBufferIndices, 
						const float* positionBufferFloat, 
						const float* uvBufferFloat, 
						bool bAddAdjacentEdges,
						bool bAddDominantEdges,
						bool bAddDominantPosition,
						MHWRender::MGeometry::DataType indexBufferDataType,
						void* indexData )
{
	unsigned int numTriVerts = originalBufferIndices.length();

	EdgeMapping edges;
	{
		VertexFMap vertexMap;

		// Iterate all triangles found in the old index buffer:
		unsigned int vertexIndex = 0;

		while (vertexIndex < numTriVerts)
		{
			unsigned int faceVertexId0 = originalBufferIndices[vertexIndex++];
			unsigned int polyVertexId0 = vertexMap.getVertexId(VertexF(positionBufferFloat, faceVertexId0));

			unsigned int faceVertexId1 = originalBufferIndices[vertexIndex++];
			unsigned int polyVertexId1 = vertexMap.getVertexId(VertexF(positionBufferFloat, faceVertexId1));

			unsigned int faceVertexId2 = originalBufferIndices[vertexIndex++];
			unsigned int polyVertexId2 = vertexMap.getVertexId(VertexF(positionBufferFloat, faceVertexId2));

			edges.addTriangle(faceVertexId0, faceVertexId1, faceVertexId2, polyVertexId0, polyVertexId1, polyVertexId2);

			if(bAddDominantPosition && uvBufferFloat)
			{
				unsigned int uvIndex;
				uvIndex = faceVertexId0 * 2;
				edges.addPositionUV(faceVertexId0, polyVertexId0, uvBufferFloat[uvIndex], uvBufferFloat[uvIndex+1]);

				uvIndex = faceVertexId1 * 2;
				edges.addPositionUV(faceVertexId1, polyVertexId1, uvBufferFloat[uvIndex], uvBufferFloat[uvIndex+1]);

				uvIndex = faceVertexId2 * 2;
				edges.addPositionUV(faceVertexId2, polyVertexId2, uvBufferFloat[uvIndex], uvBufferFloat[uvIndex+1]);
			}
		}
	}

	unsigned int newTriId = 0;
	for(unsigned int triId = 0; triId < numTriVerts; )
	{
		unsigned int vertexId0 = originalBufferIndices[triId++];
		unsigned int vertexId1 = originalBufferIndices[triId++];
		unsigned int vertexId2 = originalBufferIndices[triId++];

		if (indexBufferDataType == MHWRender::MGeometry::kUnsignedInt32) {
			// Triangle vertices
			((unsigned int*)indexData)[newTriId++] = vertexId0;
			((unsigned int*)indexData)[newTriId++] = vertexId1;
			((unsigned int*)indexData)[newTriId++] = vertexId2;

			// Adjacent edges
			if(bAddAdjacentEdges)
			{
				Edge adjacentEdge;

				// Edge0 : vertexId0 - vertexId1
				edges.adjacentEdge(Edge(vertexId0, vertexId1), adjacentEdge);
				((unsigned int*)indexData)[newTriId++] = adjacentEdge.v0;
				((unsigned int*)indexData)[newTriId++] = adjacentEdge.v1;

				// Edge1 : vertexId1 - vertexId2
				edges.adjacentEdge(Edge(vertexId1, vertexId2), adjacentEdge);
				((unsigned int*)indexData)[newTriId++] = adjacentEdge.v0;
				((unsigned int*)indexData)[newTriId++] = adjacentEdge.v1;

				// Edge2 : vertexId2 - vertexId0
				edges.adjacentEdge(Edge(vertexId2, vertexId0), adjacentEdge);
				((unsigned int*)indexData)[newTriId++] = adjacentEdge.v0;
				((unsigned int*)indexData)[newTriId++] = adjacentEdge.v1;
			}

			// Dominant edges
			if(bAddDominantEdges)
			{
				Edge dominantEdge;

				// Edge0 : vertexId0 - vertexId1
				edges.dominantEdge(Edge(vertexId0, vertexId1), dominantEdge);
				((unsigned int*)indexData)[newTriId++] = dominantEdge.v0;
				((unsigned int*)indexData)[newTriId++] = dominantEdge.v1;

				// Edge1 : vertexId1 - vertexId2
				edges.dominantEdge(Edge(vertexId1, vertexId2), dominantEdge);
				((unsigned int*)indexData)[newTriId++] = dominantEdge.v0;
				((unsigned int*)indexData)[newTriId++] = dominantEdge.v1;

				// Edge2 : vertexId2 - vertexId0
				edges.dominantEdge(Edge(vertexId2, vertexId0), dominantEdge);
				((unsigned int*)indexData)[newTriId++] = dominantEdge.v0;
				((unsigned int*)indexData)[newTriId++] = dominantEdge.v1;
			}

			// Dominant position
			if(bAddDominantPosition)
			{
				unsigned int dominantVertexId;

				edges.dominantPosition(vertexId0, dominantVertexId);
				((unsigned int*)indexData)[newTriId++] = dominantVertexId;

				edges.dominantPosition(vertexId1, dominantVertexId);
				((unsigned int*)indexData)[newTriId++] = dominantVertexId;

				edges.dominantPosition(vertexId2, dominantVertexId);
				((unsigned int*)indexData)[newTriId++] = dominantVertexId;
			}
		}
		else if (indexBufferDataType == MHWRender::MGeometry::kUnsignedChar) {
			((unsigned short*)indexData)[newTriId++] = (unsigned short)vertexId0;
			((unsigned short*)indexData)[newTriId++] = (unsigned short)vertexId1;
			((unsigned short*)indexData)[newTriId++] = (unsigned short)vertexId2;

			// Adjacent edges
			if(bAddAdjacentEdges)
			{
				Edge adjacentEdge;

				// Edge0 : vertexId0 - vertexId1
				edges.adjacentEdge(Edge(vertexId0, vertexId1), adjacentEdge);
				((unsigned short*)indexData)[newTriId++] = (unsigned short)adjacentEdge.v0;
				((unsigned short*)indexData)[newTriId++] = (unsigned short)adjacentEdge.v1;

				// Edge1 : vertexId1 - vertexId2
				edges.adjacentEdge(Edge(vertexId1, vertexId2), adjacentEdge);
				((unsigned short*)indexData)[newTriId++] = (unsigned short)adjacentEdge.v0;
				((unsigned short*)indexData)[newTriId++] = (unsigned short)adjacentEdge.v1;

				// Edge2 : vertexId2 - vertexId0
				edges.adjacentEdge(Edge(vertexId2, vertexId0), adjacentEdge);
				((unsigned short*)indexData)[newTriId++] = (unsigned short)adjacentEdge.v0;
				((unsigned short*)indexData)[newTriId++] = (unsigned short)adjacentEdge.v1;
			}

			// Dominant edges
			if(bAddDominantEdges)
			{
				Edge dominantEdge;

				// Edge0 : vertexId0 - vertexId1
				edges.dominantEdge(Edge(vertexId0, vertexId1), dominantEdge);
				((unsigned short*)indexData)[newTriId++] = (unsigned short)dominantEdge.v0;
				((unsigned short*)indexData)[newTriId++] = (unsigned short)dominantEdge.v1;

				// Edge1 : vertexId1 - vertexId2
				edges.dominantEdge(Edge(vertexId1, vertexId2), dominantEdge);
				((unsigned short*)indexData)[newTriId++] = (unsigned short)dominantEdge.v0;
				((unsigned short*)indexData)[newTriId++] = (unsigned short)dominantEdge.v1;

				// Edge2 : vertexId2 - vertexId0
				edges.dominantEdge(Edge(vertexId2, vertexId0), dominantEdge);
				((unsigned short*)indexData)[newTriId++] = (unsigned short)dominantEdge.v0;
				((unsigned short*)indexData)[newTriId++] = (unsigned short)dominantEdge.v1;
			}

			// Dominant position
			if(bAddDominantPosition)
			{
				unsigned int dominantVertexId;

				edges.dominantPosition(vertexId0, dominantVertexId);
				((unsigned short*)indexData)[newTriId++] = (unsigned short)dominantVertexId;

				edges.dominantPosition(vertexId1, dominantVertexId);
				((unsigned short*)indexData)[newTriId++] = (unsigned short)dominantVertexId;

				edges.dominantPosition(vertexId2, dominantVertexId);
				((unsigned short*)indexData)[newTriId++] = (unsigned short)dominantVertexId;
			}
		}
	}
}


MHWRender::MGeometry::Primitive CrackFreePrimitiveGenerator::mutateIndexing(const MHWRender::MComponentDataIndexingList& sourceIndexBuffers, 
		const MHWRender::MVertexBufferArray& vertexBuffers,
		MHWRender::MIndexBuffer& indexBuffer,
		int& primitiveStride) const
{
	MHWRender::MVertexBuffer *positionBuffer = NULL;
	MHWRender::MVertexBuffer *uvBuffer = NULL;
	
	for (unsigned int ivb = 0; ivb < vertexBuffers.count() && (positionBuffer == NULL || uvBuffer == NULL); ++ivb)
	{
		MHWRender::MVertexBuffer *currBuffer = vertexBuffers.getBuffer(ivb);

		if (positionBuffer == NULL && currBuffer->descriptor().semantic() == MHWRender::MGeometry::kPosition)
			positionBuffer = currBuffer;

		if (uvBuffer == NULL && currBuffer->descriptor().semantic() == MHWRender::MGeometry::kTexture)
			uvBuffer = currBuffer;
	}

	if (positionBuffer == NULL)
		// We need at least the positions:
		return MHWRender::MGeometry::kInvalidPrimitive;

	float* positionBufferFloat = (float*)positionBuffer->map();
	float* uvBufferFloat = NULL;
	if (uvBuffer)
		uvBufferFloat = (float*)uvBuffer->map();

    for (int x = 0; x < sourceIndexBuffers.length(); ++x)
    {
        if (sourceIndexBuffers[x]->componentType() != MHWRender::MComponentDataIndexing::kFaceVertex)
			continue;

		const MUintArray& originalBufferIndices = sourceIndexBuffers[x]->indices();
		unsigned int numTriVerts = originalBufferIndices.length();

		unsigned int numTri = numTriVerts / 3;
		unsigned int triSize = computeTriangleSize(fAddAdjacentEdges, fAddDominantEdges, fAddDominantPosition);
		unsigned int bufferSize = numTri * triSize;

		void* indexData = indexBuffer.acquire(bufferSize, true /*writeOnly - we don't need the current buffer values*/);
		if (indexData != NULL)
		{
			mutateIndexBuffer( originalBufferIndices, positionBufferFloat, uvBufferFloat, 
							   fAddAdjacentEdges, fAddDominantEdges, fAddDominantPosition,
							   indexBuffer.dataType(), indexData );
		}

		if (positionBuffer) positionBuffer->unmap();
		if (uvBuffer) uvBuffer->unmap();
		indexBuffer.commit(indexData);
		primitiveStride = triSize;
		return MHWRender::MGeometry::kPatch;
	}

	if (positionBuffer) positionBuffer->unmap();
	if (uvBuffer) uvBuffer->unmap();
	return MHWRender::MGeometry::kInvalidPrimitive;
}

// This is the primitive generator creation function registered with the DrawRegistry.
// Used to initialize a custom primitive generator.
MHWRender::MPxIndexBufferMutator* CrackFreePrimitiveGenerator::createCrackFreePrimitiveGenerator18()
{
    return new CrackFreePrimitiveGenerator(true /*addAdjacentEdges*/, true /*addDominantEdges*/, true /*addDominantPosition*/);
}

MHWRender::MPxIndexBufferMutator* CrackFreePrimitiveGenerator::createCrackFreePrimitiveGenerator9()
{
    return new CrackFreePrimitiveGenerator(true /*addAdjacentEdges*/, false /*addDominantEdges*/, false /*addDominantPosition*/);
}
