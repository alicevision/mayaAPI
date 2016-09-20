#ifndef _gpuCacheMaterial_h_
#define _gpuCacheMaterial_h_

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

#include <map>
#include <vector>
#include <assert.h>

#include <boost/make_shared.hpp>
#include <boost/ref.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>
#include <boost/weak_ptr.hpp>

#include <maya/MColor.h>
#include <maya/MString.h>


namespace GPUCache {

// Hash object for MString
struct MStringHash : std::unary_function<MString, std::size_t>
{
    std::size_t operator()(const MString& key) const
    {
        unsigned int length = key.length();
        const char* begin = key.asChar();
        return boost::hash_range(begin, begin + length);
    }
};


/*==============================================================================
 * CLASS MaterialProperty
 *============================================================================*/

class MaterialNode;

// A typed material property with connections and animated values.
// The property has 3 states:
//   1) Default Value (fValues.size() == 0): A brand new property with no samples.
//   2) Static Value  (fValues.size() == 1): A static property with only 1 sample.
//   3) Animated Value(fValues.size() >= 2): An animated property (rare case?).
// MaterialProperty = Maya MPlug = Alembic (I|O)ScalarProperty
class MaterialProperty : boost::noncopyable
{
public:
    // Const, Weak and Mutable pointers
    typedef boost::shared_ptr<const MaterialProperty> Ptr;
    typedef boost::weak_ptr<const MaterialProperty>   WPtr;
    typedef boost::shared_ptr<MaterialProperty>       MPtr;

    // MaterialNode class has not been declared.
    // Declare the shared pointers for MaterialNode class.
    typedef boost::shared_ptr<const MaterialNode> NodePtr;
    typedef boost::weak_ptr<const MaterialNode>   NodeWPtr;

    // The value type of this property.
    enum Type {
        kBool,
        kInt32,
        kFloat,
        kFloat2,
        kFloat3,
        kRGB,
        kString,
    };

    // The internal structures to store the property values.
    struct PropertyData;
    typedef boost::shared_ptr<PropertyData> PropertyDataPtr;

    // Map: timeInSeconds -> propertyData
    typedef std::map<double,PropertyDataPtr> SampleMap;

    // Create a property with the given type.
    static MPtr create(const MString& name, Type type);

    // Constructor and Destructor
    MaterialProperty(const MString& name, Type type);
    ~MaterialProperty();

    // Name and Type methods
    const MString& name() const { return fName; }
    Type           type() const { return fType; }

    // Get and Set methods
    bool asBool(double seconds) const;
    void setBool(double seconds, bool value);

    int asInt32(double seconds) const;
    void setInt32(double seconds, int value);

    float asFloat(double seconds) const;
    void setFloat(double seconds, float value);

    void asFloat2(double seconds, float& x, float& y) const;
    void setFloat2(double seconds, float x, float y);

    void asFloat3(double seconds, float& x, float& y, float& z) const;
    void setFloat3(double seconds, float x, float y, float z);

    const MColor& asColor(double seconds) const;
    void setColor(double seconds, const MColor& value);

    const MString& asString(double seconds) const;
    void setString(double seconds, const MString& value);

    // Default value methods
    void setDefault(bool value);
    void setDefault(int value);
    void setDefault(float value);
    void setDefault(float x, float y);
    void setDefault(float x, float y, float z);
    void setDefault(const MColor& value);
    void setDefault(const MString& value);

    bool           getDefaultAsBool() const;
    int            getDefaultAsInt32() const;
    float          getDefaultAsFloat() const;
    void           getDefaultAsFloat2(float& x, float& y) const;
    void           getDefaultAsFloat3(float& x, float& y, float& z) const;
    const MColor&  getDefaultAsColor() const;
    const MString& getDefaultAsString() const;

    bool isDefault() const { return fValues.empty(); }

    // Animated value methods 
    bool isAnimated() const { return fValues.size() > 1; }

    const SampleMap& getSamples() const { return fValues; }

    // Connection methods
    void connect(const NodePtr& node, const Ptr& prop)
    { assert(node && prop); fSourceNode = node; fSourceProp = prop; }

    const NodePtr srcNode() const
    { return fSourceNode.lock(); }

    const Ptr srcProp() const
    { return fSourceProp.lock(); }

private:
    struct BoolPropertyData;
    struct Int32PropertyData;
    struct FloatPropertyData;
    struct Float2PropertyData;
    struct Float3PropertyData;
    struct ColorPropertyData;
    struct StringPropertyData;

    // Create a property data by type.
    static PropertyDataPtr createData(Type type);

    // Find the pointer to the value at the given time.
    template<typename T>
    const T* findValue(double seconds) const
    {
        if (isAnimated()) {
            // Animated
            SampleMap::const_iterator it = fValues.upper_bound(seconds);
            if (it != fValues.begin()) --it;
            return static_cast<const T*>((*it).second.get());
        }
        else if (fValues.size() == 1) {
            return static_cast<const T*>((*fValues.begin()).second.get());
        }
        else {
            return static_cast<const T*>(fDefaultValue.get());
        }
    }

    // Set the value at the given time.
    void setValue(double seconds, const PropertyDataPtr& data)
    {
        assert(fValues.find(seconds) == fValues.end());
        fValues.insert(std::make_pair(seconds, data));
    }

    const MString   fName;
    const Type      fType;
    PropertyDataPtr fDefaultValue;
    SampleMap       fValues;
    NodeWPtr        fSourceNode;
    WPtr            fSourceProp;
};


/*==============================================================================
 * CLASS MaterialPropertyRef
 *============================================================================*/

 // A reference to the real property pointer.
 // We don't need to find the hash map for the known properties.
class MaterialPropertyRef : boost::noncopyable
{
public:
    MaterialPropertyRef()  {}
    ~MaterialPropertyRef() {}

    const MaterialProperty::Ptr operator->() const
    { assert(fProp); return fProp; }
    
    MaterialProperty::MPtr operator->()
    { assert(fProp); return fProp; }

    operator MaterialProperty::MPtr& ()
    { assert(fProp); return fProp;}

    operator const MaterialProperty::Ptr () const
    { assert(fProp); return fProp;}

    bool operator== (const MaterialProperty::Ptr& rv) const
    { assert(fProp); return fProp == rv; }

private:
    friend class MaterialNode;
    void initialize(MaterialProperty::MPtr& prop)
    { fProp = prop; }

    MaterialProperty::MPtr fProp;
};


/*==============================================================================
 * CLASS MaterialNodeVisitor
 *============================================================================*/

// This is a degenerated visitor class that follows acyclic visitor pattern.
class MaterialNodeVisitor : boost::noncopyable
{
public:
    virtual ~MaterialNodeVisitor() {}
};


/*==============================================================================
 * CLASS MaterialNode
 *============================================================================*/

// A material node with a set of properties.
// MaterialNode = Maya shadingNode = Alembic (I|O)MaterialSchema::NetworkNode
class MaterialNode : boost::noncopyable
{
public:
    // Const, Weak and Mutable pointers.
    typedef boost::shared_ptr<const MaterialNode> Ptr;
    typedef boost::weak_ptr<const MaterialNode>   WPtr;
    typedef boost::shared_ptr<MaterialNode>       MPtr;

    // Map: MString -> Property
    typedef boost::unordered_map<MString,MaterialProperty::Ptr,MStringHash> PropertyMap;

    static MaterialNode::MPtr create(const MString& name, const MString& nodeType);

    // Constructor and Destructor
    MaterialNode(const MString& name, const MString& type)
        : fName(name), fType(type)
    {}
    virtual ~MaterialNode() {}

    // Name and Type methods
    const MString& name() const { return fName; }
    const MString& type() const { return fType; }

    // Properties methods
    MaterialProperty::MPtr createProperty(const MString& name, MaterialProperty::Type type);

    MaterialProperty::MPtr findProperty(const MString& name);

    MaterialProperty::Ptr findProperty(const MString& name) const;

    const PropertyMap& properties() const { return fProperties; }

    // Visitor
    virtual void accept(MaterialNodeVisitor& visitor) const = 0;

protected:
    // Called by derived class only.
    // Create a known property and initialize its property reference.
    void createProperty(const MString& name, MaterialProperty::Type type, MaterialPropertyRef& ref);

private:
    const MString fName;
    const MString fType;
    PropertyMap   fProperties;
};


/*==============================================================================
 * CLASS MaterialGraph
 *============================================================================*/

// This class holds all the shading nodes.
// MaterialGraph = Maya shading nodes connected to a surface material = Alembic (I|O)MaterialSchema
// These nodes can be listed by MEL command: listHistory -pruneDagObjects surfaceMaterial
class MaterialGraph : boost::noncopyable
{
public:
    // Const and Mutable pointers
    typedef boost::shared_ptr<const MaterialGraph> Ptr;
    typedef boost::shared_ptr<MaterialGraph>       MPtr;
    typedef boost::weak_ptr<const MaterialGraph>   WPtr;

    // Map: MString -> MaterialNode
    typedef boost::unordered_map<MString, MaterialNode::Ptr, MStringHash > NamedMap;

    // Constructor and Destructor
    MaterialGraph(const MString& name)
        : fName(name)
    {}
    virtual ~MaterialGraph() {}

    // Name methods
    const MString& name()const 
    { return fName; }

    // Node Management
    void addNode(const MaterialNode::Ptr& node)
    { assert(node); fMaterialNodeMap.insert(std::make_pair(node->name(), node)); }

    const NamedMap& getNodes() const
    { return fMaterialNodeMap; }

    // Root (Terminal) Node
    void setRootNode(const MaterialNode::Ptr& node)
    { assert(node); fRootNode = node; }

    const MaterialNode::Ptr& rootNode() const
    { return fRootNode; }

    bool isAnimated() const;

private:
    const MString     fName;
    NamedMap          fMaterialNodeMap;
    MaterialNode::Ptr fRootNode;
};


/*==============================================================================
 * CLASS MaterialGraphMap
 *============================================================================*/

// This class contains all materials for a gpuCache node.
class MaterialGraphMap : boost::noncopyable
{
public:
    // Const and Mutable pointers.
    typedef boost::shared_ptr<const MaterialGraphMap> Ptr;
    typedef boost::shared_ptr<MaterialGraphMap>       MPtr;

    // Map: MString -> MaterialGraph
    typedef boost::unordered_map<MString, MaterialGraph::Ptr, MStringHash > NamedMap;

    // Constructor and Destructor
    MaterialGraphMap() {}
    virtual ~MaterialGraphMap() {}

    // Add the material graph to this map.
    void addMaterialGraph(const MaterialGraph::Ptr& graph);

    // Get all material graphs.
    const NamedMap& getGraphs() const
    { return fMaterialGraphMap; }

    // Find the material graph by name.
    const MaterialGraph::Ptr find(const MString name) const;

private:
    NamedMap fMaterialGraphMap;
};


} // namespace GPUCache

#endif

