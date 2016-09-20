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

/* 

	This example is based on the squareScaleManip example but uses
	a context and context command. Template classes are used
	for defining the context and context command below. If the
	plug-in, context is active, selecting geometry will show
	the manipulator.  Only the right and left sides of the
	square currently modify the geometry if moved.

	Loading and unloading:
	----------------------

	The square scale manipulator context and tool button can be created with the 
	following MEL commands:

		loadPlugin squareScaleManipContext;
		squareScaleManipContext squareScaleManipContext1;
		setParent Shelf1;
		toolButton -cl toolCluster
					-i1 "moveManip.xpm"
					-t squareScaleManipContext1
					squareManip1;

	If the preceding commands were used to create the manipulator context, 
	the following commands can destroy it:

		deleteUI squareScaleManipContext1;
		deleteUI squareManip1;

	If the plug-in is loaded and unloaded frequently (eg. during testing),
	it is useful to make these command sequences into shelf buttons.

	How to use:
	-----------

	Once the tool button has been created using the script above, select the
	tool button then click on an object. Move the right and left edges of the
	square to modify the selected object's scale.

	There is code duplication between this example and squareSclaeManip.  But
	the important additions here are the calls to addDoubleValue() and
	addDoubleValue() and the virtual connectToDependNode(). This functionality
	ties the updating of the manipulator into changing a node's attribute(s).
	
*/

#include <maya/MIOStream.h>

#include <maya/MPxNode.h>
#include <maya/MPxManipulatorNode.h>
#include <maya/MPxSelectionContext.h>
#include <maya/MPxContextCommand.h>
#include <maya/MModelMessage.h>
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include <maya/MItSelectionList.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MDagPath.h>
#include <maya/MManipData.h>
#include <maya/MHardwareRenderer.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnTransform.h>
#include <maya/MMatrix.h>

#include <maya/MTemplateManipulator.h>

#include "squareScaleManipContext.h"

// Statics
MTypeId squareScaleManipulator::id( 0x81048 );

//
// squareScaleManipulator
//

// Utility class for returning square points
class squareGeometry
{
public:
	static MPoint topLeft() {
		return MPoint( -0.5f, 0.5f, 0.0f );
	}
	static MPoint topRight() {
		return MPoint( 0.5f, 0.5f, 0.0f );
	}
	static MPoint bottomLeft() {
		return MPoint( -0.5f, -0.5f, 0.0f );
	}
	static MPoint bottomRight() {
		return MPoint( 0.5f, -0.5f, 0.0f );
	}
};

//
// class implementation
//
squareScaleManipulator::squareScaleManipulator()
{
	// Setup the plane with a point on the
	// plane along with a normal
	MPoint pointOnPlane(squareGeometry::topLeft());
	// Normal = cross product of two vectors on the plane
	MVector normalToPlane = (MVector(squareGeometry::topLeft()) - MVector(squareGeometry::topRight())) ^ 
					(MVector(squareGeometry::topRight()) - MVector(squareGeometry::bottomRight()));
	// Necessary to normalize
	normalToPlane.normalize();
	plane.setPlane( pointOnPlane, normalToPlane );

	// Set plug indicies to a default
	topIndex = rightIndex = bottomIndex = leftIndex = -1;

	// initialize rotate/translate to a good default
	rotateX = rotateY = rotateZ = 0.0f;
	translateX = translateY = translateZ = 0.0f;
}

squareScaleManipulator::~squareScaleManipulator()
{
	// No-op
}

// virtual 
void squareScaleManipulator::postConstructor()
{
	// In the postConstructor, the manipulator node
	// is setup.  Add the values that we want to
	// track
	MStatus status;
	status = addDoubleValue( "topValue", 0, topIndex );
	if ( !status )
		return;

	status = addDoubleValue( "rightValue", 0, rightIndex );
	if ( !status )
		return;

	status = addDoubleValue( "bottomValue", 0, bottomIndex );
	if ( !status )
		return;

	status = addDoubleValue( "leftValue", 0, leftIndex );
	if ( !status )
		return;
}

// virtual 
MStatus squareScaleManipulator::connectToDependNode(const MObject &dependNode)
{
	// Make sure we have a scaleX plug and connect the
	// plug to the rightIndex we created in the
	// postConstructor
	MStatus status;
	MFnDependencyNode nodeFn(dependNode,&status);
	if ( ! status )
		return MS::kFailure;

	MPlug scaleXPlug = nodeFn.findPlug("scaleX", &status);
	if ( ! status )
		return MS::kFailure;

	int plugIndex = 0;
	status = connectPlugToValue(scaleXPlug,rightIndex, plugIndex);
	if ( !status )
		return MS::kFailure;

	finishAddingManips();

	return MPxManipulatorNode::connectToDependNode( dependNode );
}

// virtual 
void squareScaleManipulator::draw(M3dView &view, const MDagPath &path,
				M3dView::DisplayStyle style, M3dView::DisplayStatus status)
{
	static MGLFunctionTable *gGLFT = 0;
	if ( 0 == gGLFT )
		gGLFT = MHardwareRenderer::theRenderer()->glFunctionTable();

	// Populate the point arrays which are in local space
	float tl[4],tr[4],br[4],bl[4];
	squareGeometry::topLeft().get(tl);
	squareGeometry::topRight().get(tr);
	squareGeometry::bottomLeft().get(bl);
	squareGeometry::bottomRight().get(br);

	// Depending on what's active, we modify the
	// end points with mouse deltas in local
	// space
	MGLuint active = 0;
	if ( glActiveName( active ) )
	{
		float *a = 0,*b = 0;
		if ( active == topName )
		{
			a = &tl[0]; b = &tr[0];
		}
		if ( active == bottomName )
		{
			a = &bl[0]; b = &br[0];
		}
		if ( active == rightName )
		{
			a = &tr[0]; b = &br[0];
		}
		if ( active == leftName )
		{
			a = &tl[0]; b = &bl[0];
		}

		if ( active != 0 )
		{
			a[0] += (float) mousePointGlName.x; a[1] += (float) mousePointGlName.y; a[2] += (float) mousePointGlName.z;
			b[0] += (float) mousePointGlName.x; b[1] += (float) mousePointGlName.y; b[2] += (float) mousePointGlName.z;
		}
	}

	// Begin the drawing
	view.beginGL();

	// Push the matrix and set the translate/rotate. Perform
	// operations in reverse order
	DegreeRadianConverter convert;
	gGLFT->glPushMatrix();
	gGLFT->glTranslatef( translateX, translateY, translateZ );
	gGLFT->glRotatef( (float) convert.radiansToDegrees(rotateZ), 0.0f, 0.0f, 1.0f );
	gGLFT->glRotatef( (float) convert.radiansToDegrees(rotateY), 0.0f, 1.0f, 0.0f );
	gGLFT->glRotatef( (float) convert.radiansToDegrees(rotateX), 1.0f, 0.0f, 0.0f );

	// Get the starting index of the first pickable component
	MGLuint glPickableItem;
	glFirstHandle( glPickableItem );

	// Top
	topName = glPickableItem;
	// Place before you draw the manipulator component that can
	// be pickable.
	colorAndName( view, glPickableItem, false, mainColor() );
	gGLFT->glBegin( MGL_LINES );
		gGLFT->glVertex3fv( tl );
		gGLFT->glVertex3fv( tr );
	gGLFT->glEnd();

	// Right
	glPickableItem++;
	rightName = glPickableItem;
	colorAndName( view, glPickableItem, true, mainColor() );
	gGLFT->glBegin( MGL_LINES );
		gGLFT->glVertex3fv( tr );
		gGLFT->glVertex3fv( br );
	gGLFT->glEnd();

	// Bottom
	glPickableItem++;
	bottomName = glPickableItem;
	colorAndName( view, glPickableItem, false, mainColor() );
	gGLFT->glBegin( MGL_LINES );
		gGLFT->glVertex3fv( br );
		gGLFT->glVertex3fv( bl );
	gGLFT->glEnd();

	// Left
	glPickableItem++;
	leftName = glPickableItem;
	colorAndName( view, glPickableItem, true, mainColor() );
	gGLFT->glBegin( MGL_LINES );
		gGLFT->glVertex3fv( bl );
		gGLFT->glVertex3fv( tl );
	gGLFT->glEnd();

	// Pop matrix
	gGLFT->glPopMatrix();

	// End the drawing
	view.endGL();
}

// virtual 
MStatus	squareScaleManipulator::doPress( M3dView& view )
{
	// Reset the mousePoint information on
	// a new press
	mousePointGlName = MPoint::origin;
	updateDragInformation();
	return MS::kSuccess;
}

// virtual
MStatus	squareScaleManipulator::doDrag( M3dView& view )
{
	updateDragInformation();
	return MS::kSuccess;
}

// virtual
 MStatus squareScaleManipulator::doRelease( M3dView& view )
{
	return MS::kSuccess;
}

void squareScaleManipulator::setDrawTransformInfo( double rotation[3], MVector translation )
{
	rotateX = (float) rotation[0]; rotateY = (float) rotation[1]; rotateZ =  (float) rotation[2];
	translateX = (float) translation.x; translateY = (float) translation.y; translateZ = (float) translation.z;
}

MStatus squareScaleManipulator::updateDragInformation()
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
            if ( active == topName )
            {
                    squareGeometry::topLeft().get(start);
                    squareGeometry::topRight().get(end);
            }
            if ( active == bottomName )
            {
                    squareGeometry::bottomLeft().get(start);
                    squareGeometry::bottomRight().get(end);
            }
            if ( active == rightName )
            {
					squareGeometry::topRight().get(start);
                    squareGeometry::bottomRight().get(end);
            }
            if ( active == leftName )
            {
					squareGeometry::topLeft().get(start);
                    squareGeometry::bottomLeft().get(end);
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
 
				double minChangeValue = minOfThree( mousePointGlName.x, mousePointGlName.y, mousePointGlName.z );
				double maxChangeValue = maxOfThree( mousePointGlName.x, mousePointGlName.y, mousePointGlName.z );
				if ( active == rightName )
				{
					setDoubleValue( rightIndex, maxChangeValue );
				}
				if ( active == leftName )
				{
					setDoubleValue( rightIndex, minChangeValue );
				}
			}
			return MS::kSuccess;
    }
	return MS::kFailure;	
}

//
// squareScaleManipContext
//

class squareScaleManipContext;
char contextName[] = "squareScaleManipContext";
char manipulatorNodeName[] = "squareScaleContextManipulator";

class squareScaleManipContext : 
	public MTemplateSelectionContext<contextName, squareScaleManipContext, 
		MFn::kTransform, squareScaleManipulator, manipulatorNodeName >
{
public:
	squareScaleManipContext() {}
	virtual ~squareScaleManipContext() {}

	// Only work on scaleX
	virtual void namesOfAttributes(MStringArray& namesOfAttributes)
	{
		namesOfAttributes.append("scaleX");
	}

	// firstObjectSelected will be set so that we can
	// determine translate and rotate. We then push
	// this info into the manipulator using the
	// manipulatorClassPtr pointer
	virtual void setInitialState()
	{
		MStatus status;
		MFnTransform xform( firstObjectSelected, &status );
		if ( MS::kSuccess != status )
			return;

		MTransformationMatrix xformMatrix = xform.transformation(&status);
		if ( MS::kSuccess != status )
			return;

		MTransformationMatrix rotateTranslateMatrix = xformMatrix.asRotateMatrix();
		rotateTranslateMatrix.setTranslation( xformMatrix.getTranslation(MSpace::kWorld), MSpace::kWorld );
		MMatrix matrix = rotateTranslateMatrix.asMatrix();
		cout << matrix << endl;

		double rotation[3];
		MTransformationMatrix::RotationOrder ro;
		xformMatrix.getRotation( rotation, ro );
		MVector translation;
		translation = xformMatrix.getTranslation( MSpace::kWorld );

		manipulatorClassPtr->setDrawTransformInfo( rotation, translation );
	}

};

//
//	Setup the context command which makes the context
//

class squareScaleManipContextCommand;
char contextCommandName[] = "squareScaleManipContext";

class squareScaleManipContextCommand : 
	public MTemplateContextCommand<contextCommandName, squareScaleManipContextCommand, squareScaleManipContext >
{
public:
	squareScaleManipContextCommand() {}
	virtual ~squareScaleManipContextCommand() {}
};

static squareScaleManipContextCommand _squareScaleManipContextCommand;


//
// Static methods for the manipulator node
//
void* squareScaleManipulator::creator()
{
	return new squareScaleManipulator();
}

MStatus squareScaleManipulator::initialize()
{
	return MS::kSuccess;
}

//
// Entry points
//

MStatus initializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "2009", "Any");

	status = _squareScaleManipContextCommand.registerContextCommand( obj );
	if (!status) 
	{
		MString errorInfo("Error: registering context command : ");
		errorInfo += contextCommandName;
		MGlobal::displayError(errorInfo);
		return status;
	}

	status = plugin.registerNode(manipulatorNodeName, squareScaleManipulator::id, 
								 &squareScaleManipulator::creator, &squareScaleManipulator::initialize,
								 MPxNode::kManipulatorNode);
	if (!status) 
	{
		MString str("Error registering node: ");
		str+= manipulatorNodeName;
		MGlobal::displayError(str);
		return status;
	}

	return status;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj);

	status = _squareScaleManipContextCommand.deregisterContextCommand( obj );
	if (!status) 
	{
		MString errorInfo("Error: deregistering context command : ");
		errorInfo += contextCommandName;
		MGlobal::displayError(errorInfo);
		return status;
	}

	status = plugin.deregisterNode(squareScaleManipulator::id);
	if (!status) 
	{
		MString str("Error deregistering node: ");
		str+= manipulatorNodeName;
		MGlobal::displayError(str);
		return status;
	}

	return status;
}
