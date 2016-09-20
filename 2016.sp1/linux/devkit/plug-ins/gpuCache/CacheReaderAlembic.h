#ifndef _CacheReaderAlembic_h_
#define _CacheReaderAlembic_h_

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
#include <Alembic/AbcCoreAbstract/TimeSampling.h>
#include <Alembic/Abc/IArchive.h>
#include <Alembic/AbcGeom/INuPatch.h>
#include <Alembic/AbcGeom/IPolyMesh.h>
#include <Alembic/AbcGeom/ISubD.h>
#include <Alembic/AbcGeom/IXform.h>
#include <Alembic/AbcMaterial/IMaterial.h>
#include <Alembic/AbcMaterial/MaterialAssignment.h>
#include <Alembic/AbcCoreHDF5/ReadWrite.h>

#include "CacheReader.h"

#include <maya/MString.h>
#include <maya/MTime.h>
#include <maya/MFileObject.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MFnNurbsSurfaceData.h>

#include <boost/enable_shared_from_this.hpp>
#include <boost/unordered_map.hpp>
#include <boost/scoped_ptr.hpp>

#include <tbb/recursive_mutex.h>

#include <map>

namespace GPUCache {
// Forward Declarations
class AlembicCacheReader;

namespace CacheReaderAlembicPrivate {

// Forward Declarations
class AlembicCacheMeshReader;

typedef Alembic::Abc::index_t           index_t; 
typedef Alembic::Abc::chrono_t          chrono_t;
typedef Alembic::AbcGeom::IXformSchema  IXformSchema;
typedef Alembic::AbcGeom::XformSample   XformSample;
typedef Alembic::AbcGeom::XformOp       XformOp;
typedef Alembic::AbcCoreAbstract::TimeSamplingPtr TimeSamplingPtr;


//==============================================================================
// CLASS BaseTypeOfElem
//==============================================================================

template <class ElemType>
struct BaseTypeOfElem
{
    // When the element type is already a POD...
    typedef ElemType value_type;
    static const size_t kDimensions = 1;
};

// Alembic stores index buffer as signed integers, while this
// plug-in handles them as unsigned integers...
template <>
struct BaseTypeOfElem<int32_t>
{
    // When the element type is already a POD...
    typedef uint32_t value_type;
    static const size_t kDimensions = 1;
};
    
template <class T>
struct BaseTypeOfElem<Imath::Vec2<T> > 
{
    typedef T value_type;
    static const size_t kDimensions = 2;
};
    
template <class T>
struct BaseTypeOfElem<Imath::Vec3<T> > 
{
    typedef T value_type;
    static const size_t kDimensions = 3;
};
    

//==============================================================================
// CLASS AlembicArray
//==============================================================================

// A wrapper around alembic sample arrays
template <class ArrayProperty>
class AlembicArray :
    public ReadableArray<
    typename BaseTypeOfElem<
        typename ArrayProperty::traits_type::value_type
        >::value_type
    >
{
public:

    /*----- member functions -----*/
    typedef typename ArrayProperty::sample_ptr_type ArraySamplePtr;
    typedef typename ArrayProperty::traits_type traits_type;
    typedef typename BaseTypeOfElem<typename traits_type::value_type>::value_type T;
    typedef typename Array<T>::Digest Digest;
        
    static const size_t kDimensions =
        BaseTypeOfElem<typename traits_type::value_type>::kDimensions;
        
    // Returns a pointer to an Array that has the same content as the
    // buffer passed-in as determined by the computed digest hash-key.
    static boost::shared_ptr<ReadableArray<T> > create(
        const ArraySamplePtr& arraySamplePtr, const Digest& digest );

    virtual ~AlembicArray();
    virtual const T* get() const;

private:

    /*----- member functions -----*/

    // The constructor is declare private to force user to go through
    // the create() factory member function.
    GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_2;
 
    AlembicArray(
        const ArraySamplePtr& arraySamplePtr, const Digest& digest
    )
        : ReadableArray<T>(
            arraySamplePtr->size() * kDimensions, digest),
          fArraySamplePtr(arraySamplePtr)
    {
        assert(traits_type::dataType().getExtent() == kDimensions);
    }
 
    /*----- data members -----*/

    const ArraySamplePtr    fArraySamplePtr;
};


//==============================================================================
// CLASS PropertyCache
//==============================================================================

template <typename PROPERTY, typename KEY, typename VALUE, typename DERIVED>
class PropertyCache 
{
public:
    typedef PROPERTY    Property;
    typedef KEY         Key;
    typedef VALUE       Value;
    typedef DERIVED     Derived;

    PropertyCache()
        : fValidityInterval(TimeInterval::kInvalid)
    {}

    void reset()
    {
        fProperty = Property();
        fValidityInterval = TimeInterval(TimeInterval::kInvalid);
        fValue = Value();
    }
        
    bool valid() const
    {
        return fProperty.valid();
    }
        
    void init(const Property& property)
    {
        this->fProperty = property;
            
        const index_t numSamples       = this->fProperty.getNumSamples();
        const TimeSamplingPtr sampling = this->fProperty.getTimeSampling();

        if (this->fProperty.isConstant()) {
            // Delay the read of constant properties until the first call to setTime()
            fValidityInterval = TimeInterval(TimeInterval::kInvalid);
        }
        else {
            // We need to read-in all the sample keys in
            // sequential order to determine which keys are
            // truly unique. This has to be done at init time
            // because later on, it is possible that the samples
            // be asked in random order and it will be difficult
            // to determine the validity interval of the returned
            // sample.

            // There is always a sample at index 0!
            this->fUniqueSampleIndexes.push_back(0);
            this->fTimeBoundaries.push_back(-std::numeric_limits<chrono_t>::infinity());
            Key prevKey;
            static_cast<Derived*>(this)->getKey(prevKey, 0);
            for (index_t i=1; i<numSamples; ++i) {
                Key key;
                static_cast<Derived*>(this)->getKey(key, i);

                if (key != prevKey) {
                    this->fUniqueSampleIndexes.push_back(i);
                    // We store the time at which a sample stop being
                    // valid. This is reprented by the mid-way point
                    // between 2 samples.
                    this->fTimeBoundaries.push_back(
                        0.5 * (sampling->getSampleTime(i-1) + sampling->getSampleTime(i))
                    );
                    prevKey = key;
                }
            }
            this->fTimeBoundaries.push_back(+std::numeric_limits<chrono_t>::infinity());
        }
    }

    bool setTime(chrono_t time)
    {
        if (fProperty.isConstant())
        {
            // Delayed read of constant properties.
            if (!fValidityInterval.valid()) {
                static_cast<Derived*>(this)->readValue(0);
                // If an IXform node is constant identity, getNumSamples() returns 0
                fValidityInterval = TimeInterval(TimeInterval::kInfinite);
            }
            return false;
        }

        if (fValidityInterval.contains(time)) {
            return false;
        }

        std::vector<chrono_t>::const_iterator bgn = fTimeBoundaries.begin();
        std::vector<chrono_t>::const_iterator end = fTimeBoundaries.end();
        std::vector<chrono_t>::const_iterator it = std::upper_bound(
            bgn, end, time);
        assert(it != bgn);
        
        index_t idx = fUniqueSampleIndexes[std::distance(bgn, it) - 1];

        // Do this first for exception safety.
        static_cast<Derived*>(this)->readValue(idx);
            
        // Ok, we have successfully read the value. We can
        // now update the time information.
        fValidityInterval = TimeInterval(*(it-1), *it);
        return true;
    }

    const Value& getValue() const
    {
        return fValue;
    }
        
    TimeInterval getValidityInterval() const
    {
        return fValidityInterval;
    }
        
protected:
    Property              fProperty;
    std::vector<index_t>  fUniqueSampleIndexes;
    std::vector<chrono_t> fTimeBoundaries;
        
    TimeInterval          fValidityInterval;
    Value                 fValue;
};


//==============================================================================
// CLASS ScalarPropertyCache
//==============================================================================

template <typename PROPERTY>
class ScalarPropertyCache 
    : public PropertyCache<
        PROPERTY,
        typename PROPERTY::value_type,
        typename PROPERTY::value_type,
        ScalarPropertyCache<PROPERTY>
    >
{

public:
    typedef PropertyCache<
        PROPERTY,
        typename PROPERTY::value_type,
        typename PROPERTY::value_type,
        ScalarPropertyCache<PROPERTY>
    > BaseClass;
    typedef typename BaseClass::Key Key;
        
    void readValue(index_t idx)
    {
        // For scalar properties, the value is the key...
        getKey(this->fValue, idx);
    }
        
    void getKey(Key& key, index_t idx)
    {
        this->fProperty.get(key, idx);
    }
        
};


//==============================================================================
// CLASS XformPropertyCache
//==============================================================================

class XformPropertyCache 
    : public PropertyCache<
        IXformSchema,
        MMatrix,
        MMatrix,
        XformPropertyCache
    >
{
public:
    void readValue(index_t idx)
    {
        // For xform properties, the value is the key...
        getKey(fValue, idx);
    }
        
    void getKey(Key& key, index_t idx)
    {
        XformSample sample;
        fProperty.get(sample, idx);
        key = toMatrix(sample);
    }

private:
    // Helper function to extract the transformation matrix out of
    // an XformSample.
    static MMatrix toMatrix(const XformSample& sample)
    {
        Alembic::Abc::M44d matrix = sample.getMatrix();
        return MMatrix(matrix.x);
    }
        
};
    

//==============================================================================
// CLASS ArrayPropertyCache
//==============================================================================

template <typename PROPERTY>
class ArrayPropertyCache 
    : public PropertyCache<
        PROPERTY,
        Alembic::AbcCoreAbstract::ArraySampleKey,
        boost::shared_ptr<
            ReadableArray<
                typename BaseTypeOfElem<
                    typename PROPERTY::traits_type::value_type
                    >::value_type> >,
        ArrayPropertyCache<PROPERTY>
    >
{   
public:
    typedef PropertyCache<
        PROPERTY,
        Alembic::AbcCoreAbstract::ArraySampleKey,
        boost::shared_ptr<
            ReadableArray<
                typename BaseTypeOfElem<
                    typename PROPERTY::traits_type::value_type
                    >::value_type> >,
        ArrayPropertyCache<PROPERTY>
    > BaseClass;

    typedef typename BaseClass::Property Property;
    typedef typename BaseClass::Key Key;
    typedef typename BaseClass::Value Value;

    typedef typename Property::sample_ptr_type ArraySamplePtr;
    typedef typename Property::traits_type traits_type;
    typedef typename BaseTypeOfElem<typename traits_type::value_type>::value_type BaseType;
    static const size_t kDimensions =
        BaseTypeOfElem<typename traits_type::value_type>::kDimensions;

    void readValue(index_t idx)
    {
        Key key;
        getKey(key, idx);

        // Can't figure out how this can differs... It seems like a
        // provision for a future feature. Unfortunately, I can't
        // figure out if the key digest would be relative the read
        // or the orig POD!
        assert(key.origPOD == key.readPOD);
        assert(key.origPOD == traits_type::dataType().getPod());
        assert(sizeof(BaseType) == PODNumBytes(traits_type::dataType().getPod()));
        assert(kDimensions == traits_type::dataType().getExtent());

        const size_t size = size_t(key.numBytes / sizeof(BaseType));
            
        // First, let's try to an array out of the global
        // registry.
        //
        // Important, we first have to get rid of the previously
        // referenced value outside of the lock or else we are
        // risking a dead-lock on Linux and Mac (tbb::mutex is
        // non-recursive on these platforms).
        this->fValue = Value();
        {
            tbb::mutex::scoped_lock lock(ArrayRegistry<BaseType>::mutex());
            this->fValue = ArrayRegistry<BaseType>::lookupReadable(key.digest, size);
        
            if (this->fValue) return;
        }            

        // Sample not found. Read it.
        ArraySamplePtr sample;
        this->fProperty.get(sample, idx);

#ifndef NDEBUG
        Key key2 = sample->getKey();
        const size_t size2 = (sample->size() *
                             Property::traits_type::dataType().getExtent());
        assert(key == key2);
        assert(size == size2);
#endif
        
        // Insert the read sample into the cache.
        this->fValue = AlembicArray<Property>::create(sample, key.digest);
    }
        
    void getKey(Key& key, index_t idx)
    {
#ifndef NDEBUG
        bool result =
#endif                
            this->fProperty.getKey(key, idx);
        // There should always be a key...
        assert(result);
    }
};


template <typename PROPERTY>
class ArrayPropertyCacheWithConverter
    : public PropertyCache<
        PROPERTY,
        Alembic::AbcCoreAbstract::ArraySampleKey,
        boost::shared_ptr<
            ReadableArray<
                typename BaseTypeOfElem<
                    typename PROPERTY::traits_type::value_type
                    >::value_type> >,
        ArrayPropertyCacheWithConverter<PROPERTY>
    >
{
public:
    typedef PropertyCache<
        PROPERTY,
        Alembic::AbcCoreAbstract::ArraySampleKey,
        boost::shared_ptr<
            ReadableArray<
                typename BaseTypeOfElem<
                    typename PROPERTY::traits_type::value_type
                    >::value_type> >,
        ArrayPropertyCacheWithConverter<PROPERTY>
    > BaseClass;

    typedef typename BaseClass::Property Property;
    typedef typename BaseClass::Key Key;
    typedef typename BaseClass::Value Value;

    typedef typename Property::sample_ptr_type ArraySamplePtr;
    typedef typename Property::traits_type traits_type;
    typedef typename BaseTypeOfElem<typename traits_type::value_type>::value_type BaseType;
    typedef Alembic::Util::Digest Digest;

    typedef boost::shared_ptr<ReadableArray<BaseType> >
    (*Converter)(const typename PROPERTY::sample_ptr_type& sample);

    static const size_t kDimensions =
        BaseTypeOfElem<typename traits_type::value_type>::kDimensions;

    ArrayPropertyCacheWithConverter(Converter converter) 
        : fConverter(converter)
    {}
        
    void readValue(index_t idx)
    {
        Key key;
        getKey(key, idx);

        // Can't figure out how this can differs... It seems like a
        // provision for a future feature. Unfortunately, I can't
        // figure out if the key digest would be relative the read
        // or the orig POD!
        assert(key.origPOD == key.readPOD);
        assert(key.origPOD == traits_type::dataType().getPod());
        assert(sizeof(BaseType) == PODNumBytes(traits_type::dataType().getPod()));
        assert(kDimensions == traits_type::dataType().getExtent());

        const size_t size = size_t(key.numBytes / sizeof(BaseType));
            
        // First, let's try to an array out of the global
        // registry.
        //
        // Important, we first have to get rid of the previously
        // referenced value outside of the lock or else we are
        // risking a dead-lock on Linux and Mac (tbb::mutex is
        // non-recursive on these platforms).
        this->fValue = Value();
        typename ConvertionMap::const_iterator it = fsConvertionMap.find(key.digest);
        if (it != fsConvertionMap.end()) {
            tbb::mutex::scoped_lock lock(ArrayRegistry<BaseType>::mutex());
            this->fValue = ArrayRegistry<BaseType>::lookupReadable(it->second, size);
        
            if (this->fValue) return;
        }            

        // Sample not found. Read it.
        ArraySamplePtr sample;
        this->fProperty.get(sample, idx);

#ifndef NDEBUG        
        Key key2 = sample->getKey();
        const size_t size2 = (sample->size() *
                             Property::traits_type::dataType().getExtent());
        assert(key == key2);
        assert(size == size2);
#endif
        
        // Insert the read sample into the cache.
        this->fValue = fConverter(sample);

        fsConvertionMap[key.digest] = this->fValue->digest();
    }
        
    void getKey(Key& key, index_t idx)
    {
#ifndef NDEBUG
        bool result =
#endif                
            this->fProperty.getKey(key, idx);
        // There should always be a key...
        assert(result);
    }

private:

    struct DigestHash : std::unary_function<Digest, std::size_t>
    {
        std::size_t operator()(Digest const& v) const
        {
            std::size_t seed = 0;
            boost::hash_combine(seed, v.words[0]);
            boost::hash_combine(seed, v.words[1]);
            return seed;
        }
    };

    typedef boost::unordered_map<Digest, Digest, DigestHash> ConvertionMap;
    static ConvertionMap fsConvertionMap;

    const Converter fConverter;
};


//==============================================================================
// CLASS DataProvider
//==============================================================================

// An abstract class to wrap the details of different data sources.
// Currently, we have four kinds of Alembic geometries:
//     1) IPolyMesh from gpuCache command
//     2) IPolyMesh from arbitrary Alembic exporter such as AbcExport
//     3) INuPatch  from arbitrary Alembic exporter such as AbcExport
//     4) ISubD     from arbitrary Alembic exporter such as AbcExport
// Of course, 1) is much faster than 2).
// Caller is responsible for locking.
//
class DataProvider
{
public:
    virtual ~DataProvider();

    // Check if all the properties are valid
    virtual bool valid() const;

    // The following two methods are used when reading hierarchy.
    //
    // Fill minimal property caches that will display a bounding box place holder
    void fillBBoxAndVisSample(chrono_t time);

    // Return the validity interval of a bounding box place holder sample
    TimeInterval getBBoxAndVisValidityInterval() const { return fBBoxAndVisValidityInterval; }
    
    // The following two methods are used when reading shapes.
    //
    // Fill property caches with the data at the specified index
    void fillTopoAndAttrSample(chrono_t time);

    // Return the combined validity interval of the the property
    // caches for the last updated index.
    TimeInterval getValidityInterval() const { return fValidityInterval; }


    // Check for the visibility
    bool isVisible() const;

    // Retrieve the current bounding box proxy sample from property cache
    boost::shared_ptr<const ShapeSample>
    getBBoxPlaceHolderSample(double seconds);

    // Retrieve the current sample from property cache
    virtual boost::shared_ptr<const ShapeSample>
    getSample(double seconds) = 0;

    // Return the bounding box and validity interval for the current
    // sample, i.e. the time of the last call to sample.
    MBoundingBox getBoundingBox() const
    {
        const Imath::Box<Imath::V3d> boundingBox =
            fBoundingBoxCache.getValue();
        if (boundingBox.isEmpty()) {
            return MBoundingBox();
        }
        return MBoundingBox(         
            MPoint(boundingBox.min.x, boundingBox.min.y, boundingBox.min.z),
            MPoint(boundingBox.max.x, boundingBox.max.y, boundingBox.max.z));
    }

    TimeInterval getBoundingBoxValidityInterval() const
    { return fBoundingBoxCache.getValidityInterval(); }

    TimeInterval getAnimTimeRange() const
    { return fAnimTimeRange; }
    
protected:
    // Prohibited and not implemented.
    DataProvider(const DataProvider&);
    const DataProvider& operator= (const DataProvider&);

    template<class INFO>
    DataProvider(Alembic::AbcGeom::IGeomBaseSchema<INFO>& abcGeom,
                 Alembic::Abc::TimeSamplingPtr            timeSampling,
                 size_t numSamples,
                 bool   needUVs);

    // Update Bounding Box and Visibility property caches
    TimeInterval updateBBoxAndVisCache(chrono_t time);

    // Update the property caches
    virtual TimeInterval updateCache(chrono_t time);

    // Whether UV coordinates should be read or generated.
    const bool fNeedUVs;

    // Exact animation time range
    TimeInterval fAnimTimeRange;

    // The valid range of bbox and visibility in property caches
    TimeInterval fBBoxAndVisValidityInterval;

    // The valid range of the current data in property caches
    TimeInterval fValidityInterval;

    // Shape Visibility
    ScalarPropertyCache<Alembic::Abc::ICharProperty>  fVisibilityCache;

    // Bounding Box
    ScalarPropertyCache<Alembic::Abc::IBox3dProperty> fBoundingBoxCache;

    // Parent Visibility
    std::vector<ScalarPropertyCache<Alembic::Abc::ICharProperty> > fParentVisibilityCache;
};


//==============================================================================
// CLASS PolyDataProvider
//==============================================================================

// Base class for all polygon data sources.
//
class PolyDataProvider : public DataProvider
{
public:
    virtual ~PolyDataProvider();

    // Check if all the properties are valid
    virtual bool valid() const;

protected:
    // Prohibited and not implemented.
    PolyDataProvider(const PolyDataProvider&);
    const PolyDataProvider& operator= (const PolyDataProvider&);

    template<class SCHEMA>
    PolyDataProvider(SCHEMA&                         abcMesh,
                     bool                            needUVs);

    // Update the property caches
    virtual TimeInterval updateCache(chrono_t time);

    // Polygons
    ArrayPropertyCache<Alembic::Abc::IInt32ArrayProperty> fFaceCountsCache;
    ArrayPropertyCache<Alembic::Abc::IP3fArrayProperty>   fPositionsCache;
};


//==============================================================================
// CLASS RawDataProvider
//==============================================================================

// This class reads mesh data that is written by gpuCache command.
//
class RawDataProvider : public PolyDataProvider
{
public:
    RawDataProvider(Alembic::AbcGeom::IPolyMeshSchema& abcMesh,
                    bool needUVs);
    virtual ~RawDataProvider();

    // Check if all the properties are valid
    virtual bool valid() const;

    // Retrieve the current sample from property cache
    virtual boost::shared_ptr<const ShapeSample>
    getSample(double seconds);

protected:
    // Prohibited and not implemented.
    RawDataProvider(const RawDataProvider&);
    const RawDataProvider& operator= (const RawDataProvider&);

    // Convert triangles winding from CW to CCW
    static boost::shared_ptr<ReadableArray<IndexBuffer::index_t> >
    correctPolygonWinding(const Alembic::Abc::Int32ArraySamplePtr& indices);

    // Update the property caches
    virtual TimeInterval updateCache(chrono_t time);

    ArrayPropertyCacheWithConverter<Alembic::Abc::IInt32ArrayProperty> fFaceIndicesCache;
    ArrayPropertyCache<Alembic::Abc::IInt32ArrayProperty> fWireIndicesCache;  // required
    ArrayPropertyCache<Alembic::Abc::IInt32ArrayProperty> fGroupSizesCache;   // optional
    ScalarPropertyCache<Alembic::Abc::IC4fProperty>       fDiffuseColorCache; // required
    ArrayPropertyCache<Alembic::Abc::IN3fArrayProperty>   fNormalsCache;      // currently, required
    ArrayPropertyCache<Alembic::Abc::IV2fArrayProperty>   fUVsCache;          
};


//==============================================================================
// CLASS Triangulator
//==============================================================================

// This class reads mesh data that is written by an arbitrary Alembic exporter.
// Triangulate a polygon mesh and convert multi-indexed streams to single-indexed streams.
//
class Triangulator : public PolyDataProvider
{
public:
    Triangulator(Alembic::AbcGeom::IPolyMeshSchema& abcMesh,
                 bool needUVs);
    virtual ~Triangulator();

    // Check if all the properties are valid
    virtual bool valid() const;

    // Retrieve the current sample from property cache
    virtual boost::shared_ptr<const ShapeSample>
    getSample(double seconds);

protected:
    // Prohibited and not implemented.
    Triangulator(const Triangulator&);
    const Triangulator& operator= (const Triangulator&);

    // Update the property caches
    virtual TimeInterval updateCache(chrono_t time);

    // Polygon Indices
    ArrayPropertyCache<Alembic::Abc::IInt32ArrayProperty> fFaceIndicesCache;

    // Normals
    Alembic::AbcGeom::GeometryScope                        fNormalsScope;
    ArrayPropertyCache<Alembic::Abc::IN3fArrayProperty>    fNormalsCache;
    ArrayPropertyCache<Alembic::Abc::IUInt32ArrayProperty> fNormalIndicesCache;

    // UVs
    Alembic::AbcGeom::GeometryScope                        fUVsScope;
    ArrayPropertyCache<Alembic::Abc::IV2fArrayProperty>    fUVsCache;
    ArrayPropertyCache<Alembic::Abc::IUInt32ArrayProperty> fUVIndicesCache;
        
private:
    template<size_t SIZE>
    boost::shared_ptr<ReadableArray<float> > convertMultiIndexedStream(
        boost::shared_ptr<ReadableArray<float> > attribArray,
        boost::shared_ptr<ReadableArray<IndexBuffer::index_t> > indexArray);

    void check();
    void computeNormals();
    void convertMultiIndexedStreams();
    void remapVertAttribs();
    void computeWireIndices();
    void triangulate();

    // compute in check();
    Alembic::AbcGeom::GeometryScope                 fCheckedNormalsScope;
    boost::shared_ptr<ReadableArray<float> >                fCheckedNormals;
    boost::shared_ptr<ReadableArray<IndexBuffer::index_t> > fCheckedNormalIndices;
    Alembic::AbcGeom::GeometryScope                 fCheckedUVsScope;
    boost::shared_ptr<ReadableArray<float> >                fCheckedUVs;
    boost::shared_ptr<ReadableArray<IndexBuffer::index_t> > fCheckedUVIndices;

    // compute in computeNormals()
    Alembic::AbcGeom::GeometryScope                 fComputedNormalsScope;
    boost::shared_ptr<ReadableArray<float> >                fComputedNormals;
    boost::shared_ptr<ReadableArray<IndexBuffer::index_t> > fComputedNormalIndices;

    // compute in convertMultiIndexedStreams()
    size_t                                          fNumVertices;
    boost::shared_array<unsigned int>               fVertAttribsIndices;
    boost::shared_ptr<ReadableArray<IndexBuffer::index_t> > fMappedFaceIndices;

    // compute in remapVertAttribs()
    boost::shared_ptr<ReadableArray<float> >                fMappedPositions;
    boost::shared_ptr<ReadableArray<float> >                fMappedNormals;
    boost::shared_ptr<ReadableArray<float> >                fMappedUVs;
        
    // compute in computeWireIndices()
    boost::shared_ptr<ReadableArray<IndexBuffer::index_t> > fWireIndices;

    // compute in triangulate()
    boost::shared_ptr<ReadableArray<IndexBuffer::index_t> > fTriangleIndices;
};


//==============================================================================
// CLASS NurbsTessellator
//==============================================================================

// This class reads NURBS data that is written by an arbitrary Alembic exporter.
// NURBS with trimmed curves are tessellated by MFnNurbsSurface
//
class NurbsTessellator : public DataProvider
{
public:
    NurbsTessellator(Alembic::AbcGeom::INuPatchSchema& abcNurbs,
                     bool needUVs);
        
    virtual ~NurbsTessellator();

    // Check if all the properties are valid
    virtual bool valid() const;

    // Retrieve the current sample from property cache
    virtual boost::shared_ptr<const ShapeSample>
    getSample(double seconds);

protected:
    // Prohibited and not implemented.
    NurbsTessellator(const NurbsTessellator&);
    const NurbsTessellator& operator= (const NurbsTessellator&);

    // Update the property caches
    virtual TimeInterval updateCache(chrono_t time);

    // NURBS required properties
    ArrayPropertyCache<Alembic::Abc::IP3fArrayProperty>       fPositionsCache;
    ScalarPropertyCache<Alembic::Abc::IInt32Property>         fNumUCache;
    ScalarPropertyCache<Alembic::Abc::IInt32Property>         fNumVCache;
    ScalarPropertyCache<Alembic::Abc::IInt32Property>         fUOrderCache;
    ScalarPropertyCache<Alembic::Abc::IInt32Property>         fVOrderCache;
    ArrayPropertyCache<Alembic::AbcGeom::IFloatArrayProperty> fUKnotCache;
    ArrayPropertyCache<Alembic::AbcGeom::IFloatArrayProperty> fVKnotCache;

    // NURBS optional properties
    //   Currently, normals and UVs are ignored..
    ArrayPropertyCache<Alembic::AbcGeom::IFloatArrayProperty> fPositionWeightsCache;

    // Optional trim curves
    ScalarPropertyCache<Alembic::Abc::IInt32Property>         fTrimNumLoopsCache;
    ArrayPropertyCache<Alembic::Abc::IInt32ArrayProperty>     fTrimNumCurvesCache;
    ArrayPropertyCache<Alembic::Abc::IInt32ArrayProperty>     fTrimNumVerticesCache;
    ArrayPropertyCache<Alembic::Abc::IInt32ArrayProperty>     fTrimOrderCache;
    ArrayPropertyCache<Alembic::AbcGeom::IFloatArrayProperty> fTrimKnotCache;
    ArrayPropertyCache<Alembic::AbcGeom::IFloatArrayProperty> fTrimUCache;
    ArrayPropertyCache<Alembic::AbcGeom::IFloatArrayProperty> fTrimVCache;
    ArrayPropertyCache<Alembic::AbcGeom::IFloatArrayProperty> fTrimWCache;

private:
    void check();
    void setNurbs(bool rebuild, bool positionsChanged);
    void tessellate();
    void convertToPoly();

    // compute in check()
    bool fSurfaceValid;

    // compute in setNubs()
    MFnNurbsSurfaceData fNurbsData;
    MFnNurbsSurface     fNurbs;

    // compute in tessellate()
    MFnMeshData fPolyMeshData;
    MFnMesh     fPolyMesh;

    // compute in convertToPoly()
    boost::shared_ptr<ReadableArray<IndexBuffer::index_t> > fTriangleIndices;
    boost::shared_ptr<ReadableArray<IndexBuffer::index_t> > fWireIndices;
    boost::shared_ptr<ReadableArray<float> >                fPositions;
    boost::shared_ptr<ReadableArray<float> >                fNormals;
    boost::shared_ptr<ReadableArray<float> >                fUVs;
};


//==============================================================================
// CLASS SubDSmoother
//==============================================================================

// This class reads SubD data that is written by an arbitrary Alembic exporter.
//
class SubDSmoother : public PolyDataProvider
{
public:
    SubDSmoother(Alembic::AbcGeom::ISubDSchema&     abcSubd,
                 const bool                         needUVs);
    virtual ~SubDSmoother();

    // Check if all the properties are valid
    virtual bool valid() const;

    // Retrieve the current sample from property cache
    virtual boost::shared_ptr<const ShapeSample>
    getSample(double seconds);

protected:
    // Prohibited and not implemented.
    SubDSmoother(const SubDSmoother&);
    const SubDSmoother& operator= (const SubDSmoother&);

    // Update the property caches
    virtual TimeInterval updateCache(chrono_t time);

    // Polygon Indices
    ArrayPropertyCache<Alembic::Abc::IInt32ArrayProperty> fFaceIndicesCache;

    // Crease Edges
    ArrayPropertyCache<Alembic::Abc::IInt32ArrayProperty> fCreaseIndicesCache;
    ArrayPropertyCache<Alembic::Abc::IInt32ArrayProperty> fCreaseLengthsCache;
    ArrayPropertyCache<Alembic::Abc::IFloatArrayProperty> fCreaseSharpnessesCache;

    // Crease Vertices
    ArrayPropertyCache<Alembic::Abc::IInt32ArrayProperty> fCornerIndicesCache;
    ArrayPropertyCache<Alembic::Abc::IFloatArrayProperty> fCornerSharpnessesCache;

    // Invisible Faces
    ArrayPropertyCache<Alembic::Abc::IInt32ArrayProperty> fHolesCache;

    // UVs
    Alembic::AbcGeom::GeometryScope                        fUVsScope;
    ArrayPropertyCache<Alembic::Abc::IV2fArrayProperty>    fUVsCache;
    ArrayPropertyCache<Alembic::Abc::IUInt32ArrayProperty> fUVIndicesCache;

private:
    void check();
    void rebuildSubD();
    void setPositions();
    void setCreaseEdges();
    void setCreaseVertices();
    void setInvisibleFaces();
    void setUVs();
    void convertToPoly();

    // compute in check()
    Alembic::AbcGeom::GeometryScope                 fCheckedUVsScope;
    boost::shared_ptr<ReadableArray<float> >                fCheckedUVs;
    boost::shared_ptr<ReadableArray<IndexBuffer::index_t> > fCheckedUVIndices;

    // compute in rebuildSubD()
    MFnMeshData fSubDData;
    MFnMesh     fSubD;

    // compute in convertToPoly()
    boost::shared_ptr<ReadableArray<IndexBuffer::index_t> > fTriangleIndices;
    boost::shared_ptr<ReadableArray<IndexBuffer::index_t> > fWireIndices;
    boost::shared_ptr<ReadableArray<float> >                fPositions;
    boost::shared_ptr<ReadableArray<float> >                fNormals;
    boost::shared_ptr<ReadableArray<float> >                fUVs;
};

    
//==============================================================================
// CLASS AlembicCacheObjectReader
//==============================================================================

// Abstract reader interface for reading an Alembic object along with
// all of its children.
class AlembicCacheObjectReader
    : boost::noncopyable, 
      public boost::enable_shared_from_this<AlembicCacheObjectReader>
{
public:

    typedef  boost::shared_ptr<AlembicCacheObjectReader> Ptr;
    
    /*----- static member functions -----*/

    static Ptr create(Alembic::Abc::IObject& abcObj, bool needUVs);


    /*----- member functions -----*/

    virtual ~AlembicCacheObjectReader() = 0;

    virtual bool valid() const = 0;

    // Read and append a sample of the given mesh at the given time
    // and sample index value.
    // This method only reads hierarchy information:
    // xform, bounding box, visibility, ...
    virtual TimeInterval sampleHierarchy(double seconds,
        const MMatrix& rootMatrix, TimeInterval rootMatrixInterval) = 0;

    // Read and append a sample of the given mesh at the given time
    // and sample index value.
    // This method reads the mesh buffers.
    virtual TimeInterval sampleShape(double seconds) = 0;

    // Return the read hierarchy.
    virtual SubNode::MPtr get() const = 0;

    // Return the bounding box and validity interval for the current
    // sample, i.e. the time of the last call to sample.
    // The bounding box is in the axis of root sub-node.
    virtual MBoundingBox getBoundingBox() const = 0;
    virtual TimeInterval getBoundingBoxValidityInterval() const = 0;  

    // Return the exact animation time range.
    virtual TimeInterval getAnimTimeRange() const = 0;

    // Save this object reader and reset its content for reuse.
    virtual void saveAndReset(AlembicCacheReader& cacheReader) = 0;
};


//==============================================================================
// CLASS AlembicCacheTopReader
//==============================================================================

class AlembicCacheTopReader : public AlembicCacheObjectReader
{
public:
    AlembicCacheTopReader(Alembic::Abc::IObject object, bool needUVs);
    ~AlembicCacheTopReader();

    virtual bool valid() const;
    virtual TimeInterval sampleHierarchy(double seconds, 
        const MMatrix& rootMatrix, TimeInterval rootMatrixInterval);
    virtual TimeInterval sampleShape(double seconds);
    virtual SubNode::MPtr get() const;

    virtual MBoundingBox getBoundingBox() const;
    virtual TimeInterval getBoundingBoxValidityInterval() const;

    virtual TimeInterval getAnimTimeRange() const;

    virtual void saveAndReset(AlembicCacheReader& cacheReader);

private:

    // Bounding box and its interval
    MBoundingBox fBoundingBox;
    TimeInterval fBoundingBoxValidityInterval;

    // The sub node data currently being filled-in.
    XformData::MPtr fXformData;

    // Readers of children nodes.
    std::vector<AlembicCacheObjectReader::Ptr> fChildren;

};


//==============================================================================
// CLASS AlembicCacheXformReader
//==============================================================================

class AlembicCacheXformReader : public AlembicCacheObjectReader
{
public:
    AlembicCacheXformReader(Alembic::Abc::IObject object, bool needUVs);
    ~AlembicCacheXformReader();

    virtual bool valid() const;
    virtual TimeInterval sampleHierarchy(double seconds, 
        const MMatrix& rootMatrix, TimeInterval rootMatrixInterval);
    virtual TimeInterval sampleShape(double seconds);
    virtual SubNode::MPtr get() const;

    virtual MBoundingBox getBoundingBox() const;
    virtual TimeInterval getBoundingBoxValidityInterval() const;

    virtual TimeInterval getAnimTimeRange() const;

    virtual void saveAndReset(AlembicCacheReader& cacheReader);

private:

    void fillTopoAndAttrSample(chrono_t time);
    bool isVisible() const;
    

    // Alembic readers
    const std::string                 fName;

    // The valid range of the current data in property caches
    TimeInterval fValidityInterval;

    // Transform
    XformPropertyCache fXformCache;

    // Transform Visibility
    ScalarPropertyCache<Alembic::Abc::ICharProperty>  fVisibilityCache;

    // Bounding box and its interval
    MBoundingBox fBoundingBox;
    TimeInterval fBoundingBoxValidityInterval;

    // The sub node data currently being filled-in.
    XformData::MPtr fXformData;
    
    // Readers of children nodes.
    std::vector<AlembicCacheObjectReader::Ptr> fChildren;
};


//==============================================================================
// CLASS AlembicCacheMeshReader
//==============================================================================

class AlembicCacheMeshReader : public AlembicCacheObjectReader
{
public:
    AlembicCacheMeshReader(Alembic::Abc::IObject object, bool needUVs);
    ~AlembicCacheMeshReader();

    virtual bool valid() const;
    virtual TimeInterval sampleHierarchy(double seconds, 
        const MMatrix& rootMatrix, TimeInterval rootMatrixInterval);
    virtual TimeInterval sampleShape(double seconds);
    virtual SubNode::MPtr get() const;

protected:
    
    virtual MBoundingBox getBoundingBox() const;
    virtual TimeInterval getBoundingBoxValidityInterval() const;

    virtual TimeInterval getAnimTimeRange() const;

    virtual void saveAndReset(AlembicCacheReader& cacheReader);

private:

    // Alembic readers
    const std::string                 fName;
    const std::string                 fFullName;
    boost::scoped_ptr<DataProvider>   fDataProvider;

    // Bounding box and its interval
    MBoundingBox fBoundingBox;
    TimeInterval fBoundingBoxValidityInterval;

    // The sub node data currently being filled-in.
    ShapeData::MPtr fShapeData;
    size_t          fNumTransparentSample;
};

//==============================================================================
// CLASS AlembicCacheMaterialReader
//==============================================================================

class AlembicCacheMaterialReader : boost::noncopyable
{
public:
    AlembicCacheMaterialReader(Alembic::Abc::IObject abcObj);
    ~AlembicCacheMaterialReader();

    TimeInterval sampleMaterial(double seconds);
    MaterialGraph::MPtr get() const;

private:
    // Alembic readers
    const std::string fName;

    // Templated classes to translate Alembic properties.
    template<typename ABC_PROP> 
    class ScalarMaterialProp
    {
    public:
        ScalarMaterialProp(Alembic::Abc::ICompoundProperty& parent,
                           const std::string& name,
                           MaterialNode::MPtr& node)
            : fName(name)
        {
            // Create Alembic input property
            ABC_PROP abcProp(parent, name);
            assert(abcProp.valid());

            // Create reader cache
            fCache.reset(new ScalarPropertyCache<ABC_PROP>());
            fCache->init(abcProp);

            // Find existing property.
            fProp = node->findProperty(name.c_str());
            assert(!fProp || fProp->type() == propertyType<ABC_PROP>());

            if (fProp && fProp->type() != propertyType<ABC_PROP>()) {
                // Something goes wrong..
                fCache.reset();
                fProp.reset();
                return;
            }

            // This is not a known property.
            if (!fProp) {
                fProp = node->createProperty(name.c_str(), propertyType<ABC_PROP>());
                assert(fProp->type() == propertyType<ABC_PROP>());
            }
        }

        TimeInterval sample(double seconds)
        {
            TimeInterval validityInterval(TimeInterval::kInfinite);

            if (fCache && fCache->valid()) {
                fCache->setTime(seconds);
                validityInterval &= fCache->getValidityInterval();

                if (seconds == validityInterval.startTime()) {
                    setMaterialProperty<ABC_PROP>(fCache, fProp, seconds);
                }
            }

            return validityInterval;
        }


    private:
        std::string                                       fName;
        boost::shared_ptr<ScalarPropertyCache<ABC_PROP> > fCache;
        MaterialProperty::MPtr                            fProp;
    };

    template<typename T> static MaterialProperty::Type propertyType()
    { assert(0); return MaterialProperty::kFloat; }

    template<typename T> static void setMaterialProperty(
        boost::shared_ptr<ScalarPropertyCache<T> >& cache,
        MaterialProperty::MPtr prop,
        double seconds)
    {}

    // The list of animated property caches
    std::vector<ScalarMaterialProp<Alembic::Abc::IBoolProperty> >    fBoolCaches;
    std::vector<ScalarMaterialProp<Alembic::Abc::IInt32Property> >   fInt32Caches;
    std::vector<ScalarMaterialProp<Alembic::Abc::IFloatProperty> >   fFloatCaches;
    std::vector<ScalarMaterialProp<Alembic::Abc::IV2fProperty> >     fFloat2Caches;
    std::vector<ScalarMaterialProp<Alembic::Abc::IV3fProperty> >     fFloat3Caches;
    std::vector<ScalarMaterialProp<Alembic::Abc::IC3fProperty> >     fRGBCaches;
    std::vector<ScalarMaterialProp<Alembic::Abc::IWstringProperty> > fStringCaches;

    // The valid range of the current data in property caches
    TimeInterval fValidityInterval;

    // The material graph currently being filled-in.
    MaterialGraph::MPtr fMaterialGraph;
};

// Template explicit specialization must be in namespace scope.
template<>
inline MaterialProperty::Type AlembicCacheMaterialReader::propertyType<Alembic::Abc::IBoolProperty>()
{ return MaterialProperty::kBool; }

template<>
inline MaterialProperty::Type AlembicCacheMaterialReader::propertyType<Alembic::Abc::IInt32Property>()
{ return MaterialProperty::kInt32; }

template<>
inline MaterialProperty::Type AlembicCacheMaterialReader::propertyType<Alembic::Abc::IFloatProperty>()
{ return MaterialProperty::kFloat; }

template<>
inline MaterialProperty::Type AlembicCacheMaterialReader::propertyType<Alembic::Abc::IV2fProperty>()
{ return MaterialProperty::kFloat2; }

template<>
inline MaterialProperty::Type AlembicCacheMaterialReader::propertyType<Alembic::Abc::IV3fProperty>()
{ return MaterialProperty::kFloat3; }

template<>
inline MaterialProperty::Type AlembicCacheMaterialReader::propertyType<Alembic::Abc::IC3fProperty>()
{ return MaterialProperty::kRGB; }

template<>
inline MaterialProperty::Type AlembicCacheMaterialReader::propertyType<Alembic::Abc::IWstringProperty>()
{ return MaterialProperty::kString; }

template<>
inline void AlembicCacheMaterialReader::setMaterialProperty<Alembic::Abc::IBoolProperty>(
    boost::shared_ptr<ScalarPropertyCache<Alembic::Abc::IBoolProperty> >& cache,
    MaterialProperty::MPtr prop,
    double seconds)
{
    prop->setBool(seconds, cache->getValue());
}

template<>
inline void AlembicCacheMaterialReader::setMaterialProperty<Alembic::Abc::IInt32Property>(
    boost::shared_ptr<ScalarPropertyCache<Alembic::Abc::IInt32Property> >& cache,
    MaterialProperty::MPtr prop,
    double seconds)
{
    prop->setInt32(seconds, cache->getValue());
}

template<>
inline void AlembicCacheMaterialReader::setMaterialProperty<Alembic::Abc::IFloatProperty>(
    boost::shared_ptr<ScalarPropertyCache<Alembic::Abc::IFloatProperty> >& cache,
    MaterialProperty::MPtr prop,
    double seconds)
{
    prop->setFloat(seconds, cache->getValue());
}

template<>
inline void AlembicCacheMaterialReader::setMaterialProperty<Alembic::Abc::IV2fProperty>(
    boost::shared_ptr<ScalarPropertyCache<Alembic::Abc::IV2fProperty> >& cache,
    MaterialProperty::MPtr prop,
    double seconds)
{
    Alembic::Abc::V2f value = cache->getValue();
    prop->setFloat2(seconds, value.x, value.y);
}

template<>
inline void AlembicCacheMaterialReader::setMaterialProperty<Alembic::Abc::IV3fProperty>(
    boost::shared_ptr<ScalarPropertyCache<Alembic::Abc::IV3fProperty> >& cache,
    MaterialProperty::MPtr prop,
    double seconds)
{
    Alembic::Abc::V3f value = cache->getValue();
    prop->setFloat3(seconds, value.x, value.y, value.z);
}

template<>
inline void AlembicCacheMaterialReader::setMaterialProperty<Alembic::Abc::IC3fProperty>(
    boost::shared_ptr<ScalarPropertyCache<Alembic::Abc::IC3fProperty> >& cache,
    MaterialProperty::MPtr prop,
    double seconds)
{
    Alembic::Abc::C3f value = cache->getValue();
    prop->setColor(seconds, MColor(value.x, value.y, value.z));
}

template<>
inline void AlembicCacheMaterialReader::setMaterialProperty<Alembic::Abc::IWstringProperty>(
    boost::shared_ptr<ScalarPropertyCache<Alembic::Abc::IWstringProperty> >& cache,
    MaterialProperty::MPtr prop,
    double seconds)
{
    std::wstring value = cache->getValue();
    prop->setString(seconds, value.c_str());
}

} // namespace CacheReaderAlembicPrivate


//==============================================================================
// CLASS AlembicCacheReader
//==============================================================================

class AlembicCacheReader : public CacheReader
{
public:
    static boost::shared_ptr<CacheReader> create(const MFileObject& file);

    virtual ~AlembicCacheReader();
    virtual bool valid() const;

    virtual bool validateGeomPath(
        const MString& geomPath, MString& validatedGeomPath) const;

    virtual SubNode::Ptr readScene(
        const MString& geomPath, bool needUVs);

    virtual SubNode::Ptr readHierarchy(
        const MString& geomPath, bool needUVs);

    virtual SubNode::Ptr readShape(
        const MString& geomPath, bool needUVs);

    virtual MaterialGraphMap::Ptr readMaterials();

    virtual bool readAnimTimeRange(TimeInterval& range);

    void saveReader(const std::string& fullName, 
                    CacheReaderAlembicPrivate::AlembicCacheObjectReader::Ptr& reader);

private:
    friend class AlembicCacheMeshReader;
    GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_1;

    AlembicCacheReader(const MFileObject& file);

    const MFileObject fFile;
    mutable Alembic::Abc::IArchive fAbcArchive;

    typedef boost::unordered_map<std::string,CacheReaderAlembicPrivate::AlembicCacheObjectReader::Ptr> ObjectReaderMap;
    ObjectReaderMap fSavedReaders;
};


} // namespace GPUCache

#endif

