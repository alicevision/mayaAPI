#ifndef _LENSDISTORTIONCALLBACK_H
#define _LENSDISTORTIONCALLBACK_H
#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MUiMessage.h>
#include <maya/M3dView.h>
#include <maya/MImage.h>
#include <maya/MStringArray.h>

#define MAX_MODEL_PANEL 4

/*
 This example plug-in demonstrates how to perform mutiple draw pass with 
 MUiMessage::add3dViewPreMultipleDrawPassMsgCallback() and 
 MUiMessage::add3dViewPostMultipleDrawPassMsgCallback().

 The command "lensDistortionCallback" supports the following options.
	-r/-remove	: Remove lens distortion callback from given model panel.
	-ex/-exists	: Query existence of lens distortion callback for given model panel.
	-l/-list	: List names of the panels which have a callback attached.

If none of the above options are specified the default is to add the lens distortion 
 callback to the the given model panel


 Following dynamic attributes should be added to the camera which is associated with the model panel.
 
 	int		previewResolutionX			: X division of lens distortion plane. e.g. 40.
	int		previewResolutionY			: Y division of lens distortion plane. e.g. 30.
	int		renderResolutionX			: Rendering resolution width. e.g. 1920.
	int		renderResolutionY			: Rendering resolution height. e.g. 1080.
	double	principalPointX				: Center of Lens X. e.g. 961.1.
	double  principalPointY				: Center of Lens Y. e.g. 540.5.
	double	radialDistortionCoef1		: Radial Distortion coeff 1. e.g 0.0068.
	double	radialDistortionCoef2		: Radial Distortion coeff 2. e.g -0.00016.
	double	tangentialDistortionCoef1	: Tangential Distortion coeff 1. e.g. -0.00051
	double	tangentialDistortionCoef2	: Tangential Distortion coeff 2. e.g. 0.0000
	bool	drawWireframe				: Turn on/off wireframe

 For details on the lens model used here and the meanings of the various parameters, see 
 "Manual of Photogrammetry, fourth ed., C.C. Slama, ed., Falls Church, Va.: Am. Soc. Photogramettry, 1980".


 Example:
	lensDistortionCallback `getPanel -withFocus`;
*/

class lensDistortionCompute;

// lensDistortionCallback class offers
//
//	- Add an instance of lensDistortionCompute and 
//	associate it with a model panel.
//	- Remove an instance of lensDistortionCompute which 
//	associated with model panel by "-remove" flag
//	- Query existence of instance of lensDistortionCompute
//	which associated with model panel by "-exists" flag
//	- List names of the panels which have a callback attached 
//	by "-list" flag.
//
class lensDistortionCallback : public MPxCommand
{
public:
	lensDistortionCallback();
	virtual                 ~lensDistortionCallback(); 

	MStatus                 doIt( const MArgList& args );

	static MSyntax			newSyntax();
	static void*			creator();

private:
	// Create a new syntax and add removeFlag and existFlag
	//	MS::kSuccess : Parsing finished successfully
	//	MS::kFailure : Parsing failed. Member variable will not be set properly
	MStatus                 parseArgs( const MArgList& args );

	MString					mPanelName;
	bool					mRemoveOperation;
	bool					mExistOperation;
	bool					mListOperation;
};


// lensDistortionCompute offers
//
// - deleteCB(): a callback when associated model panel deleted
//
// - preRenderCB(): a callback called before Maya drawing.
//		Enable multiple pass drawing and set 2 to mulitple pass count
//
// - postRenderCB(): a callback called after Maya drawing.
//		This callback draws lens distortion preview plane.
//		This callback blend buffers 1st pass image and with 2nd pass, and 
//		disable multiple pass drawing.
//
// - preMultipleDrawPassCB() :
// - postMultipleDrawPassCB() :
//		These callback called twice.
//		1st pass renders image plane only and store it to a buffer.
//		2nd pass renders scene except image plane.

class lensDistortionCompute
{
public:
	lensDistortionCompute(const MString &panelName);
	~lensDistortionCompute();

	MString				getPanelName(){ return mPanelName;};
	void				setPanelName(const MString &panelName) { mPanelName = panelName; }


	// Add an instance of lensDistortionCompute to given panal 
	//
	//	MS::kSuccess : Adding finished successfully
	//	MS::kFailure : Adding failed. Panel is not added. 
	static MStatus		addCallbackToPanel( MString& pPanelName );

	// Remove an instance of lensDistortionCompute from panel 
	//
	//	MS::kSuccess : Removing finished successfully
	//	MS::kFailure : Removing failed. Panel is not removed. 
	static MStatus		removeCallbackFromPanel( MString& pPanelName );

	// Query existence of instance of lensDistortionCompute for panel
	static bool			panelHasCallback( MString& pPanelName );

	// List names of the panels which have a callback attached
	static MStatus		listCallback( MStringArray& pPanelNames );

	// Clear all registered callbacks 
	void				clearCallbacks();

	static lensDistortionCompute* currentLensDistortionCompute[MAX_MODEL_PANEL];

protected:
	static void			deleteCB(const MString& panelName, void * data);
	static void			preRenderCB(const MString& panelName, void * data);
	static void			postRenderCB(const MString& panelName, void * data);
	static void			preMultipleDrawPassCB( const MString& pPanelName, unsigned int passIndex, void * data);
	static void			postMultipleDrawPassCB( const MString& pPanelName, unsigned int passIndex, void * data);

	MCallbackId			mDeleteId;
	MCallbackId			mPreRenderId;
	MCallbackId			mPostRenderId;
	MCallbackId			mPreMultipleDrawPassId;
	MCallbackId			mPostMultipleDrawPassId;

private:
	// Initialize/Update OpenGL texture object 
	void	textureUpdate( );
	
	// Main draw routine
	void	draw();
	
	// Draw lens distortion preview plane with given draw method.
	void	drawLensDistortionPlane( 
				GLenum drawMethod
				, double renderResolutionX
				, double renderResolutionY
				, int previewResolutionX
				, int previewResolutionY
				, double width
				, double height
				, double principalPointX
				, double principalPointY
				, double kc1, double kc2, double kc3, double kc4
				, double horizontalFilmAperture
				, double verticalFilmAperture
				);

	// Apply lens distortion to given x/y position.
	void	applyLensDistortion( 
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
				);

	// Get attribute values from given node;
	int		getIntValueFromCameraAttr( MObject& node, MString attrName );
	double	getDoubleValueFromCameraAttr( MObject& node, MString attrName );
	bool	getBoolValueFromCameraAttr(  MObject& node, MString attrName );

	// Panel related variables
	MString			mPanelName;

	// Current view.
	M3dView			mCurrentView;
	
	// Store rendered buffer in 1st multiple draw pass that all objects are 
	// drawn except the image plane.
	MImage			mPrimaryBuffer;

	// Texture related member variables
	unsigned int	mTextureIndex;
	unsigned int	mTextureWidth;
	unsigned int	mTextureHeight;

	// Store multiple draw pass count;
	unsigned int	mMultipleDrawPassCount;
	// Store display status
	unsigned int	mObjectDisplayState;
};

#endif
