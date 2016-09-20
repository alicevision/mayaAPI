//-
// ==========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

///////////////////////////////////////////////////////////////////////////////
//
// apiMeshSubSceneOverride.h
//
// Handles vertex data preparation for drawing the user defined shape in
// Viewport 2.0.
//
////////////////////////////////////////////////////////////////////////////////

#include <maya/MPxSubSceneOverride.h>
#include <map>
#include <set>
#include <vector>

namespace MHWRender {
	class MVertexBuffer;
	class MIndexBuffer;
	class MShaderInstance;
}

class apiMesh;
class apiMeshGeom;
struct ID3D11Buffer;
namespace apiMeshSubSceneOverrideHelpers {
	class ShadedItemUserData;
}

class apiMeshSubSceneOverride : public MHWRender::MPxSubSceneOverride
{
public:
	static MPxSubSceneOverride* Creator(const MObject& obj)
	{
		return new apiMeshSubSceneOverride(obj);
	}

	virtual ~apiMeshSubSceneOverride();

	virtual MHWRender::DrawAPI supportedDrawAPIs() const;

	virtual bool requiresUpdate(
		const MHWRender::MSubSceneContainer& container,
		const MHWRender::MFrameContext& frameContext) const;
	virtual void update(
		MHWRender::MSubSceneContainer& container,
		const MHWRender::MFrameContext& frameContext);
	virtual bool furtherUpdateRequired(
		const MHWRender::MFrameContext& frameContext);

	virtual bool areUIDrawablesDirty() const;
	virtual bool hasUIDrawables() const;
	virtual void addUIDrawables(
		MHWRender::MUIDrawManager& drawManager,
		const MHWRender::MFrameContext& frameContext);

	virtual void updateSelectionGranularity(
		const MDagPath& path,
		MHWRender::MSelectionContext& selectionContext);

	void untrackLinkLostData(apiMeshSubSceneOverrideHelpers::ShadedItemUserData* data);

	static MStatus registerComponentConverters();
	static MStatus deregisterComponentConverters();

private:
	apiMeshSubSceneOverride(const MObject& obj);

	void manageRenderItems(
		MHWRender::MSubSceneContainer& container,
		const MHWRender::MFrameContext& frameContext,
		bool updateGeometry);

	void rebuildGeometryBuffers();
	void rebuildActiveComponentIndexBuffers();

	void deleteBuffers();
	void deleteGeometryBuffers();
	void deleteActiveComponentIndexBuffers();

	static void shadedItemLinkLost(MUserData* userData);

	MObject fObject;
	apiMesh* fMesh;

    struct InstanceInfo
    {
        MMatrix fTransform;
        bool fIsSelected;

        InstanceInfo() : fIsSelected(false) {}
        InstanceInfo(const MMatrix& matrix, bool selected) : fTransform(matrix), fIsSelected(selected) {}
    };
    typedef std::map<unsigned int, InstanceInfo> InstanceInfoMap;
	InstanceInfoMap fInstanceInfoCache;

	static const MString sWireName;
	static const MString sSelectName;
	static const MString sBoxName;
	static const MString sSelectedBoxName;
	static const MString sShadedName;

	static const MString sVertexSelectionName;
	static const MString sEdgeSelectionName;
	static const MString sFaceSelectionName;

	static const MString sActiveVertexName;
	static const MString sActiveEdgeName;
	static const MString sActiveFaceName;

	MHWRender::MShaderInstance* fWireShader;
	MHWRender::MShaderInstance* fThickWireShader;
	MHWRender::MShaderInstance* fSelectShader;
	MHWRender::MShaderInstance* fThickSelectShader;
	MHWRender::MShaderInstance* fShadedShader;
	MHWRender::MShaderInstance* fVertexComponentShader;
	MHWRender::MShaderInstance* fEdgeComponentShader;
	MHWRender::MShaderInstance* fFaceComponentShader;

	MHWRender::MVertexBuffer* fPositionBuffer;
	MHWRender::MVertexBuffer* fNormalBuffer;
	MHWRender::MVertexBuffer* fBoxPositionBuffer;
	MHWRender::MIndexBuffer* fWireIndexBuffer;
	MHWRender::MIndexBuffer* fVertexIndexBuffer;
	MHWRender::MIndexBuffer* fBoxIndexBuffer;
	MHWRender::MIndexBuffer* fShadedIndexBuffer;
	MHWRender::MIndexBuffer* fActiveVerticesIndexBuffer;
	MHWRender::MIndexBuffer* fActiveEdgesIndexBuffer;
	MHWRender::MIndexBuffer* fActiveFacesIndexBuffer;

	// Client buffers
	unsigned int fBoxPositionBufferId;
	unsigned int fBoxIndexBufferId;
	ID3D11Buffer* fBoxPositionBufferDX;
	ID3D11Buffer* fBoxIndexBufferDX;

    float fThickLineWidth;
    unsigned int fNumInstances;
    bool fIsInstanceMode;

	// Variables to control sample queue of updates to
	// allow for line width to increase incrementally without
	// user control.
	bool fUseQueuedLineUpdate;
	float fQueuedLineWidth;
	bool fQueueUpdate;

	std::set<int> fActiveVerticesSet;
	std::set<int> fActiveEdgesSet;
	std::set<int> fActiveFacesSet;
	std::vector<apiMeshSubSceneOverrideHelpers::ShadedItemUserData*> fLinkLostCallbackData;
};

