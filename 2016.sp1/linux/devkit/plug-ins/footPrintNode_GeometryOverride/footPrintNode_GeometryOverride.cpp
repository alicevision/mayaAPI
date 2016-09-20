//-
// ==========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
// ==========================================================================
//+

////////////////////////////////////////////////////////////////////////
// 
// footPrintNode.cpp
// 
// This plug-in demonstrates how to draw a simple mesh like foot Print in an efficient way.
//
// This efficient way is supported in Viewport 2.0.
//
// For comparison, you can also reference a devkit sample named footPrintNode.
// In that sample, we draw the footPrint using the MUIDrawManager primitives in footPrintDrawOverride::addUIDrawables()
//
// For comparison, you can also reference another devkit sample named rawfootPrintNode.
// In that sample, we draw the footPrint with OpenGL\DX in method rawFootPrintDrawOverride::draw().
//
// Note the method 
//   footPrint::draw() 
// is only called in legacy default viewport to draw foot Print.
// while the methods 
//   FootPrintGeometryOverride::updateDG() 
//   FootPrintGeometryOverride::updateRenderItems() 
//   FootPrintGeometryOverride::populateGeometry()
// are only called in Viewport 2.0 to prepare and draw foot Print.
// 
////////////////////////////////////////////////////////////////////////

#include <maya/MPxLocatorNode.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MVector.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MColor.h>
#include <maya/M3dView.h>
#include <maya/MFnPlugin.h>
#include <maya/MDistance.h>
#include <maya/MFnUnitAttribute.h>

// Viewport 2.0 includes
#include <maya/MDrawRegistry.h>
#include <maya/MPxGeometryOverride.h>
#include <maya/MUserData.h>
#include <maya/MDrawContext.h>
#include <maya/MShaderManager.h>
#include <maya/MHWGeometry.h>
#include <maya/MHWGeometryUtilities.h>
#include <maya/MPointArray.h>

#include <assert.h>

// Foot Data
//
static float sole[][3] = { {  0.00f, 0.0f, -0.70f },
						   {  0.04f, 0.0f, -0.69f },
						   {  0.09f, 0.0f, -0.65f },
						   {  0.13f, 0.0f, -0.61f },
						   {  0.16f, 0.0f, -0.54f },
						   {  0.17f, 0.0f, -0.46f },
						   {  0.17f, 0.0f, -0.35f },
						   {  0.16f, 0.0f, -0.25f },
						   {  0.15f, 0.0f, -0.14f },
						   {  0.13f, 0.0f,  0.00f },
						   {  0.00f, 0.0f,  0.00f },
						   { -0.13f, 0.0f,  0.00f },
						   { -0.15f, 0.0f, -0.14f },
						   { -0.16f, 0.0f, -0.25f },
						   { -0.17f, 0.0f, -0.35f },
						   { -0.17f, 0.0f, -0.46f },
						   { -0.16f, 0.0f, -0.54f },
						   { -0.13f, 0.0f, -0.61f },
						   { -0.09f, 0.0f, -0.65f },
						   { -0.04f, 0.0f, -0.69f },
						   { -0.00f, 0.0f, -0.70f } };
static float heel[][3] = { {  0.00f, 0.0f,  0.06f },
						   {  0.13f, 0.0f,  0.06f },
						   {  0.14f, 0.0f,  0.15f },
						   {  0.14f, 0.0f,  0.21f },
						   {  0.13f, 0.0f,  0.25f },
						   {  0.11f, 0.0f,  0.28f },
						   {  0.09f, 0.0f,  0.29f },
						   {  0.04f, 0.0f,  0.30f },
						   {  0.00f, 0.0f,  0.30f },
						   { -0.04f, 0.0f,  0.30f },
						   { -0.09f, 0.0f,  0.29f },
						   { -0.11f, 0.0f,  0.28f },
						   { -0.13f, 0.0f,  0.25f },
						   { -0.14f, 0.0f,  0.21f },
						   { -0.14f, 0.0f,  0.15f },
						   { -0.13f, 0.0f,  0.06f },
						   { -0.00f, 0.0f,  0.06f } };
static int soleCount = 21;
static int heelCount = 17;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Node implementation with standard viewport draw
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

class footPrint : public MPxLocatorNode
{
public:
	footPrint();
	virtual ~footPrint();

    virtual MStatus   		compute( const MPlug& plug, MDataBlock& data );

	virtual void            draw( M3dView & view, const MDagPath & path,
								  M3dView::DisplayStyle style,
								  M3dView::DisplayStatus status );

	virtual bool            isBounded() const;
	virtual MBoundingBox    boundingBox() const;

	static  void *          creator();
	static  MStatus         initialize();

	static  MObject         size;         // The size of the foot

public:
	static	MTypeId		id;
	static	MString		drawDbClassification;
	static	MString		drawRegistrantId;
};

MObject footPrint::size;
MTypeId footPrint::id( 0x80007 );
MString	footPrint::drawDbClassification("drawdb/geometry/footPrint");
MString	footPrint::drawRegistrantId("FootprintNodePlugin");

footPrint::footPrint() {}
footPrint::~footPrint() {}

MStatus footPrint::compute( const MPlug& /*plug*/, MDataBlock& /*data*/ )
{
	return MS::kUnknownParameter;
}

// called by legacy default viewport
void footPrint::draw( M3dView & view, const MDagPath & /*path*/,
							 M3dView::DisplayStyle style,
							 M3dView::DisplayStatus status )
{
	// Get the size
	//
	MObject thisNode = thisMObject();
	MPlug plug( thisNode, size );
	MDistance sizeVal;
	plug.getValue( sizeVal );

	float multiplier = (float) sizeVal.asCentimeters();

	view.beginGL();


	if ( ( style == M3dView::kFlatShaded ) ||
		 ( style == M3dView::kGouraudShaded ) )
	{ 
		// Push the color settings
		//
		glPushAttrib( GL_CURRENT_BIT );

		if ( status == M3dView::kActive ) {
			view.setDrawColor( 13, M3dView::kActiveColors );
		} else {
			view.setDrawColor( 13, M3dView::kDormantColors );
		}

		glBegin( GL_TRIANGLE_FAN );
	        int i;
			int last = soleCount - 1;
			for ( i = 0; i < last; ++i ) {
				glVertex3f( sole[i][0] * multiplier,
							sole[i][1] * multiplier,
							sole[i][2] * multiplier );
			}
		glEnd();
		glBegin( GL_TRIANGLE_FAN );
			last = heelCount - 1;
			for ( i = 0; i < last; ++i ) {
				glVertex3f( heel[i][0] * multiplier,
							heel[i][1] * multiplier,
							heel[i][2] * multiplier );
			}
		glEnd();

		glPopAttrib();
	}

	// Draw the outline of the foot
	//
	glBegin( GL_LINES );
	    int i;
	    int last = soleCount - 1;
	    for ( i = 0; i < last; ++i ) {
			glVertex3f( sole[i][0] * multiplier,
						sole[i][1] * multiplier,
						sole[i][2] * multiplier );
			glVertex3f( sole[i+1][0] * multiplier,
						sole[i+1][1] * multiplier,
						sole[i+1][2] * multiplier );
		}
		last = heelCount - 1;
	    for ( i = 0; i < last; ++i ) {
			glVertex3f( heel[i][0] * multiplier,
						heel[i][1] * multiplier,
						heel[i][2] * multiplier );
			glVertex3f( heel[i+1][0] * multiplier,
						heel[i+1][1] * multiplier,
						heel[i+1][2] * multiplier );
		}
	glEnd();


	view.endGL();
}

bool footPrint::isBounded() const
{
	return true;
}

MBoundingBox footPrint::boundingBox() const
{
	// Get the size
	//
	MObject thisNode = thisMObject();
	MPlug plug( thisNode, size );
	MDistance sizeVal;
	plug.getValue( sizeVal );

	double multiplier = sizeVal.asCentimeters();

	MPoint corner1( -0.17, 0.0, -0.7 );
	MPoint corner2( 0.17, 0.0, 0.3 );

	corner1 = corner1 * multiplier;
	corner2 = corner2 * multiplier;

	return MBoundingBox( corner1, corner2 );
}

void* footPrint::creator()
{
	return new footPrint();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Viewport 2.0 override implementation
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

class FootPrintGeometryOverride : public MHWRender::MPxGeometryOverride
{
public:
	static MHWRender::MPxGeometryOverride* Creator(const MObject& obj)
	{
		return new FootPrintGeometryOverride(obj);
	}

	virtual ~FootPrintGeometryOverride();

	virtual MHWRender::DrawAPI supportedDrawAPIs() const;

	virtual bool hasUIDrawables() const { return false; }

	virtual void updateDG();
	virtual bool isIndexingDirty(const MHWRender::MRenderItem &item) { return false; }
	virtual bool isStreamDirty(const MHWRender::MVertexBufferDescriptor &desc) { return mMultiplierChanged; }
	virtual void updateRenderItems(const MDagPath &path, MHWRender::MRenderItemList& list);
	virtual void populateGeometry(const MHWRender::MGeometryRequirements &requirements, const MHWRender::MRenderItemList &renderItems, MHWRender::MGeometry &data);
	virtual void cleanUp() {};


private:
	FootPrintGeometryOverride(const MObject& obj);

	MHWRender::MShaderInstance* mSolidUIShader;
	MObject mLocatorNode;
	float mMultiplier;
	bool mMultiplierChanged;
};

FootPrintGeometryOverride::FootPrintGeometryOverride(const MObject& obj)
: MHWRender::MPxGeometryOverride(obj)
, mSolidUIShader(NULL)
, mLocatorNode(obj)
, mMultiplier(0.0f)
, mMultiplierChanged(true)
{
    MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
    if (!renderer)
        return;

    const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
    if (!shaderMgr)
        return;

	mSolidUIShader = shaderMgr->getStockShader(MHWRender::MShaderManager::k3dSolidShader);
}

FootPrintGeometryOverride::~FootPrintGeometryOverride()
{
	if(mSolidUIShader)
	{
		MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
		if (renderer)
		{
    		const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
    		if (shaderMgr)
    		{
				shaderMgr->releaseShader(mSolidUIShader);
			}
		}
		mSolidUIShader = NULL;
    }
}

MHWRender::DrawAPI FootPrintGeometryOverride::supportedDrawAPIs() const
{
	// this plugin supports both GL and DX
	return (MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile);
}

const MString colorParameterName_   = "solidColor";
const MString wireframeHeelItemName_    = "heelLocatorWires";
const MString wireframeSoleItemName_    = "soleLocatorWires";
const MString shadedHeelItemName_       = "heelLocatorTriangles";
const MString shadedSoleItemName_       = "soleLocatorTriangles";

void FootPrintGeometryOverride::updateDG()
{
	MPlug plug(mLocatorNode, footPrint::size);
	float newScale = 1.0f;
	if (!plug.isNull())
	{
		MDistance sizeVal;
		if (plug.getValue(sizeVal))
		{
			newScale = (float)sizeVal.asCentimeters();
		}
	}

	if (newScale != mMultiplier)
	{
		mMultiplier = newScale;
		mMultiplierChanged = true;
	}
}

void FootPrintGeometryOverride::updateRenderItems( const MDagPath& path, MHWRender::MRenderItemList& list )
{
    MHWRender::MRenderItem* wireframeHeel = NULL;

    int index = list.indexOf(wireframeHeelItemName_);

    if (index < 0)
    {
        wireframeHeel = MHWRender::MRenderItem::Create(
            wireframeHeelItemName_,
            MHWRender::MRenderItem::DecorationItem,
            MHWRender::MGeometry::kLineStrip);
        wireframeHeel->setDrawMode(MHWRender::MGeometry::kWireframe);
        wireframeHeel->depthPriority(5);

        list.append(wireframeHeel);
    }
    else
    {
        wireframeHeel = list.itemAt(index);
    }

    if(wireframeHeel)
    {
		if (mSolidUIShader)
		{
			MColor color = MHWRender::MGeometryUtilities::wireframeColor(path);
			float  wireframeColor[4] = { color.r, color.g, color.b, 1.0f };
			mSolidUIShader->setParameter(colorParameterName_, wireframeColor);
			wireframeHeel->setShader(mSolidUIShader);
		}
		wireframeHeel->enable(true);
    }

    MHWRender::MRenderItem* wireframeSole = NULL;

    index = list.indexOf(wireframeSoleItemName_);

    if (index < 0)
    {
        wireframeSole = MHWRender::MRenderItem::Create(
            wireframeSoleItemName_,
            MHWRender::MRenderItem::DecorationItem,
            MHWRender::MGeometry::kLineStrip);
        wireframeSole->setDrawMode(MHWRender::MGeometry::kWireframe);
        wireframeSole->depthPriority(5);

        list.append(wireframeSole);
    }
    else
    {
        wireframeSole = list.itemAt(index);
    }

    if(wireframeSole)
    {
		if (mSolidUIShader)
		{
			MColor color = MHWRender::MGeometryUtilities::wireframeColor(path);
			float  wireframeColor[4] = { color.r, color.g, color.b, 1.0f };
			mSolidUIShader->setParameter(colorParameterName_, wireframeColor);
			wireframeSole->setShader(mSolidUIShader);
		}
		wireframeSole->enable(true);
    }

    MHWRender::MRenderItem* shadedHeelItem = NULL;

    index = list.indexOf(shadedHeelItemName_);

    if (index < 0)
    {
        shadedHeelItem = MHWRender::MRenderItem::Create(
            shadedHeelItemName_,
            MHWRender::MRenderItem::DecorationItem,
            MHWRender::MGeometry::kTriangleStrip);
        shadedHeelItem->setDrawMode(MHWRender::MGeometry::kShaded);
        shadedHeelItem->depthPriority(5);

        list.append(shadedHeelItem);
    }
    else
    {
        shadedHeelItem = list.itemAt(index);
    }

    if(shadedHeelItem)
    {
		if (mSolidUIShader)
		{
			MColor color = MHWRender::MGeometryUtilities::wireframeColor(path);
			float  wireframeColor[4] = { color.r, color.g, color.b, 1.0f };
			mSolidUIShader->setParameter(colorParameterName_, wireframeColor);
			shadedHeelItem->setShader(mSolidUIShader);
		}
		shadedHeelItem->enable(true);
    }

    MHWRender::MRenderItem* shadedSoleItem = NULL;

    index = list.indexOf(shadedSoleItemName_);

    if (index < 0)
    {
        shadedSoleItem = MHWRender::MRenderItem::Create(
            shadedSoleItemName_,
            MHWRender::MRenderItem::DecorationItem,
            MHWRender::MGeometry::kTriangleStrip);
        shadedSoleItem->setDrawMode(MHWRender::MGeometry::kShaded);
        shadedSoleItem->depthPriority(5);

        list.append(shadedSoleItem);
    }
    else
    {
        shadedSoleItem = list.itemAt(index);
    }

    if(shadedSoleItem)
    {
		if (mSolidUIShader)
		{
			MColor color = MHWRender::MGeometryUtilities::wireframeColor(path);
			float  wireframeColor[4] = { color.r, color.g, color.b, 1.0f };
			mSolidUIShader->setParameter(colorParameterName_, wireframeColor);
			shadedSoleItem->setShader(mSolidUIShader);
		}
		shadedSoleItem->enable(true);
    }

}

void FootPrintGeometryOverride::populateGeometry(
    const MHWRender::MGeometryRequirements& requirements,
    const MHWRender::MRenderItemList& renderItems,
    MHWRender::MGeometry& data)
{
    MHWRender::MVertexBuffer* verticesBuffer       = NULL;

    float* vertices                     = NULL;

	const MHWRender::MVertexBufferDescriptorList& vertexBufferDescriptorList = requirements.vertexRequirements();
	const int numberOfVertexRequirments = vertexBufferDescriptorList.length();

	MHWRender::MVertexBufferDescriptor vertexBufferDescriptor;
	for (int requirmentNumber = 0; requirmentNumber < numberOfVertexRequirments; ++requirmentNumber)
	{
		if (!vertexBufferDescriptorList.getDescriptor(requirmentNumber, vertexBufferDescriptor))
		{
			continue;
		}

		switch (vertexBufferDescriptor.semantic())
		{
		case MHWRender::MGeometry::kPosition:
			{
				if (!verticesBuffer)
				{
					verticesBuffer = data.createVertexBuffer(vertexBufferDescriptor);
					if (verticesBuffer)
					{
						vertices = (float*)verticesBuffer->acquire(soleCount+heelCount);
					}
				}
			}
			break;
		default:
			// do nothing for stuff we don't understand
			break;
		}
	}

	int verticesPointerOffset = 0;

	// We concatenate the heel and sole positions into a single vertex buffer.
	// The index buffers will decide which positions will be selected for each render items.
	for (int currentVertex = 0 ; currentVertex < soleCount+heelCount; ++currentVertex)
	{
		if(vertices)
		{
			if (currentVertex < heelCount)
			{
				int heelVtx = currentVertex;
				vertices[verticesPointerOffset++] = heel[heelVtx][0] * mMultiplier;
				vertices[verticesPointerOffset++] = heel[heelVtx][1] * mMultiplier;
				vertices[verticesPointerOffset++] = heel[heelVtx][2] * mMultiplier;
			}
			else
			{
				int soleVtx = currentVertex - heelCount;
				vertices[verticesPointerOffset++] = sole[soleVtx][0] * mMultiplier;
				vertices[verticesPointerOffset++] = sole[soleVtx][1] * mMultiplier;
				vertices[verticesPointerOffset++] = sole[soleVtx][2] * mMultiplier;
			}
		}
	}

	if(verticesBuffer && vertices)
	{
		verticesBuffer->commit(vertices);
	}

	for (int i=0; i < renderItems.length(); ++i)
	{
		const MHWRender::MRenderItem* item = renderItems.itemAt(i);

		if (!item)
		{
			continue;
		}

		// Start position in the index buffer (start of heel or sole positions):
		int startIndex = 0;
		int endIndex = 0;
		// Number of index to generate (for line strip, or triangle list)
		int numIndex = 0;
		bool isWireFrame = true;

		if (item->name() == wireframeHeelItemName_ )
		{
			numIndex = heelCount;
		}
		else if (item->name() == wireframeSoleItemName_ )
		{
			startIndex = heelCount;
			numIndex = soleCount;
		}
		else if (item->name() == shadedHeelItemName_ )
		{
			numIndex = heelCount - 2;
			startIndex = 1;
			endIndex = heelCount - 2;
			isWireFrame = false;
		}
		else if (item->name() == shadedSoleItemName_ )
		{
			startIndex = heelCount;
			endIndex = heelCount + soleCount - 2;
			numIndex = soleCount - 2;
			isWireFrame = false;
		}

		if (numIndex)
		{
		    MHWRender::MIndexBuffer*  indexBuffer = data.createIndexBuffer(MHWRender::MGeometry::kUnsignedInt32);
		    unsigned int* indices = (unsigned int*)indexBuffer->acquire(numIndex);

			for(int i = 0; i < numIndex; )
			{
				if (isWireFrame)
				{
					// Line strip starts at first position and iterates thru all vertices:
					indices[i] = startIndex + i;
					i++;
				}
				else
				{
					// Triangle strip zig-zagging thru the points:
					indices[i] = startIndex + i/2;
					if (i+1 < numIndex)
						indices[i+1] = endIndex - i/2;
					i += 2;
				}
			}

			indexBuffer->commit(indices);
			item->associateWithIndexBuffer(indexBuffer);
		}
	}
	mMultiplierChanged = false;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Plugin Registration
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

MStatus footPrint::initialize()
{
	MFnUnitAttribute unitFn;
	MStatus			 stat;

	size = unitFn.create( "size", "sz", MFnUnitAttribute::kDistance );
	unitFn.setDefault( 1.0 );

	stat = addAttribute( size );
	if (!stat) {
		stat.perror("addAttribute");
		return stat;
	}

	return MS::kSuccess;
}

MStatus initializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");

	status = plugin.registerNode(
				"footPrint",
				footPrint::id,
				&footPrint::creator,
				&footPrint::initialize,
				MPxNode::kLocatorNode,
				&footPrint::drawDbClassification);
	if (!status) {
		status.perror("registerNode");
		return status;
	}

    status = MHWRender::MDrawRegistry::registerGeometryOverrideCreator(
		footPrint::drawDbClassification,
		footPrint::drawRegistrantId,
		FootPrintGeometryOverride::Creator);
	if (!status) {
		status.perror("registerDrawOverrideCreator");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = MHWRender::MDrawRegistry::deregisterGeometryOverrideCreator(
		footPrint::drawDbClassification,
		footPrint::drawRegistrantId);
	if (!status) {
		status.perror("deregisterDrawOverrideCreator");
		return status;
	}

	status = plugin.deregisterNode( footPrint::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}
	return status;
}
