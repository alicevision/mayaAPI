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

//
//	lineManip.cpp
//		Creates manipulator node lineManip
//		Creates command lineManipCmd
//
//	This example demonstrates how to use the MPxManipulatorNode
//	class along with a command to create a user defined
//	manipulator.  The manipulator created is a simple line
//	which is an OpenGL pickable component.  As you
//	move the pickable component, selected transforms have
//	their scale attribute modified. The line's movements
//	are restricted in a plane.
//	A corresponding command is used to create and delete
//	the manipulator node and to support undo/redo etc.
 
/*

// To show this example using MEL, run the following:
loadPlugin lineManip.so;
lineManipCmd -create;

// To delete the manipulator using MEL:
lineManipCmd -delete;

*/

#include "lineManip.h"

#include <maya/MHardwareRenderer.h>
#include <maya/MIOStream.h>
#include <maya/MMatrix.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MFnTransform.h>
#include <maya/MGlobal.h>
#include <maya/MFnCamera.h>
#include <maya/MTemplateCommand.h>

// Statics
MTypeId lineManip::id( 0x81047 );

// Utility class for returning line points
class lineGeometry
{
public:
	static MPoint topPoint() {
		return MPoint( 1.0f, 1.0f, 0.0f );
	}
	static MPoint bottomPoint() {
		return MPoint( 1.0f, -1.0f, 0.0f );
	}
	static MPoint otherPoint() {
		return MPoint( 2.0f, -1.0f, 0.0f );
	}
};

//
// class implementation
//
lineManip::lineManip()
{
	// Setup the plane with a point on the
	// plane along with a normal
	MPoint pointOnPlane(lineGeometry::topPoint());
	// Normal = cross product of two vectors on the plane
	MVector normalToPlane = (MVector(lineGeometry::topPoint()) - MVector(lineGeometry::otherPoint())) ^ 
					(MVector(lineGeometry::otherPoint()) - MVector(lineGeometry::bottomPoint()));
	// Necessary to normalize
	normalToPlane.normalize();
	// Plane defined by a point and a normal
	plane.setPlane( pointOnPlane, normalToPlane );
}

lineManip::~lineManip()
{
	// No-op
}

void lineManip::postConstructor()
{
	// Get the starting value of the pickable items
	glFirstHandle( lineName );
}

// virtual 
void lineManip::draw(M3dView &view, const MDagPath &path,
				M3dView::DisplayStyle style, M3dView::DisplayStatus status)
{
	// Initialize GL function table first time through
	static MGLFunctionTable *gGLFT = 0;
	if ( 0 == gGLFT )
		gGLFT = MHardwareRenderer::theRenderer()->glFunctionTable();

	// Are we in the right view
	MDagPath dpath;
	view.getCamera(dpath);
	MFnCamera viewCamera(dpath);
	const char *nameBuffer = viewCamera.name().asChar();
	if ( 0 == nameBuffer )
		return;
	if ( ( 0 == strstr(nameBuffer,"persp") ) && ( 0 == strstr(nameBuffer,"front") ) )
		return;

	// Populate the point arrays which are in local space
	float top[4],bottom[4];
	lineGeometry::topPoint().get(top);
	lineGeometry::bottomPoint().get(bottom);

	// Depending on what's active, we modify the
	// end points with mouse deltas in local
	// space
	MGLuint active = 0;
	if ( glActiveName( active ) )
	{
		float *a = 0,*b = 0;
		if ( active == lineName )
		{
			a = &top[0]; b = &bottom[0];
		}

		if ( active != 0 )
		{
			a[0] += (float) mousePointGlName.x; a[1] += (float) mousePointGlName.y; a[2] += (float) mousePointGlName.z;
			b[0] += (float) mousePointGlName.x; b[1] += (float) mousePointGlName.y; b[2] += (float) mousePointGlName.z;
		}
	}

	// Begin the drawing
	view.beginGL();

	// Top
	// Place before you draw the manipulator component that can
	// be pickable.
	colorAndName( view, lineName, true, mainColor() );
	gGLFT->glBegin( MGL_LINES );
		gGLFT->glVertex3fv( top );
		gGLFT->glVertex3fv( bottom );
	gGLFT->glEnd();

	// End the drawing
	view.endGL();
}

// virtual 
void	lineManip::preDrawUI( const M3dView &view )
{
	// Initial as draw manipulator in vp2
	fDrawManip = true;

	// Are we in the right view
	M3dView *viewPtr = const_cast<M3dView*>( &view );
	MDagPath dpath;
	viewPtr->getCamera(dpath);
	MFnCamera viewCamera(dpath);
	const char *nameBuffer = viewCamera.name().asChar();
	if ( 0 == nameBuffer )
		fDrawManip = false;
	if ( ( 0 == strstr(nameBuffer,"persp") ) && ( 0 == strstr(nameBuffer,"front") ) )
		fDrawManip = false;

	if ( !fDrawManip )
		return;

	fLineColorIndex = mainColor();
	fSelectedLineColorIndex = selectedColor();
	fLineStart = lineGeometry::topPoint() + MVector( mousePointGlName );
	fLineEnd   = lineGeometry::bottomPoint()   + MVector( mousePointGlName );
}

// virtual 
void	lineManip::drawUI(
	MHWRender::MUIDrawManager& drawManager,
	const MHWRender::MFrameContext& frameContext ) const
{
	if ( !fDrawManip )
		return;

	bool drawAsSelected = false;
	shouldDrawHandleAsSelected(lineName, drawAsSelected);

	drawManager.beginDrawable(lineName, true);
	drawManager.setColorIndex( drawAsSelected ? fSelectedLineColorIndex : fLineColorIndex );
	drawManager.line( fLineStart, fLineEnd );
	drawManager.endDrawable();

	drawManager.beginDrawable();
	drawManager.setColorIndex( fLineColorIndex );
	drawManager.text( fLineStart, MString("line manip"));
	drawManager.endDrawable();

	drawManager.beginDrawable(lineName, true);
	drawManager.setColorIndex( drawAsSelected ? fSelectedLineColorIndex : fLineColorIndex );
	drawManager.line2d(MPoint(100, 100), MPoint(200, 100));
	drawManager.setLineWidth(5.0f);
	drawManager.endDrawable();

	drawManager.beginDrawable();
	drawManager.setColorIndex( fLineColorIndex );
	drawManager.setLineWidth(5.0f);
	drawManager.text2d(MPoint(100, 105), MString("line manip 2D"));
	drawManager.endDrawable();
}

// virtual 
MStatus	lineManip::doPress( M3dView& view )
{
	// Reset the mousePoint information on
	// a new press
	mousePointGlName = MPoint::origin;
	updateDragInformation();
	return MS::kSuccess;
}

// virtual
MStatus	lineManip::doDrag( M3dView& view )
{
	updateDragInformation();
	return MS::kSuccess;
}

// virtual
 MStatus lineManip::doRelease( M3dView& view )
{
	// Scale nodes on the selection list.
	// Simple implementation that does not
	// support undo.
	MSelectionList list;
	MGlobal::getActiveSelectionList( list );
	for ( MItSelectionList iter( list ); !iter.isDone(); iter.next() ) 
	{
		MObject node;
		MStatus status;
	        iter.getDependNode( node );
		MFnTransform xform( node, &status );
		if ( MS::kSuccess == status )
		{
			double newScale[3];
			newScale[0] = mousePointGlName.x + 1;
			newScale[1] = mousePointGlName.y + 1;
			newScale[2] = mousePointGlName.z + 1;
			xform.setScale( newScale );
		}
	}
	return MS::kSuccess;
}

MStatus lineManip::updateDragInformation()
{
	// Find the mouse point in local space
	MPoint localMousePoint;
	MVector localMouseDirection;
	if ( MS::kFailure == mouseRay( localMousePoint, localMouseDirection) )
		return MS::kFailure;

	// Find the intersection of the mouse point with the
	// manip plane
	MPoint mouseIntersectionWithManipPlane;
	if ( ! plane.intersect( localMousePoint, localMouseDirection, 	mouseIntersectionWithManipPlane ) )
		return MS::kFailure;

	mousePointGlName = mouseIntersectionWithManipPlane;

	MGLuint active = 0;
	if ( glActiveName( active ) )
	{
		float start[4],end[4];
		if ( active == lineName )
		{
			lineGeometry::topPoint().get(start);
			lineGeometry::bottomPoint().get(end);
		}

		if ( active != 0 )
		{
			lineMath line;
			// Find a vector on the plane
			MPoint a( start[0], start[1], start[2] );
			MPoint b( end[0], end[1], end[2] );
			MPoint vab = a - b;
			// Define line with a point and a vector on the plane
			line.setLine( start, vab );
			MPoint cpt;
			// Find the closest point so that we can get the
			// delta change of the mouse in local space
			if ( line.closestPoint( mousePointGlName, cpt ) )
			{
				mousePointGlName.x -= cpt.x;
				mousePointGlName.y -= cpt.y;
				mousePointGlName.z -= cpt.z;
			}
		}
	}
	return MS::kFailure;
}

//
// Template command that creates and deletes the manipulator
//

class lineManipCmd;
char cmdName[] = "lineManipCmd";
char nodeName[] = "simpleLineManip"; // lineManip already taken

class lineManipCmd : public MTemplateCreateNodeCommand<lineManipCmd,cmdName,nodeName>
{
public:
	//
	lineManipCmd()
	{}
};

static lineManipCmd _lineManipCmd;

//
// Static methods
//
void* lineManip::creator()
{
	return new lineManip();
}

MStatus lineManip::initialize()
{
	return MS::kSuccess;
}

//
//	Entry points
//

MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "2009", "Any");

	status = plugin.registerNode( nodeName, lineManip::id, lineManip::creator,
		lineManip::initialize, MPxNode::kManipulatorNode );
	if (!status) 
	{
		status.perror("registerNode");
		return status;
	}

	status = _lineManipCmd.registerCommand( obj );
	if (!status) 
	{
		status.perror("registerCommand");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterNode( lineManip::id );
	if (!status) 
	{
		status.perror("deregisterNode");
		return status;
	}
	status = _lineManipCmd.deregisterCommand( obj );
	if (!status) 
	{
		status.perror("deregisterCommand");
		return status;
	}

	return status;
}
