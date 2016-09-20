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
// blast2Cmd.cpp
//
// This plugin is an example of capturing frames from VP2
//
////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <maya/MObject.h>
#include <maya/MFStream.h>

#include <maya/M3dView.h>

#include <maya/MFnPlugin.h>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>

#include <maya/MAnimControl.h>

#include <maya/MGlobal.h>
#include <maya/MImage.h>
#include <maya/MIOStream.h>

#include <maya/MViewport2Renderer.h>
#include <maya/MDrawContext.h>
#include <maya/MRenderTargetManager.h>
#include <maya/MRenderUtilities.h>

/*
	Command arguments and command name
*/
#define kFilenameFlag		"-f"
#define kFilenameFlagLong	"-filename"

#define kStartFrameFlag		"-sf"
#define kStartFrameFlagLong	"-startFrame"

#define kEndFrameFlag		"-ef"
#define kEndFrameFlagLong	"-endFrame"

#define kImageSizeFlag		"-is"
#define kImageSizeFlagLong	"-imageSize"

#define commandName			"blast2"

/*
	Command class declaration
*/
class blast2Cmd : public MPxCommand
{
public:
	blast2Cmd();
	virtual			~blast2Cmd(); 

	MStatus			doIt( const MArgList& args );
	static MSyntax	newSyntax();
	static void*	creator();
private:
	MStatus			parseArgs( const MArgList& args );

	// Capture options
	MString			mFilename;	// File name
	MTime			mStart;		// Start time
	MTime			mEnd;		// End time

	// Temporary to keep track of current time being captured
	MTime			mCurrentTime;

	// Override width and height
	unsigned int	mWidth;
	unsigned int	mHeight;

	// VP2 capture notification information
	MString			mPostRenderNotificationName;
	MString			mPostRenderNotificationSemantic;
	static void		captureCallback(MHWRender::MDrawContext &context, void* clientData);

	// This code is not required for the logic of this command put put in for
	// completeness to show the additional possible callbacks. Will only be set up
	// if the debug flag mDebugTraceNotifications is set to true. It is set to false by default.
	/* The debug output could look something like this for a 2 pass render using the printPassInformation() utility:

		Pass Identifier = blast2CmdPreRender
		Pass semantic: colorPass
		Pass semantic: beginRender
	
		Pass Identifier = blast2CmdPreSceneRender
		Pass semantic: colorPass
		Pass semantic: beginSceneRender
		Pass Identifier = blast2CmdPostSceneRender
		Pass semantic: colorPass
		Pass semantic: endSceneRender
	
		Pass Identifier = blast2CmdPreSceneRender
		Pass semantic: colorPass
		Pass semantic: beginSceneRender
		Pass Identifier = blast2CmdPostSceneRender
		Pass semantic: colorPass
		Pass semantic: endSceneRender
	
		Pass Identifier = blas2CmdPostRender
		Pass semantic: endRender
	*/
	bool			mDebugTraceNotifications;
	MString mPreRenderNotificationName;
	MString mPreRenderNotificationSemantic;
	static void		preFrameCallback(MHWRender::MDrawContext &context, void* clientData);
	MString mPreSceneRenderNotificationName;
	MString mPreSceneRenderNotificationSemantic;
	static void		preSceneCallback(MHWRender::MDrawContext &context, void* clientData);
	MString mPostSceneRenderNotificationName;
	MString mPostSceneRenderNotificationSemantic;
	static void		postSceneCallback(MHWRender::MDrawContext &context, void* clientData);

	// Utility to print out pass information at notification time
	void printPassInformation(MHWRender::MDrawContext &context);
};

blast2Cmd::blast2Cmd()
: mPreRenderNotificationName("blast2CmdPreRender")
, mPreRenderNotificationSemantic(MHWRender::MPassContext::kBeginRenderSemantic)

, mPostRenderNotificationName("blas2CmdPostRender")
, mPostRenderNotificationSemantic(MHWRender::MPassContext::kEndRenderSemantic)

, mPreSceneRenderNotificationName("blast2CmdPreSceneRender")
, mPreSceneRenderNotificationSemantic(MHWRender::MPassContext::kBeginSceneRenderSemantic)

, mPostSceneRenderNotificationName("blast2CmdPostSceneRender")
, mPostSceneRenderNotificationSemantic(MHWRender::MPassContext::kEndSceneRenderSemantic)
, mWidth(0)
, mHeight(0)
, mDebugTraceNotifications(false) /* Set to true to enable additional notifications + pass information debug output */
{
}

blast2Cmd::~blast2Cmd()
{
}

void* blast2Cmd::creator()
{
	return (void *)(new blast2Cmd);
}

/*
	Add flags to the command syntax
*/
MSyntax blast2Cmd::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag(kFilenameFlag, kFilenameFlagLong, MSyntax::kString);
	syntax.addFlag(kStartFrameFlag, kStartFrameFlagLong, MSyntax::kTime);
	syntax.addFlag(kEndFrameFlag, kEndFrameFlagLong, MSyntax::kTime);
	syntax.addFlag(kImageSizeFlag, kImageSizeFlagLong, MSyntax::kUnsigned, MSyntax::kUnsigned);
	return syntax;
}

/*
	Parse command line arguments:
	1) Filename (required)
	2) Start time. Defaults to 0
	3) End time. Defaults to 1
*/
MStatus blast2Cmd::parseArgs( const MArgList& args )
{
	MStatus status = MStatus::kSuccess;
	MArgDatabase argData(syntax(), args);

	mStart = 0.0;
	mEnd = 1.0;
	mWidth = 0;
	mHeight = 0;

	if (argData.isFlagSet(kFilenameFlag))
	{
		status = argData.getFlagArgument(kFilenameFlag, 0, mFilename);
	}
	else
	{
		return MStatus::kFailure;
	}
	if (argData.isFlagSet(kStartFrameFlag))
	{
		argData.getFlagArgument(kStartFrameFlag, 0, mStart);
	}
	if (argData.isFlagSet(kEndFrameFlag))
	{
		argData.getFlagArgument(kEndFrameFlag, 0, mEnd);
	}
	if (argData.isFlagSet(kImageSizeFlag))
	{
		argData.getFlagArgument(kImageSizeFlag, 0, mWidth);
		argData.getFlagArgument(kImageSizeFlag, 1, mHeight);
	}
	return status;
}

// Print out the pass identifier and pass semantics 
void blast2Cmd::printPassInformation(MHWRender::MDrawContext &context)
{
	if (!mDebugTraceNotifications)
		return;

	const MHWRender::MPassContext & passCtx = context.getPassContext();
	const MString & passId = passCtx.passIdentifier();
	const MStringArray & passSem = passCtx.passSemantics();

	printf("\tPass Identifier = %s\n", passId.asChar());
	for (unsigned int i=0; i<passSem.length(); i++)
	{
		printf("\tPass semantic: %s\n", passSem[i].asChar()); 
	}
}

void blast2Cmd::preSceneCallback(MHWRender::MDrawContext &context, void* clientData)
{
	// Get back command
	blast2Cmd *cmd = (blast2Cmd *)clientData;
	if (!cmd)
		return;
	cmd->printPassInformation(context);
}

void blast2Cmd::postSceneCallback(MHWRender::MDrawContext &context, void* clientData)
{
	// Get back command
	blast2Cmd *cmd = (blast2Cmd *)clientData;
	if (!cmd)
		return;
	cmd->printPassInformation(context);
}

void blast2Cmd::preFrameCallback(MHWRender::MDrawContext &context, void* clientData)
{
	// Get back command
	blast2Cmd *cmd = (blast2Cmd *)clientData;
	if (!cmd)
		return;
	cmd->printPassInformation(context);
}

/*
	Callback which is called at end of render to perform the capture.
	Client data contains a reference back to the command to
	allow for the capture options to be read.
*/
void blast2Cmd::captureCallback(MHWRender::MDrawContext &context, void* clientData)
{
	// Get back command
	blast2Cmd *cmd = (blast2Cmd *)clientData;
	if (!cmd)
		return;

	cmd->printPassInformation(context);

	MHWRender::MRenderer *renderer = MHWRender::MRenderer::theRenderer();
	if (!renderer)
		return;

	// Create a final frame name of:
	// <filename>.<framenumber>.<frameExtension>
	// In this example we always write out iff files.
	//
	MString frameName(cmd->mFilename);
	frameName += ".";
	frameName += cmd->mCurrentTime.value();

	bool saved = false;

	/*	The following is one example of how to retrieve pixels and store them to disk

		The most flexible way is to get access to the raw data using MRenderTarget::rawData(), perform
		any custom saving as desired, and then use MRenderTarget::freeRawData().

		Note that context.getCurrentDepthRenderTarget() can be used to access the depth buffer
	*/
	const MHWRender::MRenderTarget* colorTarget = context.getCurrentColorRenderTarget();
	if (colorTarget)
	{
		/*	Query for the target format. If it is floating point then we switch ti EXR so that we
			we save properly.
		*/
		MString frameExtension;
		MHWRender::MRenderTargetDescription desc;
		colorTarget->targetDescription(desc);
		MHWRender::MRasterFormat format = desc.rasterFormat();
		switch (format)
		{
			case MHWRender::kR32G32B32_FLOAT:
			case MHWRender::kR16G16B16A16_FLOAT:
			case MHWRender::kR32G32B32A32_FLOAT:
				frameExtension = ".exr";
				break;
			case MHWRender::kR8G8B8A8_UNORM:
			case MHWRender::kB8G8R8A8:
			case MHWRender::kA8B8G8R8:
				frameExtension = ".iff";
				break;
			default:
				frameExtension = "";
				break;
		};
		if (frameExtension.length() == 0)
			return;
		frameName += frameExtension;

		/*	Get a copy of the render target. We get it back as a texture to allow to
			use the "save texture" method on the texture manager for this example.
		*/
		MHWRender::MTextureManager* textureManager = renderer->getTextureManager();
		MHWRender::MTexture* colorTexture = context.copyCurrentColorRenderTargetToTexture();
		if (colorTexture)
		{
			// Save the texture to disk
			MStatus status = textureManager->saveTexture(colorTexture, frameName);
			saved = (status == MStatus::kSuccess);

			// When finished with the texture, release it.
			textureManager->releaseTexture(colorTexture);
		}

		// Release reference to the color target
		const MHWRender::MRenderTargetManager* targetManager = renderer->getRenderTargetManager();
		targetManager->releaseRenderTarget((MHWRender::MRenderTarget*)colorTarget);
	}

	// Output some status information about the save
	//
	char msgBuffer[256];
	if (!saved)
	{
		sprintf(msgBuffer, "Failed to color render target to %s.", frameName.asChar());
		MGlobal::displayError(msgBuffer);
	}
	else
	{
		sprintf(msgBuffer, "Captured color render target to %s.",
				frameName.asChar());
		MGlobal::displayInfo(msgBuffer);
	}
}

/*
	Perform the blast command on the current 3d view
	
	1) Setting up a post render callback on VP2.
	2) iterating from start to end time.
	3) During the callback writing the current VP2 render target to disk	
*/
MStatus blast2Cmd::doIt( const MArgList& args )
{
	MStatus status = MStatus::kFailure;

	MHWRender::MRenderer *renderer = MHWRender::MRenderer::theRenderer();
	if (!renderer)
	{
		MGlobal::displayError( "VP2 renderer not initialized.");
		return status;
	}

	status = parseArgs ( args );
	if ( !status )
	{
		char msgBuffer[256];
		sprintf( msgBuffer, "Failed to parse args for %s command.\n", commandName );
		MGlobal::displayError( msgBuffer );
		return status;
	}

	// Find then current 3dView.
	//
	M3dView view = M3dView::active3dView(&status);
	if ( !status )
	{
		MGlobal::displayError( "Faled to find an active 3d view to capture." );
		return status;
	}

	// Set up notification of end of render. Send the blast command
	// to allow accessing data members.
	//
	renderer->addNotification(captureCallback, mPostRenderNotificationName, mPostRenderNotificationSemantic,
					(void *)this );

	// Sample code to show additional notification usage
	if (mDebugTraceNotifications)
	{
		renderer->addNotification(preFrameCallback, mPreRenderNotificationName, mPreRenderNotificationSemantic, (void *)this);
		renderer->addNotification(preSceneCallback, mPreSceneRenderNotificationName, mPreSceneRenderNotificationSemantic, (void *)this);
		renderer->addNotification(postSceneCallback, mPostSceneRenderNotificationName, mPostSceneRenderNotificationSemantic, (void *)this );
	}

	// Check for override image size.
	bool setOverride = (mWidth > 0 && mHeight > 0);
	if (setOverride)
	{
		renderer->setOutputTargetOverrideSize( mWidth, mHeight );
	}
	// Temporarily turn off on-screen updates
	renderer->setPresentOnScreen(false);

	for ( mCurrentTime = mStart; mCurrentTime <= mEnd; mCurrentTime++ )
	{
		MAnimControl::setCurrentTime( mCurrentTime );			
		view.refresh( false /* all */, true /* force */ );
	}

	/// Remove notification of end of render
	renderer->removeNotification(mPostRenderNotificationName, mPostRenderNotificationSemantic);

	if (mDebugTraceNotifications)
	{
		renderer->removeNotification(mPreRenderNotificationName, mPreRenderNotificationSemantic);
		renderer->removeNotification(mPreSceneRenderNotificationName, mPreSceneRenderNotificationSemantic);
		renderer->removeNotification(mPostSceneRenderNotificationName, mPostSceneRenderNotificationSemantic);
	}

	// Restore off on-screen updates
	renderer->setPresentOnScreen(true);
	// Disable target size override
	renderer->unsetOutputTargetOverrideSize();

	return status;
}


//////////////////////////////////////////////////////////////////////////////////////
/*!
	Register the command
*/
MStatus initializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "1.0", "Any");

	status = plugin.registerCommand( commandName,
									  blast2Cmd::creator,
									  blast2Cmd::newSyntax);
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
