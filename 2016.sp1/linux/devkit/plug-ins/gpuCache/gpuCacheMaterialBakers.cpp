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

#include "gpuCacheMaterialBakers.h"
#include "gpuCacheMaterialNodes.h"
#include "gpuCacheShapeNode.h"
#include "gpuCacheUtil.h"

#include <boost/foreach.hpp>

#include <set>

#include <maya/MFnDagNode.h>
#include <maya/MFnNumericData.h>
#include <maya/MPlugArray.h>


namespace GPUCache {

// Bakers for concrete materials.
//
namespace MaterialBakers {

    // This class is the base class for all shading node bakers.
    class BaseMaterialNodeBaker : boost::noncopyable
    {
    public:
        typedef boost::shared_ptr<BaseMaterialNodeBaker> Ptr;

        // Create a material baker for the given DG node.
        static BaseMaterialNodeBaker::Ptr create(
            const MObject&         node,
            std::set<std::string>* traversedNodes);

        BaseMaterialNodeBaker(const MObject& node)
            : fNode(node), fTraversedNodes(NULL)
        {}
        
        virtual ~BaseMaterialNodeBaker()
        {}

        // Create the baked material node and collect the 
        // properties and plugs for sampling.
        void setupNetwork()
        {
            // There are virtual function calls so we can't do this in c'tor.
            fBakedNode = createNode(fNode.name());
            assert(fBakedNode);

            collectPlugsAndProperties();
        }

        // Sample the shading node at the given time.
        void sample(const MTime& time)
        {
            assert(fBakedNode); 
            // Loop over all channels.
            BOOST_FOREACH (Channel& channel, fChannels) {
                // Sample the plug and add the sample to the property
                switch (channel.prop()->type()) {
                case MaterialProperty::kBool:
                    sampleBoolPlug(time, channel.plug(), channel.prop());
                    break;
                case MaterialProperty::kInt32:
                    sampleInt32Plug(time, channel.plug(), channel.prop());
                    break;
                case MaterialProperty::kFloat:
                    sampleFloatPlug(time, channel.plug(), channel.prop());
                    break;
                case MaterialProperty::kFloat2:
                    sampleFloat2Plug(time, channel.plug(), channel.prop());
                    break;
                case MaterialProperty::kFloat3:
                    sampleFloat3Plug(time, channel.plug(), channel.prop());
                    break;
                case MaterialProperty::kRGB:
                    sampleFloat3PlugAsColor(time, channel.plug(), channel.prop());
                    break;
                case MaterialProperty::kString:
                    sampleStringPlug(time, channel.plug(), channel.prop());
                    break;
                default:
                    assert(0); // the data type is not implemented!
                    break;
                }

                // Recursively sample source nodes.
                BaseMaterialNodeBaker::Ptr& srcBaker = channel.srcBaker();
                if (srcBaker) {
                    srcBaker->sample(time);
                }
            }
        }

        // Add the shading node to the graph.
        // The graph will have the ownership of all the shading nodes.
        void addToGraph(MaterialGraph::MPtr& graph)
        {
            assert(fBakedNode);  // created by derived classes
            if (fBakedNode) {
                graph->addNode(fBakedNode);
            }

            // Recursively add the connected shading nodes
            BOOST_FOREACH (Channel& channel, fChannels) {
                BaseMaterialNodeBaker::Ptr& srcBaker = channel.srcBaker();
                if (srcBaker) {
                    srcBaker->addToGraph(graph);
                }
            }
        }

        // Connect the shading nodes.
        void connect()
        {
            // Loop over all the channels and connect to its source node.
            BOOST_FOREACH (Channel& channel, fChannels) {
                MaterialProperty::MPtr     dstProp  = channel.prop();
                BaseMaterialNodeBaker::Ptr srcBaker = channel.srcBaker();
                MaterialProperty::MPtr     srcProp  = channel.srcProp();

                // Connect to srcNode.srcProp
                if (dstProp && srcBaker && srcProp) {
                    MaterialNode::Ptr srcNode = srcBaker->bakedNode();

                    if (srcNode) {
                        dstProp->connect(srcNode, srcProp);
                    }
                }
            }
        }

        // Return the baked material node.
        MaterialNode::MPtr bakedNode()
        { assert(fBakedNode); return fBakedNode; }

    protected:
        // Override by derived classes.
        virtual MaterialNode::MPtr createNode(const MString& name) = 0;
        virtual void               collectPlugsAndProperties() = 0;

        // Sample a Maya bool plug
        void sampleBoolPlug(const MTime& time, const MPlug& plug, MaterialProperty::MPtr& prop)
        {
            double timeInSeconds = time.as(MTime::kSeconds);

            bool value = plug.asBool();
            if (prop->isDefault() || prop->asBool(timeInSeconds) != value) {
                prop->setBool(timeInSeconds, value);
            }
        }

        // Sample a Maya int plug
        void sampleInt32Plug(const MTime& time, const MPlug& plug, MaterialProperty::MPtr& prop)
        {
            double timeInSeconds = time.as(MTime::kSeconds);

            int value = plug.asInt();
            if (prop->isDefault() || prop->asInt32(timeInSeconds) != value) {
                prop->setInt32(timeInSeconds, value);
            }
        }

        // Sample a Maya float plug
        void sampleFloatPlug(const MTime& time, const MPlug& plug, MaterialProperty::MPtr& prop)
        {
            double timeInSeconds = time.as(MTime::kSeconds);

            float value = plug.asFloat();
            if (prop->isDefault() || prop->asFloat(timeInSeconds) != value) {
                prop->setFloat(timeInSeconds, value);
            }
        }

        // Sample a Maya (float,float) plug
        void sampleFloat2Plug(const MTime& time, const MPlug& plug, MaterialProperty::MPtr& prop)
        {
            double timeInSeconds = time.as(MTime::kSeconds);

            MObject data = plug.asMObject();
            assert(data.hasFn(MFn::kNumericData));

            float value[2], prev[2];
            MFnNumericData(data).getData2Float(value[0], value[1]);
            prop->asFloat2(timeInSeconds, prev[0], prev[1]);

            if (prop->isDefault() || value[0] != prev[0] || value[1] != prev[1]) {
                prop->setFloat2(timeInSeconds, value[0], value[1]);
            }
        }

        // Sample a Maya (float,float,float) plug
        void sampleFloat3Plug(const MTime& time, const MPlug& plug, MaterialProperty::MPtr& prop)
        {
            double timeInSeconds = time.as(MTime::kSeconds);

            MObject data = plug.asMObject();
            assert(data.hasFn(MFn::kNumericData));

            float value[3], prev[3];
            MFnNumericData(data).getData3Float(value[0], value[1], value[2]);
            prop->asFloat3(timeInSeconds, prev[0], prev[1], prev[2]);

            if (prop->isDefault() || value[0] != prev[0] || value[1] != prev[1] || value[2] != prev[2]) {
                prop->setFloat3(timeInSeconds, value[0], value[1], value[2]);
            }
        }

        // Sample a Maya (float,float,float) plug as MColor
        void sampleFloat3PlugAsColor(const MTime& time, const MPlug& plug, MaterialProperty::MPtr prop)
        {
            double timeInSeconds = time.as(MTime::kSeconds);

            MObject data = plug.asMObject();
            assert(data.hasFn(MFn::kNumericData));

            MColor value;
            MFnNumericData(data).getData3Float(value.r, value.g, value.b);

            if (prop->isDefault() || value != prop->asColor(timeInSeconds)) {
                prop->setColor(timeInSeconds, value);
            }
        }

        // Sample a Maya MString plug
        void sampleStringPlug(const MTime& time, const MPlug& plug, MaterialProperty::MPtr& prop)
        {
            double timeInSeconds = time.as(MTime::kSeconds);

            MString value = plug.asString();
            if (prop->isDefault() || value != prop->asString(timeInSeconds)) {
                prop->setString(timeInSeconds, value);
            }
        }

        // Register the plug and its source plugs for sampling.
        void sampleChannel(const MString& name, MaterialProperty::MPtr& prop)
        {
            // Find the plug by its name
            MPlug plug = fNode.findPlug(name, false);
            assert(!plug.isNull());
            assert(prop);

            if (!plug.isNull() && prop) {
                // Track the connection to the source node.
                BaseMaterialNodeBaker::Ptr srcBaker;
                MaterialProperty::MPtr     srcProp;

                if (plug.isDestination()) {
                    // Find the source node.
                    MPlugArray plugArray;
                    plug.connectedTo(plugArray, true, false);
                    assert(plugArray.length() == 1);
                    
                    if (plugArray.length() > 0) {
                        MPlug srcPlug = plugArray[0];
                        assert(!srcPlug.isNull());

                        MObject srcNode = srcPlug.node();
                        assert(!srcNode.isNull());

                        // If there is a circular connection, we stop tracking the
                        // connection to the source node. Instead, we sample the
                        // plug value directly.
                        if (!isTraversed(srcNode)) {
                            // Create the baker for the source node.
                            srcBaker = BaseMaterialNodeBaker::create(srcNode, fTraversedNodes);
                            if (srcBaker) {
                                // We recognize the node. Find the source property.
                                BOOST_FOREACH (Channel& channel, srcBaker->fChannels) {
                                    if (channel.plug() == srcPlug) {
                                        srcProp = channel.prop();
                                        break;
                                    }
                                }

                                // Can't find a source property.. give up.
                                if (!srcProp) {
                                    srcBaker.reset();
                                }
                            }
                        }
                    }
                }

                // Add this channel to the list (with optional source baker)
                fChannels.push_back(Channel(plug, prop, srcBaker, srcProp));
            }
        }

        // Set the traversed nodes to prevent infinite recursive.
        void setTraversedNodes(std::set<std::string>* traversedNodes)
        {
            fTraversedNodes = traversedNodes;
        }

        // Query if the node has been traversed.
        bool isTraversed(const MObject& node)
        {
            std::string name = MFnDependencyNode(node).name().asChar();
            assert(!name.empty());

            if (fTraversedNodes && (*fTraversedNodes).find(name) != (*fTraversedNodes).end()) {
                return true;
            }
            return false;
        }

        // Set the traversed state of the node.
        void setTraversed(const MObject& node)
        {
            std::string name = MFnDependencyNode(node).name().asChar();
            assert(!name.empty());

            if (fTraversedNodes) {
                (*fTraversedNodes).insert(name);
            }
        }

    private:
        class Channel
        {
        public:
            Channel(const MPlug&                plug, 
                    MaterialProperty::MPtr&     prop,
                    BaseMaterialNodeBaker::Ptr& srcBaker,
                    MaterialProperty::MPtr&     srcProp)
                : fPlug(plug), 
                  fProp(prop), 
                  fSrcBaker(srcBaker), 
                  fSrcProp(srcProp)
            {}
            ~Channel() {}

            const MPlug&                plug()     { return fPlug; }
            MaterialProperty::MPtr&     prop()     { return fProp; }
            BaseMaterialNodeBaker::Ptr& srcBaker() { return fSrcBaker; }
            MaterialProperty::MPtr&     srcProp()  { return fSrcProp; }

        private:
            MPlug                      fPlug;
            MaterialProperty::MPtr     fProp;
            BaseMaterialNodeBaker::Ptr fSrcBaker;
            MaterialProperty::MPtr     fSrcProp;
        };
        std::vector<Channel> fChannels;

    private:
        MFnDependencyNode      fNode;
        MaterialNode::MPtr     fBakedNode;
        std::set<std::string>* fTraversedNodes;
    };

    class SurfaceMaterialBaker : public BaseMaterialNodeBaker
    {
    public:
        SurfaceMaterialBaker(const MObject& node)
            : BaseMaterialNodeBaker(node) {}

        virtual MaterialNode::MPtr createNode(const MString& name)
        {
            return boost::make_shared<SurfaceMaterial>(name);
        }

        virtual void collectPlugsAndProperties()
        {
            boost::shared_ptr<SurfaceMaterial> surfaceMaterial = 
                boost::dynamic_pointer_cast<SurfaceMaterial>(bakedNode());

            sampleChannel("outColor",        surfaceMaterial->OutColor);
            sampleChannel("outTransparency", surfaceMaterial->OutTransparency);
        }
    };

    class LambertBaker : public SurfaceMaterialBaker
    {
    public:
        LambertBaker(const MObject& node)
            : SurfaceMaterialBaker(node) {}

        virtual MaterialNode::MPtr createNode(const MString& name)
        {
            return boost::make_shared<LambertMaterial>(name);
        }

        virtual void collectPlugsAndProperties()
        {
            SurfaceMaterialBaker::collectPlugsAndProperties();

            boost::shared_ptr<LambertMaterial> lambert = 
                boost::dynamic_pointer_cast<LambertMaterial>(bakedNode());

            sampleChannel("color",             lambert->Color);
            sampleChannel("transparency",      lambert->Transparency);
            sampleChannel("ambientColor",      lambert->AmbientColor);
            sampleChannel("incandescence",     lambert->Incandescence);
            sampleChannel("diffuse",           lambert->Diffuse);
            sampleChannel("translucence",      lambert->Translucence);
            sampleChannel("translucenceDepth", lambert->TranslucenceDepth);
            sampleChannel("translucenceFocus", lambert->TranslucenceFocus);
            sampleChannel("hideSource",        lambert->HideSource);
            sampleChannel("glowIntensity",     lambert->GlowIntensity);
        }
    };

    class PhongBaker : public LambertBaker
    {
    public:
        PhongBaker(const MObject& node)
            : LambertBaker(node) {}

        virtual MaterialNode::MPtr createNode(const MString& name)
        {
            return boost::make_shared<PhongMaterial>(name);
        }

        virtual void collectPlugsAndProperties()
        {
            LambertBaker::collectPlugsAndProperties();

            boost::shared_ptr<PhongMaterial> phong = 
                boost::dynamic_pointer_cast<PhongMaterial>(bakedNode());

            sampleChannel("cosinePower",    phong->CosinePower);
            sampleChannel("specularColor",  phong->SpecularColor);
            sampleChannel("reflectivity",   phong->Reflectivity);
            sampleChannel("reflectedColor", phong->ReflectedColor);
        }
    };

    class BlinnBaker : public LambertBaker
    {
    public:
        BlinnBaker(const MObject& node)
            : LambertBaker(node) {}

        virtual MaterialNode::MPtr createNode(const MString& name)
        {
            return boost::make_shared<BlinnMaterial>(name);
        }

        virtual void collectPlugsAndProperties()
        {
            LambertBaker::collectPlugsAndProperties();

            boost::shared_ptr<BlinnMaterial> phong = 
                boost::dynamic_pointer_cast<BlinnMaterial>(bakedNode());

            sampleChannel("eccentricity",   phong->Eccentricity);
			sampleChannel("specularRollOff",phong->SpecularRollOff);
            sampleChannel("specularColor",  phong->SpecularColor);
            sampleChannel("reflectivity",   phong->Reflectivity);
            sampleChannel("reflectedColor", phong->ReflectedColor);
        }
    };

    class Texture2dBaker : public BaseMaterialNodeBaker
    {
    public:
        Texture2dBaker(const MObject& node)
            : BaseMaterialNodeBaker(node) {}

        virtual MaterialNode::MPtr createNode(const MString& name) = 0;

        virtual void collectPlugsAndProperties()
        {
            boost::shared_ptr<Texture2d> texture2d = 
                boost::dynamic_pointer_cast<Texture2d>(bakedNode());

            sampleChannel("defaultColor", texture2d->DefaultColor);
            sampleChannel("outColor", texture2d->OutColor);
            sampleChannel("outAlpha", texture2d->OutAlpha);
        }
    };

    class FileTextureBaker : public Texture2dBaker
    {
    public:
        FileTextureBaker(const MObject& node)
            : Texture2dBaker(node) 
		{
		}

        virtual MaterialNode::MPtr createNode(const MString& name)
        {
            return boost::make_shared<FileTexture>(name);
        }

        virtual void collectPlugsAndProperties()
        {
            Texture2dBaker::collectPlugsAndProperties();

            boost::shared_ptr<FileTexture> file = 
                boost::dynamic_pointer_cast<FileTexture>(bakedNode());

            sampleChannel("outTransparency", file->OutTransparency);
			sampleChannel("fileTextureName", file->FileTextureName);
        }
    };

    class UnknownTexture2dBaker : public Texture2dBaker
    {
    public:
        UnknownTexture2dBaker(const MObject& node)
            : Texture2dBaker(node) {}

        virtual MaterialNode::MPtr createNode(const MString& name)
        {
            return boost::make_shared<UnknownTexture2d>(name);
        }

        virtual void collectPlugsAndProperties()
        {
            Texture2dBaker::collectPlugsAndProperties();
        }
    };

    BaseMaterialNodeBaker::Ptr BaseMaterialNodeBaker::create(
        const MObject&         node,
        std::set<std::string>* traversedNodes)
    {
        BaseMaterialNodeBaker::Ptr baker;

        if (node.hasFn(MFn::kPhong)) {
            baker = boost::make_shared<PhongBaker>(boost::ref(node));
        }
        else if (node.hasFn(MFn::kBlinn)) {
            baker = boost::make_shared<BlinnBaker>(boost::ref(node));
        }
        else if (node.hasFn(MFn::kLambert)) {
            baker = boost::make_shared<LambertBaker>(boost::ref(node));
        }
        else if (node.hasFn(MFn::kFileTexture)) {
            baker = boost::make_shared<FileTextureBaker>(boost::ref(node));
        }
        else if (node.hasFn(MFn::kTexture2d)) {
            baker = boost::make_shared<UnknownTexture2dBaker>(boost::ref(node));
        }

        // Recursively create connected bakers.
        if (baker) {
            baker->setTraversedNodes(traversedNodes);
            baker->setTraversed(node);
            baker->setupNetwork();
        }
        return baker;
    }

}

using namespace MaterialBakers;


/*==============================================================================
 * CLASS MaterialBaker
 *============================================================================*/

// This class bakes a material graph that has a surface material as its root.
class MaterialBaker::MaterialGraphBaker : boost::noncopyable
{
public:
    MaterialGraphBaker(const MObject& node)
    {
        fRootBaker = BaseMaterialNodeBaker::create(node, &fTraversedNodes);
    }

    ~MaterialGraphBaker() {}

    void sample(const MTime& time)
    {
        if (fRootBaker) {
            fRootBaker->sample(time);
        }
    }

    void buildGraph()
    {
        if (fRootBaker) {
            MaterialNode::Ptr rootNode = fRootBaker->bakedNode();

            if (rootNode) {
                // Create the material graph.
                MaterialGraph::MPtr graph = boost::make_shared<MaterialGraph>(rootNode->name());

                // Add all shading nodes to the graph
                fRootBaker->addToGraph(graph);

                // Connect the shading nodes
                fRootBaker->connect();

                // Set the root node of the graph
                graph->setRootNode(rootNode);

                fGraph = graph;
            }

            // We are done with the bakers
            fRootBaker.reset();
        }
    }

    MaterialGraph::Ptr get() const
    {
        return fGraph;
    }

private:
    BaseMaterialNodeBaker::Ptr fRootBaker;
    MaterialGraph::MPtr        fGraph;
    std::set<std::string>      fTraversedNodes;
};

MaterialBaker::MaterialBaker()
{}

MaterialBaker::~MaterialBaker()
{}

MStatus MaterialBaker::addShapePath(const MDagPath& dagPath)
{
    // Must be a shape.
    if (!dagPath.node().hasFn(MFn::kShape)) {
        return MS::kFailure;
    }

    // Check if we are recursively baking gpuCache nodes
    MFnDagNode dagNode(dagPath);
    if (dagNode.typeId() == ShapeNode::id) {
        const ShapeNode* node = (const ShapeNode*)dagNode.userNode();
        if (node) {
            const MaterialGraphMap::Ptr materials = node->getCachedMaterial();
            if (materials) {
                // Grab the existing materials.
                const MaterialGraphMap::NamedMap& graphs = materials->getGraphs();
                if (!graphs.empty()) {
                    fExistingGraphs.insert(graphs.cbegin(), graphs.cend());
                }
            }
        }

        return MS::kSuccess;
    }

    // Find all connected materials.
    InstanceMaterialLookup lookup(dagPath);
    if (lookup.hasWholeObjectMaterial()) {
        // Single material applied to the whole object.
        MObject surfaceMaterial = lookup.findWholeObjectSurfaceMaterial();

        // No material, silently ignored.
        if (surfaceMaterial.isNull()) {
            return MS::kSuccess;
        }

        // Get the name of the surface material
        MFnDependencyNode dgNode(surfaceMaterial);
        MString name = dgNode.name();

        // Create a new material baker
        MaterialGraphBakers::iterator iter = fMaterialGraphBakers.find(name);
        if (iter == fMaterialGraphBakers.end()) {
            MaterialGraphBakerPtr baker = 
                boost::make_shared<MaterialGraphBaker>(surfaceMaterial);
            fMaterialGraphBakers.insert(std::make_pair(name, baker));
        }
    }
    else if (lookup.hasComponentMaterials()) {
        // Multiple materials applied to components.
        std::vector<MObject> surfaceMaterials;
        lookup.findSurfaceMaterials(surfaceMaterials);

        BOOST_FOREACH (const MObject& surfaceMaterial, surfaceMaterials) {
            if (surfaceMaterial.isNull()) continue;

            // Get the name of the surface material
            MFnDependencyNode dgNode(surfaceMaterial);
            MString name = dgNode.name();

            // Create a new material baker
            MaterialGraphBakers::iterator iter = fMaterialGraphBakers.find(name);
            if (iter == fMaterialGraphBakers.end()) {
                MaterialGraphBakerPtr baker = 
                    boost::make_shared<MaterialGraphBaker>(surfaceMaterial);
                fMaterialGraphBakers.insert(std::make_pair(name, baker));
            }
        }
    }

    return MS::kSuccess;
}

MStatus MaterialBaker::sample(const MTime& time)
{
    BOOST_FOREACH (MaterialGraphBakers::value_type& val, fMaterialGraphBakers) {
        val.second->sample(time);
    }
    return MS::kSuccess;
}

MStatus MaterialBaker::buildGraph()
{
    BOOST_FOREACH (MaterialGraphBakers::value_type& val, fMaterialGraphBakers) {
        val.second->buildGraph();
    }
    return MS::kSuccess;
}


MaterialGraphMap::Ptr MaterialBaker::get()
{
    MaterialGraphMap::MPtr graphMap = boost::make_shared<MaterialGraphMap>();

    // Add baked materials.
    BOOST_FOREACH (const MaterialGraphBakers::value_type& val, fMaterialGraphBakers) {
        MaterialGraph::Ptr graph = val.second->get();
        if (graph) {
            graphMap->addMaterialGraph(graph);
        }
    }

    // Add existing materials
    BOOST_FOREACH (const NamedMaterialGraphs::value_type& val, fExistingGraphs) {
        if (val.second && !graphMap->find(val.first)) {
            graphMap->addMaterialGraph(val.second);
        }
    }

    return !graphMap->getGraphs().empty() ? graphMap : MaterialGraphMap::Ptr();

}


} // namespace GPUCache

