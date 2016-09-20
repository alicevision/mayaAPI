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
// squaresNode_noDepthTest.cpp
// 
// This plug-in demonstrates how to draw a simple mesh without Depth Test in an easy way.
//
// This easy way is supported in Viewport 2.0.
// In Viewport 2.0, MUIDrawManager can be used to draw simple UI elements without Depth Test.
//
// Note the method 
//   "squares::draw()"
// is only called in legacy default viewport to draw squares,
// while the methods 
//   SquaresDrawOverride::prepareForDraw() 
//   SquaresDrawOverride::addUIDrawables() 
// are only called in Viewport 2.0 to prepare and draw squares.
// 
////////////////////////////////////////////////////////////////////////

#include <maya/MPxLocatorNode.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MColor.h>
#include <maya/M3dView.h>
#include <maya/MFnPlugin.h>
#include <maya/MDistance.h>
#include <maya/MFnUnitAttribute.h>

// Viewport 2.0 includes
#include <maya/MDrawRegistry.h>
#include <maya/MPxDrawOverride.h>
#include <maya/MUserData.h>
#include <maya/MDrawContext.h>
#include <maya/MHWGeometryUtilities.h>
#include <maya/MPointArray.h>

#include <assert.h>

// Squares Data
//
static float topSquare[][3] = { 
	{ -1.00f, 1.00f, -1.00f },
	{ -1.00f, 1.00f,  1.00f },
	{  1.00f, 1.00f, -1.00f },
	{  1.00f, 1.00f,  1.00f } };
static float bottomSquare[][3] = { 
	{ -1.00f, 0.00f, -1.00f },
	{ -1.00f, 0.00f,  1.00f },
	{  1.00f, 0.00f, -1.00f },
	{  1.00f, 0.00f,  1.00f } };
static int squareCount = 4;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Node implementation with standard viewport draw
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

class squares : public MPxLocatorNode
{
public:
	squares();
	virtual ~squares();

    virtual MStatus   		compute( const MPlug& plug, MDataBlock& data );

	virtual void            draw( M3dView & view, const MDagPath & path,
								  M3dView::DisplayStyle style,
								  M3dView::DisplayStatus status );

	virtual bool            isBounded() const;
	virtual MBoundingBox    boundingBox() const;

	static  void *          creator();
	static  MStatus         initialize();

	static  MObject         size;         // The size of the Square

public:
	static	MTypeId		id;
	static	MString		drawDbClassification;
	static	MString		drawRegistrantId;
};

MObject squares::size;
MTypeId squares::id( 0x00080032 );
MString	squares::drawDbClassification("drawdb/geometry/squares");
MString	squares::drawRegistrantId("SquaresNodePlugin");

squares::squares() {}
squares::~squares() {}

MStatus squares::compute( const MPlug& /*plug*/, MDataBlock& /*data*/ )
{
	return MS::kUnknownParameter;
}

// called by legacy default viewport
void squares::draw( M3dView & view, const MDagPath & /*path*/,
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

	MColor topColor, bottomColor;
	if ( status == M3dView::kActive || status == M3dView::kLead )
	{
		topColor = MColor( 1.0f, 0.0f, 0.0f, 1.0f );
		bottomColor = MColor( 1.0f, 1.0f, 0.0f, 1.0f );
	}
	else
	{
		topColor = MColor( 0.8f, 0.0f, 0.5f, 1.0f );
		bottomColor = MColor( 0.8f, 0.2f, 0.0f, 1.0f );
	}

	view.beginGL();

	glPushAttrib( GL_DEPTH_BUFFER_BIT );
	glDisable( GL_DEPTH_TEST ); 

	glBegin( GL_LINE_LOOP );
	glVertex3f( bottomSquare[0][0] * multiplier, bottomSquare[0][1] * multiplier, bottomSquare[0][2] * multiplier );
	glVertex3f( bottomSquare[1][0] * multiplier, bottomSquare[1][1] * multiplier, bottomSquare[1][2] * multiplier );
	glVertex3f( bottomSquare[3][0] * multiplier, bottomSquare[3][1] * multiplier, bottomSquare[3][2] * multiplier );
	glVertex3f( bottomSquare[2][0] * multiplier, bottomSquare[2][1] * multiplier, bottomSquare[2][2] * multiplier );
	glEnd();
	glBegin( GL_LINE_LOOP );
	glVertex3f( topSquare[0][0] * multiplier, topSquare[0][1] * multiplier, topSquare[0][2] * multiplier );
	glVertex3f( topSquare[1][0] * multiplier, topSquare[1][1] * multiplier, topSquare[1][2] * multiplier );
	glVertex3f( topSquare[3][0] * multiplier, topSquare[3][1] * multiplier, topSquare[3][2] * multiplier );
	glVertex3f( topSquare[2][0] * multiplier, topSquare[2][1] * multiplier, topSquare[2][2] * multiplier );
	glEnd();

	// Draw solid
	if ( ( style == M3dView::kFlatShaded ) || ( style == M3dView::kGouraudShaded ) )
	{ 
		glPushAttrib( GL_CURRENT_BIT );
		glColor4f( bottomColor.r, bottomColor.g, bottomColor.b, bottomColor.a );
		glBegin( GL_TRIANGLE_STRIP );
		for ( int i = 0; i < squareCount; ++i ) 
		{
			glVertex3f( bottomSquare[i][0] * multiplier, bottomSquare[i][1] * multiplier, bottomSquare[i][2] * multiplier );
		}
		glEnd();
		glColor4f( topColor.r, topColor.g, topColor.b, topColor.a );
		glBegin( GL_TRIANGLE_STRIP );
		for ( int i = 0; i < squareCount; ++i ) 
		{
			glVertex3f( topSquare[i][0] * multiplier, topSquare[i][1] * multiplier, topSquare[i][2] * multiplier );
		}
		glEnd();
		glPopAttrib();
	}

	glPopAttrib();

	view.endGL();

	// Draw the name of the squares
	view.setDrawColor( MColor( 0.1f, 0.8f, 0.8f, 1.0f ) );
	view.drawText( MString("Squares without Depth Test"), MPoint( 0.0, 0.0, 0.0 ), M3dView::kCenter );
}

bool squares::isBounded() const
{
	return true;
}

MBoundingBox squares::boundingBox() const
{
	// Get the size
	//
	MObject thisNode = thisMObject();
	MPlug plug( thisNode, size );
	MDistance sizeVal;
	plug.getValue( sizeVal );

	double multiplier = sizeVal.asCentimeters();

	MPoint corner1( -1.0, 1.0, -1.0 );
	MPoint corner2( 1.0, 0.0, 1.0 );

	corner1 = corner1 * multiplier;
	corner2 = corner2 * multiplier;

	return MBoundingBox( corner1, corner2 );
}

void* squares::creator()
{
	return new squares();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Viewport 2.0 override implementation
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

class SquareData : public MUserData
{
public:
	SquareData() : MUserData(false) {} // don't delete after draw
	virtual ~SquareData() {}

	MColor fTopSquareColor;
	MColor fBottomSquareColor;
	MColor fWireFrameColor;

	MPointArray fTopSquareList;
	MPointArray fBottomSquareList;

	MPointArray fTopLineList;
	MPointArray fBottomLineList;
};

class SquaresDrawOverride : public MHWRender::MPxDrawOverride
{
public:
	static MHWRender::MPxDrawOverride* Creator(const MObject& obj)
	{
		return new SquaresDrawOverride(obj);
	}

	virtual ~SquaresDrawOverride();

	virtual MHWRender::DrawAPI supportedDrawAPIs() const;

	virtual bool isBounded(
		const MDagPath& objPath,
		const MDagPath& cameraPath) const;

	virtual MBoundingBox boundingBox(
		const MDagPath& objPath,
		const MDagPath& cameraPath) const;

	virtual MUserData* prepareForDraw(
		const MDagPath& objPath,
		const MDagPath& cameraPath,
		const MHWRender::MFrameContext& frameContext,
		MUserData* oldData);

	virtual bool hasUIDrawables() const { return true; }

	virtual void addUIDrawables(
		const MDagPath& objPath,
		MHWRender::MUIDrawManager& drawManager,
		const MHWRender::MFrameContext& frameContext,
		const MUserData* data);

	static void draw(const MHWRender::MDrawContext& context, const MUserData* data) {};

private:
	SquaresDrawOverride(const MObject& obj);
	float getMultiplier(const MDagPath& objPath) const;
};

SquaresDrawOverride::SquaresDrawOverride(const MObject& obj)
: MHWRender::MPxDrawOverride(obj, SquaresDrawOverride::draw)
{
}

SquaresDrawOverride::~SquaresDrawOverride()
{
}

MHWRender::DrawAPI SquaresDrawOverride::supportedDrawAPIs() const
{
	// this plugin supports both GL and DX
	return (MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile);
}

float SquaresDrawOverride::getMultiplier(const MDagPath& objPath) const
{
	// Retrieve value of the size attribute from the node
	MStatus status;
	MObject SquaresNode = objPath.node(&status);
	if (status)
	{
		MPlug plug(SquaresNode, squares::size);
		if (!plug.isNull())
		{
			MDistance sizeVal;
			if (plug.getValue(sizeVal))
			{
				return (float)sizeVal.asCentimeters();
			}
		}
	}

	return 1.0f;
}

bool SquaresDrawOverride::isBounded(const MDagPath& /*objPath*/,
                                      const MDagPath& /*cameraPath*/) const
{
	return true;
}

MBoundingBox SquaresDrawOverride::boundingBox(
	const MDagPath& objPath,
	const MDagPath& cameraPath) const
{
	MPoint corner1( -1.0, 1.0, -1.0 );
	MPoint corner2( 1.0, 0.0, 1.0 );

	float multiplier = getMultiplier(objPath);
	corner1 = corner1 * multiplier;
	corner2 = corner2 * multiplier;

	return MBoundingBox( corner1, corner2 );
}

// Called by Maya each time the object needs to be drawn.
MUserData* SquaresDrawOverride::prepareForDraw(
	const MDagPath& objPath,
	const MDagPath& cameraPath,
	const MHWRender::MFrameContext& frameContext,
	MUserData* oldData)
{
	// Any data needed from the Maya dependency graph must be retrieved and cached in this stage.
	// There is one cache data for each drawable instance, if it is not desirable to allow Maya to handle data
	// caching, simply return null in this method and ignore user data parameter in draw callback method.
	// e.g. in this sample, we compute and cache the data for usage later when we create the 
	// MUIDrawManager to draw Squares in method addUIDrawables().
	SquareData* data = dynamic_cast<SquareData*>(oldData);
	if (!data)
	{
		data = new SquareData();
	}
	
	float fMultiplier = getMultiplier(objPath);

	data->fTopSquareList.clear();
	data->fTopSquareList.append(topSquare[0][0] * fMultiplier, topSquare[0][1] * fMultiplier, topSquare[0][2] * fMultiplier);
	data->fTopSquareList.append(topSquare[1][0] * fMultiplier, topSquare[1][1] * fMultiplier, topSquare[1][2] * fMultiplier);
	data->fTopSquareList.append(topSquare[2][0] * fMultiplier, topSquare[2][1] * fMultiplier, topSquare[2][2] * fMultiplier);
	data->fTopSquareList.append(topSquare[2][0] * fMultiplier, topSquare[2][1] * fMultiplier, topSquare[2][2] * fMultiplier);
	data->fTopSquareList.append(topSquare[1][0] * fMultiplier, topSquare[1][1] * fMultiplier, topSquare[1][2] * fMultiplier);
	data->fTopSquareList.append(topSquare[3][0] * fMultiplier, topSquare[3][1] * fMultiplier, topSquare[3][2] * fMultiplier);

	data->fBottomSquareList.clear();
	data->fBottomSquareList.append(bottomSquare[0][0] * fMultiplier, bottomSquare[0][1] * fMultiplier, bottomSquare[0][2] * fMultiplier);
	data->fBottomSquareList.append(bottomSquare[1][0] * fMultiplier, bottomSquare[1][1] * fMultiplier, bottomSquare[1][2] * fMultiplier);
	data->fBottomSquareList.append(bottomSquare[2][0] * fMultiplier, bottomSquare[2][1] * fMultiplier, bottomSquare[2][2] * fMultiplier);
	data->fBottomSquareList.append(bottomSquare[2][0] * fMultiplier, bottomSquare[2][1] * fMultiplier, bottomSquare[2][2] * fMultiplier);
	data->fBottomSquareList.append(bottomSquare[1][0] * fMultiplier, bottomSquare[1][1] * fMultiplier, bottomSquare[1][2] * fMultiplier);
	data->fBottomSquareList.append(bottomSquare[3][0] * fMultiplier, bottomSquare[3][1] * fMultiplier, bottomSquare[3][2] * fMultiplier);

	data->fTopLineList.clear();
	data->fTopLineList.append(topSquare[0][0] * fMultiplier, topSquare[0][1] * fMultiplier, topSquare[0][2] * fMultiplier);
	data->fTopLineList.append(topSquare[1][0] * fMultiplier, topSquare[1][1] * fMultiplier, topSquare[1][2] * fMultiplier);
	data->fTopLineList.append(topSquare[1][0] * fMultiplier, topSquare[1][1] * fMultiplier, topSquare[1][2] * fMultiplier);
	data->fTopLineList.append(topSquare[3][0] * fMultiplier, topSquare[3][1] * fMultiplier, topSquare[3][2] * fMultiplier);
	data->fTopLineList.append(topSquare[3][0] * fMultiplier, topSquare[3][1] * fMultiplier, topSquare[3][2] * fMultiplier);
	data->fTopLineList.append(topSquare[2][0] * fMultiplier, topSquare[2][1] * fMultiplier, topSquare[2][2] * fMultiplier);
	data->fTopLineList.append(topSquare[2][0] * fMultiplier, topSquare[2][1] * fMultiplier, topSquare[2][2] * fMultiplier);
	data->fTopLineList.append(topSquare[0][0] * fMultiplier, topSquare[0][1] * fMultiplier, topSquare[0][2] * fMultiplier);

	data->fBottomLineList.clear();
	data->fBottomLineList.append(bottomSquare[0][0] * fMultiplier, bottomSquare[0][1] * fMultiplier, bottomSquare[0][2] * fMultiplier);
	data->fBottomLineList.append(bottomSquare[1][0] * fMultiplier, bottomSquare[1][1] * fMultiplier, bottomSquare[1][2] * fMultiplier);
	data->fBottomLineList.append(bottomSquare[1][0] * fMultiplier, bottomSquare[1][1] * fMultiplier, bottomSquare[1][2] * fMultiplier);
	data->fBottomLineList.append(bottomSquare[3][0] * fMultiplier, bottomSquare[3][1] * fMultiplier, bottomSquare[3][2] * fMultiplier);
	data->fBottomLineList.append(bottomSquare[3][0] * fMultiplier, bottomSquare[3][1] * fMultiplier, bottomSquare[3][2] * fMultiplier);
	data->fBottomLineList.append(bottomSquare[2][0] * fMultiplier, bottomSquare[2][1] * fMultiplier, bottomSquare[2][2] * fMultiplier);
	data->fBottomLineList.append(bottomSquare[2][0] * fMultiplier, bottomSquare[2][1] * fMultiplier, bottomSquare[2][2] * fMultiplier);
	data->fBottomLineList.append(bottomSquare[0][0] * fMultiplier, bottomSquare[0][1] * fMultiplier, bottomSquare[0][2] * fMultiplier);

	const MHWRender::DisplayStatus displayStatus = MHWRender::MGeometryUtilities::displayStatus(objPath);
	if ( displayStatus == MHWRender::kActive || displayStatus == MHWRender::kLead )
	{
		data->fTopSquareColor = MColor( 1.0f, 0.0f, 0.0f, 1.0f ); // Top is a red One
		data->fBottomSquareColor = MColor( 1.0f, 1.0f, 0.0f, 1.0f );
	}
	else
	{
		data->fTopSquareColor = MColor( 0.8f, 0.0f, 0.5f, 1.0f );
		data->fBottomSquareColor = MColor( 0.8f, 0.2f, 0.0f, 1.0f );
	}

	data->fWireFrameColor = MHWRender::MGeometryUtilities::wireframeColor(objPath);

	return data;
}

// addUIDrawables() provides access to the MUIDrawManager, which can be used
// to queue up operations for drawing simple UI elements such as lines, circles and
// text. To enable addUIDrawables(), override hasUIDrawables() and make it return true.
void SquaresDrawOverride::addUIDrawables(
		const MDagPath& objPath,
		MHWRender::MUIDrawManager& drawManager,
		const MHWRender::MFrameContext& frameContext,
		const MUserData* data)
{
	// Get data cached by prepareForDraw() for each drawable instance, then MUIDrawManager 
	// can draw simple UI by these data.
	SquareData* pLocatorData = (SquareData*)data;
	if (!pLocatorData)
	{
		return;
	}

	drawManager.beginDrawable();

	drawManager.setColor( pLocatorData->fWireFrameColor);
	drawManager.beginDrawInXray();
	drawManager.mesh(MHWRender::MUIDrawManager::kLines, pLocatorData->fBottomLineList);
	drawManager.mesh(MHWRender::MUIDrawManager::kLines, pLocatorData->fTopLineList);
	drawManager.endDrawInXray();

	if (frameContext.getDisplayStyle() & MHWRender::MFrameContext::kGouraudShaded)
	{
		// The drawables to be drawn between calls to beginDrawInXray() and endDrawInXray() would display
		// on the top of other geometries in the scene.
		// Please Refer to API documentation to get more technical details.
		drawManager.beginDrawInXray();
		drawManager.setColor( pLocatorData->fBottomSquareColor );
		drawManager.mesh(MHWRender::MUIDrawManager::kTriangles, pLocatorData->fBottomSquareList);
		drawManager.setColor( pLocatorData->fTopSquareColor );
		drawManager.mesh(MHWRender::MUIDrawManager::kTriangles, pLocatorData->fTopSquareList);
		drawManager.endDrawInXray();
	}

	// Draw a text on top
	MPoint pos( 0.0, 0.0, 0.0 ); // Position of the text
	MColor textColor( 0.1f, 0.8f, 0.8f, 1.0f ); // Text color

	drawManager.setColor( textColor );
	drawManager.setFontSize( MHWRender::MUIDrawManager::kSmallFontSize );
	drawManager.text( pos,  MString("Squares without Depth Test"), MHWRender::MUIDrawManager::kCenter );

	drawManager.endDrawable();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Plugin Registration
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

MStatus squares::initialize()
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
				"squares",
				squares::id,
				&squares::creator,
				&squares::initialize,
				MPxNode::kLocatorNode,
				&squares::drawDbClassification);
	if (!status) {
		status.perror("registerNode");
		return status;
	}

    status = MHWRender::MDrawRegistry::registerDrawOverrideCreator(
		squares::drawDbClassification,
		squares::drawRegistrantId,
		SquaresDrawOverride::Creator);
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

	status = MHWRender::MDrawRegistry::deregisterDrawOverrideCreator(
		squares::drawDbClassification,
		squares::drawRegistrantId);
	if (!status) {
		status.perror("deregisterDrawOverrideCreator");
		return status;
	}

	status = plugin.deregisterNode( squares::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}
	return status;
}