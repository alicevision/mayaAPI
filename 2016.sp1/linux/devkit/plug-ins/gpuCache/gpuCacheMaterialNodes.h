#ifndef _gpuCacheMaterialNodes_h_
#define _gpuCacheMaterialNodes_h_

//-
//**************************************************************************/
// Copyright 2012 Autodesk, Inc. All rights reserved. 
//
// Use of this software is subject to the terms of the Autodesk 
// license agreement provided at the time of installation or download, 
// or which otherwise accompanies this software in either electronic 
// or hard copy form.
//**************************************************************************/
//+

#include "gpuCacheMaterial.h"


namespace GPUCache {

/*==============================================================================
 * CLASS SurfaceMaterial
 *============================================================================*/

// Base class for all surface materials (lambert, phong, ...)
class SurfaceMaterial : public MaterialNode
{
public:
    SurfaceMaterial(const MString& name, const MString& type = "surfaceShader");

    virtual void accept(MaterialNodeVisitor& visitor) const;

public:
    MaterialPropertyRef OutColor;
    MaterialPropertyRef OutTransparency;
};

// Acyclic visitor
class SurfaceMaterialVisitor : public virtual MaterialNodeVisitor
{
public:
    virtual void visit(const SurfaceMaterial& node) = 0;
};

/*==============================================================================
 * CLASS LambertMaterial
 *============================================================================*/

// Lambert material
class LambertMaterial : public SurfaceMaterial
{
public:
    LambertMaterial(const MString& name, const MString& type = "lambert");

    virtual void accept(MaterialNodeVisitor& visitor) const;

public:
    MaterialPropertyRef Color;
    MaterialPropertyRef Transparency;
    MaterialPropertyRef AmbientColor;
    MaterialPropertyRef Incandescence;
    MaterialPropertyRef Diffuse;
    MaterialPropertyRef Translucence;
    MaterialPropertyRef TranslucenceDepth;
    MaterialPropertyRef TranslucenceFocus;
    MaterialPropertyRef HideSource;
    MaterialPropertyRef GlowIntensity;
};

// Acyclic visitor
class LambertMaterialVisitor : public virtual MaterialNodeVisitor
{
public:
    virtual void visit(const LambertMaterial& node) = 0;
};

/*==============================================================================
 * CLASS PhongMaterial
 *============================================================================*/

// Phong material
class PhongMaterial : public LambertMaterial
{
public:
    PhongMaterial(const MString& name, const MString& type = "phong");

    virtual void accept(MaterialNodeVisitor& visitor) const;

public:
    MaterialPropertyRef CosinePower;
    MaterialPropertyRef SpecularColor;
    MaterialPropertyRef Reflectivity;
    MaterialPropertyRef ReflectedColor;
};

// Acyclic visitor
class PhongMaterialVisitor : public virtual MaterialNodeVisitor
{
public:
    virtual void visit(const PhongMaterial& node) = 0;
};

/*==============================================================================
 * CLASS BlinnMaterial
 *============================================================================*/

// Blinn material
class BlinnMaterial : public LambertMaterial
{
public:
    BlinnMaterial(const MString& name, const MString& type = "blinn");

    virtual void accept(MaterialNodeVisitor& visitor) const;

public:
    MaterialPropertyRef Eccentricity;
    MaterialPropertyRef SpecularRollOff;
    MaterialPropertyRef SpecularColor;
    MaterialPropertyRef Reflectivity;
    MaterialPropertyRef ReflectedColor;
};

// Acyclic visitor
class BlinnMaterialVisitor : public virtual MaterialNodeVisitor
{
public:
    virtual void visit(const BlinnMaterial& node) = 0;
};

/*==============================================================================
 * CLASS Texture2d
 *============================================================================*/

// 2D Texture.
// Textures are not supported. We just make use of the .defaultColor attribute.
class Texture2d : public MaterialNode
{
public:
    Texture2d(const MString& name, const MString& type = "texture2d");

    virtual void accept(MaterialNodeVisitor& visitor) const;

public:
    MaterialPropertyRef DefaultColor;
    MaterialPropertyRef OutColor;
    MaterialPropertyRef OutAlpha;
};

// Acyclic visitor
class Texture2dVisitor : public virtual MaterialNodeVisitor
{
public:
    virtual void visit(const Texture2d& node) = 0;
};

/*==============================================================================
 * CLASS FileTexture
 *============================================================================*/

// File Texture
// Textures are not supported. We just make use of the .defaultColor attribute.
class FileTexture : public Texture2d
{
public:
    FileTexture(const MString& name, const MString& type = "file");

    virtual void accept(MaterialNodeVisitor& visitor) const;

public:
    MaterialPropertyRef OutTransparency;
    MaterialPropertyRef FileTextureName;

    // Nothing. Textures are not supported!
};

// Acyclic visitor
class FileTextureVisitor : public virtual MaterialNodeVisitor
{
public:
    virtual void visit(const FileTexture& node) = 0;
};

/*==============================================================================
 * CLASS UnknownTexture2d
 *============================================================================*/

// This is a generic texture2d node that we don't recognize the node type.
class UnknownTexture2d : public Texture2d
{
public:
    UnknownTexture2d(const MString& name, const MString& type = "unknownTexture2d");

    virtual void accept(MaterialNodeVisitor& visitor) const;
};

// Acyclic visitor
class UnknownTexture2dVisitor : public virtual MaterialNodeVisitor
{
public:
    // Optional to visit unknown texture2d node.
    virtual void visit(const UnknownTexture2d& node) {}
};

/*==============================================================================
 * CLASS UnknownMaterialNode
 *============================================================================*/

// This is a generic material node that we don't recognize the node type.
class UnknownMaterialNode : public MaterialNode
{
public:
    UnknownMaterialNode(const MString& name, const MString& type = "unknown");

    virtual void accept(MaterialNodeVisitor& visitor) const;
};

// Acyclic visitor
class UnknownMaterialNodeVisitor : public virtual MaterialNodeVisitor
{
public:
    // Optional to visit unknown node.
    virtual void visit(const UnknownMaterialNode& node) {}
};


/*==============================================================================
 * CLASS ConcreteMaterialNodeVisitor
 *============================================================================*/

class ConcreteMaterialNodeVisitor :
    public SurfaceMaterialVisitor,
    public LambertMaterialVisitor,
    public PhongMaterialVisitor,
    public BlinnMaterialVisitor,
    public Texture2dVisitor,
    public FileTextureVisitor,
    public UnknownTexture2dVisitor,
    public UnknownMaterialNodeVisitor
{};


} // namespace GPUCache

#endif

