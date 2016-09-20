///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2009, Sony Pictures Imageworks
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the
// distribution.  Neither the name of Sony Pictures Imageworks nor the
// names of its contributors may be used to endorse or promote
// products derived from this software without specific prior written
// permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _SPRETICLE_LOC_H_
#define _SPRETICLE_LOC_H_

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

#include <maya/MPxLocatorNode.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MColor.h>
#include <maya/M3dView.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MIOStream.h>
#include <maya/MDagPath.h>
#include <maya/MFnCamera.h>
#include <maya/MVector.h>
#include <maya/MMatrix.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnStringData.h>
#include <maya/MTime.h>
#include <maya/MGlobal.h>
#include <maya/MFileIO.h>
#include <maya/MFileObject.h>
#include <maya/MDrawContext.h>

//viewport 2.0 includes
#include <maya/MUserData.h>
#include <maya/MDrawRegistry.h>
#include <maya/MPxDrawOverride.h>
#include <maya/MViewport2Renderer.h>

	class Geom
	{
	public:
		double  x1, x2;
		double  y1, y2;
		double  x, y;
		MColor  lineColor;
		MColor  maskColor;
		bool    isValid;

		Geom& operator=(const Geom& rh) {
			x1			= rh.x1			;
			x2			= rh.x2			;
			y1			= rh.y1			;
			y2			= rh.y2			;
			x			= rh.x			;
			y			= rh.y			;
			lineColor	= rh.lineColor	;
			maskColor	= rh.maskColor	;
			isValid		= rh.isValid	;	

			return *this;
		}	
	};

	class Aspect_Ratio
	{
	public:
		
		double aspectRatio;
		int    displayMode;
		bool   displaySafeAction;
		bool   displaySafeTitle;

		Geom   aspectGeom;
		Geom   safeActionGeom;
		Geom   safeTitleGeom;

		Aspect_Ratio& operator=(const Aspect_Ratio& rh) {
			aspectRatio       = rh.aspectRatio      ;
			displayMode       =	rh.displayMode      ;
			displaySafeAction =	rh.displaySafeAction;
			displaySafeTitle  =	rh.displaySafeTitle ;

			aspectGeom        =	rh.aspectGeom       ;
			safeActionGeom    =	rh.safeActionGeom   ;
			safeTitleGeom     =	rh.safeTitleGeom    ;

			return *this;
		}
	};

	class PanScan : public Aspect_Ratio
	{
	public:
		double panScanRatio;
		double panScanOffset;

		PanScan& operator= (const PanScan& rh) {
			Aspect_Ratio::operator=(rh);
			panScanRatio = rh.panScanRatio;
			panScanOffset = rh.panScanOffset;

			return *this;
		}
	};

	class Filmback
	{
	public:
		double horizontalFilmAperture;
		double verticalFilmAperture;
		double soundTrackWidth;
		double displayFilmGate;

		double horizontalImageAperture;
		double verticalImageAperture;

		double displayProjGate;
		double horizontalProjectionGate;
		double verticalProjectionGate;

		bool   displaySafeAction;
		double horizontalSafeAction;
		double verticalSafeAction;

		bool   displaySafeTitle;
		double horizontalSafeTitle;
		double verticalSafeTitle;

		Geom   filmbackGeom;

		Geom   safeActionGeom;
		Geom   safeTitleGeom;
		Geom   imageGeom;

		Geom   projGeom;

		Filmback& operator=(const Filmback& rh)
		{
			horizontalFilmAperture	= rh.horizontalFilmAperture		;
			verticalFilmAperture	= rh.verticalFilmAperture		;
			soundTrackWidth			= rh.soundTrackWidth			;
			horizontalImageAperture	= rh.horizontalImageAperture	;
			verticalImageAperture	= rh.verticalImageAperture		;
			displayFilmGate			= rh.displayFilmGate			;
			horizontalProjectionGate   = rh.horizontalProjectionGate;
			verticalProjectionGate	= rh.verticalProjectionGate		;
			horizontalSafeAction	= rh.horizontalSafeAction		;
			verticalSafeAction		= rh.verticalSafeAction			;
			horizontalSafeTitle		= rh.horizontalSafeTitle		;
			verticalSafeTitle		= rh.verticalSafeTitle			;
			displayProjGate			= rh.displayProjGate			;
			displaySafeAction		= rh.displaySafeAction			;
			displaySafeTitle		= rh.displaySafeTitle			;

			filmbackGeom			= rh.filmbackGeom				;
			safeActionGeom			= rh.safeActionGeom				;
			safeTitleGeom			= rh.safeTitleGeom				;
			imageGeom				= rh.imageGeom					;

			return *this;
		}
	};

	class PadOptions
	{
	public:
	
		bool   usePad;
		bool   isPadded;
		double padAmountX;
		double padAmountY;
		int    displayMode;

		Geom   padGeom;

		PadOptions& operator=(const PadOptions& rh) {
			usePad       = rh.usePad     ;
			isPadded     = rh.isPadded   ;
			padAmountX   = rh.padAmountX ;
			padAmountY   = rh.padAmountY ;
			displayMode  = rh.displayMode;
			padGeom	     = rh.padGeom	 ;  

			return *this;
		}
	};

	class Options
	{
	public:

		bool   drawingEnabled;
		bool   enableTextDrawing;

		MColor textColor;
		MColor lineColor;

		bool   displayLineH;
		bool   displayLineV;
		bool   displayThirdsH;
		bool   displayThirdsV;
		bool   displayCrosshair;

		bool   useSpRet;
		bool   driveCameraAperture;
		bool   useOverscan;

		double maximumDistance;

		Options& operator=(const Options& rh) {
			drawingEnabled      = rh.drawingEnabled     ;
			enableTextDrawing   = rh.enableTextDrawing  ;
			useSpRet		    = rh.useSpRet		   	;
			displayLineH        = rh.displayLineH       ;
			displayLineV        = rh.displayLineV       ;
			displayThirdsH      = rh.displayThirdsH     ;
			displayThirdsV      = rh.displayThirdsV     ;
			displayCrosshair    = rh.displayCrosshair   ;
			driveCameraAperture = rh.driveCameraAperture;
			useOverscan         = rh.useOverscan        ;
			maximumDistance     = rh.maximumDistance    ;
			textColor           = rh.textColor          ;
			lineColor           = rh.lineColor          ;

			return *this;
		}
	};

	class TextData
	{
	public:
	
		int     textType;
		MString textStr;
		int     textAlign;
		double textPosX;
		double textPosY;
		int    textPosRel;
		int    textLevel;
		int    textARLevel;
		MColor textColor;

		TextData& operator=(const TextData& rh) {
			textType    = rh.textType   ;
			textStr     = rh.textStr    ;
			textAlign   = rh.textAlign  ;
			textPosX    = rh.textPosX   ;
			textPosY    = rh.textPosY   ;
			textPosRel  = rh.textPosRel ;
			textLevel   = rh.textLevel  ;
			textARLevel = rh.textARLevel;
			textColor   = rh.textColor  ;

			return *this;
		}
	};

	class Camera
	{
	public:

		double		nearClippingPlane;
		double		farClippingPlane;
		bool		isOrtho;
		MDagPath	cameraPath;
		
		Camera& operator=(const Camera& rh){
			nearClippingPlane = rh.nearClippingPlane;
			farClippingPlane  = rh.farClippingPlane;
			isOrtho			  = rh.isOrtho;
			cameraPath		  = rh.cameraPath;

			return *this;
		}
	};


class spReticleLocData;

class spReticleLoc : public MPxLocatorNode
{
public:
	spReticleLoc();
	virtual                 ~spReticleLoc();

	virtual void            draw( M3dView & view, const MDagPath & path,
									M3dView::DisplayStyle style,
									M3dView::DisplayStatus status );

	virtual bool            setInternalValueInContext( const MPlug &,
													const MDataHandle &,
													MDGContext &);

	static  void            *creator();
	static  MStatus         initialize();

	virtual bool            excludeAsLocator() const;
	virtual void            postConstructor();

	virtual bool            isTransparent() const {return true;}
	virtual bool            drawLast() const      {return true;}
	virtual bool            isBounded() const     {return true;}
	virtual MBoundingBox    boundingBox() const;

	//get data that impacts drawing 
	void getDrawData(spReticleLocData& drawData, 
					const MDagPath &cameraPath,
					const double& portHeight, const double& portWidth);

public:
	static MTypeId id;
	static MString drawDbClassification;
	static MString drawRegistrantId;
	static MObject DrawingEnabled;
	static MObject EnableTextDrawing; // added by sjt@sjt.is (May 2010).
	static MObject FilmbackAperture;
	static MObject HorizontalFilmAperture;
	static MObject VerticalFilmAperture;
	static MObject SoundTrackWidth;
	static MObject DisplayFilmGate;
	static MObject ProjectionGate;
	static MObject HorizontalProjectionGate;
	static MObject VerticalProjectionGate;
	static MObject DisplayProjectionGate;
	static MObject SafeAction;
	static MObject HorizontalSafeAction;
	static MObject VerticalSafeAction;
	static MObject DisplaySafeAction;
	static MObject SafeTitle;
	static MObject HorizontalSafeTitle;
	static MObject VerticalSafeTitle;
	static MObject DisplaySafeTitle;
	static MObject AspectRatios;
	static MObject AspectRatio;
	static MObject DisplayMode;
	static MObject AspectMaskColor;
	static MObject AspectMaskTrans;
	static MObject AspectLineColor;
	static MObject AspectLineTrans;
	static MObject AspectDisplaySafeAction;
	static MObject AspectDisplaySafeTitle;
	static MObject PanScanAttr;
	static MObject PanScanAspectRatio;
	static MObject PanScanDisplayMode;
	static MObject PanScanDisplaySafeTitle;
	static MObject PanScanDisplaySafeAction;
	static MObject PanScanRatio;
	static MObject PanScanOffset;
	static MObject PanScanMaskColor;
	static MObject PanScanMaskTrans;
	static MObject PanScanLineColor;
	static MObject PanScanLineTrans;
	static MObject FilmGateColor;
	static MObject FilmGateTrans;
	static MObject ProjGateColor;
	static MObject ProjGateTrans;
	static MObject HideLocator;
	static MObject UseSpReticle;
	static MObject DisplayLineH;
	static MObject DisplayLineV;
	static MObject DisplayThirdsH;
	static MObject DisplayThirdsV;
	static MObject DisplayCrosshair;
	static MObject MiscTextColor;
	static MObject MiscTextTrans;
	static MObject LineColor;
	static MObject LineTrans;
	static MObject Time;
	static MObject DriveCameraAperture;
	static MObject MaximumDistance;
	static MObject UseOverscan;
	static MObject Pad;
	static MObject UsePad;
	static MObject PadAmount;
	static MObject PadAmountX;
	static MObject PadAmountY;
	static MObject PadDisplayMode;
	static MObject PadMaskColor;
	static MObject PadMaskTrans;
	static MObject PadLineColor;
	static MObject PadLineTrans;
	static MObject Text;
	static MObject TextType;
	static MObject TextStr;
	static MObject TextAlign;
	static MObject TextPos;
	static MObject TextPosX;
	static MObject TextPosY;
	static MObject TextPosRel;
	static MObject TextLevel;
	static MObject TextARLevel;
	static MObject TextColor;
	static MObject TextTrans;
	static MObject Tag;

	MObject thisNode;

	double loadDefault;
	bool   needRefresh;

private:
	MStatus getPadData(spReticleLocData& drawData);
	MStatus getFilmbackData(spReticleLocData& drawData);
	MStatus getProjectionData(spReticleLocData& drawData);
	MStatus getSafeActionData(spReticleLocData& drawData);
	MStatus getSafeTitleData(spReticleLocData& drawData);
	MStatus getAspectRatioChildren ( MPlug arPlug, Aspect_Ratio & ar );
	static bool aspectRatioSortPredicate( const Aspect_Ratio &, const Aspect_Ratio &);
	MStatus getAspectRatioData (spReticleLocData& drawData);
	bool	needToUpdateAspectRatios(spReticleLocData& drawData);
	MStatus getPanScanData (PanScan & ps );
	MStatus getTextChildren ( MPlug tPlug, TextData & td );
	MStatus getTextData(spReticleLocData& drawData);
	void	getCustomTextElement(spReticleLocData& drawData);
	bool	needToUpdateTextData(spReticleLocData& drawData);
	MStatus getOptions(Options& options );

	MPoint	getPoint (float x, float y, M3dView & view, MMatrix& wim);
	MStatus getColor ( MObject colorObj, MObject transObj, MColor & color );
	MMatrix getMatrix( MString matrixStr );
	void printAspectRatio ( Aspect_Ratio & ar );
	void printPanScan ( PanScan & ps );
	void printText ( TextData & td );
	void printGeom ( Geom & g );
	void printOptions (Options& opt);

	//draw methods
	void drawMask3D( Geom g1, Geom g2, double z, MColor color, bool sides );
	void drawMask( Geom g1, Geom g2, MColor color, bool sides, double ncp);
	void drawLine3D( double x1, double x2, double y1, double y2, double z, MColor color, bool stipple );
	void drawLines3D( Geom g, double z, MColor color, bool sides, bool stipple );
	void drawLine( double x1, double x2, double y1, double y2, MColor color, bool stipple, double ncp );
	void drawLines( Geom g, MColor color, bool sides, bool stipple, double ncp );
	void drawText( MString text, double tx, double ty, MColor color, M3dView::TextPosition, M3dView & view, MMatrix wim );
	void drawInternalTextElements(M3dView & view);
	void drawCustomTextElements(std::vector<TextData> textWhenDraw, MMatrix wim, M3dView & view);
};

class spReticleLocData : public MUserData
{
public:
	spReticleLocData() : MUserData(false) {} // don't delete after draw
	virtual ~spReticleLocData() {}

	void doCompute();

	//geometry computing methods
	void calcPortGeom();
	void calcFilmbackGeom();
	void calcMaskGeom( Geom & g, double w, double h, Geom & gSrc, double wSrc, double hSrc );
	void calcFilmbackSafeActionGeom();
	void calcFilmbackSafeTitleGeom();
	void calcSafeActionGeom( Aspect_Ratio & ar );
	void calcSafeTitleGeom( Aspect_Ratio & ar );
	void calcAspectGeom( Aspect_Ratio & ar);
	void calcPanScanGeom( PanScan & ps );
	
	//data
	Filmback					oFilmback,filmback;
	PadOptions					pad;
	PanScan						panScan;
	Geom						portGeom;	
	Options						options;
	Camera						cameraData;

	double						portWidth;
	double						portHeight;
	double						overscan;	
	double						ncp;
	MMatrix						wim;

	int							numAspectRatios;
	double						maximumDist;		
	
	std::vector<TextData>		text;	
	std::vector<TextData>		textWhenDraw;
	std::vector<Aspect_Ratio>	ars;		

	MString						nodeName;
	int							useReticle;
	
	bool						fCustomBoxDraw;
	MBoundingBox				fCurrentBoundingBox;

private:
	void setDataHelper();

};

class spReticleLocDrawOverride : public MHWRender::MPxDrawOverride
{
public:
	static MHWRender::MPxDrawOverride* Creator(const MObject& obj)
	{
		return new spReticleLocDrawOverride(obj);
	}

	virtual ~spReticleLocDrawOverride();

	virtual MHWRender::DrawAPI supportedDrawAPIs() const;

	virtual bool isBounded(
		const MDagPath& objPath,
		const MDagPath& cameraPath) const;

	virtual MBoundingBox boundingBox(
		const MDagPath& objPath,
		const MDagPath& cameraPath) const;

	virtual MUserData* prepareForDraw(
		const MDagPath& objPath,
		const MDagPath& cameraPath,
		const MHWRender::MFrameContext& frameContext,
		MUserData* oldData);

	virtual bool hasUIDrawables() const { return true; }

	virtual void addUIDrawables(
		const MDagPath& objPath,
		MHWRender::MUIDrawManager& drawManager,
		const MHWRender::MFrameContext& frameContext,
		const MUserData* data);

	static void draw(const MHWRender::MDrawContext& context, const MUserData* data);

protected:
	bool		 mCustomBoxDraw;

	//draw methods
	void drawMask( Geom g1, Geom g2, MColor color, bool sides, MHWRender::MUIDrawManager& drawManager );
	void drawLine( double x1, double x2, double y1, double y2, MColor color, bool stipple, MHWRender::MUIDrawManager& drawManager);
	void drawLines( Geom g, MColor color, bool sides, bool stipple, MHWRender::MUIDrawManager& drawManager);
	void drawText( MString text, double tx, double ty, MColor color, MHWRender::MUIDrawManager::TextAlignment textAlignment, MHWRender::MUIDrawManager& drawManager );
	void drawCustomTextElements(std::vector<TextData> textWhenDraw, MHWRender::MUIDrawManager& drawManager);

private:
	spReticleLocDrawOverride(const MObject& obj);

	int		loadDefault;
	bool	needRefresh;
};

#endif
