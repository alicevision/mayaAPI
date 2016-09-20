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

#include "gpuCacheSubSceneOverride.h"
#include "gpuCacheShapeNode.h"
#include "gpuCacheUnitBoundingBox.h"
#include "gpuCacheFrustum.h"
#include "gpuCacheUtil.h"
#include "CacheReader.h"

#include <boost/foreach.hpp>
#include <boost/unordered_set.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/hashed_index.hpp>

#include <tbb/tbb_thread.h>
#include <tbb/mutex.h>

#include <maya/MDagMessage.h>
#include <maya/MDGMessage.h>
#include <maya/MModelMessage.h>
#include <maya/MNodeMessage.h>
#include <maya/MSceneMessage.h>
#include <maya/MEventMessage.h>

#include <maya/MHWGeometryUtilities.h>
#include <maya/MAnimControl.h>
#include <maya/MDrawContext.h>
#include <maya/MFnAttribute.h>
#include <maya/MFnDagNode.h>
#include <maya/MGlobal.h>
#include <maya/MItDag.h>
#include <maya/MSelectionList.h>
#include <maya/MShaderManager.h>
#include <maya/MUserData.h>


namespace {

using namespace GPUCache;
using namespace MHWRender;

//==============================================================================
// LOCAL FUNCTIONS and CLASSES
//==============================================================================

// Guard pattern.
template<typename T>
class ScopedGuard : boost::noncopyable
{
public:
    ScopedGuard(T& value)
        : fValueRef(value), fValueBackup(value)
    {}

    ~ScopedGuard()
    {
        fValueRef = fValueBackup;
    }

private:
    T& fValueRef;
    T  fValueBackup;
};


// The thread id of the main thread.
tbb::tbb_thread::id gsMainThreadId = tbb::this_tbb_thread::get_id();


// Helper functions for MayaBufferArray to fetch the buffer sizes.  Results are in numbers 
// of 4-byte words.
size_t MayaBufferSizeHelper(MHWRender::MIndexBuffer* mayaBuffer)
{
    return mayaBuffer->size();
}

size_t MayaBufferSizeHelper(MHWRender::MVertexBuffer* mayaBuffer)
{
    return mayaBuffer->descriptor().dimension() * mayaBuffer->vertexCount();
}


// An implementation of the Array interface which wraps a Maya-owned data buffer.  This
// buffer may reside on the GPU, so we do not provide direct read access.  Read access
// can be granted, but this is only safe to do from the main thread.  Readback won't be
// as fast as from a raw memory buffer, but it will typically be fast enough to be useful.
// With huge scenes we can't afford to store two entire copies of the scene geometry.  So
// we can convert our arrays to this type and depend solely on the Maya copy.  We leave
// memory management of the buffers to Maya, so they may be paged out to system memory or
// to disk as needed.
// T - the raw datatype of the array, float or unsigned int.
// C - the maya buffer class containing the data, MVertexBuffer or MIndexBuffer.
template < typename T, typename C >
class MayaBufferArray : public Array<T>
{
    // Some places only need temporary read-access to the contents of a Maya buffer.  So
    // instead of creating a full SharedArray which goes in the ArrayRegistry, we can provide
    // an alternate implementation of ArrayReadInterface which provides a bare-bones temporary
    // memory buffer.  This is useful for selection, which is the most common case of readback
    // from renderable buffers.  Less common use cases are when gpuCache exports a copy of itself
    // into a new alembic cache file or when the viewport mode switches to the default viewport.
    class TempCopyReadableInterface : public ArrayReadInterface<T>
    {
         boost::shared_array<T> fLocalArray;
         GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_1;
    public:
        TempCopyReadableInterface(boost::shared_array<T> localArray) : fLocalArray(localArray) {}
        virtual ~TempCopyReadableInterface() {}

        virtual const T* get() const { return fLocalArray.get(); }
    };

public:
    typedef typename Array<T>::Digest Digest;
    
    static boost::shared_ptr<Array<T> > create(const boost::shared_ptr<C>& mayaBuffer, Digest digest)
    {
        // The Digest is pre-calculated.
        size_t size = MayaBufferSizeHelper(mayaBuffer.get());
       
        // We first look if a similar array already exists in the
        // cache. If so, we return the cached array to promote sharing as
        // much as possible.
        boost::shared_ptr<Array<T> > ret;
        {
            tbb::mutex::scoped_lock lock(ArrayRegistry<T>::mutex());

            ret = ArrayRegistry<T>::lookupNonReadable(digest, size);
        
            if (!ret) {
                ret = boost::make_shared<MayaBufferArray<T, C> >(
                    mayaBuffer, digest);
                ArrayRegistry<T>::insert(ret);
            }
        }

        return ret;
    }


    virtual ~MayaBufferArray() {}


    virtual boost::shared_ptr<const ArrayReadInterface<T> > getReadable() const
    {
        // Get a temporary readable copy of the buffer contents.  Nothing new
        // will be registered with the ArrayRegistry.
        // This function can only be called from the main thread.
        boost::shared_ptr<const TempCopyReadableInterface> ret(boost::make_shared<const TempCopyReadableInterface>(GetTempArrayCopy()));
        return ret;
    }


    virtual boost::shared_ptr<ReadableArray<T> > getReadableArray() const
    {
        // Get a full-fledged SharedArray version of the buffer contents.  This
        // SharedArray will be registered with the ArrayRegistry.
        // This function can only be called from the main thread.
        {
            // If the readable version already exists in the registry, return that one.
            tbb::mutex::scoped_lock lock(ArrayRegistry<T>::mutex());

            boost::shared_ptr<ReadableArray<T> > ret;
            // Linux gcc complains about these base class functions unless they are explicitly 
            // disambiguated by proving this->
            ret = ArrayRegistry<T>::lookupReadable(this->digest(), this->bytes());
            if (ret)
                return ret;
        }

        // If the readable version doesn't exist in the registry, then create one.
        boost::shared_array<T> rawData(GetTempArrayCopy());

        return SharedArray<T>::create(rawData, this->digest(), this->bytes()/sizeof(T));
    }

    boost::shared_ptr<C> getMBuffer() const
    {
        return fMayaBuffer;
    }

private:

    MayaBufferArray(const boost::shared_ptr<C>& mayaBuffer, Digest digest)  // private constructor.  use create() instead.
        : Array<T>(MayaBufferSizeHelper(mayaBuffer.get()), digest, false)
        , fMayaBuffer(mayaBuffer)
    {}

    MayaBufferArray(const MayaBufferArray& other);  //private and unimplemented

    boost::shared_array<T> GetTempArrayCopy() const
    {
        // Read the buffer contents back out of the Maya buffer and store it in a
        // temporary system memory buffer.
        // If the Maya buffer is resident in GPU ram, then the graphics API calls to 
        // access it can only be performed from the main thread.  gpuCache uses a
        // worker thread for file reading, so that code in CacheReaderAlembic has
        // to avoid converting Arrays into ReadableArrays.  It is possible that the
        // file reader thread may create an array which duplicates the contents of
        // a MayaBufferArray, but that situation should clean itself up when the array
        // is eventually converted into a BufferEntry for rendering.

        // We copy the data into a temporary buffer instead of just holding the mapped
        // pointer because the selection code intermixes buffer readback with it's own
        // OpenGL calls. That conflicts with leaving the buffer bound for mapping in vp2.
        // The unmap() API function guarantees that it resets the GL buffer binding to 0
        // so this will behave predictably mixed with other GL code.

        assert(gsMainThreadId == tbb::this_tbb_thread::get_id());
        if (gsMainThreadId != tbb::this_tbb_thread::get_id())
            return boost::shared_array<T>();

        const T* src = (const T *)fMayaBuffer->map();
        size_t numBytes = this->bytes();
        size_t numValues = numBytes / sizeof(T);
        boost::shared_array<T> rawData(new T[numValues]);
        memcpy(rawData.get(), src, numBytes);
        fMayaBuffer->unmap();

        return rawData;
    }
    
    GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_2;

    const boost::shared_ptr<C> fMayaBuffer;
};


// Explicitly instantiate the two versions we need.
template class MayaBufferArray<unsigned int, MHWRender::MIndexBuffer>;
template class MayaBufferArray<float, MHWRender::MVertexBuffer>;

typedef MayaBufferArray<unsigned int, MHWRender::MIndexBuffer> MayaIndexBufferWrapper;
typedef MayaBufferArray<float, MHWRender::MVertexBuffer> MayaVertexBufferWrapper;



//==============================================================================
// CLASS BuffersCache
//==============================================================================

// This class manages all Viewport 2.0 buffers.
// When VRAM is hitting the threshold, the cache will delete free buffers to get 
// more room for the new buffers.
// Allocating and evicting are done between frames.
class BuffersCache : boost::noncopyable
{
public:
    static BuffersCache& getInstance()
    {
        // Singleton
        static BuffersCache sSingleton;
        return sSingleton;
    }

    // Set Viewport 2.0 buffers to the render item and add these buffers
    // to this cache. This means that these buffers are going to be used
    // in the render item.
    void setBuffers(
        SubSceneOverride&                            subSceneOverride,
        MRenderItem*                                 renderItem,
        const boost::shared_ptr<const IndexBuffer>&  indices,
        const boost::shared_ptr<const VertexBuffer>& positions,
        const boost::shared_ptr<const VertexBuffer>& normals,
        const boost::shared_ptr<const VertexBuffer>& uvs,
        const MBoundingBox&                          boundingBox
    )
    {
        assert(indices);
        if (!indices) return;

        assert(positions);
        if (!positions) return;

        // Unloaded render item! Just count the reference.
        if (!renderItem) {
            acquireIndexBuffer(indices);
            acquireVertexBuffer(positions);
            if (normals) {
                acquireVertexBuffer(normals);
            }
            if (uvs) {
                acquireVertexBuffer(uvs);
            }
            return;
        }

        // Semantic Constants
        static const MString sPositions("positions");
        static const MString sNormals("normals");
        static const MString sUVs("uvs");

        MVertexBufferArray buffers;
        buffers.addBuffer(sPositions, acquireVertexBuffer(positions));
        if (normals) {
            buffers.addBuffer(sNormals, acquireVertexBuffer(normals));
        }
        if (uvs) {
            buffers.addBuffer(sUVs, acquireVertexBuffer(uvs));
        }

        subSceneOverride.setGeometryForRenderItem(
            *renderItem,
            buffers,
            *acquireIndexBuffer(indices),
            &boundingBox
        );
    }

    // Remove Viewport 2.0 buffers from this cache. This means that these
    // buffers is no longer used (and might become free buffers and then deleted).
    void removeBuffers(
        const boost::shared_ptr<const IndexBuffer>&  indices,
        const boost::shared_ptr<const VertexBuffer>& positions,
        const boost::shared_ptr<const VertexBuffer>& normals = boost::shared_ptr<const VertexBuffer>(),
        const boost::shared_ptr<const VertexBuffer>& uvs     = boost::shared_ptr<const VertexBuffer>()
    )
    {
        if (indices) {
            removeBufferFromCache(indices);
        }

        if (positions) {
            removeBufferFromCache(positions);
        }

        if (normals) {
            removeBufferFromCache(normals);
        }

        if (uvs) {
            removeBufferFromCache(uvs);
        }
    }

    // Shorthand method to do removeBuffers() and setBuffers()
    void updateBuffers(
        SubSceneOverride&                            subSceneOverride,
        MRenderItem*                                 renderItem,
        const boost::shared_ptr<const IndexBuffer>&  indices,
        const boost::shared_ptr<const VertexBuffer>& positions,
        const boost::shared_ptr<const VertexBuffer>& normals,
        const boost::shared_ptr<const VertexBuffer>& uvs,
        const MBoundingBox&                          boundingBox,
        const boost::shared_ptr<const IndexBuffer>&  prevIndices,
        const boost::shared_ptr<const VertexBuffer>& prevPositions,
        const boost::shared_ptr<const VertexBuffer>& prevNormals = boost::shared_ptr<const VertexBuffer>(),
        const boost::shared_ptr<const VertexBuffer>& prevUVs     = boost::shared_ptr<const VertexBuffer>()
    )
    {
        removeBuffers(prevIndices, prevPositions, prevNormals, prevUVs);
        setBuffers(subSceneOverride, renderItem, indices, positions, normals, uvs, boundingBox);
    }

    // Find the Viewport 2.0 index buffer in the cache. Returns NULL if not found.
    MIndexBuffer* lookup(const boost::shared_ptr<const IndexBuffer>& indices)
    {
        MIndexBuffer* buffer = NULL;
        lookup(indices, buffer);
        return buffer;
    }

    // Find the Viewport 2.0 vertex buffer in the cache. Returns NULL if not found.
    MVertexBuffer* lookup(const boost::shared_ptr<const VertexBuffer>& vertices)
    {
        MVertexBuffer* buffer = NULL;
        lookup(vertices, buffer);
        return buffer;
    }

    // Shrink the cache if the total size of buffers is hitting the threshold.
    // Buffers with 0 reference count will be deleted.
    void shrink()
    {
        // Delete Viewport 2.0 buffers that are queued for deletion.
        // Their IndexBuffer/VertexBuffer arrays have already been deleted.
        doDeleteQueuedBuffers();

        while (fTotalBufferSize > Config::maxVBOSize()) {
            // No more free buffers can be deleted.
            // All active buffers are already used by render items.
            if (fFreeBuffers.empty()) break;

            // Delete a random free buffer.
            BufferSet::iterator it = fFreeBuffers.begin();
            fTotalBufferSize -= (*it).bytes();
            fFreeBuffers.erase(it);
        }
    }

    // Clear and delete all buffers.
    void clear()
    {
        fTotalBufferSize = 0;
        fActiveBuffers.clear();
        fFreeBuffers.clear();

        {
            tbb::mutex::scoped_lock lock(fBuffersToDeleteMutex);
            fBuffersToDelete.clear();
        }
    }

private:
    class BufferEntry;

    BuffersCache()
        : fTotalBufferSize(0)
    {
        ArrayBase::registerDestructionCallback(sArrayDestructionCb);
    }

    ~BuffersCache()
    {
        ArrayBase::unregisterDestructionCallback(sArrayDestructionCb);
    }

    // Allocate an index buffer or return the existing index buffer.
    // This will add the reference count by 1.
    MIndexBuffer* acquireIndexBuffer(const boost::shared_ptr<const IndexBuffer>& indices)
    {
        assert(indices);
        MIndexBuffer* buffer = NULL;
        addBufferToCache(indices).getBuffer(buffer);
        return buffer;
    }

    // Allocate a vertex buffer or return the existing vertex buffer.
    // This will add the reference count by 1.
    MVertexBuffer* acquireVertexBuffer(const boost::shared_ptr<const VertexBuffer>& vertices)
    {
        assert(vertices);
        MVertexBuffer* buffer = NULL;
        addBufferToCache(vertices).getBuffer(buffer);
        return buffer;
    }

    // Add the buffer to the cache. If the buffer already exists
    // in the cache, the reference count will be added by 1.
    template<typename T>
    const BufferEntry& addBufferToCache(const T& buffer)
    {
        // Already a buffer in use?
        BufferSet::iterator it = fActiveBuffers.find(buffer);
        if (it != fActiveBuffers.end()) {
            // The buffer is already used by a render item.
            // Just increase the reference count.
            assert((*it).refCount() > 0);
            (*it).ref();
            return *it;
        }

        // A free buffer?
        it = fFreeBuffers.find(buffer);
        if (it != fFreeBuffers.end()) {
            // The buffer is not used by any render items.
            // Move it to active buffer set and increase the reference count.
            assert((*it).refCount() == 0);
            BufferSet::iterator newIt = fActiveBuffers.insert(*it).first;
            fFreeBuffers.erase(it);
            (*newIt).ref();
            return *newIt;
        }

        // Allocate a new buffer.
        // This will construct a new MIndexBuffer or MVertexBuffer.
        BufferSet::iterator newIt = fActiveBuffers.insert(buffer).first;
        (*newIt).ref();
        fTotalBufferSize += (*newIt).bytes();
        return *newIt;
    }

    // Declaim that the buffer is not used by a render item.
    // The reference count will be decreased by 1.
    // If the reference count reaches 0, the buffer has the 
    // possibility to be deleted in shrink().
    template<typename T>
    void removeBufferFromCache(const T& buffer)
    {
        // This must be a buffer in use.
        BufferSet::iterator it = fActiveBuffers.find(buffer);
        assert(fFreeBuffers.find(buffer) == fFreeBuffers.end());

        if (it != fActiveBuffers.end()) {
            assert((*it).refCount() > 0);
            (*it).unref();

            // This buffer is no longer used by any render items.
            // Move it to free buffer set.
            if ((*it).refCount() == 0) {
                fFreeBuffers.insert(*it);
                fActiveBuffers.erase(it);
            }
        }
    }

    // Find the buffer in the cache.
    // This will not allocate any new buffers.
    template<typename T, typename R>
    void lookup(const T& buffer, R& pointer)
    {
        // In active buffer set?
        BufferSet::iterator it = fActiveBuffers.find(buffer);
        if (it != fActiveBuffers.end()) {
            assert((*it).refCount() > 0);
            (*it).getBuffer(pointer);
            return;
        }

        // In free buffer set?
        it = fFreeBuffers.find(buffer);
        if (it != fFreeBuffers.end()) {
            assert((*it).refCount() == 0);
            (*it).getBuffer(pointer);
            return;
        }

        pointer = NULL;
    }

    // Queue the buffer for deletion. This method is thread-safe.
    // Sometimes, a buffer might be deleted in a worker thread.
    void queueBufferForDelete(const ArrayBase::Key& key)
    {
        tbb::mutex::scoped_lock lock(fBuffersToDeleteMutex);
        fBuffersToDelete.insert(key);
    }

    // Delete all queued buffers. This method must be called from the main thread.
    void doDeleteQueuedBuffers()
    {
        if (!fBuffersToDelete.empty()) {
            tbb::mutex::scoped_lock lock(fBuffersToDeleteMutex);

            typedef BufferSet::nth_index<1>::type::iterator KeyIterator;
            BOOST_FOREACH (const ArrayBase::Key& key, fBuffersToDelete) {
                // Find all the buffers that have the same key.
                // It's possible that the array is used for both position and normal.
                std::pair<KeyIterator,KeyIterator> range = 
                    fFreeBuffers.get<1>().equal_range(key);
                for (KeyIterator it = range.first; it != range.second; it++) {
                    assert((*it).refCount() == 0);
                    fTotalBufferSize -= (*it).bytes();
                }
                fFreeBuffers.get<1>().erase(key);
            }

            // Done.
            fBuffersToDelete.clear();
        }
    }

    static void sArrayDestructionCb(const ArrayBase::Key& key)
    {
        // Queue the buffers for deletion.
        BuffersCache::getInstance().queueBufferForDelete(key);

        // Delete the buffers immediately if we are in the main thread.
        if (tbb::this_tbb_thread::get_id() == gsMainThreadId) {
            BuffersCache::getInstance().doDeleteQueuedBuffers();
        }
    }

    // This class provides a common interface for vertex/index buffers.
    class BufferEntry
    {
    public:
        // The unique key for index/vertex buffers.
        struct BufferKey
        {
            enum BufferType { kIndex, kVertex };
            BufferType          type;
            ArrayBase::Key      arrayKey;
            MGeometry::DataType dataType;
            MGeometry::Semantic semantic;

            BufferKey(const boost::shared_ptr<const IndexBuffer>& indices)
                : type(kIndex),
                  arrayKey(indices->array()->key()),
                  dataType(MGeometry::kUnsignedInt32),
                  semantic(MGeometry::kInvalidSemantic)
            {}

            BufferKey(const boost::shared_ptr<const VertexBuffer>& vertices)
                : type(kVertex),
                  arrayKey(vertices->array()->key()),
                  dataType(vertices->descriptor().dataType()),
                  semantic(vertices->descriptor().semantic())
            {}
        };

        struct BufferKeyHash : std::unary_function<BufferKey, std::size_t>
        {
            std::size_t operator()(const BufferKey& key) const
            {
                std::size_t hashCode = 0;
                boost::hash_combine(hashCode, key.type);
                boost::hash_combine(hashCode, ArrayBase::KeyHash()(key.arrayKey));
                boost::hash_combine(hashCode, key.dataType);
                boost::hash_combine(hashCode, key.semantic);
                return hashCode;
            }
        };

        struct BufferKeyEqualTo : std::binary_function<BufferKey, BufferKey, bool>
        {
            bool operator()(const BufferKey& x, const BufferKey& y) const
            {
                return x.type == y.type &&
                        ArrayBase::KeyEqualTo()(x.arrayKey, y.arrayKey) &&
                        x.dataType == y.dataType &&
                        x.semantic == y.semantic;
            }
        };

        BufferEntry(const boost::shared_ptr<const IndexBuffer>& indices)
            : fKey(indices),
              fRefCount(0)
        {
            // Allocate the index buffer and initialize the contents.
            if (indices->numIndices() > 0) {

                if (!indices->array()->isReadable())
                {
                    // The IndexBuffer has already been converted to a Maya buffer so we can reuse it.  This can
                    // happen if the BufferEntry has been deleted but the IndexBuffer that it converted remains
                    // and is being reused.  We want to avoid an expensive readback and creation of a duplicate buffer.
                    const MayaIndexBufferWrapper* mbufferWrapper = dynamic_cast<const MayaIndexBufferWrapper*>(indices->array().get());
                    assert(mbufferWrapper);
                    if (mbufferWrapper) {
                        boost::shared_ptr<MIndexBuffer> mbuffer = mbufferWrapper->getMBuffer();
                        assert(mbuffer);
                        if (mbuffer) {
                            fIndexBuffer = mbuffer;
                            return;
                        }
                    }
                }


                fIndexBuffer.reset(new MIndexBuffer(MGeometry::kUnsignedInt32));

                {
                    IndexBuffer::ReadInterfacePtr readable = indices->readableInterface();
                    const IndexBuffer::index_t* data = readable->get();
                    fIndexBuffer->update(data, 0, (unsigned int)indices->numIndices(), true);
                }

                // We want to avoid storing two copies of all the scene geometry.  One copy of the scene
                // goes into the Maya SubSceneOverride interface.  The other copy of the scene stored in
                // ReadableArrays is now mostly redundant.  If we want to load huge scenes close to the limit
                // of our system ram, then we can't keep the local ReadableArray copy.
                // So after creating the Maya MIndexBuffer, we graft a non-readable version of the Array
                // back into the IndexBuffer.  The readable version that it previously held can then be freed.

                if (indices->array()->isReadable()) {
                    boost::shared_ptr<Array<IndexBuffer::index_t> > mayaArray = MayaIndexBufferWrapper::create(fIndexBuffer, indices->array()->digest());
                    indices->ReplaceArrayInstance(mayaArray);
                }
            }
        }

        BufferEntry(const boost::shared_ptr<const VertexBuffer>& vertices)
            : fKey(vertices),
              fRefCount(0)
        {
            // Allocate the vertex buffer and initialize the contents.
            if (vertices->numVerts() > 0) {
                // FIXME: Assumes 32-bit float data.
                assert(fKey.dataType == MGeometry::kFloat);

                bool allowReplaceBufferArray = true;
                if (!vertices->array()->isReadable())
                {
                    // The VertexBuffer has already been converted to a Maya buffer.  We can reuse it if the
                    // semantic matches.  This can happen if the BufferEntry has been deleted but the VertexBuffer
                    // that it converted remains and is being reused.  We want to avoid an expensive readback and
                    // creation of a duplicate buffer.
                    const MayaVertexBufferWrapper* mbufferWrapper = dynamic_cast<const MayaVertexBufferWrapper*>(vertices->array().get());
                    assert(mbufferWrapper);
                    if (mbufferWrapper) {
                        boost::shared_ptr<MVertexBuffer> mbuffer = mbufferWrapper->getMBuffer();
                        assert(mbuffer);
                        if (mbuffer) {
                            if (mbuffer->descriptor().semantic() == vertices->descriptor().semantic()) {
                                // The semantic matches.  Simply reuse the buffer and we are finished.
                                fVertexBuffer = mbuffer;
                                return;

                            } else {
                                // The semantic doesn't match, so we can't reuse the buffer.  An example is a normal
                                // and position buffer that happen to match their contents.  The unique key rules mean
                                // that we can't make a duplicate MayaVertexBufferWrapper, so make a new MBuffer backed
                                // by a plain software buffer.  Graft the software buffer back into the VertexBuffer so
                                // that we store both.
                                boost::shared_ptr<Array<float> > softwareArray = vertices->array()->getReadableArray();
                                vertices->ReplaceArrayInstance(softwareArray);
                                allowReplaceBufferArray = false;
                                // Now proceed with normal MBuffer creation, but skip the final step of converting the
                                // VertexBuffer back.
                            }
                        }
                    }
                }


                fVertexBuffer.reset(new MVertexBuffer(vertices->descriptor()));

                {
                    VertexBuffer::ReadInterfacePtr readable = vertices->readableInterface();
                    const float* data = readable->get();
                    fVertexBuffer->update(data, 0, (unsigned int)vertices->numVerts(), true);
                }
                
                // We want to avoid storing two copies of all the scene geometry.  One copy of the scene
                // goes into the Maya SubSceneOverride interface.  The other copy of the scene stored in
                // ReadableArrays is now mostly redundant.  If we want to load huge scenes close to the limit
                // of our system ram, then we can't keep the local ReadableArray copy.
                // So after creating the Maya MVertexBuffer, we graft a non-readable version of the Array
                // back into the VertexBuffer.  The readable version that it previously held can then be freed.
                if (allowReplaceBufferArray && vertices->array()->isReadable()) {
                    boost::shared_ptr<Array<float> > mayaArray = MayaVertexBufferWrapper::create(fVertexBuffer, vertices->array()->digest());
                    vertices->ReplaceArrayInstance(mayaArray);
                }
            }
        }

        // The unique key of this buffer.
        const BufferKey&      key() const      { return fKey; }

        // The array key of this buffer. (without type and semantic)
        const ArrayBase::Key& arrayKey() const { return fKey.arrayKey; }

        // The size of this buffer.
        size_t bytes() const    { return fKey.arrayKey.fBytes; }

        // Get the index buffer pointer.
        void getBuffer(MIndexBuffer*& buffer) const
        {
            assert(fIndexBuffer);
            buffer = fIndexBuffer.get();
        }

        // Get the vertex buffer pointer.
        void getBuffer(MVertexBuffer*& buffer) const
        {
            assert(fVertexBuffer);
            buffer = fVertexBuffer.get();
        }

        void   ref() const      { fRefCount++; }
        void   unref() const    { fRefCount--; }
        size_t refCount() const { return fRefCount; }
        
    private:
        BufferKey                           fKey;
        boost::shared_ptr<MIndexBuffer>     fIndexBuffer;
        boost::shared_ptr<MVertexBuffer>    fVertexBuffer;
        mutable size_t                      fRefCount;
    };

    typedef boost::multi_index_container<
        BufferEntry,
        boost::multi_index::indexed_by<
            // Index 0: The unique index for each Viewport 2.0 buffer.
            boost::multi_index::hashed_unique<
                BOOST_MULTI_INDEX_CONST_MEM_FUN(BufferEntry,const BufferEntry::BufferKey&,key),
                BufferEntry::BufferKeyHash,
                BufferEntry::BufferKeyEqualTo
            >,
            // Index 1: The index for each array murmur3 key.
            //          Note: The same key might be used for different semantic.
            boost::multi_index::hashed_non_unique<
                BOOST_MULTI_INDEX_CONST_MEM_FUN(BufferEntry,const ArrayBase::Key&,arrayKey),
                ArrayBase::KeyHash,
                ArrayBase::KeyEqualTo
            >
        >
    > BufferSet;

    typedef boost::unordered_set<
        ArrayBase::Key,
        ArrayBase::KeyHash,
        ArrayBase::KeyEqualTo
    > KeySet;

    size_t          fTotalBufferSize;
    BufferSet       fActiveBuffers;
    BufferSet       fFreeBuffers;

    tbb::mutex      fBuffersToDeleteMutex;
    KeySet          fBuffersToDelete;
};


// The user data is attached on bounding box place holder render items.
// When the bounding box place holder is drawn, a post draw callback is
// triggered to hint the shape should be read in priority.
class SubNodeUserData : public MUserData
{
public:
    SubNodeUserData(const SubNode& subNode)
        : MUserData(false /*deleteAfterUse*/),
          fSubNode(subNode)
    {}

    virtual ~SubNodeUserData()
    {}

    void hintShapeReadOrder() const
    {
        // Hint the shape read order.
        // The shape will be loaded in priority.
        GlobalReaderCache::theCache().hintShapeReadOrder(fSubNode);
    }

private:
    const SubNode& fSubNode;
};

// For GPUCache OGS draw, make sure the pattern has its first bit == 1
static void SetDashLinePattern(MShaderInstance* shader, unsigned short pattern)
{
	static const MString sDashPattern = "dashPattern";

	unsigned short newPattern = pattern;
	if (newPattern != 0) {
		while ((newPattern & 0x8000) == 0) {
			newPattern <<= 1;
		}
	}
	shader->setParameter(sDashPattern, newPattern);
}

void BoundingBoxPlaceHolderDrawCallback(MDrawContext& context,
                                        const MRenderItemList& renderItemList,
                                        MShaderInstance* shader)
{
    int numRenderItems = renderItemList.length();
    for (int i = 0; i < numRenderItems; i++) {
        const MRenderItem* renderItem = renderItemList.itemAt(i);
        if (renderItem) {
            SubNodeUserData* userData =
                dynamic_cast<SubNodeUserData*>(renderItem->customData());
            if (userData) {
                userData->hintShapeReadOrder();
            }
        }
    }
}

void WireframePreDrawCallback(MDrawContext& context,
                              const MRenderItemList& renderItemList,
                              MShaderInstance* shader)
{
    // Wireframe on Shaded: Full / Reduced / None
    const DisplayPref::WireframeOnShadedMode wireOnShadedMode =
        DisplayPref::wireframeOnShadedMode();

    // Early out if we are not drawing Reduced/None wireframe.
    if (wireOnShadedMode == DisplayPref::kWireframeOnShadedFull) {
        assert(0);  // Only Reduced/None mode has callbacks.
        return;
    }

    // Wireframe on shaded.
    unsigned int displayStyle = context.getDisplayStyle();
    if (displayStyle & (MDrawContext::kGouraudShaded | MDrawContext::kTextured)) {
        const unsigned short pattern =
            (wireOnShadedMode == DisplayPref::kWireframeOnShadedReduced)
            ? Config::kLineStippleDotted  // Reduce: dotted line
            : 0;                          // None: no wire
        static const MString sDashPattern = "dashPattern";        
		SetDashLinePattern(shader, pattern);
    }
}

void WireframePostDrawCallback(MDrawContext& context,
                               const MRenderItemList& renderItemList,
                               MShaderInstance* shader)
{
    // Wireframe on Shaded: Full / Reduced / None
    const DisplayPref::WireframeOnShadedMode wireOnShadedMode =
        DisplayPref::wireframeOnShadedMode();

    // Early out if we are not drawing reduced wireframe.
    if (wireOnShadedMode == DisplayPref::kWireframeOnShadedFull) {
        assert(0);  // Only Reduced/None mode has callbacks.
        return;
    }

    // Restore the default pattern.
    SetDashLinePattern(shader, Config::kLineStippleShortDashed);
}

MShaderInstance* getWireShaderInstance()
{
    MRenderer* renderer = MRenderer::theRenderer();
    if (!renderer) return NULL;
    const MShaderManager* shaderMgr = renderer->getShaderManager();
    if (!shaderMgr) return NULL;

    return shaderMgr->getFragmentShader("mayaDashLineShader", "", false);
}

MShaderInstance* getWireShaderInstanceWithCB()
{
    MRenderer* renderer = MRenderer::theRenderer();
    if (!renderer) return NULL;
    const MShaderManager* shaderMgr = renderer->getShaderManager();
    if (!shaderMgr) return NULL;

    return shaderMgr->getFragmentShader("mayaDashLineShader", "", false,
        WireframePreDrawCallback, WireframePostDrawCallback);
}

MShaderInstance* getBoundingBoxPlaceHolderShaderInstance()
{
    MRenderer* renderer = MRenderer::theRenderer();
    if (!renderer) return NULL;
    const MShaderManager* shaderMgr = renderer->getShaderManager();
    if (!shaderMgr) return NULL;

    return shaderMgr->getFragmentShader("mayaDashLineShader", "", false,
        NULL, BoundingBoxPlaceHolderDrawCallback);
}

MShaderInstance* getDiffuseColorShaderInstance()
{
    MRenderer* renderer = MRenderer::theRenderer();
    if (!renderer) return NULL;
    const MShaderManager* shaderMgr = renderer->getShaderManager();
    if (!shaderMgr) return NULL;

    return shaderMgr->getFragmentShader("mayaLambertSurface", "outSurfaceFinal", true);
}

void releaseShaderInstance(MShaderInstance*& shader)
{
    MRenderer* renderer = MRenderer::theRenderer();
    if (!renderer) return;
    const MShaderManager* shaderMgr = renderer->getShaderManager();
    if (!shaderMgr) return;

    if (shader) {
        shaderMgr->releaseShader(shader);
        shader = NULL;
    }
}

void setDiffuseColor(MShaderInstance* shader, const MColor& diffuseColor)
{
    if (shader) {
        // Color
        const float color[3] = {diffuseColor.r, diffuseColor.g, diffuseColor.b};
        shader->setParameter("color", color);

        // Transparency
        if (diffuseColor.a < 1.0f) {
            const float oneMinusAlpha =
                (diffuseColor.a >= 0.0f) ? 1.0f - diffuseColor.a : 1.0f;
            const float transparency[3] = {oneMinusAlpha, oneMinusAlpha, oneMinusAlpha};
            shader->setParameter("transparency", transparency);
            shader->setIsTransparent(true);
        }
        else {
            shader->setIsTransparent(false);
        }

        // Diffuse
        shader->setParameter("diffuse", 1.0f);
    }
}

bool useHardwareInstancing()
{
    // hardwareRenderingGlobals is a default node so we assume
    // that it will never be deleted.
    static MPlug sHwInstancingPlug;
    if (sHwInstancingPlug.isNull()) {
        MSelectionList sl;
        sl.add("hardwareRenderingGlobals.hwInstancing");
        MStatus stat = sl.getPlug(0, sHwInstancingPlug);
        MStatAssert(stat);
    }

    return sHwInstancingPlug.asBool() && Config::useHardwareInstancing();
}


//==============================================================================
// CLASS ShaderInstancePtr
//==============================================================================

// This class wraps a MShaderInstance* and its template shader.
class ShaderInstancePtr
{
public:
    // Invalid shader instance.
    ShaderInstancePtr()
    {}

    // Wraps a MShaderInstance* and its template MShaderInstance*.
    ShaderInstancePtr(boost::shared_ptr<MShaderInstance> shader,
                      boost::shared_ptr<MShaderInstance> source)
        : fShader(shader), fTemplate(source)
    {}

    ~ShaderInstancePtr()
    {}

    operator bool () const
    {
        return fShader && fTemplate;
    }

    MShaderInstance* operator->() const
    {
        assert(fShader);
        return fShader.get();
    }

    MShaderInstance* get() const
    {
        assert(fShader);
        return fShader.get();
    }

    boost::shared_ptr<MShaderInstance> getShader() const
    {
        assert(fShader);
        return fShader;
    }

    boost::shared_ptr<MShaderInstance> getTemplate() const
    {
        assert(fTemplate);
        return fTemplate;
    }

    void reset()
    {
        fShader.reset();
        fTemplate.reset();
    }

    bool operator==(const ShaderInstancePtr& rv) const
    {
        return fShader == rv.fShader && fTemplate == rv.fTemplate;
    }

    bool operator!=(const ShaderInstancePtr& rv) const
    {
        return !(operator==(rv));
    }

private:
    boost::shared_ptr<MShaderInstance> fShader;
    boost::shared_ptr<MShaderInstance> fTemplate;
};


//==============================================================================
// CLASS ShaderTemplatePtr
//==============================================================================

// This class wraps a MShaderInstance* as a template.
class ShaderTemplatePtr
{
public:
    // Invalid shader template.
    ShaderTemplatePtr()
    {}

    // Wrap a shader instance to be used as a template.
    ShaderTemplatePtr(boost::shared_ptr<MShaderInstance> source)
        : fTemplate(source)
    {}

    ~ShaderTemplatePtr()
    {}

    operator bool () const
    {
        return (fTemplate.get() != NULL);
    }

    MShaderInstance* get() const
    {
        assert(fTemplate);
        return fTemplate.get();
    }

    boost::shared_ptr<MShaderInstance> getTemplate() const
    {
        assert(fTemplate);
        return fTemplate;
    }

    typedef void (*Deleter)(MShaderInstance*);
    ShaderInstancePtr newShaderInstance(Deleter deleter) const
    {
        assert(fTemplate);
        boost::shared_ptr<MShaderInstance> newShader;
        newShader.reset(fTemplate->clone(), std::ptr_fun(deleter));
        return ShaderInstancePtr(newShader, fTemplate);
    }

private:
    boost::shared_ptr<MShaderInstance> fTemplate;
};


//==============================================================================
// CLASS ShaderCache
//==============================================================================

// This class manages the shader templates. A shader template can be used to create
// shader instances with different parameters.
class ShaderCache : boost::noncopyable
{
public:
    static ShaderCache& getInstance()
    {
        // Singleton
        static ShaderCache sSingleton;
        return sSingleton;
    }

    typedef void (*Deleter)(MShaderInstance*);

    ShaderInstancePtr newWireShader(Deleter deleter)
    {
        // Look for a cached shader.
        MString key = "_reserved_wire_shader_";
        FragmentAndShaderTemplateCache::nth_index<0>::type::iterator it =
            fFragmentCache.get<0>().find(key);

        // Found in cache.
        if (it != fFragmentCache.get<0>().end()) {
            ShaderTemplatePtr templateShader = it->ptr.lock();
            assert(templateShader);  // no staled pointer
            return templateShader.newShaderInstance(deleter);
        }

        // Not found. Get a new shader.
        ShaderTemplatePtr templateShader =
            wrapShaderTemplate(getWireShaderInstance());
        if (templateShader) {
            // Insert into cache.
            FragmentAndShaderTemplate entry;
            entry.fragmentAndOutput = key;
            entry.shader            = templateShader.get();
            entry.ptr               = templateShader.getTemplate();
            fFragmentCache.insert(entry);

            return templateShader.newShaderInstance(deleter);
        }

        assert(0);
        return ShaderInstancePtr();
    }

    ShaderInstancePtr newWireShaderWithCB(Deleter deleter)
    {
        // Look for a cached shader.
        MString key = "_reserved_wire_shader_with_cb_";
        FragmentAndShaderTemplateCache::nth_index<0>::type::iterator it =
            fFragmentCache.get<0>().find(key);

        // Found in cache.
        if (it != fFragmentCache.get<0>().end()) {
            ShaderTemplatePtr templateShader = it->ptr.lock();
            assert(templateShader);  // no staled pointer
            return templateShader.newShaderInstance(deleter);
        }

        // Not found. Get a new shader.
        ShaderTemplatePtr templateShader =
            wrapShaderTemplate(getWireShaderInstanceWithCB());
        if (templateShader) {
            // Insert into cache.
            FragmentAndShaderTemplate entry;
            entry.fragmentAndOutput = key;
            entry.shader            = templateShader.get();
            entry.ptr               = templateShader.getTemplate();
            fFragmentCache.insert(entry);

            return templateShader.newShaderInstance(deleter);
        }

        assert(0);
        return ShaderInstancePtr();
    }

    ShaderInstancePtr newBoundingBoxPlaceHolderShader(Deleter deleter)
    {
        // Look for a cached shader.
        MString key = "_reserved_bounding_box_place_holder_shader_";
        FragmentAndShaderTemplateCache::nth_index<0>::type::iterator it =
            fFragmentCache.get<0>().find(key);

        // Found in cache.
        if (it != fFragmentCache.get<0>().end()) {
            ShaderTemplatePtr templateShader = it->ptr.lock();
            assert(templateShader);  // no staled pointer
            return templateShader.newShaderInstance(deleter);
        }

        // Not found. Get a new shader.
        ShaderTemplatePtr templateShader =
            wrapShaderTemplate(getBoundingBoxPlaceHolderShaderInstance());
        if (templateShader) {
            // Insert into cache.
            FragmentAndShaderTemplate entry;
            entry.fragmentAndOutput = key;
            entry.shader            = templateShader.get();
            entry.ptr               = templateShader.getTemplate();
            fFragmentCache.insert(entry);

            return templateShader.newShaderInstance(deleter);
        }

        assert(0);
        return ShaderInstancePtr();
    }

    ShaderInstancePtr newDiffuseColorShader(Deleter deleter)
    {
        // Look for a cached shader.
        MString key = "_reserved_diffuse_color_shader_";
        FragmentAndShaderTemplateCache::nth_index<0>::type::iterator it =
            fFragmentCache.get<0>().find(key);

        // Found in cache.
        if (it != fFragmentCache.get<0>().end()) {
            ShaderTemplatePtr templateShader = it->ptr.lock();
            assert(templateShader);  // no staled pointer
            return templateShader.newShaderInstance(deleter);
        }

        // Not found. Get a new shader.
        ShaderTemplatePtr templateShader =
            wrapShaderTemplate(getDiffuseColorShaderInstance());
        if (templateShader) {
            // Insert into cache.
            FragmentAndShaderTemplate entry;
            entry.fragmentAndOutput = key;
            entry.shader            = templateShader.get();
            entry.ptr               = templateShader.getTemplate();
            fFragmentCache.insert(entry);

            return templateShader.newShaderInstance(deleter);
        }

        assert(0);
        return ShaderInstancePtr();
    }

    ShaderInstancePtr newFragmentShader(const MString& fragmentName,
                                        const MString& outputStructName,
                                        Deleter        deleter)
    {
        // Look for a cached shader.
        MString key = fragmentName + ":" + outputStructName;
        FragmentAndShaderTemplateCache::nth_index<0>::type::iterator it =
            fFragmentCache.get<0>().find(key);

        // Found in cache.
        if (it != fFragmentCache.get<0>().end()) {
            ShaderTemplatePtr templateShader = it->ptr.lock();
            assert(templateShader);  // no staled pointer
            return templateShader.newShaderInstance(deleter);
        }

        // Not found. Get a new shader.
        ShaderTemplatePtr templateShader =
            createFragmentShader(fragmentName, outputStructName);
        if (templateShader) {
            // Insert into cache.
            FragmentAndShaderTemplate entry;
            entry.fragmentAndOutput = key;
            entry.shader            = templateShader.get();
            entry.ptr               = templateShader.getTemplate();
            fFragmentCache.insert(entry);

            return templateShader.newShaderInstance(deleter);
        }

        assert(0);
        return ShaderInstancePtr();
    }

private:
    ShaderCache()  {}
    ~ShaderCache() {}

    // Release the MShaderInstance and remove the pointer from the cache.
    static void shaderTemplateDeleter(MShaderInstance* shader)
    {
        assert(shader);
        getInstance().removeShaderTemplateFromCache(shader);
        releaseShaderInstance(shader);
    }

    // Remove the pointer from the cache.
    void removeShaderTemplateFromCache(MShaderInstance* shader)
    {
        assert(shader);
        if (!shader) return;

        // Remove the MShaderInstance* from the cache.
        fFragmentCache.get<1>().erase(shader);
    }

    // Wrap the MShaderInstance* as template.
    ShaderTemplatePtr wrapShaderTemplate(MShaderInstance* shader)
    {
        assert(shader);
        if (!shader) return ShaderTemplatePtr();

        boost::shared_ptr<MShaderInstance> ptr;
        ptr.reset(shader, std::ptr_fun(shaderTemplateDeleter));
        return ShaderTemplatePtr(ptr);
    }

    // Create a shader template from a Maya fragment.
    ShaderTemplatePtr createFragmentShader(const MString& fragmentName,
                                           const MString& outputStructName)
    {
        MRenderer* renderer = MRenderer::theRenderer();
        if (!renderer) return ShaderTemplatePtr();
        const MShaderManager* shaderMgr = renderer->getShaderManager();
        if (!shaderMgr) return ShaderTemplatePtr();

        return wrapShaderTemplate(
            shaderMgr->getFragmentShader(fragmentName, outputStructName, true));
    }

private:
    struct FragmentAndShaderTemplate {
        MString                          fragmentAndOutput;
        MShaderInstance*                 shader;
        boost::weak_ptr<MShaderInstance> ptr;
    };
    typedef boost::multi_index_container<
        FragmentAndShaderTemplate,
        boost::multi_index::indexed_by<
            boost::multi_index::hashed_unique<
                BOOST_MULTI_INDEX_MEMBER(FragmentAndShaderTemplate,MString,fragmentAndOutput),
                MStringHash
            >,
            boost::multi_index::hashed_unique<
                BOOST_MULTI_INDEX_MEMBER(FragmentAndShaderTemplate,MShaderInstance*,shader)
            >
        >
    > FragmentAndShaderTemplateCache;

    FragmentAndShaderTemplateCache  fFragmentCache;
};


//==============================================================================
// CLASS MaterialGraphTranslatorShaded
//==============================================================================

// This class translates a MaterialGraph to a MShaderInstance*
// that can be used in VP2.0.
class MaterialGraphTranslatorShaded : public ConcreteMaterialNodeVisitor
{
public:
    // Create a new shader instance.
    typedef void (*Deleter)(MShaderInstance*);
    MaterialGraphTranslatorShaded(Deleter deleter, double timeInSeconds)
        : fShader(), fDeleter(deleter), fTimeInSeconds(timeInSeconds)
    {}

    // Update an existing shader instance.
    MaterialGraphTranslatorShaded(ShaderInstancePtr& shader, double timeInSeconds)
        : fShader(shader), fDeleter(NULL), fTimeInSeconds(timeInSeconds)
    {}

    virtual ~MaterialGraphTranslatorShaded() {}

    ShaderInstancePtr getShader() const
    { return fShader; }

    virtual void visit(const LambertMaterial& node)
    {
        if (!fShader) {
            createShader("mayaLambertSurface", "outSurfaceFinal");
        }

        setupLambert(node);
    }

    virtual void visit(const PhongMaterial& node)
    {
        if (!fShader) {
            createShader("mayaPhongSurface", "outSurfaceFinal");
        }

        setupPhong(node);
        setupLambert(node);
    }

    virtual void visit(const BlinnMaterial& node)
    {
        if (!fShader) {
            createShader("mayaBlinnSurface", "outSurfaceFinal");
        }

        setupBlinn(node);
        setupLambert(node);
    }

    // Nodes that can't be used as root material node.
    virtual void visit(const SurfaceMaterial& node) {}
    virtual void visit(const Texture2d& node) {}
    virtual void visit(const FileTexture& node) {}

private:
    void createShader(const MString& fragmentName,
                      const MString& structOutputName)
    {
        assert(fDeleter);
        fShader = ShaderCache::getInstance().newFragmentShader(
            fragmentName, structOutputName, fDeleter);
        assert(fShader);
    }

    void setupLambert(const LambertMaterial& lambert)
    {
        if (!fShader) return;

        // Color
        {
            const MColor color =
                ShadedModeColor::evaluateDefaultColor(lambert.Color, fTimeInSeconds);
            const float buffer[3] = {color.r, color.g, color.b};
            fShader->setParameter("color", buffer);
        }

        // Transparency
        {
            const MColor transparency =
                ShadedModeColor::evaluateColor(lambert.Transparency, fTimeInSeconds);
            const float buffer[3] = {transparency.r, transparency.g, transparency.b};
            fShader->setParameter("transparency", buffer);

            if (transparency.r > 0 || transparency.g > 0 || transparency.b > 0) {
                fShader->setIsTransparent(true);
            }
            else {
                fShader->setIsTransparent(false);
            }
        }

        // Ambient Color
        {
            const MColor ambientColor =
                ShadedModeColor::evaluateColor(lambert.AmbientColor, fTimeInSeconds);
            const float buffer[3] = {ambientColor.r, ambientColor.g, ambientColor.b};
            fShader->setParameter("ambientColor", buffer);
        }

        // Incandescence
        {
            const MColor incandescence =
                ShadedModeColor::evaluateColor(lambert.Incandescence, fTimeInSeconds);
            const float buffer[3] = {incandescence.r, incandescence.g, incandescence.b};
            fShader->setParameter("incandescence", buffer);
        }

        // Diffuse
        {
            const float diffuse =
                ShadedModeColor::evaluateFloat(lambert.Diffuse, fTimeInSeconds);
            fShader->setParameter("diffuse", diffuse);
        }

        // Translucence
        {
            const float translucence =
                ShadedModeColor::evaluateFloat(lambert.Translucence, fTimeInSeconds);
            fShader->setParameter("translucence", translucence);
        }

        // Translucence Depth
        {
            const float translucenceDepth =
                ShadedModeColor::evaluateFloat(lambert.TranslucenceDepth, fTimeInSeconds);
            fShader->setParameter("translucenceDepth", translucenceDepth);
        }

        // Translucence Focus
        {
            const float translucenceFocus =
                ShadedModeColor::evaluateFloat(lambert.TranslucenceFocus, fTimeInSeconds);
            fShader->setParameter("translucenceFocus", translucenceFocus);
        }

        // Hide Source
        {
            const bool hideSource =
                ShadedModeColor::evaluateBool(lambert.HideSource, fTimeInSeconds);
            fShader->setParameter("hideSource", hideSource);
        }

        // Glow Intensity
        {
            const float glowIntensity =
                ShadedModeColor::evaluateFloat(lambert.GlowIntensity, fTimeInSeconds);
            fShader->setParameter("glowIntensity", glowIntensity);
        }
    }

    void setupPhong(const PhongMaterial& phong)
    {
        if (!fShader) return;

        // Cosine Power
        {
            const float cosinePower =
                ShadedModeColor::evaluateFloat(phong.CosinePower, fTimeInSeconds);
            fShader->setParameter("cosinePower", cosinePower);
        }

        // Specular Color
        {
            const MColor specularColor =
                ShadedModeColor::evaluateColor(phong.SpecularColor, fTimeInSeconds);
            const float buffer[3] = {specularColor.r, specularColor.g, specularColor.b};
            fShader->setParameter("specularColor", buffer);
        }

        // Reflectivity
        {
            const float reflectivity =
                ShadedModeColor::evaluateFloat(phong.Reflectivity, fTimeInSeconds);
            fShader->setParameter("reflectivity", reflectivity);
        }

        // Reflected Color
        {
            const MColor reflectedColor =
                ShadedModeColor::evaluateColor(phong.ReflectedColor, fTimeInSeconds);
            const float buffer[3] = {reflectedColor.r, reflectedColor.g, reflectedColor.b};
            fShader->setParameter("reflectedColor", buffer);
        }
    }

    void setupBlinn(const BlinnMaterial& blinn)
    {
        if (!fShader) return;

        // Eccentricity
        {
            const float eccentricity =
				ShadedModeColor::evaluateFloat(blinn.Eccentricity, fTimeInSeconds);
            fShader->setParameter("eccentricity", eccentricity);
        }

        // SpecularRollOff
        {
            const float specularRollOff =
				ShadedModeColor::evaluateFloat(blinn.SpecularRollOff, fTimeInSeconds);
            fShader->setParameter("specularRollOff", specularRollOff);
        }

		// Specular Color
        {
            const MColor specularColor =
                ShadedModeColor::evaluateColor(blinn.SpecularColor, fTimeInSeconds);
            const float buffer[3] = {specularColor.r, specularColor.g, specularColor.b};
            fShader->setParameter("specularColor", buffer);
        }

        // Reflectivity
        {
            const float reflectivity =
                ShadedModeColor::evaluateFloat(blinn.Reflectivity, fTimeInSeconds);
            fShader->setParameter("reflectivity", reflectivity);
        }

        // Reflected Color
        {
            const MColor reflectedColor =
                ShadedModeColor::evaluateColor(blinn.ReflectedColor, fTimeInSeconds);
            const float buffer[3] = {reflectedColor.r, reflectedColor.g, reflectedColor.b};
            fShader->setParameter("reflectedColor", buffer);
        }
    }

    ShaderInstancePtr fShader;
    Deleter           fDeleter;
    const double      fTimeInSeconds;
};


//==============================================================================
// CLASS ShaderInstanceCache
//==============================================================================

// This class manages MShaderInstance across multiple gpuCache nodes.
// The cache returns a shared pointer to the requested MShaderInstance.
// The caller shouldn't modify the MShaderInstance* that is returned from
// getSharedXXXShader() because the shader instance might be shared
// with other render items.
// The caller is responsible to hold the pointer.
// If the reference counter goes 0, the MShaderInstance is released.
class ShaderInstanceCache : boost::noncopyable
{
public:
    static ShaderInstanceCache& getInstance()
    {
        // Singleton
        static ShaderInstanceCache sSingleton;
        return sSingleton;
    }

    ShaderInstancePtr getSharedWireShader(const MColor& color)
    {
        // Look for the cached MShaderInstance.
        ColorAndShaderInstanceCache::nth_index<0>::type::iterator it =
            fWireShaders.get<0>().find(color);

        // Found in cache.
        if (it != fWireShaders.get<0>().end()) {
            boost::shared_ptr<MShaderInstance> shader = it->ptr.lock();
            assert(shader);  // no staled pointer.
            return ShaderInstancePtr(shader, it->source);
        }

        // Not found. Get a new MShaderInstance.
        ShaderInstancePtr shader =
            ShaderCache::getInstance().newWireShader(shaderInstanceDeleter);
        if (shader) {
            // Wireframe dash-line pattern.
           SetDashLinePattern(shader.get(), Config::kLineStippleShortDashed);

            // Wireframe color.
            const float solidColor[4] = {color.r, color.g, color.b, 1.0f};
            shader->setParameter("solidColor", solidColor);

            // Insert into cache.
            ColorAndShaderInstance entry;
            entry.color  = color;
            entry.shader = shader.get();
            entry.ptr    = shader.getShader();
            entry.source = shader.getTemplate();
            fWireShaders.insert(entry);

            return shader;
        }

        assert(0);
        return ShaderInstancePtr();
    }

    ShaderInstancePtr getSharedWireShaderWithCB(const MColor& color)
    {
        // Look for the cached MShaderInstance.
        ColorAndShaderInstanceCache::nth_index<0>::type::iterator it =
            fWireShadersWithCB.get<0>().find(color);

        // Found in cache.
        if (it != fWireShadersWithCB.get<0>().end()) {
            boost::shared_ptr<MShaderInstance> shader = it->ptr.lock();
            assert(shader);  // no staled pointer.
            return ShaderInstancePtr(shader, it->source);
        }

        // Not found. Get a new MShaderInstance.
        ShaderInstancePtr shader =
            ShaderCache::getInstance().newWireShaderWithCB(shaderInstanceDeleter);
        if (shader) {
            // Wireframe dash-line pattern.
            SetDashLinePattern(shader.get(), Config::kLineStippleShortDashed);

            // Wireframe color.
            const float solidColor[4] = {color.r, color.g, color.b, 1.0f};
            shader->setParameter("solidColor", solidColor);

            // Insert into cache.
            ColorAndShaderInstance entry;
            entry.color  = color;
            entry.shader = shader.get();
            entry.ptr    = shader.getShader();
            entry.source = shader.getTemplate();
            fWireShadersWithCB.insert(entry);

            return shader;
        }

        assert(0);
        return ShaderInstancePtr();
    }

    ShaderInstancePtr getSharedBoundingBoxPlaceHolderShader(const MColor& color)
    {
        // Look for the cached MShaderInstance.
        ColorAndShaderInstanceCache::nth_index<0>::type::iterator it =
            fBoundingBoxPlaceHolderShaders.get<0>().find(color);

        // Found in cache.
        if (it != fBoundingBoxPlaceHolderShaders.get<0>().end()) {
            boost::shared_ptr<MShaderInstance> shader = it->ptr.lock();
            assert(shader);  // no staled pointer.
            return ShaderInstancePtr(shader, it->source);
        }

        // Not found. Get a new MShaderInstance.
        ShaderInstancePtr shader =
            ShaderCache::getInstance().newBoundingBoxPlaceHolderShader(shaderInstanceDeleter);
        if (shader) {
            // Wireframe dash-line pattern.
            SetDashLinePattern(shader.get(), Config::kLineStippleShortDashed);

            // Wireframe color.
            const float solidColor[4] = {color.r, color.g, color.b, 1.0f};
            shader->setParameter("solidColor", solidColor);

            // Insert into cache.
            ColorAndShaderInstance entry;
            entry.color  = color;
            entry.shader = shader.get();
            entry.ptr    = shader.getShader();
            entry.source = shader.getTemplate();
            fBoundingBoxPlaceHolderShaders.insert(entry);

            return shader;
        }

        assert(0);
        return ShaderInstancePtr();
    }

    ShaderInstancePtr getSharedDiffuseColorShader(const MColor& color)
    {
        // Look for the cached MShaderInstance.
        ColorAndShaderInstanceCache::nth_index<0>::type::iterator it =
            fDiffuseColorShaders.get<0>().find(color);

        // Found in cache.
        if (it != fDiffuseColorShaders.get<0>().end()) {
            boost::shared_ptr<MShaderInstance> shader = it->ptr.lock();
            assert(shader);  // no staled pointer.
            return ShaderInstancePtr(shader, it->source);
        }

        // Not found. Get a new MShaderInstance.
        ShaderInstancePtr shader =
            ShaderCache::getInstance().newDiffuseColorShader(shaderInstanceDeleter);
        if (shader) {
            // Set the diffuse color.
            setDiffuseColor(shader.get(), color);

            // Insert into cache.
            ColorAndShaderInstance entry;
            entry.color  = color;
            entry.shader = shader.get();
            entry.ptr    = shader.getShader();
            entry.source = shader.getTemplate();
            fDiffuseColorShaders.insert(entry);

            return shader;
        }

        assert(0);
        return ShaderInstancePtr();
    }

    // Create a unique lambert shader for diffuse color.
    // The caller can change the shader parameters for material animation.
    ShaderInstancePtr getUniqueDiffuseColorShader(const MColor& color)
    {
        ShaderInstancePtr shader =
            ShaderCache::getInstance().newDiffuseColorShader(shaderInstanceDeleter);
        if (shader) {
            // Set the diffuse color.
            setDiffuseColor(shader.get(), color);
            return shader;
        }

        return ShaderInstancePtr();
    }

    // This method will get a cached MShaderInstance for the given material.
    ShaderInstancePtr getSharedShadedMaterialShader(
        const MaterialGraph::Ptr& material,
        double                    timeInSeconds
    )
    {
        assert(material);
        if (!material) return ShaderInstancePtr();

        // Look for the cached MShaderInstance.
        MaterialAndShaderInstanceCache::nth_index<0>::type::iterator it =
            fShadedMaterialShaders.get<0>().find(material);

        // Found in cache.
        if (it != fShadedMaterialShaders.get<0>().end()) {
            boost::shared_ptr<MShaderInstance> shader = it->ptr.lock();
            assert(shader);  // no staled pointer.
            return ShaderInstancePtr(shader, it->source);
        }

        // Not found. Get a new MShaderInstance.
        const MaterialNode::Ptr& rootNode = material->rootNode();
        assert(rootNode);

        ShaderInstancePtr shader;
        if (rootNode) {
            MaterialGraphTranslatorShaded shadedTranslator(shaderInstanceDeleter, timeInSeconds);
            rootNode->accept(shadedTranslator);
            shader = shadedTranslator.getShader();
        }

        if (shader) {
            // Insert into cache.
            MaterialAndShaderInstance entry;
            entry.material      = material;
            entry.shader        = shader.get();
            entry.ptr           = shader.getShader();
            entry.source        = shader.getTemplate();
            entry.isAnimated    = material->isAnimated();
            entry.timeInSeconds = timeInSeconds;
            fShadedMaterialShaders.insert(entry);

            return shader;
        }

        assert(0);
        return ShaderInstancePtr();
    }

    void updateCachedShadedShaders(double timeInSeconds)
    {
        // Update all cached MShaderInstance* for shaded mode to the current time.
        BOOST_FOREACH (const MaterialAndShaderInstance& entry, fShadedMaterialShaders) {
            // Not animated. Skipping.
            if (!entry.isAnimated) continue;

            // Already up-to-date. Skipping.
            if (entry.timeInSeconds == timeInSeconds) continue;

            // Update the MShaderInstance*
            const MaterialNode::Ptr& rootNode = entry.material->rootNode();
            if (rootNode) {
                ShaderInstancePtr shader(entry.ptr.lock(), entry.source);
                if (shader) {
                    MaterialGraphTranslatorShaded shadedTranslator(shader, timeInSeconds);
                    rootNode->accept(shadedTranslator);
                }
            }

            // Remember the last update time.
            entry.timeInSeconds = timeInSeconds;
        }
    }

private:
    // Release the MShaderInstance and remove the pointer from the cache.
    static void shaderInstanceDeleter(MShaderInstance* shader)
    {
        assert(shader);
        getInstance().removeShaderInstanceFromCache(shader);
        releaseShaderInstance(shader);
    }

    // Remove the pointer from the cache.
    void removeShaderInstanceFromCache(MShaderInstance* shader)
    {
        assert(shader);
        if (!shader) return;

        // Remove the MShaderInstance* from the cache.
        fWireShaders.get<1>().erase(shader);
        fWireShadersWithCB.get<1>().erase(shader);
        fBoundingBoxPlaceHolderShaders.get<1>().erase(shader);
        fDiffuseColorShaders.get<1>().erase(shader);
        fShadedMaterialShaders.get<1>().erase(shader);
    }

private:
    ShaderInstanceCache()  {}
    ~ShaderInstanceCache() {}

    // MColor as hash key.
    struct MColorHash : std::unary_function<MColor, std::size_t>
    {
        std::size_t operator()(const MColor& key) const
        {
            std::size_t seed = 0;
            boost::hash_combine(seed, key.r);
            boost::hash_combine(seed, key.g);
            boost::hash_combine(seed, key.b);
            boost::hash_combine(seed, key.a);
            return seed;
        }
    };

    // MaterialGraph as hash key.
    struct MaterialGraphHash : std::unary_function<MaterialGraph::Ptr, std::size_t>
    {
        std::size_t operator()(const MaterialGraph::Ptr& key) const
        {
            return boost::hash_value(key.get());
        }
    };

    // MShaderInstance* cached by MColor as hash key.
    struct ColorAndShaderInstance {
        MColor                             color;
        MShaderInstance*                   shader;
        boost::weak_ptr<MShaderInstance>   ptr;
        boost::shared_ptr<MShaderInstance> source;
    };
    typedef boost::multi_index_container<
        ColorAndShaderInstance,
        boost::multi_index::indexed_by<
            boost::multi_index::hashed_unique<
                BOOST_MULTI_INDEX_MEMBER(ColorAndShaderInstance,MColor,color),
                MColorHash
            >,
            boost::multi_index::hashed_unique<
                BOOST_MULTI_INDEX_MEMBER(ColorAndShaderInstance,MShaderInstance*,shader)
            >
        >
    > ColorAndShaderInstanceCache;

    // MShaderInstance* cached by MaterialGraph as hash key.
    struct MaterialAndShaderInstance {
        MaterialGraph::Ptr                 material;
        MShaderInstance*                   shader;
        boost::weak_ptr<MShaderInstance>   ptr;
        boost::shared_ptr<MShaderInstance> source;
        bool                               isAnimated;
        mutable double                     timeInSeconds;
    };
    typedef boost::multi_index_container<
        MaterialAndShaderInstance,
        boost::multi_index::indexed_by<
            boost::multi_index::hashed_unique<
                BOOST_MULTI_INDEX_MEMBER(MaterialAndShaderInstance,MaterialGraph::Ptr,material),
                MaterialGraphHash
            >,
            boost::multi_index::hashed_unique<
                BOOST_MULTI_INDEX_MEMBER(MaterialAndShaderInstance,MShaderInstance*,shader)
            >
        >
    > MaterialAndShaderInstanceCache;

    ColorAndShaderInstanceCache    fWireShaders;
    ColorAndShaderInstanceCache    fWireShadersWithCB;
    ColorAndShaderInstanceCache    fBoundingBoxPlaceHolderShaders;
    ColorAndShaderInstanceCache    fDiffuseColorShaders;
    MaterialAndShaderInstanceCache fShadedMaterialShaders;
};


//==============================================================================
// CLASS HardwareInstanceData
//==============================================================================

class RenderItemWrapper;
class HardwareInstanceManagerImpl;

// This class contains the hardware instancing information for a render item.
// Each RenderItemWrapper object has the ownership of this object.
// If a RenderItemWrapper holds an instance of this class, the render item is
// already instanced or is an instance candidate (not-yet-instanced).
class HardwareInstanceData : boost::noncopyable
{
public:
    HardwareInstanceData(HardwareInstanceManagerImpl* manager,
                         RenderItemWrapper*           renderItem)
        : fMasterData(NULL),
          fInstanceId(0),
          fRenderItem(renderItem),
          fManager(manager)
    {}

    ~HardwareInstanceData()
    {}

    // Returns the master render item.
    HardwareInstanceData* masterData() const { return fMasterData; }

    // Returns the instance id.
    unsigned int instanceId() const { return fInstanceId; }
    
    // Returns the owner render item.
    RenderItemWrapper* renderItem() const { return fRenderItem; }

    // Returns true if this render item is hardware instanced.
    bool isInstanced() const  { return fInstanceId > 0; }

    // Returns true if this render item is a mater instance item.
    bool isMasterItem() const { return fMasterData == this; }

    // Set up to be an instance candidate.
    void setupCandidate(HardwareInstanceData* master)
    {
        assert(master);
        fMasterData = master;
        fInstanceId = 0;
    }

    // Set up to be an instance.
    void setupInstance(HardwareInstanceData* master, unsigned int instanceId)
    {
        assert(master);
        assert(instanceId > 0);
        fMasterData = master;
        fInstanceId = instanceId;
    }

    // Clear the instance data.
    void clearInstanceData()
    {
        fMasterData = NULL;
        fInstanceId = 0;
    }

    // Notify that the render item has been changed so its instancing
    // should be recomputed.
    void notifyInstancingChange();

    // Notify that the render item has been changed but the change is
    // destructive (shader or geometry change).
    void notifyInstancingClear();

    // Notify that the render item's world matrix has been changed.
    void notifyWorldMatrixChange();

    // Notify that the render item is going to be destroyed.
    void notifyDestroy();

private:
    HardwareInstanceData* fMasterData;
    unsigned int          fInstanceId;

    RenderItemWrapper*           fRenderItem;
    HardwareInstanceManagerImpl* fManager;
};


//==============================================================================
// CLASS RenderItemWrapper
//==============================================================================

// This class wraps a MRenderItem* object. This will make us easier to track
// the state of a render item.
class RenderItemWrapper : boost::noncopyable
{
public:
    typedef boost::shared_ptr<RenderItemWrapper> Ptr;

    RenderItemWrapper(const MString&                     name,
                      const MRenderItem::RenderItemType  type,
                      const MGeometry::Primitive         primitive)
        : fName(name),
          fType(type),
          fPrimitive(primitive),
          fRenderItem(NULL),
          fEnabled(true),
          fDrawMode((MGeometry::DrawMode)0),
          fDepthPriority(MRenderItem::sDormantFilledDepthPriority),
          fExcludedFromPostEffects(true),
          fCastsShadows(false),
          fReceivesShadows(false)
    {
        assert(name.length() > 0);

        // Create the render item.
        fRenderItem = MRenderItem::Create(
            name,
            type,
            primitive
        );
        assert(fRenderItem);
    }

    ~RenderItemWrapper()
    {
        // The buffers are no longer used by this render item.
        BuffersCache::getInstance().removeBuffers(
            fIndices,
            fPositions,
            fNormals,
            fUVs
        );

        // Notify that the render item is destroyed or already destroyed.
        notifyDestroy();
    }

    void addToContainer(MSubSceneContainer& container)
    {
        assert(fRenderItem);
        container.add(fRenderItem);
    }

    void removeFromContainer(MSubSceneContainer& container)
    {
        if (fRenderItem) {
            assert(fName == fRenderItem->name());
            fRenderItem->setCustomData(NULL);
            container.remove(fName);
            fRenderItem = NULL;
        }
    }

    void setEnabled(bool enabled)
    {
        if (fEnabled != enabled) {
            // Enable/disable the render item.
            if (fRenderItem) {
                fRenderItem->enable(enabled);
            }

            // Cache the enable flag.
            fEnabled = enabled;

            // Visibility changed. We need to recompute shadow map.
            if (fType == MRenderItem::MaterialSceneItem) {
                MRenderer::setLightsAndShadowsDirty();
            }

            notifyInstancingChange();
        }
    }

    void setWorldMatrix(const MMatrix& worldMatrix)
    {
        if (fWorldMatrix != worldMatrix) {
            // Set the world matrix to the render item.
            if (fRenderItem) {
                fRenderItem->setMatrix(&worldMatrix);
            }

            // Cache the world matrix.
            fWorldMatrix = worldMatrix;

            // World matrix changed. We need to recompute shadow map.
            if (fType == MRenderItem::MaterialSceneItem) {
                MRenderer::setLightsAndShadowsDirty();
            }

            notifyWorldMatrixChange();
        }
    }

    void setBuffers(SubSceneOverride&                               subSceneOverride,
                    const boost::shared_ptr<const IndexBuffer>&     indices,
                    const boost::shared_ptr<const VertexBuffer>&    positions,
                    const boost::shared_ptr<const VertexBuffer>&    normals,
                    const boost::shared_ptr<const VertexBuffer>&    uvs,
                    const MBoundingBox&                             boundingBox)
    {
        const bool buffersChanged =
            fIndices    !=  indices     ||
            fPositions  !=  positions   ||
            fNormals    !=  normals     ||
            fUVs        !=  uvs;

        if (buffersChanged) {
            // Update the geometry on the render item.
            BuffersCache::getInstance().updateBuffers(
                subSceneOverride,
                fRenderItem,
                indices,
                positions,
                normals,
                uvs,
                boundingBox,
                fIndices,
                fPositions,
                fNormals,
                fUVs
            );

            // Cache the buffers.
            fIndices        =   indices;
            fPositions      =   positions;
            fNormals        =   normals;
            fUVs            =   uvs;
            fBoundingBox    =   boundingBox;

            // World matrix changed. We need to recompute shadow map.
            if (fType == MRenderItem::MaterialSceneItem) {
                MRenderer::setLightsAndShadowsDirty();
            }

            // Setting the geometry is destructive.
            // Viewport 2.0 will override the geometry for hardware instancing.
            notifyInstancingClear();
        }
    }

    void setShader(const ShaderInstancePtr& shader)
    {
        if (fShader != shader) {
            // Set the new shader to the render item.
            if (fRenderItem) {
                fRenderItem->setShader(shader.get());
            }

            // Cache the shader pointer.
            fShader = shader;

            // Setting the shader is destructive.
            // Viewport 2.0 will override the shader for hardware instancing.
            notifyInstancingClear();
        }
    }

    void setCustomData(const boost::shared_ptr<MUserData>& userData)
    {
        if (fUserData != userData) {
            if (fRenderItem) {
                fRenderItem->setCustomData(userData.get());
            }
            fUserData = userData;

            notifyInstancingChange();
        }
    }

    void setDrawMode(MGeometry::DrawMode drawMode)
    {
        if (fDrawMode != drawMode) {
            if (fRenderItem) {
                fRenderItem->setDrawMode(drawMode);
            }
            fDrawMode = drawMode;

            notifyInstancingChange();
        }
    }

    void setDepthPriority(unsigned int depthPriority)
    {
        if (fDepthPriority != depthPriority) {
            if (fRenderItem) {
                fRenderItem->depthPriority(depthPriority);
            }
            fDepthPriority = depthPriority;

            notifyInstancingChange();
        }
    }

    void setExcludedFromPostEffects(bool exclude)
    {
        if (fExcludedFromPostEffects != exclude) {
            if (fRenderItem) {
                fRenderItem->setExcludedFromPostEffects(exclude);
            }
            fExcludedFromPostEffects = exclude;

            notifyInstancingChange();
        }
    }

    void setCastsShadows(bool cast)
    {
        if (fCastsShadows != cast) {
            // Set whether the render item will cast shadows on other objects.
            if (fRenderItem) {
                fRenderItem->castsShadows(cast);
            }

            // Cache the cast shadow flag.
            fCastsShadows = cast;

            // Recompute shadow map if cast shadow flag changes
            if (fType == MRenderItem::MaterialSceneItem) {
                MRenderer::setLightsAndShadowsDirty();
            }

            notifyInstancingChange();
        }
    }

    void setReceivesShadows(bool receive)
    {
        if (fReceivesShadows != receive) {
            // Set whether the render item will receive shadows from other objects.
            if (fRenderItem) {
                fRenderItem->receivesShadows(receive);
            }

            // Cache the receive shadow flag.
            fReceivesShadows = receive;

            notifyInstancingChange();
        }
    }

    // Set up for hardware instancing. 
    // If the hardware instance data is NULL, the render item will never be instanced.
    // This method must be called from HardwareInstanceManager.
    void installHardwareInstanceData(const boost::shared_ptr<HardwareInstanceData>& data)
    {
        fHardwareInstanceData = data;
        notifyInstancingChange();
    }

    // Remove hardware instancing data. This render item will never be instanced.
    // This method must be called from HardwareInstanceManager.
    void removeHardwareInstanceData(SubSceneOverride&   subSceneOverride,
                                    MSubSceneContainer& container)
    {
        if (fHardwareInstanceData) {
            if (fHardwareInstanceData->isInstanced()) {
                // Get rid of the render item that is set up hardware instancing.
                if (fRenderItem) {
                    unloadItem(container);
                }

                assert(!fRenderItem);
                loadItem(subSceneOverride, container);
            }

            // Delete the hardware instancing data.
            fHardwareInstanceData.reset();
        }
    }

    // Returns true if the render item is already instaned or not-yet-instanced.
    bool hasHardwareInstanceData()
    {
        return fHardwareInstanceData.get() != NULL;
    }

    // Unload the render item. This will delete the actual MRenderItem.
    void unloadItem(MSubSceneContainer& container)
    {
        // Already unloaded?
        if (!fRenderItem) return;

        // Remove the render item from the container. The container claims
        // a strong ownership so the render item is actually deleted.
        removeFromContainer(container);
        fRenderItem = NULL;
    }

    // Load the render item. This will create a new identical MRenderItem.
    void loadItem(SubSceneOverride& subSceneOverride,
                  MSubSceneContainer& container)
    {
        // Already loaded?
        if (fRenderItem) return;

        // Create the render item.
        fRenderItem = MRenderItem::Create(
            fName,
            fType,
            fPrimitive
        );
        assert(fRenderItem);

        // Add back to container.
        addToContainer(container);

        // Restore parameters.
        fRenderItem->setCustomData(fUserData.get());
        fRenderItem->enable(fEnabled);
        fRenderItem->setMatrix(&fWorldMatrix);
        fRenderItem->setDrawMode(fDrawMode);
        fRenderItem->depthPriority(fDepthPriority);
        fRenderItem->setExcludedFromPostEffects(fExcludedFromPostEffects);
        fRenderItem->castsShadows(fCastsShadows);
        fRenderItem->receivesShadows(fReceivesShadows);
        fRenderItem->setShader(fShader.get());

        // Restore buffers.
        BuffersCache::getInstance().updateBuffers(
            subSceneOverride,
            fRenderItem,
            fIndices,
            fPositions,
            fNormals,
            fUVs,
            fBoundingBox,
            fIndices,
            fPositions,
            fNormals,
            fUVs
        );
    }

    // Query methods
    //
    const MString&                      name() const      { return fName; }
    const MRenderItem::RenderItemType   type() const      { return fType; }
    const MGeometry::Primitive          primitive() const { return fPrimitive; }
    const boost::shared_ptr<MUserData>& userData() const  { return fUserData; }

    const boost::shared_ptr<const IndexBuffer>&  indices() const    { return fIndices; }
    const boost::shared_ptr<const VertexBuffer>& positions() const  { return fPositions; }
    const boost::shared_ptr<const VertexBuffer>& normals() const    { return fNormals; }
    const boost::shared_ptr<const VertexBuffer>& uvs() const        { return fUVs; }
    const MBoundingBox& boundingBox() const { return fBoundingBox; }

    bool                enabled() const                 { return fEnabled; }
    const MMatrix&      worldMatrix() const             { return fWorldMatrix; }
    MGeometry::DrawMode drawMode() const                { return fDrawMode; }
    unsigned int        depthPriority() const           { return fDepthPriority; }
    bool                excludedFromPostEffects() const { return fExcludedFromPostEffects; }
    bool                castsShadows() const            { return fCastsShadows; }
    bool                receivesShadows() const         { return fReceivesShadows; }

    const ShaderInstancePtr& shader() const { return fShader; }

    // Extract the wrapped render item.
    MRenderItem* wrappedItem() const { return fRenderItem; }

private:
    // Hardware instancing notification methods.
    //
    // Slight change that we can reuse existing instancing.
    void notifyInstancingChange()
    {
        if (fHardwareInstanceData) {
            fHardwareInstanceData->notifyInstancingChange();
        }
    }

    // Destructive change that we have to clear instancing.
    void notifyInstancingClear()
    {
        if (fHardwareInstanceData) {
            fHardwareInstanceData->notifyInstancingClear();
        }
    }

    // World matrix change. We need to update instance transform.
    void notifyWorldMatrixChange()
    {
        if (fHardwareInstanceData) {
            fHardwareInstanceData->notifyWorldMatrixChange();
        }
    }

    // Destructor is being called.
    void notifyDestroy()
    {
        if (fHardwareInstanceData) {
            fHardwareInstanceData->notifyDestroy();
        }
    }

private:
    const MString                           fName;
    const MRenderItem::RenderItemType       fType;
    const MGeometry::Primitive              fPrimitive;
    boost::shared_ptr<MUserData>            fUserData;

    MRenderItem*                            fRenderItem;

    boost::shared_ptr<const IndexBuffer>    fIndices;
    boost::shared_ptr<const VertexBuffer>   fPositions;
    boost::shared_ptr<const VertexBuffer>   fNormals;
    boost::shared_ptr<const VertexBuffer>   fUVs;
    MBoundingBox                            fBoundingBox;

    bool                                    fEnabled;
    MMatrix                                 fWorldMatrix;
    MGeometry::DrawMode                     fDrawMode;
    unsigned int                            fDepthPriority;
    bool                                    fExcludedFromPostEffects;
    bool                                    fCastsShadows;
    bool                                    fReceivesShadows;

    ShaderInstancePtr                       fShader;

    boost::shared_ptr<HardwareInstanceData> fHardwareInstanceData;
};


//==============================================================================
// CLASS HardwareInstanceManagerImpl
//==============================================================================

// This class is expected to manage all hardware instances inside a single
// subscene. Currently, we don't support hardware instances between gpuCache nodes.
// Each SubSceneOverride owns a HardwareInstanceManager.
// The manager tracks the render item changes. At the end of update() method,
// processInstances() is called to set up instances.
// There are 3 places that hold instancing information:
//   1) HardwareInstanceManagerImpl: This class holds all instancing info.
//   2) HardwareInstanceData: This class is attached to each render item to
//                            keep track of the per-renderItem info.
//   3) MRenderItem: An instance render item is set up by calling MPxSubSceneOverride
//                   hardware instancing methods.
class HardwareInstanceManagerImpl : boost::noncopyable
{
public:
    HardwareInstanceManagerImpl(SubSceneOverride& subSceneOverride)
        : fSubSceneOverride(subSceneOverride)
    {}

    ~HardwareInstanceManagerImpl()
    {}

    // This method is called at the end of the subscene's update() method.
    // We have collected all changed/destroyed render items.
    // In this method, we can choose the render items to form hardware
    // instances. Or remove a render item from an existing instance.
    void processInstances(MSubSceneContainer& container)
    {
        // Clean up removed instances.
        removePendingInstances(container);

        // Update all instance world matrices.
        updateInstanceTransforms();

        // Extract dirty source render items.
        boost::unordered_set<HardwareInstanceData*> dirtySourceItems;
        extractDirtySourceItems(container, dirtySourceItems);

        // Process all dirty source render items.
        boost::unordered_set<HardwareInstanceData*> dirtyCandidates;
        processDirtySourceItems(container, dirtySourceItems, dirtyCandidates);

        // Process all dirty candidates.
        processCandidates(container, dirtyCandidates);

        // Load the render items back if they are still not instances.
        loadPendingItems(container);
    }

    // This method is called at the beginning of the subscene's update method().
    // In this method, we delete everything related to hardware instancing.
    void resetInstances(MSubSceneContainer& container)
    {
        // This method must be called before the update() method.
        // So there are no dirty render items.
        assert(fInstancingChangeItems.empty());
        assert(fWorldMatrixChangeItems.empty());
        assert(fItemsPendingLoad.empty());
        assert(fItemsPendingRemove.empty());

        // Collect all render items.
        boost::unordered_set<RenderItemWrapper*> renderItems;

        BOOST_FOREACH (const HardwareInstance& hwInstance, fInstances) {
            renderItems.insert(hwInstance.master->renderItem());
            BOOST_FOREACH (HardwareInstanceData* data, hwInstance.sources) {
                renderItems.insert(data->renderItem());
            }
        }

        // Throw away all the instancing information.
        fInstancingChangeItems.clear();
        fWorldMatrixChangeItems.clear();
        fItemsPendingLoad.clear();
        fItemsPendingRemove.clear();
        fInstances.clear();

        // Delete the attached hardware instance data on the render item.
        // If the render item is already instanced, it will be re-created to
        // get rid of the instancing.
        BOOST_FOREACH (RenderItemWrapper* renderItem, renderItems) {
            renderItem->removeHardwareInstanceData(fSubSceneOverride, container);
        }
    }

    // Callback that a render item has been changed. We need to look at
    // the render item in processInstances() method later.
    void notifyInstancingChange(HardwareInstanceData* data)
    {
        assert(data && data->renderItem());
        fInstancingChangeItems.insert(data);
    }

    // Callback that a render item's world matrix has been changed.
    // We need to update the instance transform in the master render item.
    void notifyWorldMatrixChange(HardwareInstanceData* data)
    {
        assert(data && data->renderItem() && data->isInstanced());
        fWorldMatrixChangeItems.insert(data);
    }

    // Callback that a render item has been changed but the change is
    // destructive. The render item should no longer be an instance.
    // e.g. shader change and/or geometry change are destructive.
    void notifyInstancingClear(HardwareInstanceData* data, bool destroy = false)
    {
        assert(data && data->renderItem());

        // Dirty the render item so it will get processed again later.
        fInstancingChangeItems.insert(data);
        fWorldMatrixChangeItems.erase(data);

        // All instanced source render items are pending reloading because
        // the master render item has gone.
        // But we don't reload them immediately for performance.
        if (data->isInstanced()) {
            fItemsPendingLoad.insert(data);
        }

        // Update hardware instance set.
        if (data->isMasterItem()) {
            HardwareInstanceSet::iterator it = fInstances.find(data);
            assert(it != fInstances.end() && it->master == data);

            // This is a master render item. We dismiss this hardware instance or
            // instance candidate.
            BOOST_FOREACH (HardwareInstanceData* sourceData, it->sources) {
                // Dirty the source item so it will get processed again later.
                fInstancingChangeItems.insert(sourceData);
                fWorldMatrixChangeItems.erase(sourceData);

                // All instanced source render items are pending reloading because
                // the master render item has gone.
                // But we don't reload them immediately for performance.
                if (sourceData->isInstanced()) {
                    fItemsPendingLoad.insert(sourceData);
                }

                // The source render item is no longer instanced.
                sourceData->clearInstanceData();
            }

            // Clear the master render item's instancing.
            if (data->isInstanced()) {
                // If the render item is going to be destroyed, we don't need
                // to call removeAllInstances() ..
                if (!destroy) {
                    // Master render item is gone. No survivals..
                    MStatus stat = fSubSceneOverride.removeAllInstances(
                        *data->renderItem()->wrappedItem()
                    );
                    MStatAssert(stat);
                }

                // Schedule for reloading the master render item to
                // totally get rid of the instancing set up.
                fItemsPendingRemove.insert(data);
            }

            fInstances.erase(it);
            data->clearInstanceData();
        }
        else {
            HardwareInstanceData* master = data->masterData();
            if (master) {
                // This is a source render item. Find the master render item first.
                HardwareInstanceSet::iterator it = fInstances.find(master);
                assert(it != fInstances.end());

                // Remove this source render item from the set.
                assert(it->sources.find(data) != it->sources.end());
                it->sources.erase(data);

                // Remove the instance from the master render item.
                if (!it->candidate) {
                    assert(master->isInstanced() && data->isInstanced());
                    MStatus stat = fSubSceneOverride.removeInstance(
                        *master->renderItem()->wrappedItem(),
                        data->instanceId()
                    );
                    MStatAssert(stat);
                }

                // The source render item is no longer instanced.
                data->clearInstanceData();
            }
        }
    }

    // Callback that a render item is going to be destroyed.
    // This is similar to a destructive change but we will remove the
    // render item permanently.
    void notifyDestroy(HardwareInstanceData* data)
    {
        assert(data && data->renderItem());

        // Same as a destructive change.
        notifyInstancingClear(data, true /* destroy */ );
        
        // The render item is going to be destroyed. We don't want to
        // deal with it any more.
        fInstancingChangeItems.erase(data);
        fWorldMatrixChangeItems.erase(data);
        fItemsPendingLoad.erase(data);
        fItemsPendingRemove.erase(data);
    }

    void dump() const
    {
        using namespace std;
        ostringstream tmp;

        size_t hwInstCounter = 0;
        BOOST_FOREACH (const HardwareInstance& hwInstance, fInstances) {
            tmp << "HW Instance #" << (hwInstCounter++) << endl;

            tmp << '\t' << "Master: " 
                << hwInstance.master->renderItem()->name().asChar()
                << endl;

            tmp << '\t' << "Candidate: "
                << (hwInstance.candidate ? "true" : "false")
                << endl;

            tmp << '\t' << "Sources: "
                << hwInstance.sources.size()
                << endl;

            size_t sourceCounter = 0;
            BOOST_FOREACH (HardwareInstanceData* sourceData, hwInstance.sources) {
                tmp << '\t' << '\t' << "Source #" << (sourceCounter++)
                    << sourceData->renderItem()->name().asChar()
                    << endl;
            }
        }

        printf("%s\n", tmp.str().c_str());
    }

private:
    // Some render items are destroyed or have destructive changes.
    // This is the final step to update the underlying MRenderItem.
    void removePendingInstances(MSubSceneContainer& container)
    {
        BOOST_FOREACH (HardwareInstanceData* data, fItemsPendingRemove) {
            data->renderItem()->unloadItem(container);
        }
        fItemsPendingRemove.clear();
    }

    // Some render items are no longer instances. We need to re-create
    // the underlying MRenderItem. But we do this lazily because the render
    // item might become instance again.
    void loadPendingItems(MSubSceneContainer& container)
    {
        BOOST_FOREACH (HardwareInstanceData* data, fItemsPendingLoad) {
            assert(data);
            data->renderItem()->loadItem(fSubSceneOverride, container);
        }
        fItemsPendingLoad.clear();
    }

    // Some render items' world matrices have been changed. We need to
    // update the corresponding instance matrix in the master render item.
    void updateInstanceTransforms()
    {
        BOOST_FOREACH (HardwareInstanceData* data, fWorldMatrixChangeItems) {
            assert(data && data->isInstanced());
            if (!data || !data->isInstanced()) continue;

            // Note that if this is a master item, master equals to data.
            HardwareInstanceData* master = data->masterData();
            assert(master && master->isInstanced());

            // Render items.
            RenderItemWrapper* masterItem = master->renderItem();
            assert(masterItem);
            RenderItemWrapper* thisItem = data->renderItem();
            assert(thisItem);

            // Set instance transform.
            MStatus stat = fSubSceneOverride.updateInstanceTransform(
                *masterItem->wrappedItem(),
                data->instanceId(),
                thisItem->worldMatrix()
            );
            MStatAssert(stat);
        }
        fWorldMatrixChangeItems.clear();
    }

    // This method will find all dirty source render items based on the current
    // dirty render items (master + source). If a master render item is dirty,
    // we consider all its source render items are dirty as well.
    void extractDirtySourceItems(
        MSubSceneContainer&                          container,
        boost::unordered_set<HardwareInstanceData*>& dirtySourceItems)
    {
        BOOST_FOREACH (HardwareInstanceData* data, fInstancingChangeItems) {
            assert(data);

            // This is a source item. Skip it.
            if (!data->isMasterItem()) {
                assert(fInstances.find(data) == fInstances.end());
                dirtySourceItems.insert(data);
                continue;
            }

            // We only deal with master render items.
            HardwareInstanceSet::iterator it = fInstances.find(data);
            assert(it != fInstances.end());
            if (it == fInstances.end()) continue;
            assert(it->master == data);
            if (it->master != data) continue;

            // Search the source items. If the source item is different from
            // the changed master item, mark it as dirty.
            BOOST_FOREACH (HardwareInstanceData* sourceData, it->sources) {
                assert(sourceData);
                dirtySourceItems.insert(sourceData);
            }

            // Re-hash the master render item since it's changed.
            HardwareInstance hwInstance = *it;
            fInstances.erase(it);
            if (fInstances.get<1>().find(hwInstance.master) == fInstances.get<1>().end()) {
                // Insert back..
                fInstances.insert(hwInstance);
            }
            else {
                // We already have a hardware instance that has the same look..
                // Dismiss this instance.
                BOOST_FOREACH (HardwareInstanceData* sourceData, hwInstance.sources) {
                    dirtySourceItems.insert(sourceData);
                    if (sourceData->isInstanced()) {
                        sourceData->renderItem()->unloadItem(container);
                        fItemsPendingLoad.insert(sourceData);
                    }
                    sourceData->clearInstanceData();
                }
                dirtySourceItems.insert(data);
                if (data->isInstanced()) {
                    data->renderItem()->unloadItem(container);
                    fItemsPendingLoad.insert(data);
                }
                data->clearInstanceData();
            }
        }
        fInstancingChangeItems.clear();
    }

    // This method goes through all dirty source render items and put them
    // to the correct instance group.
    void processDirtySourceItems(
        MSubSceneContainer&                                container,
        const boost::unordered_set<HardwareInstanceData*>& dirtySourceItems,
        boost::unordered_set<HardwareInstanceData*>&       dirtyCandidates)
    {
        // Process all dirty source items.
        BOOST_FOREACH (HardwareInstanceData* data, dirtySourceItems) {
            assert(data && !data->isMasterItem());
            assert(fInstances.find(data) == fInstances.end());

            // Remove the dirty item since its hash (look) is changed.
            HardwareInstanceData* master = data->masterData();
            if (master) {
                // Remove the source item from its master's source set.
                HardwareInstanceSet::iterator it = fInstances.find(master);
                assert(it != fInstances.end());
                if (it != fInstances.end()) {
                    assert(it->sources.find(data) != it->sources.end());
                    it->sources.erase(data);
                }
            }

            // Process this dirty render item.
            bool skipThisItem = false;
            HardwareInstanceSet::nth_index<1>::type::iterator it = 
                fInstances.get<1>().find(data);

            if (data->isInstanced()) {
                // This render item is already instanced.
                if (it != fInstances.get<1>().end()) {
                    if (data->masterData() == it->master) {
                        // Both the render item and its master item are changed,
                        // but they finally have the same look again...
                        // Simply add the render item back and skip.
                        it->sources.insert(data);
                        skipThisItem = true;
                    }
                    else {
                        // The instanced render item is changed. We need to
                        // remove it from its master.
                        leaveInstance(data, container);
                    }
                }
                else {
                    // The instanced render item is changed. We need to
                    // remove it from its master.
                    leaveInstance(data, container);
                }
            }

            if (!skipThisItem) {
                assert(!data->isInstanced());
                if (it != fInstances.get<1>().end()) {
                    if (it->candidate) {
                        // There is a candidate hardware instance. Join
                        // the candidate.
                        joinCandidate(it->master, data);

                        // We will review the candidate later.
                        dirtyCandidates.insert(it->master);
                    }
                    else {
                        // There is already a master render item that has the
                        // same look.
                        joinInstance(it->master, data, container);
                    }
                }
                else {
                    // There are no instances or candidates that have the
                    // same look. We create a new candidate.
                    newCandidate(data);
                }
            }
        }
    }

    // This method goes through all instance candidates and make them instances if
    // the number of source render items meets the threshold requirement.
    void processCandidates(
        MSubSceneContainer&                                container,
        const boost::unordered_set<HardwareInstanceData*>& dirtyCandidates)
    {
        BOOST_FOREACH (HardwareInstanceData* data, dirtyCandidates) {
            assert(data);

            HardwareInstanceSet::iterator it = fInstances.find(data);
            assert(it != fInstances.end());
            if (it == fInstances.end()) continue;
            assert(it->candidate && data == it->master);
            if (!it->candidate || data != it->master) continue;

            // If the number of master and source items in the candidate does
            // not meet the threshold requirement, skip this candidate.
            if (it->sources.size() + 1 < Config::hardwareInstancingThreshold()) {
                continue;
            }

            // Remove the candidate.
            HardwareInstance hwInstance = *it;
            fInstances.erase(it);

            // Create a new hardware instance.
            assert(hwInstance.master && !hwInstance.master->isInstanced());
            hwInstance.master->clearInstanceData();
            newInstance(hwInstance.master, container);

            // Join the remaining instances.
            BOOST_FOREACH (HardwareInstanceData* data, hwInstance.sources) {
                assert(data && !data->isInstanced() && !data->isMasterItem());
                data->clearInstanceData();
                joinInstance(hwInstance.master, data, container);
            }
        }
    }

    void newCandidate(HardwareInstanceData* source)
    {
        // source must be a not-yet-instanced item.
        assert(source);
        assert(!source->isInstanced() && !source->isMasterItem());
        if (source->isInstanced() || source->isMasterItem()) return;

        // The master of the candidate is the source.
        source->setupCandidate(source);

        // Create a new candidate.
        HardwareInstance hwInstance;
        hwInstance.master    = source;
        hwInstance.candidate = true;
        fInstances.insert(hwInstance);
    }

    void joinCandidate(HardwareInstanceData* master,
                       HardwareInstanceData* source)
    {
        // master must be an not instanced master render item.
        assert(master);
        assert(!master->isInstanced() && master->isMasterItem());
        if (master->isInstanced() || !master->isMasterItem()) return;

        // We should know the master render item.
        HardwareInstanceSet::iterator it = fInstances.find(master);
        assert(it != fInstances.end());
        if (it == fInstances.end()) return;
        assert(it->master == master && it->candidate);
        if (it->master != master || !it->candidate) return;
        assert(it->sources.find(source) == it->sources.end());

        // source must be a not-yet-instanced item.
        assert(source);
        assert(!source->isInstanced() && !source->isMasterItem());
        if (source->isInstanced() || source->isMasterItem()) return;

        // Set up the remaining data.
        source->setupCandidate(master);
        it->sources.insert(source);
    }

    void newInstance(HardwareInstanceData* source,
                     MSubSceneContainer&   container)
    {
        // source must be a not-yet-instanced item.
        assert(source);
        assert(!source->isInstanced() && !source->isMasterItem());
        if (source->isInstanced() || source->isMasterItem()) return;

        // Render items.
        RenderItemWrapper* sourceItem = source->renderItem();
        assert(sourceItem);

        // Make sure the master render item is loaded.
        sourceItem->loadItem(fSubSceneOverride, container);
        fItemsPendingLoad.erase(source);

        // Make the source render item as a master item.
        unsigned int instanceId = fSubSceneOverride.addInstanceTransform(
            *sourceItem->wrappedItem(),
            sourceItem->worldMatrix()
        );
        assert(instanceId > 0);
        if (instanceId == 0) return;    // failure?

        // The master of the candidate is the source.
        source->setupInstance(source, instanceId);

        // Create a new instance.
        HardwareInstance hwInstance;
        hwInstance.master    = source;
        hwInstance.candidate = false;
        fInstances.insert(hwInstance);
    }

    void joinInstance(HardwareInstanceData* master, 
                      HardwareInstanceData* source,
                      MSubSceneContainer&   container)
    {
        // master must be an instanced master render item.
        assert(master);
        assert(master->isInstanced() && master->isMasterItem());
        if (!master->isInstanced() || !master->isMasterItem()) return;

        // We should know the master render item.
        HardwareInstanceSet::iterator it = fInstances.find(master);
        assert(it != fInstances.end());
        if (it == fInstances.end()) return;
        assert(it->master == master && !it->candidate);
        if (it->master != master || it->candidate) return;
        assert(it->sources.find(source) == it->sources.end());

        // source must be a not-yet-instanced item.
        assert(source);
        assert(!source->isInstanced() && !source->isMasterItem());
        if (source->isInstanced() || source->isMasterItem()) return;

        // Render items.
        RenderItemWrapper* masterItem = master->renderItem();
        assert(masterItem);
        RenderItemWrapper* sourceItem = source->renderItem();
        assert(sourceItem);
        
        // Add a new hardware instance to the master render item.
        unsigned int instanceId = fSubSceneOverride.addInstanceTransform(
            *masterItem->wrappedItem(),
            sourceItem->worldMatrix()
        );
        assert(instanceId > 0);
        if (instanceId == 0) return;    // failure?

        // Delete the source render item. 
        sourceItem->unloadItem(container);
        fItemsPendingLoad.erase(source);

        // Set up the remaining data.
        source->setupInstance(master, instanceId);
        it->sources.insert(source);
    }

    void leaveInstance(HardwareInstanceData* source,
                       MSubSceneContainer&   container)
    {
        // source must be an instanced item but not a master.
        assert(source);
        assert(source->isInstanced() && !source->isMasterItem());
        if (!source->isInstanced() || source->isMasterItem()) return;

        HardwareInstanceData* master = source->masterData();
        assert(master);

        // Render items.
        RenderItemWrapper* masterItem = master->renderItem();
        assert(masterItem);
        RenderItemWrapper* sourceItem = source->renderItem();
        assert(sourceItem);

        // Remove the hardware instance from the master render item.
        MStatus stat = fSubSceneOverride.removeInstance(
            *masterItem->wrappedItem(),
            source->instanceId()
        );
        assert(stat == MS::kSuccess);
        if (!stat) return;  // failure?

        // Reload the source render item.
        sourceItem->loadItem(fSubSceneOverride, container);

        // Set up the remaining data.
        source->clearInstanceData();
    }

    // This function object returns the same hash code for
    // render items that have identical look.
    // We ignore the render item's name and its world matrix.
    struct VisHash : std::unary_function<HardwareInstanceData*, std::size_t>
    {
        std::size_t operator()(const HardwareInstanceData* const& data) const
        {
            std::size_t seed = 0;
            const RenderItemWrapper* w = data->renderItem();
            assert(w);
            boost::hash_combine(seed, w->type());
            boost::hash_combine(seed, w->primitive());
            boost::hash_combine(seed, w->userData().get());
            boost::hash_combine(seed, w->indices().get());
            boost::hash_combine(seed, w->positions().get());
            boost::hash_combine(seed, w->normals().get());
            boost::hash_combine(seed, w->uvs().get());
            boost::hash_combine(seed, w->enabled());
            boost::hash_combine(seed, w->drawMode());
            boost::hash_combine(seed, w->depthPriority());
            boost::hash_combine(seed, w->excludedFromPostEffects());
            boost::hash_combine(seed, w->castsShadows());
            boost::hash_combine(seed, w->receivesShadows());
            boost::hash_combine(seed, w->shader().get());
            return seed;
        }
    };

    // This function object returns true if two render items have
    // identical look.
    // We ignore the render item's name and its world matrix.
    struct VisEqualTo : std::binary_function<HardwareInstanceData*, HardwareInstanceData*, bool>
    {
        bool operator()(const HardwareInstanceData* const& xData,
            const HardwareInstanceData* const& yData) const
        {
            const RenderItemWrapper* x = xData->renderItem();
            const RenderItemWrapper* y = yData->renderItem();
            assert(x && y);
            return x->type()                   == y->type() &&
                x->primitive()                 == y->primitive() &&
                x->userData().get()            == y->userData().get() &&
                x->indices().get()             == y->indices().get() &&
                x->positions().get()           == y->positions().get() &&
                x->normals().get()             == y->normals().get() &&
                x->uvs().get()                 == y->uvs().get() &&
                x->enabled()                   == y->enabled() &&
                x->drawMode()                  == y->drawMode() &&
                x->depthPriority()             == y->depthPriority() &&
                x->excludedFromPostEffects()   == y->excludedFromPostEffects() &&
                x->castsShadows()              == y->castsShadows() &&
                x->receivesShadows()           == y->receivesShadows() &&
                x->shader().get()              == y->shader().get();
        }
    };

    // Each HardwareInstance stands for a group of render items that
    // have the same appearance.
    // If the group is instanced, only the master render item has the
    // actual MRenderItem. Other render items have no MRenderItems.
    // Otherwise, the group is an instance candidate. The master and 
    // other render items behave normal.
    struct HardwareInstance
    {
        // The master render item.
        HardwareInstanceData* master;

        // True if this group is an instance candidate (not-yet-instanced).
        // Otherwise, this group is hardware instanced.
        mutable bool candidate;

        // Other render items that have the same appearance as the
        // master render item.
        mutable boost::unordered_set<HardwareInstanceData*> sources;
    };

    typedef boost::multi_index_container<
        HardwareInstance,
        boost::multi_index::indexed_by<
            // Index 0: The pointer of hardware instance data.
            boost::multi_index::hashed_unique<
                BOOST_MULTI_INDEX_MEMBER(HardwareInstance,HardwareInstanceData*,master)
            >,
            // Index 1: The render item visual hash.
            boost::multi_index::hashed_non_unique<
                BOOST_MULTI_INDEX_MEMBER(HardwareInstance,HardwareInstanceData*,master),
                VisHash,
                VisEqualTo
            >
        >
    > HardwareInstanceSet;

    // The associated subscene.
    SubSceneOverride&   fSubSceneOverride;

    // Keeps all hardware instancing information.
    HardwareInstanceSet fInstances;

    // Helper structures to track render item changes.
    // They should be empty after processInstances() method.
    boost::unordered_set<HardwareInstanceData*> fInstancingChangeItems;
    boost::unordered_set<HardwareInstanceData*> fWorldMatrixChangeItems;
    boost::unordered_set<HardwareInstanceData*> fItemsPendingLoad;
    boost::unordered_set<HardwareInstanceData*> fItemsPendingRemove;
};

// The following methods have dependency on HardwareInstanceManagerImpl.
void HardwareInstanceData::notifyInstancingChange()
{
    fManager->notifyInstancingChange(this);
}

void HardwareInstanceData::notifyInstancingClear()
{
    fManager->notifyInstancingClear(this);
}

void HardwareInstanceData::notifyWorldMatrixChange()
{
    // Only need to update instance transform.
    if (isInstanced()) {
        fManager->notifyWorldMatrixChange(this);
    }
}

void HardwareInstanceData::notifyDestroy()
{
    fManager->notifyDestroy(this);
}


//==============================================================================
// CLASS ModelCallbacks
//==============================================================================

// This class manages model-level callbacks.
// gpuCache node-level callbacks are registered in GPUCache::SubSceneOverride.
class ModelCallbacks : boost::noncopyable
{
public:
    static ModelCallbacks& getInstance()
    {
        // Singleton
        static ModelCallbacks sSingleton;
        return sSingleton;
    }

    ModelCallbacks()
    {
        // Initialize DAG object attributes that affect display appearance
        // of their descendant shapes.
        fAttrsAffectAppearance.insert("visibility");
        fAttrsAffectAppearance.insert("lodVisibility");
        fAttrsAffectAppearance.insert("intermediateObject");
        fAttrsAffectAppearance.insert("template");
        fAttrsAffectAppearance.insert("renderLayerInfo");
        fAttrsAffectAppearance.insert("renderLayerId");
        fAttrsAffectAppearance.insert("renderLayerRenderable");
        fAttrsAffectAppearance.insert("renderLayerColor");
        fAttrsAffectAppearance.insert("drawOverride");
        fAttrsAffectAppearance.insert("overrideDisplayType");
        fAttrsAffectAppearance.insert("overrideLevelOfDetail");
        fAttrsAffectAppearance.insert("overrideShading");
        fAttrsAffectAppearance.insert("overrideTexturing");
        fAttrsAffectAppearance.insert("overridePlayback");
        fAttrsAffectAppearance.insert("overrideEnabled");
        fAttrsAffectAppearance.insert("overrideVisibility");
        fAttrsAffectAppearance.insert("overrideColor");
        fAttrsAffectAppearance.insert("useObjectColor");
        fAttrsAffectAppearance.insert("objectColor");
        fAttrsAffectAppearance.insert("ghosting");
        fAttrsAffectAppearance.insert("castsShadows");
        fAttrsAffectAppearance.insert("receiveShadows");

        // Hook model/scene/DG/event callbacks.
        fMayaExitingCallback = MSceneMessage::addCallback(
            MSceneMessage::kMayaExiting, MayaExitingCallback, NULL);
        fSelectionChangedCallback = MModelMessage::addCallback(
            MModelMessage::kActiveListModified, SelectionChangedCallback, this);
        fTimeChangeCallback = MDGMessage::addTimeChangeCallback(
            TimeChangeCallback, this);
        fRenderLayerChangeCallback = MEventMessage::addEventCallback(
            "renderLayerChange", RenderLayerChangeCallback, this);
        fRenderLayerManagerChangeCallback = MEventMessage::addEventCallback(
            "renderLayerManagerChange", RenderLayerChangeCallback, this);

        // Trigger the callback to initialize the selection list.
        selectionChanged();
    }

    ~ModelCallbacks()
    {
        MMessage::removeCallback(fMayaExitingCallback);
        MMessage::removeCallback(fSelectionChangedCallback);
        MMessage::removeCallback(fTimeChangeCallback);
        MMessage::removeCallback(fRenderLayerChangeCallback);
        MMessage::removeCallback(fRenderLayerManagerChangeCallback);
    }

    void registerSubSceneOverride(const ShapeNode* shapeNode, SubSceneOverride* subSceneOverride)
    {
        assert(shapeNode);
        if (!shapeNode) return;

        assert(subSceneOverride);
        if (!subSceneOverride) return;

        // Register the MPxSubSceneOverride to receive callbacks.
        fShapeNodes.insert(std::make_pair(shapeNode, subSceneOverride));
    }

    void deregisterSubSceneOverride(const ShapeNode* shapeNode)
    {
        assert(shapeNode);
        if (!shapeNode) return;

        // Deregister the MPxSubSceneOverride.
        fShapeNodes.erase(shapeNode);
    }

    // Detect selection change and dirty SubSceneOverride.
    void selectionChanged()
    {
        // Retrieve the current selection list.
        MSelectionList list;
        MGlobal::getActiveSelectionList(list);

        // Find all selected gpuCache nodes.
        ShapeNodeNameMap currentSelection;

        MDagPath   dagPath;
        MItDag     dagIt;
        MFnDagNode dagNode;
        for (unsigned int i = 0, size = list.length(); i < size; i++) {
            if (list.getDagPath(i, dagPath) && dagPath.isValid()) {
                // Iterate the DAG to find descendant gpuCache nodes.
                dagIt.reset(dagPath, MItDag::kDepthFirst, MFn::kPluginShape);
                for (; !dagIt.isDone(); dagIt.next()) {
                    if (dagNode.setObject(dagIt.currentItem()) && dagNode.typeId() == ShapeNode::id) {
                        const ShapeNode* shapeNode = (const ShapeNode*)dagNode.userNode();
                        if (shapeNode) {
                            currentSelection.insert(std::make_pair(dagIt.fullPathName(), shapeNode));
                        }
                    }
                }
            }
        }

        // Check Active -> Dormant
        BOOST_FOREACH (const ShapeNodeNameMap::value_type& val, fLastSelection) {
            if (currentSelection.find(val.first) == currentSelection.end()) {
                ShapeNodeSubSceneMap::iterator it = fShapeNodes.find(val.second);
                if (it != fShapeNodes.end() && it->second) {
                    it->second->dirtyEverything();
                }
            }
        }

        // Check Dormant -> Active
        BOOST_FOREACH (const ShapeNodeNameMap::value_type& val, currentSelection) {
            if (fLastSelection.find(val.first) == fLastSelection.end()) {
                ShapeNodeSubSceneMap::iterator it = fShapeNodes.find(val.second);
                if (it != fShapeNodes.end() && it->second) {
                    it->second->dirtyEverything();
                }
            }
        }

        fLastSelection.swap(currentSelection);
    }

    // Detect time change and dirty SubSceneOverride.
    void timeChanged()
    {
        BOOST_FOREACH (ShapeNodeSubSceneMap::value_type& val, fShapeNodes) {
            val.second->dirtyVisibility();   // visibility animation
            val.second->dirtyWorldMatrix();  // xform animation
            val.second->dirtyStreams();      // vertex animation
            val.second->dirtyMaterials();    // material animation
        }
    }

    // Detect render layer change and dirty SubSceneOverride.
    void renderLayerChanged()
    {
        BOOST_FOREACH (ShapeNodeSubSceneMap::value_type& val, fShapeNodes) {
            val.second->dirtyEverything();   // render layer change is destructive
        }
    }

    bool affectAppearance(const MString& attr) const
    {
        return (fAttrsAffectAppearance.find(attr) != fAttrsAffectAppearance.cend());
    }

private:
    static void MayaExitingCallback(void* clientData)
    {
        // Free VP2.0 buffers on exit.
        BuffersCache::getInstance().clear();
        UnitBoundingBox::clear();
    }

    static void SelectionChangedCallback(void* clientData)
    {
        assert(clientData);
        static_cast<ModelCallbacks*>(clientData)->selectionChanged();
    }

    static void TimeChangeCallback(MTime& time, void* clientData)
    {
        assert(clientData);
        static_cast<ModelCallbacks*>(clientData)->timeChanged();
    }

    static void RenderLayerChangeCallback(void* clientData)
    {
        assert(clientData);
        static_cast<ModelCallbacks*>(clientData)->renderLayerChanged();
    }

private:
    MCallbackId fMayaExitingCallback;
    MCallbackId fSelectionChangedCallback;
    MCallbackId fTimeChangeCallback;
    MCallbackId fRenderLayerChangeCallback;
    MCallbackId fRenderLayerManagerChangeCallback;

    typedef boost::unordered_map<MString,const ShapeNode*,MStringHash> ShapeNodeNameMap;
    typedef boost::unordered_map<const ShapeNode*,SubSceneOverride*>   ShapeNodeSubSceneMap;

    ShapeNodeNameMap     fLastSelection;
    ShapeNodeSubSceneMap fShapeNodes;

    boost::unordered_set<MString, MStringHash> fAttrsAffectAppearance;
};


//==============================================================================
// Callbacks
//==============================================================================

// Instance Changed Callback
void InstanceChangedCallback(MDagPath& child, MDagPath& parent, void* clientData)
{
    assert(clientData);
    static_cast<SubSceneOverride*>(clientData)->dirtyEverything();
    static_cast<SubSceneOverride*>(clientData)->resetDagPaths();
}

// World Matrix Changed Callback
void WorldMatrixChangedCallback(MObject& transformNode,
    MDagMessage::MatrixModifiedFlags& modified,
    void* clientData)
{
    assert(clientData);
    static_cast<SubSceneOverride*>(clientData)->dirtyWorldMatrix();
}

// Parent Add/Remove Callback
void ParentChangedCallback(MDagPath& child, MDagPath& parent, void* clientData)
{
    // We register node dirty callbacks on all transform parents/ancestors.
    // If the parent is changed, we will have to re-register all callbacks.
    assert(clientData);
    // Clear the callbacks on parents.
    static_cast<SubSceneOverride*>(clientData)->clearNodeDirtyCallbacks();
    // Dirty the render items so we re-register callbacks again in update().
    static_cast<SubSceneOverride*>(clientData)->dirtyEverything();
}

// Node Dirty Callback
void NodeDirtyCallback(MObject& node, MPlug& plug, void* clientData)
{
    // One of the parent/ancestor has changed.
    // Dirty the SubSceneOverride if the attribute will affect
    // the appearance of the gpuCache shape.
    assert(clientData);
    MFnAttribute attr(plug.attribute());
    if (ModelCallbacks::getInstance().affectAppearance(attr.name())) {
        static_cast<SubSceneOverride*>(clientData)->dirtyEverything();
    }
}

}


namespace GPUCache {

using namespace MHWRender;


//==============================================================================
// CLASS SubSceneOverride::HardwareInstanceManager
//==============================================================================

// This class resolve the dependency problem between RenderItemWrapper and
// GPUCache::SubSceneOverride.
class SubSceneOverride::HardwareInstanceManager : boost::noncopyable
{
public:
    HardwareInstanceManager(SubSceneOverride& subSceneOverride)
        : fImpl(new HardwareInstanceManagerImpl(subSceneOverride))
    {}

    ~HardwareInstanceManager()
    {}

    void processInstances(MSubSceneContainer& container)
    {
        fImpl->processInstances(container);
    }

    void resetInstances(MSubSceneContainer& container)
    {
        fImpl->resetInstances(container);
    }

    void installHardwareInstanceData(RenderItemWrapper::Ptr& renderItem)
    {
        if (!renderItem->hasHardwareInstanceData()) {
            boost::shared_ptr<HardwareInstanceData> data(
                new HardwareInstanceData(fImpl.get(), renderItem.get())
            );
            renderItem->installHardwareInstanceData(data);
        }
    }

private:
    boost::shared_ptr<HardwareInstanceManagerImpl> fImpl;
};


//==============================================================================
// CLASS SubSceneOverride::HierarchyStat
//==============================================================================

// This class contains the analysis result of the sub-node hierarchy.
class SubSceneOverride::HierarchyStat : boost::noncopyable
{
public:
    typedef boost::shared_ptr<const HierarchyStat> Ptr;

    // This is the status of a sub-node and its descendants.
    struct SubNodeStat
    {
        // False if the sub-node and all its descendants have no visibility animation.
        bool   isVisibilityAnimated;
        // False if the sub-node and all its descendants have no xform animation.
        bool   isXformAnimated;
        // False if the sub-node and all its descendants have no vertices animation.
        bool   isShapeAnimated;
        // False if the sub-node and all its descendants have no diffuse color animation.
        bool   isDiffuseColorAnimated;
        // The next sub-node id if we prune at this sub-node. (depth first, preorder)
        size_t nextSubNodeIndex;
        // The next shape sub-node id if we prune at this sub-node. (depth first, preorder)
        size_t nextShapeSubNodeIndex;

        SubNodeStat()
            : isVisibilityAnimated(false),
              isXformAnimated(false),
              isShapeAnimated(false),
              isDiffuseColorAnimated(false),
              nextSubNodeIndex(0),
              nextShapeSubNodeIndex(0)
        {}
    };

    ~HierarchyStat() {}

    void setStat(size_t subNodeIndex, SubNodeStat& stat)
    {
        if (subNodeIndex >= fStats.size()) {
            fStats.resize(subNodeIndex+1);
        }
        fStats[subNodeIndex] = stat;
    }

    const SubNodeStat& stat(size_t subNodeIndex) const
    { return fStats[subNodeIndex]; }

private:
    HierarchyStat() {}
    friend class HierarchyStatVisitor;

    std::vector<SubNodeStat> fStats;
};


//==============================================================================
// CLASS SubSceneOverride::HierarchyStatVisitor
//==============================================================================

// This class analyzes the sub-node hierarchy to help pruning non-animated sub-hierarchy.
class SubSceneOverride::HierarchyStatVisitor : public SubNodeVisitor
{
public:
    HierarchyStatVisitor(const SubNode::Ptr& geometry)
        : fGeometry(geometry),
          fIsParentVisibilityAnimated(false),
          fIsVisibilityAnimated(false),
          fIsParentXformAnimated(false),
          fIsXformAnimated(false),
          fIsShapeAnimated(false),
          fIsDiffuseColorAnimated(false),
          fSubNodeIndex(0),
          fShapeSubNodeIndex(0)
    {
        fHierarchyStat.reset(new HierarchyStat());
    }

    virtual ~HierarchyStatVisitor()
    {}

    const HierarchyStat::Ptr getStat() const
    { return fHierarchyStat; }

    virtual void visit(const XformData&   xform,
                       const SubNode&     subNode)
    {
        // Increase the sub-node counter.
        size_t thisSubNodeIndex = fSubNodeIndex;
        fSubNodeIndex++;

        // Is the visibility animated?
        bool isVisibilityAnimated = false;
        if (xform.getSamples().size() > 1) {
            const boost::shared_ptr<const XformSample>& sample =
                xform.getSamples().begin()->second;
            if (sample) {
                const bool oneVisibility = sample->visibility();
                BOOST_FOREACH (const XformData::SampleMap::value_type& val, xform.getSamples()) {
                    if (val.second && val.second->visibility() != oneVisibility) {
                        isVisibilityAnimated = true;
                        break;
                    }
                }
            }
        }

        // Is the xform animated?
        bool isXformAnimated = false;
        if (xform.getSamples().size() > 1) {
            const boost::shared_ptr<const XformSample>& sample =
                xform.getSamples().begin()->second;
            if (sample) {
                const MMatrix& oneMatrix = sample->xform();
                BOOST_FOREACH (const XformData::SampleMap::value_type& val, xform.getSamples()) {
                    if (val.second && val.second->xform() != oneMatrix) {
                        isXformAnimated = true;
                        break;
                    }
                }
            }
        }

        // Push the xform/visibility animated flag down the hierarchy.
        {
            ScopedGuard<bool> parentVisibilityGuard(fIsParentVisibilityAnimated);
            fIsParentVisibilityAnimated = fIsParentVisibilityAnimated || isVisibilityAnimated;

            ScopedGuard<bool> parentXformGuard(fIsParentXformAnimated);
            fIsParentXformAnimated = fIsParentXformAnimated || isXformAnimated;

            // Shape animated flags for all descendant shapes.
            bool isShapeAnimated        = false;
            bool isDiffuseColorAnimated = false;

            // Recursive calls into children
            BOOST_FOREACH (const SubNode::Ptr& child, subNode.getChildren()) {
                child->accept(*this);

                // Merge shape animated flags.
                isVisibilityAnimated   = isVisibilityAnimated   || fIsVisibilityAnimated;
                isXformAnimated        = isXformAnimated        || fIsXformAnimated;
                isShapeAnimated        = isShapeAnimated        || fIsShapeAnimated;
                isDiffuseColorAnimated = isDiffuseColorAnimated || fIsDiffuseColorAnimated;
            }

            // Pull shape animated flags up the hierarchy.
            fIsVisibilityAnimated   = isVisibilityAnimated;
            fIsXformAnimated        = isXformAnimated;
            fIsShapeAnimated        = isShapeAnimated;
            fIsDiffuseColorAnimated = isDiffuseColorAnimated;
        }

        appendStat(thisSubNodeIndex);
    }

    virtual void visit(const ShapeData&   shape,
                       const SubNode&     subNode)
    {
        // Increase the sub-node counter.
        size_t thisSubNodeIndex = fSubNodeIndex;
        fSubNodeIndex++;
        fShapeSubNodeIndex++;

        // Is the shape animated ?
        fIsShapeAnimated = shape.getSamples().size() > 1;

        // Is the diffuse color animated?
        fIsDiffuseColorAnimated = false;

        if (fIsShapeAnimated) {
            const boost::shared_ptr<const ShapeSample>& sample =
                shape.getSamples().begin()->second;
            if (sample) {
                const MColor& oneColor = sample->diffuseColor();
                BOOST_FOREACH (const ShapeData::SampleMap::value_type& val, shape.getSamples()) {
                    if (val.second && val.second->diffuseColor() != oneColor) {
                        fIsDiffuseColorAnimated = true;
                        break;
                    }
                }
            }
        }

        // Is the visibility animated?
        fIsVisibilityAnimated = false;

        if (fIsShapeAnimated) {
            const boost::shared_ptr<const ShapeSample>& sample =
                shape.getSamples().begin()->second;
            if (sample) {
                const bool oneVisibility = sample->visibility();
                BOOST_FOREACH (const ShapeData::SampleMap::value_type& val, shape.getSamples()) {
                    if (val.second && val.second->visibility() != oneVisibility) {
                        fIsVisibilityAnimated = true;
                        break;
                    }
                }
            }
        }

        // Shape's xform is not animated..
        fIsXformAnimated = false;

        appendStat(thisSubNodeIndex);
    }

    void appendStat(size_t subNodeIndex)
    {
        // Record the stat of this sub-node.
        HierarchyStat::SubNodeStat stat;
        stat.isVisibilityAnimated   = fIsVisibilityAnimated || fIsParentVisibilityAnimated;
        stat.isXformAnimated        = fIsXformAnimated || fIsParentXformAnimated;
        stat.isShapeAnimated        = fIsShapeAnimated;
        stat.isDiffuseColorAnimated = fIsDiffuseColorAnimated;
        stat.nextSubNodeIndex       = fSubNodeIndex;
        stat.nextShapeSubNodeIndex  = fShapeSubNodeIndex;

        fHierarchyStat->setStat(subNodeIndex, stat);
    }

private:
    const SubNode::Ptr fGeometry;
    bool               fIsParentVisibilityAnimated;
    bool               fIsVisibilityAnimated;
    bool               fIsParentXformAnimated;
    bool               fIsXformAnimated;
    bool               fIsShapeAnimated;
    bool               fIsDiffuseColorAnimated;
    size_t             fSubNodeIndex;
    size_t             fShapeSubNodeIndex;

    boost::shared_ptr<HierarchyStat> fHierarchyStat;
};


//==============================================================================
// CLASS SubSceneOverride::SubNodeRenderItems
//==============================================================================

// This class contains the render items for each sub node.
class SubSceneOverride::SubNodeRenderItems : boost::noncopyable
{
public:
    typedef boost::shared_ptr<SubNodeRenderItems> Ptr;

    SubNodeRenderItems()
        : fIsBoundingBoxPlaceHolder(false),
          fIsSelected(false),
          fVisibility(true),
          fValidPoly(true)
    {}

    ~SubNodeRenderItems()
    {}

    void updateRenderItems(SubSceneOverride&   subSceneOverride,
                           MSubSceneContainer& container,
                           const MString&      subNodePrefix,
                           const MColor&       wireColor,
                           const ShapeData&    shape,
                           const SubNode&      subNode,
                           const bool          isSelected)
    {
        // Get the current shape sample.
        const boost::shared_ptr<const ShapeSample>& sample =
            shape.getSample(subSceneOverride.getTime());
        if (!sample) return;

        // Cache flags
        fIsBoundingBoxPlaceHolder = sample->isBoundingBoxPlaceHolder();
        fIsSelected               = isSelected;

        // Bounding box place holder.
        updateBoundingBoxItems(subSceneOverride, container, subNodePrefix, wireColor, subNode);

        // Dormant Wireframe
        updateDormantWireItems(subSceneOverride, container, subNodePrefix, wireColor);

        // Active Wireframe
        updateActiveWireItems(subSceneOverride, container, subNodePrefix, wireColor);

        // Shaded
        updateShadedItems(subSceneOverride, container, subNodePrefix, shape,
            sample->diffuseColor(), sample->numIndexGroups());
    }

    void updateVisibility(SubSceneOverride&   subSceneOverride,
                          MSubSceneContainer& container,
                          const bool          visibility,
                          const ShapeData&    shape)
    {
        // Cache the sub-node visibility flag.
        fVisibility = visibility;

        // Enable or disable render items.
        toggleBoundingBoxItem();
        toggleDormantWireItem();
        toggleActiveWireItem();
        toggleShadedItems();
    }

    void updateWorldMatrix(SubSceneOverride&   subSceneOverride,
                           MSubSceneContainer& container,
                           const MMatrix&      matrix,
                           const ShapeData&    shape)
    {
        // Set the world matrix.
        if (fBoundingBoxItem) {
            const boost::shared_ptr<const ShapeSample>& sample =
                shape.getSample(subSceneOverride.getTime());
            if (sample) {
                const MBoundingBox& boundingBox = sample->boundingBox();
                const MMatrix worldMatrix =
                    UnitBoundingBox::boundingBoxMatrix(boundingBox) * matrix;
                fBoundingBoxItem->setWorldMatrix(worldMatrix);
            }
        }

        if (fDormantWireItem) {
            fDormantWireItem->setWorldMatrix(matrix);
        }

        if (fActiveWireItem) {
            fActiveWireItem->setWorldMatrix(matrix);
        }

        BOOST_FOREACH (RenderItemWrapper::Ptr& shadedItem, fShadedItems) {
            shadedItem->setWorldMatrix(matrix);
        }
    }

    void updateStreams(SubSceneOverride&   subSceneOverride,
                       MSubSceneContainer& container,
                       const ShapeData&    shape)
    {
        const boost::shared_ptr<const ShapeSample>& sample =
            shape.getSample(subSceneOverride.getTime());
        if (!sample) return;

        // If this sample is an empty poly, we disable all render items and return.
        fValidPoly = sample->numVerts() > 0 &&
                     sample->numWires() > 0 &&
                     sample->numTriangles() > 0 &&
                     sample->positions();
        // Enable or disable render items.
        toggleBoundingBoxItem();
        toggleDormantWireItem();
        toggleActiveWireItem();
        toggleShadedItems();
        if (!fValidPoly) {
            // Nothing to do. Render items are disabled.
            return;
        }

        // Update the wireframe streams.
        if (fDormantWireItem) {
            fDormantWireItem->setBuffers(
                subSceneOverride,
                sample->wireVertIndices(),
                sample->positions(),
                boost::shared_ptr<const VertexBuffer>(),
                boost::shared_ptr<const VertexBuffer>(),
                sample->boundingBox()
            );
        }

        if (fActiveWireItem) {
            fActiveWireItem->setBuffers(
                subSceneOverride,
                sample->wireVertIndices(),
                sample->positions(),
                boost::shared_ptr<const VertexBuffer>(),
                boost::shared_ptr<const VertexBuffer>(),
                sample->boundingBox()
            );
        }

        // Update the shaded streams.
        for (size_t groupId = 0; groupId < sample->numIndexGroups(); groupId++) {
            if (groupId >= fShadedItems.size()) break;  // background loading

            assert(fShadedItems[groupId]);
            if (!fShadedItems[groupId]) continue;

            fShadedItems[groupId]->setBuffers(
                subSceneOverride,
                sample->triangleVertIndices(groupId),
                sample->positions(),
                sample->normals(),
                sample->uvs(),
                sample->boundingBox()
            );
        }
    }

    void updateMaterials(SubSceneOverride&   subSceneOverride,
                         MSubSceneContainer& container,
                         const ShapeData&    shape)
    {
        const boost::shared_ptr<const ShapeSample>& sample =
            shape.getSample(subSceneOverride.getTime());
        if (!sample) return;

        for (size_t groupId = 0; groupId < sample->numIndexGroups(); groupId++) {
            if (groupId >= fShadedItems.size()) break;  // background loading
            if (groupId >= fSharedDiffuseColorShaders.size()) break;
            if (groupId >= fUniqueDiffuseColorShaders.size()) break;
            if (groupId >= fMaterialShaders.size()) break;

            assert(fShadedItems[groupId]);
            if (!fShadedItems[groupId]) continue;

            // First, check if the shader instance is created from a MaterialGraph.
            ShaderInstancePtr shader = fMaterialShaders[groupId];
            if (shader) {
                // Nothing to do.
                continue;
            }

            // Then, check if the shader instance is already unique to the render item.
            shader = fUniqueDiffuseColorShaders[groupId];
            if (shader) {
                // Unique shader instance belongs to this render item.
                // Set the diffuse color directly.
                setDiffuseColor(shader.get(), sample->diffuseColor());
                continue;
            }

            // Then, get a shared shader instance from cache.
            shader = ShaderInstanceCache::getInstance().getSharedDiffuseColorShader(
                sample->diffuseColor());

            // If the shared shader instance is different from the existing one,
            // there is diffuse color animation.
            // We promote the shared shader instance to a unique shader instance.
            assert(fSharedDiffuseColorShaders[groupId]);  // set in updateRenderItems()
            if (shader != fSharedDiffuseColorShaders[groupId]) {
                shader = ShaderInstanceCache::getInstance().getUniqueDiffuseColorShader(
                    sample->diffuseColor());

                fSharedDiffuseColorShaders[groupId].reset();
                fUniqueDiffuseColorShaders[groupId] = shader;

                fShadedItems[groupId]->setShader(shader);
            }
        }
    }

    void updateBoundingBoxItems(SubSceneOverride&   subSceneOverride,
                                MSubSceneContainer& container,
                                const MString&      subNodePrefix,
                                const MColor&       wireColor,
                                const SubNode&      subNode)
    {
        if (!fIsBoundingBoxPlaceHolder) {
            // This shape is no longer a bounding box place holder.
            if (fBoundingBoxItem) {
                fBoundingBoxItem->removeFromContainer(container);
                fBoundingBoxItem.reset();
            }
            return;
        }

        // Bounding box place holder render item.
        if (!fBoundingBoxItem) {
            // Create the bounding box render item.
            const MString boundingBoxItemName = subNodePrefix + ":boundingBox";
            fBoundingBoxItem.reset(new RenderItemWrapper(
                boundingBoxItemName,
                MRenderItem::NonMaterialSceneItem,
                MGeometry::kLines
            ));
            fBoundingBoxItem->setDrawMode((MGeometry::DrawMode)(MGeometry::kWireframe | MGeometry::kShaded | MGeometry::kTextured));
            fBoundingBoxItem->setDepthPriority(MRenderItem::sDormantWireDepthPriority);

            // Set the shader so that we can fill the geometry data.
            ShaderInstancePtr boundingBoxShader = 
                ShaderInstanceCache::getInstance().getSharedBoundingBoxPlaceHolderShader(wireColor);
            if (boundingBoxShader) {
                fBoundingBoxItem->setShader(boundingBoxShader);
            }

            // Add to the container.
            fBoundingBoxItem->addToContainer(container);

            // Set unit bounding box buffer.
            fBoundingBoxItem->setBuffers(
                subSceneOverride,
                UnitBoundingBox::indices(),
                UnitBoundingBox::positions(),
                boost::shared_ptr<const VertexBuffer>(),
                boost::shared_ptr<const VertexBuffer>(),
                UnitBoundingBox::boundingBox()
            );

            // Add a custom data to indicate the sub-node.
            fBoundingBoxItem->setCustomData(
                boost::shared_ptr<MUserData>(new SubNodeUserData(subNode))
            );
        }

        // Update shader color.
        ShaderInstancePtr boundingBoxShader = 
            ShaderInstanceCache::getInstance().getSharedBoundingBoxPlaceHolderShader(wireColor);
        if (boundingBoxShader) {
            fBoundingBoxItem->setShader(boundingBoxShader);
        }

        toggleBoundingBoxItem();
    }

    void updateDormantWireItems(SubSceneOverride&   subSceneOverride,
                                MSubSceneContainer& container,
                                const MString&      subNodePrefix,
                                const MColor&       wireColor)
    {
        if (fIsBoundingBoxPlaceHolder) {
            // This shape is a bounding box place holder.
            if (fDormantWireItem) fDormantWireItem->setEnabled(false);
            return;
        }

        // Update dormant wireframe item.
        if (!fDormantWireItem) {
            // Create the dormant wireframe render item.
            const MString dormantWireItemName = subNodePrefix + ":dormantWire";
            fDormantWireItem.reset(new RenderItemWrapper(
                dormantWireItemName,
                MRenderItem::DecorationItem,
                MGeometry::kLines
            ));
            fDormantWireItem->setDrawMode(MGeometry::kWireframe);
            fDormantWireItem->setDepthPriority(MRenderItem::sDormantWireDepthPriority);

            // Add to the container
            fDormantWireItem->addToContainer(container);
        }

        // Hardware instancing.
        boost::shared_ptr<HardwareInstanceManager>& hwInstanceManager = 
            subSceneOverride.hardwareInstanceManager();
        if (hwInstanceManager) {
            hwInstanceManager->installHardwareInstanceData(fDormantWireItem);
        }

        toggleDormantWireItem();

        // Dormant wireframe color.
        ShaderInstancePtr dormantWireShader =
            (DisplayPref::wireframeOnShadedMode() == DisplayPref::kWireframeOnShadedFull)
            ? ShaderInstanceCache::getInstance().getSharedWireShader(wireColor)
            : ShaderInstanceCache::getInstance().getSharedWireShaderWithCB(wireColor);
        if (dormantWireShader) {
            fDormantWireItem->setShader(dormantWireShader);
        }
    }

    void updateActiveWireItems(SubSceneOverride&   subSceneOverride,
                               MSubSceneContainer& container,
                               const MString&      subNodePrefix,
                               const MColor&       wireColor)
    {
        if (fIsBoundingBoxPlaceHolder) {
            // This shape is a bounding box place holder or unselected.
            if (fActiveWireItem)  fActiveWireItem->setEnabled(false);
            return;
        }

        if (!fActiveWireItem) {
            // Create the active wireframe render item.
            const MString activeWireItemName = subNodePrefix + ":activeWire";
            fActiveWireItem.reset(new RenderItemWrapper(
                activeWireItemName,
                MRenderItem::DecorationItem,
                MGeometry::kLines
            ));
            fActiveWireItem->setDrawMode((MGeometry::DrawMode)(MGeometry::kWireframe | MGeometry::kShaded | MGeometry::kTextured));
            fActiveWireItem->setDepthPriority(MRenderItem::sActiveWireDepthPriority);

            // Add to the container.
            fActiveWireItem->addToContainer(container);
        }

        // Hardware instancing.
        boost::shared_ptr<HardwareInstanceManager>& hwInstanceManager = 
            subSceneOverride.hardwareInstanceManager();
        if (hwInstanceManager) {
            hwInstanceManager->installHardwareInstanceData(fActiveWireItem);
        }

        toggleActiveWireItem();

        // Active wireframe color.
        ShaderInstancePtr activeWireShader =
            (DisplayPref::wireframeOnShadedMode() == DisplayPref::kWireframeOnShadedFull)
            ? ShaderInstanceCache::getInstance().getSharedWireShader(wireColor)
            : ShaderInstanceCache::getInstance().getSharedWireShaderWithCB(wireColor);
        if (activeWireShader) {
            fActiveWireItem->setShader(activeWireShader);
        }
    }

    void updateShadedItems(SubSceneOverride&   subSceneOverride,
                           MSubSceneContainer& container,
                           const MString&      subNodePrefix,
                           const ShapeData&    shape,
                           const MColor&       diffuseColor,
                           const size_t        nbIndexGroups)
    {
        // Shaded render items.
        if (fIsBoundingBoxPlaceHolder) {
            // This shape is a bounding box place holder.
            BOOST_FOREACH (RenderItemWrapper::Ptr& item, fShadedItems) {
                item->setEnabled(false);
            }
            return;
        }

        if (fShadedItems.empty()) {
            // Create a render item for each index group.
            fShadedItems.reserve(nbIndexGroups);

            // Each render item has an associated MShaderInstance.
            fSharedDiffuseColorShaders.reserve(nbIndexGroups);
            fUniqueDiffuseColorShaders.reserve(nbIndexGroups);
            fMaterialShaders.reserve(nbIndexGroups);

            for (size_t groupId = 0; groupId < nbIndexGroups; groupId++) {
                const MString shadedItemName = subNodePrefix + ":shaded" + (int)groupId;
                RenderItemWrapper::Ptr renderItem(new RenderItemWrapper(
                    shadedItemName,
                    MRenderItem::MaterialSceneItem,
                    MGeometry::kTriangles
                ));
                renderItem->setDrawMode((MGeometry::DrawMode)(MGeometry::kShaded | MGeometry::kTextured));
                renderItem->setExcludedFromPostEffects(false);  // SSAO, etc..
                fShadedItems.push_back(renderItem);

                // Check if we have any material that is assigned to this index group.
                ShaderInstancePtr shader;
                const std::vector<MString>&  materialsAssignment = shape.getMaterials();
                const MaterialGraphMap::Ptr& materials = subSceneOverride.getMaterial();
                if (materials && groupId < materialsAssignment.size()) {
                    const MaterialGraph::Ptr graph = materials->find(materialsAssignment[groupId]);
                    if (graph) {
                        shader = ShaderInstanceCache::getInstance().getSharedShadedMaterialShader(
                            graph, subSceneOverride.getTime()
                        );
                    }
                }

                if (shader) {
                    // We have successfully created a material shader.
                    renderItem->setShader(shader);

                    fMaterialShaders.push_back(shader);
                    fSharedDiffuseColorShaders.push_back(ShaderInstancePtr());
                    fUniqueDiffuseColorShaders.push_back(ShaderInstancePtr());
                }
                else {
                    // There is no materials. Fallback to diffuse color.

                    // Let's assume that the diffuse color is not animated at beginning.
                    // If the diffuse color changes, we will promote the shared shader to
                    // a unique shader.
                    ShaderInstancePtr sharedShader =
                        ShaderInstanceCache::getInstance().getSharedDiffuseColorShader(diffuseColor);
                    if (sharedShader) {
                        renderItem->setShader(sharedShader);
                    }

                    fMaterialShaders.push_back(ShaderInstancePtr());
                    fSharedDiffuseColorShaders.push_back(sharedShader);
                    fUniqueDiffuseColorShaders.push_back(ShaderInstancePtr());
                }

                // Add to the container.
                renderItem->addToContainer(container);
            }
        }

        // Check if we can cast/receive shadows and hardware instancing.
        const bool castsShadows   = subSceneOverride.castsShadows();
        const bool receiveShadows = subSceneOverride.receiveShadows();

        BOOST_FOREACH (RenderItemWrapper::Ptr& renderItem, fShadedItems) {
            // Set Casts Shadows and Receives Shadows.
            renderItem->setCastsShadows(castsShadows);
            renderItem->setReceivesShadows(receiveShadows);

            // Hardware instancing.
            boost::shared_ptr<HardwareInstanceManager>& hwInstanceManager = 
                subSceneOverride.hardwareInstanceManager();
            if (hwInstanceManager && renderItem->shader() && !renderItem->shader()->isTransparent()) {
                hwInstanceManager->installHardwareInstanceData(renderItem);
            }
        }

        toggleShadedItems();
    }

    // Enable or disable bounding box place holder item.
    void toggleBoundingBoxItem()
    {
        if (fBoundingBoxItem) {
            if (fIsBoundingBoxPlaceHolder) {
                fBoundingBoxItem->setEnabled(fVisibility);
            }
            else {
                fBoundingBoxItem->setEnabled(false);
            }
        }
    }

    // Enable or disable dormant wireframe item.
    void toggleDormantWireItem()
    {
        if (fDormantWireItem) {
            if (fIsBoundingBoxPlaceHolder) {
                fDormantWireItem->setEnabled(false);
            }
            else {
                fDormantWireItem->setEnabled(fVisibility && fValidPoly && !fIsSelected);
            }
        }
    }

    // Enable or disable active wireframe item.
    void toggleActiveWireItem()
    {
        if (fActiveWireItem) {
            if (fIsBoundingBoxPlaceHolder) {
                fActiveWireItem->setEnabled(false);
            }
            else {
                fActiveWireItem->setEnabled(fVisibility && fValidPoly && fIsSelected);
            }
        }
    }

    // Enable or disable shaded items.
    void toggleShadedItems()
    {
        BOOST_FOREACH (RenderItemWrapper::Ptr& shadedItem, fShadedItems) {
            if (fIsBoundingBoxPlaceHolder) {
                shadedItem->setEnabled(false);
            }
            else {
                shadedItem->setEnabled(fVisibility && fValidPoly);
            }
        }
    }

    void hideRenderItems()
    {
        // Simply disable all render items.
        if (fActiveWireItem) {
            fActiveWireItem->setEnabled(false);
        }

        if (fDormantWireItem) {
            fDormantWireItem->setEnabled(false);
        }

        if (fBoundingBoxItem) {
            fBoundingBoxItem->setEnabled(false);
        }

        BOOST_FOREACH (RenderItemWrapper::Ptr& item, fShadedItems) {
            item->setEnabled(false);
        }
    }

    void destroyRenderItems(MSubSceneContainer& container)
    {
        // Destroy all render items.
        if (fActiveWireItem) {
            fActiveWireItem->removeFromContainer(container);
            fActiveWireItem.reset();
        }

        if (fDormantWireItem) {
            fDormantWireItem->removeFromContainer(container);
            fDormantWireItem.reset();
        }

        if (fBoundingBoxItem) {
            fBoundingBoxItem->removeFromContainer(container);
            fBoundingBoxItem.reset();
        }

        BOOST_FOREACH (RenderItemWrapper::Ptr& item, fShadedItems) {
            item->removeFromContainer(container);
            item.reset();
        }
        fShadedItems.clear();
    }

private:
    // Render items for this sub-node.
    RenderItemWrapper::Ptr               fBoundingBoxItem;
    RenderItemWrapper::Ptr               fActiveWireItem;
    RenderItemWrapper::Ptr               fDormantWireItem;
    std::vector<RenderItemWrapper::Ptr>  fShadedItems;

    // The following flags control the enable/disable state of render items.
    bool fIsBoundingBoxPlaceHolder; // The sub-node has not been loaded.
    bool fIsSelected;               // Selection state for this sub-node.
    bool fVisibility;               // Visibility for this sub-node.
    bool fValidPoly;                // False if the poly has 0 vertices.

    // Shader instances for shaded render items.
    std::vector<ShaderInstancePtr>  fSharedDiffuseColorShaders;
    std::vector<ShaderInstancePtr>  fUniqueDiffuseColorShaders;
    std::vector<ShaderInstancePtr>  fMaterialShaders;
};


//==============================================================================
// CLASS SubSceneOverride::UpdateRenderItemsVisitor
//==============================================================================

// Update the render items.
class SubSceneOverride::UpdateRenderItemsVisitor : public SubNodeVisitor
{
public:
    UpdateRenderItemsVisitor(SubSceneOverride&      subSceneOverride,
                             MSubSceneContainer&    container,
                             const MString&         instancePrefix,
                             const MColor&          wireColor,
                             const bool             isSelected,
                             SubNodeRenderItemList& subNodeItems)
        : fSubSceneOverride(subSceneOverride),
          fContainer(container),
          fWireColor(wireColor),
          fIsSelected(isSelected),
          fSubNodeItems(subNodeItems),
          fLongName(instancePrefix),
          fSubNodeIndex(0)
    {}

    virtual ~UpdateRenderItemsVisitor()
    {}

    virtual void visit(const XformData&   xform,
                       const SubNode&     subNode)
    {
        // We use the hierarchical name to represent the unique render item name.
        ScopedGuard<MString> longNameGuard(fLongName);
        bool isTop = subNode.getParents().empty() && subNode.getName() == "|";
        if (!isTop) {
            fLongName += "|";
            fLongName += subNode.getName();
        }

        // Recursive calls into children
        BOOST_FOREACH (const SubNode::Ptr& child, subNode.getChildren()) {
            child->accept(*this);
        }
    }

    virtual void visit(const ShapeData&   shape,
                       const SubNode&     subNode)
    {
        // We use the hierarchical name to represent the unique render item name.
        const MString prevName = fLongName;
        fLongName += "|";
        fLongName += subNode.getName();

        // Update render items for this sub-node.
        updateRenderItems(shape, subNode);
        fSubNodeIndex++;

        // Restore to the previous name.
        fLongName = prevName;
    }

    void updateRenderItems(const ShapeData& shape, const SubNode& subNode)
    {
        // Create new sub-node render items.
        if (fSubNodeIndex >= fSubNodeItems.size()) {
            fSubNodeItems.push_back(boost::make_shared<SubNodeRenderItems>());
        }

        // Update the render items for this sub-node.
        fSubNodeItems[fSubNodeIndex]->updateRenderItems(
            fSubSceneOverride,
            fContainer,
            fLongName,
            fWireColor,
            shape,
            subNode,
            fIsSelected
        );
    }

private:
    SubSceneOverride&      fSubSceneOverride;
    MSubSceneContainer&    fContainer;
    const MColor&          fWireColor;
    const bool             fIsSelected;
    SubNodeRenderItemList& fSubNodeItems;

    MString fLongName;
    size_t  fSubNodeIndex;
};


//==============================================================================
// CLASS SubSceneOverride::UpdateVisitorWithPrune
//==============================================================================

// This class is a visitor for the sub-node hierarchy and allowing to prune
// a sub part of it.
// Curiously recurring template pattern.
// The derived class should implement the following two methods:
//
// Test if this sub-node and its descendants can be pruned.
// bool canPrune(const HierarchyStat::SubNodeStat& stat);
//
// Update the shape sub-node.
// void update(const ShapeData&         shape,
//             const SubNode&           subNode,
//             SubNodeRenderItems::Ptr& subNodeItems);
//
template<typename DERIVED>
class SubSceneOverride::UpdateVisitorWithPrune : public SubNodeVisitor
{
public:
    UpdateVisitorWithPrune(SubSceneOverride&      subSceneOverride,
                           MSubSceneContainer&    container,
                           SubNodeRenderItemList& subNodeItems)
        : fSubSceneOverride(subSceneOverride),
          fContainer(container),
          fSubNodeItems(subNodeItems),
          fDontPrune(false),
          fTraverseInvisible(false),
          fSubNodeIndex(0),
          fShapeSubNodeIndex(0)
    {}

    virtual ~UpdateVisitorWithPrune()
    {}

    // Disable prune.
    void setDontPrune(bool dontPrune)
    {
        fDontPrune = dontPrune;
    }

    // Traverse invisible sub-nodes.
    void setTraverseInvisible(bool traverseInvisible)
    {
        fTraverseInvisible = traverseInvisible;
    }

    virtual void visit(const XformData&   xform,
                       const SubNode&     subNode)
    {
        // Try to prune this sub-hierarchy.
        const HierarchyStat::Ptr& hierarchyStat = fSubSceneOverride.getHierarchyStat();
        if (hierarchyStat) {
            const HierarchyStat::SubNodeStat& stat = hierarchyStat->stat(fSubNodeIndex);

            if (!fDontPrune) {
                if (static_cast<DERIVED*>(this)->canPrune(stat)) {
                    // Prune this sub-hierarchy.
                    // Fast-forward to the next sub-node.
                    fSubNodeIndex      = stat.nextSubNodeIndex;
                    fShapeSubNodeIndex = stat.nextShapeSubNodeIndex;
                    return;
                }

                if (!fTraverseInvisible) {
                    const boost::shared_ptr<const XformSample>& sample =
                        xform.getSample(fSubSceneOverride.getTime());
                    if (sample && !sample->visibility()) {
                        // Invisible sub-node. Prune this sub-hierarchy.
                        // Fast-forward to the next sub-node.
                        fSubNodeIndex      = stat.nextSubNodeIndex;
                        fShapeSubNodeIndex = stat.nextShapeSubNodeIndex;
                        return;
                    }
                }
            }
        }

        fSubNodeIndex++;

        // Recursive calls into children.
        BOOST_FOREACH (const SubNode::Ptr& child, subNode.getChildren()) {
            child->accept(*this);
        }
    }

    virtual void visit(const ShapeData&   shape,
                       const SubNode&     subNode)
    {
        // Update the sub-node.
        assert(fShapeSubNodeIndex < fSubNodeItems.size());
        if (fShapeSubNodeIndex < fSubNodeItems.size()) {
            static_cast<DERIVED*>(this)->update(shape, subNode, fSubNodeItems[fShapeSubNodeIndex]);
        }
        fSubNodeIndex++;
        fShapeSubNodeIndex++;
    }

protected:
    SubSceneOverride&      fSubSceneOverride;
    MSubSceneContainer&    fContainer;
    SubNodeRenderItemList& fSubNodeItems;
    bool                   fDontPrune;
    bool                   fTraverseInvisible;

    size_t  fSubNodeIndex;
    size_t  fShapeSubNodeIndex;
};


//==============================================================================
// CLASS SubSceneOverride::UpdateVisibilityVisitor
//==============================================================================

// Update the visibility.
class SubSceneOverride::UpdateVisibilityVisitor :
    public SubSceneOverride::UpdateVisitorWithPrune<SubSceneOverride::UpdateVisibilityVisitor>
{
public:
    typedef SubSceneOverride::UpdateVisitorWithPrune<SubSceneOverride::UpdateVisibilityVisitor> ParentClass;

    UpdateVisibilityVisitor(SubSceneOverride&      subSceneOverride,
                            MSubSceneContainer&    container,
                            SubNodeRenderItemList& subNodeItems)
        : ParentClass(subSceneOverride, container, subNodeItems),
          fVisibility(true)
    {
        // The visibility visitor should always traverse into invisible sub-nodes
        // because we have to disable the render items for these invisible sub-nodes.
        setTraverseInvisible(true);
    }

    virtual ~UpdateVisibilityVisitor()
    {}

    bool canPrune(const HierarchyStat::SubNodeStat& stat)
    {
        return !stat.isVisibilityAnimated;
    }

    void update(const ShapeData&         shape,
                const SubNode&           subNode,
                SubNodeRenderItems::Ptr& subNodeItems)
    {
        // Get the shape sample.
        const boost::shared_ptr<const ShapeSample>& sample =
            shape.getSample(fSubSceneOverride.getTime());
        if (!sample) return;

        // Shape visibility.
        bool visibility = fVisibility && sample->visibility();

        subNodeItems->updateVisibility(
            fSubSceneOverride,
            fContainer,
            visibility,
            shape
        );
    }

    virtual void visit(const XformData&   xform,
                       const SubNode&     subNode)
    {
        // Get the xform sample.
        const boost::shared_ptr<const XformSample>& sample =
            xform.getSample(fSubSceneOverride.getTime());
        if (!sample) return;

        // Push visibility.
        ScopedGuard<bool> guard(fVisibility);
        fVisibility = fVisibility && sample->visibility();

        ParentClass::visit(xform, subNode);
    }

private:
    bool fVisibility;
};


//==============================================================================
// CLASS SubSceneOverride::UpdateWorldMatrixVisitor
//==============================================================================

// Update the world matrices.
class SubSceneOverride::UpdateWorldMatrixVisitor :
    public SubSceneOverride::UpdateVisitorWithPrune<SubSceneOverride::UpdateWorldMatrixVisitor>
{
public:
    typedef SubSceneOverride::UpdateVisitorWithPrune<SubSceneOverride::UpdateWorldMatrixVisitor> ParentClass;

    UpdateWorldMatrixVisitor(SubSceneOverride&      subSceneOverride,
                             MSubSceneContainer&    container,
                             const MMatrix&         dagMatrix,
                             SubNodeRenderItemList& subNodeItems)
        : ParentClass(subSceneOverride, container, subNodeItems),
          fMatrix(dagMatrix)
    {}

    virtual ~UpdateWorldMatrixVisitor()
    {}

    bool canPrune(const HierarchyStat::SubNodeStat& stat)
    {
        return !stat.isXformAnimated;
    }

    void update(const ShapeData&         shape,
                const SubNode&           subNode,
                SubNodeRenderItems::Ptr& subNodeItems)
    {
        subNodeItems->updateWorldMatrix(
            fSubSceneOverride,
            fContainer,
            fMatrix,
            shape
        );
    }

    virtual void visit(const XformData&   xform,
                       const SubNode&     subNode)
    {
        // Get the xform sample.
        const boost::shared_ptr<const XformSample>& sample =
            xform.getSample(fSubSceneOverride.getTime());
        if (!sample) return;

        // Push matrix.
        ScopedGuard<MMatrix> guard(fMatrix);
        fMatrix = sample->xform() * fMatrix;

        ParentClass::visit(xform, subNode);
    }

private:
    MMatrix fMatrix;
};


//==============================================================================
// CLASS SubSceneOverride::UpdateStreamsVisitor
//==============================================================================

// Update the streams.
class SubSceneOverride::UpdateStreamsVisitor :
    public SubSceneOverride::UpdateVisitorWithPrune<SubSceneOverride::UpdateStreamsVisitor>
{
public:
    typedef SubSceneOverride::UpdateVisitorWithPrune<SubSceneOverride::UpdateStreamsVisitor> ParentClass;

    UpdateStreamsVisitor(SubSceneOverride&      subSceneOverride,
                         MSubSceneContainer&    container,
                         SubNodeRenderItemList& subNodeItems)
        : ParentClass(subSceneOverride, container, subNodeItems)
    {}

    virtual ~UpdateStreamsVisitor()
    {}

    bool canPrune(const HierarchyStat::SubNodeStat& stat)
    {
        return !stat.isShapeAnimated;
    }

    void update(const ShapeData&         shape,
                const SubNode&           subNode,
                SubNodeRenderItems::Ptr& subNodeItems)
    {
        subNodeItems->updateStreams(
            fSubSceneOverride,
            fContainer,
            shape
        );
    }
};


//==============================================================================
// CLASS SubSceneOverride::UpdateDiffuseColorVisitor
//==============================================================================

// Update the streams.
class SubSceneOverride::UpdateDiffuseColorVisitor :
    public SubSceneOverride::UpdateVisitorWithPrune<SubSceneOverride::UpdateDiffuseColorVisitor>
{
public:
    typedef SubSceneOverride::UpdateVisitorWithPrune<SubSceneOverride::UpdateDiffuseColorVisitor> ParentClass;

    UpdateDiffuseColorVisitor(SubSceneOverride&      subSceneOverride,
                              MSubSceneContainer&    container,
                              SubNodeRenderItemList& subNodeItems)
        : ParentClass(subSceneOverride, container, subNodeItems)
    {}

    virtual ~UpdateDiffuseColorVisitor()
    {}

    bool canPrune(const HierarchyStat::SubNodeStat& stat)
    {
        return !stat.isDiffuseColorAnimated;
    }

    void update(const ShapeData&         shape,
                const SubNode&           subNode,
                SubNodeRenderItems::Ptr& subNodeItems)
    {
        subNodeItems->updateMaterials(
            fSubSceneOverride,
            fContainer,
            shape
        );
    }
};


//==============================================================================
// CLASS SubSceneOverride::InstanceRenderItems
//==============================================================================

// This class contains the render items for an instance of gpuCache node.
class SubSceneOverride::InstanceRenderItems : boost::noncopyable
{
public:
    typedef boost::shared_ptr<InstanceRenderItems> Ptr;

    InstanceRenderItems()
        : fVisibility(true),
          fVisibilityValid(false),
          fWorldMatrixValid(false),
          fStreamsValid(false),
          fMaterialsValid(false)
    {}

    ~InstanceRenderItems()
    {}

    // Update the bounding box render item.
    void updateRenderItems(SubSceneOverride&   subSceneOverride,
                           MSubSceneContainer& container,
                           const MDagPath&     dagPath,
                           const MString&      instancePrefix)
    {
        assert(dagPath.isValid());
        if (!dagPath.isValid()) return;

        // Set the path of this instance.
        fDagPath = dagPath;

        // Check if we can see the DAG node.
        fVisibility = dagPath.isVisible();

        // Early out if we can't see this instance.
        if (!fVisibility) {
            // Disable all render items that belong to this instance.
            BOOST_FOREACH (SubNodeRenderItemList::value_type& items, fSubNodeItems) {
                items->hideRenderItems();
            }

            // We have disabled all render items that belong to this instance.
            // When the DAG object is visible again, we need to restore visibility.
            fVisibilityValid = false;
            return;
        }

        // Check if this instance is selected.
        const DisplayStatus displayStatus =
            MGeometryUtilities::displayStatus(dagPath);
        fIsSelected = (displayStatus == kActive) ||
                      (displayStatus == kLead)   ||
                      (displayStatus == kHilite);

        // Get the wireframe color for the whole gpuCache node.
        const MColor wireColor = MGeometryUtilities::wireframeColor(dagPath);

        // Update the bounding box render item.
        if (!fBoundingBoxItem) {
            const MString boundingBoxName = instancePrefix + "BoundingBox";

            // Create the bounding box render item.
            fBoundingBoxItem.reset(new RenderItemWrapper(
                boundingBoxName,
                MRenderItem::NonMaterialSceneItem,
                MGeometry::kLines
            ));
            fBoundingBoxItem->setDrawMode(MGeometry::kBoundingBox);

            // Set the shader so that we can fill geometry data.
            fBoundingBoxShader =
                ShaderInstanceCache::getInstance().getSharedWireShader(wireColor);
            if (fBoundingBoxShader) {
                fBoundingBoxItem->setShader(fBoundingBoxShader);
            }

            // Add to the container.
            fBoundingBoxItem->addToContainer(container);

            // Set unit bounding box buffer.
            fBoundingBoxItem->setBuffers(
                subSceneOverride,
                UnitBoundingBox::indices(),
                UnitBoundingBox::positions(),
                boost::shared_ptr<const VertexBuffer>(),
                boost::shared_ptr<const VertexBuffer>(),
                UnitBoundingBox::boundingBox()
            );
        }

        // Bounding box color
        fBoundingBoxShader =
            ShaderInstanceCache::getInstance().getSharedWireShader(wireColor);
        if (fBoundingBoxShader) {
            fBoundingBoxItem->setShader(fBoundingBoxShader);
        }

        // Bounding box depth priority
        fBoundingBoxItem->setDepthPriority(
            fIsSelected ?
                MRenderItem::sActiveWireDepthPriority :
                MRenderItem::sDormantWireDepthPriority
        );

        fBoundingBoxItem->setEnabled(true);

        // Update the sub-node render items.
        UpdateRenderItemsVisitor visitor(subSceneOverride, container,
            instancePrefix, wireColor, fIsSelected, fSubNodeItems);
        subSceneOverride.getGeometry()->accept(visitor);
    }

    void updateVisibility(SubSceneOverride&   subSceneOverride,
                          MSubSceneContainer& container)
    {
        assert(fDagPath.isValid());
        if (!fDagPath.isValid()) return;

        // Early out if we can't see this instance.
        if (!fVisibility) {
            return;
        }

        // Update the sub-node visibility.
        UpdateVisibilityVisitor visitor(subSceneOverride, container, fSubNodeItems);
        visitor.setDontPrune(!fVisibilityValid);
        subSceneOverride.getGeometry()->accept(visitor);
        fVisibilityValid = true;
    }

    void updateWorldMatrix(SubSceneOverride&   subSceneOverride,
                           MSubSceneContainer& container)
    {
        assert(fDagPath.isValid());
        if (!fDagPath.isValid()) return;

        // Early out if we can't see this instance.
        if (!fVisibility) {
            return;
        }

        // The DAG node's world matrix.
        const MMatrix pathMatrix = fDagPath.inclusiveMatrix();
        const bool pathMatrixChanged = fMatrix != pathMatrix;
        fMatrix = pathMatrix;

        // Update the bounding box render item's world matrix.
        if (fBoundingBoxItem) {
            const MBoundingBox boundingBox = BoundingBoxVisitor::boundingBox(
                subSceneOverride.getGeometry(),
                subSceneOverride.getTime()
            );
            const MMatrix worldMatrix =
                UnitBoundingBox::boundingBoxMatrix(boundingBox) * fMatrix;
            fBoundingBoxItem->setWorldMatrix(worldMatrix);
        }

        // Update the sub-node world matrices
        UpdateWorldMatrixVisitor visitor(subSceneOverride, container,
            fMatrix, fSubNodeItems);
        visitor.setDontPrune(pathMatrixChanged || !fWorldMatrixValid);  // The DAG object's matrix has changed.
        subSceneOverride.getGeometry()->accept(visitor);
        fWorldMatrixValid = true;
    }

    void updateStreams(SubSceneOverride&   subSceneOverride,
                       MSubSceneContainer& container)
    {
        assert(fDagPath.isValid());
        if (!fDagPath.isValid()) return;

        // Early out if we can't see this instance.
        if (!fVisibility) {
            return;
        }

        // Update the sub-node streams.
        UpdateStreamsVisitor visitor(subSceneOverride, container, fSubNodeItems);
        visitor.setDontPrune(!fStreamsValid);
        subSceneOverride.getGeometry()->accept(visitor);
        fStreamsValid = true;
    }

    void updateMaterials(SubSceneOverride&   subSceneOverride,
                         MSubSceneContainer& container)
    {
        assert(fDagPath.isValid());
        if (!fDagPath.isValid()) return;

        // Early out if we can't see this instance.
        if (!fVisibility) {
            return;
        }

        // Update the sub-node diffuse color materials.
        UpdateDiffuseColorVisitor visitor(subSceneOverride, container, fSubNodeItems);
        visitor.setDontPrune(!fMaterialsValid);
        subSceneOverride.getGeometry()->accept(visitor);
        fMaterialsValid = true;
    }

    void destroyRenderItems(MSubSceneContainer& container)
    {
        // Destroy the bounding box render item for this instance.
        if (fBoundingBoxItem) {
            fBoundingBoxItem->removeFromContainer(container);
            fBoundingBoxItem.reset();
        }

        // Destroy the sub node render items.
        BOOST_FOREACH (SubNodeRenderItems::Ptr& subNodeItem, fSubNodeItems) {
            subNodeItem->destroyRenderItems(container);
        }
    }

private:
    MDagPath               fDagPath;
    bool                   fIsSelected;
    bool                   fVisibility;
    MMatrix                fMatrix;
    RenderItemWrapper::Ptr fBoundingBoxItem;
    ShaderInstancePtr      fBoundingBoxShader;
    SubNodeRenderItemList  fSubNodeItems;

    bool fVisibilityValid;
    bool fWorldMatrixValid;
    bool fStreamsValid;
    bool fMaterialsValid;
};


//==============================================================================
// CLASS SubSceneOverride
//==============================================================================

MPxSubSceneOverride* SubSceneOverride::creator(const MObject& object)
{
    return new SubSceneOverride(object);
}

void SubSceneOverride::clear()
{
    // Delete the buffers in the cache.
    BuffersCache::getInstance().clear();
}

MIndexBuffer* SubSceneOverride::lookup(const boost::shared_ptr<const IndexBuffer>& indices)
{
    // Find the corresponding index buffer.
    return BuffersCache::getInstance().lookup(indices);
}

MVertexBuffer* SubSceneOverride::lookup(const boost::shared_ptr<const VertexBuffer>& vertices)
{
    // Find the corresponding vertex buffer.
    return BuffersCache::getInstance().lookup(vertices);
}

SubSceneOverride::SubSceneOverride(const MObject& object)
    : MPxSubSceneOverride(object),
      fObject(object),
      fShapeNode(NULL),
      fUpdateRenderItemsRequired(true),
      fUpdateVisibilityRequired(true),
      fUpdateWorldMatrixRequired(true),
      fUpdateStreamsRequired(true),
      fUpdateMaterialsRequired(true),
      fOutOfViewFrustum(false),
      fOutOfViewFrustumUpdated(false),
      fWireOnShadedMode(DisplayPref::kWireframeOnShadedFull)
{
    // Extract the ShapeNode pointer.
    MFnDagNode dagNode(object);
    fShapeNode = (const ShapeNode*)dagNode.userNode();
    assert(fShapeNode);

    // Get all DAG paths.
    resetDagPaths();

    // Cache the non-networked plugs.
    fCastsShadowsPlug   = dagNode.findPlug("castsShadows", false);
    fReceiveShadowsPlug = dagNode.findPlug("receiveShadows", false);

    // Register callbacks
    MDagPath dagPath = MDagPath::getAPathTo(object);  // any path
    fInstanceAddedCallback = MDagMessage::addInstanceAddedDagPathCallback(
        dagPath, InstanceChangedCallback, this);
    fInstanceRemovedCallback = MDagMessage::addInstanceRemovedDagPathCallback(
        dagPath, InstanceChangedCallback, this);
    fWorldMatrixChangedCallback = MDagMessage::addWorldMatrixModifiedCallback(
        dagPath, WorldMatrixChangedCallback, this);
    registerNodeDirtyCallbacks();
    ModelCallbacks::getInstance().registerSubSceneOverride(fShapeNode, this);

    fUpdateTime = boost::date_time::microsec_clock<boost::posix_time::ptime>::local_time();
}

SubSceneOverride::~SubSceneOverride()
{
    // Deregister callbacks
    MMessage::removeCallback(fInstanceAddedCallback);
    MMessage::removeCallback(fInstanceRemovedCallback);
    MMessage::removeCallback(fWorldMatrixChangedCallback);
    MMessage::removeCallbacks(fNodeDirtyCallbacks);
    ModelCallbacks::getInstance().deregisterSubSceneOverride(fShapeNode);

    // Destroy render items.
    fInstanceRenderItems.clear();
    fHardwareInstanceManager.reset();
}

MHWRender::DrawAPI SubSceneOverride::supportedDrawAPIs() const
{
    // We support both OpenGL and DX11 in VP2.0!
    return MHWRender::kAllDevices;
}

bool SubSceneOverride::requiresUpdate(const MSubSceneContainer& container,
                                      const MFrameContext&      frameContext) const
{
    assert(fShapeNode);
    if (!fShapeNode) return false;

    MRenderer* renderer = MRenderer::theRenderer();
    if (!renderer) return false;

    // Cache the DAG paths for all instances.
    if (fInstanceDagPaths.length() == 0) {
        SubSceneOverride* nonConstThis = const_cast<SubSceneOverride*>(this);
        MDagPath::getAllPathsTo(fObject, nonConstThis->fInstanceDagPaths);
    }

    // Turn on/off hardware instancing.
    const bool hwInstancing = useHardwareInstancing();
    if ((hwInstancing && !fHardwareInstanceManager) ||
            (!hwInstancing && fHardwareInstanceManager)) {
        return true;
    }

    // Get the cached geometry and materials.
    SubNode::Ptr          geometry = fShapeNode->getCachedGeometry();
    MaterialGraphMap::Ptr material = fShapeNode->getCachedMaterial();

    // Check if the cached geometry or materials have been changed.
    if (geometry != fGeometry || material != fMaterial) {
        return true;
    }

    // Check if the Wireframe on Shaded mode has been changed.
    if (fWireOnShadedMode != DisplayPref::wireframeOnShadedMode()) {
        return true;
    }

    // Skip update if all instances are out of view frustum.
    // Only cull when we are using default lights.
    // Shadow map generation requires the update() even if the whole
    // DAG object is out of the camera view frustum.
    if (geometry && frameContext.getLightingMode() == MFrameContext::kLightDefault) {
        // The world view proj inv matrix.
        const MMatrix viewProjInv = frameContext.getMatrix(MFrameContext::kViewProjInverseMtx);

        // The bounding box in local DAG transform space.
        BoundingBoxVisitor visitor(MAnimControl::currentTime().as(MTime::kSeconds));
        geometry->accept(visitor);

        bool outOfViewFrustum = true;
        for (unsigned int i = 0; i < fInstanceDagPaths.length(); i++) {
            const MMatrix worldInv = fInstanceDagPaths[i].inclusiveMatrixInverse();

            // Test view frustum.
            Frustum frustum(viewProjInv * worldInv,
                renderer->drawAPIIsOpenGL() ? Frustum::kOpenGL : Frustum::kDirectX);

            if (frustum.test(visitor.boundingBox()) != Frustum::kOutside) {
                outOfViewFrustum = false;
                break;
            }
        }

        // We know all the render items are going to be culled so skip update them.
        if (outOfViewFrustum) {
            // It's important to call update() once after the shape is out of the view frustum.
            // This will make sure all render items are going to be culled.
            // If the render items are still going to be culled in this frame,
            // we can then skip calling update().
            if (fOutOfViewFrustum && fOutOfViewFrustumUpdated) {
                return false;
            }
        }

        SubSceneOverride* nonConstThis = const_cast<SubSceneOverride*>(this);
        nonConstThis->fOutOfViewFrustum        = outOfViewFrustum;
        nonConstThis->fOutOfViewFrustumUpdated = false;
    }

    // Check if we are loading geometry in background.
    CacheFileEntry::BackgroundReadingState readingState = fShapeNode->backgroundReadingState();
    if (readingState != fReadingState) {
        // Force an update when reading is done.
        return true;
    }
    if (readingState != CacheFileEntry::kReadingDone) {
        // Don't update too frequently.
        boost::posix_time::ptime currentTime =
            boost::date_time::microsec_clock<boost::posix_time::ptime>::local_time();
        boost::posix_time::time_duration interval = currentTime - fUpdateTime;
        if (interval.total_milliseconds() >= (int)(Config::backgroundReadingRefresh() / 2)) {
            return true;
        }
        return false;
    }

    return fUpdateRenderItemsRequired ||
            fUpdateVisibilityRequired ||
            fUpdateWorldMatrixRequired ||
            fUpdateStreamsRequired ||
            fUpdateMaterialsRequired;
}

void SubSceneOverride::update(MSubSceneContainer&  container,
                              const MFrameContext& frameContext)
{
    assert(fShapeNode);
    if (!fShapeNode) return;

    // Register node dirty callbacks if necessary.
    if (fNodeDirtyCallbacks.length() == 0) {
        registerNodeDirtyCallbacks();
    }

    // Update hardware instances.
    const bool hwInstancing = useHardwareInstancing();
    if (hwInstancing && !fHardwareInstanceManager) {
        // Turn on hardware instancing.
        dirtyRenderItems();     // force updating
        fHardwareInstanceManager.reset(new HardwareInstanceManager(*this));
    }
    else if (!hwInstancing && fHardwareInstanceManager) {
        // Turn off hardware instancing.
        fHardwareInstanceManager->resetInstances(container);
        fHardwareInstanceManager.reset();
    }

    // Shrink the buffer cache to make room for new buffers.
    // When the total size of the buffers is hitting the threshold,
    // buffers that are not used by any render items will be evicted.
    BuffersCache::getInstance().shrink();
    
    // Get the cached geometry and materials.
    SubNode::Ptr          geometry = fShapeNode->getCachedGeometry();
    MaterialGraphMap::Ptr material = fShapeNode->getCachedMaterial();

    // Remember the current time.
    fUpdateTime = boost::date_time::microsec_clock<boost::posix_time::ptime>::local_time();

    // Check if the cached geometry or materials have been changed.
    if (geometry != fGeometry || material != fMaterial) {
        // Set the cached geometry and materials.
        fGeometry = geometry;
        fMaterial = material;

        // Rebuild render items.
        fInstanceRenderItems.clear();
        container.clear();
        fHierarchyStat.reset();
        dirtyEverything();
    }

    // Check if we are loading geometry in background.
    CacheFileEntry::BackgroundReadingState readingState = fShapeNode->backgroundReadingState();
    if (readingState != fReadingState || readingState != CacheFileEntry::kReadingDone) {
        // Background reading has not finished. Update all render items.
        // (Remove bounding box render items and add shaded/wire render items.)
        fReadingState = readingState;
        dirtyEverything();
    }

    // Update the render items to match the Wireframe on Shaded mode.
    if (fWireOnShadedMode != DisplayPref::wireframeOnShadedMode()) {
        fWireOnShadedMode = DisplayPref::wireframeOnShadedMode();
        dirtyRenderItems();
    }

    // Current time in seconds
    fTimeInSeconds = MAnimControl::currentTime().as(MTime::kSeconds);

    // Update the render items.
    if (fUpdateRenderItemsRequired) {
        updateRenderItems(container, frameContext);
        fUpdateRenderItemsRequired = false;
    }

    // Update the visibility.
    if (fUpdateVisibilityRequired) {
        updateVisibility(container, frameContext);
        fUpdateVisibilityRequired = false;
    }

    // Update the world matrices.
    if (fUpdateWorldMatrixRequired) {
        updateWorldMatrix(container, frameContext);
        fUpdateWorldMatrixRequired = false;
    }

    // Update streams.
    if (fUpdateStreamsRequired) {
        updateStreams(container, frameContext);
        fUpdateStreamsRequired = false;
    }

    // Update materials.
    if (fUpdateMaterialsRequired) {
        updateMaterials(container, frameContext);
        fUpdateMaterialsRequired = false;
    }

    // Analysis the sub-node hierarchy so that we can prune it.
    if (!fHierarchyStat && fReadingState == CacheFileEntry::kReadingDone && fGeometry) {
        HierarchyStatVisitor visitor(fGeometry);
        fGeometry->accept(visitor);
        fHierarchyStat = visitor.getStat();

        // The geometry is fully loaded. Recompute the shadow map.
        MRenderer::setLightsAndShadowsDirty();
    }

    // Update hardware instancing.
    if (fHardwareInstanceManager) {
        fHardwareInstanceManager->processInstances(container);
    }

    // We have done update() when the shape is out of view frustum.
    if (fOutOfViewFrustum) {
        fOutOfViewFrustumUpdated = true;
    }
}

bool SubSceneOverride::getSelectionPath(const MHWRender::MRenderItem& renderItem, MDagPath& dagPath) const
{
	// The path to the instance is encoded in the render item name:
	MStringArray renderItemParts;
	renderItem.name().split(':', renderItemParts);

	if (renderItemParts.length() > 1 && renderItemParts[0].isUnsigned())
	{
		unsigned int pathIndex = renderItemParts[0].asUnsigned();
		if (pathIndex < fInstanceDagPaths.length())
		{
			 dagPath.set(fInstanceDagPaths[pathIndex]);
			 return true;
		}
	}
	
	return false;
}

void SubSceneOverride::dirtyEverything()
{
    dirtyRenderItems();
    dirtyVisibility();
    dirtyWorldMatrix();
    dirtyStreams();
    dirtyMaterials();
}

void SubSceneOverride::dirtyRenderItems()
{
    fUpdateRenderItemsRequired = true;
}

void SubSceneOverride::dirtyVisibility()
{
    fUpdateVisibilityRequired = true;
}

void SubSceneOverride::dirtyWorldMatrix()
{
    fUpdateWorldMatrixRequired = true;
}

void SubSceneOverride::dirtyStreams()
{
    fUpdateStreamsRequired = true;
}

void SubSceneOverride::dirtyMaterials()
{
    fUpdateMaterialsRequired = true;
}

void SubSceneOverride::resetDagPaths()
{
    fInstanceDagPaths.clear();
}

void SubSceneOverride::registerNodeDirtyCallbacks()
{
    assert(!fObject.isNull());
    if (fObject.isNull()) return;

    // Register callbacks to all parents.
    MDagPathArray paths;
    MDagPath::getAllPathsTo(fObject, paths);

    for (unsigned int i = 0; i < paths.length(); i++) {
        MDagPath dagPath = paths[i];

        // Register callbacks for this instance.
        while (dagPath.isValid() && dagPath.length() > 0) {
            MObject node = dagPath.node();

            // Monitor the parents and re-register callbacks.
            MCallbackId parentAddedCallback = MDagMessage::addParentAddedDagPathCallback(
                dagPath, ParentChangedCallback, this);
            MCallbackId parentRemovedCallback = MDagMessage::addParentRemovedDagPathCallback(
                dagPath, ParentChangedCallback, this);

            // Monitor parent display status changes.
            MCallbackId nodeDirtyCallback = MNodeMessage::addNodeDirtyPlugCallback(
                node, NodeDirtyCallback, this);

            // Add to array.
            fNodeDirtyCallbacks.append(parentAddedCallback);
            fNodeDirtyCallbacks.append(parentRemovedCallback);
            fNodeDirtyCallbacks.append(nodeDirtyCallback);

            dagPath.pop();
        }
    }
}

void SubSceneOverride::clearNodeDirtyCallbacks()
{
    if (fNodeDirtyCallbacks.length() > 0) {
        MMessage::removeCallbacks(fNodeDirtyCallbacks);
        fNodeDirtyCallbacks.clear();
    }
}

void SubSceneOverride::updateRenderItems(MHWRender::MSubSceneContainer&  container,
                                         const MHWRender::MFrameContext& frameContext)
{
    // Early out if the gpuCache node has no cached data.
    if (!fGeometry) {
        return;
    }

    // Match the number of the instances.
    unsigned int instanceCount = fInstanceDagPaths.length();
    if (instanceCount > fInstanceRenderItems.size()) {
        // Instance Added.
        unsigned int difference = (unsigned int)(instanceCount - fInstanceRenderItems.size());
        for (unsigned int i = 0; i < difference; i++) {
            fInstanceRenderItems.push_back(
                boost::make_shared<InstanceRenderItems>());
        }

        // Recompute shadow map.
        MRenderer::setLightsAndShadowsDirty();
    }
    else if (instanceCount < fInstanceRenderItems.size()) {
        // Instance Removed.
        unsigned int difference = (unsigned int)(fInstanceRenderItems.size() - instanceCount);
        for (unsigned int i = 0; i < difference; i++) {
            fInstanceRenderItems.back()->destroyRenderItems(container);
            fInstanceRenderItems.pop_back();
        }

        // Recompute shadow map.
        MRenderer::setLightsAndShadowsDirty();
    }
    assert(fInstanceDagPaths.length() == fInstanceRenderItems.size());

    // The MDagPath and MMatrix (world matrix) are the differences among instances.
    // We don't care the the instance number mapping. Just update the path and matrix.
    for (unsigned int i = 0; i < fInstanceRenderItems.size(); i++) {
        assert(fInstanceRenderItems[i]);
        // The name prefix for all render items of this instance
        // e.g. "1:" stands for the 2nd instance of the gpuCache node.
        const MString instancePrefix = MString("") + i + ":";

        // Update the bounding box render item.
        fInstanceRenderItems[i]->updateRenderItems(
            *this, container, fInstanceDagPaths[i], instancePrefix);
    }
}

void SubSceneOverride::updateVisibility(MHWRender::MSubSceneContainer&  container,
                                        const MHWRender::MFrameContext& frameContext)
{
    // Early out if the gpuCache node has no cached data.
    if (!fGeometry) {
        return;
    }

    // Update the visibility for all instances.
    BOOST_FOREACH (InstanceRenderItems::Ptr& instance, fInstanceRenderItems) {
        instance->updateVisibility(*this, container);
    }
}

void SubSceneOverride::updateWorldMatrix(MHWRender::MSubSceneContainer&  container,
                                         const MHWRender::MFrameContext& frameContext)
{
    // Early out if the gpuCache node has no cached data.
    if (!fGeometry) {
        return;
    }

    // Update the world matrix for all instances.
    BOOST_FOREACH (InstanceRenderItems::Ptr& instance, fInstanceRenderItems) {
        instance->updateWorldMatrix(*this, container);
    }
}

void SubSceneOverride::updateStreams(MHWRender::MSubSceneContainer&  container,
                                     const MHWRender::MFrameContext& frameContext)
{
    // Early out if the gpuCache node has no cached data.
    if (!fGeometry) {
        return;
    }

    // Update the streams for all instances.
    BOOST_FOREACH (InstanceRenderItems::Ptr& instance, fInstanceRenderItems) {
        instance->updateStreams(*this, container);
    }
}

void SubSceneOverride::updateMaterials(MHWRender::MSubSceneContainer&  container,
                                       const MHWRender::MFrameContext& frameContext)
{
    // Early out if the gpuCache node has no cached data.
    if (!fGeometry) {
        return;
    }

    // Update the diffuse color materials for all instances.
    BOOST_FOREACH (InstanceRenderItems::Ptr& instance, fInstanceRenderItems) {
        instance->updateMaterials(*this, container);
    }

    //Update the materials.
    ShaderInstanceCache::getInstance().updateCachedShadedShaders(fTimeInSeconds);
}

}
