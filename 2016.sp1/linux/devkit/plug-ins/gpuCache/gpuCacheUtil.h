#ifndef _gpuCacheUtil_h_
#define _gpuCacheUtil_h_

//-
//**************************************************************************/
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk 
// license agreement provided at the time of installation or download, 
// or which otherwise accompanies this software in either electronic 
// or hard copy form.
//**************************************************************************/
//+

// Includes
#include <cassert>

#include <maya/MPlugArray.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnMesh.h>
#include <maya/MPlug.h>
#include <maya/MFloatArray.h>
#include <maya/MFloatVector.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatPoint.h>
#include <maya/MFloatPointArray.h>
#include <maya/MGlobal.h>
#include <maya/MStringResource.h>

#include <boost/shared_array.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/foreach.hpp>
#include <map>

#include "gpuCacheSample.h"
#include "gpuCacheGeometry.h"
#include "gpuCacheMaterialNodes.h"

#ifdef _DEBUG
    #define MStatAssert(status) assert(status)
#else
    #define MStatAssert(status) ((void)status)
#endif


namespace GPUCache {


// Check the visibility of a shape node
// including visibility plug and display layer
class ShapeVisibilityChecker
{
public:
    ShapeVisibilityChecker(const MObject& shapeNode)
        : fShape(shapeNode)
    {}

    bool isVisible()
    {
        // Check visibility plug
        MPlug visibilityPlug = fShape.findPlug("visibility");
        assert(!visibilityPlug.isNull());

        if (!visibilityPlug.asBool()) {
            return false;
        }

        // Check display layer
        MPlug drawOverridePlug = fShape.findPlug("drawOverride");
        assert(!drawOverridePlug.isNull());

        MPlugArray displayLayers;
        drawOverridePlug.connectedTo(displayLayers, true, false);

        for (unsigned int i = 0; i < displayLayers.length(); i++) {
            MObject displayLayerNode = displayLayers[i].node();

            if (displayLayerNode.hasFn(MFn::kDisplayLayer)) {
                // Found a display layer
                MFnDependencyNode displayLayer(displayLayerNode);
                visibilityPlug = displayLayer.findPlug("visibility");

                assert(!visibilityPlug.isNull());
                if (!visibilityPlug.asBool()) {
                    return false;
                }
            }
        }

        return true;
    }

private:
    // Forbidden and not implemented.
    ShapeVisibilityChecker(const ShapeVisibilityChecker&);
    const ShapeVisibilityChecker& operator=(const ShapeVisibilityChecker&);

    MFnDagNode fShape;
};


// This class is used for generating wireframe indices.
//
template<class INDEX_TYPE>
class WireIndicesGenerator
{
public:
    typedef INDEX_TYPE index_type;

    WireIndicesGenerator(size_t numFaceCounts, const unsigned int* faceCounts,
                         size_t numFaceIndices, const index_type* faceIndices,
                         const index_type* mappedFaceIndices)
      : fNumFaceCounts(numFaceCounts), fFaceCounts(faceCounts),
        fNumFaceIndices(numFaceIndices), fFaceIndices(faceIndices),
        fMappedFaceIndices(mappedFaceIndices),
        fNumWires(0)
    {}

    void compute()
    {
        if (fNumFaceCounts == 0 || fNumFaceIndices == 0) {
            return;
        }

        // pre-allocate buffers for the worst case
        size_t maxNumWires = fNumFaceIndices;
        boost::unordered_set<WirePair, typename WirePair::Hash, typename WirePair::EqualTo>
                wireSet(size_t(maxNumWires / 0.75f));

        // insert all wires to the set
        size_t polyIndex = 0;
        size_t endOfPoly = fFaceCounts[polyIndex];
        for (size_t i = 0; i < fNumFaceIndices; i++) {
            // find the two vertices of the wireframe
            // v1 and v2 (face indices before splitting vertices) are hashed to
            // remove duplicated wireframe lines.
            // mappedV1 and mappedV2 are the actual indices to remapped
            // positions/normals/UVs
            index_type v1, v2, mappedV1, mappedV2;
            v1       = fFaceIndices[i];
            mappedV1 = fMappedFaceIndices[i];

            size_t v2Index;
            if (i + 1 == endOfPoly) {
                // wrap at the end of the polygon
                v2Index = i + 1 - fFaceCounts[polyIndex];
                if (++polyIndex < fNumFaceCounts) {
                    endOfPoly += fFaceCounts[polyIndex];
                }
            }
            else {
                v2Index = i + 1;
            }

            v2       = fFaceIndices[v2Index];
            mappedV2 = fMappedFaceIndices[v2Index];

            // insert
            wireSet.insert(WirePair(v1, v2, mappedV1, mappedV2));
        }

        // the number of wireframes
        size_t numWires = wireSet.size();

        // allocate buffers for wireframe indices
        boost::shared_array<index_type> wireIndices(new index_type[numWires * 2]);
        size_t wireCount = 0;
        BOOST_FOREACH(const WirePair& pair, wireSet) {
            wireIndices[wireCount * 2 + 0] = pair.fMappedV1;
            wireIndices[wireCount * 2 + 1] = pair.fMappedV2;
            wireCount++;
        }

        fNumWires    = numWires;
        fWireIndices = wireIndices;
    }

    size_t                           numWires()    { return fNumWires; }
    boost::shared_array<index_type>& wireIndices() { return fWireIndices; }

private:
    // Prohibited and not implemented.
    WireIndicesGenerator(const WireIndicesGenerator&);
    const WireIndicesGenerator& operator= (const WireIndicesGenerator&);

    // This class represents an unordered pair for wireframe indices
    struct WirePair
    {
        WirePair(index_type v1, index_type v2, 
                 index_type mappedV1, index_type mappedV2)
            : fV1(v1), fV2(v2), fMappedV1(mappedV1), fMappedV2(mappedV2)
        {}

        struct Hash : std::unary_function<WirePair, std::size_t>
        {
            std::size_t operator()(const WirePair& pair) const
            {
                std::size_t seed = 0;
                if (pair.fV1 < pair.fV2) {
                    boost::hash_combine(seed, pair.fV1);
                    boost::hash_combine(seed, pair.fV2);
                }
                else {
                    boost::hash_combine(seed, pair.fV2);
                    boost::hash_combine(seed, pair.fV1);
                }
                return seed;
            }
        };

        struct EqualTo : std::binary_function<WirePair, WirePair, bool>
        {
            bool operator()(const WirePair& x, const WirePair& y) const
            {
                if (x.fV1 < x.fV2) {
                    if (y.fV1 < y.fV2) {
                        return (x.fV1 == y.fV1 && x.fV2 == y.fV2);
                    }
                    else {
                        return (x.fV1 == y.fV2 && x.fV2 == y.fV1);
                    }
                }
                else {
                    if (y.fV1 < y.fV2) {
                        return (x.fV2 == y.fV1 && x.fV1 == y.fV2);
                    }
                    else {
                        return (x.fV2 == y.fV2 && x.fV1 == y.fV1);
                    }
                }
            }
        };

        index_type fV1, fV2;
        index_type fMappedV1, fMappedV2;
    };

    // Input
    size_t              fNumFaceCounts;
    const unsigned int* fFaceCounts;

    size_t              fNumFaceIndices;
    const index_type*   fFaceIndices;
    const index_type*   fMappedFaceIndices;

    // Output
    size_t                          fNumWires;
    boost::shared_array<index_type> fWireIndices;
};


// This class converts multi-indexed streams to single-indexed streams.
//
template<class INDEX_TYPE, size_t MAX_NUM_STREAMS = 16>
class MultiIndexedStreamsConverter
{
public:
    typedef INDEX_TYPE index_type;

    MultiIndexedStreamsConverter(size_t numFaceIndices, const index_type* faceIndices)
        : fNumFaceIndices(numFaceIndices), fFaceIndices(faceIndices), fNumStreams(0),
          fNumVertices(0)
    {
        // position stream
        addMultiIndexedStream(faceIndices);
    }

    void addMultiIndexedStream(const index_type* indices)
    {
        // indices can be NULL, sequence 0,1,2,3,4,5... is assumed
        fStreams[fNumStreams++] = indices;
        assert(fNumStreams <= MAX_NUM_STREAMS);
    }

    void compute()
    {
        // pre-allocate buffers for the worst case
        boost::shared_array<index_type> indicesRegion(
            new index_type[fNumStreams * fNumFaceIndices]);

        // the hash map to find unique combination of multi-indices
        typedef boost::unordered_map<IndexTuple,size_t,typename IndexTuple::Hash,typename IndexTuple::EqualTo> IndicesMap;
        IndicesMap indicesMap(size_t(fNumFaceIndices / 0.75f));

        // fill the hash map with multi-indices
        size_t vertexAttribIndex = 0;  // index to the remapped vertex attribs
        boost::shared_array<index_type> mappedFaceIndices(new index_type[fNumFaceIndices]);

        for (size_t i = 0; i < fNumFaceIndices; i++) {
            // find the location in the region to copy multi-indices
            index_type* indices = &indicesRegion[i * fNumStreams];

            // make a tuple consists of indices for positions, normals, UVs..
            for (unsigned int j = 0; j < fNumStreams; j++) {
                indices[j] = fStreams[j] ? fStreams[j][i] : (index_type)i;
            }

            // try to insert the multi-indices tuple to the hash map
            IndexTuple tuple(indices, fNumStreams, (unsigned int)i);
            std::pair<typename IndicesMap::iterator,bool> ret = indicesMap.insert(std::make_pair(tuple, 0));

            if (ret.second) {
                // a success insert, allocate a vertex attrib index to the multi-index combination
                ret.first->second = vertexAttribIndex++;
            }

            // remap face indices
            mappedFaceIndices[i] = (index_type)ret.first->second;
        }

        // the number of unique combination is the size of vertex attrib arrays
        size_t numVertex = vertexAttribIndex;
        assert(vertexAttribIndex == indicesMap.size());

        // allocate memory for the indices into faceIndices
        boost::shared_array<unsigned int> vertAttribsIndices(new unsigned int[numVertex]);

        // build the indices (how the new vertex maps to the poly vert)
        BOOST_FOREACH(const typename IndicesMap::value_type& pair, indicesMap) {
            vertAttribsIndices[pair.second] = pair.first.faceIndex();
        }

        fMappedFaceIndices  = mappedFaceIndices;
        fVertAttribsIndices = vertAttribsIndices;
        fNumVertices        = numVertex;
    }

    unsigned int                       numStreams()         { return fNumStreams; }
    size_t                             numVertices()        { return fNumVertices; }
    boost::shared_array<unsigned int>& vertAttribsIndices() { return fVertAttribsIndices; }
    boost::shared_array<index_type>&   mappedFaceIndices()  { return fMappedFaceIndices; }

private:
    // Prohibited and not implemented.
    MultiIndexedStreamsConverter(const MultiIndexedStreamsConverter&);
    const MultiIndexedStreamsConverter& operator= (const MultiIndexedStreamsConverter&);

    // This class represents a multi-index combination
    class IndexTuple
    {
    public:
        IndexTuple(index_type* indices, unsigned int size, unsigned int faceIndex)
            : fIndices(indices), fSize(size), fFaceIndex(faceIndex)
        {}

        const index_type& operator[](unsigned int index) const
        {
            assert(index < fSize);
            return fIndices[index];
        }

        unsigned int faceIndex() const 
        {
            return fFaceIndex;
        }

        struct Hash : std::unary_function<IndexTuple, std::size_t>
        {
            std::size_t operator()(const IndexTuple& tuple) const
            {
                std::size_t seed = 0;
                for (unsigned int i = 0; i < tuple.fSize; i++) {
                    boost::hash_combine(seed, tuple.fIndices[i]);
                }
                return seed;
            }
        };

        struct EqualTo : std::binary_function<IndexTuple, IndexTuple, bool>
        {
            bool operator()(const IndexTuple& x, const IndexTuple& y) const
            {
                if (x.fSize == y.fSize) {
                    return memcmp(x.fIndices, y.fIndices, sizeof(index_type) * x.fSize) == 0;
                }
                return false;
            }
        };

    private:
        index_type*  fIndices;
        unsigned int fFaceIndex;
        unsigned int fSize;
    };

    // Input
    size_t            fNumFaceIndices;       
    const index_type* fFaceIndices;

    const index_type* fStreams[MAX_NUM_STREAMS];
    unsigned int      fNumStreams;

    // Output
    size_t                            fNumVertices;
    boost::shared_array<unsigned int> fVertAttribsIndices;
    boost::shared_array<index_type>   fMappedFaceIndices;
};

// This class drops indices for a vertex attrib
template<class INDEX_TYPE, size_t SIZE>
class IndicesDropper
{
public:
    typedef INDEX_TYPE index_type;

    IndicesDropper(const float* attribArray, const index_type* indexArray, size_t numVerts)
    {
        // map the indexed array to direct array
        boost::shared_array<float> mappedAttribs(new float[numVerts * SIZE]);
        for (size_t i = 0; i < numVerts; i++) {
            for (size_t j = 0; j < SIZE; j++) {
                mappedAttribs[i * SIZE + j] = attribArray[indexArray[i] * SIZE + j];
            }
        }

        fMappedAttribs = mappedAttribs;
    }

    boost::shared_array<float>& mappedAttribs() { return fMappedAttribs; }

private:
    // Prohibited and not implemented.
    IndicesDropper(const IndicesDropper&);
    const IndicesDropper& operator= (const IndicesDropper&);

    boost::shared_array<float> fMappedAttribs;
};


// This class remaps multi-indexed vertex attribs (drop indices).
//
template<class INDEX_TYPE, size_t MAX_NUM_STREAMS = 16>
class MultiIndexedStreamsRemapper
{
public:
    typedef INDEX_TYPE index_type;

    MultiIndexedStreamsRemapper(const index_type* faceIndices,
            size_t numNewVertices, const unsigned int* vertAttribsIndices)
        : fFaceIndices(faceIndices), fNumNewVertices(numNewVertices),
          fVertAttribsIndices(vertAttribsIndices), fNumStreams(0)
    {}

    void addMultiIndexedStream(const float* attribs, const index_type* indices, bool faceVarying, int stride)
    {
        fAttribs[fNumStreams]     = attribs;
        fIndices[fNumStreams]     = indices;
        fFaceVarying[fNumStreams] = faceVarying;
        fStride[fNumStreams]      = stride;
        fNumStreams++;
    }

    void compute()
    {
        // remap vertex attribs
        for (unsigned int i = 0; i < fNumStreams; i++) {
            const float*      attribs     = fAttribs[i];
            const index_type* indices     = fIndices[i];
            bool              faceVarying = fFaceVarying[i];
            int               stride      = fStride[i];

            // allocate memory for remapped vertex attrib arrays
            boost::shared_array<float> mappedVertAttrib(
                new float[fNumNewVertices * stride]);

            for (size_t j = 0; j < fNumNewVertices; j++) {
                // new j-th vertices maps to polyVertIndex-th poly vert
                unsigned int polyVertIndex = fVertAttribsIndices[j];

                // if the scope is varying/vertex, need to convert poly vert
                // index to vertex index
                index_type pointOrPolyVertIndex = faceVarying ?
                            polyVertIndex : fFaceIndices[polyVertIndex];

                // look up the vertex attrib index
                index_type attribIndex = indices ?
                            indices[pointOrPolyVertIndex] : pointOrPolyVertIndex;

                if (stride == 3) {
                    mappedVertAttrib[j * 3 + 0] = attribs[attribIndex * 3 + 0];
                    mappedVertAttrib[j * 3 + 1] = attribs[attribIndex * 3 + 1];
                    mappedVertAttrib[j * 3 + 2] = attribs[attribIndex * 3 + 2];
                }
                else if (stride == 2) {
                    mappedVertAttrib[j * 2 + 0] = attribs[attribIndex * 2 + 0];
                    mappedVertAttrib[j * 2 + 1] = attribs[attribIndex * 2 + 1];
                }
                else {
                    assert(0);
                }
            }

            fMappedVertAttribs[i] = mappedVertAttrib;
        }
    }

    boost::shared_array<float>& mappedVertAttribs(unsigned int index)
    {
        assert(index < fNumStreams);
        return fMappedVertAttribs[index];
    }

private:
    // Prohibited and not implemented.
    MultiIndexedStreamsRemapper(const MultiIndexedStreamsRemapper&);
    const MultiIndexedStreamsRemapper& operator= (const MultiIndexedStreamsRemapper&);

    // Input
    const index_type*   fFaceIndices;
    size_t              fNumNewVertices;
    const unsigned int* fVertAttribsIndices;

    const float*      fAttribs[MAX_NUM_STREAMS];
    const index_type* fIndices[MAX_NUM_STREAMS];
    bool              fFaceVarying[MAX_NUM_STREAMS];
    int               fStride[MAX_NUM_STREAMS];
    unsigned int      fNumStreams;

    // Output, NULL means no change
    boost::shared_array<float> fMappedVertAttribs[MAX_NUM_STREAMS];
};


// This class triangulates polygons.
//
template<class INDEX_TYPE>
class PolyTriangulator
{
public:
    typedef INDEX_TYPE index_type;

    PolyTriangulator(size_t numFaceCounts, const unsigned int* faceCounts,
                     const index_type* faceIndices, bool faceIndicesCW,
                     const float* positions, const float* normals)
        : fNumFaceCounts(numFaceCounts), fFaceCounts(faceCounts),
          fFaceIndices(faceIndices), fFaceIndicesCW(faceIndicesCW),
          fPositions(positions), fNormals(normals),
          fNumTriangles(0)
    {}

    void compute()
    {
        // empty mesh
        if (fNumFaceCounts == 0) {
            return;
        }

        // scan the polygons to estimate the buffer size
        size_t maxPoints      = 0;  // the max number of vertices in one polygon
        size_t totalTriangles = 0;  // the number of triangles in the mesh

        for (size_t i = 0; i < fNumFaceCounts; i++) {
            size_t numPoints = fFaceCounts[i];

            // ignore degenerate polygon
            if (numPoints < 3) {
                continue;
            }

            // max number of points in a polygon
            maxPoints = std::max(numPoints, maxPoints);

            // the number of triangles expected in the polygon
            size_t numTriangles = numPoints - 2;
            totalTriangles += numTriangles;
        }

        size_t maxTriangles = maxPoints - 2;  // the max number of triangles in a polygon

        // allocate buffers for the worst case
        boost::shared_array<index_type>     indices(new index_type[maxPoints]);
        boost::shared_array<unsigned short> triangles(new unsigned short[maxTriangles * 3]);
        boost::shared_array<float>          aPosition(new float[maxPoints * 2]);
        boost::shared_array<float>          aNormal;
        if (fNormals) {
            aNormal.reset(new float[maxPoints * 3]);
        }

        boost::shared_array<index_type> triangleIndices(new index_type[totalTriangles * 3]);

        // triangulate each polygon
        size_t triangleCount  = 0;  // the triangles
        for (size_t i = 0, polyVertOffset = 0; i < fNumFaceCounts; polyVertOffset += fFaceCounts[i], i++) {
            size_t numPoints = fFaceCounts[i];

            // ignore degenerate polygon
            if (numPoints < 3) {
                continue;
            }

            // no need to triangulate a triangle
            if (numPoints == 3) {
                if (fFaceIndicesCW) {
                    triangleIndices[triangleCount * 3 + 0] = fFaceIndices[polyVertOffset + 2];
                    triangleIndices[triangleCount * 3 + 1] = fFaceIndices[polyVertOffset + 1];
                    triangleIndices[triangleCount * 3 + 2] = fFaceIndices[polyVertOffset + 0];
                }
                else {
                    triangleIndices[triangleCount * 3 + 0] = fFaceIndices[polyVertOffset + 0];
                    triangleIndices[triangleCount * 3 + 1] = fFaceIndices[polyVertOffset + 1];
                    triangleIndices[triangleCount * 3 + 2] = fFaceIndices[polyVertOffset + 2];
                }
                triangleCount++;
                continue;
            }

            // 1) correct the polygon winding from CW to CCW
            if (fFaceIndicesCW)
            {
                for (size_t j = 0; j < numPoints; j++) {
                    size_t jCCW = numPoints - j - 1;
                    indices[j] = fFaceIndices[polyVertOffset + jCCW];
                }
            }
            else {
                for (size_t j = 0; j < numPoints; j++) {
                    indices[j] = fFaceIndices[polyVertOffset + j];
                }
            }

            // 2) compute the face normal (Newell's Method)
            MFloatVector faceNormal(0.0f, 0.0f, 0.0f);
            for (size_t j = 0; j < numPoints; j++) {
                const float* thisPoint = &fPositions[indices[j] * 3];
                const float* nextPoint = &fPositions[indices[(j + numPoints - 1) % numPoints] * 3];
                faceNormal.x += (thisPoint[1] - nextPoint[1]) * (thisPoint[2] + nextPoint[2]);
                faceNormal.y += (thisPoint[2] - nextPoint[2]) * (thisPoint[0] + nextPoint[0]);
                faceNormal.z += (thisPoint[0] - nextPoint[0]) * (thisPoint[1] + nextPoint[1]);
            }
            faceNormal.normalize();

            // 3) project the vertices to 2d plane by faceNormal
            float cosa, sina, cosb, sinb, cacb, sacb;
            sinb = -sqrtf(faceNormal[0] * faceNormal[0] + faceNormal[1] * faceNormal[1]);
            if (sinb < -1e-5) {
                cosb =  faceNormal[2];
                sina =  faceNormal[1] / sinb;
                cosa = -faceNormal[0] / sinb;

                cacb = cosa * cosb;
                sacb = sina * cosb;
            }
            else {
                cacb = 1.0f;
                sacb = 0.0f;
                sinb = 0.0f;
                sina = 0.0f;
                if (faceNormal[2] > 0.0f) {
                    cosa = 1.0f;
                    cosb = 1.0f;
                }
                else {
                    cosa = -1.0f;
                    cosb = -1.0f;
                }
            }

            for (size_t j = 0; j < numPoints; j++) {
                const float* point = &fPositions[indices[j] * 3];
                aPosition[j * 2 + 0] = cacb * point[0] - sacb * point[1] + sinb * point[2];
                aPosition[j * 2 + 1] = sina * point[0] + cosa * point[1];
            }

            // 4) copy normals
            if (fNormals) {
                for (size_t j = 0; j < numPoints; j++) {
                    aNormal[j * 3 + 0] = fNormals[indices[j] * 3 + 0];
                    aNormal[j * 3 + 1] = fNormals[indices[j] * 3 + 1];
                    aNormal[j * 3 + 2] = fNormals[indices[j] * 3 + 2];
                }
            }

            // 5) do triangulation
            int numResultTriangles = 0;
            MFnMesh::polyTriangulate(
                aPosition.get(),
                (unsigned int)numPoints,
                (unsigned int)numPoints,
                0,
                fNormals != NULL,
                aNormal.get(),
                triangles.get(),
                numResultTriangles);

            if (numResultTriangles == int(numPoints - 2)) {
                // triangulation success
                for (size_t j = 0; j < size_t(numResultTriangles); j++) {
                    triangleIndices[triangleCount * 3 + 0] = indices[triangles[j * 3 + 0]];
                    triangleIndices[triangleCount * 3 + 1] = indices[triangles[j * 3 + 1]];
                    triangleIndices[triangleCount * 3 + 2] = indices[triangles[j * 3 + 2]];
                    triangleCount++;
                }
            }
            else {
                // triangulation failure, use the default triangulation
                for (size_t j = 1; j < numPoints - 1; j++) {
                    triangleIndices[triangleCount * 3 + 0] = indices[0];
                    triangleIndices[triangleCount * 3 + 1] = indices[j];
                    triangleIndices[triangleCount * 3 + 2] = indices[j + 1];
                    triangleCount++;
                }
            }
        }

        fNumTriangles    = totalTriangles;
        fTriangleIndices = triangleIndices;
    }

    size_t numTriangles() { return fNumTriangles; }
    boost::shared_array<index_type>& triangleIndices() { return fTriangleIndices; }

private:
    // Prohibited and not implemented.
    PolyTriangulator(const PolyTriangulator&);
    const PolyTriangulator& operator= (const PolyTriangulator&);

    // Input
    size_t              fNumFaceCounts;
    const unsigned int* fFaceCounts;
    const index_type*   fFaceIndices;
    bool                fFaceIndicesCW;
    const float*        fPositions;
    const float*        fNormals;

    // Output
    size_t                          fNumTriangles;
    boost::shared_array<index_type> fTriangleIndices;
};


// This class extracts mesh information from Maya mesh
//
template<class INDEX_TYPE>
class MayaMeshExtractor
{
public:
    typedef INDEX_TYPE index_type;

    MayaMeshExtractor(const MObject& meshObj)
        : fPolyMesh(meshObj), fWantUVs(true)
    {}

    void setWantUVs(bool wantUVs) { fWantUVs = wantUVs; }

    void compute()
    {
        MStatus status;
        bool needTriangulate = false;

        // Topology
        size_t                         numFaceCounts;
        boost::shared_array<unsigned int> faceCounts;
        
        size_t                       numFaceIndices;
        boost::shared_array<index_type> faceIndices;

        {
            MIntArray mayaVertexCount, mayaVertexList;
            status = fPolyMesh.getVertices(mayaVertexCount, mayaVertexList);
            assert(status == MS::kSuccess);

            // Copy Maya arrays
            numFaceCounts             = mayaVertexCount.length();
            const int* srcVertexCount = &mayaVertexCount[0];
            faceCounts.reset(new unsigned int[numFaceCounts]);
            for (size_t i = 0; i < numFaceCounts; i++) {
                faceCounts[i] = srcVertexCount[i];
                if (faceCounts[i] != 3) needTriangulate = true;
            }

            numFaceIndices           = mayaVertexList.length();
            const int* srcVertexList = &mayaVertexList[0];
            faceIndices.reset(new index_type[numFaceIndices]);
            for (size_t i = 0; i < numFaceIndices; i++) {
                faceIndices[i] = srcVertexList[i];
            }
        }

        // Positions
        boost::shared_array<float> positions;
        {
            MFloatPointArray mayaPositions;
            status = fPolyMesh.getPoints(mayaPositions);
            assert(status == MS::kSuccess);

            // Allocate memory for positions
            unsigned int numPositions = mayaPositions.length();
            positions.reset(new float[numPositions * 3]);

            // just copy the positions to shared_array
            for (unsigned int i = 0; i < numPositions; i++) {
                MFloatPoint& point = mayaPositions[i];
                positions[i * 3 + 0] = point.x;
                positions[i * 3 + 1] = point.y;
                positions[i * 3 + 2] = point.z;
            }
        }

        // Normals
        boost::shared_array<float>      normals;
        boost::shared_array<index_type> normalIndices;
        {
            MFloatVectorArray mayaNormals;
            status = fPolyMesh.getNormals(mayaNormals);
            assert(status == MS::kSuccess);

            MIntArray mayaNormalIdCounts, mayaNormalIds;
            status = fPolyMesh.getNormalIds(mayaNormalIdCounts, mayaNormalIds);
            assert(status == MS::kSuccess);

            // Allocate memory for normals
            unsigned int numNormals   = mayaNormals.length();
            unsigned int numNormalIds = mayaNormalIds.length();
            normals.reset(new float[numNormals * 3]);
            normalIndices.reset(new index_type[numNormalIds]);

            // just copy the normals and normalIds to shared_array
            for (unsigned int i = 0; i < numNormals; i++) {
                MFloatVector& normal = mayaNormals[i];
                normals[i * 3 + 0] = normal.x;
                normals[i * 3 + 1] = normal.y;
                normals[i * 3 + 2] = normal.z;
            }
            for (unsigned int i = 0; i < numNormalIds; i++) {
                normalIndices[i] = mayaNormalIds[i];
            }
        }

        // UVs
        boost::shared_array<float>      UVs;
        boost::shared_array<index_type> uvIndices;
        if (fWantUVs) {
            MFloatArray mayaUArray, mayaVArray;
            status = fPolyMesh.getUVs(mayaUArray, mayaVArray);
            assert(status == MS::kSuccess);

            MIntArray mayaUVCounts, mayaUVIds;
            status = fPolyMesh.getAssignedUVs(mayaUVCounts, mayaUVIds);
            assert(status == MS::kSuccess);

            // Allocate memory for normals
            unsigned int numUVs   = mayaUArray.length();
            unsigned int numUVIds = mayaUVIds.length();
            if (numUVs > 0 && numUVIds > 0) {
                UVs.reset(new float[numUVs * 2]);
                uvIndices.reset(new index_type[numUVIds]);

                // just copy the UVs and uvIds to shared_array
                for (unsigned int i = 0; i < numUVs; i++) {
                    UVs[i * 2 + 0] = mayaUArray[i];
                    UVs[i * 2 + 1] = mayaVArray[i];
                }
                for (unsigned int i = 0; i < numUVIds; i++) {
                    uvIndices[i] = mayaUVIds[i];
                }
            }
        }

        // Convert multi-indexed streams
        size_t numVertices = 0;
        boost::shared_array<index_type>   mappedFaceIndices;
        boost::shared_array<unsigned int> vertAttribsIndices;
        {
            MultiIndexedStreamsConverter<index_type>
                converter(numFaceIndices, faceIndices.get());

            converter.addMultiIndexedStream(normalIndices.get());
            if (fWantUVs && uvIndices) {
                converter.addMultiIndexedStream(uvIndices.get());
            }

            converter.compute();

            numVertices        = converter.numVertices();
            mappedFaceIndices  = converter.mappedFaceIndices();
            vertAttribsIndices = converter.vertAttribsIndices();
        }

        // Remap vertex streams
        boost::shared_array<float> mappedPositions;
        boost::shared_array<float> mappedNormals;
        boost::shared_array<float> mappedUVs;
        {
            MultiIndexedStreamsRemapper<index_type>
                remapper(faceIndices.get(), numVertices, vertAttribsIndices.get());

            remapper.addMultiIndexedStream(positions.get(), NULL, false, 3);
            remapper.addMultiIndexedStream(normals.get(), normalIndices.get(), true, 3);
            if (fWantUVs && UVs && uvIndices) {
                remapper.addMultiIndexedStream(UVs.get(), uvIndices.get(), true, 2);
            }

            remapper.compute();

            mappedPositions = remapper.mappedVertAttribs(0);
            mappedNormals   = remapper.mappedVertAttribs(1);
            if (fWantUVs && UVs && uvIndices) {
                mappedUVs = remapper.mappedVertAttribs(2);
            }
        }

        // Wireframe indices
        size_t numWires = 0;
        boost::shared_array<index_type> wireIndices;
        {
            WireIndicesGenerator<index_type> wireIndicesGenerator(
                numFaceCounts,  faceCounts.get(),
                numFaceIndices, faceIndices.get(),
                mappedFaceIndices.get());

            wireIndicesGenerator.compute();

            numWires    = wireIndicesGenerator.numWires();
            wireIndices = wireIndicesGenerator.wireIndices();
        }

        // Triangle indices
        size_t numTriangles = 0;
        boost::shared_array<index_type> triangleIndices;
        if (needTriangulate) {
            PolyTriangulator<index_type> polyTriangulator(
                numFaceCounts, faceCounts.get(),
                mappedFaceIndices.get(), false,
                mappedPositions.get(), mappedNormals.get());

            polyTriangulator.compute();

            numTriangles    = polyTriangulator.numTriangles();
            triangleIndices = polyTriangulator.triangleIndices();
        }
        else {
            assert(numFaceIndices % 3 == 0);
            numTriangles    = numFaceIndices / 3;
            triangleIndices = mappedFaceIndices;
        }

        // done
        fWireIndices     = SharedArray<index_type>::create(
                wireIndices, numWires * 2);
        fTriangleIndices = SharedArray<index_type>::create(
                triangleIndices, numTriangles * 3);

        fPositions = SharedArray<float>::create(
                mappedPositions, numVertices * 3);
        fNormals   = SharedArray<float>::create(
                mappedNormals, numVertices * 3);
        if (fWantUVs && mappedUVs) {
            fUVs = SharedArray<float>::create(
                    mappedUVs, numVertices * 2);
        }
    }

    boost::shared_ptr<ReadableArray<index_type> >& triangleIndices()
    { return fTriangleIndices; }

    boost::shared_ptr<ReadableArray<index_type> >& wireIndices()
    { return fWireIndices; }

    boost::shared_ptr<ReadableArray<float> >& positions()
    { return fPositions; }

    boost::shared_ptr<ReadableArray<float> >& normals()
    { return fNormals; }

    boost::shared_ptr<ReadableArray<float> >& uvs()
    { return fUVs; }

private:
    // Prohibited and not implemented.
    MayaMeshExtractor(const MayaMeshExtractor&);
    const MayaMeshExtractor& operator= (const MayaMeshExtractor&);

    // Input
    MFnMesh fPolyMesh;
    bool    fWantUVs;

    // Output
    boost::shared_ptr<ReadableArray<index_type> > fTriangleIndices;
    boost::shared_ptr<ReadableArray<index_type> > fWireIndices;
    boost::shared_ptr<ReadableArray<float> >      fPositions;
    boost::shared_ptr<ReadableArray<float> >      fNormals;
    boost::shared_ptr<ReadableArray<float> >      fUVs;
};

// This class is used to update the TransparentPruneType of Xform sub nodes.
// Once a bounding box place holder sub node is loaded, its sample map and 
// TransparentPruneType is updated to reflect the real geometry. 
// It needs to update all the parent sub nodes' TransparentPruneType.
class SubNodeTransparentTypeVisitor : public SubNodeVisitor
{
public:
    SubNodeTransparentTypeVisitor() {}
    virtual ~SubNodeTransparentTypeVisitor() {}

    virtual void visit(const XformData& xform,
                       const SubNode&   subNode)
    {
        // The transparent type is unknown at first
        fTransparentTypes.push_back(SubNode::kUnknown);

        // Recursive into children
        BOOST_FOREACH (const SubNode::Ptr& child, subNode.getChildren()) {
            child->accept(*this);
        }

        // Update the transparent type of this xform sub node
        const_cast<SubNode&>(subNode).setTransparentType(fTransparentTypes.back());
        fTransparentTypes.pop_back();
    }

    virtual void visit(const ShapeData& shape,
                       const SubNode&   subNode)
    {
        // Update all parents' transparent type
        for (size_t i = 0; i < fTransparentTypes.size(); i++) {
            if (fTransparentTypes[i] == SubNode::kUnknown) {
                // Parent transparent type is unknown, use this type
                fTransparentTypes[i] = subNode.transparentType();
            }
            else {
                // Parent transparent type is different, use opaque and transparent
                if (fTransparentTypes[i] != subNode.transparentType()) {
                    fTransparentTypes[i] = SubNode::kOpaqueAndTransparent;
                }
            }
        }
    }

private:
    std::vector<SubNode::TransparentType> fTransparentTypes;
};

// This class returns the top-level bounding box of a sub-node hierarchy.
class BoundingBoxVisitor :  public SubNodeVisitor
{
public:
    BoundingBoxVisitor(double timeInSeconds)
      : fTimeInSeconds(timeInSeconds)
    {}
    virtual ~BoundingBoxVisitor() {}

    // Returns the current bounding box.
    const MBoundingBox& boundingBox() const
    { return fBoundingBox; }

    // Get the bounding box from a xform node.
    virtual void visit(const XformData&   xform,
                       const SubNode&     subNode)
    {
        const boost::shared_ptr<const XformSample>& sample =
            xform.getSample(fTimeInSeconds);
        if (sample) {
            fBoundingBox = sample->boundingBox();
        }
    }

    // Get the bounding box from a shape node.
    virtual void visit(const ShapeData&   shape,
                       const SubNode&     subNode)
    {
        const boost::shared_ptr<const ShapeSample>& sample =
            shape.getSample(fTimeInSeconds);
        if (sample) {
            fBoundingBox = sample->boundingBox();
        }
    }

    // Helper method to get the bounding box in one line.
    static MBoundingBox boundingBox(const SubNode::Ptr& subNode,
                                    const double        timeInSeconds)
    {
        if (subNode) {
            BoundingBoxVisitor visitor(timeInSeconds);
            subNode->accept(visitor);
            return visitor.boundingBox();
        }
        return MBoundingBox();
    }

private:
    const double fTimeInSeconds;
    MBoundingBox fBoundingBox;
};

// Extract the shape geometry paths
class ShapePathVisitor : public SubNodeVisitor
{
public:
    typedef std::pair<MString,const SubNode*> ShapePathAndSubNode;
    typedef std::vector<ShapePathAndSubNode>  ShapePathAndSubNodeList;

    ShapePathVisitor(ShapePathAndSubNodeList& shapePaths)
        : fShapePaths(shapePaths)
    {}

    virtual ~ShapePathVisitor() 
    {}

    virtual void visit(const XformData& xform,
                       const SubNode&   subNode)
    {
        // Remember this xform name
        bool isTop = subNode.getName() == "|";
        if (!isTop) fCurrentPath.push_back(subNode.getName());

        // Recursive into children
        BOOST_FOREACH (const SubNode::Ptr& child, subNode.getChildren()) {
            child->accept(*this);
        }

        if (!isTop) fCurrentPath.pop_back();
    }

    virtual void visit(const ShapeData& shape,
                       const SubNode&   subNode)
    {
        // Construct geometry path
        MString path;
        for (size_t i = 0; i < fCurrentPath.size(); i++) {
            path += "|";
            path += fCurrentPath[i];
        }
        path += "|";
        path += subNode.getName();

        fShapePaths.push_back(
            std::make_pair(path, &subNode));
    }

private:
    ShapePathAndSubNodeList& fShapePaths;
    std::vector<MString>     fCurrentPath;
};

inline bool ReplaceSubNodeData(const SubNode::Ptr& top, const SubNode::Ptr& node, const MString& path)
{
    // Split the geometry path into steps
    MStringArray steps;
    path.split('|', steps);

    // Invalid path
    if (steps.length() == 0) return false;

    // Find the first step
    SubNode::Ptr firstNode;
    if (top->getName() == "|") {
        // Dummy top node case
        BOOST_FOREACH (const SubNode::Ptr& child, top->getChildren()) {
            if (child->getName() == steps[0]) {
                firstNode = child;
                break;
            }
        }
    }
    else {
        if (top->getName() == steps[0]) {
            firstNode = top;
        }
    }

    // Can't find the first sub node
    if (!firstNode) return false;

    // Find the sub node by stepping through the path
    SubNode::Ptr current = firstNode;
    for (unsigned int i = 1; i < steps.length(); i++) {
        bool found = false;
        BOOST_FOREACH (const SubNode::Ptr& child, current->getChildren()) {
            if (child->getName() == steps[i]) {
                current = child;
                found = true;
                break;
            }
        }
        // Not found
        if (!found) return false;
    }

    // Found a sub node
    assert(current);
    assert(node);

    // Cast to mutable pointer.
    // Currently, this is the only exception that we need to change 
    // sub node (actually sub node data) outside reader.
    SubNode::MPtr mCurrent = boost::const_pointer_cast<SubNode>(current);
    SubNode::MPtr mNode    = boost::const_pointer_cast<SubNode>(node);
    SubNode::swapNodeData(mCurrent, mNode);

    return true;
}

// This function will validate a geom path given a SubNode hierarchy.
// It will return true if the given geom path was valid, false otherwise.
// Additionally, it will return the closest valid path.
inline bool ValidateGeomPath(const SubNode::Ptr& top, const MString& geomPath, MString& validatedGeomPath)
{
	if( !top )
	{
        validatedGeomPath = MString("|");
        return false;
    }

    // path: |xform1|xform2|meshShape
    MStringArray pathArray;
    geomPath.split('|', pathArray);

    bool valid = true;
        
    // find the mesh in Alembic archive
    validatedGeomPath = MString();
    SubNode::Ptr current = top;
    for( unsigned int i = 0; i < pathArray.length(); i++ )
	{
        MString step = pathArray[i];
		bool foundChild = false;
		const std::vector<SubNode::Ptr>& children = current->getChildren();
		for( unsigned int j = 0; j < children.size(); j++ )
		{
			if( children[j]->getName() == step )
			{
				current = children[j];
				foundChild = true;
			}
		}
		if( !foundChild )
		{
			valid = false;
			break;
		}
        validatedGeomPath += MString("|");
        validatedGeomPath += step;
    }

    if (validatedGeomPath.length() == 0) {
        validatedGeomPath = MString("|");
    }
    
    return valid;
}


// This function will return a SubNode hierarchy given an existing SubNode hierarchy and
// geom path. The geom path will be validated and this function will return both the
// closest valid path and the associated SubNode hierarchy.
//
// This function will return true if a valid SubNode hierarchy was returned, false otherwise.
inline bool CreateSubNodeHierarchy(const SubNode::Ptr& top, const MString& geomPath, MString& validatedGeomPath, SubNode::Ptr& out)
{
	if( !top )
	{
		return false;
	}

	// Validate the geomPath
	ValidateGeomPath( top, geomPath, validatedGeomPath );

	// path: |xform1|xform2|meshShape
    MStringArray pathArray;
    validatedGeomPath.split('|', pathArray);

	if( pathArray.length() == 0 )
	{
		// Early exit, geomPath is either empty or "|".
		out = top;
	}
	else
	{
		// We have a geom path to consider. Generate a new SubNode hierarchy.
		//
		// In this case, we must duplicate SubNodes since up until the last step along
		// the geomPath, the children will differ.
		//
		// For example, let's say we have the following scene hierarchy:
		//
		// |group1|pSphere1|pSphereShape1
		// |group1|pCube1|pCubeShape1
		//
		// The geomPath is:  |group1|pCube1
		//
		// We cannot simply reference the same SubNode::Ptr for |group1, as that
		// SubNode contains child SubNodes for pSphere1 and pCube1. Thus, we need
		// to duplicate the SubNode and only reference the children of interest.
		// 
		// We only need to duplicate the SubNode hierarchy up until the last path step.
		// For the last path step we can simply reference the same SubNode::Ptr since
		// we know that we want the same hierarchy under that node.
		//
		SubNode::MPtr copyTop = SubNode::create( top->getName(), top->getData() );
		copyTop->setTransparentType( top->transparentType() );
		SubNode::MPtr copyCurrent = copyTop;

		// Walk the hierarchy and copy data.
		SubNode::Ptr current = top;
		for( unsigned int i = 0; i < pathArray.length(); i++ )
		{
			MString step = pathArray[i];

			const std::vector<SubNode::Ptr>& children = current->getChildren();
			bool foundChild = false;
			for( unsigned int j = 0; j < children.size(); j++ )
			{
				if( children[j]->getName() == step )
				{
					current = children[j];
					foundChild = true;
					break;
				}
			}
			assert(foundChild);

			SubNode::MPtr copyChild;
			if( i+1 < pathArray.length() )
			{
				copyChild = SubNode::create( current->getName(), current->getData() );
				copyChild->setTransparentType( current->transparentType() );
			}
			else
			{
				// For the last path step, we can reuse the same SubNode::Ptr - no need to copy.
				copyChild = boost::const_pointer_cast<SubNode>(current);
			}
			SubNode::connect(copyCurrent, copyChild);
			copyCurrent = copyChild;
		}

		out = copyTop;
    }
	return true;
}

/*==============================================================================
 * CLASS InstanceMaterialLookup
 *============================================================================*/

// Find the connect shading groups and surface materials by tracking connections.
class InstanceMaterialLookup : boost::noncopyable
{
public:
    InstanceMaterialLookup(const MDagPath& dagPath);
    ~InstanceMaterialLookup();

    // Whole object material assignment.
    bool    hasWholeObjectMaterial();
    MObject findWholeObjectShadingGroup();
    MObject findWholeObjectSurfaceMaterial();

    // Per-face or Per-patch material assignment.
    bool hasComponentMaterials();
    bool findShadingGroups(std::vector<MObject>& shadingGroups);
    bool findSurfaceMaterials(std::vector<MObject>& surfaceMaterials);

private:
    // Find instObjGroups[instanceNumber] plug.
    static MPlug findInstObjGroupsPlug(const MDagPath& dagPath);
    static MObject findShadingGroupByPlug(const MPlug& srcPlug);
    static MObject findSurfaceMaterialByShadingGroup(const MObject& shadingGroup);
    static void findObjectGroupsPlug(const MPlug& iogPlug, std::vector<MPlug>& ogPlugs);

    const MPlug fInstObjGroupsPlug;
};


/*==============================================================================
 * CLASS ShadedModeColor
 *============================================================================*/

// This class evaluates the material property values of a material node.
// The value is expected to be the same as viewport's shaded mode.
class ShadedModeColor : boost::noncopyable
{
public:
    static bool evaluateBool(const MaterialProperty::Ptr& prop,
                             double                       timeInSeconds);

    static float evaluateFloat(const MaterialProperty::Ptr& prop,
                               double                       timeInSeconds);

    static MColor evaluateDefaultColor(const MaterialProperty::Ptr& prop,
                                       double                       timeInSeconds);

    static MColor evaluateColor(const MaterialProperty::Ptr& prop,
                                double                       timeInSeconds);
};


inline MString EncodeString(const MString& msg)
{
    MString encodedMsg;

    unsigned int   length = msg.numChars();
    const wchar_t* buffer = msg.asWChar();

    std::wstringstream stream;
    for (unsigned int i = 0; i < length; i++) {
        wchar_t ch = buffer[i];
        switch (ch) {
        case '\n': stream << L"\\n"; continue;
        case '\t': stream << L"\\t"; continue;
        case '\b': stream << L"\\b"; continue;
        case '\r': stream << L"\\r"; continue;
        case '\f': stream << L"\\f"; continue;
        case '\v': stream << L"\\v"; continue;
        case '\a': stream << L"\\a"; continue;
        case '\\': stream << L"\\\\"; continue;
        case '\"': stream << L"\\\""; continue;
        case '\'': stream << L"\\\'"; continue;
        }
        stream << ch;
    }

    std::wstring str = stream.str();
    return MString(str.c_str());
}

inline void DisplayError(const MString& msg)
{
    // This is the threadsafe version of MGlobal::displayError()
    MGlobal::executeCommandOnIdle("error \"" + EncodeString(msg) + "\"");
}

inline void DisplayError(const MStringResourceId& id)
{
    // Threadsafe displayError() bundled with MStringResourceId
    MStatus stat;
    MString msg = MStringResource::getString(id, stat);
    DisplayError(msg);
}

inline void DisplayError(const MStringResourceId& id,
                         const MString& arg1,
                         const MString& arg2 = MString(""),
                         const MString& arg3 = MString(""),
                         const MString& arg4 = MString(""),
                         const MString& arg5 = MString(""),
                         const MString& arg6 = MString(""),
                         const MString& arg7 = MString(""),
                         const MString& arg8 = MString(""),
                         const MString& arg9 = MString(""))
{
    // Threadsafe displayError() bundled with MStringResourceId and format
    MStatus stat;
    MString format = MStringResource::getString(id, stat);
    MString msg;
    msg.format(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    DisplayError(msg);
}

inline void DisplayWarning(const MString& msg)
{
    // This is the threadsafe version of MGlobal::displayWarning()
    MGlobal::executeCommandOnIdle("warning \"" + EncodeString(msg) + "\"");
}

inline void DisplayWarning(const MStringResourceId& id)
{
    // Threadsafe displayWarning() bundled with MStringResourceId
    MStatus stat;
    MString msg = MStringResource::getString(id, stat);
    DisplayWarning(msg);
}

inline void DisplayWarning(const MStringResourceId& id,
                           const MString& arg1,
                           const MString& arg2 = MString(""),
                           const MString& arg3 = MString(""),
                           const MString& arg4 = MString(""),
                           const MString& arg5 = MString(""),
                           const MString& arg6 = MString(""),
                           const MString& arg7 = MString(""),
                           const MString& arg8 = MString(""),
                           const MString& arg9 = MString(""))
{
    // Threadsafe displayWarning() bundled with MStringResourceId and format
    MStatus stat;
    MString format = MStringResource::getString(id, stat);
    MString msg;
    msg.format(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    DisplayWarning(msg);
}



} // namespace GPUCache

#endif

