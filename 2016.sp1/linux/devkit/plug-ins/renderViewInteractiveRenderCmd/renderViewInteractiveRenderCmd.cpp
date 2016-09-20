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

#include <maya/MSimple.h>
#include <maya/MIOStream.h>
#include <maya/MRenderView.h>
#include <maya/M3dView.h>
#include <math.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MGlobal.h>
#include <maya/MDGMessage.h>
#include <maya/MTime.h>
#include <maya/MAnimControl.h>

#include <stdlib.h>

//
//	renderViewInteractiveRender command declaration
//
class renderViewInteractiveRender : public MPxCommand 
{							
public:					
	renderViewInteractiveRender() {};
	~renderViewInteractiveRender() {};

	virtual MStatus    doIt( const MArgList& );
	
	static void*       creator();
	
	static MSyntax     newSyntax();
	MStatus            parseSyntax (MArgDatabase &argData);

    // methods to support render view updates
    static MStatus     updateRenderView();
    static MStatus     updateRenderViewDefault();

    // methods to support IPR rendering
	static void        timeChangeCB(MTime &time, void *clientData);
    static MStatus     updateRenderViewIpr();

	static const char  *cmdName;

private:
    static RV_PIXEL    evaluate(int x, int y);
};

namespace {
    bool         fullRefresh;
    bool         immediateRefresh;
    bool         doNotClearBackground        = true;
    bool         verbose;
    bool         editMode                    = false;
    bool         useRandomColors             = false;
    double       radius;
    unsigned int size[2];
    unsigned int tileSize[2];
    unsigned int numberLoops;
    RV_PIXEL     color1;
    RV_PIXEL     color2;
    bool         iprPaused                   = false;
    bool         iprMode                     = false;
    unsigned int width                       = 640;
    unsigned int height                      = 480;
    unsigned int left                        = 0;
    unsigned int right                       = width;
    unsigned int top                         = height;
    unsigned int bottom                      = 0;
    MCallbackId  timeChangeCallbackID;

    const char *kVerbose                    = "-v";
    const char *kVerboseLong                = "-verbose";
    const char *kDoNotClearBackground       = "-b";
    const char *kDoNotClearBackgroundLong   = "-background";
    const char *kRadius                     = "-r";
    const char *kRadiusLong                 = "-radius";
    const char *kSizeX                      = "-sx";
    const char *kSizeXLong                  = "-sizeX";
    const char *kSizeY                      = "-sy";
    const char *kSizeYLong                  = "-sizeY";
    const char *kSizeTileX                  = "-tx";
    const char *kSizeTileXLong              = "-sizeTileX";
    const char *kSizeTileY                  = "-ty";
    const char *kSizeTileYLong              = "-sizeTileY";
    const char *kNumberLoops                = "-nl";
    const char *kNumberLoopsLong            = "-numberLoops";
    const char *kImmediateRefresh           = "-ir";
    const char *kImmediateRefreshLong       = "-immediateRefresh";
    const char *kFullRefresh                = "-fr";
    const char *kFullRefreshLong            = "-fullRefresh";
    const char *kIprMode                    = "-ipr";
    const char *kIprModeLong                = "-iprMode";
    const char *kPause                      = "-p";
    const char *kPauseLong                  = "-pause";
    const char *kLeft                       = "-lft";
    const char *kLeftLong                   = "-leftEdge";
    const char *kRight                      = "-rgt";
    const char *kRightLong			        = "-rightEdge";
    const char *kTop                        = "-tp";
    const char *kTopLong                    = "-topEdge";
    const char *kBottom                     = "-bot";
    const char *kBottomLong                 = "-bottomEdge";
    const char *kUseRandomColors            = "-rc";
    const char *kUseRandomColorsLong        = "-useRandomColors";
    const char *kEditMode                   = "-e";
    const char *kEditModeLong               = "-editMode";
};

const char * renderViewInteractiveRender::cmdName = "renderViewInteractiveRender";

void* renderViewInteractiveRender::creator()					
{
	return new renderViewInteractiveRender;
}													

MSyntax renderViewInteractiveRender::newSyntax()
{
	MStatus status;
	MSyntax syntax;

	syntax.addFlag(kDoNotClearBackground, kDoNotClearBackgroundLong);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

    syntax.addFlag(kEditMode, kEditModeLong);
    CHECK_MSTATUS_AND_RETURN(status, syntax);

	syntax.addFlag(kVerbose, kVerboseLong);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

	syntax.addFlag(kImmediateRefresh, kImmediateRefreshLong);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

	syntax.addFlag(kFullRefresh, kFullRefreshLong);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

	syntax.addFlag(kRadius, kRadiusLong, MSyntax::kDouble);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

	syntax.addFlag(kSizeX, kSizeXLong, MSyntax::kLong);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

	syntax.addFlag(kSizeY, kSizeYLong, MSyntax::kLong);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

	syntax.addFlag(kSizeTileX, kSizeTileXLong, MSyntax::kLong);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

	syntax.addFlag(kSizeTileY, kSizeTileYLong, MSyntax::kLong);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

	syntax.addFlag(kNumberLoops, kNumberLoopsLong, MSyntax::kLong);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

	syntax.addFlag(kPause, kPauseLong, MSyntax::kBoolean);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

	syntax.addFlag(kIprMode, kIprModeLong, MSyntax::kBoolean);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

	syntax.addFlag(kLeft, kLeftLong, MSyntax::kDouble);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

	syntax.addFlag(kRight, kRightLong, MSyntax::kDouble);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

	syntax.addFlag(kTop, kTopLong, MSyntax::kDouble);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

	syntax.addFlag(kBottom, kBottomLong, MSyntax::kDouble);
	CHECK_MSTATUS_AND_RETURN(status, syntax);

    syntax.addFlag(kUseRandomColors, kUseRandomColorsLong, MSyntax::kBoolean);
    CHECK_MSTATUS_AND_RETURN(status, syntax);
	return syntax;
}

//
// Description:
//		Read the values of the additional flags for this command.
//
MStatus renderViewInteractiveRender::parseSyntax (MArgDatabase &argData)
{
	// Get the flag values, otherwise the default values are used.
	doNotClearBackground = argData.isFlagSet( kDoNotClearBackground );
	verbose = argData.isFlagSet( kVerbose );
	fullRefresh = argData.isFlagSet( kFullRefresh );
	immediateRefresh = argData.isFlagSet( kImmediateRefresh );

    editMode = false;
    editMode = argData.isFlagSet( kEditMode );

	radius = 50.;							// pattern frequency, in pixels
	if (argData.isFlagSet( kRadius ))
		argData.getFlagArgument(kRadius, 0, radius);

	size[0] = width;
	size[1] = height;
	if (argData.isFlagSet( kSizeX ))
		argData.getFlagArgument(kSizeX, 0, size[0]);
	if (argData.isFlagSet( kSizeY ))
		argData.getFlagArgument(kSizeY, 0, size[1]);

	tileSize[0] = 16;
	tileSize[1] = 16;
	if (argData.isFlagSet( kSizeTileX ))
		argData.getFlagArgument(kSizeTileX, 0, tileSize[0]);
	if (argData.isFlagSet( kSizeTileY ))
		argData.getFlagArgument(kSizeTileY, 0, tileSize[1]);

	numberLoops = 10;
	if (argData.isFlagSet( kNumberLoops ))
		argData.getFlagArgument(kNumberLoops, 0, numberLoops);

    if (argData.isFlagSet( kPause ))
        argData.getFlagArgument(kPauseLong, 0, iprPaused);

    if (argData.isFlagSet( kIprMode ))
        argData.getFlagArgument(kIprModeLong, 0, iprMode);

    if (argData.isFlagSet( kLeft ))
        argData.getFlagArgument(kLeftLong, 0, left);

    if (argData.isFlagSet( kRight ))
        argData.getFlagArgument(kRightLong, 0, right);

    if (argData.isFlagSet( kTop ))
        argData.getFlagArgument(kTopLong, 0, top);

    if (argData.isFlagSet( kBottom ))
        argData.getFlagArgument(kBottomLong, 0, bottom);

    if (argData.isFlagSet( kUseRandomColors ))
        argData.getFlagArgument(kUseRandomColorsLong, 0, useRandomColors);

    return MS::kSuccess;
}

//
// Description:
//		If during an IPR session, perform an update of the Render View Window
//
void renderViewInteractiveRender::timeChangeCB(MTime &time, void *clientData)
{
    if(iprMode && !iprPaused)
    {
        // Avoid flickering if someone scrubs the timeline
        doNotClearBackground = true;
        renderViewInteractiveRender::updateRenderView();
    }
}

//
// Description:
//		register the command
//
MStatus initializePlugin( MObject obj )			
{															
	MFnPlugin	plugin( obj, PLUGIN_COMPANY, "4.5" );	
	MStatus		stat;										
	stat = plugin.registerCommand(	renderViewInteractiveRender::cmdName,
									renderViewInteractiveRender::creator,
									renderViewInteractiveRender::newSyntax);	
	if ( !stat )												
		stat.perror( "registerCommand" );

    plugin.registerUI("registerSampleRenderer", "");

    timeChangeCallbackID = MDGMessage::addTimeChangeCallback(renderViewInteractiveRender::timeChangeCB, NULL, &stat);

    if( !stat )
        timeChangeCallbackID = 0;

    return stat;												
}																

//
// Description:
//		unregister the command
//
MStatus uninitializePlugin( MObject obj )						
{																
	MFnPlugin	plugin( obj );									
	MStatus		stat;											
	stat = plugin.deregisterCommand( renderViewInteractiveRender::cmdName );
	if ( !stat )									
		stat.perror( "deregisterCommand" );		

    stat = MDGMessage::removeCallback(timeChangeCallbackID);
    if ( !stat )									
        stat.perror( "removeCallback" );

    return stat;									
}

RV_PIXEL renderViewInteractiveRender::evaluate(int x, int y)
//
//	Description:
//		Generates a simple procedural circular pattern to be sent to the 
//		Render View.
//
//	Arguments:
//		x, y - coordinates in the current tile (the pattern is centered 
//			   around (0,0) ).
//
//	Return Value:
//		An RV_PIXEL structure containing the colour of pixel (x,y).
//
{
	double distance = sqrt(double((x*x) + (y*y))) / radius;
	float percent = (float)(cos(distance*2*3.1415927)/2.+.5);

	RV_PIXEL pixel;
	pixel.r = color1.r * percent + color2.r * (1-percent);
	pixel.g = color1.g * percent + color2.g * (1-percent);
	pixel.b = color1.b * percent + color2.b * (1-percent);
	pixel.a = 255.0f;

	return pixel;
}

//
// Description:
//		update the Render View Window
//
MStatus renderViewInteractiveRender::updateRenderView()
{
    MStatus stat = MS::kSuccess;

    // Check if the render view exists. It should always exist, unless
    // Maya is running in batch mode.
    //
    if (!MRenderView::doesRenderEditorExist())
    {
        displayError( 
            "Cannot renderViewInteractiveRender in batch render mode.\n"
            "Run in interactive mode, so that the render editor exists." );
        return MS::kFailure;
    };

    if(iprMode)
    {
        stat = updateRenderViewIpr();
    }
    else
    {
        stat = updateRenderViewDefault();
    }

    // Inform the Render View that we have completed rendering the entire image.
    //
    if (MRenderView::endRender() != MS::kSuccess)
    {
        displayError( "renderViewInteractiveRender: error occurred in endRender." );
        return MS::kFailure;
    }

    return stat;
}

//
// Description:
//		update the Render View Window
//
MStatus renderViewInteractiveRender::updateRenderViewDefault()
{
    unsigned int image_width = size[0], image_height = size[1];

    if (MRenderView::startRender( image_width, image_height, 
        doNotClearBackground, 
        immediateRefresh) != MS::kSuccess)
    {
        displayError("renderViewInteractiveRender: error occurred in startRender.");
        return MS::kFailure;
    }

    // The image will be composed of tiles consisting of circular patterns.
    //

    // Draw each tile
    //
    static float colors[] = { 255, 150,  69, 
                              255,  84, 112,
                              255,  94, 249,
                               86,  62, 255,
                               46, 195, 255,
                               56, 255, 159,
                              130, 255,  64 };
    int indx1 = 0;
    int indx2 = 3*3;

    RV_PIXEL* pixels = new RV_PIXEL[tileSize[0] * tileSize[1]];
    for (unsigned int loopId = 0 ; loopId < numberLoops ; loopId++ )
    {
        color1.r = colors[indx1]; 
        color1.g = colors[indx1+1]; 
        color1.b = colors[indx1+2];
        color1.a = 255;
        indx1 += 3; if (indx1 >= 21) indx1 -= 21;

        color2.r = colors[indx2]; 
        color2.g = colors[indx2+1]; 
        color2.b = colors[indx2+2]; 
        color2.a = 255;
        indx2 += 6; if (indx2 >= 21) indx2 -= 21;

        for (unsigned int min_y = 0; min_y < size[1] ; min_y += tileSize[1] )
        {
            unsigned int max_y = min_y + tileSize[1] - 1;
            if (max_y >= size[1]) max_y = size[1]-1;

            for (unsigned int min_x = 0; min_x < size[0] ; min_x += tileSize[0] )
            {
                unsigned int max_x = min_x + tileSize[0] - 1;
                if (max_x >= size[0]) max_x = size[0]-1;

                // Fill up the pixel array with some the pattern, which is 
                // generated by the 'evaluate' function.  The Render View
                // accepts floating point pixel values only.
                //
                unsigned int index = 0;
                for (unsigned int j = min_y; j <= max_y; j++ )
                {
                    for (unsigned int i = min_x; i <= max_x; i++)
                    {
                        pixels[index] = evaluate(i, j);
                        index++;
                    }
                }

                // Send the data to the render view.
                //
                if (MRenderView::startRegionRender( width, height, left, right-1, bottom, top-1, true, false) != MS::kSuccess)
                {
                    displayError( "renderViewInteractiveRender: error occurred in updatePixels." );
                    delete [] pixels;
                    return MS::kFailure;
                }

                // Force the Render View to refresh the display of the
                // affected region.
                //
                MStatus st;
                if (fullRefresh)
                    st =MRenderView::refresh(0, image_width-1, 0, image_height-1);
                else
                    st = MRenderView::refresh(min_x, max_x, min_y, max_y);
                if (st != MS::kSuccess)
                {
                    displayError( "renderViewInteractiveRender: error occurred in refresh." );
                    delete [] pixels;
                    return MS::kFailure;
                }

                if (verbose)
                    cerr << "Tile "<<min_x<<", "<<min_y<<
                    " (iteration "<<loopId<<")completed\n";
            }
        }
    }

    delete [] pixels;
    return MS::kSuccess;
}

//
// Description:
//		update the Render View Window for and IPR session
//
MStatus renderViewInteractiveRender::updateRenderViewIpr()
{
    unsigned int image_width  = right - left;
    unsigned int image_height = top - bottom;

    if (MRenderView::startRender( width, height, doNotClearBackground, true) != MS::kSuccess)
    {
        displayError("renderViewInteractiveRender: error occurred in startRender.");
        return MS::kFailure;
    }

    RV_PIXEL* pixels = new RV_PIXEL[image_width * image_height];

    RV_PIXEL pixel;
    if(useRandomColors)
    {
        pixel.r = (float)(rand() % 255);
        pixel.g = (float)(rand() % 255);
        pixel.b = (float)(rand() % 255);
        pixel.a = 255.0f;
    }else
    {
        MTime currentTimeObj = MAnimControl::currentTime();
        double currentTime = currentTimeObj.value();
        // Arbitrary function to create non gray colors for the
        // purpose of illustration
        pixel.r = (float)((sin(currentTime * 0.05) + 1.0) * 127.5);
        pixel.g = (float)((sin(currentTime * 0.10) + 1.0) * 127.5);
        pixel.b = (float)((sin(currentTime * 0.20) + 1.0) * 127.5);
        pixel.a = 255.0f;
    }

    // Fill buffer with uniform color
    for (unsigned int index = 0; index < image_height * image_width; ++index )
    {
        pixels[index] = pixel;
    }

    // Pushing buffer to Render View
    if (MRenderView::updatePixels(left, right - 1, bottom, top - 1, pixels) 
        != MS::kSuccess)
    {
        displayError( "renderViewInteractiveRender: error occurred in updatePixels." );
        delete[] pixels;
        return MS::kFailure;
    }

    delete[] pixels;
    return MS::kSuccess;

}

MStatus renderViewInteractiveRender::doIt( const MArgList& args )
//
//	Description:
//		Implements the MEL renderViewInteractiveRender command. This command
//		Draws a 640x480 tiled pattern of circles into Maya's Render
//		View window.
//
//	Arguments:
//		args - The argument list that was passed to the command from MEL.
//
//	Return Value:
//		MS::kSuccess - command succeeded
//		MS::kFailure - command failed (returning this value will cause the 
//                     MEL script that is being run to terminate unless the
//                     error is caught using a "catch" statement.
//
{
    MStatus stat = MS::kSuccess;

    // get optional flags
    MArgDatabase argData( syntax(), args );
    parseSyntax( argData );

    // When running the command in editMode, we are only
    // interested in updating the arguments passed to the plugin
    // i.e. we don't want to render anything
    if(!editMode)
    {
        renderViewInteractiveRender::updateRenderView();
    }

    return stat;
}

