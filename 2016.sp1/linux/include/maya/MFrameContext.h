#ifndef _MFrameContext
#define _MFrameContext
//-
// ===========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================
//+
#if defined __cplusplus

// ****************************************************************************
// INCLUDED HEADER FILES
#include <maya/MStatus.h>
#include <maya/MMatrix.h>
#include <maya/MFloatArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MBoundingBox.h>
#include <maya/MDagPath.h>
#include <maya/MString.h>
#include <maya/MColor.h>

// ****************************************************************************
// CLASS FORWARDS
namespace MHWRender{
	class MRenderTarget;
}

class THmanipContainer;
class THmanip;
class THstandardContext;
class THselectContext;
class THtexContext; 

namespace MHWRender
{

// ****************************************************************************
// DECLARATIONS
//
//! \ingroup OpenMayaRender
//! \brief This class contains some global information for the current render frame.
/*!
MFrameContext is designed to provide information which is available per frame
render. This includes information such as render targets, viewport size and
camera information.

In terms of relative scope, MFrameContext can be thought of as encompassing
the time period for a "pass" (MPassContext) and the time period for actual drawing
(MDrawContext).

MDrawContext is derived from MFrameContext and provides its own implementation
for all virtual methods. The values returned from these methods may differ
slightly between MFrameContext and MDrawContext as MFrameContext retrieves the
values from Maya and MDrawContext retrieves the values from the GPU device
state. Also, MFrameContext::getMatrix() is not able to return values for any
matrix type requiring the object-to-world matrix as that information is only
available at draw time.
	*/
class OPENMAYARENDER_EXPORT MFrameContext
{
public:
	//! Matrices that can be accessed
	enum MatrixType {
		kWorldMtx,                      //!< Object to world matrix
		kWorldTransposeMtx,             //!< Object to world matrix transpose
		kWorldInverseMtx,               //!< Object to world matrix inverse
		kWorldTranspInverseMtx,         //!< Object to world matrix transpose inverse (adjoint)
		kViewMtx,                       //!< World to view matrix
		kViewTransposeMtx,              //!< World to view matrix tranpose
		kViewInverseMtx,                //!< World to view matrix inverse
		kViewTranspInverseMtx,          //!< World to view matrix transpose inverse (adjoint)
		kProjectionMtx,                 //!< Projection matrix
		kProjectionTranposeMtx,         //!< Projection matrix tranpose
		kProjectionInverseMtx,          //!< Projection matrix inverse
		kProjectionTranspInverseMtx,    //!< Projection matrix tranpose inverse (adjoint)
		kViewProjMtx,                   //!< View * projection matrix
		kViewProjTranposeMtx,           //!< View * projection matrix tranpose
		kViewProjInverseMtx,            //!< View * projection matrix inverse
		kViewProjTranspInverseMtx,      //!< View * projection matrix tranpose inverse (adjoint)
		kWorldViewMtx,                  //!< World * view matrix
		kWorldViewTransposeMtx,         //!< World * view matrix transpose
		kWorldViewInverseMtx,           //!< World * view matrix inverse
		kWorldViewTranspInverseMtx,     //!< World * view matrix transpose inverse (adjoint)
		kWorldViewProjMtx,              //!< World * view * projection matrix
		kWorldViewProjTransposeMtx,     //!< World * view * projection matrix transpose
		kWorldViewProjInverseMtx,       //!< World * view * projection matrix inverse
		kWorldViewProjTranspInverseMtx  //!< World * view * projection matrix tranpose inverse (adjoint)
	};
	static MFrameContext::MatrixType semanticToMatrixType(const MString &value, MStatus* ReturnStatus=NULL);
	virtual MMatrix getMatrix(MFrameContext::MatrixType mtype, MStatus* ReturnStatus=NULL) const;

	//! Vectors or positions that can be accessed
	enum TupleType {
		kViewPosition,		//!< View position
		kViewDirection,		//!< View direction
		kViewUp,			//!< View up vector
		kViewRight,			//!< View right vector
		kViewportPixelSize,	//!< Viewport size in pixels
	};
	static MFrameContext::TupleType semanticToTupleType(const MString &value, MStatus* ReturnStatus=NULL);
	virtual MDoubleArray getTuple(MFrameContext::TupleType ttype, MStatus* ReturnStatus=NULL) const;

	virtual MStatus getViewportDimensions(int &originX, int &originY, int &width, int &height) const;

	virtual float getGlobalLineWidth() const;

	MDagPath getCurrentCameraPath(MStatus* ReturnStatus=NULL) const;

	const MRenderTarget* getCurrentColorRenderTarget() const;
	const MRenderTarget* getCurrentDepthRenderTarget() const;

	//! Display styles
    enum DisplayStyle
    {
        kGouraudShaded = (0x1),        //!< Shaded display.
        kWireFrame = (0x1 << 1),       //!< Wire frame display.
		kBoundingBox = (0x1 << 2),     //!< Bounding box display.
		kTextured = (0x1 << 3),        //!< Textured display.
		kDefaultMaterial = (0x1 << 4), //!< Default material display.
		kXrayJoint = (0x1 << 5),       //!< X-ray joint display.
		kXray = (0x1 << 6),            //!< X-ray display.
		kTwoSidedLighting = (0x1 << 7), //!< Two-sided lighting enabled.
		kFlatShaded = (0x1 << 8),		//!< Flat shading display. 
		kShadeActiveOnly = (0x1 << 9)	//!< Shade active object only. 
    };
	unsigned int getDisplayStyle() const;

	//! Lighting modes
	enum LightingMode
	{
		kNoLighting,	 //!< Use no light
		kAmbientLight,   //!< Use global ambient light
		kLightDefault,   //!< Use default light
		kSelectedLights, //!< Use lights which are selected
		kSceneLights,    //!< Use all lights in the scene
		kCustomLights	 //!< A custom set of lights which are not part of the scene. Currently this applies for use in the Hypershade Material Viewer panel
	};
	LightingMode getLightingMode() const;
	unsigned int getLightLimit() const;

	//! Types of post effects which may be enabled during rendering
	enum PostEffectType {
		kAmbientOcclusion,	//!< Screen-space ambient occlusion
		kMotionBlur,		//!< 2D Motion blur
		kGammaCorrection,	//!< Gamma correction
        kViewColorTransformEnabled = kGammaCorrection, //!< Color managed viewing
		kDepthOfField,		//!< Depth of field
		kAntiAliasing		//!< Hardware multi-sampling
	};
	bool getPostEffectEnabled( PostEffectType postEffectType ) const;

	//! Types of the fog mode
	enum FogMode {
		kFogLinear,	//!< The linear fog
		kFogExp,		//!< The exponential fog
		kFogExp2	//!< The exponential squared fog
	};
	//! Struct for hardware fog parameters
	struct HwFogParams
	{
		bool	HwFogEnabled;	//!< If hardware fog is enabled
		FogMode HwFogMode; //!< Hardware fog mode, like Linear, Exponential, Exponential squared.
		float HwFogStart;		//!< The near distance used in the linear fog.
		float HwFogEnd;		//!< The far distance used in the linear fog.
		float HwFogDensity;		//!< The density of the exponential fog.
		MColor HwFogColor;		//!< The fog color includes (r, g, b, a).
	};

	HwFogParams getHwFogParameters() const;

	//! Options for transparency algorithm
	enum TransparencyAlgorithm {
		kUnsorted = 0,	//!< Unsorted transparent object drawing
		kObjectSorting,	//!< Object sorting of transparent objects
		kWeightedAverage, //!< Weight average transparency
		kDepthPeeling, //!< Depth-peel transparency
	};
	TransparencyAlgorithm getTransparencyAlgorithm() const;

	static bool inUserInteraction();
	static bool userChangingViewContext();
	//! Wireframe on shaded modes
	enum WireOnShadedMode
	{
		kWireframeOnShadedFull,		//!< Draw wireframe 
		kWireFrameOnShadedReduced,	//!< Draw wireframe but with reduced quality
		kWireFrameOnShadedNone		//!< Do not draw wireframe
	};
	static WireOnShadedMode wireOnShadedMode();
	static bool shadeTemplates();

	//! Rendering destinations
	enum RenderingDestination
	{
		k3dViewport,				//!< Rendering to an interactive 3d viewport
		k2dViewport,				//!< Rendering to an interactive 2d viewport such as the render view
		kImage						//!< Rendering to an image
	};
	RenderingDestination renderingDestination(MString & destinationName) const;

	static const char* className();

protected:
	MFrameContext();
	virtual ~MFrameContext();

	friend class ::THmanipContainer;  
	friend class ::THmanip;
	friend class ::THstandardContext;
	friend class ::THselectContext;
	friend class ::THtexContext; 
};

} // namespace MHWRender

#endif /* __cplusplus */
#endif /* _MFrameContext */
