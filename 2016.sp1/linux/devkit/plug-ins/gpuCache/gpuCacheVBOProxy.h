#ifndef _gpuCacheVBOProxy_h_
#define _gpuCacheVBOProxy_h_

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

#include "gpuCacheSample.h"

#include <maya/MGLdefinitions.h>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>


namespace GPUCache {

///////////////////////////////////////////////////////////////////////////////
//
// VBOBuffer
//
// This class represents a VBO buffer.
// All functions assume a valid OpenGL context.
//
////////////////////////////////////////////////////////////////////////////////

class VBOBuffer
{
public:

    /*----- typedefs and enumerations -----*/

    typedef ArrayBase::Key        Key;
    typedef ArrayBase::KeyHash    KeyHash;
    typedef ArrayBase::KeyEqualTo KeyEqualTo;

    enum BufferType {
        kIndexBufferType,
        kVertexBufferType,
        kFlippedNormalBufferType,
        kNbBufferType
    };
    
    /*----- static member functions -----*/
    
    // Allocate a VBO and upload the index buffer data to the VBO.
    //
    // A temporary VBO will be immediately discarded when no longer
    // referenced. This is mainly used when running low on video
    // memory and it is no longer possible to keep VBO loaded from
    // frame to frame.
    static boost::shared_ptr<const VBOBuffer> create(
        const boost::shared_ptr<const IndexBuffer>& buffer,
        const bool isTemporary = false);

    // Allocate a VBO and upload the vertex buffer data to the VBO
    static boost::shared_ptr<const VBOBuffer> create(
        const boost::shared_ptr<const VertexBuffer>& buffer,
        const bool isTemporary = false);

    // Allocate a VBO and initialize it by flipping the normals of the
    // passed VBO.
    static boost::shared_ptr<const VBOBuffer> createFlippedNormals(
        const boost::shared_ptr<const VertexBuffer>& buffer,
        const bool isTemporary = false);

    
    // Lookup to see if VBOBuffer for the given buffer already exists.
    static boost::shared_ptr<const VBOBuffer> lookup(
        const boost::shared_ptr<const IndexBuffer>& buffer);

    // Lookup to see if VBOBuffer for the given buffer already exists.
    static boost::shared_ptr<const VBOBuffer> lookup(
        const boost::shared_ptr<const VertexBuffer>& buffer);

    // Lookup to see if VBOBuffer for the given buffer already exists.
    static boost::shared_ptr<const VBOBuffer> lookupFlippedNormals(
        const boost::shared_ptr<const VertexBuffer>& buffer);

    
    // Total size of all the VBOs currently allocated
    static size_t nbAllocatedBytes();
    static size_t nbIndexAllocatedBytes();
    static size_t nbVertexAllocatedBytes();

    // Number of VBOs currently allocated
    static size_t nbAllocated();
    static size_t nbIndexAllocated();
    static size_t nbVertexAllocated();

    // Statistics about the VBO operations that have occurred since
    // the plug-in was loaded.
    static size_t nbUploaded();
    static size_t nbUploadedBytes();
    static size_t nbEvicted();
    static size_t nbEvictedBytes();

    // Flush all VBO buffers.
    static void clear();

    // Tell the registry that we are about to start drawing a new
    // frame. This is used as hint to mark some VBOs as potential
    // candidates for eviction. 
    static void nextRefresh();

    /*----- member functions -----*/

    // Free the VBO
    virtual ~VBOBuffer();

    // The buffer type.
    BufferType bufferType() const { return fBufferType; }

    // The key used to lookup the buffer in maps.
    const Key& key() const { return fKey; }

    // OpenGL VBO handle
    MGLuint name() const { return fVBOName; }

    
private:

    /*----- member functions -----*/
    
    GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_3;

    // Construct the VBO Buffer with a memory address and size 
    VBOBuffer(BufferType bufferType, const Key& key, const void* buffer);

    // Construct the VBO Buffer with a VBO handle and  size
    VBOBuffer(BufferType bufferType, const Key& key, MGLuint vboName);

    // Forbidden and not implemented.
    VBOBuffer(const VBOBuffer&);
    const VBOBuffer& operator=(const VBOBuffer&);

    /*----- static data members -----*/
    
    // This is used to limit the size of VBOs used in VP1.0 and in
    // VP2.0 when using the MPxDrawOverride API.  The display driver
    // will start to use system memory when the graphic card's video
    // memory is used up.
    static size_t fsTotalVBOSize;

    // This is used to limit the number of VBOs used in VP1.0 and in
    // VP2.0 when using the MPxDrawOverride API. Some display drivers
    // behaves badly when too many VBOs are allocated.
    static size_t fsNbAllocated;

    // Statistics.
    static size_t fsNbUploaded;
    static size_t fsNbUploadedBytes;
    static size_t fsNbEvicted;
    static size_t fsNbEvictedBytes;

    
    /*----- data members -----*/
    
    const BufferType        fBufferType;
    const ArrayBase::Key    fKey;
    MGLuint                 fVBOName;
};


///////////////////////////////////////////////////////////////////////////////
//
// VBOProxy
//
// Helper class used to draw geometry samples using VBOs if the amount
// of available graphic memory allows it, or using vertex arrays
// otherwise.
//
// When the VBOProxy class is created, it assumes that no vertex
// arrays are currently active and that no VBOs are currently
// bound. It then takes charge of managing and cache the OpenGL client
// state related to vertex arrays and VBOs. It attempts to minimize
// the amount of OpenGL state changes necessary to draw multiple
// samples.   
//
////////////////////////////////////////////////////////////////////////////////

class VBOProxy
{
public:

    /*----- typedefs and enumerations -----*/

    enum NormalsMode {
        kNoNormals,
        kFrontNormals,
        kBackNormals
    };

    enum UVsMode {
        kNoUVs,
        kUVs
    };

    enum VBOMode {
        kUseVBOIfPossible,
        kDontUseVBO
    };

    /*----- member functions -----*/

    // Initializes the VBOProxy and takes over the OpenGL client state.
    VBOProxy();

    // Unbounds any currently active vertex array or VBO and releases
    // control of the OpenGL client state.
    ~VBOProxy();

    // Draw the vertices of the given geometry sample
    void drawVertices(
        const boost::shared_ptr<const ShapeSample>& sample,
        const VBOMode vboMode = kUseVBOIfPossible);

    // Draw the wireframe of the given geometry sample
    void drawWireframe(
        const boost::shared_ptr<const ShapeSample>& sample,
        const VBOMode vboMode = kUseVBOIfPossible);

    // Draw the triangles of the given geometry sample
    void drawTriangles(
        const boost::shared_ptr<const ShapeSample>& sample,
        const size_t groupId,
        const NormalsMode normalsMode, const UVsMode uvsMode,
        const VBOMode vboMode = kUseVBOIfPossible);

    // Draw the bounding box of the given geometry sample
    void drawBoundingBox(
        const boost::shared_ptr<const ShapeSample>& sample,
        bool overrideShadedState = false);
    void drawBoundingBox(
        const MBoundingBox& boundingBox,
        bool overrideShadedState = false);

private:

    /*----- typedefs and enumerations -----*/

    typedef IndexBuffer::index_t index_t;
    
    enum BindingType {
        kPrimitives,
        kVertexArrays,
        kVBOs
    };
    

    /*----- member functions -----*/

    // Forbidden and not implemented.
    VBOProxy(const VBOProxy&);
    const VBOProxy& operator=(const VBOProxy&);
    
	// Try to upload/bind all of the following buffers the graphic
	// card. Returns an enum representing the graphic API that should
	// be used to perform the drawing.
    BindingType updateBuffers(
        const boost::shared_ptr<const IndexBuffer>&   indices,
        const boost::shared_ptr<const VertexBuffer>&  positions,
        const boost::shared_ptr<const VertexBuffer>&  normals,
        const boost::shared_ptr<const VertexBuffer>&  uvs,
        const bool                                    areNormalsFlipped,
        const VBOMode                                 vboMode,
        VertexBuffer::ReadInterfacePtr&               positionsRead,
        VertexBuffer::ReadInterfacePtr&               normalsRead,
        VertexBuffer::ReadInterfacePtr&               uvsRead
    );

    /*----- member functions -----*/

    // Currently bound buffers.
    boost::shared_ptr<const IndexBuffer>    fIndices;
    boost::shared_ptr<const VertexBuffer>   fPositions;
    boost::shared_ptr<const VertexBuffer>   fNormals;
    boost::shared_ptr<const VertexBuffer>   fUVs;

    // Currently bound VBOs.
    boost::shared_ptr<const VBOBuffer>      fVBOIndices;
    boost::shared_ptr<const VBOBuffer>      fVBOPositions;
    boost::shared_ptr<const VBOBuffer>      fVBONormals;
    boost::shared_ptr<const VBOBuffer>      fVBOUVs;
    bool                                    fAreNormalsFlipped;

    // Last binding type.
    BindingType                             fLastBindingType;
};


}

#endif
