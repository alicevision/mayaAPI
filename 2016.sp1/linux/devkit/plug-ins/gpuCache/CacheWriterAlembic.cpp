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

#include "CacheWriterAlembic.h"
#include "CacheAlembicUtil.h"
#include "gpuCacheStrings.h"

#include <maya/MString.h>
#include <maya/MTime.h>
#include <maya/MGlobal.h>

#include <Alembic/AbcCoreHDF5/ReadWrite.h>
#include <Alembic/AbcCoreOgawa/ReadWrite.h>
#include <Alembic/AbcCoreAbstract/TimeSamplingType.h>
#include <Alembic/AbcGeom/ArchiveBounds.h>

#include <boost/foreach.hpp>

#include <cassert>

using namespace CacheAlembicUtil;

namespace {

using namespace GPUCache;


//==============================================================================
// LOCAL FUNCTIONS
//==============================================================================

    class SubNodeWriterVisitor : public SubNodeVisitor
    {
    public:

        SubNodeWriterVisitor(
            Alembic::Abc::OObject   parent,
            double                  secondsPerSample,
            double                  startTimeInSeconds
        ) 
            : fParent(parent),
              fSecondsPerSample(secondsPerSample),
              fStartTimeInSeconds(startTimeInSeconds),
              fMaxNumSamples(0)
        {}
        
        virtual void visit(const XformData&   xform,
                           const SubNode&     subNode)
        {
            AlembicXformWriter xformWriter(
                fParent, subNode.getName(), fSecondsPerSample, fStartTimeInSeconds);

            // The number of samples of this xform.
            fMaxNumSamples = 0;

            XformData::SampleMap::const_iterator it  =
                xform.getSamples().begin();
            XformData::SampleMap::const_iterator end =
                xform.getSamples().end();
            if (it != end) {
                boost::shared_ptr<const XformSample> sample = it->second;
                double time = fStartTimeInSeconds + 0.5f * fSecondsPerSample;
                xformWriter.write(sample);

                ++it;
                time += fSecondsPerSample;
                ++fMaxNumSamples;

                for (;it != end; ++it, time += fSecondsPerSample, ++fMaxNumSamples) {
                    const double nextTime = it->second->timeInSeconds();
                    while (time < nextTime) {
                        xformWriter.write(sample, sample);
                        time += fSecondsPerSample;
                        ++fMaxNumSamples;
                    }

                    boost::shared_ptr<const XformSample> prev = sample;
                    sample = it->second;
                    xformWriter.write(sample, prev);
                }
            }

            // Recurse into children sub nodes. Expand all instances.
            SubNodeWriterVisitor visitor(xformWriter.object(),
                                         fSecondsPerSample, fStartTimeInSeconds);
            BOOST_FOREACH(const SubNode::Ptr& child,
                          subNode.getChildren()) {
                child->accept(visitor);
                fMaxNumSamples = std::max(fMaxNumSamples, visitor.maxNumSamples());
            }
        }
        
        virtual void visit(const ShapeData&   shape,
                           const SubNode&     subNode)
        {
            AlembicMeshWriter meshWriter(
                fParent, subNode.getName(), fSecondsPerSample, fStartTimeInSeconds);

            // The number of samples of this shape.
            fMaxNumSamples = 0;

            ShapeData::SampleMap::const_iterator it  =
                shape.getSamples().begin();
            ShapeData::SampleMap::const_iterator end =
                shape.getSamples().end();
            if (it != end) {
                boost::shared_ptr<const ShapeSample> sample = it->second;
                double time = fStartTimeInSeconds + 0.5f * fSecondsPerSample;
                meshWriter.write(sample);

                ++it;
                time += fSecondsPerSample;
                ++fMaxNumSamples;

                for (;it != end; ++it, time += fSecondsPerSample, ++fMaxNumSamples) {
                    const double nextTime = it->second->timeInSeconds();
                    while (time < nextTime) {
                        meshWriter.write(sample, sample);
                        time += fSecondsPerSample;
                        ++fMaxNumSamples;
                    }

                    boost::shared_ptr<const ShapeSample> prev = sample;
                    sample = it->second;
                    meshWriter.write(sample, prev);
                }
            }

            // Write material assignment (only whole object assignment now)
            assert(shape.getMaterials().size() <= 1);
            if (!shape.getMaterials().empty() && shape.getMaterials()[0].length() > 0) {
                MString material = shape.getMaterials()[0];

                // Full IMaterial object path within the Alembic archive
                std::string materialAssignmentPath = "/";
                materialAssignmentPath += kMaterialsObject;
                materialAssignmentPath += "/";
                materialAssignmentPath += material.asChar();

                Alembic::AbcMaterial::addMaterialAssignment(
                    meshWriter.object(),
                    materialAssignmentPath
                );
            }
        }
        
        unsigned int maxNumSamples() const
        {
            // We are using the same time sampling for all properties.
            // Returns the max number of samples that can be used to compute
            // the end time: startTime + (numSamples-1) * secondsPerSample.
            return fMaxNumSamples;
        }

    private:
    
        Alembic::Abc::OObject   fParent;
        const double            fSecondsPerSample;
        const double            fStartTimeInSeconds;
        unsigned int            fMaxNumSamples;
    };


    // This class computes the archive bounds from a sub-node hierarchy.
    class ArchiveBoundsVisitor : public SubNodeVisitor
    {
    public:
        ArchiveBoundsVisitor(double timeInSeconds, const MMatrix& matrix)
            : fTimeInSeconds(timeInSeconds), fMatrix(matrix)
        {}
        
        virtual void visit(const XformData&   xform,
                           const SubNode&     subNode)
        {
            const boost::shared_ptr<const XformSample>& sample = 
                xform.getSample(fTimeInSeconds);
            if (!sample) return;

            const MMatrix matrix = sample->xform() * fMatrix;

            BOOST_FOREACH (const SubNode::Ptr& child, subNode.getChildren()) {
                ArchiveBoundsVisitor visitor(fTimeInSeconds, matrix);
                child->accept(visitor);
                fBoundingBox.expand(visitor.getBoundingBox());
            }
        }
        
        virtual void visit(const ShapeData&   shape,
                           const SubNode&     subNode)
        {
            const boost::shared_ptr<const ShapeSample>& sample = 
                shape.getSample(fTimeInSeconds);
            if (!sample) return;

            fBoundingBox = sample->boundingBox();
            fBoundingBox.transformUsing(fMatrix);
        }

        const MBoundingBox& getBoundingBox() const
        {
            return fBoundingBox;
        }

        static void computeArchiveBounds(const SubNode::Ptr&            topNode, 
                                         Alembic::Abc::TimeSamplingPtr& timeSampling,
                                         unsigned int                   maxNumSamples,
                                         std::vector<MBoundingBox>&     archiveBounds)
        {
            if (!topNode || !timeSampling || maxNumSamples == 0) return;

            // Match the number of samples.
            assert(maxNumSamples >= archiveBounds.size());
            if (maxNumSamples > archiveBounds.size()) {
                archiveBounds.resize(maxNumSamples, 
                    !archiveBounds.empty() ? archiveBounds.back() : MBoundingBox());
            }

            for (unsigned int i = 0; i < maxNumSamples; i++) {
                // The top-level bounding box of this sub-node hierarchy.
                double timeInSeconds = timeSampling->getSampleTime(i);
                ArchiveBoundsVisitor visitor(timeInSeconds, MMatrix::identity);
                topNode->accept(visitor);
                archiveBounds[i].expand(visitor.getBoundingBox());
            }
        }
        
    private:
        double       fTimeInSeconds;
        MMatrix      fMatrix;
        MBoundingBox fBoundingBox;
    };
}


//==============================================================================
// CLASS AlembicCacheWriter
//==============================================================================

boost::shared_ptr<CacheWriter> AlembicCacheWriter::create(
    const MFileObject& file,
    char compressLevel,
    const MString& dataFormat)
{
    return boost::make_shared<AlembicCacheWriter>(file, compressLevel, dataFormat);
}

AlembicCacheWriter::AlembicCacheWriter(const MFileObject& file,
        char compressLevel,
        const MString& dataFormat)
    : fFile(file), 
      fCompressLevel(compressLevel),
      fDataFormat(dataFormat),
      fMaxNumSamples(0)
{
    // open HDF5 archive for writing
    MString fileName = file.resolvedFullName();
    try {
        tbb::mutex::scoped_lock alembicLock(gsAlembicMutex);

        if (MString(dataFormat).toLowerCase() == "ogawa") {
            fAbcArchive = Alembic::Abc::OArchive(
                Alembic::AbcCoreOgawa::WriteArchive(),
                fileName.asChar(),
                Alembic::Abc::ErrorHandler::kThrowPolicy
            );
        }
        else
        {
            fAbcArchive = Alembic::Abc::OArchive(
                Alembic::AbcCoreHDF5::WriteArchive(),
                fileName.asChar(),
                Alembic::Abc::ErrorHandler::kThrowPolicy
            );
        }

        if (fAbcArchive.valid()) {
            // compress level, -1,0~9
            fAbcArchive.setCompressionHint(fCompressLevel);

            // update the file name, Alembic might rename the file
            std::string realName = fAbcArchive.getName();
            fFile.setRawFullName(realName.c_str());
        }
    }
    catch (std::exception& ex) {
        MStatus stat;
        MString msgFmt = MStringResource::getString(kOpenFileForWriteErrorMsg, stat);
        MString errorMsg;
        errorMsg.format(msgFmt, fileName, ex.what());
        MGlobal::displayError(errorMsg);
    }
}

AlembicCacheWriter::~AlembicCacheWriter()
{
    try {
        tbb::mutex::scoped_lock alembicLock(gsAlembicMutex);

        // Write meta data to archive.
        if (fAbcTimeSampling && fMaxNumSamples != 0) {
            // Attach *.samples property to indicate the max number of samples.
            unsigned int tsIndex = fAbcArchive.addTimeSampling(*fAbcTimeSampling);
            if (tsIndex != 0) {
                std::stringstream propName;
                propName << tsIndex << ".samples";
                Alembic::Abc::OUInt32Property samplesProp(
                    fAbcArchive.getTop().getProperties(), propName.str());
                samplesProp.set(fMaxNumSamples);
            }

            // Attach archive bounds property.
            Alembic::Abc::OBox3dProperty boxProp = 
                Alembic::AbcGeom::CreateOArchiveBounds(fAbcArchive, fAbcTimeSampling);
            for (unsigned int i = 0; i < fMaxNumSamples; i++) {
                const MPoint min = fArchiveBounds[i].min();
                const MPoint max = fArchiveBounds[i].max();
                boxProp.set(Alembic::Abc::Box3d(
                    Alembic::Abc::V3d(min.x, min.y, min.z),
                    Alembic::Abc::V3d(max.x, max.y, max.z)
                ));
            }
        }

        // close all handles
        fAbcArchive.reset();
    }
    catch (std::exception& ex) {
        MStatus stat;
        MString msgFmt = MStringResource::getString(kWriteAlembicErrorMsg, stat);
        MString errorMsg;
        errorMsg.format(msgFmt, fFile.resolvedFullName(), ex.what());
        MGlobal::displayError(errorMsg);
    }
}

bool AlembicCacheWriter::valid() const
{
    tbb::mutex::scoped_lock alembicLock(gsAlembicMutex);
    return fAbcArchive.valid();
}

void AlembicCacheWriter::writeSubNodeHierarchy(
    const SubNode::Ptr& topNode, 
    double secondsPerSample, double startTimeInSeconds)
{
	try {
		tbb::mutex::scoped_lock alembicLock(gsAlembicMutex);
        SubNodeWriterVisitor visitor(
            fAbcArchive.getTop(), secondsPerSample, startTimeInSeconds);
        topNode->accept(visitor);

        // We always use the same time sampling.
        if (!fAbcTimeSampling) {
            Alembic::Abc::TimeSamplingPtr ts = Alembic::Abc::TimeSamplingPtr(
                new Alembic::Abc::TimeSampling(secondsPerSample, startTimeInSeconds));
            unsigned int tsIndex = fAbcArchive.addTimeSampling(*ts);
            fAbcTimeSampling = fAbcArchive.getTimeSampling(tsIndex);
        }
        else {
            // Uniform
            assert(fAbcTimeSampling->getNumStoredTimes() == 1);
            assert(fAbcTimeSampling->getStoredTimes()[0] == startTimeInSeconds);
            assert(fAbcTimeSampling->getTimeSamplingType().getTimePerCycle() == secondsPerSample);
        }

        // The max number of samples for the time sampling.
        fMaxNumSamples = std::max(fMaxNumSamples, visitor.maxNumSamples());

        // The archive bounds.
        ArchiveBoundsVisitor::computeArchiveBounds(
            topNode, fAbcTimeSampling, fMaxNumSamples, fArchiveBounds);
	}
	catch (std::exception& ex) {
		MStatus stat;
		MString msgFmt = MStringResource::getString(kWriteAlembicErrorMsg, stat);
		MString errorMsg;
		errorMsg.format(msgFmt, fFile.resolvedFullName(), ex.what());
		MGlobal::displayError(errorMsg);
	}
}

void AlembicCacheWriter::writeMaterials(
    const MaterialGraphMap::Ptr& materialGraphMap,
    double secondsPerSample, double startTimeInSeconds)
{
    try {
        tbb::mutex::scoped_lock alembicLock(gsAlembicMutex);

        // We write all materials to /materials.
        // Maya doesn't support material hierarchy so we write a flat hierarchy.
        // As a result, we will have
        //     /materials/lambert1
        //     /materials/phong1

        // Create the "materials" object.
        Alembic::Abc::OObject materialsObject(fAbcArchive.getTop(), kMaterialsObject);

        // Loop over all material graphs and write them separately.
        assert(materialGraphMap);
        BOOST_FOREACH (
                const MaterialGraphMap::NamedMap::value_type& val,
                materialGraphMap->getGraphs()) {
            const MaterialGraph::Ptr& graph = val.second;
            assert(graph);

            // Write a material graph.
            MaterialGraphWriter writer(
                materialsObject, secondsPerSample, startTimeInSeconds, graph);
            writer.write();
        }
    }
    catch (std::exception& ex) {
        MStatus stat;
        MString msgFmt = MStringResource::getString(kWriteAlembicErrorMsg, stat);
        MString errorMsg;
        errorMsg.format(msgFmt, fFile.resolvedFullName(), ex.what());
        MGlobal::displayError(errorMsg);
    }
}

const MFileObject& AlembicCacheWriter::getFileObject() const
{
    return fFile;
}


//==============================================================================
// CLASS AlembicXformWriter
//==============================================================================

AlembicXformWriter::AlembicXformWriter(
    const Alembic::Abc::OObject& parent, const MString& name,
    double secondsPerSample, double startTimeInSeconds)
  : fCachedWrite(0)
{
    // determine the time between two samples and the start time
    fTimeSampPtr = Alembic::Abc::TimeSamplingPtr(
        new Alembic::Abc::TimeSampling(secondsPerSample, startTimeInSeconds));

    // create a xform object
    Alembic::AbcGeom::OXform xformObject(parent, name.asChar(), fTimeSampPtr);
    fAbcXform = xformObject.getSchema();
}

Alembic::Abc::OObject AlembicXformWriter::object()
{
    return fAbcXform.getObject();
}

void AlembicXformWriter::write(
    const boost::shared_ptr<const XformSample>&  sample)
{
    // create empty xform samples
    Alembic::AbcGeom::XformSample xformSample;

    // fill it
    fillXform(xformSample, sample);

    // write it
    fAbcXform.set(xformSample);

    // write visibility
    if (!sample->visibility()) {
        Alembic::Abc::OObject object = fAbcXform.getObject();
        fVisibility = Alembic::AbcGeom::CreateVisibilityProperty(object, fTimeSampPtr);
        fVisibility.set(char(Alembic::AbcGeom::kVisibilityHidden));
    }
    fCachedWrite++;

}

void AlembicXformWriter::write(
    const boost::shared_ptr<const XformSample>&  sample,
    const boost::shared_ptr<const XformSample>&  prev)
{
    if (!sample->xform().isEquivalent(prev->xform()))
    {
        // create empty xform samples
        Alembic::AbcGeom::XformSample xformSample;

        // fill it
        fillXform(xformSample, sample);

        // write it
        fAbcXform.set(xformSample);
    }
    else {
        // reuse the previous sample
        fAbcXform.setFromPrevious();
    }

    // write visibility
    if (!sample->visibility() && !fVisibility) {
        // create visibility property
        Alembic::Abc::OObject object = fAbcXform.getObject();
        fVisibility = Alembic::AbcGeom::CreateVisibilityProperty(object, fTimeSampPtr);

        // flush cached visibility sample
        for (size_t i = 0; i < fCachedWrite; i++) {
            fVisibility.set(char(Alembic::AbcGeom::kVisibilityDeferred));
        }
    }

    if (fVisibility) {
        if (sample->visibility() == prev->visibility()) {
            fVisibility.setFromPrevious();
        }
        else {
            fVisibility.set(char(sample->visibility() ? 
                Alembic::AbcGeom::kVisibilityDeferred : 
                Alembic::AbcGeom::kVisibilityHidden));
        }
    }
    fCachedWrite++;

}

void AlembicXformWriter::fillXform(
    Alembic::AbcGeom::XformSample&              xformSample,
    const boost::shared_ptr<const XformSample>& sample)
{
    // get the world transformation matrix
    const Alembic::Abc::M44d abcWorldMatrix(sample->xform().matrix);

    // setup Xform matrix operation
    Alembic::AbcGeom::XformOp opMatrix(Alembic::AbcGeom::kMatrixOperation,
                                       Alembic::AbcGeom::kMatrixHint);
    opMatrix.setMatrix(abcWorldMatrix);

    // add matrix operation to op stack
    xformSample.addOp(opMatrix);
}


//==============================================================================
// CLASS AlembicMeshWriter
//==============================================================================

AlembicMeshWriter::AlembicMeshWriter(
    const Alembic::Abc::OObject& parent,
    const MString& name, double secondsPerSample, double startTimeInSeconds)
  : fCachedWrite(0)
{
    // determine the time between two samples and the start time
    fTimeSampPtr = Alembic::Abc::TimeSamplingPtr(new Alembic::Abc::TimeSampling(secondsPerSample, startTimeInSeconds));

    // create poly mesh object
    Alembic::AbcGeom::OPolyMesh meshObject(parent, name.asChar(), fTimeSampPtr);
    fAbcMesh = meshObject.getSchema();

    // create custom property
	fAbcCreator = Alembic::Abc::OStringProperty(
		fAbcMesh.getPtr(), kCustomPropertyCreator, fTimeSampPtr);
	fAbcVersion = Alembic::Abc::OStringProperty(
		fAbcMesh.getPtr(), kCustomPropertyVersion, fTimeSampPtr);
	fAbcWireIndices = Alembic::Abc::OInt32ArrayProperty(
		fAbcMesh.getPtr(), kCustomPropertyWireIndices, fTimeSampPtr);
    fAbcDiffuseColor = Alembic::Abc::OC4fProperty(
        fAbcMesh.getPtr(), kCustomPropertyDiffuseColor, fTimeSampPtr);
}

Alembic::Abc::OObject AlembicMeshWriter::object()
{
	return fAbcMesh.getObject();
}

void AlembicMeshWriter::write(
    const boost::shared_ptr<const ShapeSample>&  sample)
{
    // create empty mesh samples
    Alembic::AbcGeom::OPolyMeshSchema::Sample meshSample;
	Alembic::Abc::Int32ArraySample wireIndicesSample;
	Alembic::Abc::Int32ArraySample groupSizesSample;
    Alembic::Abc::C4f              diffuseColorSample;

    // associate samples with arrays
    fillWireframeSample(wireIndicesSample, sample);
    fillTriangleSample(meshSample, groupSizesSample, sample);
    fillPositionSample(meshSample, sample);
    fillNormalSample(meshSample, sample, false);
    fillUVSample(meshSample, sample, false);
    fillBoundingBoxSample(meshSample, sample);
    fillDiffuseColorSample(diffuseColorSample, sample);

    // store the sample!
    fAbcMesh.set(meshSample);
	fAbcCreator.set(kCustomPropertyCreatorValue);
	fAbcVersion.set(kCustomPropertyVersionValue);
    fAbcWireIndices.set(wireIndicesSample);
	if (groupSizesSample.size() > 1) {
		fAbcGroupSizes = Alembic::Abc::OInt32ArrayProperty(
			fAbcMesh.getPtr(), kCustomPropertyShadingGroupSizes, fTimeSampPtr);
		fAbcGroupSizes.set(groupSizesSample);
	}
    fAbcDiffuseColor.set(diffuseColorSample);

    // write visibility
    if (!sample->visibility()) {
        Alembic::Abc::OObject object = fAbcMesh.getObject();
        fVisibility = Alembic::AbcGeom::CreateVisibilityProperty(object, fTimeSampPtr);
        fVisibility.set(char(Alembic::AbcGeom::kVisibilityHidden));
    }
    fCachedWrite++;
}

void AlembicMeshWriter::write(
    const boost::shared_ptr<const ShapeSample>&  sample,
    const boost::shared_ptr<const ShapeSample>&  prev)
{
    // create empty mesh samples
    Alembic::AbcGeom::OPolyMeshSchema::Sample meshSample;
    Alembic::Abc::Int32ArraySample wireIndicesSample;
	Alembic::Abc::Int32ArraySample groupSizesSample;
    Alembic::Abc::C4f              diffuseColorSample;
    
    // associate samples with arrays
    if (sample->wireVertIndices() != prev->wireVertIndices()) {
        fillWireframeSample(wireIndicesSample, sample);
    }

	assert(sample->numIndexGroups() == prev->numIndexGroups());
	for(size_t i =0; i<sample->numIndexGroups() && i<prev->numIndexGroups(); ++i) {
		if (sample->triangleVertIndices(i) != prev->triangleVertIndices(i)) {
			fillTriangleSample(meshSample, groupSizesSample, sample);
			break;
		}
	}

    if (sample->positions() != prev->positions()) {
        fillPositionSample(meshSample, sample);
    }

    if (sample->normals() != prev->normals()) {
        fillNormalSample(meshSample, sample, prev->normals().get() != NULL);
    }
    
    if (sample->uvs() != prev->uvs()) {
        fillUVSample(meshSample, sample, prev->uvs().get() != NULL);
    }

    MBoundingBox boundingBox     = sample->boundingBox();
    MBoundingBox prevBoundingBox = prev->boundingBox();
    if (!boundingBox.min().isEquivalent(prevBoundingBox.min()) ||
        !boundingBox.max().isEquivalent(prevBoundingBox.max())) {
        fillBoundingBoxSample(meshSample, sample);
    }

    // store the sample!
    fAbcMesh.set(meshSample);
	Alembic::AbcGeom::SetPropUsePrevIfNull(fAbcWireIndices, wireIndicesSample);

    if (sample->diffuseColor() != prev->diffuseColor()) {
        fillDiffuseColorSample(diffuseColorSample, sample);
        fAbcDiffuseColor.set(diffuseColorSample);
    }

    // write visibility
    if (!sample->visibility() && !fVisibility) {
        // create visibility property
        Alembic::Abc::OObject object = fAbcMesh.getObject();
        fVisibility = Alembic::AbcGeom::CreateVisibilityProperty(object, fTimeSampPtr);

        // flush cached visibility sample
        for (size_t i = 0; i < fCachedWrite; i++) {
            fVisibility.set(char(Alembic::AbcGeom::kVisibilityDeferred));
        }
    }

    if (fVisibility) {
        if (sample->visibility() == prev->visibility()) {
            fVisibility.setFromPrevious();
        }
        else {
            fVisibility.set(char(sample->visibility() ? 
                Alembic::AbcGeom::kVisibilityDeferred : 
                Alembic::AbcGeom::kVisibilityHidden));
        }
    }
    fCachedWrite++;
}

void AlembicMeshWriter::fillWireframeSample(
    Alembic::Abc::Int32ArraySample& wireIndicesSample,
    const boost::shared_ptr<const ShapeSample>& sample)
{
    // Wrap the wireframe index sample, no memcpy occurs if the source array is already
    // readable.  If it's a non-readable buffer, then it will be copied into temporary
    // storage.  The raw data isn't accessed until the end of this process, so the
    // temporary buffer will be kept alive until the AlembicMeshWriter is destroyed.
    // Wireframe index is stored as custom property.
    const int* wireIndexArray = NULL;
    IndexBuffer::ReadableArrayPtr readable;
    if (sample->wireVertIndices()) {
        readable = sample->wireVertIndices()->array()->getReadableArray();
        fIndexReadInterfaces.push_back(readable);
        wireIndexArray = (const int*)readable->get();
    }
    size_t wireIndexCount = sample->numWires() * 2;
    wireIndicesSample = Alembic::Abc::Int32ArraySample(wireIndexArray, wireIndexCount);
}

void AlembicMeshWriter::fillTriangleSample(
    Alembic::AbcGeom::OPolyMeshSchema::Sample& meshSample,
	Alembic::Abc::Int32ArraySample& groupSizesSample,
	const boost::shared_ptr<const ShapeSample>& sample)
{
    size_t numTriangles = 0;
	for(size_t i=0; i<sample->numIndexGroups(); ++i) {
		numTriangles += sample->numTriangles(i);
		fGroupSizes.push_back((int)sample->numTriangles(i));
	}

	// wrap the group info in custom property.
	groupSizesSample = Alembic::Abc::Int32ArraySample(fGroupSizes);

	// re-allocate polygon count array
	// all polygons are triangles
    fPolygonCount.resize(numTriangles, 3);

    // wrap the polygon count sample, no memcpy occur
    meshSample.setFaceCounts(Alembic::Abc::Int32ArraySample(fPolygonCount));

    // merge index groups and convert polygon winding from CCW to CW
	std::vector<Alembic::Abc::int32_t> faceIndices(numTriangles*3);
	int* faceIndicesCW = numTriangles>0 ? (int*)&(faceIndices[0]) : NULL;
	for(size_t i=0; i<sample->numIndexGroups(); ++i) {
        IndexBuffer::ReadableArrayPtr readable;
        const int* faceIndicesCCW = NULL;
        if (sample->triangleVertIndices(i)) {
            readable = sample->triangleVertIndices(i)->array()->getReadableArray();
            faceIndicesCCW = (const int* )readable->get();
        }
		size_t indicesCount = sample->numTriangles(i)*3;
		for (size_t offset=0; offset<indicesCount; faceIndicesCW+=3, offset+=3) {
			faceIndicesCW[0] = faceIndicesCCW[offset + 2];
			faceIndicesCW[1] = faceIndicesCCW[offset + 1];
			faceIndicesCW[2] = faceIndicesCCW[offset + 0];
		}
	}

    // wrap the index sample, no memcpy occur
    fFaceIndices.swap(faceIndices);
    meshSample.setFaceIndices(Alembic::Abc::Int32ArraySample(fFaceIndices));
}

void AlembicMeshWriter::fillPositionSample(
    Alembic::AbcGeom::OPolyMeshSchema::Sample& meshSample,
    const boost::shared_ptr<const ShapeSample>& sample)
{
    // Wrap the position sample, no memcpy occurs if the source array is already
    // readable.  If it's a non-readable buffer, then it will be copied into temporary
    // storage.  The raw data isn't accessed until the end of this process, so the
    // temporary buffer will be kept alive until the AlembicMeshWriter is destroyed.
    VertexBuffer::ReadableArrayPtr readable;
    const Alembic::Abc::V3f* positionArray = NULL;
    if (sample->positions()) {
        readable = sample->positions()->array()->getReadableArray();
        fVertexReadInterfaces.push_back(readable);
        positionArray = (const Alembic::Abc::V3f*)readable->get();
    }
    size_t positionCount = sample->numVerts();
    meshSample.setPositions(Alembic::Abc::P3fArraySample(positionArray, positionCount));
}

void AlembicMeshWriter::fillNormalSample(
    Alembic::AbcGeom::OPolyMeshSchema::Sample& meshSample,
    const boost::shared_ptr<const ShapeSample>& sample,
    bool forceWrite)
{
    // We have 3 state here:
    // 1) Set normals 
    // 2) Set normals with a NULL array
    // 3) Not set anything

    // Alembic will write normals to file in case 1) and 2)
    // In case 3), Alembic will reuse the previous sample.
    // If forceWrite is true, we write a zero-length array (case 2)

    if (sample->normals()) {
        Alembic::AbcGeom::ON3fGeomParam::Sample normalSample;
        normalSample.setScope(Alembic::AbcGeom::kVertexScope);
        
        // Wrap the normal sample, no memcpy occurs if the source array is already
        // readable.  If it's a non-readable buffer, then it will be copied into temporary
        // storage.  The raw data isn't accessed until the end of this process, so the
        // temporary buffer will be kept alive until the AlembicMeshWriter is destroyed.
        VertexBuffer::ReadableArrayPtr readable = sample->normals()->array()->getReadableArray();
        fVertexReadInterfaces.push_back(readable);
        const Alembic::Abc::N3f* normalArray =
            (const Alembic::Abc::N3f*)(readable->get());
        size_t normalCount = sample->numVerts();

        normalSample.setVals(
            Alembic::AbcGeom::N3fArraySample(normalArray, normalCount));

        meshSample.setNormals(normalSample);
    }
    else if (forceWrite) {
        Alembic::AbcGeom::ON3fGeomParam::Sample normalSample;
        normalSample.setScope(Alembic::AbcGeom::kVertexScope);

        // We need to explicitly pass a 0-length array so that Alembic
        // will write a 0-length array instead of use prev sample
        normalSample.setVals(
            Alembic::AbcGeom::N3fArraySample(NULL, 0));

        meshSample.setNormals(normalSample);
    }
}

void AlembicMeshWriter::fillUVSample(
    Alembic::AbcGeom::OPolyMeshSchema::Sample& meshSample,
    const boost::shared_ptr<const ShapeSample>& sample,
    bool forceWrite)
{
    if (sample->uvs()) {
        Alembic::AbcGeom::OV2fGeomParam::Sample uvSample;
        uvSample.setScope(Alembic::AbcGeom::kVertexScope);
        
        // Wrap the uv sample, no memcpy occurs if the source array is already
        // readable.  If it's a non-readable buffer, then it will be copied into temporary
        // storage.  The raw data isn't accessed until the end of this process, so the
        // temporary buffer will be kept alive until the AlembicMeshWriter is destroyed.
        VertexBuffer::ReadableArrayPtr readable = sample->uvs()->array()->getReadableArray();
        fVertexReadInterfaces.push_back(readable);
        const Alembic::Abc::V2f* uvArray =
            (const Alembic::Abc::V2f*)(readable->get());
        size_t uvCount = sample->numVerts();

        uvSample.setVals(Alembic::AbcGeom::V2fArraySample(uvArray, uvCount));

        meshSample.setUVs(uvSample);
    }
    else if (forceWrite) {
        Alembic::AbcGeom::OV2fGeomParam::Sample uvSample;
        uvSample.setScope(Alembic::AbcGeom::kVertexScope);
        
        // We need to explicitly pass a 0-length array so that Alembic
        // will write a 0-length array instead of use prev sample
        uvSample.setVals(Alembic::AbcGeom::V2fArraySample(NULL, 0));

        meshSample.setUVs(uvSample);
    }
}

void AlembicMeshWriter::fillBoundingBoxSample(
    Alembic::AbcGeom::OPolyMeshSchema::Sample& meshSample,
    const boost::shared_ptr<const ShapeSample>& sample)
{
    // get bounding box sample
    const MBoundingBox boundingBox = sample->boundingBox();
    const MPoint min = boundingBox.min();
    const MPoint max = boundingBox.max();

    // copy the bounding box
    Alembic::Abc::Box3d selfBounds(
        Alembic::Abc::V3d(min.x, min.y, min.z),
        Alembic::Abc::V3d(max.x, max.y, max.z));
    meshSample.setSelfBounds(selfBounds);
}

void AlembicMeshWriter::fillDiffuseColorSample(
    Alembic::AbcGeom::C4f& diffuseColorSample,
    const boost::shared_ptr<const ShapeSample>& sample)
{
  const MColor diffuseColor = sample->diffuseColor();
  diffuseColorSample.r = diffuseColor.r;
  diffuseColorSample.g = diffuseColor.g;
  diffuseColorSample.b = diffuseColor.b;
  diffuseColorSample.a = diffuseColor.a;
}


//==============================================================================
// CLASS MaterialGraphWriter
//==============================================================================

MaterialGraphWriter::MaterialGraphWriter(
    Alembic::Abc::OObject     parent,
    double                    secondsPerSample,
    double                    startTimeInSeconds,
    const MaterialGraph::Ptr& graph
)
    : fSecondsPerSample(secondsPerSample),
      fStartTimeInSeconds(startTimeInSeconds),
      fGraph(graph)
{
    assert(fGraph);

    // Create the time sampling for this material object and all its properties.
    fTimeSampPtr = Alembic::Abc::TimeSamplingPtr(
        new Alembic::Abc::TimeSampling(secondsPerSample, startTimeInSeconds));

    // Create an OMaterial object
    Alembic::AbcMaterial::OMaterial materialObject(
        parent, graph->name().asChar(), fTimeSampPtr);
    fAbcMaterial = materialObject.getSchema();
}

void MaterialGraphWriter::write()
{
    // Add shading nodes to the OMaterial
    BOOST_FOREACH (const MaterialGraph::NamedMap::value_type& val, fGraph->getNodes()) {
        std::string name = val.second->name().asChar();
        std::string type = val.second->type().asChar();
        fAbcMaterial.addNetworkNode(name, kMaterialsGpuCacheTarget, type);
    }

    // Write properties
    BOOST_FOREACH (const MaterialGraph::NamedMap::value_type& val, fGraph->getNodes()) {
        const MaterialNode::Ptr& node = val.second;

        // Get the Alembic's parent compound property.
        Alembic::Abc::OCompoundProperty abcCompoundProp = 
            fAbcMaterial.getNetworkNodeParameters(node->name().asChar());
        assert(abcCompoundProp.valid());

        // Loop over properties
        BOOST_FOREACH (
                const MaterialNode::PropertyMap::value_type& val, 
                node->properties()) {
            const MaterialProperty::Ptr& prop = val.second;

            switch (prop->type()) {
            case MaterialProperty::kBool:
                writeMaterialProperty<Alembic::Abc::OBoolProperty>(abcCompoundProp, prop);
                break;
            case MaterialProperty::kInt32:
                writeMaterialProperty<Alembic::Abc::OInt32Property>(abcCompoundProp, prop);
                break;
            case MaterialProperty::kFloat:
                writeMaterialProperty<Alembic::Abc::OFloatProperty>(abcCompoundProp, prop);
                break;
            case MaterialProperty::kFloat2:
                writeMaterialProperty<Alembic::Abc::OV2fProperty>(abcCompoundProp, prop);
                break;
            case MaterialProperty::kFloat3:
                writeMaterialProperty<Alembic::Abc::OV3fProperty>(abcCompoundProp, prop);
                break;
            case MaterialProperty::kRGB:
                writeMaterialProperty<Alembic::Abc::OC3fProperty>(abcCompoundProp, prop);
                break;
            case MaterialProperty::kString:
                // use wide char ?
                writeMaterialProperty<Alembic::Abc::OWstringProperty>(abcCompoundProp, prop);
                break;
            default:
                assert(0);
                break;
            }
        }
    }

    // Add connections to the OMaterial
    BOOST_FOREACH (const MaterialGraph::NamedMap::value_type& val, fGraph->getNodes()) {
        const MaterialNode::Ptr& node     = val.second;
        std::string              nodeName = node->name().asChar();

        // Loop over properties and write source connection.
        BOOST_FOREACH (
                const MaterialNode::PropertyMap::value_type& val, 
                node->properties()) {
            const MaterialProperty::Ptr& prop    = val.second;
            const MaterialNode::Ptr      srcNode = prop->srcNode();
            const MaterialProperty::Ptr  srcProp = prop->srcProp();
            if (srcNode && srcProp) {
                // Found a connected property.
                std::string propName    = prop->name().asChar();
                std::string srcNodeName = srcNode->name().asChar();
                std::string srcPropName = srcProp->name().asChar();

                // Write the connection.
                fAbcMaterial.setNetworkNodeConnection(
                    nodeName,
                    propName,
                    srcNodeName,
                    srcPropName
                );
            }
        }
    }

    // Write the root (terminal) node.
    const MaterialNode::Ptr& rootNode = fGraph->rootNode();
    if (rootNode) {
        fAbcMaterial.setNetworkTerminal(
            kMaterialsGpuCacheTarget, 
            kMaterialsGpuCacheType,
            rootNode->name().asChar()
        );
    }
}


