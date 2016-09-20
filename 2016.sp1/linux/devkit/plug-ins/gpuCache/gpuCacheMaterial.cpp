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

#include "gpuCacheMaterial.h"

#include <boost/foreach.hpp>


namespace GPUCache {

/*==============================================================================
 * CLASS MaterialProperty
 *============================================================================*/

struct MaterialProperty::PropertyData : boost::noncopyable
{
    PropertyData()          {}
    virtual ~PropertyData() {}
};

struct MaterialProperty::BoolPropertyData : MaterialProperty::PropertyData 
{
    bool value;
    BoolPropertyData()       : value(false) {}
    BoolPropertyData(bool v) : value(v)     {}
};

struct MaterialProperty::Int32PropertyData : MaterialProperty::PropertyData
{
    int value;
    Int32PropertyData()      : value(0) {}
    Int32PropertyData(int v) : value(v) {}
};

struct MaterialProperty::FloatPropertyData : MaterialProperty::PropertyData
{
    float value;
    FloatPropertyData()        : value(0) {}
    FloatPropertyData(float v) : value(v) {}
};

struct MaterialProperty::Float2PropertyData : MaterialProperty::PropertyData
{
    float x, y;
    Float2PropertyData()                 : x(0), y(0) {}
    Float2PropertyData(float a, float b) : x(a), y(b) {}
};

struct MaterialProperty::Float3PropertyData : MaterialProperty::PropertyData
{
    float x, y, z;
    Float3PropertyData()                          : x(0), y(0), z(0) {}
    Float3PropertyData(float a, float b, float c) : x(a), y(b), z(c) {}
};

struct MaterialProperty::ColorPropertyData : MaterialProperty::PropertyData
{
    MColor value;
    ColorPropertyData()                           {}
    ColorPropertyData(const MColor& v) : value(v) {}
};

struct MaterialProperty::StringPropertyData : MaterialProperty::PropertyData
{
    MString value;
    StringPropertyData()                            {}
    StringPropertyData(const MString& v) : value(v) {}
};

MaterialProperty::MPtr MaterialProperty::create(
    const MString&         name, 
    MaterialProperty::Type type
)
{
    return boost::make_shared<MaterialProperty>(boost::ref(name), type);
}

MaterialProperty::MaterialProperty(const MString& name, Type type)
    : fName(name), fType(type)
{
    fDefaultValue = createData(type);
}

MaterialProperty::~MaterialProperty()
{}

bool MaterialProperty::asBool(double seconds) const
{
    assert(fType == kBool);
    return findValue<BoolPropertyData>(seconds)->value;
}

void MaterialProperty::setBool(double seconds, bool value)
{
    assert(fType == kBool);
    setValue(seconds, boost::make_shared<BoolPropertyData>(value));
}

int MaterialProperty::asInt32(double seconds) const
{
    assert(fType == kInt32);
    return findValue<Int32PropertyData>(seconds)->value;
}

void MaterialProperty::setInt32(double seconds, int value)
{
    assert(fType == kInt32);
    setValue(seconds, boost::make_shared<Int32PropertyData>(value));
}

float MaterialProperty::asFloat(double seconds) const
{
    assert(fType == kFloat);
    return findValue<FloatPropertyData>(seconds)->value;
}

void MaterialProperty::setFloat(double seconds, float value)
{
    assert(fType == kFloat);
    setValue(seconds, boost::make_shared<FloatPropertyData>(value));
}

void MaterialProperty::asFloat2(double seconds, float& x, float& y) const
{
    assert(fType == kFloat2);
    const Float2PropertyData* data = findValue<Float2PropertyData>(seconds);
    x = data->x;
    y = data->y;
}

void MaterialProperty::setFloat2(double seconds, float x, float y)
{
    assert(fType == kFloat2);
    setValue(seconds, boost::make_shared<Float2PropertyData>(x, y));
}

void MaterialProperty::asFloat3(double seconds, float& x, float& y, float& z) const
{
    assert(fType == kFloat3);
    const Float3PropertyData* data = findValue<Float3PropertyData>(seconds);
    x = data->x;
    y = data->y;
    z = data->z;
}

void MaterialProperty::setFloat3(double seconds, float x, float y, float z)
{
    assert(fType == kFloat3);
    setValue(seconds, boost::make_shared<Float3PropertyData>(x, y, z));
}

const MColor& MaterialProperty::asColor(double seconds) const
{
    assert(fType == kRGB);
    return findValue<ColorPropertyData>(seconds)->value;
}

void MaterialProperty::setColor(double seconds, const MColor& value)
{
    assert(fType == kRGB);
    setValue(seconds, boost::make_shared<ColorPropertyData>(boost::ref(value)));
}

const MString& MaterialProperty::asString(double seconds) const
{
    assert(fType == kString);
    return findValue<StringPropertyData>(seconds)->value;
}

void MaterialProperty::setString(double seconds, const MString& value)
{
    assert(fType == kString);
    setValue(seconds, boost::make_shared<StringPropertyData>(boost::ref(value)));
}

void MaterialProperty::setDefault(bool value)
{
    fDefaultValue = boost::make_shared<BoolPropertyData>(value);
}

void MaterialProperty::setDefault(int value)
{
    fDefaultValue = boost::make_shared<Int32PropertyData>(value);
}

void MaterialProperty::setDefault(float value)
{
    fDefaultValue = boost::make_shared<FloatPropertyData>(value);
}

void MaterialProperty::setDefault(float x, float y)
{
    fDefaultValue = boost::make_shared<Float2PropertyData>(x, y);
}

void MaterialProperty::setDefault(float x, float y, float z)
{
    fDefaultValue = boost::make_shared<Float3PropertyData>(x, y, z);
}

void MaterialProperty::setDefault(const MColor& value)
{
    fDefaultValue = boost::make_shared<ColorPropertyData>(boost::ref(value));
}

void MaterialProperty::setDefault(const MString& value)
{
    fDefaultValue = boost::make_shared<StringPropertyData>(boost::ref(value));
}

bool MaterialProperty::getDefaultAsBool() const
{
    const BoolPropertyData* data = static_cast<const BoolPropertyData*>(fDefaultValue.get());
    return data->value;
}

int MaterialProperty::getDefaultAsInt32() const
{
    const Int32PropertyData* data = static_cast<const Int32PropertyData*>(fDefaultValue.get());
    return data->value;
}

float MaterialProperty::getDefaultAsFloat() const
{
    const FloatPropertyData* data = static_cast<const FloatPropertyData*>(fDefaultValue.get());
    return data->value;
}

void MaterialProperty::getDefaultAsFloat2(float& x, float& y) const
{
    const Float2PropertyData* data = static_cast<const Float2PropertyData*>(fDefaultValue.get());
    x = data->x;
    y = data->y;
}

void MaterialProperty::getDefaultAsFloat3(float& x, float& y, float& z) const
{
    const Float3PropertyData* data = static_cast<const Float3PropertyData*>(fDefaultValue.get());
    x = data->x;
    y = data->y;
    z = data->z;
}

const MColor&  MaterialProperty::getDefaultAsColor() const
{
    const ColorPropertyData* data = static_cast<const ColorPropertyData*>(fDefaultValue.get());
    return data->value;
}

const MString& MaterialProperty::getDefaultAsString() const
{
    const StringPropertyData* data = static_cast<const StringPropertyData*>(fDefaultValue.get());
    return data->value;
}

MaterialProperty::PropertyDataPtr MaterialProperty::createData(Type type)
{
    switch (type) {
    case kBool:
        return boost::make_shared<BoolPropertyData>();
    case kInt32:
        return boost::make_shared<Int32PropertyData>();
    case kFloat:
        return boost::make_shared<FloatPropertyData>();
    case kFloat2:
        return boost::make_shared<Float2PropertyData>();
    case kFloat3:
        return boost::make_shared<Float3PropertyData>();
    case kRGB:
        return boost::make_shared<ColorPropertyData>();
    case kString:
        return boost::make_shared<StringPropertyData>();
    default:
        assert(0);  // Unknown type
        return boost::make_shared<FloatPropertyData>();
    }
}

/*==============================================================================
 * CLASS MaterialNode
 *============================================================================*/

MaterialProperty::MPtr MaterialNode::createProperty(const MString& name, MaterialProperty::Type type)
{
    // Create a new property and insert to the map.
    MaterialProperty::MPtr prop = MaterialProperty::create(name, type);
    assert(fProperties.find(name) == fProperties.end());
    fProperties.insert(std::make_pair(name, prop));
    return prop;
}

void MaterialNode::createProperty(const MString& name, MaterialProperty::Type type, MaterialPropertyRef& ref)
{
    // Create a new property and initialize the property reference.
    MaterialProperty::MPtr prop = createProperty(name, type);
    ref.initialize(prop);
}

MaterialProperty::MPtr MaterialNode::findProperty(const MString& name)
{
    // Find a mutable property pointer.
    PropertyMap::iterator it = fProperties.find(name);
    if (it != fProperties.end()) {
        return boost::const_pointer_cast<MaterialProperty>((*it).second);
    }
    return MaterialProperty::MPtr();
}

MaterialProperty::Ptr MaterialNode::findProperty(const MString& name) const
{
    // Find a const property pointer.
    PropertyMap::const_iterator it = fProperties.find(name);
    if (it != fProperties.end()) {
        return (*it).second;
    }
    return MaterialProperty::Ptr();
}


/*==============================================================================
 * CLASS MaterialGraph
 *============================================================================*/

bool MaterialGraph::isAnimated() const
{
    BOOST_FOREACH (const NamedMap::value_type& val, fMaterialNodeMap) {
        const MaterialNode::Ptr& node = val.second;
        if (!node) continue;

        BOOST_FOREACH (const MaterialNode::PropertyMap::value_type& val2, node->properties()) {
            const MaterialProperty::Ptr& prop = val2.second;
            if (!prop) continue;

            // Any of the property is animated.
            if (prop->isAnimated()) {
                return true;
            }
        }
    }

    return false;
}


/*==============================================================================
 * CLASS MaterialGraphMap
 *============================================================================*/

void MaterialGraphMap::addMaterialGraph(const MaterialGraph::Ptr& graph)
{
    assert(fMaterialGraphMap.find(graph->name()) == fMaterialGraphMap.end());
    fMaterialGraphMap.insert(std::make_pair(graph->name(), graph));
}

// Find the material graph by name.
const MaterialGraph::Ptr MaterialGraphMap::find(const MString name) const
{
    NamedMap::const_iterator it = fMaterialGraphMap.find(name);
    if (it != fMaterialGraphMap.end()) {
        return (*it).second;
    }
    return MaterialGraph::Ptr();
}


} // namespace GPUCache

