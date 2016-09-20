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
// apiMeshGeometryOverride.h
//
// Handles vertex data preparation for drawing the user defined shape in
// Viewport 2.0.
//
////////////////////////////////////////////////////////////////////////////////

#include <maya/MPxGeometryOverride.h>
#include <maya/MIntArray.h>
#include <maya/MStateManager.h>
#include <maya/MTextureManager.h>
#include <set>

class apiMesh;
class apiMeshGeom;
namespace MHWRender
{
	class MIndexBuffer;
	class MVertexBuffer;
	class MVertexBufferDescriptor;
}

class apiMeshGeometryOverride : public MHWRender::MPxGeometryOverride
{
public:
	static MPxGeometryOverride* Creator(const MObject& obj)
	{
		return new apiMeshGeometryOverride(obj);
	}

	virtual ~apiMeshGeometryOverride();

	virtual MHWRender::DrawAPI supportedDrawAPIs() const;

	virtual void updateDG();
	virtual void updateRenderItems(
		const MDagPath& path,
		MHWRender::MRenderItemList& list);
	virtual void populateGeometry(
		const MHWRender::MGeometryRequirements& requirements,
        const MHWRender::MRenderItemList& renderItems,
		MHWRender::MGeometry& data);
	virtual void cleanUp();

	virtual void updateSelectionGranularity(
		const MDagPath& path,
		MHWRender::MSelectionContext& selectionContext);

	static MStatus registerComponentConverters();
	static MStatus deregisterComponentConverters();

protected:
	void printShader(MHWRender::MShaderInstance* shader);
	void setSolidColor(MHWRender::MShaderInstance* shaderInstance, const float *value);
	void setSolidPointSize(MHWRender::MShaderInstance* shaderInstance, float value);
	void setLineWidth(MHWRender::MShaderInstance* shaderInstance, float lineWidth);
	bool enableActiveComponentDisplay(const MDagPath &path) const;

	// Render item handling methods
	void updateDormantAndTemplateWireframeItems(const MDagPath& path, MHWRender::MRenderItemList& list, const MHWRender::MShaderManager* shaderMgr);
	void updateActiveWireframeItem(const MDagPath& path, MHWRender::MRenderItemList& list, const MHWRender::MShaderManager* shaderMgr);
	void updateWireframeModeFaceCenterItem(const MDagPath& path, MHWRender::MRenderItemList& list, const MHWRender::MShaderManager* shaderMgr);
	void updateShadedModeFaceCenterItem(const MDagPath& path, MHWRender::MRenderItemList& list, const MHWRender::MShaderManager* shaderMgr);
	void updateDormantVerticesItem(const MDagPath& path, MHWRender::MRenderItemList& list, const MHWRender::MShaderManager* shaderMgr);
	void updateActiveVerticesItem(const MDagPath& path, MHWRender::MRenderItemList& list, const MHWRender::MShaderManager* shaderMgr);
	void updateVertexNumericItems(const MDagPath& path, MHWRender::MRenderItemList& list, const MHWRender::MShaderManager* shaderMgr);
	void updateAffectedComponentItems(const MDagPath& path, MHWRender::MRenderItemList& list, const MHWRender::MShaderManager* shaderMgr);
	void updateSelectionComponentItems(const MDagPath& path, MHWRender::MRenderItemList& list, const MHWRender::MShaderManager* shaderMgr);
	void updateProxyShadedItem(const MDagPath& path, MHWRender::MRenderItemList& list, const MHWRender::MShaderManager* shaderMgr);

	// Data stream (geometry requirements) handling
	void updateGeometryRequirements(const MHWRender::MGeometryRequirements& requirements,
		MHWRender::MGeometry& data,
		unsigned int activeVertexCount,
		unsigned int totalVerts,
		bool debugPopulateGeometry);

	void cloneVertexBuffer(
		MHWRender::MVertexBuffer* srcBuffer,
		MHWRender::MGeometry& data,
		MHWRender::MVertexBufferDescriptor& desc,
		unsigned int bufferSize,
		bool debugPopulateGeometry);

	// Indexing for render item handling methods
	void updateIndexingForWireframeItems(MHWRender::MIndexBuffer* wireIndexBuffer,
		const MHWRender::MRenderItem* item,
		MHWRender::MGeometry& data,
		unsigned int totalVerts);
	void updateIndexingForDormantVertices(const MHWRender::MRenderItem* item, MHWRender::MGeometry& data, unsigned int numTriangles);
	void updateIndexingForFaceCenters(const MHWRender::MRenderItem* item, MHWRender::MGeometry& data, bool debugPopulateGeometry);
	void updateIndexingForVertices(const MHWRender::MRenderItem* item, 
		MHWRender::MGeometry& data,
		unsigned int numTriangles,
		unsigned int activeVertexCount,
		bool debugPopulateGeometry);
	void updateIndexingForEdges(const MHWRender::MRenderItem* item,
		MHWRender::MGeometry& data, 
		unsigned int totalVerts,
		bool fromSelection);
	void updateIndexingForFaces(const MHWRender::MRenderItem* item,
		MHWRender::MGeometry& data, 
		unsigned int numTriangles,
		bool fromSelection);
	void updateIndexingForShadedTriangles(const MHWRender::MRenderItem* item,
		MHWRender::MGeometry& data, 
		unsigned int numTriangles);

	apiMeshGeometryOverride(const MObject& obj);

	apiMesh* fMesh;
	apiMeshGeom* fMeshGeom;
	MIntArray fActiveVertices;
	std::set<int> fActiveVerticesSet;
	std::set<int> fActiveEdgesSet;
	std::set<int> fActiveFacesSet;
	bool fCastsShadows;
    bool fReceivesShadows;

	// Render item names
	static const MString sWireframeItemName;
	static const MString sShadedTemplateItemName;
	static const MString sSelectedWireframeItemName;
	static const MString sVertexItemName;
	static const MString sActiveVertexItemName;
	static const MString sVertexIdItemName;
	static const MString sVertexPositionItemName;
	static const MString sShadedModeFaceCenterItemName;
	static const MString sWireframeModeFaceCenterItemName;
	static const MString sShadedProxyItemName;
	static const MString sAffectedEdgeItemName;
	static const MString sAffectedFaceItemName;
	static const MString sEdgeSelectionItemName;
	static const MString sFaceSelectionItemName;

	// Stream names used for filling in different data
	// for different streams required for different render items,
	// and toggle to choose whether to use name streams
	//
	static const MString sActiveVertexStreamName;
	bool fDrawSharedActiveVertices;
	bool fDrawActiveVerticesWithRamp;
	MHWRender::MTexture *fColorRemapTexture;
	const MHWRender::MSamplerState* fLinearSampler;

	//Vertex stream for face centers which is calculated from faces.
	static const MString sFaceCenterStreamName;
	bool fDrawFaceCenters;

	// Custom color option
	bool fUseCustomColors;

	// Proxy shader. Fallback shader to use when no shader is assigned
	// if set to < 0 then will use a fragment shader
	int fProxyShader;

	// Test overrides on shaded mode render items
	bool fInternalItems_NoShadowCast;
	bool fInternalItems_NoShadowReceive;
	bool fInternalItems_NoPostEffects;
	bool fExternalItems_NoShadowCast;
	bool fExternalItems_NoShadowReceive;
	bool fExternalItems_NoPostEffects;
	bool fExternalItemsNonTri_NoShadowCast;
	bool fExternalItemsNonTri_NoShadowReceive;
	bool fExternalItemsNonTri_NoPostEffects;
};

