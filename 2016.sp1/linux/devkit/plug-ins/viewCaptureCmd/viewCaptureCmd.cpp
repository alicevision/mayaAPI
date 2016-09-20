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

// This plugin uses OpenGL to capture the current 3D view to a ppm file.
//
// To use it, give it a filename as an argument into which the PPM image
// of the current view should be written.
//
// Limitations:
// 	- any parts of other X windows that are obscuring the view
// 	  will be captured rather than the view underneath.  This is
// 	  an effect of the OpenGL buffer system on SGIs.
// 
// 	- colour index mode buffers cannot be read by this plugin, so
// 	  the view should be set to shaded mode before doing the capture.
// 	  It is posible to read an OpenGL colour index mode buffer, but it
// 	  is more complicated, and is therefore an exercise left to the
// 	  readers.

#include "viewCapturePPM.h"   // PPM routines

#include <maya/MSimple.h>
#include <maya/MObject.h>
#include <maya/MGlobal.h> 
#include <maya/M3dView.h> 

#include <maya/MGL.h> 

#include <stdlib.h>
#include <maya/MIOStream.h>

DeclareSimpleCommand( viewCapture, PLUGIN_COMPANY, "3.0")

MStatus viewCapture::doIt( const MArgList& args )
{
	MStatus status = MS::kSuccess;

	if ( args.length() != 1 ) {
		// Need the file name argument
		//
		return MS::kFailure;
	}

	MString fileName;
	args.get( 0, fileName );

	// Get the active 3D view
	//
	M3dView view = M3dView::active3dView();

	// Capture the current view
	//
	view.refresh();
	view.beginGL();
 
	// Set the target for our pixel read to be the front buffer.  First, the
	// current state is saved using the glPushAttrib call.  It is important
	// to leave the OpenGL in the same state that we found it.
	//
	glPushAttrib( GL_PIXEL_MODE_BIT ); 

	int width = view.portWidth();
	int height = view.portHeight(); 

	// Allocate buffers for the pixel data
	//
	GLfloat * red   = new GLfloat[width*height];
	GLfloat * green = new GLfloat[width*height];
	GLfloat * blue  = new GLfloat[width*height];

	// Read the values from the OpenGL frame buffer
	//
	glReadBuffer( GL_FRONT );
	glReadPixels( 0, 0, width, height, GL_RED, GL_FLOAT, red );
	glReadPixels( 0, 0, width, height, GL_GREEN, GL_FLOAT, green );
	glReadPixels( 0, 0, width, height, GL_BLUE, GL_FLOAT, blue );
	
	// Put the gl read target back
	//
	glPopAttrib(); 

	view.endGL();

	// Write file as a PPM
	//
	Pic_Pixel * line = PixelAlloc( width );
	int idx;

	Pic * file = PicOpen( fileName.asChar(), (short) width, (short) height );
	if ( NULL != file ) { 
		for ( int row = height - 1; row >= 0; row-- ) {
			// Covert the row of pixels into PPM format
			//
			for ( int col = 0; col < width; col++ ) {
				// Find the array elements for this pixel
				//
				idx = ( row * width ) + ( col );
				line[col].r = (Pic_byte)( red[idx]   * 255.0 );
				line[col].g = (Pic_byte)( green[idx] * 255.0 );
				line[col].b = (Pic_byte)( blue[idx] * 255.0 );
			}
			// Write the line
			//
			if ( !PicWriteLine( file, line ) ) {
				status = MS::kFailure; 
				return MS::kFailure;
			}
		}
		PicClose( file ); 
	}
 
	delete []red;
	delete []green;
	delete []blue;
	PixelFree( line );

	return status;
}
