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

// Plugin: geometryReplicator.cpp
//
// This plugin demonstrates the use of MGeometryExtractor.
// We implement the GeometryOverride of geometryReplicator to extract the geometry data
// from a linked scene object of this plugin node, so the plugin node is rendered the same
// as its linked object.
//
// The supported data includes vertex position, normal, color, uv, tangent and bitangent.
//
// Detailed usage:
// 1. create a polygon (or NURBS surface / NURBS curve / Bezier Curve), and add an attribute named "extractorLink".
// 2. create a node of "geometryReplicator"; connect the "message" attribute to the "extractorLink" attribute of the scene object above.
// 3. if the scene object is a polygon or NURBS surface, assign a material to the geometryReplicator node.
// 4. refresh to see that the "geometryReplicator" node shows the same geometry as its associated scene object.

#include <maya/MPxSurfaceShape.h>
#include <maya/MPxSurfaceShapeUI.h>
#include <maya/MFnPlugin.h>
#include <maya/MPxGeometryOverride.h>
#include <maya/MDagPath.h>
#include <maya/MGeometryExtractor.h>
#include <maya/MDrawRegistry.h>
#include <maya/MFnData.h>
#include <maya/MItSelectionList.h>
#include <maya/MGlobal.h>
#include <maya/MPlugArray.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MShaderManager.h>
#include <maya/MFnNumericAttribute.h>

using namespace MHWRender;

class geometryReplicator : public MPxSurfaceShape
{
public:
	geometryReplicator();
	virtual ~geometryReplicator();

	virtual void postConstructor();

	virtual bool            isBounded() const;
	virtual MBoundingBox    boundingBox() const;

	static  void *          creator();
	static  MStatus         initialize();

	// attributes
	static  MObject			aShowCPV;
	static  MObject			aBaseMesh;

public:
	static	MTypeId		id;
	static	MString		drawDbClassification;
	static	MString		drawRegistrantId;
};

MObject geometryReplicator::aShowCPV;
MObject geometryReplicator::aBaseMesh;
MTypeId geometryReplicator::id(0x00080029);
MString	geometryReplicator::drawDbClassification("drawdb/geometry/geometryReplicator");
MString	geometryReplicator::drawRegistrantId("geometryReplicatorPlugin");

geometryReplicator::geometryReplicator()
{

}

geometryReplicator::~geometryReplicator() {}

bool geometryReplicator::isBounded() const
{
	return true;
}

MBoundingBox geometryReplicator::boundingBox() const
{
	MPoint corner1( -0.5, 0.0, -0.5 );
	MPoint corner2( 0.5, 0.0, 0.5 );

	return MBoundingBox( corner1, corner2 );
}

void* geometryReplicator::creator()
{
	return new geometryReplicator();
}

void geometryReplicator::postConstructor()
{
	setRenderable(true);
}

///////////////////////////////////////////////////////////////////////////////
//
// geometryReplicatorShapeUI
//
// There's no need to draw and select this node in vp1.0, so this class
// doesn't override draw(), select(), etc. But the creator() is needed for
// plugin registration and avoid crash in cases (e.g., RB pop up menu of this node).
//
////////////////////////////////////////////////////////////////////////////////

class geometryReplicatorShapeUI : public MPxSurfaceShapeUI
{
public:

	static void* creator();

	geometryReplicatorShapeUI();
	~geometryReplicatorShapeUI();

private:
	// Prohibited and not implemented.
	geometryReplicatorShapeUI(const geometryReplicatorShapeUI& obj);
	const geometryReplicatorShapeUI& operator=(const geometryReplicatorShapeUI& obj);
};

void* geometryReplicatorShapeUI::creator()
{
	return new geometryReplicatorShapeUI;
}

geometryReplicatorShapeUI::geometryReplicatorShapeUI()
{}

geometryReplicatorShapeUI::~geometryReplicatorShapeUI()
{}


///////////////////////////////////////////////////////////////////////////////
//
// geometryReplicatorGeometryOverride.h
//
// Handles vertex data preparation for drawing the user defined shape in
// Viewport 2.0.
//
////////////////////////////////////////////////////////////////////////////////
class geometryReplicatorGeometryOverride : public MPxGeometryOverride
{
public:
	static MPxGeometryOverride* Creator(const MObject& obj)
	{
		return new geometryReplicatorGeometryOverride(obj);
	}

	virtual ~geometryReplicatorGeometryOverride();

	virtual DrawAPI supportedDrawAPIs() const;

	virtual bool hasUIDrawables() const;

	virtual void updateDG();

	virtual void updateRenderItems(
		const MDagPath& path,
		MRenderItemList& list);

	virtual void addUIDrawables(
		const MDagPath& path,
		MUIDrawManager& drawManager,
		const MFrameContext& frameContext);

	virtual void populateGeometry(
		const MGeometryRequirements& requirements,
		const MRenderItemList& renderItems,
		MGeometry& data);

	virtual void cleanUp();

protected:
	geometryReplicatorGeometryOverride(const MObject& obj);

	bool isCPVShown();
	bool isBaseMesh();

	MObject		fThisNode;

	MDagPath	fPath;	// the associated object path.
	MFn::Type	fType;	// the type of the associated object.
};

geometryReplicatorGeometryOverride::geometryReplicatorGeometryOverride(const MObject& obj)
	: MPxGeometryOverride(obj),
	fThisNode(obj),
	fType(MFn::kInvalid)
{
}

geometryReplicatorGeometryOverride::~geometryReplicatorGeometryOverride()
{

}

DrawAPI geometryReplicatorGeometryOverride::supportedDrawAPIs() const
{
	// this plugin supports both GL and DX
	return (kOpenGL | kDirectX11 | kOpenGLCoreProfile);
}

void geometryReplicatorGeometryOverride::updateDG()
{
	if (!fPath.isValid()) {
		MFnDependencyNode fnThisNode(fThisNode);

		MObject messageAttr = fnThisNode.attribute("message");
		MPlug messagePlug(fThisNode, messageAttr);

		MPlugArray connections;
		if (messagePlug.connectedTo(connections, false, true)) {
			for (unsigned int i = 0; i < connections.length(); ++i) {
				MObject node = connections[i].node();
				if (node.hasFn(MFn::kMesh) ||
					node.hasFn(MFn::kNurbsSurface) ||
					node.hasFn(MFn::kNurbsCurve) ||
					node.hasFn(MFn::kBezierCurve))
				{
					MDagPath path;
					MDagPath::getAPathTo(node, path);

					fPath = path;
					fType = path.apiType();

					break;
				}
			}
		}
	}
}

bool geometryReplicatorGeometryOverride::isCPVShown()
{
	bool res = false;
	MPlug plug(fThisNode, geometryReplicator::aShowCPV);
	if (!plug.isNull())
	{
		if (plug.getValue(res) != MStatus::kSuccess)
		{
			res = false;
		}
	}
	return res;
}

void geometryReplicatorGeometryOverride::updateRenderItems(
	const MDagPath& path,
	MRenderItemList& list)
{
	if (!fPath.isValid())
		return;
	MRenderer* renderer = MRenderer::theRenderer();
	if (!renderer)
		return;
	const MShaderManager* shaderManager = renderer->getShaderManager();
	if (!shaderManager)
		return;

	if (fType == MFn::kNurbsCurve || fType == MFn::kBezierCurve)
	{
		// add render items for drawing curve
		MRenderItem* curveItem = NULL;
		int index = list.indexOf("geometryReplicatorCurve");
		if (index < 0)
		{
			curveItem = MRenderItem::Create("geometryReplicatorCurve", MRenderItem::NonMaterialSceneItem, MGeometry::kLines);
			curveItem->setDrawMode(MGeometry::kAll);
			list.append(curveItem);

			MShaderInstance* shader = shaderManager->getStockShader(MShaderManager::k3dSolidShader);
			if (shader) {
				static const float theColor[] = {1.0f, 0.0f, 0.0f, 1.0f};
				shader->setParameter("solidColor", theColor);

				curveItem->setShader(shader);
				shaderManager->releaseShader(shader);
			}
		}
		else {
			curveItem = list.itemAt(index);
		}
		if (curveItem) {
			curveItem->enable(true);
		}
	}
	else if (fType == MFn::kMesh)
	{
		// add render item for drawing wireframe on the mesh
		MRenderItem* wireframeItem = NULL;
		int index = list.indexOf("geometryReplicatorWireframe");
		if (index < 0)
		{
			wireframeItem = MRenderItem::Create("geometryReplicatorWireframe", MRenderItem::DecorationItem, MGeometry::kLines);
			wireframeItem->setDrawMode(MGeometry::kWireframe);
			wireframeItem->depthPriority(MRenderItem::sActiveWireDepthPriority);
			list.append(wireframeItem);

			MShaderInstance* shader = shaderManager->getStockShader(MShaderManager::k3dSolidShader);
			if (shader) {
				static const float theColor[] = {1.0f, 0.0f, 0.0f, 1.0f};
				shader->setParameter("solidColor", theColor);

				wireframeItem->setShader(shader);
				shaderManager->releaseShader(shader);
			}
		}
		else {
			wireframeItem = list.itemAt(index);
		}
		if (wireframeItem) {
			wireframeItem->enable(true);
		}

		// disable StandardShadedItem if CPV is shown.
		bool showCPV = isCPVShown();
		index = list.indexOf("StandardShadedItem", MGeometry::kTriangles, MGeometry::kShaded);
		if (index >= 0) {
			MRenderItem* shadedItem = list.itemAt(index);
			if (shadedItem) {
				shadedItem->enable(!showCPV);
			}
		}
		index = list.indexOf("StandardShadedItem", MGeometry::kTriangles, MGeometry::kTextured);
		if (index >= 0) {
			MRenderItem* shadedItem = list.itemAt(index);
			if (shadedItem) {
				shadedItem->enable(!showCPV);
			}
		}

		// add item for CPV.
		index = list.indexOf("geometryReplicatorCPV");
		if (index >= 0) {
			MRenderItem* cpvItem = list.itemAt(index);
			if (cpvItem) {
				cpvItem->enable(showCPV);
			}
		}
		else {
			// if no cpv item and showCPV is true, created the cpv item.
			if (showCPV) {
				MRenderItem* cpvItem = MRenderItem::Create("geometryReplicatorCPV", MRenderItem::MaterialSceneItem, MGeometry::kTriangles);
				cpvItem->setDrawMode((MGeometry::DrawMode)(MGeometry::kShaded|MGeometry::kTextured));
				list.append(cpvItem);

				MShaderInstance* shader = shaderManager->getStockShader(MShaderManager::k3dCPVSolidShader);
				if (shader) {
					cpvItem->setShader(shader);
					if (cpvItem) {
						cpvItem->enable(true);
					}
					shaderManager->releaseShader(shader);
				}
			}
		}
	}
}

bool geometryReplicatorGeometryOverride::isBaseMesh()
{
	bool res = false;
	MPlug plug(fThisNode, geometryReplicator::aBaseMesh);
	if (!plug.isNull())
	{
		if (plug.getValue(res) != MStatus::kSuccess)
		{
			res = false;
		}
	}
	return res;
}

void geometryReplicatorGeometryOverride::populateGeometry(
	const MGeometryRequirements& requirements,
	const MRenderItemList& renderItems,
	MGeometry& data)
{
	if (!fPath.isValid())
		return;

	// MGeometryExtractor::MGeometryExtractor.
	// here, fPath is the path of the linked object instead of the plugin node; it
	// is used to determine the right type of the geometry shape, e.g., polygon
	// or NURBS surface.
	// The sharing flag (true here) is just for the polygon shape.
	MStatus status;
	MPolyGeomOptions options = kPolyGeom_Normal;
	if( isBaseMesh() ) options = options|kPolyGeom_BaseMesh;
	MGeometryExtractor extractor(requirements, fPath, options, &status);
	if (MS::kFailure == status)
		return;

	// fill vertex buffer
	const MVertexBufferDescriptorList& descList = requirements.vertexRequirements();
	for (int reqNum = 0; reqNum < descList.length(); ++reqNum)
	{
		MVertexBufferDescriptor desc;
		if (!descList.getDescriptor(reqNum, desc))
		{
			continue;
		}

		switch (desc.semantic())
		{
		case MGeometry::kPosition:
		case MGeometry::kNormal:
		case MGeometry::kTexture:
		case MGeometry::kTangent:
		case MGeometry::kBitangent:
		case MGeometry::kColor:
			{
				MVertexBuffer* vertexBuffer = data.createVertexBuffer(desc);
				if (vertexBuffer) {
					// MGeometryExtractor::vertexCount and MGeometryExtractor::populateVertexBuffer.
					// since the plugin node has the same vertex data as its linked scene object,
					// call vertexCount to allocate vertex buffer of the same size, and then call
					// populateVertexBuffer to copy the data.
					unsigned int vertexCount = extractor.vertexCount();
					float* data = (float*)vertexBuffer->acquire(vertexCount, true /*writeOnly - we don't need the current buffer values*/);
					if (data) {
						status = extractor.populateVertexBuffer(data, vertexCount, desc);
						if (MS::kFailure == status)
							return;
						vertexBuffer->commit(data);
					}
				}
			}
			break;

		default:
			// do nothing for stuff we don't understand
			break;
		}
	}

	// fill index buffer
	for (int i = 0; i < renderItems.length(); ++i)
	{
		const MRenderItem* item = renderItems.itemAt(i);
		if (!item) continue;

		MIndexBuffer* indexBuffer = data.createIndexBuffer(MGeometry::kUnsignedInt32);
		if (!indexBuffer) continue;

		// MGeometryExtractor::primitiveCount and MGeometryExtractor::populateIndexBuffer.
		// since the plugin node has the same index data as its linked scene object,
		// call primitiveCount to allocate index buffer of the same size, and then call
		// populateIndexBuffer to copy the data.
		if (item->primitive() == MGeometry::kTriangles)
		{
			MIndexBufferDescriptor triangleDesc(MIndexBufferDescriptor::kTriangle, MString(), MGeometry::kTriangles, 3);
			unsigned int numTriangles = extractor.primitiveCount(triangleDesc);

			unsigned int* indices = (unsigned int*)indexBuffer->acquire(3 * numTriangles, true /*writeOnly - we don't need the current buffer values*/);
			status = extractor.populateIndexBuffer(indices, numTriangles, triangleDesc);
			if (MS::kFailure == status)
				return;
			indexBuffer->commit(indices);
		}
		else if (item->primitive() == MGeometry::kLines)
		{
			MIndexBufferDescriptor edgeDesc(MIndexBufferDescriptor::kEdgeLine, MString(), MGeometry::kLines, 2);
			unsigned int numEdges = extractor.primitiveCount(edgeDesc);

			unsigned int* indices = (unsigned int*)indexBuffer->acquire(2 * numEdges, true /*writeOnly - we don't need the current buffer values*/);
			status = extractor.populateIndexBuffer(indices, numEdges, edgeDesc);
			if (MS::kFailure == status)
				return;
			indexBuffer->commit(indices);
		}

		item->associateWithIndexBuffer(indexBuffer);
	}
}

void geometryReplicatorGeometryOverride::cleanUp()
{

}

bool geometryReplicatorGeometryOverride::hasUIDrawables() const
{
	return true;
}

void geometryReplicatorGeometryOverride::addUIDrawables(
	const MDagPath& path,
	MUIDrawManager& drawManager,
	const MFrameContext& frameContext)
{
	drawManager.beginDrawable();
	drawManager.setColor( MColor(1.0f, 0.0f, 0.0f) );
	drawManager.text( MPoint(0, 0, 0), MString("Replicate") );
	drawManager.endDrawable();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Plugin Registration
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

MStatus geometryReplicator::initialize()
{
	MStatus status;
	MFnNumericAttribute nAttr;

	aShowCPV = nAttr.create("showCPV", "sc",
		MFnNumericData::kBoolean, 0, &status);
	if (!status) {
		status.perror("create attribute showCPV");
		return status;
	}
	status = MPxNode::addAttribute(aShowCPV);
	if (!status) {
		status.perror("addAttribute showCPV");
		return status;
	}

	aBaseMesh = nAttr.create("isBaseMesh", "bm",
		MFnNumericData::kBoolean, 0, &status);
	if (!status) {
		status.perror("create attribute baseMesh");
		return status;
	}
	status = MPxNode::addAttribute(aBaseMesh);
	if (!status) {
		status.perror("addAttribute baseMesh");
		return status;
	}

	return MStatus::kSuccess;
}

MStatus initializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");

	status = plugin.registerShape(
		"geometryReplicator",
		geometryReplicator::id,
		&geometryReplicator::creator,
		&geometryReplicator::initialize,
		&geometryReplicatorShapeUI::creator,
		&geometryReplicator::drawDbClassification);
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	status = MDrawRegistry::registerGeometryOverrideCreator(
		geometryReplicator::drawDbClassification,
		geometryReplicator::drawRegistrantId,
		geometryReplicatorGeometryOverride::Creator);
	if (!status) {
		status.perror("registerGeometryOverrideCreator");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = MDrawRegistry::deregisterGeometryOverrideCreator(
		geometryReplicator::drawDbClassification,
		geometryReplicator::drawRegistrantId);
	if (!status) {
		status.perror("deregisterGeometryOverrideCreator");
		return status;
	}

	status = plugin.deregisterNode( geometryReplicator::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	return status;
}
