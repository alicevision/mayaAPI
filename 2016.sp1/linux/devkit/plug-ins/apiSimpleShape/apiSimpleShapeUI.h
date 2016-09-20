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

///////////////////////////////////////////////////////////////////////////////
//
// apiSimpleShapeUI.h
//
// Encapsulates the UI portion of a user defined shape. All of the
// drawing and selection code goes here.
//
////////////////////////////////////////////////////////////////////////////////

#include <maya/MPxSurfaceShapeUI.h> 
#include "apiSimpleShape.h" 

class apiSimpleShapeUI : public MPxSurfaceShapeUI
{
public:
	apiSimpleShapeUI();
	virtual ~apiSimpleShapeUI(); 

	/////////////////////////////////////////////////////////////////////
	//
	// Overrides
	//
	/////////////////////////////////////////////////////////////////////

	// Puts draw request on the draw queue
	//
	virtual void	getDrawRequests( const MDrawInfo & info,
									 bool objectAndActiveOnly,
									 MDrawRequestQueue & requests );

	// Main draw routine. Gets called by maya with draw requests.
	//
	virtual void	draw( const MDrawRequest & request,
						  M3dView & view ) const;

	// Main selection routine
	//
	virtual bool	select( MSelectInfo &selectInfo,
							MSelectionList &selectionList,
							MPointArray &worldSpaceSelectPts ) const;

	/////////////////////////////////////////////////////////////////////
	//
	// Helper routines
	//
	/////////////////////////////////////////////////////////////////////

	void	drawVertices( const MDrawRequest & request, M3dView & view ) const;

	bool 	selectVertices( MSelectInfo &selectInfo,
		   		MSelectionList &selectionList,
				MPointArray &worldSpaceSelectPts ) const;

	static  void *      creator();

private:
	// Draw Tokens
	//
	enum {
		kDrawVertices, // component token
		kDrawWireframe,
		kDrawWireframeOnShaded,
		kDrawSmoothShaded,
		kDrawFlatShaded,
		kLastToken
	};
};
