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

////////////////////////////////////////////////////////////////////////
//
// blastCmd.cpp
//
// This plugin is an example which uses the offscreen rendering API
// extension.
//
////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <maya/MFStream.h>

#include <maya/M3dView.h>
#include <maya/MPxGlBuffer.h>

#include <maya/MFnPlugin.h>
#include <maya/MString.h>
#include <maya/MArgList.h>

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>

#include <maya/MGlobal.h>
#include <maya/MAnimControl.h>
#include <maya/MImage.h>
#include <maya/MIOStream.h>

// Use this to choose between Linux MIFF files and Maya's standard IFF format for output
#define OUTPUT_IFF_FILES

typedef unsigned char uchar_t;

#define kOnscreenFlag		"-o"
#define kOnscreenFlagLong	"-onscreen"

#define kFilenameFlag		"-f"
#define kFilenameFlagLong	"-filename"

#define kStartFlag			"-s"
#define kStartFlagLong		"-start"

#define kEndFlag			"-e"
#define kEndFlagLong		"-stop"

#define commandName			"blast"

///////////////////
// Offscreen buffer
///////////////////

class MyMPxGlBuffer : public MPxGlBuffer {
public: 
	MyMPxGlBuffer( M3dView &view );
	virtual ~MyMPxGlBuffer();
	
	virtual void beginBufferNotify();
	virtual void endBufferNotify();
};

MyMPxGlBuffer::MyMPxGlBuffer( M3dView &view ) : 
	MPxGlBuffer( view )
{
}

MyMPxGlBuffer::~MyMPxGlBuffer()
{
}
	
void MyMPxGlBuffer::beginBufferNotify()
{
	glClearColor( 0.0, 0.0, 0.0, 0.0 ); 
}
void MyMPxGlBuffer::endBufferNotify()
{
}

///////////////////////////////////////////////////
//
// Command class declaration
//
///////////////////////////////////////////////////
class blastCmd : public MPxCommand
{
public:
	blastCmd();
	virtual			~blastCmd(); 

	MStatus			doIt( const MArgList& args );

	static MSyntax	newSyntax();
	static void*	creator();

private:
	MStatus			fileDump( MTime );

	MStatus			parseArgs( const MArgList& args );
	bool			onscreen;
	MString			filename;
	MTime			start;
	MTime			end;

	MyMPxGlBuffer	*offBuff;

	short  	   	    fHeight;
	short  	   	    fWidth;
	int	   	   	    fPixelCnt;
	uchar_t 		*fPixels;
};

///////////////////////////////////////////////////
//
// Command class implementation
//
///////////////////////////////////////////////////

blastCmd::blastCmd()
{
	offBuff = NULL;
}

blastCmd::~blastCmd()
{
	// This should already be deleted but just to be safe
	//
	delete offBuff;
}

void* blastCmd::creator()
{
	return (void *)(new blastCmd);
}

MSyntax blastCmd::newSyntax()
{
	 MSyntax syntax;
	 syntax.addFlag(kOnscreenFlag, kOnscreenFlagLong);
	 syntax.addFlag(kFilenameFlag, kFilenameFlagLong, MSyntax::kString);
	 syntax.addFlag(kStartFlag, kStartFlagLong, MSyntax::kTime);
	 syntax.addFlag(kEndFlag, kEndFlagLong, MSyntax::kTime);
	 return syntax;
}

MStatus blastCmd::parseArgs( const MArgList& args )
{
	MStatus stat = MS::kSuccess;

	MArgDatabase	argData(syntax(), args);

	onscreen = argData.isFlagSet(kOnscreenFlag);
	start = 0.0;
	end = 1.0;

	if (argData.isFlagSet(kFilenameFlag))
	{
		stat = argData.getFlagArgument(kFilenameFlag, 0, filename);
	}
	else
	{
		filename = "blastOut";
	}

	if (argData.isFlagSet(kStartFlag))
	{
		argData.getFlagArgument(kStartFlag, 0, start);
	}

	if (argData.isFlagSet(kEndFlag))
	{
		argData.getFlagArgument(kEndFlag, 0, end);
	}

	return stat;
}

MStatus blastCmd::fileDump( MTime frame )
{
	MStatus stat = MS::kFailure;	// Status code

	MString suffixName(filename);

	suffixName += ".";
	suffixName += frame.value();

	// 
	//  Copy the pixels from the offscreen buffer into the MImage for export
	//
	
#ifdef OUTPUT_IFF_FILES
	char msgBuffer[256];

	//  Use the API to output a Maya IFF file
	
	MImage	iffOutput;

	if( iffOutput.create( fWidth, fHeight ) != MS::kSuccess )
	{
		cerr << "Failed to create output image\n";
		return MS::kFailure;
	}

	unsigned char *iffPixels = iffOutput.pixels();
	unsigned char *glPixels = fPixels;

	for ( int pixCtr = 0; pixCtr < fPixelCnt; pixCtr++ )
	{
		*iffPixels = *glPixels;	// R
		glPixels++;
		iffPixels++;

		*iffPixels = *glPixels; 	// G 
		glPixels++;
		iffPixels++;

		*iffPixels = *glPixels; 	// B
		glPixels++;
		iffPixels++;

		*iffPixels = *glPixels; 	// A
		glPixels++;
		iffPixels++;
	};

	//
	// Dump the image to the output file. You can specify a different 
	// file format by providing a type string to the output function, i.e.
	//
	// 		writeToFile(suffixName, "jpg" )
	// 		writeToFile(suffixName, "tif" )
	//

	if( iffOutput.writeToFile(suffixName) != MS::kSuccess)
	{
		// Oops, we failed (somehow), tell the user about it.
		//
		sprintf(msgBuffer, "Failed to output image to %s\n", suffixName.asChar());
		MGlobal::displayError(msgBuffer);

		stat = MS::kFailure;
	}
	else
	{
		// It worked.  Tell the user about our success.
		//
		sprintf(msgBuffer, "output from %s buffer to %s done.\n",
				(onscreen ? "on-screen" : "off-screen"),
				suffixName.asChar());
		MGlobal::displayInfo(msgBuffer);

		stat = MS::kSuccess;
	}
#else
	
	//  Use standard Linux tools to write/view the file

	// Dump out the image as an ImageMagik file, use the 'display' command in Linux
	// to view it

	ofstream    image( suffixName.asChar() );


	//
	//  ImageMagik header
	//

	image << "id=ImageMagick\n";
	image << "columns= " << fWidth << "\nrows=" << fHeight << "\n:\n";


	//
	//  The image needs to be flipped vertically for ImageMagik
	//
	for( int row = fHeight-1; row >= 0 ; row-- )
	{
		const  uchar_t  *rowpixels = fPixels + ( row * fWidth * 4);
		for( int col = 0; col < fWidth; col++ )
		{
			image << *rowpixels++;
			image << *rowpixels++;
			image << *rowpixels++;
			image << *rowpixels++; 
		}
	}

	image.close();

	// Use these commands on Linux to view images as they are generated
	//sprintf( msgBuffer, "display %s\n", suffixName.asChar() );
	//system(msgBuffer );
#endif

	return stat;
}


MStatus blastCmd::doIt( const MArgList& args )
//
// Description
//     This method performs the action of the command.
//
{
	char msgBuffer[256];

	MStatus stat = MS::kFailure;	// Status code

	stat = parseArgs ( args );
	if ( !stat )
	{
		sprintf( msgBuffer, "Failed to parse args for %s command\n", commandName );
		MGlobal::displayError( msgBuffer );

		return stat;
	}

	// Find then current 3dView.
	//
	M3dView view = M3dView::active3dView();

	// Set up the dimensions
	// 
	fWidth = (short)view.portWidth();
	fHeight = (short)view.portHeight();
	fPixelCnt = fWidth * fHeight;


	// Allocate a block of memory to hold the images, 4 per pixel (RGBA)
	//
	fPixels = new uchar_t[fPixelCnt * 4];


	// Make sure that we actually got the memory
	//
	if (!fPixels)
	{
		MGlobal::displayError("Failed to allocate memory for reading pixels\n");
		return MS::kFailure;
	}


	if( !onscreen )
	{
		// Create an MPxGLBuffer so that we can capture the 
		// screen render into a frame buffer object. 
		//
		offBuff = new MyMPxGlBuffer( view );

		// We must always supply the view that we will be rendering
		// into an offscreen frame buffer. 
		//
		if( ! offBuff->openFbo( fWidth, fHeight, view ) ) 
		{
			// The buffer failed to open.  Tell the user about it.
			//
			MGlobal::displayError("Failed to open offscreen buffer\n");

			// Delete the off-screen buffer and set the pointer to NULL
			// so it does not get deleted again.
			//
			delete offBuff;
			offBuff = NULL;

			return MS::kFailure;
		}
	}

	for ( MTime curTime = start; curTime <= end; curTime++ )
	{
		MAnimControl::setCurrentTime ( curTime );

		if( !onscreen )
		{
			//  Refresh the view to the off-screen buffer.  The arguments
			//  "all" and "force" to the normal refresh call are
			//  unnecessary because the refresh is always forced and is
			//  never to more than this single view.
			//
			view.refresh( *offBuff, true );

			offBuff->bindFbo();
		}
		else
		{
			// We are not using an off-screen buffer.  Simply refresh
			// the on-screen window.
			
			view.refresh( false /* all */, true /* force */ );
			glReadBuffer(GL_FRONT);
		}
	
		// Tell the view that we want to use raw OpenGL calls ...
		//
		view.beginGL();

		// ... read the pixels ...
		//
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(0, 0, fWidth, fHeight, GL_RGBA, GL_UNSIGNED_BYTE, fPixels);

		// ... and tell the view that we are done with raw OpenGL.
		view.endGL();

		if ( offBuff ) { 
			offBuff->unbindFbo();
		}
		// Output the pixels to disk	
		fileDump( curTime );
	}

	// Free up our allocated memory.
	//
	if ( offBuff ) { 
		offBuff->closeFbo( view );
		delete offBuff;
		offBuff = NULL;
	}

	delete [] fPixels;

	return stat;
}



/////////////////////////////////////////////////////////////////
//
// The following routines are used to register/unregister
// the command we are creating within Maya
//
/////////////////////////////////////////////////////////////////
MStatus initializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "6.0", "Any");

	status = plugin.registerCommand( commandName,
									  blastCmd::creator,
									  blastCmd::newSyntax);
	if (!status) {
		status.perror("registerCommand");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj )
{
	MStatus	  status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterCommand( commandName );
	if (!status) {
		status.perror("deregisterCommand");
		return status;
	}

	return status;
}
