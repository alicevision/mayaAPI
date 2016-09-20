//-
// ==========================================================================
// Copyright 2011 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

#ifndef _cgfxTextureCache_h_
#define _cgfxTextureCache_h_


#include "cgfxShaderCommon.h"
#include "cgfxAttrDef.h"

#include <string>

template <class T> class cgfxRCPtr;
class cgfxTextureCache;

class cgfxTextureCacheEntry
{
public:
    const std::string& getTextureFilePath() const {
        return fTextureFilePath;
    }

    const std::string& getShaderFxFile() const {
        return fShaderFxFile;
    }

    const std::string& getAttrName() const {
        return fAttrName;
    }

    cgfxAttrDef::cgfxAttrType getAttrType() const {
        return fAttrType;
    }

    GLuint getTextureId() const {
        return fTextureId;
    }
        
    GLuint isValid() const {
        return fValid;
    }

    GLuint isStaled() const {
        return fStaled;
    }
        
    void markAsStaled();

    // For debugging...
    int getRefCount() const {
        return fRefCount;
    }
    
private:
    friend class cgfxRCPtr<cgfxTextureCacheEntry>;
    friend class cgfxTextureCache;

    // Construct a cache entry containing both the key and the
    // texture data.
    cgfxTextureCacheEntry(
        const std::string&        textureFilePath,
        const std::string&        shaderFxFile,
        const std::string&        attrName,
        cgfxAttrDef::cgfxAttrType attrType,
        GLuint                    textureId,
        bool                      valid
    )
        : fRefCount(0),
          fTextureFilePath(textureFilePath),
          fShaderFxFile(shaderFxFile),
          fAttrName(attrName),
          fAttrType(attrType),
          fValid(valid), 
          fStaled(false),
          fTextureId(textureId)
    {}
        
    ~cgfxTextureCacheEntry();

    void addRef();
    void release();

    int     fRefCount;  // For cgfxRCPtr...

    // Key for uniquely indentifying this entry.
    //
    // FIXME: This should be changed to a pointer to an cgfxAttrDef
    // once the cgfxEffect cache is implemented...
    std::string                 fTextureFilePath;
    std::string                 fShaderFxFile;
    std::string                 fAttrName;
    cgfxAttrDef::cgfxAttrType   fAttrType;

    // Data about the loaded texture.

    // Indicates whether the has been correctly read.
    bool    fValid;     

    // Indicates that an invalidation has been recieved for this
    // texture entry. New request to the cache should create a new
    // entry by re-reading the potentially changed texture file
    // instead of reusing this entry.
    bool    fStaled;

    // The GL identifer for this entry. Might contained a stand-in
    // texture if the texture file couldn't be properly read.
    GLuint  fTextureId;        
};


class cgfxTextureCache
{
public:
    static void initialize();
    static void uninitialize();

    // Return the single instance of the texture cache.
    static cgfxTextureCache& instance();

    // Return the texture cache entry matching the parameters. If the
    // texture is not present in the cache, an entry will be created
    // and an attempt to load the texture data from the texture file
    // will be made.
    virtual cgfxRCPtr<cgfxTextureCacheEntry> getTexture(
        MString                     texFileName,
        MObject                     textureNode,
        MString                     shaderFxFile,
        MString                     attrName,
        cgfxAttrDef::cgfxAttrType   attrType
    ) = 0;

    // For debugging...
    virtual void dump() const = 0;
  
private:
    friend class cgfxTextureCacheEntry;
    class Imp;
    
    cgfxTextureCache();
    virtual ~cgfxTextureCache();

    // Prohibited and not implemented.
    cgfxTextureCache(const cgfxTextureCache&);
    const cgfxTextureCache& operator=(const cgfxTextureCache&);
};

#endif
