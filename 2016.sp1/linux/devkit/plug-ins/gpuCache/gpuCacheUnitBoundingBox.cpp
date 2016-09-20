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

#include "gpuCacheUnitBoundingBox.h"


namespace GPUCache {

//==============================================================================
// CLASS UnitBoundingBox
//==============================================================================

boost::shared_ptr<const IndexBuffer>  UnitBoundingBox::fBoundingBoxIndices;
boost::shared_ptr<const VertexBuffer> UnitBoundingBox::fBoundingBoxPositions;

const MBoundingBox& UnitBoundingBox::boundingBox()
{
    static MBoundingBox sBoundingBox(
        MPoint(-1.0f, -1.0f, -1.0f),
        MPoint(1.0f, 1.0f, 1.0f));
    return sBoundingBox;
}

boost::shared_ptr<const IndexBuffer>& UnitBoundingBox::indices()
{
    // Initialize the unit bounding box buffers
    if (!fBoundingBoxIndices) {
        const IndexBuffer::index_t indices[24] = {
            0, 1, 1, 2, 2, 3, 3, 0,
            4, 5, 5, 6, 6, 7, 7, 4,
            0, 4, 1, 5, 2, 6, 3, 7
        };
        boost::shared_array<IndexBuffer::index_t> indicesArray(new IndexBuffer::index_t[24]);
        memcpy(indicesArray.get(), indices, sizeof(IndexBuffer::index_t)*24);

        fBoundingBoxIndices = IndexBuffer::create(
            SharedArray<IndexBuffer::index_t>::create(indicesArray, 24));
    }
    return fBoundingBoxIndices;
}

boost::shared_ptr<const VertexBuffer>& UnitBoundingBox::positions()
{
    if (!fBoundingBoxPositions) {
        const float positions[24] = {
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
        };
        boost::shared_array<float> positionsArray(new float[24]);
        memcpy(positionsArray.get(), positions, sizeof(float)*24);

        fBoundingBoxPositions = VertexBuffer::createPositions(
            SharedArray<float>::create(positionsArray, 24));
    }
    return fBoundingBoxPositions;
}

void UnitBoundingBox::clear()
{
    fBoundingBoxIndices.reset();
    fBoundingBoxPositions.reset();
}

MMatrix UnitBoundingBox::boundingBoxMatrix(const MBoundingBox& boundingBox)
{
    const MPoint offset = boundingBox.center();
    const MPoint scale = MPoint(boundingBox.width()  / 2.0f, 
                                boundingBox.height() / 2.0f, 
                                boundingBox.depth()  / 2.0f);

    MMatrix boundingBoxMatrix;
    boundingBoxMatrix[3][0] = offset[0];
    boundingBoxMatrix[3][1] = offset[1];
    boundingBoxMatrix[3][2] = offset[2];
    boundingBoxMatrix[0][0] = scale[0];
    boundingBoxMatrix[1][1] = scale[1];
    boundingBoxMatrix[2][2] = scale[2];
    return boundingBoxMatrix;
}

}
