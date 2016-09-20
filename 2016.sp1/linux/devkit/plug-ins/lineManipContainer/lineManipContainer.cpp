//-
// ==========================================================================
// Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

//
// This example is based on the lineManip.cpp example but
// incorporates a manip container so that two lines can
// be created as children.  The left line will change
// translateX of the selected nodes.  The right line
// will change scaleX of the selected node.  
 
/*
	// To show this example using MEL, run the following:

	loadPlugin lineManipContainer;
	lineManipContainerContext lineManipContainerContext1;
	setParent Shelf1;
	toolButton -cl toolCluster
				-i1 "moveManip.xpm"
				-t lineManipContainerContext1
				lineManipContainer1;

	If the preceding commands were used to create the manipulator context, 
	the following commands can destroy it:

		deleteUI lineManipContainerContext1;
		deleteUI lineManipContainer1;

*/

#include "lineManipContainer.h"

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
#include <maya/MTemplateManipulator.h>

// Statics
MTypeId lineManip::id( 0x81049 );
MTypeId lineManipContainer::id( 0x8104A );

// Utility class for returning line points
class lineGeometry
{
public:
	static MPoint topPoint( bool rightLine = true ) {
		if ( rightLine )
			return MPoint( 1.0f, 1.0f, 0.0f );
		return MPoint( -1.0f, 1.0f, 0.0f );
	}
	static MPoint bottomPoint( bool rightLine = true ) {
		if ( rightLine )
			return MPoint( 1.0f, -1.0f, 0.0f );
		return MPoint( -1.0f, -1.0f, 0.0f );
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
	plane.setPlane( pointOnPlane, normalToPlane );

	// default case
	affectScale = affectTranslate = false;
}

lineManip::~lineManip()
{
	// No-op
}

// virtual 
void lineManip::draw(M3dView &view, const MDagPath &path,
				M3dView::DisplayStyle style, M3dView::DisplayStatus status)
{
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

	//
	bool rightLine = true;
	if ( affectTranslate )
			rightLine = false;

	// Populate the point arrays which are in local space
	// Return values change with setting of rightLine
	// boolean
	float top[4],bottom[4];
	lineGeometry::topPoint(rightLine).get(top);
	lineGeometry::bottomPoint(rightLine).get(bottom);

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

	// Get the starting value of the pickable items
	MGLuint glPickableItem;
	glFirstHandle( glPickableItem );

	// Top
	lineName = glPickableItem;
	// Place before you draw the manipulator component that can
	// be pickable.
	colorAndName( view, glPickableItem, true, mainColor() );
	gGLFT->glBegin( MGL_LINES );
		gGLFT->glVertex3fv( top );
		gGLFT->glVertex3fv( bottom );
	gGLFT->glEnd();

	// End the drawing
	view.endGL();
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
	// Scale nodes on the selection list
	// No undo supported
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
			if ( affectScale )
			{
				double newScale[3];
				newScale[0] = mousePointGlName.x + 1;
				newScale[1] = mousePointGlName.y + 1;
				newScale[2] = mousePointGlName.z + 1;
				xform.setScale( newScale );
			}
			if ( affectTranslate )
			{
				double newTranslate[3];
				newTranslate[0] = mousePointGlName.x;
				newTranslate[1] = mousePointGlName.y;
				newTranslate[2] = mousePointGlName.z;
				xform.setTranslation( newTranslate, MSpace::kObject );
			}
		}
	}
	return MS::kSuccess;
}

 // Utility method for finding the mouse delta
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
		float start[4] = {0,0,0,0},end[4] = {0,0,0,0};
		if ( active == lineName )
		{
			//
			bool rightLine = true;
			if ( affectTranslate )
					rightLine = false;

			lineGeometry::topPoint(rightLine).get(start);
			lineGeometry::bottomPoint(rightLine).get(end);
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
// Manip container
//

lineManipContainer::lineManipContainer() 
{ 
    // No-op
}


lineManipContainer::~lineManipContainer() 
{
    // No-op
}

// Make the children manipulators: left and
// right lines
MStatus lineManipContainer::createChildren()
{
    MStatus status;

	MPxManipulatorNode *proxyManip = 0;

	MString manipTypeName("singleLineManip");
	MString manipName("rightLineManip");
	status = addMPxManipulatorNode( manipTypeName, manipName, proxyManip );
	if ( !status )
	{
		return MS::kFailure;
	}

	lineManip *rightManipPtr = (lineManip *) proxyManip;
	if ( rightManipPtr )
	{
		rightManipPtr->affectScale = true;
	}

	proxyManip = 0;
	manipTypeName = "singleLineManip";
	manipName = "leftLineManip";
	status = addMPxManipulatorNode( manipTypeName, manipName, proxyManip );
	if ( !status )
	{
		return MS::kFailure;
	}

	lineManip *leftManipPtr = (lineManip *) proxyManip;
	if ( leftManipPtr )
	{
		leftManipPtr->affectTranslate = true;
	}

    return status;
}


MStatus lineManipContainer::connectToDependNode(const MObject &node)
{
	// No-op
	return MS::kSuccess;
}

// Drw the children and then draw our text info
void lineManipContainer::draw(M3dView & view, 
								 const MDagPath &path, 
								 M3dView::DisplayStyle style,
								 M3dView::DisplayStatus status)
{ 
    MPxManipContainer::draw(view, path, style, status);

	//
    view.beginGL(); 

    MPoint textPos = nodeTranslation();
    MString str( "Two custom line manipulators"); 
    MString distanceText(str.asChar());
    view.drawText(distanceText, textPos, M3dView::kLeft);

	// 
    view.endGL();
}
// Return the node translation
MVector lineManipContainer::nodeTranslation() const
{
	MFnDagNode dagFn(fNodePath);
	MDagPath path;
	dagFn.getPath(path);
	path.pop();  // pop from the shape to the transform
	MFnTransform transformFn(path);
	return transformFn.translation(MSpace::kWorld);
}

//
// lineManipContainerContext: implemented as a template
//

class lineManipContainerContext;
char contextName[] = "lineManipContainerContext";
char manipulatorContainerNodeName[] = "lineManipContainer";

class lineManipContainerContext : 
	public MTemplateSelectionContext<contextName, lineManipContainerContext, 
		MFn::kTransform, lineManipContainer, manipulatorContainerNodeName >
{
public:
	lineManipContainerContext() {}
	virtual ~lineManipContainerContext() {}

	virtual void namesOfAttributes(MStringArray& namesOfAttributes)
	{
		namesOfAttributes.append("scaleX");
		namesOfAttributes.append("translateX");
	}
};

//
//	lineManipContainerContextCommand: implemented as a template
//

class lineManipContainerContextCommand;
char contextCommandName[] = "lineManipContainerContext";

class lineManipContainerContextCommand : 
	public MTemplateContextCommand<contextCommandName, lineManipContainerContextCommand, lineManipContainerContext >
{
public:
	lineManipContainerContextCommand() {}
	virtual ~lineManipContainerContextCommand() {}
};

static lineManipContainerContextCommand _lineManipContainerContextCommand;

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

void* lineManipContainer::creator()
{
     return new lineManipContainer();
}


MStatus lineManipContainer::initialize()
{ 
    MStatus status;
    status = MPxManipContainer::initialize();
    return status;
}


//
//	Entry points
//

MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "2009", "Any");

	status = plugin.registerNode( "singleLineManip", lineManip::id, lineManip::creator,
		lineManip::initialize, MPxNode::kManipulatorNode );
	if (!status) 
	{
		status.perror("registerNode");
		return status;
	}

	status = plugin.registerNode( manipulatorContainerNodeName, lineManipContainer::id, lineManipContainer::creator,
			 &lineManipContainer::initialize, MPxNode::kManipContainer);
	if (!status) 
	{
		status.perror("registerNode");
		return status;
	}

	status = _lineManipContainerContextCommand.registerContextCommand( obj );
	if (!status) 
	{
		status.perror("registerContextCommand");
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
	status = plugin.deregisterNode(lineManipContainer::id);;
	if (!status) 
	{
		status.perror("deregisterNode");
		return status;
	}
	status = _lineManipContainerContextCommand.deregisterContextCommand( obj );
	if (!status) 
	{
		status.perror("deregisterContextCommand");
		return status;
	}

	return status;
}
