#ifndef _CacheWriterAlembic_h_
#define _CacheWriterAlembic_h_

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

// Includes
#include "CacheWriter.h"

#include <Alembic/Abc/OArchive.h>
#include <Alembic/Abc/OObject.h>
#include <Alembic/AbcGeom/OXform.h>
#include <Alembic/AbcGeom/OPolyMesh.h>
#include <Alembic/AbcGeom/Visibility.h>
#include <Alembic/AbcMaterial/OMaterial.h>
#include <Alembic/AbcMaterial/MaterialAssignment.h>

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>


// Forward Declarations
class AlembicXformWriter;
class AlembicMeshWriter;


//==============================================================================
// CLASS AlembicCacheWriter
//==============================================================================

class AlembicCacheWriter : public CacheWriter
{
public:
    static boost::shared_ptr<CacheWriter> create(
        const MFileObject& file, char compressLevel, const MString& dataFormat);

    virtual ~AlembicCacheWriter();
    virtual bool valid() const;

    // Write the hierarchy of nodes under the given top level node to
    // the cache file.
    virtual void writeSubNodeHierarchy(
        const GPUCache::SubNode::Ptr& topNode, 
        double secondsPerSample, double startTimeInSeconds);

    // Write the materials to the cache file.
    virtual void writeMaterials(
        const GPUCache::MaterialGraphMap::Ptr& materialGraphMap,
        double secondsPerSample, double startTimeInSeconds);

	// returns the actual file name the implementation is writing
	virtual const MFileObject& getFileObject() const;

private:

    GPUCACHE_DECLARE_MAKE_SHARED_FRIEND_3;

    AlembicCacheWriter(const MFileObject& file, char compressLevel, const MString& dataFormat);

    MFileObject                   fFile;
    char                          fCompressLevel;
    MString                       fDataFormat;
    Alembic::Abc::OArchive        fAbcArchive;
    Alembic::Abc::TimeSamplingPtr fAbcTimeSampling;
    unsigned int                  fMaxNumSamples;
    std::vector<MBoundingBox>     fArchiveBounds;
};

class AlembicSubNodeWriter
{
public:
	virtual ~AlembicSubNodeWriter(){}
	virtual Alembic::Abc::OObject object() = 0;
};

//==============================================================================
// CLASS AlembicXformWriter
//==============================================================================

class AlembicXformWriter: public AlembicSubNodeWriter
{
public:
    AlembicXformWriter(
        const Alembic::Abc::OObject& parent,
        const MString& name,
        double secondsPerSample, double startTimeInSeconds);

    virtual Alembic::Abc::OObject object();

    // Write first sample.
    void write(const boost::shared_ptr<const GPUCache::XformSample>&  sample);

    // Write following samples.
    void write(const boost::shared_ptr<const GPUCache::XformSample>&  sample,
               const boost::shared_ptr<const GPUCache::XformSample>&  prev);

private:
    void fillXform(Alembic::AbcGeom::XformSample& xformSample,
                   const boost::shared_ptr<const GPUCache::XformSample>& sample);


    Alembic::Abc::TimeSamplingPtr         fTimeSampPtr;
    Alembic::AbcGeom::OXformSchema        fAbcXform;
    Alembic::AbcGeom::OVisibilityProperty fVisibility;
    size_t                                fCachedWrite;
};


//==============================================================================
// CLASS AlembicMeshWriter
//==============================================================================

class AlembicMeshWriter: public AlembicSubNodeWriter
{
public:
    AlembicMeshWriter(
        const Alembic::Abc::OObject& parent,
        const MString& name,
        double secondsPerSample, double startTimeInSeconds);

    virtual Alembic::Abc::OObject object();

    // Write first sample.
    void write(const boost::shared_ptr<const GPUCache::ShapeSample>&  sample);

    // Write following samples.
    void write(const boost::shared_ptr<const GPUCache::ShapeSample>&  sample,
               const boost::shared_ptr<const GPUCache::ShapeSample>&  prev);

private:
    void fillWireframeSample(Alembic::Abc::Int32ArraySample& wireIndicesSample,
                             const boost::shared_ptr<const GPUCache::ShapeSample>& sample);
    void fillTriangleSample(Alembic::AbcGeom::OPolyMeshSchema::Sample& meshSample,
                            Alembic::Abc::Int32ArraySample& groupSizesSample,
                            const boost::shared_ptr<const GPUCache::ShapeSample>& sample);
    void fillPositionSample(Alembic::AbcGeom::OPolyMeshSchema::Sample& meshSample,
                            const boost::shared_ptr<const GPUCache::ShapeSample>& sample);
    void fillNormalSample(Alembic::AbcGeom::OPolyMeshSchema::Sample& meshSample,
                          const boost::shared_ptr<const GPUCache::ShapeSample>& sample,
                          bool forceWrite);
    void fillUVSample(Alembic::AbcGeom::OPolyMeshSchema::Sample& meshSample,
                      const boost::shared_ptr<const GPUCache::ShapeSample>& sample,
                      bool forceWrite);
    void fillBoundingBoxSample(Alembic::AbcGeom::OPolyMeshSchema::Sample& meshSample,
                               const boost::shared_ptr<const GPUCache::ShapeSample>& sample);
    void fillDiffuseColorSample(Alembic::AbcGeom::C4f& diffuseColorSample,
                                const boost::shared_ptr<const GPUCache::ShapeSample>& sample);

	Alembic::Abc::TimeSamplingPtr fTimeSampPtr;
    Alembic::AbcGeom::OPolyMeshSchema fAbcMesh;
	Alembic::Abc::OStringProperty     fAbcCreator;
	Alembic::Abc::OStringProperty     fAbcVersion;
	Alembic::Abc::OInt32ArrayProperty fAbcWireIndices;
	Alembic::Abc::OInt32ArrayProperty fAbcGroupSizes;
    Alembic::Abc::OC4fProperty        fAbcDiffuseColor;
	std::vector<int> fGroupSizes;
	std::vector<int> fPolygonCount;
    std::vector<Alembic::Abc::int32_t> fFaceIndices;
    
    std::vector<GPUCache::IndexBuffer::ReadableArrayPtr> fIndexReadInterfaces;
    std::vector<GPUCache::VertexBuffer::ReadableArrayPtr> fVertexReadInterfaces;

    Alembic::AbcGeom::OVisibilityProperty fVisibility;
    size_t                                fCachedWrite;
};


//==============================================================================
// CLASS MaterialGraphWriter
//==============================================================================

// Write a material graph to Alembic (OMaterial)
class MaterialGraphWriter : boost::noncopyable
{
public:
    MaterialGraphWriter(
        Alembic::Abc::OObject               parent,
        double                              secondsPerSample,
        double                              startTimeInSeconds,
        const GPUCache::MaterialGraph::Ptr& graph);

    // Write the material graph to the file.
    void write();

private:
    // Templated Write methods
    template<typename ABC_PROP>
    void setMaterialProperty(
        ABC_PROP& abcProp, 
        const GPUCache::MaterialProperty::Ptr& prop, 
        double timeInSeconds)
    {}

    template<typename ABC_PROP>
    void writeMaterialProperty(Alembic::Abc::OCompoundProperty& parent, 
                               const GPUCache::MaterialProperty::Ptr& prop)
    {
        ABC_PROP abcProp(
            parent,
            prop->name().asChar(),
            fTimeSampPtr
        );

        if (prop->isAnimated()) {
            // Animated property, we write to the last sample.
            double lastSampleTimeInSeconds = 
                (--prop->getSamples().end())->first + 0.5f * fSecondsPerSample;

            for (double time = fStartTimeInSeconds; 
                    time < lastSampleTimeInSeconds; 
                    time += fSecondsPerSample) {
                setMaterialProperty(abcProp, prop, time);
            }
        }
        else {
            // Static property, we just write one sample (sample 0)
            setMaterialProperty(abcProp, prop, 0.0);
        }
    }

    Alembic::AbcMaterial::OMaterialSchema fAbcMaterial;
    Alembic::Abc::TimeSamplingPtr         fTimeSampPtr;
    const double                          fSecondsPerSample;
    const double                          fStartTimeInSeconds;
    const GPUCache::MaterialGraph::Ptr    fGraph;
};

// Template explicit specialization must be in namespace scope.
template<>
inline void MaterialGraphWriter::setMaterialProperty(
    Alembic::Abc::OBoolProperty& abcProp, 
    const GPUCache::MaterialProperty::Ptr& prop, 
    double timeInSeconds)
{
    abcProp.set(prop->asBool(timeInSeconds));
}

template<>
inline void MaterialGraphWriter::setMaterialProperty(
    Alembic::Abc::OInt32Property& abcProp, 
    const GPUCache::MaterialProperty::Ptr& prop, 
    double timeInSeconds)
{
    abcProp.set(prop->asInt32(timeInSeconds));
}

template<>
inline void MaterialGraphWriter::setMaterialProperty(
    Alembic::Abc::OFloatProperty& abcProp, 
    const GPUCache::MaterialProperty::Ptr& prop, 
    double timeInSeconds)
{
    abcProp.set(prop->asFloat(timeInSeconds));
}

template<>
inline void MaterialGraphWriter::setMaterialProperty(
    Alembic::Abc::OV2fProperty& abcProp, 
    const GPUCache::MaterialProperty::Ptr& prop, 
    double timeInSeconds)
{
    Alembic::Abc::V2f value;
    prop->asFloat2(timeInSeconds, value.x, value.y);
    abcProp.set(value);
}

template<>
inline void MaterialGraphWriter::setMaterialProperty(
    Alembic::Abc::OV3fProperty& abcProp, 
    const GPUCache::MaterialProperty::Ptr& prop, 
    double timeInSeconds)
{
    Alembic::Abc::V3f value;
    prop->asFloat3(timeInSeconds, value.x, value.y, value.z);
    abcProp.set(value);
}

template<>
inline void MaterialGraphWriter::setMaterialProperty(
    Alembic::Abc::OC3fProperty& abcProp, 
    const GPUCache::MaterialProperty::Ptr& prop, 
    double timeInSeconds)
{
    MColor value = prop->asColor(timeInSeconds);
    abcProp.set(Alembic::Abc::C3f(value.r, value.g, value.b));
}

template<>
inline void MaterialGraphWriter::setMaterialProperty(
    Alembic::Abc::OWstringProperty& abcProp, 
    const GPUCache::MaterialProperty::Ptr& prop, 
    double timeInSeconds)
{
    MString value = prop->asString(timeInSeconds);
    abcProp.set(value.asWChar());
}

#endif
