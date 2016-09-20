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

#include "gpuCacheMaterialNodes.h"


namespace GPUCache {

/*==============================================================================
 * CLASS SurfaceMaterial
 *============================================================================*/

SurfaceMaterial::SurfaceMaterial(const MString& name, const MString& type)
    : MaterialNode(name, type)
{
    createProperty("outColor",        MaterialProperty::kRGB, OutColor);
    createProperty("outTransparency", MaterialProperty::kRGB, OutTransparency);
}

void SurfaceMaterial::accept(MaterialNodeVisitor& visitor) const
{
    SurfaceMaterialVisitor* v = dynamic_cast<SurfaceMaterialVisitor*>(&visitor);
    if (v) {
        v->visit(*this);
    }
    else {
        assert(0);
    }
}


/*==============================================================================
 * CLASS LambertMaterial
 *============================================================================*/

LambertMaterial::LambertMaterial(const MString& name, const MString& type)
    : SurfaceMaterial(name, type)
{
    createProperty("color",             MaterialProperty::kRGB,   Color);
    createProperty("transparency",      MaterialProperty::kRGB,   Transparency);
    createProperty("ambientColor",      MaterialProperty::kRGB,   AmbientColor);
    createProperty("incandescence",     MaterialProperty::kRGB,   Incandescence);
    createProperty("diffuse",           MaterialProperty::kFloat, Diffuse);
    createProperty("translucence",      MaterialProperty::kFloat, Translucence);
    createProperty("translucenceDepth", MaterialProperty::kFloat, TranslucenceDepth);
    createProperty("translucenceFocus", MaterialProperty::kFloat, TranslucenceFocus);
    createProperty("hideSource",        MaterialProperty::kBool,  HideSource);
    createProperty("glowIntensity",     MaterialProperty::kFloat, GlowIntensity);

    Color->setDefault(MColor(0.5f, 0.5f, 0.5f));
    Diffuse->setDefault(0.8f);
    TranslucenceDepth->setDefault(0.5f);
    TranslucenceFocus->setDefault(0.5f);
}

void LambertMaterial::accept(MaterialNodeVisitor& visitor) const
{
    LambertMaterialVisitor* v = dynamic_cast<LambertMaterialVisitor*>(&visitor);
    if (v) {
        v->visit(*this);
    }
    else {
        assert(0);
    }
}

/*==============================================================================
 * CLASS PhongMaterial
 *============================================================================*/

PhongMaterial::PhongMaterial(const MString& name, const MString& type)
    : LambertMaterial(name, type)
{
    createProperty("cosinePower",    MaterialProperty::kFloat, CosinePower);
    createProperty("specularColor",  MaterialProperty::kRGB,   SpecularColor);
    createProperty("reflectivity",   MaterialProperty::kFloat, Reflectivity);
    createProperty("reflectedColor", MaterialProperty::kRGB,   ReflectedColor);

    CosinePower->setDefault(20.0f);
    SpecularColor->setDefault(MColor(0.5f, 0.5f, 0.5f));
    Reflectivity->setDefault(0.5f);
}

void PhongMaterial::accept(MaterialNodeVisitor& visitor) const
{
    PhongMaterialVisitor* v = dynamic_cast<PhongMaterialVisitor*>(&visitor);
    if (v) {
        v->visit(*this);
    }
    else {
        assert(0);
    }
}

/*==============================================================================
 * CLASS BlinnMaterial
 *============================================================================*/

BlinnMaterial::BlinnMaterial(const MString& name, const MString& type)
    : LambertMaterial(name, type)
{
    createProperty("eccentricity",   MaterialProperty::kFloat, Eccentricity);
    createProperty("specularRollOff",MaterialProperty::kFloat, SpecularRollOff);
    createProperty("specularColor",  MaterialProperty::kRGB,   SpecularColor);
    createProperty("reflectivity",   MaterialProperty::kFloat, Reflectivity);
    createProperty("reflectedColor", MaterialProperty::kRGB,   ReflectedColor);

	Eccentricity->setDefault(0.3f);
	SpecularRollOff->setDefault(0.7f);
    SpecularColor->setDefault(MColor(0.5f, 0.5f, 0.5f));
    Reflectivity->setDefault(0.5f);
}

void BlinnMaterial::accept(MaterialNodeVisitor& visitor) const
{
    BlinnMaterialVisitor* v = dynamic_cast<BlinnMaterialVisitor*>(&visitor);
    if (v) {
        v->visit(*this);
    }
    else {
        assert(0);
    }
}

/*==============================================================================
 * CLASS Texture2d
 *============================================================================*/

Texture2d::Texture2d(const MString& name, const MString& type)
    : MaterialNode(name, type)
{
    createProperty("defaultColor", MaterialProperty::kRGB,   DefaultColor);
    createProperty("outColor",     MaterialProperty::kRGB,   OutColor);
    createProperty("outAlpha",     MaterialProperty::kFloat, OutAlpha);

    DefaultColor->setDefault(MColor(0.5f, 0.5f, 0.5f));
}

void Texture2d::accept(MaterialNodeVisitor& visitor) const
{
    Texture2dVisitor* v = dynamic_cast<Texture2dVisitor*>(&visitor);
    if (v) {
        v->visit(*this);
    }
    else {
        assert(0);
    }
}

/*==============================================================================
 * CLASS FileTexture
 *============================================================================*/

FileTexture::FileTexture(const MString& name, const MString& type)
    : Texture2d(name, type)
{
    createProperty("outTransparency", MaterialProperty::kRGB, OutTransparency);
	createProperty("fileTextureName", MaterialProperty::kString, FileTextureName);
}

void FileTexture::accept(MaterialNodeVisitor& visitor) const
{
    FileTextureVisitor* v = dynamic_cast<FileTextureVisitor*>(&visitor);
    if (v) {
        v->visit(*this);
    }
    else {
        assert(0);
    }
}

/*==============================================================================
 * CLASS UnknownMaterialNode
 *============================================================================*/

UnknownMaterialNode::UnknownMaterialNode(const MString& name, const MString& type)
    : MaterialNode(name, type)
{}

void UnknownMaterialNode::accept(MaterialNodeVisitor& visitor) const
{
    UnknownMaterialNodeVisitor* v = dynamic_cast<UnknownMaterialNodeVisitor*>(&visitor);
    if (v) {
        v->visit(*this);
    }
    else {
        assert(0);
    }
}

/*==============================================================================
 * CLASS UnknownTexture2d
 *============================================================================*/

UnknownTexture2d::UnknownTexture2d(const MString& name, const MString& type)
    : Texture2d(name, type)
{}

void UnknownTexture2d::accept(MaterialNodeVisitor& visitor) const
{
    UnknownTexture2dVisitor* v = dynamic_cast<UnknownTexture2dVisitor*>(&visitor);
    if (v) {
        v->visit(*this);
    }
    else {
        assert(0);
    }
}

/*==============================================================================
 * CLASS MaterialNode
 *============================================================================*/

// Create a concrete material node by type name.
MaterialNode::MPtr MaterialNode::create(const MString& name, const MString& nodeType)
{
    if (nodeType == "surfaceShader") {
        return boost::make_shared<SurfaceMaterial>(name);
    }
    else if (nodeType == "lambert") {
        return boost::make_shared<LambertMaterial>(name);
    }
    else if (nodeType == "phong") {
        return boost::make_shared<PhongMaterial>(name);
    }
    else if (nodeType == "blinn") {
        return boost::make_shared<BlinnMaterial>(name);
    }
    else if (nodeType == "file") {
        return boost::make_shared<FileTexture>(name);
    }
    else if (nodeType == "unknownTexture2d") {
        return boost::make_shared<UnknownTexture2d>(name);
    }
    else {
        return boost::make_shared<UnknownMaterialNode>(name, nodeType);
    }
}


} // namespace GPUCache

