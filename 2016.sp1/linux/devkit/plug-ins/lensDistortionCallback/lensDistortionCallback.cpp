#include "lensDistortionCallback.h"

#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MDagPath.h>
#include <maya/MMatrix.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MImage.h>
#include <maya/MIOStream.h>
#include <maya/MFnCamera.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MColor.h>
#include <maya/MFnPlugin.h>
#include <maya/MHardwareRenderer.h>
#include <maya/MGLdefinitions.h>

#define kFloatEpsilon		1.0e-10
#define kRemoveFlag			"-r"
#define kRemoveFlagLong		"-remove"
#define kExistFlag			"-ex"
#define kExistFlagLong		"-exists"
#define kListFlag			"-l"
#define kListFlagLong		"-list"

lensDistortionCompute* lensDistortionCompute::currentLensDistortionCompute[MAX_MODEL_PANEL];

// ------------------------------------------------------------
// lensDistortionCallback implementation
// ------------------------------------------------------------

lensDistortionCallback::lensDistortionCallback()
:
	mRemoveOperation( false )
,	mExistOperation( false )
,	mListOperation( false )
{
	mPanelName = MString("");
}

lensDistortionCallback::~lensDistortionCallback()
{
}

void* lensDistortionCallback::creator()
{
	return (void *) (new lensDistortionCallback);
}

MSyntax lensDistortionCallback::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( kRemoveFlag,	kRemoveFlagLong,	MSyntax::kString);
	syntax.addFlag( kExistFlag,		kExistFlagLong,		MSyntax::kString);
	syntax.addFlag( kListFlag,		kListFlagLong );
	syntax.addArg(MSyntax::kString);
	return syntax;
}

MStatus lensDistortionCallback::parseArgs(const MArgList& args)
{
	MStatus			status;
	MArgDatabase	argDatabase( syntax(), args, &status);

	if ( status != MS::kSuccess ) return status;

	if ( args.length() == 1 )
	{
		MString thisArg = args.asString( 0, &status );
		if ( thisArg == kListFlag || thisArg == kListFlagLong )
		{
			mListOperation = true;
		}
		else
		{
			// Adding callback to given panel 
			mRemoveOperation = false;
			mExistOperation	 = false;
			mPanelName = argDatabase.commandArgumentString( 0, &status );
		}
	}
	else if ( args.length() == 2 )
	{
		MString thisArg = args.asString( 0, &status );
		if ( thisArg == kRemoveFlag || thisArg == kRemoveFlagLong )
		{
			// Removing callback from given panel 
			mRemoveOperation = true;
			mPanelName = argDatabase.flagArgumentString( kRemoveFlag, 0, &status );
		}
		else if ( thisArg == kExistFlag || thisArg == kExistFlagLong )
		{
			mExistOperation = true;
			mPanelName = argDatabase.flagArgumentString( kExistFlag, 0, &status );
		}
	}
	else
	{
		return MStatus::kFailure;
	}
	return MS::kSuccess;
}

MStatus lensDistortionCallback::doIt(const MArgList& args)
{
	MStatus status;

	// Parse argument
	status = parseArgs(args);

	if (status != MS::kSuccess){	
		displayError( "Argument should have a panel name and proper option" );
		return status; 
	}

	// Process list operation
	if ( mListOperation )
	{
		MStringArray panelNames;
		status =  lensDistortionCompute::listCallback( panelNames );
		setResult ( panelNames );
		return status;
	}

	// Check if the given model panel name is valid or invalid.
	M3dView view;
	status = M3dView::getM3dViewFromModelPanel ( mPanelName,  view );

	if (status != MS::kSuccess){	
		displayError( "Specified model panel is not valid!" );
		return status; 
	}

	// Process add/remove/query existence operation
	if ( mExistOperation )
	{
		// Query existence of callback
		if ( lensDistortionCompute::panelHasCallback( mPanelName ) )
		{
			// Callback is found
			setResult( true );
		}
		else
		{
			// Callback is not found
			setResult( false );
		}
	}
	else if ( mRemoveOperation )
	{
		// Remove an instance of lensDistortionCompute
		status = lensDistortionCompute::removeCallbackFromPanel( mPanelName );
	}
	else  
	{
		// Add an instance of lensDistortionCompute
		status = lensDistortionCompute::addCallbackToPanel( mPanelName );
		if ( status == MS::kSuccess )
		{
			setResult( mPanelName );
		}
	}

	return status;
}

// ------------------------------------------------------------
// lensDistortionCompute implementation
// ------------------------------------------------------------

lensDistortionCompute::lensDistortionCompute(const MString &panelName)
:
	mTextureIndex( 0 )
,	mTextureWidth( 0 )
,	mTextureHeight( 0 )
,	mMultipleDrawPassCount( 1 )
{
	MStatus status;

	// Set panel name and operator for post rendering
	mPanelName = panelName;

	// Add the callbacks
	//
	mDeleteId
		= MUiMessage::add3dViewDestroyMsgCallback(panelName,
										   &lensDistortionCompute::deleteCB,
										   (void *) this, &status);
	if (mDeleteId == 0)
		MGlobal::displayError( MString("Could not attach view deletion callback to panel ") + panelName);

	mPreRenderId
		= MUiMessage::add3dViewPreRenderMsgCallback(panelName,
										  &lensDistortionCompute::preRenderCB,
										  (void *) this, &status);
	if (mPreRenderId == 0)
		MGlobal::displayError( MString("Could not attach view prerender callback to panel ") + panelName);

	mPostRenderId
		= MUiMessage::add3dViewPostRenderMsgCallback(panelName,
										   &lensDistortionCompute::postRenderCB,
										   (void *) this, &status);
	if (mPostRenderId == 0)
		MGlobal::displayError( MString("Could not attach view postrender callback to panel ") + panelName);

	mPreMultipleDrawPassId	
		= MUiMessage::add3dViewPreMultipleDrawPassMsgCallback(panelName,	
										&lensDistortionCompute::preMultipleDrawPassCB,	(void *) this, &status);
	if (mPreMultipleDrawPassId == 0)	
		MGlobal::displayError( MString("Could not attach view pre-multiple draw pass callback to panel ") + panelName);

	mPostMultipleDrawPassId	
		= MUiMessage::add3dViewPostMultipleDrawPassMsgCallback(panelName, 
										&lensDistortionCompute::postMultipleDrawPassCB,	(void *) this, &status);
	if (mPostMultipleDrawPassId == 0)	
		MGlobal::displayError( MString("Could not attach view post-multiple draw pass callback to panel ") + panelName);

}

lensDistortionCompute::~lensDistortionCompute()
{

	// Clear all callback that associated with this model panel.
	clearCallbacks();

	// Reset any global pointer pointing to this compute
	for (unsigned int i=0; i<MAX_MODEL_PANEL; i++)
	{
		if (currentLensDistortionCompute[i] && 
			(currentLensDistortionCompute[i])->getPanelName() == mPanelName)
		{
			currentLensDistortionCompute[i] = 0;
		}
	}

	// Delete GL texture if it is created.
	if ( mTextureIndex )
	{
		// Make rendering context current.
		MHardwareRenderer *pRenderer = MHardwareRenderer::theRenderer();
		if ( pRenderer )
		{
			const MString& backEnd = pRenderer->backEndString();
			pRenderer->makeResourceContextCurrent( backEnd );
			glDeleteTextures( 1, &mTextureIndex );
		}
		else
		{
			MGlobal::displayError( "Rendering context is not current! Memory leak will occurs.");
		}
	}
}


// Add an instance of lensDistortionCompute to given panal 
MStatus lensDistortionCompute::addCallbackToPanel( MString& pPanelName )
{
	MStatus status = MS::kFailure;
	// Check if the panel is already created.
	// Check if callback list has an empty entry.
	bool	found		= false;
	int		emptyIndex	= -1;

	for ( unsigned int i = 0; i < MAX_MODEL_PANEL; i++ )
	{
		if ( currentLensDistortionCompute[i] )
		{ 
			if ( currentLensDistortionCompute[i]->getPanelName() == pPanelName )
			{
				found = true;
			}
		}
		else
		{
			emptyIndex = i;
		}
	}

	if ( found )
	{
		// The panel already has a callback
		MGlobal::displayError("Specified model panel already has a callback!");
		status = MS::kFailure;
	}
	else if ( emptyIndex == -1 ) 
	{
		// Callback list doesn't have empty entry.
		// Any panel will not be added.
		MGlobal::displayError("Maximum number of callbacks are registered. Delete another callback");
		status = MS::kFailure;
	}
	else
	{
		// The panel doesn't has a callback and callback list has empty entry.
		// Add an instance of lensDistortionCompute.
		currentLensDistortionCompute[emptyIndex] = new lensDistortionCompute( pPanelName );
		status = MS::kSuccess;
	}

	return status;
}


// Remove an instance of lensDistortionCompute from panel
MStatus 
lensDistortionCompute::removeCallbackFromPanel( MString& pPanelName )
{

	// Check if the callback which associated with given panel is already created.
	for ( unsigned int i = 0; i < MAX_MODEL_PANEL; i++ )
	{
		if ( currentLensDistortionCompute[i] && 
			 currentLensDistortionCompute[i]->getPanelName() == pPanelName )
		{
			
			// Remove an instance of lensDistortionCompute
			delete currentLensDistortionCompute[i];
			currentLensDistortionCompute[i] = NULL;
			return MS::kSuccess;
		}
	}

	// The panel doesn't have a callback.
	MGlobal::displayError("Specified model panel doesn't have a callback!");
	return MS::kFailure;
}

// Query existence of instance of lensDistortionCompute for panel
bool 
lensDistortionCompute::panelHasCallback( MString& pPanelName )
{
	// Does this panel have the callback?
	for ( unsigned int i = 0; i < MAX_MODEL_PANEL; i++ )
	{
		if ( currentLensDistortionCompute[i] && 
			(currentLensDistortionCompute[i])->getPanelName() == pPanelName )
		{
			// Panel is found.
			return true;
		}
	}

	// Panel is not found
	return false;
}

// List names of the panels which have a callback attached
MStatus		
lensDistortionCompute::listCallback( MStringArray& pPanelNames )
{
	pPanelNames.clear();
	for ( unsigned int i = 0; i < MAX_MODEL_PANEL; i++ )
	{
		if ( currentLensDistortionCompute[i])
		{
			pPanelNames.append( (currentLensDistortionCompute[i])->getPanelName() );
		}
	}
	return MS::kSuccess;
}


void lensDistortionCompute::clearCallbacks()
{	
	if (mDeleteId)
		MMessage::removeCallback(mDeleteId);

	if (mPreRenderId)
		MMessage::removeCallback(mPreRenderId);

	if (mPostRenderId)
		MMessage::removeCallback(mPostRenderId);

	if (mPreMultipleDrawPassId)
		MMessage::removeCallback( mPreMultipleDrawPassId );

	if (mPostMultipleDrawPassId)
		MMessage::removeCallback( mPostMultipleDrawPassId );
}

// Called when associated model panel deleted
void lensDistortionCompute::deleteCB(const MString& panelName, void * data)
{
	lensDistortionCompute *thisCompute = (lensDistortionCompute *) data;

	// Check if this panel name is renamed.
	if ( panelName != thisCompute->mPanelName )
	{
		MGlobal::displayError("lensDistortionCallback does not support renaming of panels. Callback removed.");
	}
	// Delete callback.
	delete thisCompute;
}


void lensDistortionCompute::preRenderCB(const MString& panelName, void * data)
{
	// Get a pointer to current callback
	lensDistortionCompute *thisCompute = (lensDistortionCompute *) data;
	if (!thisCompute)
		return;

	MStatus status = M3dView::getM3dViewFromModelPanel(panelName, thisCompute->mCurrentView );
	if (status != MS::kSuccess) return;

	// Store current object display status and multiple draw pass count.
	thisCompute->mObjectDisplayState = thisCompute->mCurrentView.objectDisplay();
	thisCompute->mMultipleDrawPassCount = thisCompute->mCurrentView.multipleDrawPassCount();

	// Enable multi pass draw;
	thisCompute->mCurrentView.setMultipleDrawEnable( true );
	thisCompute->mCurrentView.setMultipleDrawPassCount( 2 );
}

void lensDistortionCompute::postRenderCB(const MString& panelName, void * data)
{	
	lensDistortionCompute *thisCompute = (lensDistortionCompute *) data;
	if (!thisCompute)
		return;

	// Draw 
	thisCompute->draw();

	// Disable multi pass draw.
	thisCompute->mCurrentView.setMultipleDrawEnable( false );

	// Restore object display state and multiple draw pass count.
	thisCompute->mCurrentView.setObjectDisplay( thisCompute->mObjectDisplayState );
	thisCompute->mCurrentView.setMultipleDrawPassCount( thisCompute->mMultipleDrawPassCount );

}


void 
lensDistortionCompute::preMultipleDrawPassCB( const MString& pPanelName, unsigned int passIndex, void * data)
{
	lensDistortionCompute *thisCompute = (lensDistortionCompute *) data;

	// Update view
	thisCompute->mCurrentView.beginGL();
	MColor currentBackgroundColor = thisCompute->mCurrentView.backgroundColor();

	if( passIndex == 0 /*Drawing other than image plane pass*/)
	{
		//	Clear background with alpha = 0.0 for blending.
		//
		//	Since a specific blending function is used to blend the lens distorted models with image plane, 
		//	transparent objects are not blend with properly. 
		glClearColor( currentBackgroundColor.r, currentBackgroundColor.g, currentBackgroundColor.b, 0.0 ); 
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		// Hide image plane.
		thisCompute->mCurrentView.setObjectDisplay( thisCompute->mObjectDisplayState & ~M3dView::kDisplayImagePlane);

	}
	else /* Drawing image plane pass */
	{
		// Restore clear color.
		glClearColor( currentBackgroundColor.r, currentBackgroundColor.g, currentBackgroundColor.b,  currentBackgroundColor.a );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		// Hide other than image plane.
		thisCompute->mCurrentView.setObjectDisplay( M3dView::kDisplayCameras | M3dView::kDisplayImagePlane );
	}

	thisCompute->mCurrentView.endGL();
}

void 
lensDistortionCompute::postMultipleDrawPassCB( const MString& pPanelName, unsigned int passIndex, void * data)
{
	lensDistortionCompute *thisCompute = (lensDistortionCompute *) data;

	thisCompute->mCurrentView.beginGL();
	if ( passIndex == 0 /*Drawing other than image plane pass*/)
	{
		// Store rendering result to mPrimaryBuffer for post-render callback.
		MStatus status = thisCompute->mCurrentView.readColorBuffer( thisCompute->mPrimaryBuffer, true );
		if ( status != MS::kSuccess )
		{
			MGlobal::displayError("Storing rendering result to buffer failed because of memory shortage. Please delete unused callbacks or decrease panel size.");
		}
	}
	else /* Drawing image plane pass */
	{
	}
	thisCompute->mCurrentView.endGL();
}

void 
lensDistortionCompute::textureUpdate()
{	
	
	unsigned int width, height;
	mPrimaryBuffer.getSize( width, height );

	// Rebuild gl texture if model panel size is changed.
	if ( mTextureWidth != width || mTextureHeight != height )
	{
		if ( mTextureIndex )
		{
			glDeleteTextures( 1, &mTextureIndex );
		}
		mTextureIndex = 0;
	}

	if (!mTextureIndex ){
		
		glGenTextures( 1, &mTextureIndex );
		glBindTexture( GL_TEXTURE_2D, mTextureIndex );

		glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, MGL_CLAMP_TO_EDGE );	// Refreshed texture should not be wrapped.
		glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, MGL_CLAMP_TO_EDGE );	// Refreshed texture should not be wrapped.
		glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

		glTexImage2D( 
			GL_TEXTURE_2D	// Target
			, 0				// Level
			, GL_RGBA		// Inernal Format
			, width 		// Width
			, height		// Height
			, 0				// Border
			, GL_RGBA		// Format
			, GL_UNSIGNED_BYTE // Type
			, mPrimaryBuffer.pixels() // Data
		);

		// Save texture size to check model panel size change.
		mTextureWidth  = width;
		mTextureHeight = height;
		
	}
	else
	{
		// Copy color buffer to generated texture object.
		glBindTexture( GL_TEXTURE_2D, mTextureIndex );

		glTexSubImage2D(
			GL_TEXTURE_2D	// Target
			, 0				// Level
			, 0			// X offset
			, 0			// Y offset
			, width 		// Width
			, height	// Height
			, GL_RGBA		// Format
			, GL_UNSIGNED_BYTE // Type
			, mPrimaryBuffer.pixels() // Data
		);
	}
}

void
lensDistortionCompute::draw()
{
	textureUpdate();

	if (!mTextureIndex)
	{
		// Texture is not initialized correctly. retunr.
		MGlobal::displayError( "Texture is not initialized correctly");
		return;
	}


	// Get lens distortion related param
	MDagPath camera;
	mCurrentView.getCamera( camera );
	MObject cameraNode = camera.node();

	double width = mCurrentView.portWidth();
	double height = mCurrentView.portHeight();
	int	   previewResolutionX		= getIntValueFromCameraAttr( cameraNode,"previewResolutionX" );
	int	   previewResolutionY		= getIntValueFromCameraAttr( cameraNode,"previewResolutionY" );
	double renderResolutionX		= getDoubleValueFromCameraAttr( cameraNode,"renderResolutionX" );
	double renderResolutionY		= getDoubleValueFromCameraAttr( cameraNode,"renderResolutionY" );

	double principalPointX			= getDoubleValueFromCameraAttr( cameraNode,"principalPointX");
	double principalPointY			= getDoubleValueFromCameraAttr( cameraNode,"principalPointY" );
	double kc1						= getDoubleValueFromCameraAttr( cameraNode,"radialDistortionCoef1" );
	double kc2						= getDoubleValueFromCameraAttr( cameraNode,"radialDistortionCoef2" );
	double kc3						= getDoubleValueFromCameraAttr( cameraNode,"tangentialDistortionCoef1" );
	double kc4						= getDoubleValueFromCameraAttr( cameraNode,"tangentialDistortionCoef2" );
	double horizontalFilmAperture	= getDoubleValueFromCameraAttr( cameraNode,"horizontalFilmAperture" )  * 25.4;
	double verticalFilmAperture		= getDoubleValueFromCameraAttr( cameraNode,"verticalFilmAperture" ) * 25.4;
	bool   drawWireframe			= getBoolValueFromCameraAttr( cameraNode, "drawWireframe" );

	// Begin OpenGL 
	mCurrentView.beginGL();

	// Push current matrix and load inverse projection matrix
	MMatrix projMatrix;
	mCurrentView.projectionMatrix(projMatrix);
	glPushMatrix();
	glLoadMatrixd( (const GLdouble *)projMatrix.inverse().matrix );

	// Store all OpenGL state
	glPushAttrib( GL_ALL_ATTRIB_BITS );

	// Draw lens distortion plane with blending
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable( GL_TEXTURE_2D );
	glTexEnvf( GL_TEXTURE_ENV,	GL_TEXTURE_ENV_MODE, GL_REPLACE	);

	// Enable polygon offset fill when wireframe is on.
	if ( drawWireframe )
	{
		glEnable( GL_POLYGON_OFFSET_FILL );
		glPolygonOffset( 0.95f, 1.0f ); 
	}

	drawLensDistortionPlane( 
		GL_QUADS
		,renderResolutionX, renderResolutionY
		,previewResolutionX, previewResolutionY 
		, width, height
		, principalPointX, principalPointY
		, kc1, kc2, kc3, kc4
		, horizontalFilmAperture, verticalFilmAperture
		);

	glDisable( GL_TEXTURE_2D );

	// Draw lens distortion plane with wireframe
	if ( drawWireframe )
	{
		glDisable( GL_POLYGON_OFFSET_FILL );
		glColor3f( 0.0f, 0.0f, 0.0f );

		drawLensDistortionPlane( 
			GL_LINE_LOOP
			, renderResolutionX, renderResolutionY
			, previewResolutionX, previewResolutionY
			, width, height
			, principalPointX, principalPointY
			, kc1, kc2, kc3, kc4
			, horizontalFilmAperture, verticalFilmAperture
		);
	}

	// Restore OpenGL state
	glPopAttrib();

	// Restore matrix
	glPopMatrix();

	// End OpenGL 
	mCurrentView.endGL();
}

void
lensDistortionCompute::drawLensDistortionPlane(
	GLenum drawMethod
	, double renderResolutionX
	, double renderResolutionY
	, int    previewResolutionX
	, int    previewResolutionY
	, double width
	, double height
	, double principalPointX
	, double principalPointY
	, double kc1, double kc2, double kc3, double kc4
	, double horizontalFilmAperture
	, double verticalFilmAperture)
{
	if ( previewResolutionX < 1 || previewResolutionY < 1 )
	{
		MGlobal::displayError("The previewResolutionX and previewResolutionY has to be more than 1.");
		return;
	}
	double xSub						= 1.0 / (double)previewResolutionX;
	double ySub						= 1.0 / (double)previewResolutionY;
	for ( int i = 0; i < previewResolutionX ; i ++ )
	{
		for ( int j = 0; j < previewResolutionY; j++ )
		{

			// Draw rectangle with applying lens distortion
			glBegin( drawMethod );

			double x, y;
			double u, v;
			
			// P0 
			x = u =  i * xSub;
			y = v =  j * ySub;

			applyLensDistortion( 
				x, y 
				,renderResolutionX, renderResolutionY
				, width, height
				, principalPointX, principalPointY
				, kc1, kc2, kc3, kc4
				, horizontalFilmAperture, verticalFilmAperture 
				);

			glTexCoord2d(  u, v );
			glVertex2d(  x, y );

			// P1 
			x = u =  i * xSub;
			y = v =  (j + 1) * ySub;

			applyLensDistortion( 
				x, y 
				,renderResolutionX, renderResolutionY
				, width, height
				, principalPointX, principalPointY
				, kc1, kc2, kc3, kc4
				, horizontalFilmAperture, verticalFilmAperture 
				);		

			glTexCoord2d(  u, v );
			glVertex2d(  x, y );

			// P2
			x = u =  (i + 1) * xSub;
			y = v =  (j + 1) * ySub;

			applyLensDistortion( 
				x, y 
				,renderResolutionX, renderResolutionY
				, width, height
				, principalPointX, principalPointY
				, kc1, kc2, kc3, kc4
				, horizontalFilmAperture, verticalFilmAperture 
				);

			glTexCoord2d(  u, v );
			glVertex2d(  x, y );

			// P3
			x = u =  (i + 1) * xSub;
			y = v =  j * ySub;

			applyLensDistortion( 
				x, y 
				,renderResolutionX, renderResolutionY
				, width, height
				, principalPointX, principalPointY
				, kc1, kc2, kc3, kc4
				, horizontalFilmAperture, verticalFilmAperture 
				);
				
			glTexCoord2d(  u, v );
			glVertex2d(  x, y );
		
			glEnd();

		}
	}
}

// Apply lens distortion to given x/y positions which are normalized in screen space
// with most commonly used lens distortion technique which uses decomposed radial
// and tangential distortion coefficient.
//
// Since this using lens model uses radial and tangential distortion coeffs which 
// are calibrated in millimeter unit space, positions must be converted from normalized 
// screen space to physical millimeter space to get correct result.
// 
// See "Manual of Photogrammetry, fourth ed., C.C. Slama, ed., 
// Falls Church, Va.: Am. Soc. Photogramettry, 1980" for more details on using lens model.
//
//  ------------------------------------------------------------------------
//  
//	[Apply lens distortion]
//	Xn, Yn	:	Current position					( in normalized screen space )
//	Xd, Yd	:	Lens distortion applied posittion	( in normalized screen space )
//
//	X0, Y0	:	Center of image						( in pixel space )
//	rW, rH	:	Resolution width/ height of image	( in pixel space )
//				Used for offsetting center of image 
//
//	fH, fV	:	Horizontal/vertical film aperture	( in millimeter )
//				Used for converting from normalized screen space to millimeter space
//
//	kc1		:	Radial Distortion coeff 1			( in millimeter * 10^-2 )
//	kc2		:	Radial Distortion coeff 2			( in millimeter * 10^-4 )
//	kc3		:	Tangential Distortion coeff 1		( in millimeter * 10^-1 )
//	kc4		:	Tangential Distortion coeff 2		( in millimeter * 10^-1 )
//				These coeffs should be calibrated in millimeter unit space
//
//  ------------------------------------------------------------------------
//
//	// Offset center 
//	Xn' = Xn - X0/rW	Yn' = Yn - Y0/rH
//
//  // Convert from normlized screen space to physical space ( millimeter )
//	// Xn'', Yn'' : Current position in physical space ( millimeter )
//	Xn'' = Xn' * fH		Yn'' = Yn' * fW
//
//	// Apply lens distortion model
//	RR = Xn''^2 + Yn''^2 
//	Q = 1 / (4*kc1*RR + 6*kc2*rn^4 + 8*kc3*Yn'' + 8*kc4*Xn'' + 1 )
//
//	Xd''= Xn'' - Q * ( Xn'' *( kc1 * RR + kc2 * RR^2) + 2*kc3 * Xn'' * Yn'' + kc4 * ( RR + 2*Xn''^2) )
//	Yd''= Yn'' - Q * ( Yn'' *( kc1 * RR + kc2 * RR^2) + kc3 * ( RR+ 2*Yn''^2) + 2 * kc4 * Xn'' * Yn'' ) 
//
//  // Convert from physical space ( millimeter ) to normalized screen space.
//	Xd' = Xd'' / fH		Yd' = Yd'' / hW
//
//	// Restore center offset
//	Xd =  Xd' + X0/rW	Yd = Yd' + Y0/rH
//
void 
lensDistortionCompute::applyLensDistortion( 
		double& Xd
		, double& Yd
		, double renderResolutionX
		, double renderResolutionY
		, double width
		, double height
		, double principalPointX
		, double principalPointY
		, double kc1, double kc2, double kc3, double kc4
		, double horizontalFilmAperture
		, double verticalFilmAperture
		)
{
	double Xn = Xd ;
	double Yn = ( 1.0 - Yd ) ;

	// Determine center offset by given principal point and normalize position.
	// Principal point is calibrated "center of lens".
	double centerOffsetX = principalPointX / renderResolutionX;
	double centerOffsetY = principalPointY / renderResolutionY;

	// Convert physical space (millimeter).
	// Assume image and film are fitted perfectly. 
	Xn = (Xn - centerOffsetX ) * (horizontalFilmAperture );
	Yn = (Yn - centerOffsetY ) * (verticalFilmAperture) ;

	// Apply generic pinhole camera model 

	if ( fabs( kc1 ) < kFloatEpsilon &&
		fabs( kc2 )  < kFloatEpsilon &&
		fabs( kc3 )  < kFloatEpsilon &&
		fabs( kc4 )  < kFloatEpsilon 
	)
	{
		// Normalize it 
		Xd = Xd * 2.0 - 1.0;
		Yd  = Yd * 2.0 - 1.0;

		return;
	}

	// Apply lens distortion formula
	double RR = (Xn * Xn) + (Yn * Yn);
	double Q = 1 / ( (4 * kc1 * RR) + (6 * kc2 * RR*RR) + (8 * kc3 * Yn) + (8 * kc4 * Xn) + 1 );

	Xd = Xn - Q * (	Xn * ( kc1 * RR + kc2 * RR * RR )	+ 2 * kc3 * Xn * Yn +	kc4 * ( RR + 2 * Xn * Xn) );
	Yd = Yn - Q * (	Yn * ( kc1 * RR + kc2 * RR * RR )	+kc3 * ( RR + 2 * Yn * Yn) +2 * kc4 * Xn * Yn );

	//  Convert to normalized space
	Xd = Xd / horizontalFilmAperture + centerOffsetX;
	Yd = Yd / verticalFilmAperture + centerOffsetY ;
	Yd = 1.0 - Yd;

	// Normalize it 
	Xd  = Xd * 2.0 - 1.0;
	Yd  = Yd * 2.0 - 1.0;

}

// Get an int attribute value from associated node;
int 
lensDistortionCompute::getIntValueFromCameraAttr( MObject& node, MString attrName )
{
	MStatus status;
	MFnDependencyNode nodeFn ( node );

	MPlug plug = nodeFn.findPlug( attrName, status );
	if (status == MS::kSuccess )
	{
		return plug.asInt();
	}
	else
	{
		MGlobal::displayError( "Please add :" + attrName + " attribute to " + nodeFn.name() );
	}
	return 0;
}

// Get a double attribute value from given node;
double 
lensDistortionCompute::getDoubleValueFromCameraAttr(  MObject& node, MString attrName )
{
	MStatus status;
	MFnDependencyNode nodeFn ( node );

	MPlug plug = nodeFn.findPlug( attrName, status );
	if (status == MS::kSuccess )
	{
		return plug.asDouble();
	}
	else
	{
		MGlobal::displayError( "Please add :" + attrName + " attribute to " + nodeFn.name() );
	}
	return 0.0;
}

// Get a bool attribute value from given node;
bool 
lensDistortionCompute::getBoolValueFromCameraAttr(  MObject& node, MString attrName )
{
	MStatus status;
	MFnDependencyNode nodeFn ( node );

	MPlug plug = nodeFn.findPlug( attrName, status );
	if (status == MS::kSuccess )
	{
		return plug.asBool();
	}
	else
	{
		MGlobal::displayError( "Please add :" + attrName + " attribute to " + nodeFn.name() );
	}
	return false;
}

MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin( obj, "Autodesk", "1.0", "Any");

	// Register the command so we can actually do some work
	//
	for (unsigned int i=0; i<MAX_MODEL_PANEL; i++)
	{
		lensDistortionCompute::currentLensDistortionCompute[i] = 0;
	}

	status = plugin.registerCommand("lensDistortionCallback",
									lensDistortionCallback::creator,
									lensDistortionCallback::newSyntax);

	if (!status)
	{
		MGlobal::displayError("registerCommand");
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus   status;
	MFnPlugin plugin( obj );

	// Remove all computation class + callbacks
	for (unsigned int i=0; i<MAX_MODEL_PANEL; i++)
	{
		if ( lensDistortionCompute::currentLensDistortionCompute[i] )
		{
			delete lensDistortionCompute::currentLensDistortionCompute[i];
			lensDistortionCompute::currentLensDistortionCompute[i] = 0;
		}
	}

	// Deregister the command
	//
	status = plugin.deregisterCommand("lensDistortionCallback");

	if (!status)
	{
		MGlobal::displayError("deregisterCommand");
	}

	return status;
}
