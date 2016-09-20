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

#include "gpuCacheVBOProxy.h"

#include "gpuCacheConfig.h"
#include "gpuCacheGLFT.h"
#include "gpuCacheUnitBoundingBox.h"
#include "gpuCacheVramQuery.h"

#include <maya/MHardwareRenderer.h>
#include <maya/MGlobal.h>
#include <maya/MSceneMessage.h>

#include <limits>

#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/random/mersenne_twister.hpp>

#include <tbb/tbb_thread.h>
#include <tbb/mutex.h>

namespace {

using namespace GPUCache;

//==============================================================================
// LOCAL FUNCTIONS
//==============================================================================

//------------------------------------------------------------------------------
//
void assertNoVertexArray() 
{
    assert(!gGLFT->glIsEnabled(MGL_COLOR_ARRAY));
    assert(!gGLFT->glIsEnabled(MGL_EDGE_FLAG_ARRAY));
    assert(!gGLFT->glIsEnabled(MGL_FOG_COORDINATE_ARRAY_EXT));
    assert(!gGLFT->glIsEnabled(MGL_INDEX_ARRAY));
    assert(!gGLFT->glIsEnabled(MGL_NORMAL_ARRAY));
    assert(!gGLFT->glIsEnabled(MGL_SECONDARY_COLOR_ARRAY_EXT));
    assert(!gGLFT->glIsEnabled(MGL_TEXTURE_COORD_ARRAY));
    assert(!gGLFT->glIsEnabled(MGL_VERTEX_ARRAY));
}

//------------------------------------------------------------------------------
//
void assertNoVBOs()
{
#ifndef NDEBUG
    MGLint buffer;

    gGLFT->glGetIntegerv(MGL_ARRAY_BUFFER_BINDING_ARB, &buffer);
    assert(buffer == 0);
    
    gGLFT->glGetIntegerv(MGL_ELEMENT_ARRAY_BUFFER_BINDING_ARB, &buffer);
    assert(buffer == 0);
#endif
}

//------------------------------------------------------------------------------
//
void BeginTransformFeedback(MGLenum primitiveMode)
{
    if (gGLFT->extensionExists(kMGLext_NV_transform_feedback)) {
        gGLFT->glBeginTransformFeedbackNV(primitiveMode);
    }
    else if (gGLFT->extensionExists(kMGLext_EXT_transform_feedback)) {
        gGLFT->glBeginTransformFeedbackEXT(primitiveMode);
    }
}

//------------------------------------------------------------------------------
//
void EndTransformFeedback()
{
    if (gGLFT->extensionExists(kMGLext_NV_transform_feedback)) {
        gGLFT->glEndTransformFeedbackNV();
    }
    else if (gGLFT->extensionExists(kMGLext_EXT_transform_feedback)) {
        gGLFT->glEndTransformFeedbackEXT();
    }
}

//------------------------------------------------------------------------------
//
void EnableRasterizerDiscard()
{
    if (gGLFT->extensionExists(kMGLext_NV_transform_feedback)) {
        gGLFT->glEnable(MGL_RASTERIZER_DISCARD_NV);
    }
    else if (gGLFT->extensionExists(kMGLext_EXT_transform_feedback)) {
        gGLFT->glEnable(MGL_RASTERIZER_DISCARD_EXT);
    }
}

//------------------------------------------------------------------------------
//
void DisableRasterizerDiscard()
{
    if (gGLFT->extensionExists(kMGLext_NV_transform_feedback)) {
        gGLFT->glDisable(MGL_RASTERIZER_DISCARD_NV);
    }
    else if (gGLFT->extensionExists(kMGLext_EXT_transform_feedback)) {
        gGLFT->glDisable(MGL_RASTERIZER_DISCARD_EXT);
    }
}

//------------------------------------------------------------------------------
//
void BindBufferBase(MGLuint index, MGLuint buffer)
{
    if (gGLFT->extensionExists(kMGLext_NV_transform_feedback)) {
        gGLFT->glBindBufferBaseNV(MGL_TRANSFORM_FEEDBACK_BUFFER_NV, index, buffer);
    }
    else if (gGLFT->extensionExists(kMGLext_EXT_transform_feedback)) {
        gGLFT->glBindBufferBaseEXT(MGL_TRANSFORM_FEEDBACK_BUFFER_EXT, index, buffer);
    }
}

//==============================================================================
// LOCAL CLASSES
//==============================================================================


//==============================================================================
// CLASS FlipNormalsProgram
//==============================================================================

// Compute the flipped normals by transform feedback
class FlipNormalsProgram
{
public:
    static boost::shared_ptr<FlipNormalsProgram> getInstance();
    static void resetInstance();

    ~FlipNormalsProgram();
    void use();
    void beginQuery();
    void endQuery();

private:
    // Forbidden and not implemented.
    FlipNormalsProgram(const FlipNormalsProgram&);
    const FlipNormalsProgram& operator=(const FlipNormalsProgram&);

    FlipNormalsProgram();
    bool validate();

    MGLhandleARB fProgramObj;
    MGLuint      fQuery;
    static boost::shared_ptr<FlipNormalsProgram> fsFlipNormalsProgram;
    static const char* fsFlipNormalsProgramText;
};

const char* FlipNormalsProgram::fsFlipNormalsProgramText = "#version 120\n"
    "varying vec3 outNormal;\n"
    "void main()\n"
    "{\n"
    "    outNormal = -gl_Vertex.xyz;\n"
    "    gl_Position = gl_Vertex;\n"
    "}\n\n" ;
boost::shared_ptr<FlipNormalsProgram> FlipNormalsProgram::fsFlipNormalsProgram;
    
//------------------------------------------------------------------------------
//
boost::shared_ptr<FlipNormalsProgram> FlipNormalsProgram::getInstance()
{
    // return the transform feedback program cached in weak pointer
    if (fsFlipNormalsProgram) {
        return fsFlipNormalsProgram;
    }
    
    // check transform feedback extension
    if (!gGLFT->extensionExists(kMGLext_NV_transform_feedback) &&
        !gGLFT->extensionExists(kMGLext_EXT_transform_feedback)) {
        return boost::shared_ptr<FlipNormalsProgram>();;
    }
    
    // create a new transform feedback program
    boost::shared_ptr<FlipNormalsProgram> prog(new FlipNormalsProgram());
    if (!prog->validate()) {
        return boost::shared_ptr<FlipNormalsProgram>();
    }
    fsFlipNormalsProgram = prog;

    return fsFlipNormalsProgram;
}
    
//------------------------------------------------------------------------------
//
void FlipNormalsProgram::resetInstance()
{
    fsFlipNormalsProgram.reset();
}

//------------------------------------------------------------------------------
//
FlipNormalsProgram::~FlipNormalsProgram()
{
    // delete the program and query
    if (fProgramObj != 0) {
        gGLFT->glDeleteObjectARB(fProgramObj);
    }
    if (fQuery != 0) {
        gGLFT->glDeleteQueriesARB(1, &fQuery);
    }
}
    
//------------------------------------------------------------------------------
//
void FlipNormalsProgram::use()
{
    assert(fProgramObj != 0);
    gGLFT->glUseProgramObjectARB(fProgramObj);
}
    
//------------------------------------------------------------------------------
//
void FlipNormalsProgram::beginQuery()
{
    if (gGLFT->extensionExists(kMGLext_NV_transform_feedback)) {
        gGLFT->glBeginQueryARB(MGL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_NV, fQuery);
    }
    else if (gGLFT->extensionExists(kMGLext_EXT_transform_feedback)) {
        gGLFT->glBeginQueryARB(MGL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_EXT, fQuery);
    }
}
    
//------------------------------------------------------------------------------
//
void FlipNormalsProgram::endQuery()
{
    if (gGLFT->extensionExists(kMGLext_NV_transform_feedback)) {
        gGLFT->glEndQueryARB(MGL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_NV);
    }
    else if (gGLFT->extensionExists(kMGLext_EXT_transform_feedback)) {
        gGLFT->glEndQueryARB(MGL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_EXT);
    }
}
    
//------------------------------------------------------------------------------
//
FlipNormalsProgram::FlipNormalsProgram()
    : fProgramObj(0)
{
    bool  success = true;
    MGLint status  = MGL_TRUE;
    MGLhandleARB vertShaderObj = 0;
    MGLhandleARB prog = 0;
    MGLuint      query = 0;
    
    // create vertex shader
    vertShaderObj = gGLFT->glCreateShaderObjectARB(MGL_VERTEX_SHADER_ARB);
    assert(vertShaderObj != 0);
    if (vertShaderObj == 0) {
        success = false;
    }
    
    // compile vertex shader
    if (success) {
        gGLFT->glShaderSourceARB(vertShaderObj, 1, &fsFlipNormalsProgramText, NULL);
        gGLFT->glCompileShaderARB(vertShaderObj);
    
        // check the compile result
        gGLFT->glGetObjectParameterivARB(vertShaderObj, MGL_OBJECT_COMPILE_STATUS_ARB, &status);
        if (status != MGL_TRUE) {
            success = false;
    
            char buffer[4096];
            MGLsizei count = 0;
            gGLFT->glGetInfoLogARB(vertShaderObj, 4096, &count, buffer);
            printf("gpuCache: Failed to compile vertex shader.\nReason: %s\n", buffer);
        }
    }
    
    // create transform feedback program
    if (success) {
        prog = gGLFT->glCreateProgramObjectARB();
        assert(prog != 0);
        if (prog == 0) {
            success = false;
        }
    }
    
    // set up and link program
    if (success) {
        gGLFT->glAttachObjectARB(prog, vertShaderObj);
    
        if (gGLFT->extensionExists(kMGLext_NV_transform_feedback)) {
            // do nothing. set NV_transform_feedback state post-link
        }
        else if (gGLFT->extensionExists(kMGLext_EXT_transform_feedback)) {
            const char* outputs = "outNormal";
            gGLFT->glTransformFeedbackVaryingsEXT((MGLuint)(size_t)prog, 1, &outputs, MGL_SEPARATE_ATTRIBS_EXT);
        }
    
        gGLFT->glLinkProgramARB(prog);
    
        if (gGLFT->extensionExists(kMGLext_NV_transform_feedback)) {
            const MGLint output = gGLFT->glGetVaryingLocationNV((MGLuint)(size_t)prog, "outNormal");
            gGLFT->glTransformFeedbackVaryingsNV((MGLuint)(size_t)prog, 1, &output, MGL_SEPARATE_ATTRIBS_NV);
        }
    
        // check the link result
        gGLFT->glGetObjectParameterivARB(prog, MGL_OBJECT_LINK_STATUS_ARB, &status);
        if (status != MGL_TRUE) {
            success = false;
    
            char buffer[4096];
            MGLsizei count = 0;
            gGLFT->glGetInfoLogARB(prog, 4096, &count, buffer);
            printf("gpuCache: Failed to link program.\nReason: %s\n", buffer);
        }
    }
    
    // the vertex should be deleted along with the program
    if (vertShaderObj != 0) {
        gGLFT->glDeleteObjectARB(vertShaderObj);
    }
    
    // generate the query object
    gGLFT->glGenQueriesARB(1, &query);
    assert(query != 0);
    if (query == 0) {
        success = false;
    }
    
    if (success) {
        // success, we have the program to flip normals now
        fProgramObj = prog;
        fQuery      = query;
    }
    else {
        // failure, delete the program and query
        if (prog != 0) {
            gGLFT->glDeleteObjectARB(prog);
        }
        if (query != 0) {
            gGLFT->glDeleteQueriesARB(1, &query);
        }
    }
}
    
//------------------------------------------------------------------------------
//
bool FlipNormalsProgram::validate()
{
    return (fProgramObj != 0 && fQuery != 0);
}


//==========================================================================
// CLASS FlipNormals
//==========================================================================

class FlipNormals
{
public:
    FlipNormals(size_t numVerts, MGLuint normalName)
        : fNumVerts(numVerts), fNormalName(normalName)
    {}

    MGLuint compute();

private:
    // Forbidden and not implemented.
    FlipNormals(const FlipNormals&);
    const FlipNormals& operator=(const FlipNormals&);

    size_t  fNumVerts;
    MGLuint  fNormalName;
};

//------------------------------------------------------------------------------
//
MGLuint FlipNormals::compute()
{
    boost::shared_ptr<FlipNormalsProgram> prog = FlipNormalsProgram::getInstance();
    if (!prog) {
        return 0;
    }
    
    // generate an empty buffer for flipped normals
    MGLuint flippedNormalName = 0;
    gGLFT->glGenBuffersARB(1, &flippedNormalName);
    gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, flippedNormalName);
    gGLFT->glBufferDataARB(MGL_ARRAY_BUFFER_ARB, sizeof(float) * 3 * fNumVerts,
                           NULL, MGL_STATIC_DRAW_ARB);
    gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, 0);
    
    // disable rasterization
    EnableRasterizerDiscard();
    
    // use flip normal program
    prog->use();
    
    // bind empty flipped normals buffer (#0:outNormal)
    BindBufferBase(0, flippedNormalName);
    
    // bind normals buffer
    gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, fNormalName);
    
    // normals buffer is bounded to gl_Vertex
    gGLFT->glEnableClientState(MGL_VERTEX_ARRAY);
    gGLFT->glVertexPointer(3, MGL_FLOAT, 0, 0);
    
    // begin transform feedback
    prog->beginQuery();
    BeginTransformFeedback(MGL_POINTS);
    
    // push the normals
    gGLFT->glDrawArrays(MGL_POINTS, 0, MGLsizei(fNumVerts));
    
    // end transform feedback
    EndTransformFeedback();
    prog->endQuery();
    
    // clean up
    gGLFT->glDisableClientState(MGL_VERTEX_ARRAY);
    gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, 0);
    BindBufferBase(0, 0);
    gGLFT->glUseProgramObjectARB(0);
    DisableRasterizerDiscard();
    
    return flippedNormalName;
}

//==============================================================================
// CLASS VBOBufferRegistry
//==============================================================================

// VBO buffer registry is used to cache VBOs to keep them as much as
// possible on the graphic card from frame to frame. 
class VBOBufferRegistry {
public:
    /*----- typedefs and enumerations -----*/

    typedef VBOBuffer::BufferType BufferType;
    typedef VBOBuffer::Key        Key;
    typedef VBOBuffer::KeyHash    KeyHash;
    typedef VBOBuffer::KeyEqualTo KeyEqualTo;


    /*----- static member functions -----*/

    static void arrayDestructionCb(const Key& key);

    /*----- member functions -----*/
    
    VBOBufferRegistry();
    ~VBOBufferRegistry();

    // Returns the buffer matchng the given key if it exists. Returns
    // a null pointer otherwise.
    boost::shared_ptr<const VBOBuffer> find(
        const BufferType& bufferType, const Key& key)
    {
        {
            Map::const_iterator it = fActiveBuffers[bufferType].find(key);
            if (it != fActiveBuffers[bufferType].end()) {
                return it->second;
            }
        }

        {
            Map::iterator it = fPreviousFrameBuffers[bufferType].find(key);
            if (it != fPreviousFrameBuffers[bufferType].end()) {
                // The VBO was used while drawing the previous
                // frame. Moving it to the active list.
                boost::shared_ptr<const VBOBuffer> result = it->second;
                fActiveBuffers[bufferType].insert(*it);
                fPreviousFrameBuffers[bufferType].erase(it);
                return result;
            }
        }
        
        return boost::shared_ptr<const VBOBuffer>();
    }

    // Insert the given buffer in the registry.
    void insert(boost::shared_ptr<const VBOBuffer>& buffer)
    {
        fActiveBuffers[buffer->bufferType()].insert(
            std::make_pair(buffer->key(), buffer));
    }


    void erase(const Key& key)
    {
        for (int i = 0; i < VBOBuffer::kNbBufferType; ++i) {
            fActiveBuffers[i].erase(key);
            fPreviousFrameBuffers[i].erase(key);
        }
    }

    void delayedErase(const Key& key)
    {
        tbb::mutex::scoped_lock lock(fBuffersToDeleteMutex);
        fBuffersToDelete.insert(key);
    }
    
    void doDelayedErase()
    {
        if (!fBuffersToDelete.empty()) {
            tbb::mutex::scoped_lock lock(fBuffersToDeleteMutex);
            BOOST_FOREACH (const Key& key, fBuffersToDelete) {
                erase(key);
            }
            fBuffersToDelete.clear();
        }
    }

    // Randomly selects a random buffer from the previous frame and
    // erases it. Returns false if all allocated buffers are active. 
    bool eraseRandomBuffer() 
    {
        size_t nbCandidateBuffers = 0;
        for (int i = 0; i < VBOBuffer::kNbBufferType; ++i) {
            nbCandidateBuffers += fPreviousFrameBuffers[i].size();
        }
        if (nbCandidateBuffers == 0) {
            return false;
        }
        
        size_t candidate = fRandomEvictionIndex() % nbCandidateBuffers;
        for (int i = 0; i < VBOBuffer::kNbBufferType; ++i) {
            if (candidate < fPreviousFrameBuffers[i].size()) {
                fPreviousFrameBuffers[i].erase(
                    fPreviousFrameBuffers[i].begin());
                return true;
            }
            candidate -= fPreviousFrameBuffers[i].size();
            assert(candidate >= 0);
        }

        // Should never get here...
        assert(0);
        return false;
    }

    // Tell the registry that we are about to start drawing a new
    // frame. This is used as hint to mark some VBOs as potential
    // candidates for eviction. 
    void nextRefresh()
    {
        for (int i = 0; i < VBOBuffer::kNbBufferType; ++i) {
            fPreviousFrameBuffers[i].insert(
                fActiveBuffers[i].begin(), fActiveBuffers[i].end());
            fActiveBuffers[i].clear();
        }
    }

    // Flush all VBO buffers.
    void clear()
    {
        for (int i = 0; i < VBOBuffer::kNbBufferType; ++i) {
            fActiveBuffers[i].clear();
            fPreviousFrameBuffers[i].clear();
        }
        FlipNormalsProgram::resetInstance();

        assert(VBOBuffer::nbAllocatedBytes() == 0);
        assert(VBOBuffer::nbAllocated()      == 0);
    }

    // Reserve spaces by deleting VBOs
    bool reserve(size_t bytesNeeded, size_t buffersNeeded);

    // Total size of all the index VBOs currently allocated
    size_t nbIndexAllocatedBytes() const
    {
        size_t bytes = 0;
        BOOST_FOREACH(const Map::value_type& v,
                      fActiveBuffers[VBOBuffer::kIndexBufferType]) {
            bytes += v.second->key().fBytes;
        }
        BOOST_FOREACH(const Map::value_type& v,
                      fPreviousFrameBuffers[VBOBuffer::kIndexBufferType]) {
            bytes += v.second->key().fBytes;
        }
        return bytes;
    }

    // Total size of all the vertex VBOs currently allocated
    size_t nbVertexAllocatedBytes() const
    {
        size_t bytes = 0;
        BOOST_FOREACH(const Map::value_type& v,
                      fActiveBuffers[VBOBuffer::kVertexBufferType]) {
            bytes += v.second->key().fBytes;
        }
        BOOST_FOREACH(const Map::value_type& v,
                      fPreviousFrameBuffers[VBOBuffer::kVertexBufferType]) {
            bytes += v.second->key().fBytes;
        }
        BOOST_FOREACH(const Map::value_type& v,
                      fActiveBuffers[VBOBuffer::kFlippedNormalBufferType]) {
            bytes += v.second->key().fBytes;
        }
        BOOST_FOREACH(const Map::value_type& v,
                      fPreviousFrameBuffers[VBOBuffer::kFlippedNormalBufferType]) {
            bytes += v.second->key().fBytes;
        }
        return bytes;
    }

    // Number of index VBOs currently allocated
    size_t nbIndexAllocated() const
    {
        return
            fActiveBuffers[        VBOBuffer::kIndexBufferType ].size() +
            fPreviousFrameBuffers[ VBOBuffer::kIndexBufferType ].size();
    }

    // Number of vertex VBOs currently allocated
    size_t nbVertexAllocated() const
    {
        return
            fActiveBuffers[        VBOBuffer::kVertexBufferType        ].size() +
            fPreviousFrameBuffers[ VBOBuffer::kVertexBufferType        ].size() +
            fActiveBuffers[        VBOBuffer::kFlippedNormalBufferType ].size() +
            fPreviousFrameBuffers[ VBOBuffer::kFlippedNormalBufferType ].size();
    }
    

private:
    static void mayaExitCallback(void* clientData);
    static MCallbackId fsMayaExitCallbackId;

    typedef boost::unordered_map<
        Key,
        boost::shared_ptr<const VBOBuffer>,
        KeyHash,
        KeyEqualTo
        > Map; 
    Map fActiveBuffers[VBOBuffer::kNbBufferType];
    Map fPreviousFrameBuffers[VBOBuffer::kNbBufferType];

    // This allow deleting a VBO from a non-main thread.
    tbb::mutex fBuffersToDeleteMutex;
    boost::unordered_set<Key, KeyHash, KeyEqualTo> fBuffersToDelete;

    boost::mt19937_64 fRandomEvictionIndex;
};

MCallbackId VBOBufferRegistry::fsMayaExitCallbackId;

//------------------------------------------------------------------------------
//
VBOBufferRegistry& theBufferRegistry()
{
    static VBOBufferRegistry sRegistry;
    return sRegistry;
} 

//------------------------------------------------------------------------------
//
tbb::tbb_thread::id gsMainThreadId = tbb::this_tbb_thread::get_id();
void VBOBufferRegistry::arrayDestructionCb(const Key& key)
{
    // Array d'tor Callback might be called from a worker thread (background reading)
    if (tbb::this_tbb_thread::get_id() == gsMainThreadId) {
        theBufferRegistry().erase(key);
    }
    else {
        theBufferRegistry().delayedErase(key);
    }
}

//------------------------------------------------------------------------------
//
void VBOBufferRegistry::mayaExitCallback(void* clientData)
{
    assert(clientData);
    VBOBufferRegistry* registry = static_cast<VBOBufferRegistry*>(clientData);
    registry->clear();
    UnitBoundingBox::clear();
}

//------------------------------------------------------------------------------
//
VBOBufferRegistry::VBOBufferRegistry()
{
    // hook Maya exit callback to free VBOs
    fsMayaExitCallbackId = MSceneMessage::addCallback(
        MSceneMessage::kMayaExiting, mayaExitCallback, this);

    // Get rid of the associated VBO as soon as possible
    ArrayBase::registerDestructionCallback(arrayDestructionCb);

    // Set hashmap load factor to 0.7 to decrease collision
    for (int i = 0; i < VBOBuffer::kNbBufferType; ++i) {
        fActiveBuffers[i].max_load_factor(0.7f);
        fPreviousFrameBuffers[i].max_load_factor(0.7f);
    }
}

//------------------------------------------------------------------------------
//
VBOBufferRegistry::~VBOBufferRegistry()
{
    MSceneMessage::removeCallback(fsMayaExitCallbackId);
    ArrayBase::unregisterDestructionCallback(arrayDestructionCb);
    
    clear();
}

//------------------------------------------------------------------------------
//
bool VBOBufferRegistry::reserve(
    size_t bytesNeeded, size_t buffersNeeded)
{
    if (Config::maxVBOSize()  < bytesNeeded ||
        Config::maxVBOCount() < buffersNeeded)
    {
        return false;
    }

    size_t targetBytes  = Config::maxVBOSize()  - bytesNeeded;
    size_t targetNumber = Config::maxVBOCount() - buffersNeeded;

    while (
        VBOBuffer::nbAllocatedBytes() > targetBytes ||
        VBOBuffer::nbAllocated()      > targetNumber
    ) {
        // keep deleting VBOs
        if (!eraseRandomBuffer()) {
            // no more VBOs to delete, fail
            return false;
        }
    }
    
    return true;
}


} // unnamed namespace


namespace GPUCache {

//==============================================================================
// CLASS VBOBuffer
//==============================================================================

size_t VBOBuffer::fsTotalVBOSize = 0;
size_t VBOBuffer::fsNbAllocated  = 0;

size_t VBOBuffer::fsNbUploaded = 0;
size_t VBOBuffer::fsNbUploadedBytes = 0;
size_t VBOBuffer::fsNbEvicted = 0;
size_t VBOBuffer::fsNbEvictedBytes = 0;


// When switching from vp2 SubSceneOverride mode to the default viewport, we may
// want to convert the Maya buffers back into software buffers to free up the
// gpu memory.  However we don't have a clean way to delete the SubSceneOverride
// nodes from the vp2 scene since we can only update the MSubSceneContainer when
// vp2 renders.  Having switched viewport modes, vp2 doesn't render again.  This
// would take some extra gymnastics to avoid.  So the vp2 buffers will live on
// regardless.  In that case, we can just leave the data there.
//#define DOWNCONVERT_VP2SSO_TO_SOFTWARE

//------------------------------------------------------------------------------
//
boost::shared_ptr<const VBOBuffer>
VBOBuffer::create(
    const boost::shared_ptr<const IndexBuffer>& buffer,
    const bool isTemporary)
{
    boost::shared_ptr<const VBOBuffer> result =
        theBufferRegistry().find(kIndexBufferType, buffer->array()->key());

    if (!result) {
#ifdef DOWNCONVERT_SSO_TO_SOFTWARE
        boost::shared_ptr<ReadableArray<unsigned int> > readable = buffer->array()->getReadableArray();
        if (!buffer->array()->isReadable()) {
            // We are converting from viewport 2.0 SubSceneOverride non-readable arrays back to VBOBuffers.
            // So graft the SharedArray copy of the data back into the VertexBuffer.  This happens when changing
            // the viewport mode from vp2.0 to the default viewport and allows the vp2.0 buffer to be freed.
            boost::shared_ptr<Array<unsigned int> > array(readable);
            buffer->ReplaceArrayInstance(array);
        }
#else 
        IndexBuffer::ReadInterfacePtr readable = buffer->array()->getReadable();
#endif
        const BufferType bufferType = kIndexBufferType;
        const void*      bufferData = readable->get();
        result = boost::make_shared<VBOBuffer>(
            bufferType, buffer->array()->key(), bufferData);
        if (!isTemporary)
            theBufferRegistry().insert(result);
    }
    
    return result;
}

//------------------------------------------------------------------------------
//
boost::shared_ptr<const VBOBuffer>
VBOBuffer::create(
    const boost::shared_ptr<const VertexBuffer>& buffer,
    const bool isTemporary)
{
    boost::shared_ptr<const VBOBuffer> result =
        theBufferRegistry().find(kVertexBufferType, buffer->array()->key());

    if (!result) {
#ifdef DOWNCONVERT_SSO_TO_SOFTWARE
        boost::shared_ptr<ReadableArray<float> > readable = buffer->array()->getReadableArray();
        if (!buffer->array()->isReadable()) {
            // We are converting from viewport 2.0 SubSceneOverride non-readable arrays back to VBOBuffers.
            // So graft the SharedArray copy of the data back into the VertexBuffer.  This happens when changing
            // the viewport mode from vp2.0 to the default viewport and allows the vp2.0 buffer to be freed.
            boost::shared_ptr<Array<float> > array(readableArray);
            buffer->ReplaceArrayInstance(array);
        }
#else 
        VertexBuffer::ReadInterfacePtr readable = buffer->array()->getReadable();
#endif
        const BufferType bufferType = kVertexBufferType;
        const void*      bufferData = readable->get();
        result = boost::make_shared<VBOBuffer>(
            bufferType, buffer->array()->key(), bufferData);
        if (!isTemporary)
            theBufferRegistry().insert(result);
    }
    
    return result;
}

//------------------------------------------------------------------------------
//
boost::shared_ptr<const VBOBuffer>
VBOBuffer::createFlippedNormals(
    const boost::shared_ptr<const VertexBuffer>& buffer,
    const bool isTemporary)
{
    boost::shared_ptr<const VBOBuffer> flippedVBO =
        theBufferRegistry().find(kFlippedNormalBufferType, buffer->array()->key());

    if (!flippedVBO) {
        boost::shared_ptr<const VBOBuffer> unflippedVBO = create(buffer, isTemporary);

        MGLuint flippedNormalName =
            FlipNormals(buffer->numVerts(), unflippedVBO->name()).compute();
        
        if (flippedNormalName !=0 ) {
            const BufferType bufferType = kFlippedNormalBufferType;
            flippedVBO = boost::make_shared<VBOBuffer>(
                bufferType, buffer->array()->key(), flippedNormalName);
            if (!isTemporary)
                theBufferRegistry().insert(flippedVBO);
        }
    }
    
    return flippedVBO;
}

//------------------------------------------------------------------------------
//
boost::shared_ptr<const VBOBuffer>
VBOBuffer::lookup(
    const boost::shared_ptr<const IndexBuffer>& buffer)
{
    return theBufferRegistry().find(
        kIndexBufferType, buffer->array()->key());
}

//------------------------------------------------------------------------------
//
boost::shared_ptr<const VBOBuffer>
VBOBuffer::lookup(
    const boost::shared_ptr<const VertexBuffer>& buffer)
{
    return theBufferRegistry().find(
        kVertexBufferType, buffer->array()->key());
}

//------------------------------------------------------------------------------
//
boost::shared_ptr<const VBOBuffer>
VBOBuffer::lookupFlippedNormals(
    const boost::shared_ptr<const VertexBuffer>& buffer)
{
    return theBufferRegistry().find(
        kFlippedNormalBufferType, buffer->array()->key());
}

//------------------------------------------------------------------------------
//
size_t VBOBuffer::nbAllocatedBytes()
{
    return fsTotalVBOSize;
}

//------------------------------------------------------------------------------
//
size_t VBOBuffer::nbAllocated()
{
    return fsNbAllocated;
}

//------------------------------------------------------------------------------
//
size_t VBOBuffer::nbIndexAllocatedBytes()
{
    return theBufferRegistry().nbIndexAllocatedBytes();
}

//------------------------------------------------------------------------------
//
size_t VBOBuffer::nbIndexAllocated()
{
    return theBufferRegistry().nbIndexAllocated();
}

//------------------------------------------------------------------------------
//
size_t VBOBuffer::nbVertexAllocatedBytes()
{
    return theBufferRegistry().nbVertexAllocatedBytes();
}

//------------------------------------------------------------------------------
//
size_t VBOBuffer::nbVertexAllocated()
{
    return theBufferRegistry().nbVertexAllocated();
}

//------------------------------------------------------------------------------
//
size_t VBOBuffer::nbUploaded()
{
    return fsNbUploaded;
}

//------------------------------------------------------------------------------
//
size_t VBOBuffer::nbUploadedBytes()
{
    return fsNbUploadedBytes;
}

//------------------------------------------------------------------------------
//
size_t VBOBuffer::nbEvicted()
{
    return fsNbEvicted;
}

//------------------------------------------------------------------------------
//
size_t VBOBuffer::nbEvictedBytes()
{
    return fsNbEvictedBytes;
}

//------------------------------------------------------------------------------
//
void VBOBuffer::clear()
{
    return theBufferRegistry().clear();
}

//------------------------------------------------------------------------------
//
void VBOBuffer::nextRefresh()
{
    theBufferRegistry().nextRefresh();
}

//------------------------------------------------------------------------------
//
VBOBuffer::VBOBuffer(BufferType bufferType, const Key& key, const void* buffer)
    : fKey(key), fBufferType(bufferType), fVBOName(0)
{
    // Create an VBO and copy data to it.
    gGLFT->glGenBuffersARB(1, &fVBOName);
    assert(fVBOName != 0);
    gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, fVBOName);
    gGLFT->glBufferDataARB(MGL_ARRAY_BUFFER_ARB, fKey.fBytes, buffer, MGL_STATIC_DRAW_ARB);
    gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, 0);

    // accumulate VBO size counter
    fsTotalVBOSize += fKey.fBytes;
    ++fsNbAllocated;

    fsNbUploadedBytes += fKey.fBytes;
    ++fsNbUploaded;
}

//------------------------------------------------------------------------------
//
VBOBuffer::VBOBuffer(BufferType bufferType, const Key& key, MGLuint vboName)
    : fKey(key), fBufferType(bufferType), fVBOName(vboName)
{
    assert(fVBOName != 0);

    // accumulate VBO size counter
    fsTotalVBOSize += fKey.fBytes;
    ++fsNbAllocated;

    fsNbUploadedBytes += fKey.fBytes;
    ++fsNbUploaded;
}

//------------------------------------------------------------------------------
//
VBOBuffer::~VBOBuffer()
{
    // free the VBO
    assert(gGLFT->glIsBufferARB(fVBOName));
    gGLFT->glDeleteBuffersARB(1, &fVBOName);
    fVBOName = 0;

    // reduce VBO size counter
    fsTotalVBOSize -= fKey.fBytes;
    --fsNbAllocated;

    assert(fsTotalVBOSize >= 0);
    assert(fsNbAllocated  >= 0);

    fsNbEvictedBytes += fKey.fBytes;
    ++fsNbEvicted;
}


//==============================================================================
// CLASS VBOProxy
//==============================================================================

//------------------------------------------------------------------------------
//
VBOProxy::VBOProxy()
    : fLastBindingType(kPrimitives),
      fAreNormalsFlipped(false)
{
    // Just double check that no vertex array or VBO is in use when
    // the VBOProxy object takes control of the OpenGL client state.
    assertNoVertexArray();
    assertNoVBOs();

    // For extra safety...
    gGLFT->glTexCoord2f(0.0f, 0.0f);
}

//------------------------------------------------------------------------------
//
VBOProxy::~VBOProxy()
{
    switch (fLastBindingType) {
        case kPrimitives: {
            fIndices.reset();
            fPositions.reset();
            fNormals.reset();
            fUVs.reset();
        } break;

        case kVertexArrays: {
            {
                // We should always have indices
                assert(fIndices);
                fIndices.reset();
            }
            {
                // We should always have positions!
                assert(fPositions);
                
                gGLFT->glDisableClientState(MGL_VERTEX_ARRAY);
                fPositions.reset();
            }
            if (fNormals) {
                gGLFT->glDisableClientState(MGL_NORMAL_ARRAY);
                fNormals.reset();
            }
            if (fUVs) {
                gGLFT->glDisableClientState(MGL_TEXTURE_COORD_ARRAY);
                fUVs.reset();
            }
        } break;

        
        case kVBOs: {
            gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, 0);
            gGLFT->glBindBufferARB(MGL_ELEMENT_ARRAY_BUFFER_ARB, 0);
            
            {
                // We should always have indices
                assert(fIndices);
                assert(fVBOIndices);
                
                fIndices.reset();
                fVBOIndices.reset();
            }
            {
                // We should always have positions!
                assert(fPositions);
                assert(fVBOPositions);

                gGLFT->glDisableClientState(MGL_VERTEX_ARRAY);
                
                fPositions.reset();
                fVBOPositions.reset();
            }
            if (fVBONormals) {
                assert(fNormals);

                gGLFT->glDisableClientState(MGL_NORMAL_ARRAY);
                fNormals.reset();
                fVBONormals.reset();
            }
            if (fVBOUVs) {
                assert(fUVs);
                
                gGLFT->glDisableClientState(MGL_TEXTURE_COORD_ARRAY);
                fUVs.reset();
                fVBOUVs.reset();
            }
        } break;

        default: {
            assert(0);
        } break;
    }
    
    assertNoVertexArray();
    assertNoVBOs();
}

//------------------------------------------------------------------------------
//
VBOProxy::BindingType VBOProxy::updateBuffers(
    const boost::shared_ptr<const IndexBuffer>&  indices,
    const boost::shared_ptr<const VertexBuffer>& positions,
    const boost::shared_ptr<const VertexBuffer>& normals,
    const boost::shared_ptr<const VertexBuffer>& uvs,
    const bool                                   areNormalsFlipped,
    const VBOMode                                vboMode,
    VertexBuffer::ReadInterfacePtr&              positionsRead,
    VertexBuffer::ReadInterfacePtr&              normalsRead,
    VertexBuffer::ReadInterfacePtr&              uvsRead
)
{
    assert(indices);
    assert(positions);

    theBufferRegistry().doDelayedErase();
    
    boost::shared_ptr<const VBOBuffer> vboIndices;
    boost::shared_ptr<const VBOBuffer> vboPositions;
    boost::shared_ptr<const VBOBuffer> vboNormals;
    boost::shared_ptr<const VBOBuffer> vboUVs;

    // Attempt to use VBOs as much as possible since this is the
    // highest performance API.
    BindingType bindingType = kVBOs;

    if (vboMode == kDontUseVBO || positions->numVerts() < Config::minVertsForVBOs()) {
        // We only use VBOs for elements above a certain threshold to
        // avoid using too many VBOs.
        bindingType = kVertexArrays;
    }
    else {
        // Estimate the VBO buffer size to allocate. 
        size_t bytesNeeded = 0;
        size_t buffersNeeded = 0;

        if (indices == fIndices) {
            vboIndices = fVBOIndices;
        }
        else {
            vboIndices = theBufferRegistry().find(
                VBOBuffer::kIndexBufferType, indices->array()->key());
        
            if (!vboIndices) {
                bytesNeeded += indices->array()->bytes();
                ++buffersNeeded;
            }
        }

        if (positions == fPositions) {
            vboPositions = fVBOPositions;
        }
        else {
            vboPositions = theBufferRegistry().find(
                VBOBuffer::kVertexBufferType, positions->array()->key());
        
            if (!vboPositions) {
                bytesNeeded += positions->array()->bytes();
                ++buffersNeeded;
            }
        }

        if (normals == fNormals && areNormalsFlipped == fAreNormalsFlipped) {
            vboNormals = fVBONormals;
        }
        else {
            if (areNormalsFlipped) {
                vboNormals = theBufferRegistry().find(
                    VBOBuffer::kFlippedNormalBufferType, normals->array()->key());

                if (!vboNormals) {
                    bytesNeeded += normals->array()->bytes();
                    ++buffersNeeded;

                    // The unflipped normals buffer will also be
                    // necessary to compute the flipped buffer ones..
                    boost::shared_ptr<const VBOBuffer> unflippedNormals =
                        theBufferRegistry().find(
                            VBOBuffer::kVertexBufferType, normals->array()->key());
                    if (!unflippedNormals) {
                        bytesNeeded += normals->array()->bytes();
                        ++buffersNeeded;
                    }
                }
            }
            else {
                vboNormals = theBufferRegistry().find(
                    VBOBuffer::kVertexBufferType, normals->array()->key());
                if (!vboNormals) {
                    bytesNeeded += normals->array()->bytes();
                    ++buffersNeeded;
                }
            }
        }
    
        if (uvs == fUVs) {
            vboUVs = fVBOUVs;
        }
        else {
            vboUVs = theBufferRegistry().find(
                VBOBuffer::kVertexBufferType, uvs->array()->key());
        
            if (!vboUVs) {
                bytesNeeded += uvs->array()->bytes();
                ++buffersNeeded;
            }
        }

        // Stop using VBOs if we have exceeded the limit
        if (theBufferRegistry().reserve(bytesNeeded, buffersNeeded)) {
            if (!vboIndices) {
                vboIndices = VBOBuffer::create(indices);
            }
            if (!vboPositions) {
                vboPositions = VBOBuffer::create(positions);
            }
            if (normals && !vboNormals) {
                if (areNormalsFlipped) {
                    vboNormals = VBOBuffer::createFlippedNormals(normals);
                }
                else {
                    vboNormals = VBOBuffer::create(normals);
                }
            }
            if (uvs && !vboUVs) {
                vboUVs = VBOBuffer::create(uvs);
            }
        }
        else {
            if (Config::useVertexArrayWhenVRAMIsLow()) {
                // All VBOs are in use, no more space for new VBOs. Use
                // vertex arrays instead.
                bindingType = kVertexArrays;
        
                vboIndices.reset();
                vboPositions.reset();
                vboNormals.reset();
                vboUVs.reset();
            }
            else {
                // There is not enough VRAM available to keep VBOs around
                // from frame to frame. Draw using temporary VBOs instead.
                if (!vboIndices) {
                    vboIndices = VBOBuffer::create(indices, true);
                }
                if (!vboPositions) {
                    vboPositions = VBOBuffer::create(positions, true);
                }
                if (normals && !vboNormals) {
                    if (areNormalsFlipped) {
                        vboNormals = VBOBuffer::createFlippedNormals(normals, true);
                    }
                    else {
                        vboNormals = VBOBuffer::create(normals, true);
                    }
                }
                if (uvs && !vboUVs) {
                    vboUVs = VBOBuffer::create(uvs, true);
                }
            }
        }
    }

    // Extra checks to see if vertex arrays can be safely used.
    if (bindingType == kVertexArrays) {
        if (Config::useGLPrimitivesInsteadOfVA()) {
            // For some reason, using vertex arrays on Windows/nVidia
            // Quadro gfx leads to memory corruption. Using primitive
            // OpenGL calls instead as a workaround.
            bindingType = kPrimitives;
        }
        else if (areNormalsFlipped) {
            // FIXME: We should probably implement a faster way to
            // flip normals than reverting to glBegin()/glEnd()
            // primitives...
            bindingType = kPrimitives;
        }
    }
     

    switch (fLastBindingType) {
        case kPrimitives: 
            switch (bindingType) {
                case kPrimitives:
                    break;

                case kVertexArrays:
                    {
                        gGLFT->glEnableClientState(MGL_VERTEX_ARRAY);
                        positionsRead = positions->readableInterface();
                        gGLFT->glVertexPointer(3, MGL_FLOAT, 0, positionsRead->get());
                    }
                    
                    if (normals) {
                        gGLFT->glEnableClientState(MGL_NORMAL_ARRAY);
                        normalsRead = normals->readableInterface();
                        gGLFT->glNormalPointer(MGL_FLOAT, 0, normalsRead->get());
                    }

                    if (uvs) {
                        gGLFT->glEnableClientState(MGL_TEXTURE_COORD_ARRAY);
                        uvsRead = uvs->readableInterface();
                        gGLFT->glTexCoordPointer(2, MGL_FLOAT, 0, uvsRead->get());
                    }
                    break;

                case kVBOs:
                    gGLFT->glEnableClientState(MGL_VERTEX_ARRAY);
                    gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, vboPositions->name());
                    gGLFT->glVertexPointer(3, MGL_FLOAT, 0, 0);
                    
                    if (vboNormals) {
                        gGLFT->glEnableClientState(MGL_NORMAL_ARRAY);
                        gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, vboNormals->name());
                        gGLFT->glNormalPointer(MGL_FLOAT, 0, 0);
                    }

                    if (vboUVs) {
                        gGLFT->glEnableClientState(MGL_TEXTURE_COORD_ARRAY);
                        gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, vboUVs->name());
                        gGLFT->glTexCoordPointer(2, MGL_FLOAT, 0, 0);
                    }

                    gGLFT->glBindBufferARB(MGL_ELEMENT_ARRAY_BUFFER_ARB, vboIndices->name());
                    break;
            }
            break;

        case kVertexArrays: 
            switch (bindingType) {
                case kPrimitives:
                    gGLFT->glDisableClientState(MGL_VERTEX_ARRAY);
                    
                    if (fNormals) {
                        gGLFT->glDisableClientState(MGL_NORMAL_ARRAY);
                    }

                    if (fUVs) {
                        gGLFT->glDisableClientState(MGL_TEXTURE_COORD_ARRAY);
                    }
                    break;

                case kVertexArrays:
                    if (positions != fPositions || !positions->array()->isReadable()) {
                        positionsRead = positions->readableInterface();
                        gGLFT->glVertexPointer(3, MGL_FLOAT, 0, positionsRead->get());
                    }
                    
                    if (normals) {
                        if (!fNormals)
                            gGLFT->glEnableClientState(MGL_NORMAL_ARRAY);
                        if (normals != fNormals || !normals->array()->isReadable()) {
                            normalsRead = normals->readableInterface();
                            gGLFT->glNormalPointer(MGL_FLOAT, 0, normalsRead->get());
                        }
                    }
                    else if (fNormals) {
                        gGLFT->glDisableClientState(MGL_NORMAL_ARRAY);
                    }

                    if (uvs) {
                        if (!fUVs)
                            gGLFT->glEnableClientState(MGL_TEXTURE_COORD_ARRAY);
                        if (uvs != fUVs || !uvs->array()->isReadable()) {
                            uvsRead = uvs->readableInterface();
                            gGLFT->glTexCoordPointer(2, MGL_FLOAT, 0, uvsRead->get());
                        }
                    }
                    else if (fUVs) {
                        gGLFT->glDisableClientState(MGL_TEXTURE_COORD_ARRAY);
                    }
                    break;

                case kVBOs:
                    gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, vboPositions->name());
                    gGLFT->glVertexPointer(3, MGL_FLOAT, 0, 0);
                    
                    if (vboNormals) {
                        if (!fNormals)
                            gGLFT->glEnableClientState(MGL_NORMAL_ARRAY);
                        gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, vboNormals->name());
                        gGLFT->glNormalPointer(MGL_FLOAT, 0, 0);
                    }

                    if (vboUVs) {
                        if (!fUVs)
                            gGLFT->glEnableClientState(MGL_TEXTURE_COORD_ARRAY);
                        gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, vboUVs->name());
                        gGLFT->glTexCoordPointer(2, MGL_FLOAT, 0, 0);
                    }

                    gGLFT->glBindBufferARB(MGL_ELEMENT_ARRAY_BUFFER_ARB, vboIndices->name());
                    break;
            }
            break;

        case kVBOs: 
            switch (bindingType) {
                case kPrimitives:
                    gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, 0);
                    gGLFT->glBindBufferARB(MGL_ELEMENT_ARRAY_BUFFER_ARB, 0);

                    gGLFT->glDisableClientState(MGL_VERTEX_ARRAY);
                    
                    if (fVBONormals) {
                        gGLFT->glDisableClientState(MGL_NORMAL_ARRAY);
                    }

                    if (fVBOUVs) {
                        gGLFT->glDisableClientState(MGL_TEXTURE_COORD_ARRAY);
                    }
                    break;

                case kVertexArrays:
                    gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, 0);
                    gGLFT->glBindBufferARB(MGL_ELEMENT_ARRAY_BUFFER_ARB, 0);

                    {
                        positionsRead = positions->readableInterface();
                        gGLFT->glVertexPointer(3, MGL_FLOAT, 0, positionsRead->get());
                    }
                    
                    if (normals) {
                        if (!fVBONormals)
                            gGLFT->glEnableClientState(MGL_NORMAL_ARRAY);
                        normalsRead = normals->readableInterface();
                        gGLFT->glNormalPointer(MGL_FLOAT, 0, normalsRead->get());
                    }
                    else if (fVBONormals) {
                        gGLFT->glDisableClientState(MGL_NORMAL_ARRAY);
                    }

                    if (uvs) {
                        if (!fVBOUVs)
                            gGLFT->glEnableClientState(MGL_TEXTURE_COORD_ARRAY);
                        uvsRead = uvs->readableInterface();
                        gGLFT->glTexCoordPointer(2, MGL_FLOAT, 0, uvsRead->get());
                    }
                    else if (fUVs) {
                        gGLFT->glDisableClientState(MGL_TEXTURE_COORD_ARRAY);
                    }
                    break;

                case kVBOs:
                    if (vboPositions != fVBOPositions) {
                        gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, vboPositions->name());
                        gGLFT->glVertexPointer(3, MGL_FLOAT, 0, 0);
                    }
                    
                    if (vboNormals) {
                        if (!fVBONormals)
                            gGLFT->glEnableClientState(MGL_NORMAL_ARRAY);
                        if (vboNormals != fVBONormals) {
                            gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, vboNormals->name());
                            gGLFT->glNormalPointer(MGL_FLOAT, 0, 0);
                        }
                    }
                    else if (fVBONormals) {
                        gGLFT->glDisableClientState(MGL_NORMAL_ARRAY);
                        gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, 0);
                        gGLFT->glNormalPointer(MGL_FLOAT, 0, 0);
                    }

                    if (vboUVs) {
                        if (!fVBOUVs)
                            gGLFT->glEnableClientState(MGL_TEXTURE_COORD_ARRAY);
                        if (vboUVs != fVBOUVs) {
                            gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, vboUVs->name());
                            gGLFT->glTexCoordPointer(2, MGL_FLOAT, 0, 0);
                        }
                    }
                    else if (fUVs) {
                        gGLFT->glDisableClientState(MGL_TEXTURE_COORD_ARRAY);
                        gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, 0);
                        gGLFT->glTexCoordPointer(2, MGL_FLOAT, 0, 0);
                    }

                    gGLFT->glBindBufferARB(MGL_ELEMENT_ARRAY_BUFFER_ARB, vboIndices->name());
                    break;
            }
            break;
    }

    fIndices           = indices;
    fPositions         = positions;
    fNormals           = normals;
    fUVs               = uvs;
    
    fVBOIndices        = vboIndices;
    fVBOPositions      = vboPositions;
    fVBONormals        = vboNormals;
    fVBOUVs            = vboUVs;
    fAreNormalsFlipped = areNormalsFlipped;

    fLastBindingType = bindingType;
    return fLastBindingType;
}

//------------------------------------------------------------------------------
//
void VBOProxy::drawVertices(
    const boost::shared_ptr<const ShapeSample>& sample,
    const VBOMode                               vboMode)
{
    // We may need to read from the buffers in this function and also in
    // updateBuffers.  So to avoid possibly converting the buffers twice, we
    // put the ReadInterfacePtrs in this scope and share them with updateBuffers.
    IndexBuffer::ReadInterfacePtr indicesRead;
    VertexBuffer::ReadInterfacePtr positionsRead;
    VertexBuffer::ReadInterfacePtr normalsRead;
    VertexBuffer::ReadInterfacePtr uvsRead;

    // This draws some vertices multiple times. Unfortunately, there
    // is no easy way to draw each vertices only once without
    // generating a sorted list of the vertices. This is therefore
    // probably the most efficient way to draw the wireframe vertices
    // on the fly.
    //
    // A more efficient solution would be to store an index array of
    // the wireframe vertices in the ShapeSample object. We might
    // implement this at a later time if it proved necessary.
    BindingType bindingType = updateBuffers(
        sample->wireVertIndices(),
        sample->positions(),
        boost::shared_ptr<VertexBuffer>(),
        boost::shared_ptr<VertexBuffer>(),
        false,
        vboMode,
        positionsRead,
        normalsRead,
        uvsRead
    );

    switch (bindingType) {
        case kPrimitives:
            {
                const size_t numPoints = fIndices->numIndices();
                indicesRead = fIndices->readableInterface();
                const unsigned int* const indices = indicesRead->get();
                if (!positionsRead)
                    positionsRead = fPositions->readableInterface();
                const float* const verts = positionsRead->get();

                gGLFT->glBegin(MGL_POINTS);
                for (size_t i = 0; i < numPoints; ++i) {
                    gGLFT->glVertex3f(verts[indices[i]*3 + 0],
                                      verts[indices[i]*3 + 1],
                                      verts[indices[i]*3 + 2]);
                }
                gGLFT->glEnd();
            }
            break;

        case kVertexArrays:
            {
                indicesRead = fIndices->readableInterface();
                gGLFT->glDrawElements(
                    MGL_POINTS,
                    MGLsizei(fIndices->numIndices()),
                    MGL_UNSIGNED_INT,
                    indicesRead->get());
            }
            break;

        case kVBOs:
            gGLFT->glDrawElements(
                MGL_POINTS,
                MGLsizei(fIndices->numIndices()),
                MGL_UNSIGNED_INT,
                (void*)(fIndices->beginIdx() * sizeof(index_t)));
            break;
    }

}

//------------------------------------------------------------------------------
//
void VBOProxy::drawWireframe(
    const boost::shared_ptr<const ShapeSample>& sample,
    const VBOMode                               vboMode)
{
    // We may need to read from the buffers in this function and also in
    // updateBuffers.  So to avoid possibly converting the buffers twice, we
    // put the ReadInterfacePtrs in this scope and share them with updateBuffers.
    IndexBuffer::ReadInterfacePtr indicesRead;
    VertexBuffer::ReadInterfacePtr positionsRead;
    VertexBuffer::ReadInterfacePtr normalsRead;
    VertexBuffer::ReadInterfacePtr uvsRead;

    BindingType bindingType = updateBuffers(
        sample->wireVertIndices(),
        sample->positions(),
        boost::shared_ptr<VertexBuffer>(),
        boost::shared_ptr<VertexBuffer>(),
        false,
        vboMode,
        positionsRead,
        normalsRead,
        uvsRead
    );

    switch (bindingType) {
        case kPrimitives:
            {
                
                const size_t numWires = fIndices->numIndices() / 2;
                indicesRead = fIndices->readableInterface();
                const unsigned int* const indices = indicesRead->get();
                if (!positionsRead)
                    positionsRead = fPositions->readableInterface();
                const float* const verts = positionsRead->get();

                gGLFT->glBegin(MGL_LINES);
                for (size_t i = 0; i < numWires; ++i) {
                    gGLFT->glVertex3f(verts[indices[2*i + 0]*3 + 0],
                                      verts[indices[2*i + 0]*3 + 1],
                                      verts[indices[2*i + 0]*3 + 2]);
                    gGLFT->glVertex3f(verts[indices[2*i + 1]*3 + 0],
                                      verts[indices[2*i + 1]*3 + 1],
                                      verts[indices[2*i + 1]*3 + 2]);
                }
                gGLFT->glEnd();
            }
            break;

        case kVertexArrays:
            {
                indicesRead = fIndices->readableInterface();
                gGLFT->glDrawElements(
                    MGL_LINES,
                    MGLsizei(fIndices->numIndices()),
                    MGL_UNSIGNED_INT,
                    indicesRead->get());
            }
            break;

        case kVBOs:
            gGLFT->glDrawElements(
                MGL_LINES,
                MGLsizei(fIndices->numIndices()),
                MGL_UNSIGNED_INT,
                (void*)(fIndices->beginIdx() * sizeof(index_t)));
            break;
    }
}

//------------------------------------------------------------------------------
//
void VBOProxy::drawTriangles(
    const boost::shared_ptr<const ShapeSample>& sample,
    const size_t                                groupId,
    const NormalsMode                           normalsMode,
    const UVsMode                               uvsMode,
    const VBOMode                               vboMode)
{
    // We may need to read from the buffers in this function and also in
    // updateBuffers.  So to avoid possibly converting the buffers twice, we
    // put the ReadInterfacePtrs in this scope and share them with updateBuffers.
    IndexBuffer::ReadInterfacePtr indicesRead;
    VertexBuffer::ReadInterfacePtr positionsRead;
    VertexBuffer::ReadInterfacePtr normalsRead;
    VertexBuffer::ReadInterfacePtr uvsRead;

    BindingType bindingType = updateBuffers(
        sample->triangleVertIndices(groupId),
        sample->positions(),
        normalsMode != kNoNormals ? sample->normals() : boost::shared_ptr<VertexBuffer>(),
        uvsMode != kNoUVs         ? sample->uvs()     : boost::shared_ptr<VertexBuffer>(),
        normalsMode == kBackNormals,
        vboMode,
        positionsRead,
        normalsRead,
        uvsRead
    );

    switch (bindingType) {
        case kPrimitives:
            {
                const size_t numTriangles = fIndices->numIndices() / 3;
                indicesRead = fIndices->readableInterface();
                const unsigned int* const indices = indicesRead->get();
                if (!positionsRead)
                    positionsRead = fPositions->readableInterface();
                const float* const verts = positionsRead->get();
                const float* norms = NULL;
                if (fNormals) {
                    if (!normalsRead)
                        normalsRead = fNormals->readableInterface();
                    norms = normalsRead->get();
                }
                const float* uvs = NULL;
                if (fUVs) {
                    if (!uvsRead)
                        uvsRead = fUVs->readableInterface();
                    uvs = uvsRead->get();
                }
                const bool areNormalsFlipped = fAreNormalsFlipped;
            
                gGLFT->glBegin(MGL_TRIANGLES);
                for (size_t i = 0; i < numTriangles; ++i) {
                    const index_t idx0 = indices[3*i + 0]*3;
                    const index_t idx1 = indices[3*i + 1]*3;
                    const index_t idx2 = indices[3*i + 2]*3;
                
                    // Index 0
                    if (norms) {
                        if (areNormalsFlipped) {
                            gGLFT->glNormal3f(
                                -norms[idx0 + 0], -norms[idx0 + 1], -norms[idx0 + 2]);
                        }
                        else {
                            gGLFT->glNormal3f(
                                +norms[idx0 + 0], +norms[idx0 + 1], +norms[idx0 + 2]);
                        }
                    }
                    if (uvs) {
                        gGLFT->glTexCoord2f(
                            uvs[idx0 + 0], uvs[idx0 + 1]);
                    }
                    gGLFT->glVertex3f(
                        verts[idx0 + 0], verts[idx0 + 1], verts[idx0 + 2]);

                    // Index 1
                    if (norms) {
                        if (areNormalsFlipped) {
                            gGLFT->glNormal3f(
                                -norms[idx1 + 0], -norms[idx1 + 1], -norms[idx1 + 2]);
                        }
                        else {
                            gGLFT->glNormal3f(
                                +norms[idx1 + 0], +norms[idx1 + 1], +norms[idx1 + 2]);
                        }
                    }
                    if (uvs) {
                        gGLFT->glTexCoord2f(
                            uvs[idx1 + 0], uvs[idx1 + 1]);
                    }
                    gGLFT->glVertex3f(
                        verts[idx1 + 0], verts[idx1 + 1], verts[idx1 + 2]);

                    // Index 2
                    if (norms) {
                        if (areNormalsFlipped) {
                            gGLFT->glNormal3f(
                                -norms[idx2 + 0], -norms[idx2 + 1], -norms[idx2 + 2]);
                        }
                        else {
                            gGLFT->glNormal3f(
                                +norms[idx2 + 0], +norms[idx2 + 1], +norms[idx2 + 2]);
                        }
                    }
                    if (uvs) {
                        gGLFT->glTexCoord2f(
                            uvs[idx2 + 0], uvs[idx2 + 1]);
                    }
                    gGLFT->glVertex3f(
                        verts[idx2 + 0], verts[idx2 + 1], verts[idx2 + 2]);
                }
                gGLFT->glEnd();

                // For safety...
                gGLFT->glTexCoord2f(0.0f, 0.0f);
            }
            break;

        case kVertexArrays:
            {
                indicesRead = fIndices->readableInterface();
                gGLFT->glDrawElements(
                    MGL_TRIANGLES,
                    MGLsizei(fIndices->numIndices()),
                    MGL_UNSIGNED_INT,
                    indicesRead->get());
            }
            break;

        case kVBOs:
            gGLFT->glDrawElements(
                MGL_TRIANGLES,
                MGLsizei(fIndices->numIndices()),
                MGL_UNSIGNED_INT,
                (void*)(fIndices->beginIdx() * sizeof(index_t)));
            break;
    }
}

//------------------------------------------------------------------------------
//
void VBOProxy::drawBoundingBox(
    const boost::shared_ptr<const ShapeSample>& sample, 
    bool overrideShadedState  /* = false */
)
{
    drawBoundingBox(sample->boundingBox(), overrideShadedState);
}

//------------------------------------------------------------------------------
//
void VBOProxy::drawBoundingBox(
    const MBoundingBox& boundingBox,
    bool overrideShadedState  /* = false */
)
{
    // We may need to read from the buffers in this function and also in
    // updateBuffers.  So to avoid possibly converting the buffers twice, we
    // put the ReadInterfacePtrs in this scope and share them with updateBuffers.
    IndexBuffer::ReadInterfacePtr indicesRead;
    VertexBuffer::ReadInterfacePtr positionsRead;
    VertexBuffer::ReadInterfacePtr normalsRead;
    VertexBuffer::ReadInterfacePtr uvsRead;

    BindingType bindingType = updateBuffers(
        UnitBoundingBox::indices(),
        UnitBoundingBox::positions(),
        boost::shared_ptr<VertexBuffer>(),
        boost::shared_ptr<VertexBuffer>(),
        false,
        kDontUseVBO,
        positionsRead,
        normalsRead,
        uvsRead
    );

    // A little hack. We have to draw bounding box in shaded mode.
    // Override OpenGL Shaded state for bounding box drawing
    bool lightingWasOn = false, depthMaskWasOn = false, stippleWasOn = false;
    float prevColor[4];
    if (overrideShadedState) {
        // Turn off lighting
        lightingWasOn = gGLFT->glIsEnabled(MGL_LIGHTING) == MGL_TRUE;
        if (lightingWasOn) {
            gGLFT->glDisable(MGL_LIGHTING);
        }

        // Turn on depth write
        MGLboolean depthWriteMask = MGL_TRUE;
        gGLFT->glGetBooleanv(MGL_DEPTH_WRITEMASK, &depthWriteMask);
        depthMaskWasOn = depthWriteMask == MGL_TRUE;
        if (!depthMaskWasOn) {
            gGLFT->glDepthMask(MGL_TRUE);
        }

        // Turn on line stipple
        stippleWasOn = gGLFT->glIsEnabled(MGL_LINE_STIPPLE) == MGL_TRUE;
        if (!stippleWasOn) {
            gGLFT->glEnable(MGL_LINE_STIPPLE);
        }

        // Set default wireframe color
        gGLFT->glGetFloatv(MGL_CURRENT_COLOR, prevColor);
        gGLFT->glColor4f(0.0f, 0.016f, 0.376f, 1.0f);
    }

    switch (bindingType) {
    case kPrimitives:
        {
            // We are using primitives
            float w = (float) boundingBox.width();
            float h = (float) boundingBox.height();
            float d = (float) boundingBox.depth();

            // Below we just two sides and then connect
            // the edges together
            MPoint minVertex = boundingBox.min();

            // Draw first side
            gGLFT->glBegin( MGL_LINE_LOOP );
                MPoint vertex = minVertex;
                gGLFT->glVertex3f( (float)vertex[0],   (float)vertex[1],   (float)vertex[2] );
                gGLFT->glVertex3f( (float)vertex[0]+w, (float)vertex[1],   (float)vertex[2] );
                gGLFT->glVertex3f( (float)vertex[0]+w, (float)vertex[1]+h, (float)vertex[2] );
                gGLFT->glVertex3f( (float)vertex[0],   (float)vertex[1]+h, (float)vertex[2] );
                gGLFT->glVertex3f( (float)vertex[0],   (float)vertex[1],   (float)vertex[2] );
            gGLFT->glEnd();

            // Draw second side
            MPoint sideFactor(0,0,d);
            MPoint vertex2 = minVertex + sideFactor;
            gGLFT->glBegin( MGL_LINE_LOOP );
                gGLFT->glVertex3f( (float)vertex2[0],   (float)vertex2[1],   (float)vertex2[2] );
                gGLFT->glVertex3f( (float)vertex2[0]+w, (float)vertex2[1],   (float)vertex2[2] );
                gGLFT->glVertex3f( (float)vertex2[0]+w, (float)vertex2[1]+h, (float)vertex2[2] );
                gGLFT->glVertex3f( (float)vertex2[0],   (float)vertex2[1]+h, (float)vertex2[2] );
                gGLFT->glVertex3f( (float)vertex2[0],   (float)vertex2[1],   (float)vertex2[2] );
            gGLFT->glEnd();

            // Connect the edges together
            gGLFT->glBegin( MGL_LINES );
                gGLFT->glVertex3f( (float)vertex2[0],  (float)vertex2[1],  (float)vertex2[2] );
                gGLFT->glVertex3f( (float)vertex[0],   (float)vertex[1],   (float)vertex[2]  );

                gGLFT->glVertex3f( (float)vertex2[0]+w,  (float)vertex2[1],  (float)vertex2[2] );
                gGLFT->glVertex3f( (float)vertex[0]+w,   (float)vertex[1],   (float)vertex[2]  );

                gGLFT->glVertex3f( (float)vertex2[0]+w,  (float)vertex2[1]+h,  (float)vertex2[2] );
                gGLFT->glVertex3f( (float)vertex[0]+w,   (float)vertex[1]+h,   (float)vertex[2]  );

                gGLFT->glVertex3f( (float)vertex2[0],  (float)vertex2[1]+h,  (float)vertex2[2] );
                gGLFT->glVertex3f( (float)vertex[0],   (float)vertex[1]+h,   (float)vertex[2]  );
            gGLFT->glEnd();
        }
        break;

    case kVertexArrays:
        {
            // We are using vertex arrays
            gGLFT->glPushMatrix();

            // Prepare the matrix for the unit bounding box
            const MMatrix boundingBoxMatrix = 
                UnitBoundingBox::boundingBoxMatrix(boundingBox);
            gGLFT->glMultMatrixd(boundingBoxMatrix.matrix[0]);

            // Draw the bounding box
            indicesRead = fIndices->readableInterface();
            gGLFT->glDrawElements(MGL_LINES, 24, MGL_UNSIGNED_INT, indicesRead->get());

            // Restore the matrix
            gGLFT->glPopMatrix();
        }
        break;

    case kVBOs:
    default:
        // should not get here
        assert(0);
        break;
    }

    // Restore the OpenGL state to draw shaded
    if (overrideShadedState) {
        // Lighting
        if (lightingWasOn) {
            gGLFT->glEnable(MGL_LIGHTING);
        }

        // Depth write
        if (!depthMaskWasOn) {
            gGLFT->glDepthMask(MGL_FALSE);
        }

        // Line stipple
        if (!stippleWasOn) {
            gGLFT->glDisable(MGL_LINE_STIPPLE);
        }

        // Color
        gGLFT->glColor4fv(prevColor);
    }
}


} // namespace GPUCache
