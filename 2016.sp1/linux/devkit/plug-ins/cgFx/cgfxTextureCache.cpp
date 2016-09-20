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

#include "cgfxTextureCache.h"
#include "cgfxFindImage.h"
#include "cgfxProfile.h"

#include <maya/MHardwareRenderer.h>
#include <maya/MFileObject.h>
#include <maya/MGLFunctionTable.h>
#include "nv_dds.h"

#include <map>

namespace {

    //==========================================================================
    // Helper functions
    //==========================================================================

    bool textureInitPowerOfTwo(unsigned int val, unsigned int & retval)
    {
        unsigned int res = 0;				// May be we should return 1 when val == 0
        if (val)
        {
            // Muliply all values by 2, to simplify the testing:
            // 3*(res/2) < val*2 <= 3*res
            val <<= 1;
            unsigned int low = 3;
            res = 1;
            while (val > low)
            {
                low <<= 1;
                res <<= 1;
            }
        }

        retval = res;
        return (res == (val>>1)) ? 1 : 0;
    }

    MString computeTextureFilePath(
        MString                     texFileName,
        MString                     shaderFxFile
    )
    {
        if (texFileName.length() == 0) {
            return MString();
        }

        MString path = cgfxFindFile(texFileName);

        // If that failed, try and resolve the texture path relative to the
        // effect
        //
        if (path.length() == 0)
        {
            MFileObject effectFile;
            effectFile.setRawFullName(shaderFxFile);
            path = cgfxFindFile(effectFile.path() + texFileName);
        }

        return path;
    }
    
    bool allocateAndReadTexture(
        MString                     path,
        MObject                     textureNode,
        cgfxAttrDef::cgfxAttrType   attrType,
        GLuint&                     textureId
    )
    {
        static MGLFunctionTable *gGLFT = 0;
        if ( 0 == gGLFT )
            gGLFT = MHardwareRenderer::theRenderer()->glFunctionTable();
        
        GLuint val;
        gGLFT->glGenTextures(1, &val);
        textureId = val;

        nv_dds::CDDSImage image;

        if (path.length() > 0)
        {
            switch (attrType)
            {
                case cgfxAttrDef::kAttrTypeEnvTexture:
                case cgfxAttrDef::kAttrTypeCubeTexture:
                case cgfxAttrDef::kAttrTypeNormalizationTexture:
                    // we don't want to flip cube maps...
                    image.load(path.asChar(),false);
                    break;
                default:
                    // Only flip 2D textures if we're using right-handed texture
                    // coordinates. Most of the time, we want to do the flipping
                    // on the UV coordinates rather than the texture so that procedural
                    // texture coordinates generated inside the shader work as well
                    // (and if we just flip the texture to compensate for Maya's UV
                    // coordinate system, these will get inverted)
                    image.load(
                        path.asChar(),
                        cgfxProfile::getTexCoordOrientation() == cgfxProfile::TEXCOORD_OPENGL);
                    break;
            }
        }

        // Our common stand-in "texture"
        // The code below creates a separate stand-in GL texture
        // for every attribute without a value (rather than sharing
        // the default across all node/attributes of a given type.
        // This is done because the current design does not support
        // GL texture id sharing across nodes/attributes AND because
        // we want to avoid checking disk every frame for missing
        // textures. Once this plugin is re-factored to support a
        // shared texture cache, we should revisit this to share
        // default textures too
        //
        static unsigned char whitePixel[ 4] = { 255, 255, 255, 255};
        bool imageLoaded = false;
                        
        switch (attrType)
        {
            case cgfxAttrDef::kAttrTypeColor1DTexture:
                gGLFT->glBindTexture(GL_TEXTURE_1D,textureId);
                if( image.is_valid())
                {
                    // Load the image
                    gGLFT->glTexParameteri(
                        GL_TEXTURE_1D, GL_GENERATE_MIPMAP_SGIS, image.get_num_mipmaps() == 0);
                    image.upload_texture1D();
                    imageLoaded = true;
                }
                else
                {
                    // Create a dummy stand-in texture
                    gGLFT->glTexImage1D(
                        GL_TEXTURE_1D, 0, GL_RGBA, 1, 0,
                        GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
                }
                break;
            case cgfxAttrDef::kAttrTypeColor2DTexture:
            case cgfxAttrDef::kAttrTypeNormalTexture:
            case cgfxAttrDef::kAttrTypeBumpTexture:
#if !defined(WIN32) && !defined(LINUX)
            case cgfxAttrDef::kAttrTypeColor2DRectTexture:
#endif
                gGLFT->glBindTexture(GL_TEXTURE_2D,textureId);
                if( image.is_valid())
                {
                    // Load the image
                    gGLFT->glTexParameteri(
                        GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, image.get_num_mipmaps() == 0);
                    image.upload_texture2D();
                    imageLoaded = true;
                }
                else
                {
                    // Try to use Maya's default file texture loading,
                    // if the DDS loader failed. For now all that
                    // we can support is 2D textures.
                    //
                    if (textureNode != MObject::kNullObj)
                    {
                        MImage img;
                        unsigned int width, height;

                        if (MS::kSuccess == img.readFromTextureNode(textureNode))
                        {
                            // If we're using left handed texture coordinates, flip it upside down
                            // (to undo the automatic flip it receives being read in by Maya)
                            if (cgfxProfile::getTexCoordOrientation() ==
                                cgfxProfile::TEXCOORD_DIRECTX)
                            {
                                img.verticalFlip();
                            }

                            MStatus status = img.getSize( width, height);
                            if (width > 0 && height > 0 && (status != MStatus::kFailure) )
                            {
                                // If not power of two and NPOT is not supported, then we need
                                // to resize the original system pixmap before binding.
                                //
                                if (width > 2 && height > 2)
                                {
                                    unsigned int p2Width, p2Height;
                                    bool widthPowerOfTwo  = textureInitPowerOfTwo(width,  p2Width);
                                    bool heightPowerOfTwo = textureInitPowerOfTwo(height, p2Height);
                                    if(!widthPowerOfTwo || !heightPowerOfTwo)
                                    {
                                        width = p2Width;
                                        height = p2Height;
                                        img.resize( p2Width, p2Height, false /* preserverAspectRatio */);
                                    }
                                }
                                gGLFT->glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, true);
                                gGLFT->glTexImage2D(
                                    GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                                    GL_RGBA, GL_UNSIGNED_BYTE, img.pixels());
                                imageLoaded = true;
                            }
                        }
                    }
                }

                if (!imageLoaded) {
                    // Create a dummy stand-in texture
                    gGLFT->glTexImage2D(
                        GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0,
                        GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
                }
                break;
            case cgfxAttrDef::kAttrTypeEnvTexture:
            case cgfxAttrDef::kAttrTypeCubeTexture:
            case cgfxAttrDef::kAttrTypeNormalizationTexture:
                {
                    gGLFT->glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, textureId);
                    if( image.is_valid()) {
                        gGLFT->glTexParameteri(
                            GL_TEXTURE_CUBE_MAP_ARB, GL_GENERATE_MIPMAP_SGIS,
                            image.get_num_mipmaps() == 0);
                        // loop through cubemap faces and load them as 2D textures
                        for (int n = 0; n < 6; ++n)
                        {
                            // specify cubemap face
                            GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB+n;
                            image.upload_texture2D(image.is_cubemap() ? n : 0, target);
                        }
                        imageLoaded = true;
                    }
                    else {
                        // Loop through cubemap faces and
                        // load a dummy stand-in texture
                        for (int n = 0; n < 6; ++n)
                        {
                            // specify cubemap face
                            GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB+n;
                            gGLFT->glTexImage2D(
                                target, 0, GL_RGBA, 1, 1, 0,
                                GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
                        }
                    }
                    break;
                }
            case cgfxAttrDef::kAttrTypeColor3DTexture:
                gGLFT->glBindTexture(GL_TEXTURE_3D,textureId);
                if( image.is_valid())
                {
                    image.upload_texture3D();
                    imageLoaded = true;
                }
                else {
                    // Create a dummy stand-in texture
                    gGLFT->glTexImage3D(
                        GL_TEXTURE_3D, 0, GL_RGBA, 1, 1, 1, 0,
                        GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
                                 
                }
                break;
#if defined(WIN32) || defined(LINUX)
                // No such thing as NV texture rectangle
                // on Mac.
            case cgfxAttrDef::kAttrTypeColor2DRectTexture:
                gGLFT->glBindTexture(GL_TEXTURE_RECTANGLE_NV, textureId);
                if( image.is_valid())
                {
                    // Load the image
                    image.upload_textureRectangle();
                    imageLoaded = true;
                }
                else
                {
                    // Create a dummy stand-in texture
                    gGLFT->glTexImage2D(
                        GL_TEXTURE_RECTANGLE_NV, 0, GL_RGBA, 1, 1, 0,
                        GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
                }
                break;
#endif
            default:
                assert(false);
        }

        return imageLoaded;
    }

    //==========================================================================
    // Class EntryKey
    //==========================================================================
    
    struct EntryKey 
    {
        EntryKey(
            const std::string&        textureFilePath,
            const std::string&        shaderFxFile,
            const std::string&        attrName,
            cgfxAttrDef::cgfxAttrType attrType
        )
            : fTextureFilePath(textureFilePath),
              fShaderFxFile(shaderFxFile),
              fAttrName(attrName),
              fAttrType(attrType)
        {}
        
        EntryKey(const EntryKey& rhs)
            : fTextureFilePath(rhs.fTextureFilePath),
              fShaderFxFile(rhs.fShaderFxFile),
              fAttrName(rhs. fAttrName),
              fAttrType(rhs.fAttrType)
        {}

        const std::string                 fTextureFilePath;
        const std::string                 fShaderFxFile;
        const std::string                 fAttrName;
        const cgfxAttrDef::cgfxAttrType   fAttrType;

    private:

        // Prohibited and not implemented.
        const EntryKey& operator=(const EntryKey& rhs);
    };

    struct EntryKeyLessThan
    {
        bool operator()(const EntryKey& lhs, const EntryKey& rhs) const
        {
            if (lhs.fTextureFilePath < rhs.fTextureFilePath) {
                return true;
            }
            if (lhs.fTextureFilePath > rhs.fTextureFilePath) {
                return false;
            }

            if (lhs.fShaderFxFile < rhs.fShaderFxFile) {
                return true;
            }
            if (lhs.fShaderFxFile > rhs.fShaderFxFile) {
                return false;
            }

            if (lhs.fAttrName < rhs.fAttrName) {
                return true;
            }
            if (lhs.fAttrName > rhs.fAttrName) {
                return false;
            }

            if (lhs.fAttrType < rhs.fAttrType) {
                return true;
            }

            return false;
        }
    };
}


//==============================================================================
// Implementation of the cache hash map.
//==============================================================================

class cgfxTextureCache::Imp : public cgfxTextureCache 
{
public:
    static Imp* sTheTextureCache;

    Imp();
    ~Imp();
    
    // Return the texture cache entry matching the parameters. If the
    // texture is present in the cache, an entry will be created and
    // an attempt to load the texture data from the texture file will
    // be made.
    virtual cgfxRCPtr<cgfxTextureCacheEntry> getTexture(
        MString                     texFileName,
        MObject                     textureNode,
        MString                     shaderFxFile,
        MString                     attrName,
        cgfxAttrDef::cgfxAttrType   attrType
    );

    virtual void dump() const
    {
        fprintf(stderr, "*** Dumping texture cache ***\n");
        const Map::const_iterator end = fEntries.end();
        for (Map::const_iterator it = fEntries.begin(); it != end; ++it) {
            fprintf(stderr, "   entry = 0x%p, refCount = %d\n",
                    it->second.operator->(),
                    it->second->getRefCount());
            fprintf(stderr, "   tex file = \"%s\"\n",
                    it->first.fTextureFilePath.c_str());
            fprintf(stderr, "   fx  file = \"%s\"\n",
                    it->first.fShaderFxFile.c_str());
            fprintf(stderr, "   attrName = %s, attrType = %s\n\n",
                    it->first.fAttrName.c_str(),
                    cgfxAttrDef::typeName(it->first.fAttrType));
        }
    }
    
    static void flushEntry(const EntryKey& key)
    {
        sTheTextureCache->fEntries.erase(key);
    }
    
private:

    typedef std::map<EntryKey, cgfxRCPtr<cgfxTextureCacheEntry>, EntryKeyLessThan> Map;
    Map fEntries;
};

cgfxTextureCache::Imp* cgfxTextureCache::Imp::sTheTextureCache = 0;

cgfxTextureCache::Imp::Imp()
{}

cgfxTextureCache::Imp::~Imp()
{}

// Return the texture cache entry matching the parameters. If the
// texture is present in the cache, an entry will be created and
// an attempt to load the texture data from the texture file will
// be made.
cgfxRCPtr<cgfxTextureCacheEntry> cgfxTextureCache::Imp::getTexture(
    MString                     texFileName,
    MObject                     textureNode,
    MString                     shaderFxFile,
    MString                     attrName,
    cgfxAttrDef::cgfxAttrType   attrType
)
{
    MString textureFilePath =
        computeTextureFilePath(texFileName, shaderFxFile).asChar();

    // Note that the texture node is not part of the key. We assume
    // that all texture nodes with the same filename attribute are
    // actually referencing the file... 
    EntryKey key(textureFilePath.asChar(), shaderFxFile.asChar(), attrName.asChar(), attrType);
    
    const Map::const_iterator entryIt = fEntries.find(key);
    if (entryIt != fEntries.end()) {
        return entryIt->second;
    }

    GLuint textureId;
    bool valid = allocateAndReadTexture(
        textureFilePath, textureNode, attrType, textureId);
    
    cgfxRCPtr<cgfxTextureCacheEntry> entry(
        new cgfxTextureCacheEntry(
            key.fTextureFilePath, key.fShaderFxFile, key.fAttrName, key.fAttrType,
            textureId, valid));
    fEntries.insert(std::make_pair(key,entry));

    return entry;
}


//==============================================================================
// Class cgfxTextureCacheEntry
//==============================================================================

cgfxTextureCacheEntry::~cgfxTextureCacheEntry()
{
    glDeleteTextures(1, &fTextureId);
    fTextureId = 0;
}

void cgfxTextureCacheEntry::markAsStaled()
{
    fStaled = true;

	// During the flushEntry from the cache, it happens that the entry got deleted while still in use.
	// Make sure do add 1 more ref and release it right after the flushEntry is done, so it can be deleted properly.
	addRef();

    // This texture entry has been marked as staled. We remove it from
    // the texture cache so that it is relaoded from the texture file
    // the next time a cgfxShader needs it. This is necessary to allow
    // the user to update the content of the texture file and to
    // manually force a relaod of the texture.
    cgfxTextureCache::Imp::flushEntry(
        EntryKey(fTextureFilePath, fShaderFxFile, fAttrName, fAttrType));

	release();
}

    
void cgfxTextureCacheEntry::addRef()
{
    ++fRefCount;
};

void cgfxTextureCacheEntry::release()
{
    -- fRefCount;
    
    if (fRefCount == 1) {
        // If the refCount is one, only 2 cases are possible. Either
        // the last reference comes for the texture cache and we can
        // safely remove it from the texture cache to save memory. Or,
        // the last reference is for a staled texture cache entry and
        // it is no longer referenced by the texture cache anyway.  
        //
        cgfxTextureCache::Imp::flushEntry(
            EntryKey(fTextureFilePath, fShaderFxFile, fAttrName, fAttrType));
    }
    else if (fRefCount == 0) {
        delete this;
    }
}

//==============================================================================
// Class cgfxTextureCache
//==============================================================================

void cgfxTextureCache::initialize()
{
    Imp::sTheTextureCache = new cgfxTextureCache::Imp;
}

void cgfxTextureCache::uninitialize()
{
    delete Imp::sTheTextureCache;
    Imp::sTheTextureCache = 0;
}

cgfxTextureCache& cgfxTextureCache::instance()
{
    return *Imp::sTheTextureCache;
}

cgfxTextureCache::cgfxTextureCache()
{}

cgfxTextureCache::~cgfxTextureCache()
{}

