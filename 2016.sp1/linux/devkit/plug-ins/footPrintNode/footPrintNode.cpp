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
// This plug-in demonstrates how to draw a simple mesh like foot Print in an easy way.
//
// This easy way is supported in Viewport 2.0.
// In Viewport 2.0, MUIDrawManager can used to draw simple UI elements in method addUIDrawables().
//
// For comparison, you can reference a devkit sample named rawfootPrintNode.
// In that sample, we draw the footPrint with OpenGL\DX in method rawFootPrintDrawOverride::draw().
//
// Note the method 
//   footPrint::draw() 
// is only called in legacy default viewport to draw foot Print.
// while the methods 
//   FootPrintDrawOverride::prepareForDraw() 
//   FootPrintDrawOverride::addUIDrawables() 
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
#include <maya/MPxDrawOverride.h>
#include <maya/MUserData.h>
#include <maya/MDrawContext.h>
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

	// Draw the name of the footPrint
	view.setDrawColor( MColor( 0.1f, 0.8f, 0.8f, 1.0f ) );
	view.drawText( MString("Footprint"), MPoint( 0.0, 0.0, 0.0 ), M3dView::kCenter );
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

class FootPrintData : public MUserData
{
public:
	FootPrintData() : MUserData(false) {} // don't delete after draw
	virtual ~FootPrintData() {}

	MColor fColor;
	MPointArray fSoleLineList;
	MPointArray fSoleTriangleList;
	MPointArray fHeelLineList;
	MPointArray fHeelTriangleList;
};

class FootPrintDrawOverride : public MHWRender::MPxDrawOverride
{
public:
	static MHWRender::MPxDrawOverride* Creator(const MObject& obj)
	{
		return new FootPrintDrawOverride(obj);
	}

	virtual ~FootPrintDrawOverride();

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
	FootPrintDrawOverride(const MObject& obj);
	float getMultiplier(const MDagPath& objPath) const;
};

FootPrintDrawOverride::FootPrintDrawOverride(const MObject& obj)
: MHWRender::MPxDrawOverride(obj, FootPrintDrawOverride::draw)
{
}

FootPrintDrawOverride::~FootPrintDrawOverride()
{
}

MHWRender::DrawAPI FootPrintDrawOverride::supportedDrawAPIs() const
{
	// this plugin supports both GL and DX
	return (MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile);
}

float FootPrintDrawOverride::getMultiplier(const MDagPath& objPath) const
{
	// Retrieve value of the size attribute from the node
	MStatus status;
	MObject footprintNode = objPath.node(&status);
	if (status)
	{
		MPlug plug(footprintNode, footPrint::size);
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

bool FootPrintDrawOverride::isBounded(const MDagPath& /*objPath*/,
                                      const MDagPath& /*cameraPath*/) const
{
	return true;
}

MBoundingBox FootPrintDrawOverride::boundingBox(
	const MDagPath& objPath,
	const MDagPath& cameraPath) const
{
	MPoint corner1( -0.17, 0.0, -0.7 );
	MPoint corner2( 0.17, 0.0, 0.3 );

	float multiplier = getMultiplier(objPath);
	corner1 = corner1 * multiplier;
	corner2 = corner2 * multiplier;

	return MBoundingBox( corner1, corner2 );
}

// Called by Maya each time the object needs to be drawn.
MUserData* FootPrintDrawOverride::prepareForDraw(
	const MDagPath& objPath,
	const MDagPath& cameraPath,
	const MHWRender::MFrameContext& frameContext,
	MUserData* oldData)
{
	// Any data needed from the Maya dependency graph must be retrieved and cached in this stage.
	// There is one cache data for each drawable instance, if it is not desirable to allow Maya to handle data
	// caching, simply return null in this method and ignore user data parameter in draw callback method.
	// e.g. in this sample, we compute and cache the data for usage later when we create the 
	// MUIDrawManager to draw footprint in method addUIDrawables().
	FootPrintData* data = dynamic_cast<FootPrintData*>(oldData);
	if (!data)
	{
		data = new FootPrintData();
	}
	
	float fMultiplier = getMultiplier(objPath);

	data->fSoleLineList.clear();
	for (int i = 0; i < soleCount; i++)
	{
		data->fSoleLineList.append(sole[i][0] * fMultiplier, sole[i][1] * fMultiplier, sole[i][2] * fMultiplier);
	}

	data->fHeelLineList.clear();
	for (int i = 0; i < heelCount; i++)
	{
		data->fHeelLineList.append(heel[i][0] * fMultiplier, heel[i][1] * fMultiplier, heel[i][2] * fMultiplier);
	}

	data->fSoleTriangleList.clear();	
	for (int i = 1; i <= soleCount - 2; i++)
	{
		data->fSoleTriangleList.append(sole[0][0] * fMultiplier, sole[0][1] * fMultiplier, sole[0][2] * fMultiplier);
		data->fSoleTriangleList.append(sole[i][0] * fMultiplier, sole[i][1] * fMultiplier, sole[i][2] * fMultiplier);
		data->fSoleTriangleList.append(sole[i+1][0] * fMultiplier, sole[i+1][1] * fMultiplier, sole[i+1][2] * fMultiplier);
	}

	data->fHeelTriangleList.clear();	
	for (int i = 1; i <= heelCount - 2; i++)
	{
		data->fHeelTriangleList.append(heel[0][0] * fMultiplier, heel[0][1] * fMultiplier, heel[0][2] * fMultiplier);
		data->fHeelTriangleList.append(heel[i][0] * fMultiplier, heel[i][1] * fMultiplier, heel[i][2] * fMultiplier);
		data->fHeelTriangleList.append(heel[i+1][0] * fMultiplier, heel[i+1][1] * fMultiplier, heel[i+1][2] * fMultiplier);
	}

    // get correct color based on the state of object, e.g. active or dormant
	data->fColor = MHWRender::MGeometryUtilities::wireframeColor(objPath);

	return data;
}

// addUIDrawables() provides access to the MUIDrawManager, which can be used
// to queue up operations for drawing simple UI elements such as lines, circles and
// text. To enable addUIDrawables(), override hasUIDrawables() and make it return true.
void FootPrintDrawOverride::addUIDrawables(
		const MDagPath& objPath,
		MHWRender::MUIDrawManager& drawManager,
		const MHWRender::MFrameContext& frameContext,
		const MUserData* data)
{
	// Get data cached by prepareForDraw() for each drawable instance, then MUIDrawManager 
	// can draw simple UI by these data.
	FootPrintData* pLocatorData = (FootPrintData*)data;
	if (!pLocatorData)
	{
		return;
	}

	drawManager.beginDrawable();

	// Draw the foot print solid/wireframe
	drawManager.setColor( pLocatorData->fColor );
	drawManager.setDepthPriority(5);

	if (frameContext.getDisplayStyle() & MHWRender::MFrameContext::kGouraudShaded) {
		drawManager.mesh(MHWRender::MUIDrawManager::kTriangles, pLocatorData->fSoleTriangleList);
		drawManager.mesh(MHWRender::MUIDrawManager::kTriangles, pLocatorData->fHeelTriangleList);
	}

	drawManager.mesh(MHWRender::MUIDrawManager::kClosedLine, pLocatorData->fSoleLineList);
	drawManager.mesh(MHWRender::MUIDrawManager::kClosedLine, pLocatorData->fHeelLineList);

	// Draw a text "Foot"
	MPoint pos( 0.0, 0.0, 0.0 ); // Position of the text
	MColor textColor( 0.1f, 0.8f, 0.8f, 1.0f ); // Text color

	drawManager.setColor( textColor );
	drawManager.setFontSize( MHWRender::MUIDrawManager::kSmallFontSize );
	drawManager.text( pos,  MString("Footprint"), MHWRender::MUIDrawManager::kCenter );

	drawManager.endDrawable();
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

    status = MHWRender::MDrawRegistry::registerDrawOverrideCreator(
		footPrint::drawDbClassification,
		footPrint::drawRegistrantId,
		FootPrintDrawOverride::Creator);
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
