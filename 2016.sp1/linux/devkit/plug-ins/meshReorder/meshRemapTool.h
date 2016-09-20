/*
	Author: Bruce Hickey 

	Description:

 	An example context to allow remapping vertex/edge lists from one mesh to another

	Loading and unloading:
        ----------------------

        The meshRemapContext can be created with the
        following mel commands:

                meshRemapContext;	
                setToolTo meshRemapContext1;

        How to use:
        -----------

        Once the context has created and activated follow the help line prompts. You will 
	be directed to pick 3 vertices on the source mesh, and the corresponding 3 vertices on 
	the target mesh. Once all 6 vertices are selected the meshRemap command will be invoked
	to remap the target mesh's vertices.
*/

#ifndef _MESH_REMAP_TOOL_H_
#define _MESH_REMAP_TOOL_H_

#include <maya/MString.h>
#include <maya/MItSelectionList.h>
#include <maya/MPxContextCommand.h>
#include <maya/MPxContext.h>
#include <maya/MPxSelectionContext.h>
#include <maya/MEvent.h>
#include <maya/M3dView.h>
#include <maya/MObjectArray.h>
#include <maya/MDagPathArray.h>
#include <maya/MIntArray.h>
#include "meshRemapCmd.h"

class meshRemapTool : public MPxSelectionContext
{
public:
					meshRemapTool();
	virtual			~meshRemapTool();
	void*			creator();

	void	toolOnSetup( MEvent & event );

	MStatus	doRelease( MEvent& );

	virtual void    completeAction();

private:
	void 				helpStateHasChanged();
	void				reset();
	void				executeCmd();

	bool				arePointsOverlap(const MPointArray&, const MPointArray&) const;
	static MStatus		checkForHistory(const MDagPath& mesh);
	MStatus				resolveMapping();

	MSelectionList		fSelectionList;

	MObjectArray 		fSelectedComponentSrc;
	MDagPathArray 		fSelectedPathSrc;
	MObjectArray 		fSelectedComponentDst;
	MDagPathArray 		fSelectedPathDst;

	MIntArray			fSelectVtxSrc;
	MIntArray			fSelectVtxDst;

	MString				fCurrentHelpString;

	int					fSelectedFaceSrc;
	int					fSelectedFaceDst;

	int					fNumSelectedPoints;
};

//////////////////////////////////////////////
// Command to create contexts
//////////////////////////////////////////////

class meshRemapContextCmd : public MPxContextCommand
{
public: 
						meshRemapContextCmd() {};
	virtual MPxContext* makeObj();
	static void*		creator();
};

#endif  

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
