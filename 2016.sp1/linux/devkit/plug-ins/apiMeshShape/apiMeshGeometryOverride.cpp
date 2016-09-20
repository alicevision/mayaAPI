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
// apiMeshGeometryOverride.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "apiMeshGeometryOverride.h"
#include "apiMeshShape.h"
#include "apiMeshGeom.h"

#include <maya/MGlobal.h>
#include <maya/MUserData.h>
#include <maya/MSelectionList.h>
#include <maya/MPxComponentConverter.h>
#include <maya/MSelectionContext.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MHWGeometry.h>
#include <maya/MShaderManager.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MDrawContext.h>
#include <maya/MHWGeometryUtilities.h>
#include <maya/MObjectArray.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnDagNode.h>
#include <maya/MDrawRegistry.h>
#include <set>
#include <vector>

// Custom user data class to attach to render items
class apiMeshUserData : public MUserData
{
public:
	apiMeshUserData()
	: MUserData(true) // let Maya clean up
	, fMessage("")
	, fNumModifications(0)
	{
	}

	virtual ~apiMeshUserData()
	{
	}

	MString fMessage;
	int fNumModifications;
};

// Custom user data class to attach to render items
// to help with viewport 2.0 selection
class apiMeshHWSelectionUserData : public MUserData
{
public:
	apiMeshHWSelectionUserData()
	: MUserData(true) // let Maya clean up
	, fMeshGeom(NULL)
	{
	}

	virtual ~apiMeshHWSelectionUserData()
	{
	}

	apiMeshGeom* fMeshGeom;
};

// Pre/Post callback helper
static void callbackDataPrint(
	MHWRender::MDrawContext& context,
	const MHWRender::MRenderItemList& renderItemList)
{
	int numItems = renderItemList.length();
	const MHWRender::MRenderItem* item = NULL;
	for (int i=0; i<numItems; i++)
	{
		item = renderItemList.itemAt(i);
		if (item)
		{
			MDagPath path = item->sourceDagPath();
			printf("\tITEM: '%s' -- SOURCE: '%s'\n", item->name().asChar(), path.fullPathName().asChar());
		}
	}

	const MHWRender::MPassContext & passCtx = context.getPassContext();
	const MString & passId = passCtx.passIdentifier();
	const MStringArray & passSem = passCtx.passSemantics();
	printf("tAPI mesh drawing in pass[%s], semantic[", passId.asChar());
	for (unsigned int i=0; i<passSem.length(); i++)
		printf(" %s", passSem[i].asChar());
	printf("\n");
}
// Custom pre-draw callback
static void apiMeshPreDrawCallback(
	MHWRender::MDrawContext& context,
	const MHWRender::MRenderItemList& renderItemList,
	MHWRender::MShaderInstance *shaderInstance)
{
	printf("PRE-draw callback triggered for render item list with data:\n");
	callbackDataPrint(context, renderItemList);
	printf("\n");
}
// Custom post-draw callback
static void apiMeshPostDrawCallback(
	MHWRender::MDrawContext& context,
	const MHWRender::MRenderItemList& renderItemList,
	MHWRender::MShaderInstance *shaderInstance)
{
	printf("POST-draw callback triggered for render item list with data:\n");
	callbackDataPrint(context, renderItemList);
	printf("\n");
}

// Custom component converter to select vertices
// Attached to the dormant vertices render item (sVertexItemName)
class meshVertComponentConverter : public MHWRender::MPxComponentConverter
{
public:
	meshVertComponentConverter() : MHWRender::MPxComponentConverter() {}
	virtual ~meshVertComponentConverter() {}

	virtual void initialize(const MHWRender::MRenderItem& renderItem)
	{
		// Create the component selection object .. here we are selecting vertex component
		fComponentObject = fComponent.create( MFn::kMeshVertComponent );

		// Build a lookup table to match each primitive position in the index buffer of the render item geometry
		// to the correponding vertex component of the object
		// Use same algorithm as in updateIndexingForDormantVertices

		apiMeshHWSelectionUserData* selectionData = dynamic_cast<apiMeshHWSelectionUserData*>(renderItem.customData());
		if(selectionData)
		{
			apiMeshGeom* meshGeom = selectionData->fMeshGeom;


			// Allocate vertices lookup table
			{
				unsigned int numTriangles = 0;
				for (int i=0; i<meshGeom->faceCount; i++)
				{
					int numVerts = meshGeom->face_counts[i];
					if (numVerts > 2)
					{
						numTriangles += numVerts - 2;
					}
				}
				fVertices.resize(3*numTriangles);
			}

			// Fill vertices lookup table
			unsigned int base = 0;
			unsigned int idx = 0;
			for (int faceIdx=0; faceIdx<meshGeom->faceCount; faceIdx++)
			{
				// ignore degenerate faces
				int numVerts = meshGeom->face_counts[faceIdx];
				if (numVerts > 2)
				{
					for (int v=1; v<numVerts-1; v++)
					{
						fVertices[idx++] = meshGeom->face_connects[base];
						fVertices[idx++] = meshGeom->face_connects[base+v];
						fVertices[idx++] = meshGeom->face_connects[base+v+1];
					}
					base += numVerts;
				}
			}
		}
	}

	virtual void addIntersection(MHWRender::MIntersection& intersection)
	{
		// Convert the intersection index, which represent the primitive position in the
		// index buffer, to the correct vertex component
		const int rawIdx = intersection.index();
		int idx = 0;
		if(rawIdx >= 0 && rawIdx < (int)fVertices.size())
			idx = fVertices[rawIdx];
		fComponent.addElement(idx);
	}

	virtual MObject component()
	{
		// Return the component object that contains the ids of the selected vertices
		return fComponentObject;
	}

	virtual MSelectionMask selectionMask() const
	{
		// This converter is only valid for vertex selection or snapping
		MSelectionMask retVal;
		retVal.setMask(MSelectionMask::kSelectMeshVerts);
		retVal.addMask(MSelectionMask::kSelectPointsForGravity);
		return retVal;
	}

	static MPxComponentConverter* creator()
	{
		return new meshVertComponentConverter();
	}

private:
	MFnSingleIndexedComponent fComponent;
	MObject fComponentObject;
	std::vector<int> fVertices;
};

// Custom component converter to select edges
// Attached to the edge selection render item (sEdgeSelectionItemName)
class meshEdgeComponentConverter : public MHWRender::MPxComponentConverter
{
public:
	meshEdgeComponentConverter() : MHWRender::MPxComponentConverter() {}
	virtual ~meshEdgeComponentConverter() {}

	virtual void initialize(const MHWRender::MRenderItem& renderItem)
	{
		// Create the component selection object .. here we are selecting edge component
		fComponentObject = fComponent.create( MFn::kMeshEdgeComponent );

		// Build a lookup table to match each primitive position in the index buffer of the render item geometry
		// to the correponding edge component of the object
		// Use same algorithm as in updateIndexingForEdges

		// in updateIndexingForEdges the index buffer is allocated with "totalEdges = 2*totalVerts"
		// but since we are drawing lines, we'll get only half of the data as primitive positions :
		// indices 0 & 1 : primitive #0
		// indices 2 & 3 : primitive #1
		// ...

		apiMeshHWSelectionUserData* selectionData = dynamic_cast<apiMeshHWSelectionUserData*>(renderItem.customData());
		if(selectionData)
		{
			apiMeshGeom* meshGeom = selectionData->fMeshGeom;

			// Allocate edges lookup table
			{
				unsigned int totalVerts = 0;
				for (int i=0; i<meshGeom->faceCount; i++)
				{
					int numVerts = meshGeom->face_counts[i];
					if (numVerts > 2)
					{
						totalVerts += numVerts;
					}
				}
				fEdges.resize(totalVerts);
			}

			// Fill edges lookup table
			unsigned int idx = 0;
			int edgeId = 0;
			for (int faceIdx=0; faceIdx<meshGeom->faceCount; faceIdx++)
			{
				// ignore degenerate faces
				int numVerts = meshGeom->face_counts[faceIdx];
				if (numVerts > 2)
				{
					for (int v=0; v<numVerts; v++)
					{
						fEdges[idx++] = edgeId;
						++edgeId;
					}
				}
			}
		}
	}

	virtual void addIntersection(MHWRender::MIntersection& intersection)
	{
		// Convert the intersection index, which represent the primitive position in the
		// index buffer, to the correct edge component
		const int rawIdx = intersection.index();
		int idx = 0;
		if(rawIdx >= 0 && rawIdx < (int)fEdges.size())
			idx = fEdges[rawIdx];
		fComponent.addElement(idx);
	}

	virtual MObject component()
	{
		// Return the component object that contains the ids of the selected edges
		return fComponentObject;
	}

	virtual MSelectionMask selectionMask() const
	{
		// This converter is only valid for edge selection
		return MSelectionMask::kSelectMeshEdges;
	}

	static MPxComponentConverter* creator()
	{
		return new meshEdgeComponentConverter();
	}

private:
	MFnSingleIndexedComponent fComponent;
	MObject fComponentObject;
	std::vector<int> fEdges;
};

// Custom component converter to select faces
// Attached to the face selection render item (sFaceSelectionItemName)
class meshFaceComponentConverter : public MHWRender::MPxComponentConverter
{
public:
	meshFaceComponentConverter() : MHWRender::MPxComponentConverter() {}
	virtual ~meshFaceComponentConverter() {}

	virtual void initialize(const MHWRender::MRenderItem& renderItem)
	{
		// Create the component selection object .. here we are selecting face component
		fComponentObject = fComponent.create( MFn::kMeshPolygonComponent );

		// Build a lookup table to match each primitive position in the index buffer of the render item geometry
		// to the correponding face component of the object
		// Use same algorithm as in updateIndexingForFaces

		// in updateIndexingForFaces the index buffer is allocated with "numTriangleVertices = 3*numTriangles"
		// but since we are drawing triangles, we'll get only a third of the data as primitive positions :
		// indices 0, 1 & 2 : primitive #0
		// indices 3, 4 & 5 : primitive #1
		// ...

		apiMeshHWSelectionUserData* selectionData = dynamic_cast<apiMeshHWSelectionUserData*>(renderItem.customData());
		if(selectionData)
		{
			apiMeshGeom* meshGeom = selectionData->fMeshGeom;

			// Allocate faces lookup table
			{
				unsigned int numTriangles = 0;
				for (int i=0; i<meshGeom->faceCount; i++)
				{
					int numVerts = meshGeom->face_counts[i];
					if (numVerts > 2)
					{
						numTriangles += numVerts - 2;
					}
				}
				fFaces.resize(numTriangles);
			}

			// Fill faces lookup table
			unsigned int idx = 0;
			for (int faceIdx=0; faceIdx<meshGeom->faceCount; faceIdx++)
			{
				// ignore degenerate faces
				int numVerts = meshGeom->face_counts[faceIdx];
				if (numVerts > 2)
				{
					for (int v=1; v<numVerts-1; v++)
					{
						fFaces[idx++] = faceIdx;
					}
				}
			}
		}
	}

	virtual void addIntersection(MHWRender::MIntersection& intersection)
	{
		// Convert the intersection index, which represent the primitive position in the
		// index buffer, to the correct face component
		const int rawIdx = intersection.index();
		int idx = 0;
		if(rawIdx >= 0 && rawIdx < (int)fFaces.size())
			idx = fFaces[rawIdx];
		fComponent.addElement(idx);
	}

	virtual MObject component()
	{
		// Return the component object that contains the ids of the selected faces
		return fComponentObject;
	}

	virtual MSelectionMask selectionMask() const
	{
		// This converter is only valid for face selection
		return MSelectionMask::kSelectMeshFaces;
	}

	static MPxComponentConverter* creator()
	{
		return new meshFaceComponentConverter();
	}

private:
	MFnSingleIndexedComponent fComponent;
	MObject fComponentObject;
	std::vector<int> fFaces;
};

const MString apiMeshGeometryOverride::sWireframeItemName = "apiMeshWire";
const MString apiMeshGeometryOverride::sShadedTemplateItemName = "apiMeshShadedTemplateWire";
const MString apiMeshGeometryOverride::sSelectedWireframeItemName = "apiMeshSelectedWireFrame";
const MString apiMeshGeometryOverride::sVertexItemName = "apiMeshVertices";
const MString apiMeshGeometryOverride::sActiveVertexItemName = "apiMeshActiveVertices";
const MString apiMeshGeometryOverride::sVertexIdItemName = "apiMeshVertexIds";
const MString apiMeshGeometryOverride::sVertexPositionItemName = "apiMeshVertexPositions";
const MString apiMeshGeometryOverride::sShadedModeFaceCenterItemName = "apiMeshFaceCenterInShadedMode";
const MString apiMeshGeometryOverride::sWireframeModeFaceCenterItemName = "apiMeshFaceCenterInWireframeMode";
const MString apiMeshGeometryOverride::sShadedProxyItemName = "apiShadedProxy";
const MString apiMeshGeometryOverride::sAffectedEdgeItemName = "apiMeshAffectedEdges";
const MString apiMeshGeometryOverride::sAffectedFaceItemName = "apiMeshAffectedFaces";
const MString apiMeshGeometryOverride::sEdgeSelectionItemName = "apiMeshEdgeSelection";
const MString apiMeshGeometryOverride::sFaceSelectionItemName = "apiMeshFaceSelection";
const MString apiMeshGeometryOverride::sActiveVertexStreamName = "apiMeshSharedVertexStream";
const MString apiMeshGeometryOverride::sFaceCenterStreamName = "apiMeshFaceCenterStream";

apiMeshGeometryOverride::apiMeshGeometryOverride(const MObject& obj)
: MPxGeometryOverride(obj)
, fMesh(NULL)
, fMeshGeom(NULL)
, fColorRemapTexture(NULL)
, fLinearSampler(NULL)
{
	// get the real apiMesh object from the MObject
	MStatus status;
	MFnDependencyNode node(obj, &status);
	if (status)
	{
		fMesh = dynamic_cast<apiMesh*>(node.userNode());
	}

	fDrawSharedActiveVertices = true; // false;

	fDrawFaceCenters = true; // false;

	// Turn on to view active vertices with colours lookedup from a 1D texture.
	fDrawActiveVerticesWithRamp = false;
	if (fDrawActiveVerticesWithRamp)
	{
		fDrawFaceCenters = false; // Too cluttered showing the face centers at the same time.
	}

	// Can set the following to true, but then the logic to
	// determine what color to set is the responsibility of the plugin.
	//
	fUseCustomColors = false;

	// Can change this to choose a different shader to use when
	// no shader node is assigned to the object.
	fProxyShader = 

		// Uncommenting one of the following will result in a different line
		// shader to be used. Note that color-per-vertex (CPV) is provided in populateGeometry()
		// to handle shaders which have this geometry requirement.
		//
		// -1 // Use this to indicate to later code that we want to use a built in fragment shader

		// MHWRender::MShaderManager::k3dSolidShader // - Basic line shader
		// MHWRender::MShaderManager::k3dStippleShader // - For filled stipple faces (triangles)
		// MHWRender::MShaderManager::k3dThickLineShader // For thick solid colored lines
		// MHWRender::MShaderManager::k3dCPVThickLineShader // For thick colored lines. Black if no CPV
		// MHWRender::MShaderManager::k3dDashLineShader // - For dashed solid color lines
		// MHWRender::MShaderManager::k3dCPVDashLineShader //- For dashed coloured lines. Black if no CPV
		// MHWRender::MShaderManager::k3dThickDashLineShader // For thick dashed solid color lines. black if no cpv
		MHWRender::MShaderManager::k3dCPVThickDashLineShader //- For thick, dashed and coloured lines
	;

	// Set to true to test that overriding internal items has no effect
	// for shadows and effects overrides
	fInternalItems_NoShadowCast = false;
	fInternalItems_NoShadowReceive = false;
	fInternalItems_NoPostEffects = false;

	// Use the existing shadow casts / receives flags on the shape
	// to drive settings for applicable render items.
	fExternalItems_NoShadowCast = false;
	fExternalItems_NoShadowReceive = false;
	fExternalItemsNonTri_NoShadowCast = false;
	fExternalItemsNonTri_NoShadowReceive = false;

	// Set to true to ignore post-effects for wireframe items.
	// Shaded items will still have effects applied.
	fExternalItems_NoPostEffects = true;
	fExternalItemsNonTri_NoPostEffects = true;
}

apiMeshGeometryOverride::~apiMeshGeometryOverride()
{
	fMesh = NULL;
	fMeshGeom = NULL;

	if (fColorRemapTexture)
	{
		MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
		MHWRender::MTextureManager* textureMgr = renderer ? renderer->getTextureManager() : NULL;
		if (textureMgr) 
		{
			textureMgr->releaseTexture(fColorRemapTexture);
			fColorRemapTexture = NULL;
		}
	}
	if (fLinearSampler)
	{
		MHWRender::MStateManager::releaseSamplerState(fLinearSampler);
		fLinearSampler = NULL;
	}
}

MHWRender::DrawAPI apiMeshGeometryOverride::supportedDrawAPIs() const
{
	// this plugin supports both GL and DX
	return (MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile);
}

void apiMeshGeometryOverride::updateDG()
{
	// Pull the actual outMesh from the shape, as well
	// as any active components
	fActiveVertices.clear();
	fActiveVerticesSet.clear();
	fActiveEdgesSet.clear();
	fActiveFacesSet.clear();

	if (fMesh)
	{
		fMeshGeom = fMesh->meshGeom();

		if (fMesh->hasActiveComponents())
		{
			MObjectArray activeComponents = fMesh->activeComponents();
			if (activeComponents.length())
			{
				MFnSingleIndexedComponent fnComponent( activeComponents[0] );
				if (fnComponent.elementCount())
				{
					MIntArray activeIds;
					fnComponent.getElements( activeIds );

					if(fnComponent.componentType() == MFn::kMeshVertComponent)
					{
						fActiveVertices = activeIds;

						for (unsigned int i=0; i<activeIds.length(); ++i)
							fActiveVerticesSet.insert( activeIds[i] );
					}
					else if(fnComponent.componentType() == MFn::kMeshEdgeComponent)
					{
						for (unsigned int i=0; i<activeIds.length(); ++i)
							fActiveEdgesSet.insert( activeIds[i] );
					}
					else if(fnComponent.componentType() == MFn::kMeshPolygonComponent)
					{
						for (unsigned int i=0; i<activeIds.length(); ++i)
							fActiveFacesSet.insert( activeIds[i] );
					}
				}
			}
		}
	}
}

/*
	Some example code to print out shader parameters
*/
void apiMeshGeometryOverride::printShader(MHWRender::MShaderInstance* shader)
{
	if (!shader)
		return;

	MStringArray params;
	shader->parameterList(params);
	unsigned int numParams = params.length();
	printf("DEBUGGING SHADER, BEGIN PARAM LIST OF LENGTH %d\n", numParams);
	for (unsigned int i=0; i<numParams; i++)
	{
		printf("ParamName='%s', ParamType=", params[i].asChar());
		switch (shader->parameterType(params[i]))
		{
		case MHWRender::MShaderInstance::kInvalid:
			printf("'Invalid', ");
			break;
		case MHWRender::MShaderInstance::kBoolean:
			printf("'Boolean', ");
			break;
		case MHWRender::MShaderInstance::kInteger:
			printf("'Integer', ");
			break;
		case MHWRender::MShaderInstance::kFloat:
			printf("'Float', ");
			break;
		case MHWRender::MShaderInstance::kFloat2:
			printf("'Float2', ");
			break;
		case MHWRender::MShaderInstance::kFloat3:
			printf("'Float3', ");
			break;
		case MHWRender::MShaderInstance::kFloat4:
			printf("'Float4', ");
			break;
		case MHWRender::MShaderInstance::kFloat4x4Row:
			printf("'Float4x4Row', ");
			break;
		case MHWRender::MShaderInstance::kFloat4x4Col:
			printf("'Float4x4Col', ");
			break;
		case MHWRender::MShaderInstance::kTexture1:
			printf("'1D Texture', ");
			break;
		case MHWRender::MShaderInstance::kTexture2:
			printf("'2D Texture', ");
			break;
		case MHWRender::MShaderInstance::kTexture3:
			printf("'3D Texture', ");
			break;
		case MHWRender::MShaderInstance::kTextureCube:
			printf("'Cube Texture', ");
			break;
		case MHWRender::MShaderInstance::kSampler:
			printf("'Sampler', ");
			break;
		default:
			printf("'Unknown', ");
			break;
		}
		printf("IsArrayParameter='%s'\n", shader->isArrayParameter(params[i]) ? "YES" : "NO");
	}
	printf("END PARAM LIST\n");
}

/*
	Set the solid color for solid color shaders
*/
void apiMeshGeometryOverride::setSolidColor(MHWRender::MShaderInstance* shaderInstance, const float *value)
{
	if (!shaderInstance)
		return;

	const MString colorParameterName = "solidColor";
    shaderInstance->setParameter(colorParameterName, value);
}

/*
	Set the point size for solid color shaders
*/
void apiMeshGeometryOverride::setSolidPointSize(MHWRender::MShaderInstance* shaderInstance, float pointSize)
{
	if (!shaderInstance)
		return;

	const float pointSizeArray[2] = {pointSize, pointSize};
	const MString pointSizeParameterName = "pointSize";
	shaderInstance->setParameter( pointSizeParameterName, pointSizeArray );
}

/*
	Set the line width for solid color shaders
*/
void apiMeshGeometryOverride::setLineWidth(MHWRender::MShaderInstance* shaderInstance, float lineWidth)
{
	if (!shaderInstance)
		return;

	const float lineWidthArray[2] = {lineWidth, lineWidth};
	const MString lineWidthParameterName = "lineWidth";
	shaderInstance->setParameter( lineWidthParameterName, lineWidthArray );
}

/*
	Update render items for dormant and template wireframe drawing.

	1) If the object is dormant and not templated then we require
	a render item to display when wireframe drawing is required (display modes
	is wire or wire-on-shaded)

	2a) If the object is templated then we use the same render item as in 1)
	when in wireframe drawing is required.
	2b) However we also require a render item to display when in shaded mode.
*/
void apiMeshGeometryOverride::updateDormantAndTemplateWireframeItems(
					const MDagPath& path,
					MHWRender::MRenderItemList& list,
					const MHWRender::MShaderManager* shaderMgr)
{
	// Stock colors
	static const float dormantColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	static const float templateColor[] = { 0.45f, 0.45f, 0.45f, 1.0f };
	static const float activeTemplateColor[] = { 1.0f, 0.5f, 0.5f, 1.0f };

	// Some local options to show debug interface
	//
	static const bool debugShader = false;

	static const unsigned int shadedDrawMode = MHWRender::MGeometry::kAll;

	MHWRender::MGeometry::Primitive primitive = MHWRender::MGeometry::kLines;

	// Get render item used for draw in wireframe mode
	// (Mode to draw in is MHWRender::MGeometry::kWireframe)
	//
	MHWRender::MRenderItem* wireframeItem = NULL;
	int index = list.indexOf(sWireframeItemName);
	if (index < 0)
	{
		wireframeItem = MHWRender::MRenderItem::Create(
			sWireframeItemName,
			MHWRender::MRenderItem::DecorationItem,
			primitive);
		wireframeItem->setDrawMode(MHWRender::MGeometry::kWireframe);

		// Set dormant wireframe with appropriate priority to not clash with
		// any active wireframe which may overlap in depth.
		wireframeItem->depthPriority( MHWRender::MRenderItem::sDormantWireDepthPriority );
		list.append(wireframeItem);

		MHWRender::MShaderInstance* shader = shaderMgr->getStockShader(
			MHWRender::MShaderManager::k3dSolidShader,
			debugShader ? apiMeshPreDrawCallback : NULL,
			debugShader ? apiMeshPostDrawCallback : NULL);
		if (shader)
		{
			// assign shader
			wireframeItem->setShader(shader);

			// sample debug code
			if (debugShader)
			{
				printShader( shader );
			}

			// once assigned, no need to hold on to shader instance
			shaderMgr->releaseShader(shader);
		}
	}
	else
	{
		wireframeItem = list.itemAt(index);
	}

	// Get render item for handling mode shaded template drawing
	//
	MHWRender::MRenderItem* shadedTemplateItem = NULL;
	int index2 = list.indexOf(sShadedTemplateItemName);
	if (index2 < 0)
	{
		shadedTemplateItem = MHWRender::MRenderItem::Create(
			sShadedTemplateItemName,
			MHWRender::MRenderItem::DecorationItem,
			primitive);
		shadedTemplateItem->setDrawMode((MHWRender::MGeometry::DrawMode) shadedDrawMode);

		// Set shaded item as being dormant wire since it should still be raised
		// above any shaded items, but not as high as active items.
		shadedTemplateItem->depthPriority( MHWRender::MRenderItem::sDormantWireDepthPriority );

		list.append(shadedTemplateItem);

		MHWRender::MShaderInstance* shader = shaderMgr->getStockShader(
			MHWRender::MShaderManager::k3dSolidShader,
			NULL, NULL);
		if (shader)
		{
			// assign shader
			shadedTemplateItem->setShader(shader);

			// sample debug code
			if (debugShader)
			{
				printShader( shader );
			}

			// once assigned, no need to hold on to shader instance
			shaderMgr->releaseShader(shader);
		}
	}
	else
	{
		shadedTemplateItem = list.itemAt(index2);
	}

	// Sample code to disable cast, receives shadows, and post effects.
	if (fExternalItemsNonTri_NoShadowCast)
		shadedTemplateItem->castsShadows( false );
	if (fExternalItemsNonTri_NoShadowReceive)
		shadedTemplateItem->receivesShadows( false );
	if (fExternalItemsNonTri_NoPostEffects)
		shadedTemplateItem->setExcludedFromPostEffects( true );

	MHWRender::DisplayStatus displayStatus =
		MHWRender::MGeometryUtilities::displayStatus(path);

	MColor wireColor = MHWRender::MGeometryUtilities::wireframeColor(path);

	// Enable / disable wireframe item and update the shader parameters
	//
	if (wireframeItem)
	{
		MHWRender::MShaderInstance* shader = wireframeItem->getShader();

		switch (displayStatus) {
		case MHWRender::kTemplate:
			if (shader)
			{
				setSolidColor( shader, !fUseCustomColors ? &(wireColor.r) : templateColor);
			}
			wireframeItem->enable(true);
			break;
		case MHWRender::kActiveTemplate:
			if (shader)
			{
				setSolidColor( shader, !fUseCustomColors ? &(wireColor.r) : activeTemplateColor );
			}
			wireframeItem->enable(true);
			break;
		case MHWRender::kDormant:
			if (shader)
			{
				setSolidColor( shader, !fUseCustomColors ? &(wireColor.r) : dormantColor);
			}
			wireframeItem->enable(true);
			break;
		case MHWRender::kActiveAffected:
			if (shader)
			{
				static const float theColor[] = { 0.5f, 0.0f, 1.0f, 1.0f };
				setSolidColor( shader, !fUseCustomColors ? &(wireColor.r) : theColor );
			}
			wireframeItem->enable(true);
			break;
		default:
			wireframeItem->enable(false);
			break;
		}
	}

	// Enable / disable shaded/template item and update the shader parameters
	//
	bool isTemplate = path.isTemplated();
	if (shadedTemplateItem)
	{
		MHWRender::MShaderInstance* shader = shadedTemplateItem->getShader();

		switch (displayStatus) {
		case MHWRender::kTemplate:
			if (shader)
			{
				setSolidColor( shader, !fUseCustomColors ? &(wireColor.r) : templateColor );
			}
			shadedTemplateItem->enable(isTemplate);
			break;
		case MHWRender::kActiveTemplate:
			if (shader)
			{
				setSolidColor( shader, !fUseCustomColors ? &(wireColor.r) : activeTemplateColor );
			}
			shadedTemplateItem->enable(isTemplate);
			break;
		case MHWRender::kDormant:
			if (shader)
			{
				setSolidColor( shader, !fUseCustomColors ? &(wireColor.r) : dormantColor);
			}
			shadedTemplateItem->enable(isTemplate);
			break;
		default:
			shadedTemplateItem->enable(false);
			break;
		}
	}
}

/*
	Create a render item for active wireframe if it does not exist. Updating
	shading parameters as necessary.
*/
void apiMeshGeometryOverride::updateActiveWireframeItem(const MDagPath& path,
														MHWRender::MRenderItemList& list,
														const MHWRender::MShaderManager* shaderMgr)
{
	MHWRender::MRenderItem* selectItem = NULL;
	int index = list.indexOf(sSelectedWireframeItemName);
	if (index < 0)
	{
		selectItem = MHWRender::MRenderItem::Create(
			sSelectedWireframeItemName,
			MHWRender::MRenderItem::DecorationItem,
			MHWRender::MGeometry::kLines);
		selectItem->setDrawMode(MHWRender::MGeometry::kAll);
		// This is the same as setting the argument raiseAboveShaded = true,
		// since it sets the priority value to be the same. This line is just
		// an example of another way to do the same thing after creation of
		// the render item.
		selectItem->depthPriority( MHWRender::MRenderItem::sActiveWireDepthPriority);
		list.append(selectItem);

		// For active wireframe we will use a shader which allows us to draw thick lines
		//
		static const bool drawThick = false;
		MHWRender::MShaderInstance* shader =
			shaderMgr->getStockShader( drawThick ? MHWRender::MShaderManager::k3dThickLineShader : MHWRender::MShaderManager::k3dSolidShader );
		if (shader)
		{

			// assign shader
			selectItem->setShader(shader);
			// once assigned, no need to hold on to shader instance
			shaderMgr->releaseShader(shader);
		}
	}
	else
	{
		selectItem = list.itemAt(index);
	}

	MHWRender::MShaderInstance* shader = NULL;
	if (selectItem)
	{
		shader = selectItem->getShader();
	}

	MHWRender::DisplayStatus displayStatus =
		MHWRender::MGeometryUtilities::displayStatus(path);
	MColor wireColor = MHWRender::MGeometryUtilities::wireframeColor(path);

	switch (displayStatus) {
	case MHWRender::kLead:
		selectItem->enable(true);
		if (shader)
		{
			static const float theColor[] = { 0.0f, 0.8f, 0.0f, 1.0f };
			setSolidColor( shader, !fUseCustomColors ? &(wireColor.r) : theColor );
		}
		break;
	case MHWRender::kActive:
		if (shader)
		{
			static const float theColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			setSolidColor( shader, !fUseCustomColors ? &(wireColor.r) : theColor );
		}
		selectItem->enable(true);
		break;
	case MHWRender::kHilite:
	case MHWRender::kActiveComponent:
		if (shader)
		{
			static const float theColor[] = { 0.0f, 0.5f, 0.7f, 1.0f };
			setSolidColor( shader, !fUseCustomColors ? &(wireColor.r) : theColor );
		}
		selectItem->enable(true);
		break;
	default:
		selectItem->enable(false);
		break;
	};

	// Add custom user data to selection item
	apiMeshUserData* myCustomData = dynamic_cast<apiMeshUserData*>(selectItem->customData());
	if (!myCustomData)
	{
		// create the custom data
		myCustomData = new apiMeshUserData();
		myCustomData->fMessage = "I'm custom data!";
		selectItem->setCustomData(myCustomData);
	}
	else
	{
		// modify the custom data
		myCustomData->fNumModifications++;
	}
}

/*
	Create render items for numeric display, and update shaders as necessary
*/
void apiMeshGeometryOverride::updateVertexNumericItems(const MDagPath& path, MHWRender::MRenderItemList& list, const MHWRender::MShaderManager* shaderMgr)
{
	// Enable to show numeric render items
	bool enableNumiercDisplay = false;

	// Vertex id item
	//
	MHWRender::MRenderItem* vertexItem = NULL;
	int index = list.indexOf(sVertexIdItemName);
	if (index < 0)
	{
		vertexItem = MHWRender::MRenderItem::Create(
			sVertexIdItemName,
			MHWRender::MRenderItem::DecorationItem,
			MHWRender::MGeometry::kPoints);
		vertexItem->setDrawMode(MHWRender::MGeometry::kAll);
		vertexItem->depthPriority( MHWRender::MRenderItem::sDormantPointDepthPriority );
		list.append(vertexItem);

		// Use single integer numeric shader
		MHWRender::MShaderInstance* shader = shaderMgr->getStockShader(
			MHWRender::MShaderManager::k3dIntegerNumericShader ); // k3dFloatNumericShader );
		if (shader)
		{
			// Label the fields so that they can be found later on.
			vertexItem->setShader(shader, &sVertexIdItemName);
			shaderMgr->releaseShader(shader);
		}
	}
	else
	{
		vertexItem = list.itemAt(index);
	}
	if (vertexItem)
	{
		MHWRender::MShaderInstance* shader = vertexItem->getShader();
		if (shader)
		{
			// set color
			static const float theColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
			setSolidColor( shader, theColor);
		}
		vertexItem->enable(enableNumiercDisplay);
	}

	// Vertex position numeric render item
	//
	MHWRender::MRenderItem* vertexItem2 = NULL;
	index = list.indexOf(sVertexPositionItemName);
	if (index < 0)
	{
		vertexItem2 = MHWRender::MRenderItem::Create(
			sVertexPositionItemName,
			MHWRender::MRenderItem::DecorationItem,
			MHWRender::MGeometry::kPoints);
		vertexItem2->setDrawMode(MHWRender::MGeometry::kAll);
		vertexItem2->depthPriority( MHWRender::MRenderItem::sDormantPointDepthPriority );
		list.append(vertexItem2);

		// Use triple float numeric shader
		MHWRender::MShaderInstance* shader = shaderMgr->getStockShader(
			MHWRender::MShaderManager::k3dFloat3NumericShader );
		if (shader)
		{
			//vertexItem2->setShader(shader);
			vertexItem2->setShader(shader, &sVertexPositionItemName);
			shaderMgr->releaseShader(shader);
		}
	}
	else
	{
		vertexItem2 = list.itemAt(index);
	}
	if (vertexItem2)
	{
		MHWRender::MShaderInstance* shader = vertexItem2->getShader();
		if (shader)
		{
			// set color
			static const float theColor[] = { 0.0f, 1.0f, 1.0f, 1.0f };
			setSolidColor( shader, theColor);
		}
		vertexItem2->enable(enableNumiercDisplay);
	}
}

/*
	Create a render item for dormant vertices if it does not exist. Updating
	shading parameters as necessary.
*/
void apiMeshGeometryOverride::updateDormantVerticesItem(const MDagPath& path, MHWRender::MRenderItemList& list, const MHWRender::MShaderManager* shaderMgr)
{
	MHWRender::MRenderItem* vertexItem = NULL;
	int index = list.indexOf(sVertexItemName);
	if (index < 0)
	{
		vertexItem = MHWRender::MRenderItem::Create(
			sVertexItemName,
			MHWRender::MRenderItem::DecorationItem,
			MHWRender::MGeometry::kPoints);

		// Set draw mode to kAll : the item will be visible in the viewport and also during viewport 2.0 selection
		vertexItem->setDrawMode(MHWRender::MGeometry::kAll);

		// Set the selection mask to kSelectMeshVerts : we want the render item to be used for Vertex Components selection
		MSelectionMask vertexAndGravity(MSelectionMask::kSelectMeshVerts);
		vertexAndGravity.addMask(MSelectionMask::kSelectPointsForGravity);
		vertexItem->setSelectionMask( vertexAndGravity );

		// Set depth priority higher than wireframe and shaded render items,
		// but lower than active points.
		// Raising higher than wireframe will make them not seem embedded into the surface
		vertexItem->depthPriority( MHWRender::MRenderItem::sDormantPointDepthPriority );
		list.append(vertexItem);

		MHWRender::MShaderInstance* shader = shaderMgr->getStockShader(
			MHWRender::MShaderManager::k3dFatPointShader );
		if (shader)
		{
			// Set the point size parameter
			static const float pointSize = 3.0f;
			setSolidPointSize( shader, pointSize );

			// assign shader
			vertexItem->setShader(shader);

			// once assigned, no need to hold on to shader instance
			shaderMgr->releaseShader(shader);
		}
	}
	else
	{
		vertexItem = list.itemAt(index);
	}

	if (vertexItem)
	{
		MHWRender::MShaderInstance* shader = vertexItem->getShader();
		if (shader)
		{
			// set color
			static const float theColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
			setSolidColor( shader, theColor);
		}

		MHWRender::DisplayStatus displayStatus =
			MHWRender::MGeometryUtilities::displayStatus(path);

		// Generally if the display status is hilite then we
		// draw components.
		if (displayStatus == MHWRender::kHilite || pointSnappingActive())
		{
			// In case the object is templated
			// we will hide the components to be consistent
			// with how internal objects behave.
			if (path.isTemplated())
				vertexItem->enable(false);
			else
				vertexItem->enable(true);
		}
		else
		{
			vertexItem->enable(false);
		}

		apiMeshHWSelectionUserData* mySelectionData = dynamic_cast<apiMeshHWSelectionUserData*>(vertexItem->customData());
		if (!mySelectionData)
		{
			// create the custom data
			mySelectionData = new apiMeshHWSelectionUserData();
			vertexItem->setCustomData(mySelectionData);
		}
		// update the custom data
		mySelectionData->fMeshGeom = fMeshGeom;
	}
}

/*
	Create a render item for active vertices if it does not exist. Updating
	shading parameters as necessary.
*/
void apiMeshGeometryOverride::updateActiveVerticesItem(const MDagPath& path, MHWRender::MRenderItemList& list, const MHWRender::MShaderManager* shaderMgr)
{
	MHWRender::MRenderItem* activeItem = NULL;
	int index = list.indexOf(sActiveVertexItemName);
	if (index < 0)
	{
		activeItem = MHWRender::MRenderItem::Create(
			sActiveVertexItemName,
			MHWRender::MRenderItem::DecorationItem,
			MHWRender::MGeometry::kPoints);
		activeItem->setDrawMode(MHWRender::MGeometry::kAll);
		// Set depth priority to be active point. This should offset it
		// to be visible above items with "dormant point" priority.
		activeItem->depthPriority( MHWRender::MRenderItem::sActivePointDepthPriority );
		list.append(activeItem);

		MHWRender::MShaderInstance* shader = shaderMgr->getStockShader(
			fDrawActiveVerticesWithRamp ? MHWRender::MShaderManager::k3dColorLookupFatPointShader 
			: MHWRender::MShaderManager::k3dFatPointShader );
		if (shader)
		{
			// Set the point size parameter. Make it slightly larger for active vertices
			static const float pointSize = 5.0f;
			setSolidPointSize( shader, pointSize );

			// 1D Ramp color lookup option
			//
			if (fDrawActiveVerticesWithRamp )
			{
				MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
				MHWRender::MTextureManager* textureMgr = renderer->getTextureManager();

				// Assign dummy ramp lookup
				if (!fColorRemapTexture)
				{
					// Sample 3 colour ramp
					float colorArray[12];
					colorArray[0] = 1.0f; colorArray[1] = 0.0f; colorArray[2] = 0.0f; colorArray[3] = 1.0f;
					colorArray[4] = 0.0f; colorArray[5] = 1.0f; colorArray[6] = 0.0f; colorArray[7] = 1.0f;
					colorArray[8] = 0.0f; colorArray[9] = 0.0f; colorArray[10] = 1.0f; colorArray[11] = 1.0f;

					unsigned int arrayLen = 3;
					MHWRender::MTextureDescription textureDesc;
					textureDesc.setToDefault2DTexture();
					textureDesc.fWidth = arrayLen;
					textureDesc.fHeight = 1;
					textureDesc.fDepth = 1;
					textureDesc.fBytesPerSlice = textureDesc.fBytesPerRow = 24*arrayLen;
					textureDesc.fMipmaps = 1;
					textureDesc.fArraySlices = 1;
					textureDesc.fTextureType = MHWRender::kImage1D;
					textureDesc.fFormat = MHWRender::kR32G32B32A32_FLOAT;;
					fColorRemapTexture =
						textureMgr->acquireTexture("", textureDesc, &( colorArray[0] ), false);
				}

				if (!fLinearSampler)
				{
					MHWRender::MSamplerStateDesc samplerDesc;
					samplerDesc.addressU = MHWRender::MSamplerState::kTexClamp;
					samplerDesc.addressV = MHWRender::MSamplerState::kTexClamp;
					samplerDesc.addressW = MHWRender::MSamplerState::kTexClamp;
					samplerDesc.filter = MHWRender::MSamplerState::kMinMagMipLinear;
					fLinearSampler = MHWRender::MStateManager::acquireSamplerState(samplerDesc);
				}

				if (fColorRemapTexture && fLinearSampler)
				{
					// Set up the ramp lookup
					MHWRender::MTextureAssignment texAssignment;
 					texAssignment.texture = fColorRemapTexture;
					shader->setParameter("map", texAssignment);
					shader->setParameter("samp", *fLinearSampler);

					// No remapping. The initial data created in the range 0...1
					//
					MFloatVector rampValueRange(0.0f, 1.0f);
					shader->setParameter("UVRange", (float*)&rampValueRange);
				}
			}

			// Assign shader. Use a named stream if we want to supply a different
			// set of "shared" vertices for drawing active vertices
			if (fDrawSharedActiveVertices)
			{
				activeItem->setShader(shader, &sActiveVertexStreamName );
			}
			else
			{
				activeItem->setShader(shader, NULL);
			}

			// once assigned, no need to hold on to shader instance
			shaderMgr->releaseShader(shader);
		}
	}
	else
	{
		activeItem = list.itemAt(index);
	}

	if (activeItem)
	{
		MHWRender::MShaderInstance* shader = activeItem->getShader();
		if (shader)
		{
			// Set active color
			static const float theColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
			setSolidColor( shader, theColor);
		}

		const bool enable = (fActiveVerticesSet.size() > 0 && enableActiveComponentDisplay(path));
		activeItem->enable( enable );
	}
}

//Add render item for face centers in wireframe mode, always show face centers in wireframe mode except it is drawn as template.
void apiMeshGeometryOverride::updateWireframeModeFaceCenterItem(const MDagPath& path, MHWRender::MRenderItemList& list, const MHWRender::MShaderManager* shaderMgr)
{
	MHWRender::MRenderItem* wireframeModeFaceCenterItem = NULL;
	int index = list.indexOf(sWireframeModeFaceCenterItemName);
	if (index < 0)
	{
		wireframeModeFaceCenterItem = MHWRender::MRenderItem::Create(
			sWireframeModeFaceCenterItemName,
			MHWRender::MRenderItem::DecorationItem,
			MHWRender::MGeometry::kPoints);
		wireframeModeFaceCenterItem->setDrawMode(MHWRender::MGeometry::kWireframe);
		wireframeModeFaceCenterItem->depthPriority( MHWRender::MRenderItem::sActiveWireDepthPriority );

		list.append(wireframeModeFaceCenterItem);

		MHWRender::MShaderInstance* shader = shaderMgr->getStockShader(
			MHWRender::MShaderManager::k3dFatPointShader );
		if (shader)
		{
			// Set the point size parameter. Make it slightly larger for face centers
			static const float pointSize = 5.0f;
			setSolidPointSize( shader, pointSize );


			wireframeModeFaceCenterItem->setShader(shader, &sFaceCenterStreamName );

			// once assigned, no need to hold on to shader instance
			shaderMgr->releaseShader(shader);
		}
	}
	else
	{
		wireframeModeFaceCenterItem = list.itemAt(index);
	}

	if (wireframeModeFaceCenterItem)
	{
		MHWRender::MShaderInstance* shader = wireframeModeFaceCenterItem->getShader();
		if (shader)
		{
			// Set face center color in wireframe mode
			static const float theColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
			setSolidColor( shader, theColor);
		}

		//disable the face center item when template
		bool isTemplate = path.isTemplated();
		if(isTemplate)
			wireframeModeFaceCenterItem->enable( false );
	}
}

//Add render item for face centers in shaded mode. If the geometry is not selected, face centers are not drawn.
void apiMeshGeometryOverride::updateShadedModeFaceCenterItem(const MDagPath& path, MHWRender::MRenderItemList& list, const MHWRender::MShaderManager* shaderMgr)
{

	static const unsigned int shadedDrawMode =
		MHWRender::MGeometry::kShaded | MHWRender::MGeometry::kTextured;

	MHWRender::MRenderItem* shadedModeFaceCenterItem = NULL;
	int index = list.indexOf(sShadedModeFaceCenterItemName);
	if (index < 0)
	{
		shadedModeFaceCenterItem = MHWRender::MRenderItem::Create(
			sShadedModeFaceCenterItemName,
			MHWRender::MRenderItem::DecorationItem,
			MHWRender::MGeometry::kPoints);
		shadedModeFaceCenterItem->setDrawMode((MHWRender::MGeometry::DrawMode)shadedDrawMode);

		shadedModeFaceCenterItem->depthPriority(MHWRender::MRenderItem::sActivePointDepthPriority);
		list.append(shadedModeFaceCenterItem);

		MHWRender::MShaderInstance* shader = shaderMgr->getStockShader(
			MHWRender::MShaderManager::k3dFatPointShader );
		if (shader)
		{
			// Set the point size parameter. Make it slightly larger for face centers
			static const float pointSize = 5.0f;
			setSolidPointSize( shader, pointSize );

			shadedModeFaceCenterItem->setShader(shader, &sFaceCenterStreamName );

			// once assigned, no need to hold on to shader instance
			shaderMgr->releaseShader(shader);
		}
	}
	else
	{
		shadedModeFaceCenterItem = list.itemAt(index);
	}

	if (shadedModeFaceCenterItem)
	{
		shadedModeFaceCenterItem->setExcludedFromPostEffects(true);

		MHWRender::MShaderInstance* shader = shadedModeFaceCenterItem->getShader();
		MHWRender::DisplayStatus displayStatus = MHWRender::MGeometryUtilities::displayStatus(path);
		MColor wireColor = MHWRender::MGeometryUtilities::wireframeColor(path);
		if (shader)
		{
			// Set face center color in shaded mode
			setSolidColor( shader, &(wireColor.r));
		}

		switch(displayStatus){
		case MHWRender::kActive:
		case MHWRender::kLead:
		case MHWRender::kActiveComponent:
		case MHWRender::kLive:
		case MHWRender::kHilite:
			shadedModeFaceCenterItem->enable(true);
			break;
		default:
			shadedModeFaceCenterItem->enable(false);
			break;

		}
	}
}

/*
	Test to see if active components should be enabled.
	Based on active vertices + non-template state
*/
bool apiMeshGeometryOverride::enableActiveComponentDisplay(const MDagPath &path) const
{
	bool enable = true;

	// If there are components then we need to check
	// either the display status of the object, or
	// in the case of a templated object make sure
	// to hide components to be consistent with how
	// internal objects behave
	//
	MHWRender::DisplayStatus displayStatus =
		MHWRender::MGeometryUtilities::displayStatus(path);
	if ((displayStatus & (MHWRender::kHilite | MHWRender::kActiveComponent)) == 0)
	{
		enable = false;
	}
	else
	{
		// Do an explicit path test for templated
		// since display status does not indicate this.
		if (path.isTemplated())
			enable = false;
	}

	return enable;
}

/*
	Example of adding in items to hilite edges and faces. In this
	case these are edges and faces which are connected to vertices
	and we thus call them "affected" components.
*/
void apiMeshGeometryOverride::updateAffectedComponentItems(
					const MDagPath& path,
					MHWRender::MRenderItemList& list,
					const MHWRender::MShaderManager* shaderMgr)
{
	// Create / update "affected/active" edges component render item.
	//
	MHWRender::MRenderItem* componentItem = NULL;
	int index = list.indexOf(sAffectedEdgeItemName);
	if (index < 0)
	{
		componentItem = MHWRender::MRenderItem::Create(
			sAffectedEdgeItemName,
			MHWRender::MRenderItem::DecorationItem,
			MHWRender::MGeometry::kLines);
		componentItem->setDrawMode(MHWRender::MGeometry::kAll);

		// Set depth priority to be active line so that it is above wireframe
		// but below dormant and active points.
		componentItem->depthPriority( MHWRender::MRenderItem::sActiveLineDepthPriority );
		list.append(componentItem);

		MHWRender::MShaderInstance* shader = shaderMgr->getStockShader(
			MHWRender::MShaderManager::k3dThickLineShader );
		if (shader)
		{
			// Assign shader.
			componentItem->setShader(shader, NULL);

			// once assigned, no need to hold on to shader instance
			shaderMgr->releaseShader(shader);
		}
	}
	else
	{
		componentItem = list.itemAt(index);
	}

	if (componentItem)
	{
		MHWRender::MShaderInstance* shader = componentItem->getShader();
		if (shader)
		{
			// Set lines a bit thicker to stand out
			static const float lineSize = 1.0f;
			setLineWidth( shader, lineSize );

			// Set affected color
			static const float theColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			setSolidColor( shader, theColor);
		}

		const bool enable = ((fActiveVerticesSet.size() > 0 || fActiveEdgesSet.size() > 0) && enableActiveComponentDisplay(path));
		componentItem->enable( enable );
	}

	////////////////////////////////////////////////////////////////////////////////

	// Create / update "affected/active" faces component render item
	//
	componentItem = NULL;
	index = list.indexOf(sAffectedFaceItemName);
	if (index < 0)
	{
		componentItem = MHWRender::MRenderItem::Create(
			sAffectedFaceItemName,
			MHWRender::MRenderItem::DecorationItem,
			MHWRender::MGeometry::kTriangles);
		componentItem->setDrawMode(MHWRender::MGeometry::kAll);
		// Set depth priority to be dormant wire so that edge and vertices
		// show on top.
		componentItem->depthPriority( MHWRender::MRenderItem::sDormantWireDepthPriority );
		list.append(componentItem);

		MHWRender::MShaderInstance* shader = shaderMgr->getStockShader(
			MHWRender::MShaderManager::k3dStippleShader );
		if (shader)
		{
			// Assign shader.
			componentItem->setShader(shader, NULL);

			// once assigned, no need to hold on to shader instance
			shaderMgr->releaseShader(shader);
		}
	}
	else
	{
		componentItem = list.itemAt(index);
	}

	if (componentItem)
	{
		MHWRender::MShaderInstance* shader = componentItem->getShader();
		if (shader)
		{
			// Set affected color
			static const float theColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			setSolidColor( shader, theColor);
		}

		const bool enable = ((fActiveVerticesSet.size() > 0 || fActiveFacesSet.size() > 0) && enableActiveComponentDisplay(path));
		componentItem->enable( enable );
	}
}

/*
	Example of adding in items for edges and faces selection.

	For the vertex selection, we already have a render item that display all the vertices (sVertexItemName)
	we could use it for the selection as well.
	
	But we have none that display the complete edges or faces,
	sAffectedEdgeItemName and sAffectedFaceItemName only display a subset of the edges and faces
	that are active or affected (one of their vertices is selected).

	In order to be able to perform the selection of this components we'll create new render items
	that will only be used for the selection (they will not be visible in the viewport)
*/
void apiMeshGeometryOverride::updateSelectionComponentItems(
					const MDagPath& path,
					MHWRender::MRenderItemList& list,
					const MHWRender::MShaderManager* shaderMgr)
{
	// Create / update selection edges component render item.
	//
	MHWRender::MRenderItem* selectionItem = NULL;
	int index = list.indexOf(sEdgeSelectionItemName);
	if (index < 0)
	{
		selectionItem = MHWRender::MRenderItem::Create(
			sEdgeSelectionItemName,
			MHWRender::MRenderItem::DecorationItem,
			MHWRender::MGeometry::kLines);

		// Set draw mode to kSelectionOnly: 
		//    - the item will only be visible in viewport 2.0 selection
		selectionItem->setDrawMode(MHWRender::MGeometry::kSelectionOnly);

		// Set the selection mask to kSelectMeshEdges : we want the render item to be used for Edge Components selection
		selectionItem->setSelectionMask( MSelectionMask::kSelectMeshEdges );

		// Set depth priority to be selection so that it is above everything
		selectionItem->depthPriority( MHWRender::MRenderItem::sSelectionDepthPriority );
		list.append(selectionItem);

		MHWRender::MShaderInstance* shader = shaderMgr->getStockShader(
			MHWRender::MShaderManager::k3dThickLineShader );
		if (shader)
		{
			// Assign shader.
			selectionItem->setShader(shader, NULL);

			// once assigned, no need to hold on to shader instance
			shaderMgr->releaseShader(shader);
		}
	}
	else
	{
		selectionItem = list.itemAt(index);
	}

	if (selectionItem)
	{
		selectionItem->enable(true);

		apiMeshHWSelectionUserData* mySelectionData = dynamic_cast<apiMeshHWSelectionUserData*>(selectionItem->customData());
		if (!mySelectionData)
		{
			// create the custom data
			mySelectionData = new apiMeshHWSelectionUserData();
			selectionItem->setCustomData(mySelectionData);
		}
		// update the custom data
		mySelectionData->fMeshGeom = fMeshGeom;
	}

	// Create / update selection faces component render item.
	//
	index = list.indexOf(sFaceSelectionItemName);
	if (index < 0)
	{
		selectionItem = MHWRender::MRenderItem::Create(
			sFaceSelectionItemName,
			MHWRender::MRenderItem::DecorationItem,
			MHWRender::MGeometry::kTriangles);

		// Set draw mode to kSelectionOnly: 
		//    - the item will only be visible in viewport 2.0 selection
		selectionItem->setDrawMode(MHWRender::MGeometry::kSelectionOnly);

		// Set the selection mask to kSelectMeshFaces : we want the render item to be used for Face Components selection
		selectionItem->setSelectionMask( MSelectionMask::kSelectMeshFaces );

		// Set depth priority to be selection so that it is above everything
		selectionItem->depthPriority( MHWRender::MRenderItem::sSelectionDepthPriority );
		list.append(selectionItem);

		MHWRender::MShaderInstance* shader = shaderMgr->getStockShader(
			MHWRender::MShaderManager::k3dSolidShader );
		if (shader)
		{
			// Assign shader.
			selectionItem->setShader(shader, NULL);

			// once assigned, no need to hold on to shader instance
			shaderMgr->releaseShader(shader);
		}
	}
	else
	{
		selectionItem = list.itemAt(index);
	}

	if (selectionItem)
	{
		selectionItem->enable(true);

		apiMeshHWSelectionUserData* mySelectionData = dynamic_cast<apiMeshHWSelectionUserData*>(selectionItem->customData());
		if (!mySelectionData)
		{
			// create the custom data
			mySelectionData = new apiMeshHWSelectionUserData();
			selectionItem->setCustomData(mySelectionData);
		}
		// update the custom data
		mySelectionData->fMeshGeom = fMeshGeom;
	}
}

/*
	In the event there are no shaded items we create a proxy
	render item so we can still see where the object is.
*/
void apiMeshGeometryOverride::updateProxyShadedItem(
					const MDagPath& path,
					MHWRender::MRenderItemList& list,
					const MHWRender::MShaderManager* shaderMgr)
{
	// Stock colors
	static const float dormantColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	static const float templateColor[] = { 0.45f, 0.45f, 0.45f, 1.0f };
	static const float activeTemplateColor[] = { 1.0f, 0.5f, 0.5f, 1.0f };

	// Note that we still want to raise it above shaded even though
	// we don't have a shaded render item for this override.
	// This will handle in case where there is another shaded object
	// which overlaps this object in depth
	//
	static const bool raiseAboveShaded = true;
	unsigned int shadedDrawMode =
		MHWRender::MGeometry::kShaded | MHWRender::MGeometry::kTextured;
	// Mark proxy item as wireframe if not using a material shader
	//
	bool useFragmentShader = fProxyShader < 0;
	if ( !useFragmentShader )
		shadedDrawMode |= MHWRender::MGeometry::kWireframe;

	// Fragment + stipple shaders required triangles. All others
	// in the possible list requires lines
	//
	MHWRender::MGeometry::Primitive primitive = MHWRender::MGeometry::kLines;
	bool filledProxy = ( useFragmentShader
						||
						( fProxyShader == MHWRender::MShaderManager::k3dStippleShader ) );
	if (filledProxy)
	{
		primitive = MHWRender::MGeometry::kTriangles;
	}

	MHWRender::MRenderItem* proxyItem = NULL;
	int index = list.indexOf(sShadedProxyItemName);
	if (index < 0)
	{
		proxyItem = MHWRender::MRenderItem::Create(
			sShadedProxyItemName,
			filledProxy ? MHWRender::MRenderItem::MaterialSceneItem : MHWRender::MRenderItem::NonMaterialSceneItem,
			primitive);
		proxyItem->setDrawMode((MHWRender::MGeometry::DrawMode) shadedDrawMode);
		proxyItem->depthPriority( raiseAboveShaded
			? MHWRender::MRenderItem::sActiveWireDepthPriority
			: MHWRender::MRenderItem::sDormantWireDepthPriority );

		if (fExternalItems_NoShadowCast)
			proxyItem->castsShadows( false );
		else
			proxyItem->castsShadows( fCastsShadows );
		if (fExternalItems_NoShadowReceive)
			proxyItem->receivesShadows( false );
		else
			proxyItem->receivesShadows( fReceivesShadows );
		if (fExternalItems_NoPostEffects)
			proxyItem->setExcludedFromPostEffects( true );

		list.append(proxyItem);

		// We'll draw the proxy with a proxy shader as a visual cue
		//
		MHWRender::MShaderInstance* shader = NULL;
		if (useFragmentShader)
		{
			shader = shaderMgr->getFragmentShader("mayaLambertSurface", "outSurfaceFinal", true);
			static const float sBlue[] = {0.4f, 0.4f, 1.0f};
			shader->setParameter("color", sBlue);
			shader->setIsTransparent(false);
		}
		else
		{
			shader = shaderMgr->getStockShader( (MHWRender::MShaderManager::MStockShader)fProxyShader );
		}
		if (shader)
		{
			if (!filledProxy)
				setLineWidth(shader, 10.0f);

			// assign shader
			proxyItem->setShader(shader);
			// once assigned, no need to hold on to shader instance
			shaderMgr->releaseShader(shader);
		}
	}
	else
	{
		proxyItem = list.itemAt(index);
	}

	// As this is a shaded item it is up to the plug-in to determine
	// on each update how to handle shadowing and effects.
	// Especially note that shadowing changes on the DAG object will trigger
	// a call to updateRenderItems()
	//
	if (fExternalItems_NoShadowCast)
		proxyItem->castsShadows( false );
	else
		proxyItem->castsShadows( fCastsShadows );
	if (fExternalItems_NoShadowReceive)
		proxyItem->receivesShadows( false );
	else
		proxyItem->receivesShadows( fReceivesShadows );
	if (fExternalItems_NoPostEffects)
		proxyItem->setExcludedFromPostEffects( true );

	MHWRender::DisplayStatus displayStatus =
		MHWRender::MGeometryUtilities::displayStatus(path);

	MColor wireColor = MHWRender::MGeometryUtilities::wireframeColor(path);

	// Check for any shaded render items. A lack of one indicates
	// there is no shader assigned to the object.
	//
	bool haveShadedItems = false;
	for (int i=0; i<list.length(); i++)
	{
		MHWRender::MRenderItem *item = list.itemAt(i);
		if (!item)
			continue;
		MHWRender::MGeometry::DrawMode drawMode = item->drawMode();
		if (drawMode == MHWRender::MGeometry::kShaded
			|| drawMode == MHWRender::MGeometry::kTextured
			)
		{
			if (item->name() != sShadedTemplateItemName)
			{
				haveShadedItems = true;
				break;
			}
		}
	}

	// If we are missing shaded render items then enable
	// the proxy. Otherwise disable it.
	//
	if (filledProxy)
	{
		// If templated then hide filled proxy
		if (path.isTemplated())
			proxyItem->enable(false);
		else
			proxyItem->enable(!haveShadedItems);
	}
	else
		proxyItem->enable(!haveShadedItems);

	// Note that we do not toggle the item on and off just based on
	// display state. If this was so then call to MRenderer::setLightsAndShadowsDirty()
	// would be required as shadow map update does not monitor display state.
	//
	if (proxyItem)
	{
		MHWRender::MShaderInstance* shader = proxyItem->getShader();

		switch (displayStatus) {
		case MHWRender::kTemplate:
			if (shader)
			{
				setSolidColor( shader, !fUseCustomColors ? &(wireColor.r) : templateColor);
			}
			break;
		case MHWRender::kActiveTemplate:
			if (shader)
			{
				setSolidColor( shader, !fUseCustomColors ? &(wireColor.r) : activeTemplateColor );
			}
			break;
		case MHWRender::kDormant:
			if (shader)
			{
				setSolidColor( shader, !fUseCustomColors ? &(wireColor.r) : dormantColor);
			}
			break;
		default:
			break;
		}
	}
}

/*
	Update render items. Shaded render item is provided so this
	method will be adding and updating UI render items only.
*/
void apiMeshGeometryOverride::updateRenderItems(
	const MDagPath& path,
	MHWRender::MRenderItemList& list)
{
	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (!renderer) return;
	const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
	if (!shaderMgr) return;

	MFnDagNode dagNode(path);
	MPlug castsShadowsPlug = dagNode.findPlug("castsShadows", false);
	fCastsShadows = castsShadowsPlug.asBool();
	MPlug receiveShadowsPlug = dagNode.findPlug("receiveShadows", false);
	fReceivesShadows = receiveShadowsPlug.asBool();

	// Update wireframe render items
	updateDormantAndTemplateWireframeItems(path, list, shaderMgr);
	updateActiveWireframeItem(path, list, shaderMgr);

	// Update vertex render items
	updateDormantVerticesItem(path, list, shaderMgr);
	updateActiveVerticesItem(path, list, shaderMgr);

	// Update vertex numeric render items
	updateVertexNumericItems(path, list, shaderMgr);

	// Update face center item
	if(fDrawFaceCenters)
	{
		updateWireframeModeFaceCenterItem(path, list, shaderMgr);
		updateShadedModeFaceCenterItem(path, list, shaderMgr);
	}

	// Update "affected" edge and face render items
	updateAffectedComponentItems(path, list, shaderMgr);

	// Update faces and edges selection items
	updateSelectionComponentItems(path, list, shaderMgr);

	// Update proxy shaded render item
	updateProxyShadedItem(path, list, shaderMgr);

	// Test overrides on existing shaded items.
	// In this case it is not valid to override these states
	// so there should be no change in behaviour.
	//
	const bool testShadedOverrides = fInternalItems_NoShadowCast || fInternalItems_NoShadowReceive || fInternalItems_NoPostEffects;
	if (testShadedOverrides)
	{
		for (int i=0; i<list.length(); i++)
		{
			MHWRender::MRenderItem *item = list.itemAt(i);
			if (!item)
				continue;
			MHWRender::MGeometry::DrawMode drawMode = item->drawMode();
			if (drawMode == MHWRender::MGeometry::kShaded
				|| drawMode == MHWRender::MGeometry::kTextured
				)
			{
				if (item->name() != sShadedTemplateItemName
					//&& item->name() != sShadedProxyItemName
					)
				{
					if (fInternalItems_NoShadowCast)
						item->castsShadows( false );
					else
						item->castsShadows( fCastsShadows );
					if (fInternalItems_NoShadowReceive)
						item->receivesShadows( false );
					else
						item->receivesShadows( fReceivesShadows );
					if (fInternalItems_NoPostEffects)
						item->setExcludedFromPostEffects( true );
 				}
			}
		}
	}
}

/*
	Clone a vertex buffer to fulfill a duplicate requirement.
	Can happen for effects asking for multiple UV streams by
	name.
*/
void apiMeshGeometryOverride::cloneVertexBuffer(
		MHWRender::MVertexBuffer* srcBuffer,
		MHWRender::MGeometry& data,
		MHWRender::MVertexBufferDescriptor& desc,
		unsigned int bufferSize,
		bool debugPopulateGeometry)
{
	if (!srcBuffer)
		return;

	MHWRender::MVertexBuffer* destBuffer = data.createVertexBuffer(desc);
	if (destBuffer)
	{
		if (debugPopulateGeometry)
		{
			printf(">>> Clone data for active vertex requirement with name %s. Semantic = %d\n",
					desc.name().asChar(), desc.semantic() );
		}
		void* destDataBuffer = destBuffer->acquire(bufferSize, true /*writeOnly - we don't need the current buffer values*/);
	 	void* srcDataBuffer = srcBuffer->map();
	 	if (srcDataBuffer && destDataBuffer)
			memcpy(destDataBuffer, srcDataBuffer, bufferSize * desc.dataTypeSize() * desc.dimension());

	 	if (destDataBuffer)
			destBuffer->commit(destDataBuffer);

	 	srcBuffer->unmap();	
	}
}

/*
	Examine the geometry requirements and create / update the
	appropriate data streams. As render items specify both named and
	unnamed data streams, both need to be handled here.
*/
void apiMeshGeometryOverride::updateGeometryRequirements(
		const MHWRender::MGeometryRequirements& requirements,
		MHWRender::MGeometry& data,
		unsigned int activeVertexCount,
		unsigned int totalVerts,
		bool debugPopulateGeometry)
{
	// Vertex data
	MHWRender::MVertexBuffer* positionBuffer = NULL;
	float* positions = NULL;

	MHWRender::MVertexBuffer* vertexNumericIdBuffer = NULL;
	float* vertexNumericIds = NULL;
	MHWRender::MVertexBuffer* vertexNumericIdPositionBuffer = NULL;
	float* vertexNumericIdPositions = NULL;
	MHWRender::MVertexBuffer* vertexNumericLocationBuffer = NULL;
	float* vertexNumericLocations = NULL;
	MHWRender::MVertexBuffer* vertexNumericLocationPositionBuffer = NULL;
	float* vertexNumericLocationPositions = NULL;

	MHWRender::MVertexBuffer* activeVertexPositionBuffer = NULL;
	float* activeVertexPositions = NULL;
	MHWRender::MVertexBuffer* activeVertexUVBuffer = NULL;
	float* activeVertexUVs = NULL;
	MHWRender::MVertexBuffer* faceCenterPositionBuffer = NULL;
	float* faceCenterPositions = NULL;
	MHWRender::MVertexBuffer* normalBuffer = NULL;
	float* normals = NULL;
	MHWRender::MVertexBuffer* cpvBuffer = NULL;
	float* cpv = NULL;
	MHWRender::MVertexBuffer* uvBuffer = NULL;
	float* uvs = NULL;
	int numUVs = fMeshGeom->uvcoords.uvcount();


	const MHWRender::MVertexBufferDescriptorList& descList =
		requirements.vertexRequirements();
	int numVertexReqs = descList.length();
	MHWRender::MVertexBufferDescriptor desc;
	bool* satisfiedRequirements = new bool[numVertexReqs];
	for (int reqNum=0; reqNum<numVertexReqs; reqNum++)
	{
		satisfiedRequirements[reqNum] = false;
		if (!descList.getDescriptor(reqNum, desc))
		{
			continue;
		}

		// Fill in vertex data for drawing active vertex components (if drawSharedActiveVertices=true)
		//
		if (fDrawSharedActiveVertices && (desc.name() == sActiveVertexStreamName))
		{
			switch (desc.semantic())
			{
			case MHWRender::MGeometry::kPosition:
				{
					if (!activeVertexPositionBuffer)
					{
						activeVertexPositionBuffer = data.createVertexBuffer(desc);
						if (activeVertexPositionBuffer)
						{
							satisfiedRequirements[reqNum] = true;
							if (debugPopulateGeometry)
							{
								printf(">>> Fill in data for active vertex requirement[%d] with name %s. Semantic = %d\n",
										reqNum, desc.name().asChar(), desc.semantic() );
							}
							activeVertexPositions = (float*)activeVertexPositionBuffer->acquire(activeVertexCount, true /*writeOnly - we don't need the current buffer values*/);
						}
					}
				}
				break;
			case MHWRender::MGeometry::kTexture:
				{
					if (!activeVertexUVBuffer)
					{
						activeVertexUVBuffer = data.createVertexBuffer(desc);
						if (activeVertexUVBuffer)
						{
							satisfiedRequirements[reqNum] = true;
							if (debugPopulateGeometry)
							{
								printf(">>> Fill in data for active vertex requirement[%d] with name %s. Semantic = %d\n",
										reqNum, desc.name().asChar(), desc.semantic() );
							}
							activeVertexUVs = (float*)activeVertexUVBuffer->acquire(activeVertexCount, true /*writeOnly - we don't need the current buffer values*/);
						}
					}

				}
			default:
				// do nothing for stuff we don't understand
				break;
			}
		}

		// Fill in vertex data for drawing face center components (if fDrawFaceCenters=true)
		//
		else if (fDrawFaceCenters && (desc.name() == sFaceCenterStreamName))
		{
			switch (desc.semantic())
			{
			case MHWRender::MGeometry::kPosition:
				{
					if (!faceCenterPositionBuffer)
					{
						faceCenterPositionBuffer = data.createVertexBuffer(desc);
						if (faceCenterPositionBuffer)
						{
							satisfiedRequirements[reqNum] = true;
							if (debugPopulateGeometry)
							{
								printf(">>> Fill in data for face center requirement[%d] with name %s. Semantic = %d\n",
										reqNum, desc.name().asChar(), desc.semantic() );
							}
							faceCenterPositions = (float*)faceCenterPositionBuffer->acquire(fMeshGeom->faceCount, true /*writeOnly - we don't need the current buffer values*/);
						}
					}
				}
				break;
			default:
				// do nothing for stuff we don't understand
				break;
			}
		}

		// Fill vertex stream data used for dormant vertex, wireframe and shaded drawing.
		// Fill also for active vertices if (fDrawSharedActiveVertices=false)
		else
		{
			if (debugPopulateGeometry)
			{
				printf(">>> Fill in data for requirement[%d] with name %s. Semantic = %d\n",
					reqNum, desc.name().asChar(), desc.semantic() );
			}
			switch (desc.semantic())
			{
			case MHWRender::MGeometry::kPosition:
				{
					if (desc.name() == sVertexIdItemName)
					{
						if (!vertexNumericIdPositionBuffer)
						{
							vertexNumericIdPositionBuffer = data.createVertexBuffer(desc);
							if (vertexNumericIdPositionBuffer)
							{
								satisfiedRequirements[reqNum] = true;
								if (debugPopulateGeometry)
									printf("Acquire 1float-numeric position buffer\n");
								vertexNumericIdPositions = (float*)vertexNumericIdPositionBuffer->acquire(totalVerts, true /*writeOnly - we don't need the current buffer values*/);
							}
						}
					}
					else if (desc.name() == sVertexPositionItemName)
					{
						if (!vertexNumericLocationPositionBuffer)
						{
							vertexNumericLocationPositionBuffer = data.createVertexBuffer(desc);
							if (vertexNumericLocationPositionBuffer)
							{
								satisfiedRequirements[reqNum] = true;
								if (debugPopulateGeometry)
									printf("Acquire 3float-numeric position buffer\n");
								vertexNumericLocationPositions = (float*)vertexNumericLocationPositionBuffer->acquire(totalVerts, true /*writeOnly */);
							}
						}
					}
					else
					{
						if (!positionBuffer)
						{
							positionBuffer = data.createVertexBuffer(desc);
							if (positionBuffer)
							{
								satisfiedRequirements[reqNum] = true;
								if (debugPopulateGeometry)
									printf("Acquire unnamed position buffer\n");
								positions = (float*)positionBuffer->acquire(totalVerts, true /*writeOnly - we don't need the current buffer values*/);
							}
						}
					}

				}
				break;
			case MHWRender::MGeometry::kNormal:
				{
					if (!normalBuffer)
					{
						normalBuffer = data.createVertexBuffer(desc);
						if (normalBuffer)
						{
							satisfiedRequirements[reqNum] = true;
							normals = (float*)normalBuffer->acquire(totalVerts, true /*writeOnly - we don't need the current buffer values*/);
						}
					}
				}
				break;
			case MHWRender::MGeometry::kTexture:
				{
					static const MString numericValue("numericvalue");
					static const MString numeric3Value("numeric3value");

					// Fill in single numeric field
					if ((desc.semanticName().toLowerCase() == numericValue) && (desc.name() == sVertexIdItemName))
					{
						if (!vertexNumericIdBuffer)
						{
							vertexNumericIdBuffer = data.createVertexBuffer(desc);
							if (vertexNumericIdBuffer)
							{
								satisfiedRequirements[reqNum] = true;
								if (debugPopulateGeometry)
									printf("Acquire 1float numeric buffer\n");
								vertexNumericIds = (float*)vertexNumericIdBuffer->acquire(totalVerts, true /*writeOnly */);
							}
						}
					}
					// Fill in triple numeric field
					else if ((desc.semanticName().toLowerCase() == numeric3Value) && (desc.name() == sVertexPositionItemName))
					{
						if (!vertexNumericLocationBuffer)
						{
							vertexNumericLocationBuffer = data.createVertexBuffer(desc);
							if (vertexNumericLocationBuffer)
							{
								satisfiedRequirements[reqNum] = true;
								if (debugPopulateGeometry)
									printf("Acquire 3float numeric location buffer\n");
								vertexNumericLocations = (float*)vertexNumericLocationBuffer->acquire(totalVerts, true /*writeOnly */);
							}
						}
					}
					// Fill in uv values
					else if (desc.name() != sVertexIdItemName &&
							 desc.name() != sVertexPositionItemName)
					{
						if (!uvBuffer)
						{
							uvBuffer = data.createVertexBuffer(desc);
							if (uvBuffer)
							{
								satisfiedRequirements[reqNum] = true;
								if (debugPopulateGeometry)
									printf("Acquire a uv buffer\n");
								uvs = (float*)uvBuffer->acquire(totalVerts, true /*writeOnly - we don't need the current buffer values*/);
							}
						}
					}
				}
				break;
			case MHWRender::MGeometry::kColor:
				{
					if (!cpvBuffer)
					{
						cpvBuffer = data.createVertexBuffer(desc);
						if (cpvBuffer)
						{
							satisfiedRequirements[reqNum] = true;
							cpv = (float*)cpvBuffer->acquire(totalVerts, true /*writeOnly - we don't need the current buffer values*/);
						}
					}
				}
				break;
			default:
				// do nothing for stuff we don't understand
				break;
			}
		}
	}

	int vid = 0;
	int pid = 0;
	int nid = 0;
	int uvid = 0;
	int cid = 0;
	for (int i=0; i<fMeshGeom->faceCount; i++)
	{
		// ignore degenerate faces
		int numVerts = fMeshGeom->face_counts[i];
		if (numVerts > 2)
		{
			for (int j=0; j<numVerts; j++)
			{
				if (positions || vertexNumericIdPositions || 
					vertexNumericLocationPositions || vertexNumericLocations)
				{
					MPoint position = fMeshGeom->vertices[fMeshGeom->face_connects[vid]];
					// Position used as position
					if (positions)
					{
						positions[pid] = (float)position[0];
						positions[pid+1] = (float)position[1];
						positions[pid+2] = (float)position[2];
					}
					// Move the id's a bit to avoid overlap. Position used as position.
					if (vertexNumericIdPositions)
					{
						vertexNumericIdPositions[pid] = (float)(position[0])+1.0f;
						vertexNumericIdPositions[pid+1] = (float)(position[1])+1.0f;
						vertexNumericIdPositions[pid+2] = (float)(position[2])+1.0f;
					}
					// Move the locations a bit to avoid overlap. Position used as position.
					if (vertexNumericLocationPositions)
					{
						vertexNumericLocationPositions[pid] = (float)(position[0])+3.0f;
						vertexNumericLocationPositions[pid+1] = (float)(position[1])+3.0f;
						vertexNumericLocationPositions[pid+2] = (float)(position[2])+3.0f;
					}
					// Position used as numeric display.
					if (vertexNumericLocations)
					{
						vertexNumericLocations[pid] = (float)position[0];
						vertexNumericLocations[pid+1] = (float)position[1];
						vertexNumericLocations[pid+2] = (float)position[2];
					}
					pid += 3;
				}

				if (normals)
				{
					MVector normal = fMeshGeom->normals[fMeshGeom->face_connects[vid]];
					normals[nid++] = (float)normal[0];
					normals[nid++] = (float)normal[1];
					normals[nid++] = (float)normal[2];
				}

				if (uvs)
				{
					float u = 0.0f;
					float v = 0.0f;
					if (numUVs > 0)
					{
						int uvNum = fMeshGeom->uvcoords.uvId(vid);
						fMeshGeom->uvcoords.getUV(uvNum, u, v);
					}
					uvs[uvid++] = u;
					uvs[uvid++] = v;
				}
				// Just same fake colors to show filling in requirements for
				// color-per-vertex (CPV)
				if (cpv)
				{
					MPoint position = fMeshGeom->vertices[fMeshGeom->face_connects[vid]];
					cpv[cid++] = (float)position[0];
					cpv[cid++] = (float)position[1];
					cpv[cid++] = (float)position[2];
					cpv[cid++] = 1.0f;
				}
				// Vertex id's used for numeric display
				if (vertexNumericIds)
				{
					vertexNumericIds[vid] = (float)(fMeshGeom->face_connects[vid]);
				}
				vid++;
			}
		}
		else if (numVerts > 0)
		{
			vid += numVerts;
		}
	}

	if (positions)
	{
		positionBuffer->commit(positions);
	}

	if (normals)
	{
		normalBuffer->commit(normals);
	}

	if (uvs)
	{
		uvBuffer->commit(uvs);
	}
	if (cpv)
	{
		cpvBuffer->commit(cpv);
	}
	
	if (vertexNumericIds)
	{
		vertexNumericIdBuffer->commit(vertexNumericIds);
	}
	if (vertexNumericIdPositions)
	{
		vertexNumericIdPositionBuffer->commit(vertexNumericIdPositions);
	}
	if (vertexNumericLocations)
	{
		vertexNumericLocationBuffer->commit(vertexNumericLocations);
	}
	if (vertexNumericLocationPositions)
	{
		vertexNumericLocationPositionBuffer->commit(vertexNumericLocationPositions);
	}

	// Fill in active vertex data buffer (only when fDrawSharedActiveVertices=true
	// which results in activeVertexPositions and activeVertexPositionBuffer being non-NULL)
	//
	if (activeVertexPositions && activeVertexPositionBuffer)
	{
		if (debugPopulateGeometry)
		{
			printf(">>> Fill in the data for active vertex position buffer base on component list\n");
		}
		// Fill in position buffer with positions based on active vertex indexing list
		//
		pid = 0;
		if (activeVertexCount > fMeshGeom->vertices.length())
			activeVertexCount = fMeshGeom->vertices.length();
		for (unsigned int i=0; i<activeVertexCount; i++)
		{
			MPoint position = fMeshGeom->vertices[ fActiveVertices[i] ];
			activeVertexPositions[pid++] = (float)position[0];
			activeVertexPositions[pid++] = (float)position[1];
			activeVertexPositions[pid++] = (float)position[2];
		}
		activeVertexPositionBuffer->commit(activeVertexPositions);
	}
	if (activeVertexUVs && activeVertexUVBuffer)
	{
		if (debugPopulateGeometry)
		{
			printf(">>> Fill in the data for active vertex uv buffer base on component list\n");
		}
		// Fill in position buffer with positions based on active vertex indexing list
		//
		pid = 0;
		if (activeVertexCount > fMeshGeom->vertices.length())
			activeVertexCount = fMeshGeom->vertices.length();
		for (unsigned int i=0; i<activeVertexCount; i++)
		{
			activeVertexUVs[pid++] = (float)i/ (float)activeVertexCount;
		}
		activeVertexUVBuffer->commit(activeVertexUVs);
	}

	// Fill in face center data buffer (only when fDrawFaceCenter=true
	// which results in faceCenterPositions and faceCenterPositionBuffer being non-NULL)
	//
	if (faceCenterPositions && faceCenterPositionBuffer)
	{
		if (debugPopulateGeometry)
		{
			printf(">>> Fill in the data for face center position buffer\n");
		}
		// Fill in face center buffer with positions based on realtime calculations.
		//
		pid = 0;
		vid = 0;
		for (int faceId=0; faceId < fMeshGeom->faceCount; faceId++)
		{
			//tmp variables for calculating the face center position.
			double x = 0.0;
			double y = 0.0;
			double z = 0.0;

			MPoint faceCenterPosition;

			// ignore degenerate faces
			int numVerts = fMeshGeom->face_counts[faceId];
			if (numVerts > 2){
				for (int v=0; v<numVerts; v++)
				{
					MPoint face_vertex_position = fMeshGeom->vertices[fMeshGeom->face_connects[vid]];
					x += face_vertex_position[0];
					y += face_vertex_position[1];
					z += face_vertex_position[2];

					vid++;
				}

				faceCenterPosition[0] = (float)x/numVerts;
				faceCenterPosition[1] = (float)y/numVerts;
				faceCenterPosition[2] = (float)z/numVerts;

				faceCenterPositions[pid++] = (float)faceCenterPosition[0];
				faceCenterPositions[pid++] = (float)faceCenterPosition[1];
				faceCenterPositions[pid++] = (float)faceCenterPosition[2];
			}
			else if(numVerts > 0)
			{
				vid += numVerts;
			}

		}
		faceCenterPositionBuffer->commit(faceCenterPositions);
	}

	// Run around a second time and handle duplicate buffers and unknown buffers
	for (int reqNum=0; reqNum<numVertexReqs; reqNum++)
	{
		if (satisfiedRequirements[reqNum] || !descList.getDescriptor(reqNum, desc))
		{
			continue;
		}

		if (fDrawSharedActiveVertices && (desc.name() == sActiveVertexStreamName))
		{
			switch (desc.semantic())
			{
			case MHWRender::MGeometry::kPosition:
				{
					satisfiedRequirements[reqNum] = true;
					cloneVertexBuffer(activeVertexPositionBuffer, data, desc, activeVertexCount, debugPopulateGeometry);
				}
				break;
			case MHWRender::MGeometry::kTexture:
				{
					satisfiedRequirements[reqNum] = true;
					cloneVertexBuffer(activeVertexUVBuffer, data, desc, activeVertexCount, debugPopulateGeometry);
				}
			default:
				break;
			}
		}

		else if (fDrawFaceCenters && (desc.name() == sFaceCenterStreamName))
		{
			switch (desc.semantic())
			{
			case MHWRender::MGeometry::kPosition:
				{
					satisfiedRequirements[reqNum] = true;
					cloneVertexBuffer(faceCenterPositionBuffer, data, desc, fMeshGeom->faceCount, debugPopulateGeometry);
				}
				break;
			default:
				break;
			}
		}
		else
		{
			switch (desc.semantic())
			{
			case MHWRender::MGeometry::kPosition:
				{
					if (desc.name() == sVertexIdItemName)
					{
						satisfiedRequirements[reqNum] = true;
						cloneVertexBuffer(vertexNumericIdPositionBuffer, data, desc, totalVerts, debugPopulateGeometry);
					}
					else if (desc.name() == sVertexPositionItemName)
					{
						satisfiedRequirements[reqNum] = true;
						cloneVertexBuffer(vertexNumericLocationPositionBuffer, data, desc, totalVerts, debugPopulateGeometry);
					}
					else
					{
						satisfiedRequirements[reqNum] = true;
						cloneVertexBuffer(positionBuffer, data, desc, totalVerts, debugPopulateGeometry);
					}

				}
				break;
			case MHWRender::MGeometry::kNormal:
				{
					satisfiedRequirements[reqNum] = true;
					cloneVertexBuffer(normalBuffer, data, desc, totalVerts, debugPopulateGeometry);
				}
				break;
			case MHWRender::MGeometry::kTexture:
				{
					static const MString numericValue("numericvalue");
					static const MString numeric3Value("numeric3value");

					if ((desc.semanticName().toLowerCase() == numericValue) && (desc.name() == sVertexIdItemName))
					{
						satisfiedRequirements[reqNum] = true;
						cloneVertexBuffer(vertexNumericIdBuffer, data, desc, totalVerts, debugPopulateGeometry);
					}
					else if ((desc.semanticName().toLowerCase() == numeric3Value) && (desc.name() == sVertexPositionItemName))
					{
						satisfiedRequirements[reqNum] = true;
						cloneVertexBuffer(vertexNumericLocationBuffer, data, desc, totalVerts, debugPopulateGeometry);
					}
					else if (desc.name() != sVertexIdItemName &&
							 desc.name() != sVertexPositionItemName)
					{
						satisfiedRequirements[reqNum] = true;
						cloneVertexBuffer(uvBuffer, data, desc, totalVerts, debugPopulateGeometry);
					}
				}
				break;
			case MHWRender::MGeometry::kColor:
				{
					satisfiedRequirements[reqNum] = true;
					cloneVertexBuffer(cpvBuffer, data, desc, totalVerts, debugPopulateGeometry);
				}
				break;
			default:
				break;
			}
		}

		if (!satisfiedRequirements[reqNum])
		{
			// We have a strange buffer request we do not understand. Provide a set of Zeros sufficient to cover
			// totalVerts:
			MHWRender::MVertexBuffer* destBuffer = data.createVertexBuffer(desc);
			if (destBuffer)
			{
				if (debugPopulateGeometry)
				{
					printf(">>> Dummy data for active vertex requirement with name %s. Semantic = %d\n",
							desc.name().asChar(), desc.semantic() );
				}
				void* destDataBuffer = destBuffer->acquire(totalVerts, true /*writeOnly - we don't need the current buffer values*/);
				if (destDataBuffer)
				{
					memset(destDataBuffer, 0, totalVerts * desc.dataTypeSize() * desc.dimension());
					destBuffer->commit(destDataBuffer);
				}
			}
		}
	}
	delete satisfiedRequirements;
}

/*
	Create / update indexing required to draw wireframe render items.
	There can be more than one render item using the same wireframe indexing
	so it is passed in as an argument. If it is not null then we can
	reuse it instead of creating new indexing.
*/
void apiMeshGeometryOverride::updateIndexingForWireframeItems(MHWRender::MIndexBuffer* wireIndexBuffer,
															const MHWRender::MRenderItem* item,
															MHWRender::MGeometry& data,
															unsigned int totalVerts)
{
	// Wireframe index buffer is same for both wireframe and selected render item
	// so we only compute and allocate it once, but reuse it for both render items
	if (!wireIndexBuffer)
	{
		wireIndexBuffer = data.createIndexBuffer(MHWRender::MGeometry::kUnsignedInt32);
		if (wireIndexBuffer)
		{
			unsigned int* buffer = (unsigned int*)wireIndexBuffer->acquire(2*totalVerts, true /*writeOnly - we don't need the current buffer values*/);
			if (buffer)
			{
				int vid = 0;
				int first = 0;
				unsigned int idx = 0;
				for (int faceIdx=0; faceIdx<fMeshGeom->faceCount; faceIdx++)
				{
					// ignore degenerate faces
					int numVerts = fMeshGeom->face_counts[faceIdx];
					if (numVerts > 2)
					{
						first = vid;
						for (int v=0; v<numVerts-1; v++)
						{
							buffer[idx++] = vid++;
							buffer[idx++] = vid;
						}
						buffer[idx++] = vid++;
						buffer[idx++] = first;
					}
					else
					{
						vid += numVerts;
					}
				}

				wireIndexBuffer->commit(buffer);
			}
		}
	}

	// Associate same index buffer with either render item
	if (wireIndexBuffer)
	{
		item->associateWithIndexBuffer(wireIndexBuffer);
	}
}

/*
	Create / update indexing for render items which draw dormant vertices
*/
void apiMeshGeometryOverride::updateIndexingForDormantVertices(const MHWRender::MRenderItem* item,
															  MHWRender::MGeometry& data,
															  unsigned int numTriangles)
{
	MHWRender::MIndexBuffer* indexBuffer =
		data.createIndexBuffer(MHWRender::MGeometry::kUnsignedInt32);
	if (indexBuffer)
	{
		unsigned int* buffer = (unsigned int*)indexBuffer->acquire(3*numTriangles, true /*writeOnly - we don't need the current buffer values*/);
		if (buffer)
		{
			// compute index data for triangulated convex polygons sharing
			// poly vertex data among triangles
			unsigned int base = 0;
			unsigned int idx = 0;
			for (int faceIdx=0; faceIdx<fMeshGeom->faceCount; faceIdx++)
			{
				// ignore degenerate faces
				int numVerts = fMeshGeom->face_counts[faceIdx];
				if (numVerts > 2)
				{
					for (int v=1; v<numVerts-1; v++)
					{
						buffer[idx++] = base;
						buffer[idx++] = base+v;
						buffer[idx++] = base+v+1;
					}
					base += numVerts;
				}
			}

			indexBuffer->commit(buffer);
			item->associateWithIndexBuffer(indexBuffer);
		}
	}
}

/*
	Create / update indexing for render items which draw active vertices
*/
void apiMeshGeometryOverride::updateIndexingForVertices(const MHWRender::MRenderItem* item,
														MHWRender::MGeometry& data,
														unsigned int numTriangles,
														unsigned int activeVertexCount,
														bool debugPopulateGeometry)
{
	MHWRender::MIndexBuffer* indexBuffer =
		data.createIndexBuffer(MHWRender::MGeometry::kUnsignedInt32);

	if (indexBuffer)
	{
		unsigned int* buffer = NULL;

		// If drawing shared active vertices then the indexing degenerates into
		// a numerically increasing index value. Otherwise a remapping from
		// the active vertex list indexing to the unshared position stream is required.
		//
		// 1. Create indexing for shared positions. In this case it
		// is a degenerate list since the position buffer was created
		// in linear ascending order.
		//
		if (fDrawSharedActiveVertices)
		{
			buffer = (unsigned int*)indexBuffer->acquire(activeVertexCount, true /*writeOnly - we don't need the current buffer values*/);
			if (buffer)
			{
				if (debugPopulateGeometry)
					printf(">>> Set up indexing for shared vertices\n");
				for (unsigned int i=0; i<activeVertexCount; i++)
				{
					buffer[i] = i;
				}
			}
		}

		// 2. Create indexing to remap to unshared positions
		//
		else
		{
			if (debugPopulateGeometry)
				printf(">>> Set up indexing for unshared vertices\n");


			buffer = (unsigned int*)indexBuffer->acquire(3*numTriangles, true /*writeOnly - we don't need the current buffer values*/);
			if (buffer)
			{
				for (unsigned int i=0; i<3*numTriangles; i++)
				{
					buffer[i] = 3*numTriangles+1;
				}

				const std::set<int>& selectionIdSet = fActiveVerticesSet;
				std::set<int>::const_iterator selectionIdSetIter;
				std::set<int>::const_iterator selectionIdSetIterEnd = selectionIdSet.end();

				// compute index data for triangulated convex polygons sharing
				// poly vertex data among triangles
				unsigned int base = 0;
				unsigned int lastFound = 0;
				unsigned int idx = 0;

				for (int faceIdx=0; faceIdx<fMeshGeom->faceCount; faceIdx++)
				{
					// ignore degenerate faces
					int numVerts = fMeshGeom->face_counts[faceIdx];
					if (numVerts > 2)
					{
						for (int v=1; v<numVerts-1; v++)
						{
							int vertexId = fMeshGeom->face_connects[base];
							selectionIdSetIter = selectionIdSet.find( vertexId );
							if (selectionIdSetIter != selectionIdSetIterEnd)
							{
								buffer[idx++] = base;
								lastFound = base;
							}

							vertexId = fMeshGeom->face_connects[base+v];
							selectionIdSetIter = selectionIdSet.find( vertexId );
							if (selectionIdSetIter != selectionIdSetIterEnd)
							{
								buffer[idx++] = base+v;
								lastFound = base+v;
							}

							vertexId = fMeshGeom->face_connects[base+v+1];
							selectionIdSetIter = selectionIdSet.find( vertexId );
							if (selectionIdSetIter != selectionIdSetIterEnd)
							{
								buffer[idx++] = base+v+1;
								lastFound = base+v+1;
							}
						}
						base += numVerts;
					}
				}

				for (unsigned int i=0; i<3*numTriangles; i++)
				{
					if (buffer[i] == 3*numTriangles+1)
						buffer[i] = lastFound;
				}
			}
		}

		if (buffer)
			indexBuffer->commit(buffer);
		item->associateWithIndexBuffer(indexBuffer);
	}
}

/*
	Create / update indexing for render items which draw face centers
*/
void apiMeshGeometryOverride::updateIndexingForFaceCenters(const MHWRender::MRenderItem* item, MHWRender::MGeometry& data, bool debugPopulateGeometry)
{
	MHWRender::MIndexBuffer* indexBuffer =
		data.createIndexBuffer(MHWRender::MGeometry::kUnsignedInt32);

	if (indexBuffer)
	{
		unsigned int* buffer = NULL;

		buffer = (unsigned int*)indexBuffer->acquire(fMeshGeom->faceCount, true /*writeOnly - we don't need the current buffer values*/);
		if (buffer)
		{
			if (debugPopulateGeometry)
				printf(">>> Set up indexing for face centers\n");

			for (int i=0; i<fMeshGeom->faceCount; i++)
			{
					buffer[i] = 0;
			}

			unsigned int idx = 0;
			for (int i=0; i<fMeshGeom->faceCount; i++)
			{
				// ignore degenerate faces
				int numVerts = fMeshGeom->face_counts[i];
				if (numVerts > 2)
				{
					buffer[idx] = idx;
					idx++;
				}
			}
		}

		if (buffer)
			indexBuffer->commit(buffer);
		item->associateWithIndexBuffer(indexBuffer);
	}
}

/*
	Create / update indexing for render items which draw affected edges
*/
void apiMeshGeometryOverride::updateIndexingForEdges(const MHWRender::MRenderItem* item,
													  MHWRender::MGeometry& data,
													  unsigned int totalVerts,
													  bool fromSelection)
{
	MHWRender::MIndexBuffer* indexBuffer =
		data.createIndexBuffer(MHWRender::MGeometry::kUnsignedInt32);
	if (indexBuffer)
	{
		unsigned int totalEdges = 2*totalVerts;
		unsigned int totalEdgesP1 = 2*totalVerts+1;
		unsigned int* buffer = (unsigned int*)indexBuffer->acquire(totalEdges, true /*writeOnly - we don't need the current buffer values*/);
		if (buffer)
		{
			for (unsigned int i=0; i<totalEdges; i++)
			{
				buffer[i] = totalEdgesP1;
			}

			const bool displayAll = !fromSelection;
			const bool displayActives = (!displayAll && fActiveEdgesSet.size() > 0);
			const bool displayAffected = (!displayAll && !displayActives);

			const std::set<int>& selectionIdSet = (displayActives ? fActiveEdgesSet : fActiveVerticesSet);
			std::set<int>::const_iterator selectionIdSetIter;
			std::set<int>::const_iterator selectionIdSetIterEnd = selectionIdSet.end();

			unsigned int base = 0;
			unsigned int lastFound = 0;
			unsigned int idx = 0;
			int edgeId = 0;
			for (int faceIdx=0; faceIdx<fMeshGeom->faceCount; faceIdx++)
			{
				// ignore degenerate faces
				int numVerts = fMeshGeom->face_counts[faceIdx];
				if (numVerts > 2)
				{
					for (int v=0; v<numVerts; v++)
					{
						bool enableEdge = displayAll;
						unsigned int vindex1 = base + (v % numVerts);
						unsigned int vindex2 = base + ((v+1) % numVerts);

						if( displayAffected )
						{
							// Check either ends of an "edge" to see if the
							// vertex is in the active vertex list
							//
							int vertexId = fMeshGeom->face_connects[vindex1];
							selectionIdSetIter = selectionIdSet.find( vertexId );
							if (selectionIdSetIter != selectionIdSetIterEnd)
							{
								enableEdge = true;
								lastFound = vindex1;
							}

							if (!enableEdge)
							{
								int vertexId2 = fMeshGeom->face_connects[vindex2];
								selectionIdSetIter = selectionIdSet.find( vertexId2 );
								if (selectionIdSetIter != selectionIdSetIterEnd)
								{
									enableEdge = true;
									lastFound = vindex2;
								}
							}
						}
						else if( displayActives )
						{
							// Check if the edge is active
							//
							selectionIdSetIter = selectionIdSet.find( edgeId );
							if (selectionIdSetIter != selectionIdSetIterEnd)
							{
								enableEdge = true;
								lastFound = vindex1;
							}
						}

						// Add indices for "edge"
						if (enableEdge)
						{
							buffer[idx++] = vindex1;
							buffer[idx++] = vindex2;
						}
						++edgeId;
					}
					base += numVerts;
				}
			}

			if(!displayAll)
			{
				for (unsigned int i=0; i<totalEdges; i++)
				{
					if (buffer[i] == totalEdgesP1)
						buffer[i] = lastFound;
				}
			}
		}

		if (buffer)
			indexBuffer->commit(buffer);
		item->associateWithIndexBuffer(indexBuffer);
	}
}

/*
	Create / update indexing for render items which draw affected faces
*/
void apiMeshGeometryOverride::updateIndexingForFaces(const MHWRender::MRenderItem* item,
													MHWRender::MGeometry& data,
													unsigned int numTriangles,
													bool fromSelection)
{
	MHWRender::MIndexBuffer* indexBuffer =
		data.createIndexBuffer(MHWRender::MGeometry::kUnsignedInt32);
	if (indexBuffer)
	{
		unsigned int numTriangleVertices = 3*numTriangles;
		unsigned int* buffer = (unsigned int*)indexBuffer->acquire(numTriangleVertices, true /*writeOnly - we don't need the current buffer values*/);
		if (buffer)
		{
			for (unsigned int i=0; i<numTriangleVertices; i++)
			{
				buffer[i] = numTriangleVertices+1;
			}

			const bool displayAll = !fromSelection;
			const bool displayActives = (!displayAll && fActiveFacesSet.size() > 0);
			const bool displayAffected = (!displayAll && !displayActives);

			const std::set<int>& selectionIdSet = (displayActives ? fActiveFacesSet : fActiveVerticesSet);
			std::set<int>::const_iterator selectionIdSetIter;
			std::set<int>::const_iterator selectionIdSetIterEnd = selectionIdSet.end();

			unsigned int base = 0;
			unsigned int lastFound = 0;
			unsigned int idx = 0;
			for (int faceIdx=0; faceIdx<fMeshGeom->faceCount; faceIdx++)
			{
				// ignore degenerate faces
				int numVerts = fMeshGeom->face_counts[faceIdx];
				if (numVerts > 2)
				{
					bool enableFace = displayAll;

					if( displayAffected )
					{
						// Scan for any vertex in the active list
						//
						for (int v=1; v<numVerts-1; v++)
						{
							int vertexId = fMeshGeom->face_connects[base];
							selectionIdSetIter = selectionIdSet.find( vertexId );
							if (selectionIdSetIter != selectionIdSetIterEnd)
							{
								enableFace = true;
								lastFound = base;
							}

							if (!enableFace)
							{
								int vertexId2 = fMeshGeom->face_connects[base+v];
								selectionIdSetIter = selectionIdSet.find( vertexId2 );
								if (selectionIdSetIter != selectionIdSetIterEnd)
								{
									enableFace = true;
									lastFound = base+v;
								}
							}
							if (!enableFace)
							{
								int vertexId3 = fMeshGeom->face_connects[base+v+1];
								selectionIdSetIter = selectionIdSet.find( vertexId3 );
								if (selectionIdSetIter != selectionIdSetIterEnd)
								{
									enableFace = true;
									lastFound = base+v+1;
								}
							}
						}
					}
					else if( displayActives )
					{
						selectionIdSetIter = selectionIdSet.find( faceIdx );
						if (selectionIdSetIter != selectionIdSetIterEnd)
						{
							enableFace = true;
							lastFound = base;
						}
					}

					// Found an active face
					// or one active vertex on the triangle so add indexing for the entire triangle.
					//
					if (enableFace)
					{
						for (int v=1; v<numVerts-1; v++)
						{
							buffer[idx++] = base;
							buffer[idx++] = base+v;
							buffer[idx++] = base+v+1;
						}
					}
					base += numVerts;
				}
			}

			if(!displayAll)
			{
				for (unsigned int i=0; i<numTriangleVertices; i++)
				{
					if (buffer[i] == numTriangleVertices+1)
						buffer[i] = lastFound;
				}
			}
		}

		if (buffer)
			indexBuffer->commit(buffer);
		item->associateWithIndexBuffer(indexBuffer);
	}
}

/*
	Create / update indexing for render items which draw filled / shaded
	triangles.
*/
void apiMeshGeometryOverride::updateIndexingForShadedTriangles(const MHWRender::MRenderItem* item,
															  MHWRender::MGeometry& data,
															  unsigned int numTriangles)
{
	MHWRender::MIndexBuffer* indexBuffer =
		data.createIndexBuffer(MHWRender::MGeometry::kUnsignedInt32);
	if (indexBuffer)
	{
		unsigned int* buffer = (unsigned int*)indexBuffer->acquire(3*numTriangles, true /*writeOnly - we don't need the current buffer values*/);
		if (buffer)
		{
			// compute index data for triangulated convex polygons sharing
			// poly vertex data among triangles
			unsigned int base = 0;
			unsigned int idx = 0;
			for (int faceIdx=0; faceIdx<fMeshGeom->faceCount; faceIdx++)
			{
				// ignore degenerate faces
				int numVerts = fMeshGeom->face_counts[faceIdx];
				if (numVerts > 2)
				{
					for (int v=1; v<numVerts-1; v++)
					{
						buffer[idx++] = base;
						buffer[idx++] = base+v;
						buffer[idx++] = base+v+1;
					}
					base += numVerts;
				}
			}

			indexBuffer->commit(buffer);
			item->associateWithIndexBuffer(indexBuffer);
		}
	}
}



/*
	Fill in data and index streams based on the requirements passed in.
	Associate indexing with the render items passed in.

	Note that we leave both code paths to either draw shared or non-shared active vertices.
	The choice of which to use is up to the circumstances per plug-in.
	When drawing shared vertices, this requires an additional position buffer to be
	created so will use more memory. If drawing unshared vertices redundent extra
	vertices are drawn but will use less memory. The data member fDrawSharedActiveVertices
	can be set to decide on which implementation to use.
*/
void apiMeshGeometryOverride::populateGeometry(
	const MHWRender::MGeometryRequirements& requirements,
    const MHWRender::MRenderItemList& renderItems,
	MHWRender::MGeometry& data)
{
	static bool debugPopulateGeometry = false;
	if (debugPopulateGeometry)
		printf("> Begin populate geometry\n");

	// Get the active vertex count
	unsigned int activeVertexCount = fActiveVertices.length();

	// Compute the number of triangles, assume polys are always convex
	unsigned int numTriangles = 0;
	unsigned int totalVerts = 0;
	for (int i=0; i<fMeshGeom->faceCount; i++)
	{
		int numVerts = fMeshGeom->face_counts[i];
		if (numVerts > 2)
		{
			numTriangles += numVerts - 2;
			totalVerts += numVerts;
		}
	}

	/////////////////////////////////////////////////////////////////////
	// Update data streams based on geometry requirements
	/////////////////////////////////////////////////////////////////////
	updateGeometryRequirements(requirements, data, 	activeVertexCount, totalVerts,
		debugPopulateGeometry);

	/////////////////////////////////////////////////////////////////////
	// Update indexing data for all appropriate render items
	/////////////////////////////////////////////////////////////////////
	MHWRender::MIndexBuffer* wireIndexBuffer = NULL; // reuse same index buffer for both wireframe and selected

	int numItems = renderItems.length();
	for (int i=0; i<numItems; i++)
	{
        const MHWRender::MRenderItem* item = renderItems.itemAt(i);
		if (!item) continue;

		// Enable to debug vertex buffers that are associated with each render item.
		// Can also use to generate indexing better, but we don't need that here.
		// Also debugs custom data on the render item.
		static const bool debugStuff = false;
		if (debugStuff)
		{
			const MHWRender::MVertexBufferDescriptorList& itemBuffers =
				item->requiredVertexBuffers();
			int numBufs = itemBuffers.length();
			MHWRender::MVertexBufferDescriptor desc;
			for (int bufNum=0; bufNum<numBufs; bufNum++)
			{
				if (itemBuffers.getDescriptor(bufNum, desc))
				{
					printf("Buffer Required for Item #%d ('%s'):\n", i, item->name().asChar());
					printf("\tBufferName: %s\n", desc.name().asChar());
					printf("\tDataType: %s (dimension %d)\n", MHWRender::MGeometry::dataTypeString(desc.dataType()).asChar(), desc.dimension());
					printf("\tSemantic: %s\n", MHWRender::MGeometry::semanticString(desc.semantic()).asChar());
					printf("\n");
				}
			}

			// Just print a message for illustration purposes. Note that the custom data is also
			// accessible from the MRenderItem in MPxShaderOverride::draw().
			apiMeshUserData* myCustomData = dynamic_cast<apiMeshUserData*>(item->customData());
			if (myCustomData)
			{
				printf("Custom data on Item #%d: '%s', modified count='%d'\n\n", i, myCustomData->fMessage.asChar(), myCustomData->fNumModifications);
			}
			else
			{
				printf("No custom data on Item #%d\n\n", i);
			}
		}

		// Update indexing for active vertex item
		//
		if (item->name() == sActiveVertexItemName)
		{
			updateIndexingForVertices( item, data, numTriangles, activeVertexCount, debugPopulateGeometry);
		}


		// Update indexing for face center item in wireframe mode and shaded mode
		//
		if ((item->name() == sShadedModeFaceCenterItemName || item->name() == sWireframeModeFaceCenterItemName) && fDrawFaceCenters)
		{
			updateIndexingForFaceCenters( item, data, debugPopulateGeometry);
		}

		// Create indexing for dormant and numeric vertex render items
		//
		else if (item->name() == sVertexItemName ||
				 item->name() == sVertexIdItemName ||
				 item->name() == sVertexPositionItemName)
		{
			updateIndexingForDormantVertices( item, data, numTriangles );
		}

		// Create indexing for wireframe render items
		//
		else if (item->name() == sWireframeItemName
				 || item->name() == sShadedTemplateItemName
				 || item->name() == sSelectedWireframeItemName
				 || (item->primitive() != MHWRender::MGeometry::kTriangles
					 && item->name() == sShadedProxyItemName))
		{
			updateIndexingForWireframeItems(wireIndexBuffer, item, data, totalVerts);
		}

		// Handle indexing for affected edge render items
		// For each face we check the edges. If the edges are in the active vertex
		// list we add indexing for the 2 vertices on the edge to the index buffer.
		//
		else if (item->name() == sAffectedEdgeItemName)
		{
			updateIndexingForEdges(item, data, totalVerts, true /*fromSelection*/); // Filter edges using active edges or actives vertices set
		}
		else if (item->name() == sEdgeSelectionItemName)
		{
			updateIndexingForEdges(item, data, totalVerts, false /*fromSelection*/); // No filter : all edges
		}

		// Handle indexing for affected edge render items
		// For each triangle we check the vertices. If any of the vertices are in the active vertex
		// list we add indexing for the triangle to the index buffer.
		//
		else if (item->name() == sAffectedFaceItemName)
		{
			updateIndexingForFaces(item, data, numTriangles, true /*fromSelection*/); // Filter faces using active faces or actives vertices set
		}
		else if (item->name() == sFaceSelectionItemName)
		{
			updateIndexingForFaces(item, data, numTriangles, false /*fromSelection*/); // No filter : all faces
		}

		// Create indexing for filled (shaded) render items
		//
		else if (item->primitive() == MHWRender::MGeometry::kTriangles)
		{
				updateIndexingForShadedTriangles(item, data, numTriangles);
		}
	}

	if (debugPopulateGeometry)
		printf("> End populate geometry\n");
}

void apiMeshGeometryOverride::cleanUp()
{
	fMeshGeom = NULL;
	fActiveVertices.clear();
	fActiveVerticesSet.clear();
	fActiveEdgesSet.clear();
	fActiveFacesSet.clear();
}

/*
	This is method is called during the pre-filtering phase of the viewport 2.0 selection
   	and is used to setup the selection context of the given DAG object.

	We want the whole shape to be selectable, so we set the selection level to kObject so that the shape
	will be processed by the selection.

	In case we are currently in component selection mode (vertex, edge or face),
	since we have created render items that can be use in the selection phase (kSelectionOnly draw mode)
	and we also registered component converters to handle this render items,
	we can set the selection level to kComponent so that the shape will also be processed by the selection.
*/
void apiMeshGeometryOverride::updateSelectionGranularity(
	const MDagPath& path,
	MHWRender::MSelectionContext& selectionContext)
{
	MHWRender::DisplayStatus displayStatus = MHWRender::MGeometryUtilities::displayStatus(path);
	if(displayStatus == MHWRender::kHilite)
	{
		MSelectionMask globalComponentMask = MGlobal::selectionMode() == MGlobal::kSelectComponentMode ? MGlobal::componentSelectionMask() : MGlobal::objectSelectionMask();
		MSelectionMask supportedComponents(MSelectionMask::kSelectMeshVerts);
		supportedComponents.addMask(MSelectionMask::kSelectMeshEdges);
		supportedComponents.addMask(MSelectionMask::kSelectMeshFaces);
		supportedComponents.addMask(MSelectionMask::kSelectPointsForGravity);

		if(globalComponentMask.intersects(supportedComponents))
		{
			selectionContext.setSelectionLevel(MHWRender::MSelectionContext::kComponent);
		}
	}
	else if (pointSnappingActive())
	{
		selectionContext.setSelectionLevel(MHWRender::MSelectionContext::kComponent);
	}
}

/*
   Register our component converters to the draw registry
   This should be done only once, when the plugin is initialized
*/
MStatus apiMeshGeometryOverride::registerComponentConverters()
{
	MStatus status = MHWRender::MDrawRegistry::registerComponentConverter(apiMeshGeometryOverride::sVertexItemName, meshVertComponentConverter::creator);
	if(status) {
		status = MHWRender::MDrawRegistry::registerComponentConverter(apiMeshGeometryOverride::sEdgeSelectionItemName, meshEdgeComponentConverter::creator);
		if(status) {
			MHWRender::MDrawRegistry::registerComponentConverter(apiMeshGeometryOverride::sFaceSelectionItemName, meshFaceComponentConverter::creator);
		}
	}
	return status;
}

/*
   Deregister our component converters from the draw registry
   This should be done only once, when the plugin is uninitialized
*/
MStatus apiMeshGeometryOverride::deregisterComponentConverters()
{
	MStatus status = MHWRender::MDrawRegistry::deregisterComponentConverter(apiMeshGeometryOverride::sVertexItemName);
	if(status) {
		status = MHWRender::MDrawRegistry::deregisterComponentConverter(apiMeshGeometryOverride::sEdgeSelectionItemName);
		if(status) {
			status = MHWRender::MDrawRegistry::deregisterComponentConverter(apiMeshGeometryOverride::sFaceSelectionItemName);
		}
	}
	return status;
}


