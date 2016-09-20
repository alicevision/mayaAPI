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

//////////////////////////////////////////////////////
//
// Marquee selection within a user defined context.
// Draws the marquee using OpenGL.
// Selection is done through the API (MGlobal).
//
//////////////////////////////////////////////////////

#include <maya/MIOStream.h>
#include <math.h>
#include <stdlib.h>

#include <maya/MFnPlugin.h>
#include <maya/MString.h>
#include <maya/MGlobal.h>
#include <maya/M3dView.h>
#include <maya/MDagPath.h>
 
#include <maya/MItSelectionList.h>
#include <maya/MSelectionList.h>

#include <maya/MPxContextCommand.h>
#include <maya/MPxContext.h>
#include <maya/MEvent.h>

#include <maya/MUIDrawManager.h>
#include <maya/MFrameContext.h>
#include <maya/MPoint.h>
#include <maya/MColor.h>

//////////////////////////////////////////////
// Custom XOR Draw Class
//////////////////////////////////////////////

//	Set this to 1 if you want to use the 'xorDraw' class for
//	customized XOR drawing rather than M3dView's methods.
#define CUSTOM_XOR_DRAW 0

#if CUSTOM_XOR_DRAW
// Example class code which basically does the same operations
// as M3dView's beingXorDrawing() and endXorDrawing() methods.
// This class could be used in place these methods if desired.
//
class xorDraw
{
public:
	xorDraw(M3dView	*view) { fView = view; };
	~xorDraw() {};

	void beginXorDrawing();
	void endXorDrawing();

protected:
	xorDraw() { fView = NULL; }

	// Setup for XOR drawing
	GLboolean depthTest[1];
	GLboolean colorLogicOp[1];
	GLboolean lineStipple[1];

	M3dView *fView;


};

void xorDraw::beginXorDrawing()
{
	// Save the state of these 3 attribtes and restore them later.
	glGetBooleanv (GL_DEPTH_TEST, depthTest);
	glGetBooleanv (GL_COLOR_LOGIC_OP, colorLogicOp);
	glGetBooleanv (GL_LINE_STIPPLE, lineStipple);

	glDrawBuffer( GL_FRONT );

	// Turn Line stippling on.
	glLineStipple( 1, 0x5555 );
	glLineWidth( 1.0 );
	glEnable( GL_LINE_STIPPLE );

	// Save the state of the matrix on stack
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix();

	// Setup the Orthographic projection Matrix.
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
    gluOrtho2D(
    			0.0, (GLdouble) fView->portWidth(),
    			0.0, (GLdouble) fView->portHeight()
    );
    glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
    glTranslatef(0.375, 0.375, 0.0);

	// Set the draw color
	glColor3f(1.0f, 1.0f, 1.0f);

	// Draw the marquee in XOR mode
	
	glDisable (GL_DEPTH_TEST);

	// Enable XOR mode.
	glEnable(GL_COLOR_LOGIC_OP);
	glLogicOp (GL_XOR);
}

void xorDraw::endXorDrawing()
{
	glFlush();
	glDrawBuffer( GL_BACK );

	// Restore the state of the matrix from stack
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();

	// Restore the previous state of these attributes
	if (colorLogicOp[0])
		glEnable (GL_COLOR_LOGIC_OP);
	else
		glDisable (GL_COLOR_LOGIC_OP);

	if (depthTest[0])
		glEnable (GL_DEPTH_TEST);
	else
		glDisable (GL_DEPTH_TEST);

	if (lineStipple[0])
		glEnable( GL_LINE_STIPPLE );
	else
		glDisable( GL_LINE_STIPPLE );
}
#endif


//////////////////////////////////////////////
// The user Context
//////////////////////////////////////////////
const char helpString[] =
			"Click with left button or drag with middle button to select";

class marqueeContext : public MPxContext
{
public:
	marqueeContext();
	virtual void	toolOnSetup( MEvent & event );

	// Default viewport or hardware viewport methods override, will not be triggered in viewport 2.0.
	virtual MStatus	doPress( MEvent & event );
	virtual MStatus	doDrag( MEvent & event );
	virtual MStatus	doRelease( MEvent & event );
	virtual MStatus	doEnterRegion( MEvent & event );

	// Viewport 2.0 methods, will only be triggered in viewport 2.0.
	virtual MStatus		doPress ( MEvent & event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context);
	virtual MStatus		doRelease( MEvent & event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context);
	virtual MStatus		doDrag ( MEvent & event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context);

private:
	// Marquee draw method in default viewport or hardware viewport with immediate OpenGL call
	void					drawMarqueeGL();
	// Common operation to handle when pressed
	void					doPressCommon( MEvent & event );
	// Common operation to handle when released
	void					doReleaseCommon( MEvent & event );

	short					start_x, start_y;
	short					last_x, last_y;

	bool					fsDrawn;

	MGlobal::ListAdjustment	listAdjustment;
	M3dView					view;
};

marqueeContext::marqueeContext()
{
	setTitleString ( "Marquee Tool" );

	// Tell the context which XPM to use so the tool can properly
	// be a candidate for the 6th position on the toolbar.
	setImage("marqueeTool.xpm", MPxContext::kImage1 );
}

void marqueeContext::toolOnSetup ( MEvent & )
{
	setHelpString( helpString );
}

void marqueeContext::doPressCommon( MEvent & event )
{
	// Figure out which modifier keys were pressed, and set up the
	// listAdjustment parameter to reflect what to do with the selected points.
	if (event.isModifierShift() || event.isModifierControl() ) {
		if ( event.isModifierShift() ) {
			if ( event.isModifierControl() ) {
				// both shift and control pressed, merge new selections
				listAdjustment = MGlobal::kAddToList;
			} else {
				// shift only, xor new selections with previous ones
				listAdjustment = MGlobal::kXORWithList;
			}
		} else if ( event.isModifierControl() ) {
			// control only, remove new selections from the previous list
			listAdjustment = MGlobal::kRemoveFromList; 
		}
	} else {
		listAdjustment = MGlobal::kReplaceList;
	}

	// Extract the event information
	//
	event.getPosition( start_x, start_y );
}

void marqueeContext::doReleaseCommon( MEvent & event )
{
	MSelectionList			incomingList, marqueeList;

	// Get the end position of the marquee
	event.getPosition( last_x, last_y );

	// Save the state of the current selections.  The "selectFromSceen"
	// below will alter the active list, and we have to be able to put
	// it back.
	MGlobal::getActiveSelectionList(incomingList);

	// If we have a zero dimension box, just do a point pick
	//
	if ( abs(start_x - last_x) < 2 && abs(start_y - last_y) < 2 ) {
		// This will check to see if the active view is in wireframe or not.
		MGlobal::SelectionMethod selectionMethod = MGlobal::selectionMethod();

		MGlobal::selectFromScreen( start_x, start_y, MGlobal::kReplaceList, selectionMethod );
	} else {
		// The Maya select tool goes to wireframe select when doing a marquee, so
		// we will copy that behaviour.
		// Select all the objects or components within the marquee.
		MGlobal::selectFromScreen( start_x, start_y, last_x, last_y,
			MGlobal::kReplaceList, 
			MGlobal::kWireframeSelectMethod );
	}

	// Get the list of selected items
	MGlobal::getActiveSelectionList(marqueeList);

	// Restore the active selection list to what it was before
	// the "selectFromScreen"
	MGlobal::setActiveSelectionList(incomingList, MGlobal::kReplaceList);

	// Update the selection list as indicated by the modifier keys.
	MGlobal::selectCommand(marqueeList, listAdjustment);
}

MStatus marqueeContext::doPress( MEvent & event )
//
// Begin marquee drawing (using OpenGL)
// Get the start position of the marquee 
//
{
	doPressCommon( event );

	view = M3dView::active3dView();
	fsDrawn = false;

	return MS::kSuccess;
}

MStatus marqueeContext::doDrag( MEvent & event )
//
// Drag out the marquee (using OpenGL)
//
{
#if CUSTOM_XOR_DRAW
	xorDraw XORdraw(&view);
	XORdraw.beginXorDrawing();
#else
	view.beginXorDrawing();
#endif

	if (fsDrawn) {
		// Redraw the marquee at its old position to erase it.
		drawMarqueeGL();
	}

	fsDrawn = true;

	//	Get the marquee's new end position.
	event.getPosition( last_x, last_y );

	// Draw the marquee at its new position.
	drawMarqueeGL();

#if CUSTOM_XOR_DRAW
	XORdraw.endXorDrawing();
#else
	view.endXorDrawing();
#endif

	return MS::kSuccess;		
}

MStatus marqueeContext::doRelease( MEvent & event )
//
// Selects objects within the marquee box.
{
	if (fsDrawn) {
#if CUSTOM_XOR_DRAW
		xorDraw XORdraw(&view);
		XORdraw.beginXorDrawing();
#else
		view.beginXorDrawing();
#endif
		// Redraw the marquee at its old position to erase it.
		drawMarqueeGL();

#if CUSTOM_XOR_DRAW
		XORdraw.endXorDrawing();
#else
		view.endXorDrawing();
#endif
	}

	doReleaseCommon( event );

	return MS::kSuccess;
}

MStatus	marqueeContext::doPress (
	MEvent & event,
	MHWRender::MUIDrawManager& drawMgr,
	const MHWRender::MFrameContext& context)
{
	doPressCommon( event );
	return MS::kSuccess;
}

MStatus	marqueeContext::doRelease(
	MEvent & event,
	MHWRender::MUIDrawManager& drawMgr,
	const MHWRender::MFrameContext& context)
{
	doReleaseCommon( event );
	return MS::kSuccess;
}

MStatus	marqueeContext::doDrag (
	MEvent & event,
	MHWRender::MUIDrawManager& drawMgr,
	const MHWRender::MFrameContext& context)
{
	//	Get the marquee's new end position.
	event.getPosition( last_x, last_y );

	// Draw the marquee at its new position.
	drawMgr.beginDrawable();
	drawMgr.setColor( MColor(1.0f, 1.0f, 0.0f) );
	drawMgr.line2d( MPoint( start_x, start_y), MPoint(last_x, start_y) );
	drawMgr.line2d( MPoint( last_x, start_y), MPoint(last_x, last_y) );
	drawMgr.line2d( MPoint( last_x, last_y), MPoint(start_x, last_y) );
	drawMgr.line2d( MPoint( start_x, last_y), MPoint(start_x, start_y) );

	double len = (last_y - start_y) * (last_y - start_y) + 
		(last_x - start_x) * (last_x - start_x) * 0.01;
	drawMgr.line(MPoint(0,0,0), MPoint(len, len, len));
	drawMgr.endDrawable();

	return MS::kSuccess;
}

MStatus marqueeContext::doEnterRegion( MEvent & )
{
	return setHelpString( helpString );
}

void marqueeContext::drawMarqueeGL()
{
	glBegin( GL_LINE_LOOP );
		glVertex2i( start_x, start_y );
		glVertex2i( last_x, start_y );
		glVertex2i( last_x, last_y );
		glVertex2i( start_x, last_y );
	glEnd();
}


//////////////////////////////////////////////
// Command to create contexts
//////////////////////////////////////////////

class marqueeContextCmd : public MPxContextCommand
{
public:	
						marqueeContextCmd();
	virtual MPxContext*	makeObj();
	static	void*		creator();
};

marqueeContextCmd::marqueeContextCmd() {}

MPxContext* marqueeContextCmd::makeObj()
{
	return new marqueeContext();
}

void* marqueeContextCmd::creator()
{
	return new marqueeContextCmd;
}

//////////////////////////////////////////////
// plugin initialization
//////////////////////////////////////////////
MStatus initializePlugin( MObject obj )
{
	MStatus		status;
	MFnPlugin	plugin( obj, PLUGIN_COMPANY, "12.0", "Any");

	status = plugin.registerContextCommand( "marqueeToolContext",
										    marqueeContextCmd::creator );
	return status;
}

MStatus uninitializePlugin( MObject obj )
{
	MStatus		status;
	MFnPlugin	plugin( obj );

	status = plugin.deregisterContextCommand( "marqueeToolContext" );

	return status;
}
