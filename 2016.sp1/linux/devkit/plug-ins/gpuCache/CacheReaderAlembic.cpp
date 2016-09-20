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

#include "CacheReaderAlembic.h"
#include "CacheAlembicUtil.h"
#include "gpuCacheUtil.h"
#include "gpuCacheStrings.h"

#include <Alembic/AbcCoreFactory/IFactory.h>
#include <Alembic/AbcGeom/Visibility.h>
#include <Alembic/AbcGeom/ArchiveBounds.h>

#include <maya/MStringArray.h>
#include <maya/MAnimControl.h>
#include <maya/MString.h>
#include <maya/MGlobal.h>
#include <maya/MFnMesh.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnNurbsCurveData.h>
#include <maya/MTrimBoundaryArray.h>
#include <maya/MFloatArray.h>
#include <maya/MFloatVector.h>
#include <maya/MPointArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MUintArray.h>

#include <boost/ref.hpp>

#include <fstream>
#include <cassert>

using namespace CacheAlembicUtil;

namespace GPUCache {
namespace CacheReaderAlembicPrivate {

//==============================================================================
// CLASS AlembicArray
//==============================================================================

template <class ArrayProperty>
boost::shared_ptr<ReadableArray<typename AlembicArray<ArrayProperty>::T> >
AlembicArray<ArrayProperty>::create(
    const ArraySamplePtr& arraySamplePtr, const Digest& digest ) 
{
    const size_t size = (arraySamplePtr->size() *
                         ArrayProperty::traits_type::dataType().getExtent());
#ifndef NDEBUG
    // Compute the Murmur3 cryptographic hash-key and make sure
    // that the digest found in the Alembic file is correct.
    Digest checkDigest;
    Alembic::Util::MurmurHash3_x64_128(
        arraySamplePtr->get(), size * sizeof(T), sizeof(T), checkDigest.words);
    assert(digest == checkDigest);
#endif
        
    // We first look if a similar array already exists in the
    // cache. If so, we return the cached array to promote sharing as
    // much as possible.
    boost::shared_ptr<ReadableArray<T> > ret;
    {
        tbb::mutex::scoped_lock lock(ArrayRegistry<T>::mutex());

        // Only accept arrays which contain data we own.  This array may happen on a
        // worker thread, so non-readable arrays can't be converted to readable.
        ret = ArrayRegistry<T>::lookupReadable(digest, size);
        
        if (!ret) {
            ret = boost::make_shared<AlembicArray<ArrayProperty> >(
                arraySamplePtr, digest);
            ArrayRegistry<T>::insert(ret);
        }
    }

    return ret;
}

template <class ArrayProperty>
AlembicArray<ArrayProperty>::~AlembicArray() {}

template <class ArrayProperty>
const typename AlembicArray<ArrayProperty>::T*
AlembicArray<ArrayProperty>::get() const 
{
    return reinterpret_cast<const T*>(fArraySamplePtr->get());
}


//==============================================================================
// CLASS ArrayPropertyCacheWithConverter
//==============================================================================

template <typename PROPERTY>
typename ArrayPropertyCacheWithConverter<PROPERTY>::ConvertionMap
ArrayPropertyCacheWithConverter<PROPERTY>::fsConvertionMap;

template class ArrayPropertyCacheWithConverter<
    Alembic::Abc::IInt32ArrayProperty>;


//==============================================================================
// CLASS ScopedUnlockAlembic
//==============================================================================

class ScopedUnlockAlembic : boost::noncopyable
{
public:
    ScopedUnlockAlembic()
    {
        gsAlembicMutex.unlock();
    }

    ~ScopedUnlockAlembic()
    {
        gsAlembicMutex.lock();
    }
};

// This function is the checkpoint of the worker thread's interrupt and pause state.
void CheckInterruptAndPause(const char* state)
{
    GlobalReaderCache& readerCache = GlobalReaderCache::theCache();
    if (readerCache.isInterrupted()) {
        // Interrupted. Throw an exception to terminate this reader.
        throw CacheReaderInterruptException(state);
    }
    else if (readerCache.isPaused()) {
        // Paused. Unlock the Alembic lock and return the control.
        ScopedUnlockAlembic unlock;
        readerCache.pauseUntilNotified();
    }
}


//==============================================================================
// CLASS DataProvider
//==============================================================================

template<class INFO>
DataProvider::DataProvider(
        Alembic::AbcGeom::IGeomBaseSchema<INFO>& abcGeom,
        Alembic::Abc::TimeSamplingPtr            timeSampling,
        size_t numSamples,
        bool   needUVs)
    : fAnimTimeRange(TimeInterval::kInvalid),
      fBBoxAndVisValidityInterval(TimeInterval::kInvalid),
      fValidityInterval(TimeInterval::kInvalid),
      fNeedUVs(needUVs)
{
    Alembic::Abc::IObject shapeObject = abcGeom.getObject();

    // shape visibility
    Alembic::AbcGeom::IVisibilityProperty visibility = 
                    Alembic::AbcGeom::GetVisibilityProperty(shapeObject);
    if (visibility) {
        fVisibilityCache.init(visibility);
    }

    // bounding box
    fBoundingBoxCache.init(abcGeom.getSelfBoundsProperty());

    // Find parent IObjects
    std::vector<Alembic::Abc::IObject> parents;
    Alembic::Abc::IObject current = shapeObject.getParent();
    while (current.valid()) {
        parents.push_back(current);
        current = current.getParent();
    }

    // parent visibility
    fParentVisibilityCache.resize(parents.size());
    for (size_t i = 0; i < fParentVisibilityCache.size(); i++) {
        Alembic::AbcGeom::IVisibilityProperty visibilityProp = 
            Alembic::AbcGeom::GetVisibilityProperty(parents[i]);
        if (visibilityProp) {
            fParentVisibilityCache[i].init(visibilityProp);
        }
    }

    // exact animation time range
    fAnimTimeRange = TimeInterval(
        timeSampling->getSampleTime(0),
        timeSampling->getSampleTime(numSamples > 0 ? numSamples-1 : 0)
    );
}

DataProvider::~DataProvider()
{
    fValidityInterval = TimeInterval::kInvalid;

    // free the property readers
    fVisibilityCache.reset();
    fBoundingBoxCache.reset();
    fParentVisibilityCache.clear();
}

bool DataProvider::valid() const
{
    return fBoundingBoxCache.valid();
}

boost::shared_ptr<const ShapeSample> 
DataProvider::getBBoxPlaceHolderSample(double seconds)
{
    boost::shared_ptr<ShapeSample> sample = 
        ShapeSample::createBoundingBoxPlaceHolderSample(
            seconds,
            getBoundingBox(),
            isVisible()
        );
    return sample;
}

void DataProvider::fillBBoxAndVisSample(chrono_t time)
{
    fBBoxAndVisValidityInterval = updateBBoxAndVisCache(time);
    assert(fBBoxAndVisValidityInterval.valid());
}

void DataProvider::fillTopoAndAttrSample(chrono_t time)
{
    fValidityInterval = updateCache(time);
    assert(fValidityInterval.valid());
}

TimeInterval DataProvider::updateBBoxAndVisCache(chrono_t time)
{
    // Notes:
    //
    // When possible, we try to reuse the samples from the previously
    // read sample.

    // update caches
    if (fVisibilityCache.valid()) {
        fVisibilityCache.setTime(time);
    }
    fBoundingBoxCache.setTime(time);
    BOOST_FOREACH (ScalarPropertyCache<Alembic::Abc::ICharProperty>& 
            parentVisPropCache, fParentVisibilityCache) {
        if (parentVisPropCache.valid()) { 
            parentVisPropCache.setTime(time);
        }
    }

    // return the new cache valid interval
    TimeInterval validityInterval(TimeInterval::kInfinite);
    if (fVisibilityCache.valid()) {
        validityInterval &= fVisibilityCache.getValidityInterval();
    }
    validityInterval &= fBoundingBoxCache.getValidityInterval();
    BOOST_FOREACH (ScalarPropertyCache<Alembic::Abc::ICharProperty>& 
            parentVisPropCache, fParentVisibilityCache) {
        if (parentVisPropCache.valid()) { 
            validityInterval &= parentVisPropCache.getValidityInterval();
        }
    }
    return validityInterval;
}

TimeInterval DataProvider::updateCache(chrono_t time)
{
    return updateBBoxAndVisCache(time);
}

bool DataProvider::isVisible() const
{
    // shape invisible
    if (fVisibilityCache.valid() && 
            fVisibilityCache.getValue() == char(Alembic::AbcGeom::kVisibilityHidden)) {
        return false;
    }

    // parent invisible
    BOOST_FOREACH (const ScalarPropertyCache<Alembic::Abc::ICharProperty>& 
            parentVisPropCache, fParentVisibilityCache) {
        if (parentVisPropCache.valid() && 
                parentVisPropCache.getValue() == char(Alembic::AbcGeom::kVisibilityHidden)) {
            return false;
        }
    }

    // visible
    return true;
}


//==============================================================================
// CLASS PolyDataProvider
//==============================================================================

template<class SCHEMA>
PolyDataProvider::PolyDataProvider(
    SCHEMA&                         abcMesh,
    const bool                      needUVs)
  : DataProvider(abcMesh, abcMesh.getTimeSampling(), 
                 abcMesh.getNumSamples(), needUVs)
{
    // polygon counts
    fFaceCountsCache.init(abcMesh.getFaceCountsProperty());

    // positions
    fPositionsCache.init(abcMesh.getPositionsProperty());
}

PolyDataProvider::~PolyDataProvider()
{
    // free the property readers
    fFaceCountsCache.reset();
    fPositionsCache.reset();
}

bool PolyDataProvider::valid() const
{
    return DataProvider::valid() &&
            fFaceCountsCache.valid() &&
            fPositionsCache.valid();
}

TimeInterval
PolyDataProvider::updateCache(chrono_t time)
{
    TimeInterval validityInterval(DataProvider::updateCache(time));

    // update caches
    fFaceCountsCache.setTime(time);
    fPositionsCache.setTime(time);

    // return the new cache valid interval
    validityInterval &= fFaceCountsCache.getValidityInterval();
    validityInterval &= fPositionsCache.getValidityInterval();
    return validityInterval;
}


//==============================================================================
// CLASS RawDataProvider
//==============================================================================

RawDataProvider::RawDataProvider(
    Alembic::AbcGeom::IPolyMeshSchema& abcMesh,
    const bool needUVs)
    : PolyDataProvider(abcMesh, needUVs),
      fFaceIndicesCache(correctPolygonWinding)
{
    // triangle indices
    fFaceIndicesCache.init(abcMesh.getFaceIndicesProperty());

    // custom reader for wireframe indices
    if (abcMesh.getPropertyHeader(kCustomPropertyWireIndices) != NULL) {
        fWireIndicesCache.init(
            Alembic::Abc::IInt32ArrayProperty(abcMesh.getPtr(), kCustomPropertyWireIndices));
    }
    else if (abcMesh.getPropertyHeader(kCustomPropertyWireIndicesOld) != NULL) {
        fWireIndicesCache.init(
            Alembic::Abc::IInt32ArrayProperty(abcMesh.getPtr(), kCustomPropertyWireIndicesOld));
    }

    // custom reader for group info
    if (abcMesh.getPropertyHeader(kCustomPropertyShadingGroupSizes) != NULL) {
        fGroupSizesCache.init(
            Alembic::Abc::IInt32ArrayProperty(abcMesh.getPtr(), kCustomPropertyShadingGroupSizes));
    }

    // custom reader for diffuse color
    if (abcMesh.getPropertyHeader(kCustomPropertyDiffuseColor) != NULL) {
        fDiffuseColorCache.init(
            Alembic::Abc::IC4fProperty(abcMesh.getPtr(), kCustomPropertyDiffuseColor));
    }

    // normals, we do not support indexed/facevarying normals
    Alembic::AbcGeom::IN3fGeomParam normals = abcMesh.getNormalsParam();
    if (normals.valid()) {
        assert(!normals.isIndexed());
        assert(normals.getScope() == Alembic::AbcGeom::kVertexScope);
        fNormalsCache.init(normals.getValueProperty());
    }

    if (fNeedUVs) {
        // UVs, we do not support indexed/facevarying UVs
        Alembic::AbcGeom::IV2fGeomParam UVs = abcMesh.getUVsParam();
        if (UVs.valid()) {
            assert(!UVs.isIndexed());
            assert(UVs.getScope() == Alembic::AbcGeom::kVertexScope);
            fUVsCache.init(UVs.getValueProperty());
        }
    }
}

RawDataProvider::~RawDataProvider()
{
    // free the property readers
    fFaceIndicesCache.reset();
    fWireIndicesCache.reset();
    fGroupSizesCache.reset();
    fDiffuseColorCache.reset();
    fNormalsCache.reset();
    fUVsCache.reset();
}

bool RawDataProvider::valid() const
{
    return PolyDataProvider::valid() &&
            fFaceIndicesCache.valid() &&
            fWireIndicesCache.valid() &&
            fDiffuseColorCache.valid() &&
            fNormalsCache.valid();
}

boost::shared_ptr<const ShapeSample> 
RawDataProvider::getSample(double seconds)
{
    std::vector<boost::shared_ptr<IndexBuffer> > triangleVertIndices;
    const boost::shared_ptr<ReadableArray<IndexBuffer::index_t> > indexBuffer =
        fFaceIndicesCache.getValue();
    if (fGroupSizesCache.valid()) {
        const boost::shared_ptr<ReadableArray<IndexBuffer::index_t> > groupSizes =
            fGroupSizesCache.getValue();
        
        const IndexBuffer::index_t* groupSizesPtr = groupSizes->get();
        for(size_t i=0, offset=0; i<groupSizes->size(); offset+=3*groupSizesPtr[i], ++i) {
            triangleVertIndices.push_back(
                IndexBuffer::create(
                    indexBuffer, offset, offset+3*groupSizesPtr[i]));
        }
    }
    else {
        triangleVertIndices.push_back(IndexBuffer::create(indexBuffer));
    }

    Alembic::Abc::C4f diffuseColor = fDiffuseColorCache.getValue();

    boost::shared_ptr<ShapeSample> sample = ShapeSample::create(
        seconds,                                                   // time (in seconds)
        fWireIndicesCache.getValue()->size() / 2,                  // number of wireframes
        fPositionsCache.getValue()->size() / 3,                    // number of vertices
        IndexBuffer::create(fWireIndicesCache.getValue()),         // wireframe indices
        triangleVertIndices,                                       // triangle indices
        VertexBuffer::createPositions(fPositionsCache.getValue()), // position
        getBoundingBox(),                                          // bounding box
        MColor(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a),
        isVisible()
    );

    if (fNormalsCache.valid()) {
        sample->setNormals(
            VertexBuffer::createNormals(fNormalsCache.getValue()));
    }

    if (fUVsCache.valid()) {
        sample->setUVs(
            VertexBuffer::createUVs(fUVsCache.getValue()));
    }

    return sample;
}

boost::shared_ptr<ReadableArray<IndexBuffer::index_t> >
    RawDataProvider::correctPolygonWinding(const Alembic::Abc::Int32ArraySamplePtr& indices)
{

    size_t count = (*indices).size();
    boost::shared_array<IndexBuffer::index_t> faceIndicesCCW(
        new IndexBuffer::index_t[count]);
    for (size_t i = 0; i < count; i += 3) {
        faceIndicesCCW[i + 2] = (*indices)[i + 0];
        faceIndicesCCW[i + 1] = (*indices)[i + 1];
        faceIndicesCCW[i + 0] = (*indices)[i + 2];
    }

    return SharedArray<IndexBuffer::index_t>::create(
        faceIndicesCCW, count);
}

TimeInterval RawDataProvider::updateCache(chrono_t time)
{
    TimeInterval validityInterval(PolyDataProvider::updateCache(time));

    // update caches
    fFaceIndicesCache.setTime(time);
    fWireIndicesCache.setTime(time);
    if (fGroupSizesCache.valid()) {
        fGroupSizesCache.setTime(time);
    }
    fNormalsCache.setTime(time);
    if (fUVsCache.valid()) {
        fUVsCache.setTime(time);
    }
    fDiffuseColorCache.setTime(time);

    // return the new cache valid interval
    validityInterval &= fFaceIndicesCache.getValidityInterval();
    validityInterval &= fWireIndicesCache.getValidityInterval();
    if (fGroupSizesCache.valid()) {
        validityInterval &= fGroupSizesCache.getValidityInterval();
    }
    validityInterval &= fNormalsCache.getValidityInterval();
    if (fUVsCache.valid()) {
        validityInterval &= fUVsCache.getValidityInterval();
    }
    validityInterval &= fDiffuseColorCache.getValidityInterval();

    // check sample consistency
    size_t numVerts     = fPositionsCache.getValue()->size() / 3;
    size_t numTriangles = fFaceIndicesCache.getValue()->size() / 3;
    if (fFaceCountsCache.getValue()->size() != numTriangles) {
        assert(fFaceCountsCache.getValue()->size() == numTriangles);
        return TimeInterval::kInvalid;
    }
    if (fNormalsCache.getValue()->size() / 3 != numVerts) {
        assert(fNormalsCache.getValue()->size() / 3 == numVerts);
        return TimeInterval::kInvalid;
    }
    if (fUVsCache.valid()) {
        if (fUVsCache.getValue()->size() / 2 != numVerts) {
            assert(fUVsCache.getValue()->size() / 2 == numVerts);
            return TimeInterval::kInvalid;
        }
    }
    
    return validityInterval;
}


//==============================================================================
// CLASS Triangulator
//==============================================================================

Triangulator::Triangulator(
    Alembic::AbcGeom::IPolyMeshSchema& abcMesh,
    const bool needUVs)
    : PolyDataProvider(abcMesh, needUVs),
    fNormalsScope(Alembic::AbcGeom::kUnknownScope), fUVsScope(Alembic::AbcGeom::kUnknownScope)
{
    // polygon indices
    fFaceIndicesCache.init(abcMesh.getFaceIndicesProperty());

    // optional normals
    Alembic::AbcGeom::IN3fGeomParam normals = abcMesh.getNormalsParam();
    if (normals.valid()) {
        fNormalsScope = normals.getScope();
        if (fNormalsScope == Alembic::AbcGeom::kVaryingScope ||
                fNormalsScope == Alembic::AbcGeom::kVertexScope ||
                fNormalsScope == Alembic::AbcGeom::kFacevaryingScope) {
            fNormalsCache.init(normals.getValueProperty());
            if (normals.isIndexed()) {
                fNormalIndicesCache.init(normals.getIndexProperty());
            }
        }
    }

    // optional UVs
    if (fNeedUVs) {
        Alembic::AbcGeom::IV2fGeomParam UVs = abcMesh.getUVsParam();
        if (UVs.valid()) {
            fUVsScope = UVs.getScope();
            if (fUVsScope == Alembic::AbcGeom::kVaryingScope ||
                fUVsScope == Alembic::AbcGeom::kVertexScope ||
                fUVsScope == Alembic::AbcGeom::kFacevaryingScope) {
                fUVsCache.init(UVs.getValueProperty());
                if (UVs.isIndexed()) {
                    fUVIndicesCache.init(UVs.getIndexProperty());
                }
            }
        }
    }
}

Triangulator::~Triangulator()
{
    // free the property readers
    fFaceIndicesCache.reset();

    fNormalsScope = Alembic::AbcGeom::kUnknownScope;
    fNormalsCache.reset();
    fNormalIndicesCache.reset();

    fUVsScope = Alembic::AbcGeom::kUnknownScope;
    fUVsCache.reset();
    fUVIndicesCache.reset();
}

bool Triangulator::valid() const
{
    return PolyDataProvider::valid() &&
            fFaceIndicesCache.valid();
}

boost::shared_ptr<const ShapeSample> 
Triangulator::getSample(double seconds)
{
    // empty mesh
    if (!fWireIndices || !fTriangleIndices) {
        boost::shared_ptr<ShapeSample> sample =
            ShapeSample::createEmptySample(seconds);
        return sample;
    }

    // triangle indices
    // Currently, we only have 1 group
    std::vector<boost::shared_ptr<IndexBuffer> > triangleVertIndices;
    triangleVertIndices.push_back(IndexBuffer::create(fTriangleIndices));


    boost::shared_ptr<ShapeSample> sample = ShapeSample::create(
        seconds,                                           // time (in seconds)
        fWireIndices->size() / 2,                          // number of wireframes
        fMappedPositions->size() / 3,                      // number of vertices
        IndexBuffer::create(fWireIndices),                 // wireframe indices
        triangleVertIndices,                               // triangle indices (1 group)
        VertexBuffer::createPositions(fMappedPositions),   // position
        getBoundingBox(),                                  // bounding box
        Config::kDefaultGrayColor,                         // diffuse color
        isVisible()
    );

    if (fMappedNormals) {
        sample->setNormals(
            VertexBuffer::createNormals(fMappedNormals));
    }

    if (fMappedUVs) {
        sample->setUVs(
            VertexBuffer::createUVs(fMappedUVs));
    }
    return sample;
}

TimeInterval Triangulator::updateCache(chrono_t time)
{
    // update faceCounts/position cache here so that we can detect topology/position change.
    // next setTime() in DataProvider::updateCache() simply returns early
    bool topologyChanged = fFaceCountsCache.setTime(time);
    bool positionChanged = fPositionsCache.setTime(time);

    TimeInterval validityInterval(PolyDataProvider::updateCache(time));

    // update caches
    topologyChanged = fFaceIndicesCache.setTime(time) || topologyChanged;

    if (fNormalsCache.valid()) {
        fNormalsCache.setTime(time);
        if (fNormalIndicesCache.valid()) {
            fNormalIndicesCache.setTime(time);
        }
    }

    if (fUVsCache.valid()) {
        fUVsCache.setTime(time);
        if (fUVIndicesCache.valid()) {
            fUVIndicesCache.setTime(time);
        }
    }

    // return the new cache valid interval
    validityInterval &= fFaceIndicesCache.getValidityInterval();
    if (fNormalsCache.valid()) {
        validityInterval &= fNormalsCache.getValidityInterval();
        if (fNormalIndicesCache.valid()) {
            validityInterval &= fNormalIndicesCache.getValidityInterval();
        }
    }
    if (fUVsCache.valid()) {
        validityInterval &= fUVsCache.getValidityInterval();
        if (fUVIndicesCache.valid()) {
            validityInterval &= fUVIndicesCache.getValidityInterval();
        }
    }

    // do a minimal check for the consistency
    check();

    // convert the mesh to display-friend format
    if (positionChanged || topologyChanged || !fComputedNormals) {
        // recompute normals on position/topology change
        computeNormals();
    }

    if (topologyChanged || !fVertAttribsIndices) {
        // convert multi-indexed streams on topology change
        convertMultiIndexedStreams();
    }

    remapVertAttribs();

    if (topologyChanged || !fWireIndices) {
        // recompute wireframe indices on topology change
        computeWireIndices();
    }

    if (topologyChanged || !fTriangleIndices) {
        // recompute triangulation on topology change
        triangulate();
    }

    return validityInterval;
}

template<size_t SIZE>
boost::shared_ptr<ReadableArray<float> > Triangulator::convertMultiIndexedStream(
    boost::shared_ptr<ReadableArray<float> > attribArray,
    boost::shared_ptr<ReadableArray<IndexBuffer::index_t> > indexArray)
{
    // map the indexed array to direct array
    size_t numVerts                                 = indexArray->size();
    const float* srcAttribs                         = attribArray->get();
    const IndexBuffer::index_t* srcIndices          = indexArray->get();

    boost::shared_array<float> mappedAttribs(new float[numVerts * SIZE]);
    for (size_t i = 0; i < numVerts; i++) {
        for (size_t j = 0; j < SIZE; j++) {
            mappedAttribs[i * SIZE + j] = srcAttribs[srcIndices[i] * SIZE + j];
        }
    }

    return SharedArray<float>::create(mappedAttribs, numVerts * SIZE);
}

void Triangulator::check()
{
    size_t numFaceIndices = fFaceIndicesCache.getValue()->size();
    size_t numVerts       = fPositionsCache.getValue()->size() / 3;

    // Normals
    size_t numExpectedNormals = 0;
    if (fNormalsScope == Alembic::AbcGeom::kVaryingScope ||
            fNormalsScope == Alembic::AbcGeom::kVertexScope) {
        numExpectedNormals = numVerts;
    }
    else if (fNormalsScope == Alembic::AbcGeom::kFacevaryingScope) {
        numExpectedNormals = numFaceIndices;
    }

    size_t numActualNormals = 0;
    if (fNormalsCache.valid()) {
        if (fNormalIndicesCache.valid()) {
            numActualNormals = fNormalIndicesCache.getValue()->size();
        }
        else {
            numActualNormals = fNormalsCache.getValue()->size() / 3;
        }
    }

    // clear previous result
    fCheckedNormalsScope = Alembic::AbcGeom::kUnknownScope;
    fCheckedNormals.reset();
    fCheckedNormalIndices.reset();

    // forward 
    if (numExpectedNormals == numActualNormals) {
        if (fNormalsCache.valid()) {
            fCheckedNormalsScope = fNormalsScope;
            fCheckedNormals      = fNormalsCache.getValue();
            if (fNormalIndicesCache.valid()) {
                fCheckedNormalIndices = fNormalIndicesCache.getValue();
            }
        }
    }
    else {
        DisplayWarning(kBadNormalsMsg);
    }

    // UVs
    size_t numExpectedUVs = 0;
    if (fUVsScope == Alembic::AbcGeom::kVaryingScope ||
        fUVsScope == Alembic::AbcGeom::kVertexScope) {
            numExpectedUVs = numVerts;
    }
    else if (fUVsScope == Alembic::AbcGeom::kFacevaryingScope) {
        numExpectedUVs = numFaceIndices;
    }

    size_t numActualUVs = 0;
    if (fUVsCache.valid()) {
        if (fUVIndicesCache.valid()) {
            numActualUVs = fUVIndicesCache.getValue()->size();
        }
        else {
            numActualUVs = fUVsCache.getValue()->size() / 2;
        }
    }

    // clear previous result
    fCheckedUVsScope = Alembic::AbcGeom::kUnknownScope;
    fCheckedUVs.reset();
    fCheckedUVIndices.reset();

    // forward 
    if (numExpectedUVs == numActualUVs) {
        if (fUVsCache.valid()) {
            fCheckedUVsScope = fUVsScope;
            fCheckedUVs      = fUVsCache.getValue();
            if (fUVIndicesCache.valid()) {
                fCheckedUVIndices = fUVIndicesCache.getValue();
            }
        }
    }
    else {
        DisplayWarning(kBadUVsMsg);
    }
}

void Triangulator::computeNormals()
{
    // compute normals if the normals are missing
    // later on, we can safely assume that the normals always exist
    //
    if (fCheckedNormals && (fCheckedNormalsScope == Alembic::AbcGeom::kVaryingScope
            || fCheckedNormalsScope == Alembic::AbcGeom::kVertexScope
            || fCheckedNormalsScope == Alembic::AbcGeom::kFacevaryingScope)) {
        // the normals exist and we recognize these normals
        fComputedNormals      = fCheckedNormals;
        fComputedNormalsScope = fCheckedNormalsScope;
        fComputedNormalIndices.reset();
        if (fCheckedNormalIndices) {
            fComputedNormalIndices = fCheckedNormalIndices;
        }
        return;
    }

    // input data
    size_t numFaceCounts           = fFaceCountsCache.getValue()->size();
    const unsigned int* faceCounts = fFaceCountsCache.getValue()->get();

    const IndexBuffer::index_t* faceIndices = fFaceIndicesCache.getValue()->get();

    size_t    numPositions = fPositionsCache.getValue()->size();
    const float* positions = fPositionsCache.getValue()->get();

    size_t numVerts = numPositions / 3;

    if (numVerts == 0) {
        fComputedNormalsScope = Alembic::AbcGeom::kUnknownScope;
        fComputedNormals.reset();
        fComputedNormalIndices.reset();
        return;
    }

    // allocate buffers for the new normals
    boost::shared_array<float> computedFaceNormals(new float[numFaceCounts * 3]);
    boost::shared_array<float> computedNormals(new float[numVerts * 3]);

    // compute the face normals
    for (size_t i = 0, polyVertOffset = 0; i < numFaceCounts; polyVertOffset += faceCounts[i], i++) {
        size_t numPoints = faceCounts[i];

        // Newell's Method
        MFloatVector faceNormal(0.0f, 0.0f, 0.0f);
        for (size_t j = 0; j < numPoints; j++) {
            size_t thisJ = numPoints - j - 1;
            size_t nextJ = numPoints - ((j + 1) % numPoints) - 1;
            const float* thisPoint = &positions[faceIndices[polyVertOffset + thisJ] * 3];
            const float* nextPoint = &positions[faceIndices[polyVertOffset + nextJ] * 3];
            faceNormal.x += (thisPoint[1] - nextPoint[1]) * (thisPoint[2] + nextPoint[2]);
            faceNormal.y += (thisPoint[2] - nextPoint[2]) * (thisPoint[0] + nextPoint[0]);
            faceNormal.z += (thisPoint[0] - nextPoint[0]) * (thisPoint[1] + nextPoint[1]);
        }
        faceNormal.normalize();

        computedFaceNormals[i * 3 + 0] = faceNormal.x;
        computedFaceNormals[i * 3 + 1] = faceNormal.y;
        computedFaceNormals[i * 3 + 2] = faceNormal.z;
    }

    // compute the normals
    memset(&computedNormals[0], 0, sizeof(float) * numVerts * 3);
    for (size_t i = 0, polyVertOffset = 0; i < numFaceCounts; polyVertOffset += faceCounts[i], i++) {
        size_t numPoints = faceCounts[i];
        const float* faceNormal = &computedFaceNormals[i * 3];

        // accumulate the face normal
        for (size_t j = 0; j < numPoints; j++) {
            float* normal = &computedNormals[faceIndices[polyVertOffset + j] * 3];
            normal[0] += faceNormal[0];
            normal[1] += faceNormal[1];
            normal[2] += faceNormal[2];
        }
    }

    // normalize normals, MFloatVector functions are inline functions
    for (size_t i = 0; i < numVerts; i++) {
        float* normal = &computedNormals[i * 3];

        MFloatVector vector(normal[0], normal[1], normal[2]);
        vector.normalize();

        normal[0] = vector.x;
        normal[1] = vector.y;
        normal[2] = vector.z;
    }

    fComputedNormalsScope = Alembic::AbcGeom::kVertexScope;
    fComputedNormals      = SharedArray<float>::create(computedNormals, numVerts * 3);
    fComputedNormalIndices.reset();
}

void Triangulator::convertMultiIndexedStreams()
{
    // convert multi-indexed streams to single-indexed streams
    // assume the scope is kVarying/kVertex/kFacevarying
    //

    // input polygons data
    size_t                      numFaceIndices = fFaceIndicesCache.getValue()->size();
    const IndexBuffer::index_t* faceIndices = fFaceIndicesCache.getValue()->get();

    // input normals
    bool                        normalFaceVarying = false;
    const IndexBuffer::index_t* normalIndices     = NULL;
    if (fComputedNormals) {
        normalFaceVarying = (fComputedNormalsScope == Alembic::AbcGeom::kFacevaryingScope);
        if (fComputedNormalIndices) {
            normalIndices = fComputedNormalIndices->get();
            assert(fComputedNormalIndices->size() == numFaceIndices);
        }
    }

    // input UV indices
    bool                        uvFaceVarying = false;
    const IndexBuffer::index_t* uvIndices     = NULL;
    if (fCheckedUVs) {
        uvFaceVarying = (fCheckedUVsScope == Alembic::AbcGeom::kFacevaryingScope);
        if (fCheckedUVIndices) {
            uvIndices = fCheckedUVIndices->get();
            assert(fCheckedUVIndices->size() == numFaceIndices);
        }
    }

    // determine the number of multi-indexed streams
    MultiIndexedStreamsConverter<IndexBuffer::index_t> converter(
            numFaceIndices, faceIndices);

    if (normalFaceVarying) {
        converter.addMultiIndexedStream(normalIndices);
    }
    if (uvFaceVarying) {
        converter.addMultiIndexedStream(uvIndices);
    }

    // only one multi-indexed streams, no need to convert it
    if (converter.numStreams() == 1) {
        fVertAttribsIndices.reset();
        fMappedFaceIndices = fFaceIndicesCache.getValue();
        fNumVertices       = fPositionsCache.getValue()->size() / 3;
        return;
    }

    // convert the multi-indexed streams
    converter.compute();

    // the mapped face indices
    fMappedFaceIndices = SharedArray<IndexBuffer::index_t>::create(
        converter.mappedFaceIndices(), numFaceIndices);

    // indices to remap streams
    fVertAttribsIndices = converter.vertAttribsIndices();
    fNumVertices        = converter.numVertices();
}

void Triangulator::remapVertAttribs()
{
    // remap vertex attribute streams according to the result of convertMultiIndexedStreams()
    // assume the scope is kVarying/kVertex/kFacevarying
    //

    // no multi-index streams, just drop indices
    if (!fVertAttribsIndices) {
        // positions is the only stream, just use it
        fMappedPositions = fPositionsCache.getValue();

        // get rid of normal indices
        if (fComputedNormals) {
            if (fComputedNormalIndices) {
                fMappedNormals = convertMultiIndexedStream<3>(
                    fComputedNormals, fComputedNormalIndices);
            }
            else {
                fMappedNormals = fComputedNormals;
            }
        }

        // get rid of UV indices
        if (fCheckedUVs) {
            if (fCheckedUVIndices) {
                fMappedUVs = convertMultiIndexedStream<2>(
                    fCheckedUVs, fCheckedUVIndices);
            }
            else {
                fMappedUVs = fCheckedUVs;
            }
        }

        return;
    }

    // input polygons data
    const float*                positions   = fPositionsCache.getValue()->get();
    const IndexBuffer::index_t* faceIndices = fFaceIndicesCache.getValue()->get();

    // input normals
    const float*                normals       = NULL;
    const IndexBuffer::index_t* normalIndices = NULL;
    if (fComputedNormals) {
        normals = fComputedNormals->get();
        if (fComputedNormalIndices) {
            normalIndices = fComputedNormalIndices->get();
        }
    }

    // input UV indices
    const float*                UVs       = NULL;
    const IndexBuffer::index_t* uvIndices = NULL;
    if (fCheckedUVs) {
        UVs = fCheckedUVs->get();
        if (fCheckedUVIndices) {
            uvIndices = fCheckedUVIndices->get();
        }
    }

    // set up multi-indexed streams remapper
    MultiIndexedStreamsRemapper<IndexBuffer::index_t> remapper(
        faceIndices, fNumVertices, fVertAttribsIndices.get());

    remapper.addMultiIndexedStream(positions, NULL, false, 3);

    if (normals) {
        remapper.addMultiIndexedStream(normals, normalIndices,
            fComputedNormalsScope == Alembic::AbcGeom::kFacevaryingScope, 3);
    }

    if (UVs) {
        remapper.addMultiIndexedStream(UVs, uvIndices, 
            fCheckedUVsScope == Alembic::AbcGeom::kFacevaryingScope, 2);
    }

    // remap streams
    remapper.compute();

    fMappedPositions = SharedArray<float>::create(
        remapper.mappedVertAttribs(0), fNumVertices * 3);

    unsigned int streamIndex = 1;
    if (normals) {
        fMappedNormals = SharedArray<float>::create(
            remapper.mappedVertAttribs(streamIndex++), fNumVertices * 3);
    }

    if (UVs) {
        fMappedUVs = SharedArray<float>::create(
            remapper.mappedVertAttribs(streamIndex++), fNumVertices * 2);
    }
}

void Triangulator::computeWireIndices()
{
    // compute the wireframe indices
    //

    // input data
    size_t           numFaceCounts = fFaceCountsCache.getValue()->size();
    const unsigned int* faceCounts = fFaceCountsCache.getValue()->get();

    size_t                   numFaceIndices = fFaceIndicesCache.getValue()->size();
    const IndexBuffer::index_t* faceIndices = fFaceIndicesCache.getValue()->get();
    const IndexBuffer::index_t* mappedFaceIndices = fMappedFaceIndices->get();

    // Compute wireframe indices
    WireIndicesGenerator<IndexBuffer::index_t> wireIndicesGenerator(
            numFaceCounts, faceCounts, numFaceIndices, faceIndices, mappedFaceIndices);
    wireIndicesGenerator.compute();

    if (wireIndicesGenerator.numWires() == 0) {
        fWireIndices.reset();
        return;
    }

    fWireIndices = SharedArray<IndexBuffer::index_t>::create(
        wireIndicesGenerator.wireIndices(), wireIndicesGenerator.numWires() * 2);
}

void Triangulator::triangulate()
{
    // triangulate the polygons
    // assume that there are no holes
    //

    // input data
    size_t           numFaceCounts = fFaceCountsCache.getValue()->size();
    const unsigned int* faceCounts = fFaceCountsCache.getValue()->get();

    const IndexBuffer::index_t* faceIndices = fMappedFaceIndices->get();

    const float* positions = fMappedPositions->get();
    const float* normals   = fMappedNormals ? fMappedNormals->get() : NULL;

    if (numFaceCounts == 0) {
        fTriangleIndices.reset();
        return;
    }

    // Triangulate polygons
    PolyTriangulator<IndexBuffer::index_t> polyTriangulator(
        numFaceCounts, faceCounts, faceIndices, true, positions, normals);

    polyTriangulator.compute();
    
    fTriangleIndices = SharedArray<IndexBuffer::index_t>::create(
        polyTriangulator.triangleIndices(), polyTriangulator.numTriangles() * 3);
}


//==============================================================================
// CLASS NurbsTessellator
//==============================================================================

NurbsTessellator::NurbsTessellator(
    Alembic::AbcGeom::INuPatchSchema& abcNurbs,
    const bool needUVs)
    : DataProvider(abcNurbs, abcNurbs.getTimeSampling(),
                   abcNurbs.getNumSamples(), needUVs),
    fSurfaceValid(false)
{
    // Control point positions
    fPositionsCache.init(abcNurbs.getPositionsProperty());

    // Number of control points
    fNumUCache.init(Alembic::Abc::IInt32Property(abcNurbs.getPtr(), "nu"));
    fNumVCache.init(Alembic::Abc::IInt32Property(abcNurbs.getPtr(), "nv"));

    // Order (Degree + 1)
    fUOrderCache.init(Alembic::Abc::IInt32Property(abcNurbs.getPtr(), "uOrder"));
    fVOrderCache.init(Alembic::Abc::IInt32Property(abcNurbs.getPtr(), "vOrder"));

    // Knots
    fUKnotCache.init(abcNurbs.getUKnotsProperty());
    fVKnotCache.init(abcNurbs.getVKnotsProperty());

    // Control point weights
    Alembic::AbcGeom::IFloatArrayProperty positionWeights = abcNurbs.getPositionWeightsProperty();
    if (positionWeights.valid()) {
        fPositionWeightsCache.init(positionWeights);
    }

    // Trim curves
    if (abcNurbs.hasTrimCurve()) {
        // Number of loops
        fTrimNumLoopsCache.init(
            Alembic::Abc::IInt32Property(abcNurbs.getPtr(), "trim_nloops"));

        // Number of curves
        fTrimNumCurvesCache.init(
            Alembic::Abc::IInt32ArrayProperty(abcNurbs.getPtr(), "trim_ncurves"));

        // Number of control points
        fTrimNumVerticesCache.init(
            Alembic::Abc::IInt32ArrayProperty(abcNurbs.getPtr(), "trim_n"));

        // Curve Orders
        fTrimOrderCache.init(
            Alembic::Abc::IInt32ArrayProperty(abcNurbs.getPtr(), "trim_order"));

        // Curve Knots
        fTrimKnotCache.init(
            Alembic::Abc::IFloatArrayProperty(abcNurbs.getPtr(), "trim_knot"));

        // Curve U
        fTrimUCache.init(
            Alembic::Abc::IFloatArrayProperty(abcNurbs.getPtr(), "trim_u"));

        // Curve V
        fTrimVCache.init(
            Alembic::Abc::IFloatArrayProperty(abcNurbs.getPtr(), "trim_v"));

        // Curve W
        fTrimWCache.init(
            Alembic::Abc::IFloatArrayProperty(abcNurbs.getPtr(), "trim_w"));
    }
}

NurbsTessellator::~NurbsTessellator()
{
    // free the property readers
    fPositionsCache.reset();
    fNumUCache.reset();
    fNumVCache.reset();
    fUOrderCache.reset();
    fVOrderCache.reset();
    fUKnotCache.reset();
    fVKnotCache.reset();

    fPositionWeightsCache.reset();

    fTrimNumLoopsCache.reset();
    fTrimNumCurvesCache.reset();
    fTrimNumVerticesCache.reset();
    fTrimOrderCache.reset();
    fTrimKnotCache.reset();
    fTrimUCache.reset();
    fTrimVCache.reset();
    fTrimWCache.reset();
}

bool NurbsTessellator::valid() const
{
    return DataProvider::valid() &&
            fPositionsCache.valid() &&
            fNumUCache.valid() &&
            fNumVCache.valid() &&
            fUOrderCache.valid() &&
            fVOrderCache.valid() &&
            fUKnotCache.valid() &&
            fVKnotCache.valid();
}

boost::shared_ptr<const ShapeSample> 
NurbsTessellator::getSample(double seconds)
{
    // empty mesh
    if (!fWireIndices || !fTriangleIndices) {
        boost::shared_ptr<ShapeSample> sample =
            ShapeSample::createEmptySample(seconds);
        return sample;
    }

    // triangle indices
    // Currently, we only have 1 group
    std::vector<boost::shared_ptr<IndexBuffer> > triangleVertIndices;
    triangleVertIndices.push_back(IndexBuffer::create(fTriangleIndices));


    boost::shared_ptr<ShapeSample> sample = ShapeSample::create(
        seconds,                                     // time (in seconds)
        fWireIndices->size() / 2,                    // number of wireframes
        fPositions->size() / 3,                      // number of vertices
        IndexBuffer::create(fWireIndices),           // wireframe indices
        triangleVertIndices,                         // triangle indices (1 group)
        VertexBuffer::createPositions(fPositions),   // position
        getBoundingBox(),                            // bounding box
        Config::kDefaultGrayColor,                   // diffuse color
        isVisible()
    );
    
    if (fNormals) {
        sample->setNormals(
            VertexBuffer::createNormals(fNormals));
    }

    if (fUVs) {
        sample->setUVs(
            VertexBuffer::createUVs(fUVs));
    }
    return sample;
}

TimeInterval NurbsTessellator::updateCache(chrono_t time)
{
    TimeInterval validityInterval(DataProvider::updateCache(time));

    // update caches
    bool positionsChanged = fPositionsCache.setTime(time);

    bool topologyChanged = fNumUCache.setTime(time);
    topologyChanged      = fNumVCache.setTime(time)   || topologyChanged;
    topologyChanged      = fUOrderCache.setTime(time) || topologyChanged;
    topologyChanged      = fVOrderCache.setTime(time) || topologyChanged;

    bool knotChanged = fUKnotCache.setTime(time);
    knotChanged      = fVKnotCache.setTime(time) || knotChanged;

    if (fPositionWeightsCache.valid()) {
        positionsChanged = fPositionWeightsCache.setTime(time) || positionsChanged;
    }

    bool trimCurvesChanged = false;
    if (fTrimNumLoopsCache.valid()) {
        trimCurvesChanged = fTrimNumLoopsCache.setTime(time)    || trimCurvesChanged;
        trimCurvesChanged = fTrimNumCurvesCache.setTime(time)   || trimCurvesChanged;
        trimCurvesChanged = fTrimNumVerticesCache.setTime(time) || trimCurvesChanged;
        trimCurvesChanged = fTrimOrderCache.setTime(time)       || trimCurvesChanged;
        trimCurvesChanged = fTrimKnotCache.setTime(time)        || trimCurvesChanged;
        trimCurvesChanged = fTrimUCache.setTime(time)           || trimCurvesChanged;
        trimCurvesChanged = fTrimVCache.setTime(time)           || trimCurvesChanged;
        trimCurvesChanged = fTrimWCache.setTime(time)           || trimCurvesChanged;
    }
    
    // return the new cache valid interval
    validityInterval &= fPositionsCache.getValidityInterval();
    validityInterval &= fNumUCache.getValidityInterval();
    validityInterval &= fNumVCache.getValidityInterval();
    validityInterval &= fUOrderCache.getValidityInterval();
    validityInterval &= fVOrderCache.getValidityInterval();
    validityInterval &= fUKnotCache.getValidityInterval();
    validityInterval &= fVKnotCache.getValidityInterval();

    if (fPositionWeightsCache.valid()) {
        validityInterval &= fPositionWeightsCache.getValidityInterval();
    }

    if (fTrimNumLoopsCache.valid()) {
        validityInterval &= fTrimNumLoopsCache.getValidityInterval();
        validityInterval &= fTrimNumCurvesCache.getValidityInterval();
        validityInterval &= fTrimNumVerticesCache.getValidityInterval();
        validityInterval &= fTrimOrderCache.getValidityInterval();
        validityInterval &= fTrimKnotCache.getValidityInterval();
        validityInterval &= fTrimUCache.getValidityInterval();
        validityInterval &= fTrimVCache.getValidityInterval();
        validityInterval &= fTrimWCache.getValidityInterval();
    }

    // do a minimal check for the consistency
    check();

    // set Alembic INuPatch to Maya MFnNurbsSurface
    bool rebuild = topologyChanged || knotChanged || 
                   trimCurvesChanged || fNurbsData.object().isNull();
    setNurbs(rebuild, positionsChanged);

    // tessellate Maya NURBS and convert to poly
    if (rebuild || positionsChanged) {
        tessellate();
    }

    if (isVisible()) {
        convertToPoly();
    }

    return validityInterval;
}

void NurbsTessellator::check()
{
    // reset valid flag
    MStatus status;
    fSurfaceValid = true;

    // numKnots = numCV + degree + 1
    int uDegree   = fUOrderCache.getValue() - 1;
    int vDegree   = fVOrderCache.getValue() - 1;
    int numUCV    = fNumUCache.getValue();
    int numVCV    = fNumVCache.getValue();
    int numUKnots = (int)fUKnotCache.getValue()->size();
    int numVKnots = (int)fVKnotCache.getValue()->size();
    if (numUKnots != numUCV + uDegree + 1 || numVKnots != numVCV + vDegree + 1) {
        fSurfaceValid = false;
        DisplayWarning(kBadNurbsMsg);
        return;
    }

    // numCV = numU * numV
    size_t numCVs = numUCV * numVCV;
    if (numCVs * 3 != fPositionsCache.getValue()->size()) {
        fSurfaceValid = false;
        DisplayWarning(kBadNurbsMsg);
        return;
    }

    // numCV = numWeight
    if (fPositionWeightsCache.valid() &&
            numCVs != fPositionWeightsCache.getValue()->size()) {
        fSurfaceValid = false;
        DisplayWarning(kBadNurbsMsg);
        return;
    }
}

void NurbsTessellator::setNurbs(bool rebuild, bool positionsChanged)
{
    if (!fSurfaceValid) {
        // invalid NURBS
        fNurbsData.setObject(MObject::kNullObj);
        fNurbs.setObject(MObject::kNullObj);
        return;
    }

	// Number of control points in U/V direction
	unsigned int numU = 0;
	unsigned int numV = 0;

    MPointArray mayaPositions;
    if (rebuild || positionsChanged) {
        numU = fNumUCache.getValue();
        numV = fNumVCache.getValue();

        // Positions and their weights
        const float* positions = fPositionsCache.getValue()->get();
        const float* positionWeights = NULL;
        if (fPositionWeightsCache.valid()) {
            positionWeights = fPositionWeightsCache.getValue()->get();
        }

        // allocate memory for positions
        mayaPositions.setLength(numU * numV);

        // Maya is U-major and has inversed V
        for (unsigned int u = 0; u < numU; u++) {
            for (unsigned int v = 0; v < numV; v++) {
                unsigned int alembicIndex = v * numU + u;
                unsigned int mayaIndex    = u * numV + (numV - v - 1);

                MPoint point(positions[alembicIndex * 3 + 0],
                             positions[alembicIndex * 3 + 1],
                             positions[alembicIndex * 3 + 2],
                             1.0);
                if (positionWeights) {
                    point.w = positionWeights[alembicIndex];
                }

                mayaPositions[mayaIndex] = point;
            }
        }
    }

    if (rebuild) {
        // Nurbs degree
        unsigned int uDegree = fUOrderCache.getValue() - 1;
        unsigned int vDegree = fVOrderCache.getValue() - 1;

        // Nurbs form
        // Alemblic file does not record the form of nurb surface, we get the form
        // by checking the CV data. If the first degree number CV overlap the last
        // degree number CV, then the form is kPeriodic. If only the first CV overlaps
        // the last CV, then the form is kClosed.
        MFnNurbsSurface::Form uForm = MFnNurbsSurface::kPeriodic;
        MFnNurbsSurface::Form vForm = MFnNurbsSurface::kPeriodic;
        // Check all curves
        bool notOpen = true;
        for (unsigned int v = 0; notOpen && v < numV; v++) {
            for (unsigned int u = 0; u < uDegree; u++) {
                unsigned int firstIndex = u * numV + (numV - v - 1);
                unsigned int lastPeriodicIndex = (numU - uDegree + u) * numV + (numV - v - 1);
                if (!mayaPositions[firstIndex].isEquivalent(mayaPositions[lastPeriodicIndex])) {
                    uForm = MFnNurbsSurface::kOpen;
                    notOpen = false;
                    break;
                }
            }
        }
        if (uForm == MFnNurbsSurface::kOpen) {
            uForm = MFnNurbsSurface::kClosed;
            for (unsigned int v = 0; v < numV; v++) {
                unsigned int lastUIndex = (numU - 1) * numV + (numV - v - 1);
                if (!mayaPositions[numV-v-1].isEquivalent(mayaPositions[lastUIndex])) {
                    uForm = MFnNurbsSurface::kOpen;
                    break;
                }
            }
        }

        notOpen = true;
        for (unsigned int u = 0; notOpen && u < numU; u++) {
            for (unsigned int v = 0; v < vDegree; v++) {
                unsigned int firstIndex = u * numV + (numV - v - 1);
                unsigned int lastPeriodicIndex =  u * numV + (vDegree - v - 1); //numV - (numV - vDegree + v) - 1;
                if (!mayaPositions[firstIndex].isEquivalent(mayaPositions[lastPeriodicIndex])) {
                    vForm = MFnNurbsSurface::kOpen;
                    notOpen = false;
                    break;
                }
            }
        }
        if (vForm == MFnNurbsSurface::kOpen) {
            vForm = MFnNurbsSurface::kClosed;
            for (unsigned int u = 0; u < numU; u++) {
                if (!mayaPositions[u*numV+(numV-1)].isEquivalent(mayaPositions[u*numV])) {
                    vForm = MFnNurbsSurface::kOpen;
                    break;
                }
            }
        }
        

        // Knots
        //   Dispose the leading and trailing knots
        //   Alembic duplicate CVs if the form is not kOpen
        //   For more information, refer to MFnNurbsSurface
        unsigned int numUKnot = (unsigned int)fUKnotCache.getValue()->size();
        unsigned int numVKnot = (unsigned int)fVKnotCache.getValue()->size();
        const float* uKnot    = fUKnotCache.getValue()->get();
        const float* vKnot    = fVKnotCache.getValue()->get();
        MDoubleArray mayaUKnots(uKnot + 1, numUKnot - 2);
        MDoubleArray mayaVKnots(vKnot + 1, numVKnot - 2);

        // Create the Nurbs
        MStatus status;
        MObject nurbsData = fNurbsData.create();
        MObject nurbs     = fNurbs.create(mayaPositions,
                                          mayaUKnots, mayaVKnots,
                                          uDegree, vDegree,
                                          uForm,
                                          vForm,
                                          true,
                                          nurbsData,
                                          &status);
        if (status != MS::kSuccess || nurbs.isNull()) {
            // creation failure
            fNurbsData.setObject(MObject::kNullObj);
            fNurbs.setObject(MObject::kNullObj);
            return;
        }

        // Trim Nurbs
        if (fTrimNumLoopsCache.valid()) {
            unsigned int trimNumLoops = fTrimNumLoopsCache.getValue();
            // mayaV = offsetV - alembicV
            double startU, endU, startV, endV;
            fNurbs.getKnotDomain(startU, endU, startV, endV);
            double offsetV = startV + endV;

            MTrimBoundaryArray boundaryArray;

            const unsigned int* trimNumCurves = 
                    fTrimNumCurvesCache.getValue()->get();
            const unsigned int* trimNumVertices =
                    fTrimNumVerticesCache.getValue()->get();
            const unsigned int* trimOrder =
                    fTrimOrderCache.getValue()->get();
            const float* trimKnot = fTrimKnotCache.getValue()->get();
            const float* trimU    = fTrimUCache.getValue()->get();
            const float* trimV    = fTrimVCache.getValue()->get();
            const float* trimW    = fTrimWCache.getValue()->get();

            for (unsigned int i = 0; i < trimNumLoops; i++) {
                // Set up curves for each boundary
                unsigned int numCurves = *trimNumCurves;
                MObjectArray boundary(numCurves);

                for (unsigned int j = 0; j < numCurves; j++) {
                    // Set up one curve
                    unsigned int numVertices = *trimNumVertices;
                    unsigned int degree      = *trimOrder - 1;
                    unsigned int numKnots    = numVertices + degree + 1;

                    MPointArray controlPoints;
                    controlPoints.setLength(numVertices);
                    for (unsigned int k = 0; k < numVertices; k++) {
                        MPoint point(trimU[k], offsetV - trimV[k], 0, trimW[k]);
                        controlPoints[k] = point;
                    }

                    MDoubleArray knots(trimKnot + 1, numKnots - 2);

                    // Create the curve
                    MFnNurbsCurveData curveData;
                    MObject curveDataObject = curveData.create();

                    MFnNurbsCurve curve;
                    MObject curveObject  = curve.create(controlPoints, knots, degree,
                            MFnNurbsCurve::kOpen, true, true, curveDataObject, &status);
                    if (status == MS::kSuccess && !curveObject.isNull()) {
                        boundary[j] = curveDataObject;
                    }

                    // next curve
                    trimNumVertices++;
                    trimOrder++;
                    trimKnot += numKnots;
                    trimU    += numVertices;
                    trimV    += numVertices;
                    trimW    += numVertices;
                }

                boundaryArray.append(boundary);

                // next loop
                trimNumCurves++;
            }

            MTrimBoundaryArray oneRegion;
            for (unsigned int i = 0; i < boundaryArray.length(); i++) {
                if (i > 0) {
                    MObject loopData = boundaryArray.getMergedBoundary(i, &status);
                    if (status != MS::kSuccess) continue;

                    MFnNurbsCurve loop(loopData, &status);
                    if (status != MS::kSuccess) continue;

                    // Check whether this loop is an outer boundary.
                    bool isOuterBoundary = false;

                    double       length  = loop.length();
                    unsigned int segment = std::max(loop.numCVs(), 10);

                    MPointArray curvePoints;
                    curvePoints.setLength(segment);

                    for (unsigned int j = 0; j < segment; j++) {
                        double param = loop.findParamFromLength(length * j / segment);
                        loop.getPointAtParam(param, curvePoints[j]);
                    }

                    // Find the right most curve point
                    MPoint       rightMostPoint = curvePoints[0];
                    unsigned int rightMostIndex = 0;
                    for (unsigned int j = 0; j < curvePoints.length(); j++) {
                        if (rightMostPoint.x < curvePoints[j].x) {
                            rightMostPoint = curvePoints[j];
                            rightMostIndex = j;
                        }
                    }

                    // Find the vertex just before and after the right most vertex
                    unsigned int beforeIndex = (rightMostIndex == 0) ? curvePoints.length() - 1 : rightMostIndex - 1;
                    unsigned int afterIndex  = (rightMostIndex == curvePoints.length() - 1) ? 0 : rightMostIndex + 1;

                    for (unsigned int j = 0; j < curvePoints.length(); j++) {
                        if (fabs(curvePoints[beforeIndex].x - curvePoints[rightMostIndex].x) < 1e-5) {
                            beforeIndex = (beforeIndex == 0) ? curvePoints.length() - 1 : beforeIndex - 1;
                        }
                    }

                    for (unsigned int j = 0; j < curvePoints.length(); j++) {
                        if (fabs(curvePoints[afterIndex].x - curvePoints[rightMostIndex].x) < 1e-5) {
                            afterIndex = (afterIndex == curvePoints.length() - 1) ? 0 : afterIndex + 1;
                        }
                    }

                    // failed. not a closed curve.
                    if (fabs(curvePoints[afterIndex].x - curvePoints[rightMostIndex].x) < 1e-5 &&
                        fabs(curvePoints[beforeIndex].x - curvePoints[rightMostIndex].x) < 1e-5) {
                        continue;
                    }

                    // Compute the cross product
                    MVector vector1 = curvePoints[beforeIndex] - curvePoints[rightMostIndex];
                    MVector vector2 = curvePoints[afterIndex]  - curvePoints[rightMostIndex];
                    if ((vector1 ^ vector2).z < 0) {
                        isOuterBoundary = true;
                    }

                    // Trim the NURBS surface. An outer boundary starts a new region.
                    if (isOuterBoundary) {
                        status = fNurbs.trimWithBoundaries(oneRegion, false, 1e-3, 1e-5, true);
                        if (status != MS::kSuccess) {
                            fNurbsData.setObject(MObject::kNullObj);
                            fNurbs.setObject(MObject::kNullObj);
                            return;
                        }
                        oneRegion.clear();
                    }
                }

                oneRegion.append(boundaryArray[i]);
            }


            status = fNurbs.trimWithBoundaries(oneRegion, false, 1e-3, 1e-5, true);
            if (status != MS::kSuccess) {
                fNurbsData.setObject(MObject::kNullObj);
                fNurbs.setObject(MObject::kNullObj);
                return;
            }
        }
    }
    else {
        assert(!fNurbsData.object().isNull());

        if (positionsChanged) {
            fNurbs.setCVs(mayaPositions);
        }
    }
}

void NurbsTessellator::tessellate()
{
    if (!fSurfaceValid || fNurbsData.object().isNull()) {
        fPolyMeshData.setObject(MObject::kNullObj);
        fPolyMesh.setObject(MObject::kNullObj);
        return;
    }

    // Create the mesh data to own the mesh
    MObject polyMeshData = fPolyMeshData.create();

    // Set up parameters
    MTesselationParams params(
        MTesselationParams::kStandardFitFormat, MTesselationParams::kTriangles);

    // Tess the NURBS to triangles
    MStatus status;
    MObject polyObject = fNurbs.tesselate(params, polyMeshData, &status);
    if (status != MS::kSuccess || !polyObject.hasFn(MFn::kMesh)) {
        // tessellation failed
        fPolyMeshData.setObject(MObject::kNullObj);
        fPolyMesh.setObject(MObject::kNullObj);
        return;
    }

    status = fPolyMesh.setObject(polyObject);
    assert(status == MS::kSuccess);
}

void NurbsTessellator::convertToPoly()
{
    if (!fSurfaceValid || fPolyMeshData.object().isNull() ||
            fPolyMesh.numVertices() == 0 || fPolyMesh.numFaceVertices() == 0) {
        fTriangleIndices.reset();
        fWireIndices.reset();
        fPositions.reset();
        fNormals.reset();
        fUVs.reset();
        return;
    }
    
    MayaMeshExtractor<IndexBuffer::index_t> extractor(fPolyMeshData.object());
    extractor.setWantUVs(fNeedUVs);
    extractor.compute();

    fTriangleIndices = extractor.triangleIndices();
    fWireIndices     = extractor.wireIndices();
    fPositions       = extractor.positions();
    fNormals         = extractor.normals();
    fUVs.reset();
    if (fNeedUVs) {
        fUVs         = extractor.uvs();
    }
}


//==============================================================================
// CLASS SubDSmoother
//==============================================================================

SubDSmoother::SubDSmoother(
        Alembic::AbcGeom::ISubDSchema&  abcSubD,
        const bool                      needUVs)
    : PolyDataProvider(abcSubD, needUVs)
{
    // Face Indices
    fFaceIndicesCache.init(abcSubD.getFaceIndicesProperty());

    // Crease Edges
    Alembic::Abc::IInt32ArrayProperty creaseIndicesProp =
            abcSubD.getCreaseIndicesProperty();
    Alembic::Abc::IInt32ArrayProperty creaseLengthsProp =
            abcSubD.getCreaseLengthsProperty();
    Alembic::Abc::IFloatArrayProperty creaseSharpnessesProp = 
            abcSubD.getCreaseSharpnessesProperty();
    if (creaseIndicesProp.valid() &&
            creaseLengthsProp.valid() &&
            creaseSharpnessesProp.valid()) {
        fCreaseIndicesCache.init(creaseIndicesProp);
        fCreaseLengthsCache.init(creaseLengthsProp);
        fCreaseSharpnessesCache.init(creaseSharpnessesProp);
    }

    // Crease Vertices
    Alembic::Abc::IInt32ArrayProperty cornerIndicesProp = 
            abcSubD.getCornerIndicesProperty();
    Alembic::Abc::IFloatArrayProperty cornerSharpnessesProp =
            abcSubD.getCornerSharpnessesProperty();
    if (cornerIndicesProp.valid() && cornerSharpnessesProp.valid()) {
        fCornerIndicesCache.init(cornerIndicesProp);
        fCornerSharpnessesCache.init(cornerSharpnessesProp);
    }

    // Invisible Faces
    Alembic::Abc::IInt32ArrayProperty holesProp =
            abcSubD.getHolesProperty();
    if (holesProp.valid()) {
        fHolesCache.init(holesProp);
    }

    // UVs
    if (fNeedUVs) {
        Alembic::AbcGeom::IV2fGeomParam UVs = abcSubD.getUVsParam();
        if (UVs.valid()) {
            fUVsScope = UVs.getScope();
            if (fUVsScope == Alembic::AbcGeom::kVaryingScope ||
                    fUVsScope == Alembic::AbcGeom::kVertexScope ||
                    fUVsScope == Alembic::AbcGeom::kFacevaryingScope) {
                fUVsCache.init(UVs.getValueProperty());
                if (UVs.isIndexed()) {
                    fUVIndicesCache.init(UVs.getIndexProperty());
                }
            }
        }
    }
}

SubDSmoother::~SubDSmoother()
{
    // free the property readers
    fFaceIndicesCache.reset();

    fCreaseIndicesCache.reset();
    fCreaseLengthsCache.reset();
    fCreaseSharpnessesCache.reset();

    fCornerIndicesCache.reset();
    fCornerSharpnessesCache.reset();

    fHolesCache.reset();

    fUVsScope = Alembic::AbcGeom::kUnknownScope;
    fUVsCache.reset();
    fUVIndicesCache.reset();
}

bool SubDSmoother::valid() const
{
    return PolyDataProvider::valid() &&
            fFaceIndicesCache.valid();
}

boost::shared_ptr<const ShapeSample> 
SubDSmoother::getSample(double seconds)
{
    // empty mesh
    if (!fWireIndices || !fTriangleIndices) {
        boost::shared_ptr<ShapeSample> sample =
            ShapeSample::createEmptySample(seconds);
        return sample;
    }

    // triangle indices
    // Currently, we only have 1 group
    std::vector<boost::shared_ptr<IndexBuffer> > triangleVertIndices;
    triangleVertIndices.push_back(IndexBuffer::create(fTriangleIndices));


    boost::shared_ptr<ShapeSample> sample = ShapeSample::create(
        seconds,                                     // time (in seconds)
        fWireIndices->size() / 2,                    // number of wireframes
        fPositions->size() / 3,                      // number of vertices
        IndexBuffer::create(fWireIndices),           // wireframe indices
        triangleVertIndices,                         // triangle indices (1 group)
        VertexBuffer::createPositions(fPositions),   // position
        getBoundingBox(),                            // bounding box
        Config::kDefaultGrayColor,                   // diffuse color
        isVisible()
    );

    if (fNormals) {
        sample->setNormals(
            VertexBuffer::createNormals(fNormals));
    }

    if (fUVs) {
        sample->setUVs(
            VertexBuffer::createUVs(fUVs));
    }
    return sample;
}


TimeInterval SubDSmoother::updateCache(chrono_t time)
{
    // update faceCounts/position cache here so that we can detect topology/position change.
    // next setTime() in DataProvider::updateCache() simply returns early
    bool topologyChanged = fFaceCountsCache.setTime(time);
    bool positionChanged = fPositionsCache.setTime(time);

    TimeInterval validityInterval(PolyDataProvider::updateCache(time));

    // update caches
    topologyChanged = fFaceIndicesCache.setTime(time) || topologyChanged;

    bool creaseEdgeChanged = false;
    if (fCreaseSharpnessesCache.valid()) {
        creaseEdgeChanged = fCreaseIndicesCache.setTime(time) || creaseEdgeChanged;
        creaseEdgeChanged = fCreaseLengthsCache.setTime(time) || creaseEdgeChanged;
        creaseEdgeChanged = fCreaseSharpnessesCache.setTime(time) || creaseEdgeChanged;
    }

    bool creaseVertexChanged = false;
    if (fCornerSharpnessesCache.valid()) {
        creaseVertexChanged = fCornerIndicesCache.setTime(time) || creaseVertexChanged;
        creaseVertexChanged = fCornerSharpnessesCache.setTime(time) || creaseVertexChanged;
    }

    bool invisibleFaceChanged = false;
    if (fHolesCache.valid()) {
        invisibleFaceChanged = fHolesCache.setTime(time);
    }

    bool uvChanged = false;
    if (fUVsCache.valid()) {
        uvChanged = fUVsCache.setTime(time);
        if (fUVIndicesCache.valid()) {
            uvChanged = fUVIndicesCache.setTime(time) || uvChanged;
        }
    }

    // return the new cache valid interval
    validityInterval &= fFaceIndicesCache.getValidityInterval();

    if (fCreaseSharpnessesCache.valid()) {
        validityInterval &= fCreaseIndicesCache.getValidityInterval();
        validityInterval &= fCreaseLengthsCache.getValidityInterval();
        validityInterval &= fCreaseSharpnessesCache.getValidityInterval();
    }

    if (fCornerSharpnessesCache.valid()) {
        validityInterval &= fCornerIndicesCache.getValidityInterval();
        validityInterval &= fCornerSharpnessesCache.getValidityInterval();
    }

    if (fHolesCache.valid()) {
        validityInterval &= fHolesCache.getValidityInterval();
    }

    if (fUVsCache.valid()) {
        validityInterval &= fUVsCache.getValidityInterval();
        if (fUVIndicesCache.valid()) {
            validityInterval &= fUVIndicesCache.getValidityInterval();
        }
    }

    // do a minimal check for the consistency
    check();

    if (topologyChanged || creaseEdgeChanged || creaseVertexChanged ||
            invisibleFaceChanged || fSubDData.object().isNull()) {
        rebuildSubD();
        setCreaseEdges();
        setCreaseVertices();
        setInvisibleFaces();
        setUVs();
    }
    else {
        if (positionChanged) {
            setPositions();
        }
        if (uvChanged) {
            setUVs();
        }
    }

    if (isVisible()) {
        convertToPoly();
    }

    return validityInterval;
}

void SubDSmoother::check()
{
    size_t numFaceIndices = fFaceIndicesCache.getValue()->size();
    size_t numVerts       = fPositionsCache.getValue()->size() / 3;

    // UVs
    size_t numExpectedUVs = 0;
    if (fUVsScope == Alembic::AbcGeom::kVaryingScope ||
            fUVsScope == Alembic::AbcGeom::kVertexScope) {
        numExpectedUVs = numVerts;
    }
    else if (fUVsScope == Alembic::AbcGeom::kFacevaryingScope) {
        numExpectedUVs = numFaceIndices;
    }

    size_t numActualUVs = 0;
    if (fUVsCache.valid()) {
        if (fUVIndicesCache.valid()) {
            numActualUVs = fUVIndicesCache.getValue()->size();
        }
        else {
            numActualUVs = fUVsCache.getValue()->size() / 2;
        }
    }

    // clear previous result
    fCheckedUVsScope = Alembic::AbcGeom::kUnknownScope;
    fCheckedUVs.reset();
    fCheckedUVIndices.reset();

    // forward 
    if (numExpectedUVs == numActualUVs) {
        if (fUVsCache.valid()) {
            fCheckedUVsScope = fUVsScope;
            fCheckedUVs      = fUVsCache.getValue();
            if (fUVIndicesCache.valid()) {
                fCheckedUVIndices = fUVIndicesCache.getValue();
            }
        }
    }
    else {
        DisplayWarning(kBadUVsMsg);
    }
}

void SubDSmoother::rebuildSubD()
{
    // input data
    size_t numFaceCounts           = fFaceCountsCache.getValue()->size();
    const unsigned int* faceCounts = fFaceCountsCache.getValue()->get();

    size_t numFaceIndices                   = fFaceIndicesCache.getValue()->size();
    const IndexBuffer::index_t* faceIndices = fFaceIndicesCache.getValue()->get();

    size_t numPositions    = fPositionsCache.getValue()->size();
    const float* positions = fPositionsCache.getValue()->get();

    size_t numVertices = numPositions / 3;

    // Build Maya data structure
    MIntArray mayaCounts, mayaConnects;
    mayaCounts.setLength((unsigned int)numFaceCounts);
    mayaConnects.setLength((unsigned int)numFaceIndices);

    for (unsigned int i = 0, polyVertOffset = 0; i < numFaceCounts; i++) {
        unsigned int faceCount = mayaCounts[i] = faceCounts[i];
        
        for (unsigned int j = 0; j < faceCount; j++) {
            // Alembic's polygon winding is CW
            mayaConnects[polyVertOffset + j] = faceIndices[polyVertOffset + faceCount - j - 1];
        }

        polyVertOffset += faceCount;
    }

    
    MFloatPointArray mayaPositions;
    mayaPositions.setLength((unsigned int)numVertices);

    for (unsigned int i = 0; i < numVertices; i++) {
        mayaPositions[i] = MFloatPoint(positions[i * 3 + 0],
                                       positions[i * 3 + 1],
                                       positions[i * 3 + 2]);
    }

    // Create Maya mesh
    MStatus status;
    MObject subdData = fSubDData.create(&status);
    assert(status == MS::kSuccess);

    fSubD.setCheckSamePointTwice(false);
    MObject subd = fSubD.create((int)numVertices, (int)numFaceCounts,
            mayaPositions, mayaCounts, mayaConnects, subdData, &status);
    if (status != MS::kSuccess || subd.isNull()) {
        fSubDData.setObject(MObject::kNullObj);
        fSubD.setObject(MObject::kNullObj);
    }
}

void SubDSmoother::setPositions()
{
    if (fSubDData.object().isNull()) {
        return;
    }

    // input data
    size_t numPositions    = fPositionsCache.getValue()->size();
    const float* positions = fPositionsCache.getValue()->get();

    size_t numVertices = numPositions / 3;

    // Set vertex positions only
    MFloatPointArray mayaPositions;
    mayaPositions.setLength((unsigned int)numVertices);

    for (unsigned int i = 0; i < numVertices; i++) {
        mayaPositions[i] = MFloatPoint(positions[i * 3 + 0],
            positions[i * 3 + 1],
            positions[i * 3 + 2]);
    }

    fSubD.setPoints(mayaPositions);
}

void SubDSmoother::setCreaseEdges()
{
    if (fSubDData.object().isNull() ||
            !fCreaseIndicesCache.valid() ||
            !fCreaseLengthsCache.valid() ||
            !fCreaseSharpnessesCache.valid()) {
        return;
    }

    // input data
    size_t           numCreaseIndices = fCreaseIndicesCache.getValue()->size();
    const unsigned int* creaseIndices = fCreaseIndicesCache.getValue()->get();

    size_t           numCreaseLengths = fCreaseLengthsCache.getValue()->size();
    const unsigned int* creaseLengths = fCreaseLengthsCache.getValue()->get();

    size_t    numCreaseSharpnesses = fCreaseSharpnessesCache.getValue()->size();
    const float* creaseSharpnesses = fCreaseSharpnessesCache.getValue()->get();

    if (numCreaseSharpnesses == 0) {
        return;
    }

    // Prepare (startVertex, endVertex) => (edgeId) lookup map
    typedef boost::unordered_map<std::pair<int,int>,int> EdgeMap;
    EdgeMap edgeMap(size_t(fSubD.numEdges() / 0.75f));

    int numEdges = fSubD.numEdges();
    for (int i = 0; i < numEdges; i++) {
        int2 vertexList;
        fSubD.getEdgeVertices(i, vertexList);

        if (vertexList[0] > vertexList[1]) {
            std::swap(vertexList[0], vertexList[1]);
        }

        edgeMap.insert(std::make_pair(std::make_pair(vertexList[0], vertexList[1]), i));
    }

    // Fill Maya crease edges
    MUintArray   mayaEdgeIds;
    MDoubleArray mayaCreaseData;

    for (size_t i = 0, index = 0; i < numCreaseLengths && i < numCreaseSharpnesses; i++) {
        // length should always be 2
        unsigned int length    = creaseLengths[i];
        float        sharpness = creaseSharpnesses[i];

        if (length == 2 && index + length <= numCreaseIndices) {
            // find the edge ID from vertex ID
            std::pair<int,int> edge = std::make_pair(creaseIndices[index],creaseIndices[index+1]);
            if (edge.first > edge.second) {
                std::swap(edge.first, edge.second);
            }

            EdgeMap::iterator iter = edgeMap.find(edge);
            if (iter != edgeMap.end() && iter->second < numEdges) {
                // edge found, store it crease data
                mayaEdgeIds.append(iter->second);
                mayaCreaseData.append(sharpness);
            }
        }
        index += length;
    }

    // Set Maya crease edges
    MStatus status;
    status = fSubD.setCreaseEdges(mayaEdgeIds, mayaCreaseData);
    assert(status == MS::kSuccess);
}

void SubDSmoother::setCreaseVertices()
{
    if (fSubDData.object().isNull() ||
            !fCornerIndicesCache.valid() ||
            !fCornerSharpnessesCache.valid()) {
        return;
    }

    // input data
    size_t           numCornerIndices = fCornerIndicesCache.getValue()->size();
    const unsigned int* cornerIndices = fCornerIndicesCache.getValue()->get();

    size_t    numCornerSharpnesses = fCornerSharpnessesCache.getValue()->size();
    const float* cornerSharpnesses = fCornerSharpnessesCache.getValue()->get();

    if (cornerSharpnesses == 0) {
        return;
    }

    // Fill Maya crease vertices
    MUintArray   mayaVertexIds;
    MDoubleArray mayaCreaseData;

    size_t numCreaseVertices = std::min(numCornerIndices, numCornerSharpnesses);
    mayaVertexIds.setLength((unsigned int)numCreaseVertices);
    mayaCreaseData.setLength((unsigned int)numCreaseVertices);
    for (unsigned int i = 0; i < numCreaseVertices; i++) {
        mayaVertexIds[i]  = cornerIndices[i];
        mayaCreaseData[i] = cornerSharpnesses[i];
    }

    // Set Maya crease vertices
    MStatus status;
    status = fSubD.setCreaseVertices(mayaVertexIds, mayaCreaseData);
    assert(status == MS::kSuccess);
}

void SubDSmoother::setInvisibleFaces()
{
    if (fSubDData.object().isNull() ||
            !fHolesCache.valid()) {
        return;
    }

    // input data
    size_t           numHoles = fHolesCache.getValue()->size();
    const unsigned int* holes = fHolesCache.getValue()->get();

    if (numHoles == 0) {
        return;
    }

    // Fill Maya invisible faces
    MUintArray mayaFaceIds(holes, (unsigned int)numHoles);

    // Set Maya invisible faces
    MStatus status;
    status = fSubD.setInvisibleFaces(mayaFaceIds);
    assert(status == MS::kSuccess);
}

void SubDSmoother::setUVs()
{
    if (fSubDData.object().isNull()) {
        return;
    }

    // unsupported scope
    if (fCheckedUVsScope != Alembic::AbcGeom::kVaryingScope && 
            fCheckedUVsScope != Alembic::AbcGeom::kVertexScope &&
            fCheckedUVsScope != Alembic::AbcGeom::kFacevaryingScope) {
        return;
    }

    // no UVs
    if (!fCheckedUVs) {
        return;
    }

    // input data
    size_t           numFaceCounts = fFaceCountsCache.getValue()->size();
    const unsigned int* faceCounts = fFaceCountsCache.getValue()->get();

    size_t                      numFaceIndices = fFaceIndicesCache.getValue()->size();
    const IndexBuffer::index_t* faceIndices = fFaceIndicesCache.getValue()->get();

    size_t    numUVs = fCheckedUVs->size();
    const float* UVs = fCheckedUVs->get();

    const IndexBuffer::index_t* uvIndices = NULL;
    if (fCheckedUVIndices) {
        uvIndices    = fCheckedUVIndices->get();
    }

    // Clear Maya UVs if the number of UVs does not equal
    // MFnMesh::setUVs() only allow uv arrays equal or larger than current UV set size
    if (int(numUVs) != fSubD.numUVs()) {
        fSubD.clearUVs();
    }

    // no UVs, we are done 
    if (numUVs == 0) {
        return;
    }

    // Fill Maya UVs
    MFloatArray mayaUArray((unsigned int)numUVs);
    MFloatArray mayaVArray((unsigned int)numUVs);
    for (unsigned int i = 0; i < numUVs; i++) {
        mayaUArray[i] = UVs[i * 2 + 0];
        mayaVArray[i] = UVs[i * 2 + 1];
    }

    // Fill Maya UV indices
    MIntArray mayaUVCounts((unsigned int)numFaceCounts);
    MIntArray mayaUVIds((unsigned int)numFaceIndices);
    for (unsigned int i = 0, polyVertOffset = 0; i < numFaceCounts; i++) {
        unsigned int faceCount = mayaUVCounts[i] = faceCounts[i];

        for (unsigned int j = 0; j < faceCount; j++) {
            unsigned int uvIndex = 0;

            // Alembic's polygon winding is CW
            unsigned int polyVertIndex = polyVertOffset + faceCount - j - 1;
            if (fCheckedUVsScope == Alembic::AbcGeom::kVaryingScope ||
                    fCheckedUVsScope == Alembic::AbcGeom::kVertexScope) {
                // per-vertex UV
                unsigned int vertIndex = faceIndices[polyVertIndex];
                uvIndex = uvIndices ? uvIndices[vertIndex] : vertIndex;
            }
            else if (fCheckedUVsScope == Alembic::AbcGeom::kFacevaryingScope) {
                // per-face per-vertex UV
                uvIndex = uvIndices ? uvIndices[polyVertIndex] : polyVertIndex;
            }
            else {
                assert(0);
            }

            mayaUVIds[polyVertOffset + j] = uvIndex;
        }

        polyVertOffset += faceCount;
    }

    // Set Maya UVs and UV indices
    MStatus status;
    status = fSubD.setUVs(mayaUArray, mayaVArray);
    assert(status == MS::kSuccess);
    status = fSubD.assignUVs(mayaUVCounts, mayaUVIds);
    assert(status == MS::kSuccess);
}

void SubDSmoother::convertToPoly()
{
    if (fSubDData.object().isNull() ||
            fSubD.numVertices() == 0 || fSubD.numFaceVertices() == 0) {
        fTriangleIndices.reset();
        fWireIndices.reset();
        fPositions.reset();
        fNormals.reset();
        fUVs.reset();
        return;
    }

    // Smooth the subdivision mesh
    MFnMeshData smoothMeshData;
    MObject smoothMeshDataObj = smoothMeshData.create();
    
    MMeshSmoothOptions smoothOptions;
    smoothOptions.setDivisions(2);
    MObject smoothMeshObj = fSubD.generateSmoothMesh(smoothMeshDataObj, &smoothOptions);

    MayaMeshExtractor<IndexBuffer::index_t> extractor(smoothMeshDataObj);
    extractor.setWantUVs(fNeedUVs);
    extractor.compute();

    fTriangleIndices = extractor.triangleIndices();
    fWireIndices     = extractor.wireIndices();
    fPositions       = extractor.positions();
    fNormals         = extractor.normals();
    fUVs.reset();
    if (fNeedUVs) {
        fUVs         = extractor.uvs();
    }
}


//==============================================================================
// CLASS AlembicCacheObjectReader
//==============================================================================

AlembicCacheObjectReader::Ptr
AlembicCacheObjectReader::create(Alembic::Abc::IObject& abcObj, bool needUVs)
{
    CheckInterruptAndPause("reader initialization");

    // The object type can be mesh or nurbs.
    if (Alembic::AbcGeom::IPolyMesh::matches(abcObj.getHeader()) ||
        Alembic::AbcGeom::INuPatch::matches(abcObj.getHeader()) ||
        Alembic::AbcGeom::ISubD::matches(abcObj.getHeader())) {
        Ptr reader = boost::make_shared<AlembicCacheMeshReader>(abcObj, needUVs);
        return reader->valid() ? reader : Ptr();
    }

    // or an xform...
    if (Alembic::AbcGeom::IXform::matches(abcObj.getHeader())) {
        Ptr reader = boost::make_shared<AlembicCacheXformReader>(abcObj, needUVs);
        return reader->valid() ? reader : Ptr();
    }

    return Ptr();
}

AlembicCacheObjectReader::~AlembicCacheObjectReader()
{}


//==============================================================================
// CLASS AlembicCacheTopReader
//==============================================================================

AlembicCacheTopReader::AlembicCacheTopReader(
    Alembic::Abc::IObject abcObj,
    const bool needUVs
)
 : fBoundingBoxValidityInterval(TimeInterval::kInvalid)
{
    fXformData = XformData::create();

    size_t numChildren = abcObj.getNumChildren();
    for (size_t ii=0; ii<numChildren; ++ii)
    {
        Alembic::Abc::IObject child(abcObj, abcObj.getChildHeader(ii).getName());
        Ptr childReader = create(child, needUVs);
        if (childReader)
            fChildren.push_back(childReader);
    }

    // Compute the exact animation time range
    TimeInterval animTimeRange(TimeInterval::kInvalid);
    BOOST_FOREACH(const AlembicCacheObjectReader::Ptr& childReader, fChildren) {
        animTimeRange |= childReader->getAnimTimeRange();
    }
    fXformData->setAnimTimeRange(animTimeRange);
}

AlembicCacheTopReader::~AlembicCacheTopReader()
{}

bool AlembicCacheTopReader::valid() const
{
    return true;
}

TimeInterval AlembicCacheTopReader::sampleHierarchy(double seconds, 
    const MMatrix& rootMatrix, TimeInterval rootMatrixInterval)
{
    TimeInterval validityInterval(TimeInterval::kInfinite);

    MBoundingBox bbox;
    TimeInterval bboxValIntrvl(TimeInterval::kInfinite);

    BOOST_FOREACH(const AlembicCacheObjectReader::Ptr& childReader, fChildren) {
        validityInterval &= childReader->sampleHierarchy(seconds, 
                rootMatrix, rootMatrixInterval);

        bbox.expand(childReader->getBoundingBox());
        bboxValIntrvl &= childReader->getBoundingBoxValidityInterval();
    }

    // The computed validity interval must contain the current time.
    assert(validityInterval.contains(seconds));

    // The current and previous bounding box intervals are either
    // disjoint or equal.
    assert(!(fBoundingBoxValidityInterval & bboxValIntrvl).valid() ||
            fBoundingBoxValidityInterval == bboxValIntrvl);

    if (seconds == bboxValIntrvl.startTime()) {
        fBoundingBox                 = bbox;
        fBoundingBoxValidityInterval = bboxValIntrvl;

        boost::shared_ptr<GPUCache::XformSample> sample =
            GPUCache::XformSample::create(
                seconds,
                MMatrix::identity,
                fBoundingBox,
                true
            );
        fXformData->addSample(sample);
    }

    return validityInterval;
}

TimeInterval AlembicCacheTopReader::sampleShape(double seconds)
{
    // Top reader has no shape data!
    assert(0);
    return TimeInterval(TimeInterval::kInvalid);
}

SubNode::MPtr AlembicCacheTopReader::get() const
{
    SubNode::MPtr node =
        SubNode::create(MString("|"), fXformData);

    BOOST_FOREACH(const AlembicCacheObjectReader::Ptr& childReader, fChildren) {
        SubNode::MPtr child = childReader->get();
        if (child) 
            SubNode::connect(node, child);
    }

    if (node->getChildren().empty()) {
        return SubNode::MPtr();
    }
    
    return node;
}

MBoundingBox AlembicCacheTopReader::getBoundingBox() const
{
    return fBoundingBox;
}

TimeInterval AlembicCacheTopReader::getBoundingBoxValidityInterval() const
{
    return fBoundingBoxValidityInterval;
}

TimeInterval AlembicCacheTopReader::getAnimTimeRange() const
{
    return fXformData->animTimeRange();
}

void AlembicCacheTopReader::saveAndReset(AlembicCacheReader& cacheReader)
{
    // We don't save xform readers. Just call children's saveAndReset().
    BOOST_FOREACH (const AlembicCacheObjectReader::Ptr& childReader, fChildren) {
        childReader->saveAndReset(cacheReader);
    }
}


//==============================================================================
// CLASS AlembicCacheXformReader
//==============================================================================

AlembicCacheXformReader::AlembicCacheXformReader(
    Alembic::Abc::IObject abcObj,
    const bool needUVs
)
    : fName(abcObj.getName()),
      fValidityInterval(TimeInterval::kInvalid),
      fBoundingBoxValidityInterval(TimeInterval::kInvalid)
{
    Alembic::AbcGeom::IXform xform(abcObj, Alembic::Abc::kWrapExisting);

    // Xform schema
    Alembic::AbcGeom::IXformSchema schema = xform.getSchema();
        
    // transform
    fXformCache.init(schema);

    // transform visibility
    Alembic::AbcGeom::IVisibilityProperty visibility = 
                    Alembic::AbcGeom::GetVisibilityProperty(abcObj);
    if (visibility) {
        fVisibilityCache.init(visibility);
    }

    fXformData = XformData::create();

    const size_t numChildren = abcObj.getNumChildren();
    for (size_t ii=0; ii<numChildren; ++ii)
    {
        Alembic::Abc::IObject child(abcObj, abcObj.getChildHeader(ii).getName());
        Ptr childReader = create(child, needUVs);
        if (childReader)
            fChildren.push_back(childReader);
    }

    // Compute the exact animation time range
    Alembic::Abc::TimeSamplingPtr timeSampling = schema.getTimeSampling();
    size_t numSamples = schema.getNumSamples();

    TimeInterval animTimeRange(
        timeSampling->getSampleTime(0),
        timeSampling->getSampleTime(numSamples > 0 ? numSamples-1 : 0) );

    BOOST_FOREACH(const AlembicCacheObjectReader::Ptr& childReader, fChildren) {
        animTimeRange |= childReader->getAnimTimeRange();
    }
    fXformData->setAnimTimeRange(animTimeRange);
}

AlembicCacheXformReader::~AlembicCacheXformReader()
{}

bool AlembicCacheXformReader::valid() const
{
    return fXformCache.valid();
}

TimeInterval AlembicCacheXformReader::sampleHierarchy(double seconds, 
    const MMatrix& rootMatrix, TimeInterval rootMatrixInterval)
{
    // Fill the sample if this sample has not been read
    if (!fValidityInterval.contains(seconds)) {
        fillTopoAndAttrSample(seconds);
    }

    // Inherit transformation
    MMatrix newRootMatrix = fXformCache.getValue() * rootMatrix;
    TimeInterval newRootMatrixInterval = 
        fXformCache.getValidityInterval() & rootMatrixInterval;


    TimeInterval validityInterval = fValidityInterval;

    MBoundingBox bbox;
    TimeInterval bboxValIntrvl(TimeInterval::kInfinite);

    BOOST_FOREACH(const AlembicCacheObjectReader::Ptr& childReader, fChildren) {
        validityInterval &= childReader->sampleHierarchy(seconds, 
            newRootMatrix, newRootMatrixInterval);

        bbox.expand(childReader->getBoundingBox());
        bboxValIntrvl &= childReader->getBoundingBoxValidityInterval();
    }

    // The computed validity interval must contain the current time.
    assert(validityInterval.contains(seconds));

    // The current and previous bounding box intervals are either
    // disjoint or equal.
    assert(!(fBoundingBoxValidityInterval & bboxValIntrvl).valid() ||
            fBoundingBoxValidityInterval == bboxValIntrvl);

    if (seconds == (fValidityInterval & bboxValIntrvl).startTime()) {
        fBoundingBox                 = bbox;
        fBoundingBoxValidityInterval = bboxValIntrvl;

        boost::shared_ptr<GPUCache::XformSample> sample =
            GPUCache::XformSample::create(
                seconds,
                fXformCache.getValue(),
                fBoundingBox,
                isVisible()
            );
        fXformData->addSample(sample);
    }

    return validityInterval;
}

TimeInterval AlembicCacheXformReader::sampleShape(double seconds)
{
    // Transform reader has no shape data!
    assert(0);
    return TimeInterval(TimeInterval::kInvalid);
}

SubNode::MPtr AlembicCacheXformReader::get() const
{
    SubNode::MPtr node =
        SubNode::create(MString(fName.c_str()), fXformData);

    BOOST_FOREACH(const AlembicCacheObjectReader::Ptr& childReader, fChildren) {
        SubNode::MPtr child = childReader->get();
        if (child) 
            SubNode::connect(node, child);
    }

    if (node->getChildren().empty()) {
        return SubNode::MPtr();
    }

    return node;
}
        

void AlembicCacheXformReader::fillTopoAndAttrSample(chrono_t time)
{
    // Notes:
    //
    // When possible, we try to reuse the samples from the previously
    // read sample.

    // update caches
    fXformCache.setTime(time);
    if (fVisibilityCache.valid()) {
        fVisibilityCache.setTime(time);
    }

    // return the new cache valid interval
    TimeInterval validityInterval(TimeInterval::kInfinite);
    validityInterval &= fXformCache.getValidityInterval();
    if (fVisibilityCache.valid()) {
        validityInterval &= fVisibilityCache.getValidityInterval();
    }
    assert(validityInterval.valid());

    fValidityInterval = validityInterval;
}

bool AlembicCacheXformReader::isVisible() const
{
    // xform invisible
    if (fVisibilityCache.valid() && 
            fVisibilityCache.getValue() == char(Alembic::AbcGeom::kVisibilityHidden)) {
        return false;
    }

    // visible
    return true;
}

MBoundingBox AlembicCacheXformReader::getBoundingBox() const
{
    return fBoundingBox;
}

TimeInterval AlembicCacheXformReader::getBoundingBoxValidityInterval() const
{
    return fBoundingBoxValidityInterval;
}

TimeInterval AlembicCacheXformReader::getAnimTimeRange() const
{
    return fXformData->animTimeRange();
}

void AlembicCacheXformReader::saveAndReset(AlembicCacheReader& cacheReader)
{
    // We don't save xform readers. Just call children's saveAndReset().
    BOOST_FOREACH (const AlembicCacheObjectReader::Ptr& childReader, fChildren) {
        childReader->saveAndReset(cacheReader);
    }
}


//==============================================================================
// CLASS AlembicCacheMeshReader
//==============================================================================

AlembicCacheMeshReader::AlembicCacheMeshReader(
    Alembic::Abc::IObject object,
    const bool needUVs
) 
    : fName(object.getName()),
      fFullName(object.getFullName()),
      fBoundingBoxValidityInterval(TimeInterval::kInvalid),
      fNumTransparentSample(0)
{
    // Shape schema
    if (Alembic::AbcGeom::IPolyMesh::matches(object.getHeader())) {
        Alembic::AbcGeom::IPolyMesh       meshObj(object, Alembic::Abc::kWrapExisting);
        Alembic::AbcGeom::IPolyMeshSchema schema = meshObj.getSchema();

        // check the existence of wireframe index property
        // if the mesh is written by gpuCache command, the wireframe index property must exist
        if (schema.getPropertyHeader(kCustomPropertyWireIndices) != NULL ||
            schema.getPropertyHeader(kCustomPropertyWireIndicesOld) != NULL) {
            fDataProvider.reset(new RawDataProvider(schema, needUVs));
        }
        else {
            fDataProvider.reset(new Triangulator(schema, needUVs));
        }
    }
    else if (Alembic::AbcGeom::INuPatch::matches(object.getHeader())) {
        Alembic::AbcGeom::INuPatch       nurbsObj(object, Alembic::Abc::kWrapExisting);
        Alembic::AbcGeom::INuPatchSchema schema = nurbsObj.getSchema();

        fDataProvider.reset(new NurbsTessellator(schema, needUVs));
    }
    else if (Alembic::AbcGeom::ISubD::matches(object.getHeader())) {
        Alembic::AbcGeom::ISubD       subdObj(object, Alembic::Abc::kWrapExisting);
        Alembic::AbcGeom::ISubDSchema schema = subdObj.getSchema();

        fDataProvider.reset(new SubDSmoother(schema, needUVs));
    }
    else {
        DisplayWarning(kUnsupportedGeomMsg);
    }

    fShapeData = ShapeData::create();    
    fShapeData->setAnimTimeRange(fDataProvider->getAnimTimeRange());

    // Whole object material assignment
    MString material;

    std::string materialAssignmentPath;
    if (Alembic::AbcMaterial::getMaterialAssignmentPath(
            object, materialAssignmentPath)) {
        // We assume all materials are stored in "/materials"
        std::string prefix = "/";
        prefix += kMaterialsObject;
        prefix += "/";

        if (std::equal(prefix.begin(), prefix.end(), materialAssignmentPath.begin())) {
            std::string objectName = materialAssignmentPath.substr(prefix.size()).c_str();
            // No material inheritance here.
            if (objectName.find("/") == std::string::npos) {
                material = objectName.c_str();
            }
        }
    }

    if (material.length() > 0) {
        fShapeData->setMaterial(material);
    }
}

AlembicCacheMeshReader::~AlembicCacheMeshReader()
{
    fDataProvider.reset();
}

bool AlembicCacheMeshReader::valid() const
{
    return fDataProvider && fDataProvider->valid();
}

TimeInterval AlembicCacheMeshReader::sampleHierarchy(double seconds, 
    const MMatrix& rootMatrix, TimeInterval rootMatrixInterval)
{
    CheckInterruptAndPause("sampling hierarchy");

    // Fill the sample if this sample has not been read
    if (!fDataProvider->getBBoxAndVisValidityInterval().contains(seconds)) {
        // Read minimal data to construct the hierarchy
        fDataProvider->fillBBoxAndVisSample(seconds);
    }

    TimeInterval validityInterval = fDataProvider->getBBoxAndVisValidityInterval();

    // Compute bounding box in root sub-node axis
    fBoundingBox = fDataProvider->getBoundingBox();
    fBoundingBox.transformUsing(rootMatrix);
    fBoundingBoxValidityInterval = rootMatrixInterval &
        fDataProvider->getBoundingBoxValidityInterval();

    // We only add the sample if it is the first sample of the
    // interval.
    if (seconds == validityInterval.startTime()) {
        boost::shared_ptr<const ShapeSample> sample = 
            fDataProvider->getBBoxPlaceHolderSample(seconds);
        fShapeData->addSample(sample);
    }

    return validityInterval;
}

TimeInterval AlembicCacheMeshReader::sampleShape(double seconds)
{
    CheckInterruptAndPause("sampling shape");

    // Fill the sample if this sample has not been read
    if (!fDataProvider->getValidityInterval().contains(seconds)) {
        fDataProvider->fillTopoAndAttrSample(seconds);
    }

    TimeInterval validityInterval = fDataProvider->getValidityInterval();

    // We only add the sample if it is the first sample of the
    // interval.
    if (seconds == validityInterval.startTime()) {
        if (fDataProvider->isVisible()) {
            boost::shared_ptr<const ShapeSample> sample = 
                fDataProvider->getSample(seconds);
            fShapeData->addSample(sample);

            float alpha = sample->diffuseColor()[3];
            if (alpha > 0.0f && alpha < 1.0f) {
                fNumTransparentSample++;
            }
        }
        else {
            // hidden geometry, simply append an empty sample
            boost::shared_ptr<ShapeSample> sample =
                ShapeSample::createEmptySample(seconds);
            fShapeData->addSample(sample);
        }
    }

    return validityInterval;
}

SubNode::MPtr AlembicCacheMeshReader::get() const
{
    if (fShapeData->getSamples().size() == 1 &&
        !fShapeData->getSamples().begin()->second->visibility()) {
        // Prune the node entirely if it is hidden.
        return SubNode::MPtr();
    }
    
    SubNode::MPtr subNode = SubNode::create(MString(fName.c_str()), fShapeData);
    if (fNumTransparentSample == 0) {
        subNode->setTransparentType(SubNode::kOpaque);
    }
    else if (fNumTransparentSample == fShapeData->getSamples().size()) {
        subNode->setTransparentType(SubNode::kTransparent);
    }
    else {
        subNode->setTransparentType(SubNode::kOpaqueAndTransparent);
    }
    return subNode;
}
        
MBoundingBox AlembicCacheMeshReader::getBoundingBox() const
{
    return fBoundingBox;
}

TimeInterval AlembicCacheMeshReader::getBoundingBoxValidityInterval() const
{
    return fBoundingBoxValidityInterval;
}

TimeInterval AlembicCacheMeshReader::getAnimTimeRange() const
{
    return fShapeData->animTimeRange();
}

void AlembicCacheMeshReader::saveAndReset(AlembicCacheReader& cacheReader)
{
    // Clear the content of this reader for reuse.
    fBoundingBox.clear();
    fBoundingBoxValidityInterval = TimeInterval(TimeInterval::kInvalid);
    fNumTransparentSample = 0;

    // Create a new shape data.
    ShapeData::MPtr newShapeData = ShapeData::create();

    // Animation time range and material assignment won't change so just copy them.
    newShapeData->setAnimTimeRange(fShapeData->animTimeRange());
    newShapeData->setMaterials(fShapeData->getMaterials());

    // Release the reference to the old shape data to avoid instability.
    fShapeData = newShapeData;

    Ptr thisPtr = shared_from_this();
    cacheReader.saveReader(fFullName, thisPtr);
}


//==============================================================================
// CLASS AlembicCacheMaterialReader
//==============================================================================

AlembicCacheMaterialReader::AlembicCacheMaterialReader(Alembic::Abc::IObject abcObj)
    : fName(abcObj.getName()),
      fValidityInterval(TimeInterval::kInvalid)
{
    // Wrap with IMaterial
    Alembic::AbcMaterial::IMaterial material(abcObj, Alembic::Abc::kWrapExisting);

    // Material schema
    Alembic::AbcMaterial::IMaterialSchema schema = material.getSchema();

    // Create the material graph
    fMaterialGraph = boost::make_shared<MaterialGraph>(MString(fName.c_str()));

    // The number of nodes in the material
    size_t numNetworkNodes = schema.getNumNetworkNodes();

    // Map: name -> (IMaterialSchema::NetworkNode,MaterialNode)
    typedef std::pair<Alembic::AbcMaterial::IMaterialSchema::NetworkNode,MaterialNode::MPtr> NodePair;
    typedef boost::unordered_map<std::string,NodePair> NodeMap;
    NodeMap nodeMap;

    // Read nodes
    for (size_t i = 0; i < numNetworkNodes; i++) {
        Alembic::AbcMaterial::IMaterialSchema::NetworkNode abcNode = schema.getNetworkNode(i);

        std::string target;
        if (!abcNode.valid() || !abcNode.getTarget(target) || target != kMaterialsGpuCacheTarget) {
            continue;  // Invalid node
        }

        std::string type;
        if (!abcNode.getNodeType(type) || type.empty()) {
            continue;  // Invalid type
        }

        // Node name
        std::string name = abcNode.getName();
        assert(!name.empty());

        // Create material node
        MaterialNode::MPtr node = MaterialNode::create(name.c_str(), type.c_str());
        assert(node);

        fMaterialGraph->addNode(node);
        nodeMap.insert(std::make_pair(name, std::make_pair(abcNode, node)));
    }

    // Initialize property caches.
    BOOST_FOREACH (NodeMap::value_type& val, nodeMap) {
        Alembic::AbcMaterial::IMaterialSchema::NetworkNode& abcNode = val.second.first;
        MaterialNode::MPtr& node = val.second.second;

        // Loop over all child properties
        Alembic::Abc::ICompoundProperty compoundProp = abcNode.getParameters();
        size_t numProps = compoundProp.getNumProperties();
        for (size_t i = 0; i < numProps; i++) {
            const Alembic::Abc::PropertyHeader& header = compoundProp.getPropertyHeader(i);
            const std::string propName = header.getName();

            if (Alembic::Abc::IBoolProperty::matches(header)) {
                fBoolCaches.push_back(
                    ScalarMaterialProp<Alembic::Abc::IBoolProperty>(compoundProp, propName, node));
            }
            else if (Alembic::Abc::IInt32Property::matches(header)) {
                fInt32Caches.push_back(
                    ScalarMaterialProp<Alembic::Abc::IInt32Property>(compoundProp, propName, node));
            }
            else if (Alembic::Abc::IFloatProperty::matches(header)) {
                fFloatCaches.push_back(
                    ScalarMaterialProp<Alembic::Abc::IFloatProperty>(compoundProp, propName, node));
            }
            else if (Alembic::Abc::IV2fProperty::matches(header)) {
                fFloat2Caches.push_back(
                    ScalarMaterialProp<Alembic::Abc::IV2fProperty>(compoundProp, propName, node));
            }
            else if (Alembic::Abc::IV3fProperty::matches(header)) {
                fFloat3Caches.push_back(
                    ScalarMaterialProp<Alembic::Abc::IV3fProperty>(compoundProp, propName, node));
            }
            else if (Alembic::Abc::IC3fProperty::matches(header)) {
                fRGBCaches.push_back(
                    ScalarMaterialProp<Alembic::Abc::IC3fProperty>(compoundProp, propName, node));
            }
            else if (Alembic::Abc::IWstringProperty::matches(header)) {
                fStringCaches.push_back(
                    ScalarMaterialProp<Alembic::Abc::IWstringProperty>(compoundProp, propName, node));
            }
        }
    }

    // Read connections
    BOOST_FOREACH (NodeMap::value_type& val, nodeMap) {
        Alembic::AbcMaterial::IMaterialSchema::NetworkNode& abcNode = val.second.first;
        MaterialNode::MPtr& node = val.second.second;

        // Loop over the connections and connect properties
        size_t numConnections = abcNode.getNumConnections();
        for (size_t i = 0; i < numConnections; i++) {
            std::string inputName, connectedNodeName, connectedOutputName;
            abcNode.getConnection(i, inputName, connectedNodeName, connectedOutputName);

            // Find destination property
            MaterialProperty::MPtr prop = node->findProperty(inputName.c_str());

            // Find source node
            MaterialNode::MPtr srcNode;
            NodeMap::iterator it = nodeMap.find(connectedNodeName);
            if (it != nodeMap.end()) {
                srcNode = (*it).second.second;
            }

            // Find source property
            MaterialProperty::MPtr srcProp;
            if (srcNode) {
                srcProp = srcNode->findProperty(connectedOutputName.c_str());
            }

            // Make the connection
            if (prop && srcNode && srcProp) {
                prop->connect(srcNode, srcProp);
            }
        }
    }

    // Read Terminal node (ignore output)
    std::string rootNodeName, rootOutput;
    if (schema.getNetworkTerminal(kMaterialsGpuCacheTarget, kMaterialsGpuCacheType, rootNodeName, rootOutput)) {
        NodeMap::iterator it = nodeMap.find(rootNodeName);
        if (it != nodeMap.end()) {
            fMaterialGraph->setRootNode((*it).second.second);
        }
    }
}

AlembicCacheMaterialReader::~AlembicCacheMaterialReader()
{}

TimeInterval AlembicCacheMaterialReader::sampleMaterial(double seconds)
{
    TimeInterval validityInterval(TimeInterval::kInfinite);

    BOOST_FOREACH (ScalarMaterialProp<Alembic::Abc::IBoolProperty>& cache, fBoolCaches) {
        validityInterval &= cache.sample(seconds);
    }

    BOOST_FOREACH (ScalarMaterialProp<Alembic::Abc::IInt32Property>& cache, fInt32Caches) {
        validityInterval &= cache.sample(seconds);
    }

    BOOST_FOREACH (ScalarMaterialProp<Alembic::Abc::IFloatProperty>& cache, fFloatCaches) {
        validityInterval &= cache.sample(seconds);
    }

    BOOST_FOREACH (ScalarMaterialProp<Alembic::Abc::IV2fProperty>& cache, fFloat2Caches) {
        validityInterval &= cache.sample(seconds);
    }

    BOOST_FOREACH (ScalarMaterialProp<Alembic::Abc::IV3fProperty>& cache, fFloat3Caches) {
        validityInterval &= cache.sample(seconds);
    }

    BOOST_FOREACH (ScalarMaterialProp<Alembic::Abc::IC3fProperty>& cache, fRGBCaches) {
        validityInterval &= cache.sample(seconds);
    }

    BOOST_FOREACH (ScalarMaterialProp<Alembic::Abc::IWstringProperty>& cache, fStringCaches) {
        validityInterval &= cache.sample(seconds);
    }

    return validityInterval;
}

MaterialGraph::MPtr AlembicCacheMaterialReader::get() const
{
    // Check invalid graph.
    if (!fMaterialGraph || !fMaterialGraph->rootNode() || fMaterialGraph->getNodes().empty()) {
        return MaterialGraph::MPtr();
    }
    return fMaterialGraph;
}

} // namespace CacheReaderAlembicPrivate


//==============================================================================
// CLASS AlembicCacheReader
//==============================================================================

boost::shared_ptr<CacheReader> AlembicCacheReader::create(const MFileObject& file)
{
    return boost::make_shared<AlembicCacheReader>(file);
}

AlembicCacheReader::AlembicCacheReader(const MFileObject& file)
    : fFile(file)
{
    // Open the archive for reading.
    MString resolvedFullName = file.resolvedFullName();

    try {
        tbb::mutex::scoped_lock alembicLock(gsAlembicMutex);

        if (resolvedFullName.length() != 0 && std::ifstream(resolvedFullName.asChar()).good()) {
            Alembic::AbcCoreFactory::IFactory factory;
            // Disable Alembic caching as we have implemented our own
            // caching...
            factory.setSampleCache( Alembic::AbcCoreAbstract::ReadArraySampleCachePtr());
            factory.setPolicy(Alembic::Abc::ErrorHandler::kThrowPolicy);
            fAbcArchive = factory.getArchive(resolvedFullName.asChar());

            // File exists but Alembic fails to open.
            if (!fAbcArchive.valid()) {
                DisplayError(kFileFormatWrongMsg, file.rawFullName());
            }
        }
        else {
            // File doesn't exist.
            DisplayError(kFileDoesntExistMsg, file.rawFullName());
        }
    }
    catch (CacheReaderInterruptException& ex) {
        // pass upward
        throw ex;
    }
    catch (std::exception& ex) {
        //The resolved full name will be empty if the resolution fails.
        //Print the raw full name in case of this situation.
        DisplayError(kCacheOpenFileErrorMsg, file.rawFullName(), ex.what());
    }
}

AlembicCacheReader::~AlembicCacheReader()
{
    try {
        tbb::mutex::scoped_lock alembicLock(gsAlembicMutex);
        fAbcArchive.reset();
    }
    catch (CacheReaderInterruptException& ex) {
        // pass upward
        throw ex;
    }
    catch (std::exception& ex) {
        DisplayError(kCloseFileErrorMsg, fFile.resolvedFullName(), ex.what());
    }
}

bool AlembicCacheReader::valid() const
{
    tbb::mutex::scoped_lock alembicLock(gsAlembicMutex);
    return fAbcArchive.valid();
}

bool AlembicCacheReader::validateGeomPath(
    const MString& geomPath, MString& validatedGeomPath) const
{
    if (!valid()) {
        validatedGeomPath = MString("|");
        return false;
    }

    try {
        tbb::mutex::scoped_lock alembicLock(gsAlembicMutex);

        // path: |xform1|xform2|meshShape
        MStringArray pathArray;
        geomPath.split('|', pathArray);

        bool valid = true;
        
        // find the mesh in Alembic archive
        validatedGeomPath = MString();
        Alembic::Abc::IObject current = fAbcArchive.getTop();
        for (unsigned int i = 0; i < pathArray.length(); i++) {
            MString step = pathArray[i];
            current = current.getChild(step.asChar());
            if (!current.valid()) {
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
    catch (CacheReaderInterruptException& ex) {
        // pass upward
        throw ex;
    }
    catch (std::exception& ex) {
        DisplayError(kReadMeshErrorMsg, fFile.resolvedFullName(), geomPath, ex.what());

        validatedGeomPath = MString("|");
        return false;
    }
}


SubNode::Ptr AlembicCacheReader::readScene(
    const MString& geomPath, bool needUVs)
{
    // Read sub-node hierarchy
    SubNode::Ptr top = readHierarchy(geomPath, needUVs);
    if (!top) return SubNode::Ptr();

    // Extract shape paths
    ShapePathVisitor::ShapePathAndSubNodeList shapeGeomPaths;
    ShapePathVisitor shapePathVisitor(shapeGeomPaths);
    top->accept(shapePathVisitor);

    // The absolute shape path in the archive is prefix+shapePath
    MString prefix;
    int lastStep = geomPath.rindexW('|');
    if (lastStep > 0) {
        prefix = geomPath.substringW(0, lastStep - 1);
    }

    // Read shapes
    BOOST_FOREACH (const ShapePathVisitor::ShapePathAndSubNode& pair, shapeGeomPaths) {
        SubNode::Ptr shape = readShape(prefix + pair.first, needUVs);
        if (shape && pair.first.length() > 0) {
            ReplaceSubNodeData(top, shape, pair.first);
        }
    }

    // Update transparent type
    SubNodeTransparentTypeVisitor transparentTypeVisitor;
    top->accept(transparentTypeVisitor);

    return top;
}

SubNode::Ptr AlembicCacheReader::readHierarchy(
    const MString& geomPath, bool needUVs)
{
    using namespace CacheReaderAlembicPrivate;

    if (!valid()) return SubNode::Ptr();

    try {
        tbb::mutex::scoped_lock alembicLock(gsAlembicMutex);

        // path: |xform1|xform2|meshShape
        MStringArray pathArray;
        geomPath.split('|', pathArray);

        Alembic::Abc::IObject current = fAbcArchive.getTop();
        AlembicCacheObjectReader::Ptr reader;
        if (pathArray.length() == 0) {
            // Determine the number of children under the top level object.
            // We skip objects that we don't recognize. (Cameras, Materials, ..)
            size_t numChildren = 0;
            size_t lastChild   = 0;
            for (size_t i = 0; i < current.getNumChildren(); i++) {
                if (Alembic::AbcGeom::IPolyMesh::matches(current.getChildHeader(i)) ||
                    Alembic::AbcGeom::INuPatch::matches(current.getChildHeader(i)) ||
                    Alembic::AbcGeom::ISubD::matches(current.getChildHeader(i)) ||
                    Alembic::AbcGeom::IXform::matches(current.getChildHeader(i))) {
                        numChildren++;
                        lastChild = i;
                }
            }

            if (numChildren == 1) {
                current = Alembic::Abc::IObject(
                    current, current.getChildHeader(lastChild).getName());
                if (current.valid())
                    reader = AlembicCacheObjectReader::create(
                    current, needUVs);
            }
            else if (numChildren > 1) {
                // The top level object is not a proper xform object. We
                // therefore have to create a dummy top-level transform in
                // that case.
                reader = boost::make_shared<AlembicCacheTopReader>(
                    current, needUVs);
            }
        }
        else {
            // find the top level node in the Alembic archive
            bool geometryFound = true;
            for (unsigned int i = 0; i < pathArray.length(); i++) {
                MString step = pathArray[i];
                current = current.getChild(step.asChar());
                if (!current.valid()) {
                    geometryFound = false;
                    break;
                }
            }

            if (geometryFound)
                reader = AlembicCacheObjectReader::create(current, needUVs);
        }

        if (!reader || !reader->valid()) return SubNode::Ptr();

        // Each time samplings only records the start time, i.e. there
        // is no way to ask for the end time of a TimeSampling!
        // Therefore, to determine the end of the animation, we simply
        // loop until time no longer advances...
        {
            TimeInterval interval = reader->sampleHierarchy(
                -std::numeric_limits<double>::max(),
                MMatrix::identity, TimeInterval::kInfinite);
            while (interval.endTime() != std::numeric_limits<double>::max()) {
                interval = reader->sampleHierarchy(
                    interval.endTime(), 
                    MMatrix::identity, TimeInterval::kInfinite);
            }
        }

        // The sub-node hierarchy with bounding box place holders.
        SubNode::Ptr top = reader->get();

        // Save the object readers for reuse.
        reader->saveAndReset(*this);

        return top;
    }
    catch (CacheReaderInterruptException& ex) {
        // pass upward
        throw ex;
    }
    catch (std::exception& ex) {
        DisplayError(kReadMeshErrorMsg, fFile.resolvedFullName(), geomPath, ex.what());
        return SubNode::Ptr();
    }
}

SubNode::Ptr AlembicCacheReader::readShape(
    const MString& geomPath, bool needUVs)
{
    using namespace CacheReaderAlembicPrivate;

    if (!valid()) return SubNode::Ptr();

    try {
        tbb::mutex::scoped_lock alembicLock(gsAlembicMutex);

        AlembicCacheObjectReader::Ptr reader;

        // Search saved readers
        ObjectReaderMap::iterator iter = fSavedReaders.find(geomPath.asChar());
        if (iter != fSavedReaders.end()) {
            reader = (*iter).second;
        }
        else {
            // path: |xform1|xform2|meshShape
            MStringArray pathArray;
            geomPath.split('|', pathArray);

            Alembic::Abc::IObject current = fAbcArchive.getTop();
            if (pathArray.length() > 0) {
                // Find the shape in the Alembic archive
                bool geometryFound = true;
                for (unsigned int i = 0; i < pathArray.length(); i++) {
                    MString step = pathArray[i];
                    current = current.getChild(step.asChar());
                    if (!current.valid()) {
                        geometryFound = false;
                        break;
                    }
                }

                if (geometryFound) {
                    reader = AlembicCacheObjectReader::create(current, needUVs);
                }
            }
        }

        if (!reader || !reader->valid()) return SubNode::Ptr();

        // Each time samplings only records the start time, i.e. there
        // is no way to ask for the end time of a TimeSampling!
        // Therefore, to determine the end of the animation, we simply
        // loop until time no longer advances...
        {
            TimeInterval interval = reader->sampleShape(
                -std::numeric_limits<double>::max());
            while (interval.endTime() != std::numeric_limits<double>::max()) {
                interval = reader->sampleShape(
                    interval.endTime());
            }
        }

        // The sub-node with mesh shape data.
        SubNode::Ptr top = reader->get();

        // Save the object readers for reuse.
        reader->saveAndReset(*this);

        return top;
    }
    catch (CacheReaderInterruptException& ex) {
        // pass upward
        throw ex;
    }
    catch (std::exception& ex) {
        DisplayError(kReadMeshErrorMsg, fFile.resolvedFullName(), geomPath, ex.what());
        return SubNode::Ptr();
    }
}

MaterialGraphMap::Ptr AlembicCacheReader::readMaterials()
{
    using namespace CacheReaderAlembicPrivate;

    if (!valid()) return MaterialGraphMap::Ptr();

    try {
        tbb::mutex::scoped_lock alembicLock(gsAlembicMutex);

        // Find "/materials"
        Alembic::Abc::IObject topObject = fAbcArchive.getTop();
        Alembic::Abc::IObject materialsObject = topObject.getChild(kMaterialsObject);

        // "/materials" doesn't exist!
        if (!materialsObject.valid()) {
            return MaterialGraphMap::Ptr();
        }

        MaterialGraphMap::MPtr materials = boost::make_shared<MaterialGraphMap>();

        // Read materials one by one. Hierarchical materials are not supported.
        for (size_t i = 0; i < materialsObject.getNumChildren(); i++) {
            Alembic::Abc::IObject object = materialsObject.getChild(i);
            if (Alembic::AbcMaterial::IMaterial::matches(object.getHeader())) {
                AlembicCacheMaterialReader reader(object);

                // Read the material
                TimeInterval interval = reader.sampleMaterial(
                    -std::numeric_limits<double>::max());
                while (interval.endTime() != std::numeric_limits<double>::max()) {
                    interval = reader.sampleMaterial(interval.endTime());
                }

                MaterialGraph::MPtr graph = reader.get();
                if (graph) {
                    materials->addMaterialGraph(graph);
                }
            }
        }

        // No materials..
        if (materials->getGraphs().empty()) {
            return MaterialGraphMap::Ptr();
        }

        return materials;
    }
    catch (CacheReaderInterruptException& ex) {
        // pass upward
        throw ex;
    }
    catch (std::exception& ex) {
        DisplayError(kReadFileErrorMsg, fFile.resolvedFullName(), ex.what());
        return MaterialGraphMap::Ptr();
    }
}

bool AlembicCacheReader::readAnimTimeRange(GPUCache::TimeInterval& range)
{
    if (!valid()) return false;

    try {
        tbb::mutex::scoped_lock alembicLock(gsAlembicMutex);

        // Try *.samples property.
        double samplesMin = std::numeric_limits<double>::infinity();
        double samplesMax = -std::numeric_limits<double>::infinity();

        unsigned int numTimeSamplings = fAbcArchive.getNumTimeSamplings();
        for (unsigned int i = 0; i < numTimeSamplings; i++) {
            // *.samples property
            std::stringstream propName;
            propName << i << ".samples";
            Alembic::Abc::IUInt32Property samplesProp(
                fAbcArchive.getTop().getProperties(), 
                propName.str(),
                Alembic::Abc::ErrorHandler::kQuietNoopPolicy
            );

            // The time sampling.
            Alembic::Abc::TimeSamplingPtr timeSampling = fAbcArchive.getTimeSampling(i);
            if (samplesProp && timeSampling) {
                unsigned int numSamples = 0;
                samplesProp.get(numSamples);
                if (numSamples > 0) {
                    samplesMin = std::min(samplesMin, timeSampling->getSampleTime(0));
                    samplesMax = std::max(samplesMax, timeSampling->getSampleTime(numSamples - 1));
                }
            }
        }

        // Successfully read *.samples property.
        if (samplesMin <= samplesMax) {
            range = TimeInterval(samplesMin, samplesMax);
            return true;
        }

        // Try archive bounds property.
        Alembic::Abc::IBox3dProperty boxProp = Alembic::AbcGeom::GetIArchiveBounds(
            fAbcArchive, Alembic::Abc::ErrorHandler::kQuietNoopPolicy);
        if (boxProp) {
            // The time range of the archive bounds property.
            size_t numSamples                          = boxProp.getNumSamples();
            Alembic::Abc::TimeSamplingPtr timeSampling = boxProp.getTimeSampling();
            if (numSamples > 0 && timeSampling) {
                range = TimeInterval(
                    timeSampling->getSampleTime(0),
                    timeSampling->getSampleTime(numSamples - 1)
                );
                return true;
            }
        }

        // No enough animation range info on the archive.
        return false;
    }
    catch (CacheReaderInterruptException& ex) {
        // pass upward
        throw ex;
    }
    catch (std::exception& ex) {
        DisplayError(kReadFileErrorMsg, fFile.resolvedFullName(), ex.what());
        return false;
    }
}

void AlembicCacheReader::saveReader(
    const std::string& fullName,
    CacheReaderAlembicPrivate::AlembicCacheObjectReader::Ptr& reader
)
{
    // We save the object reader in this AlembicCacheReader so that 
    // the object reader won't be destroyed after readHierarchy() or readShape().
    // The life time of the object reader would be the same as this AlembicCacheReader.
    // The object reader can be reused as long as the Alembic archive is not closed.
    // There are 2 situations that will cause an Alembic archive to be closed:
    //   1) There are no references to CacheReaderProxy. (Read complete)
    //   2) Maya is running out of file handles. (Temporarily close some inactive archives)
    if (reader && reader->valid()) {
        std::string geometryPath = fullName;
        std::replace(geometryPath.begin(), geometryPath.end(), '/', '|');
        fSavedReaders.insert(std::make_pair(geometryPath, reader));
    }
}


} // namespace GPUCache


