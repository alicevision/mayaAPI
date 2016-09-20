/*
        Author: Bruce Hickey

        Description:
	------------

        An example context to allow reordering vertex/edge lists based on a user specifed seed.


        Loading and unloading:
        ----------------------

        The meshReorderContext can be created with the
        following mel commands:

                meshReorderContext;
                setToolTo meshReorderContext1;


        How to use:
        -----------

        Once the context has created and activated follow the help line prompts. You will 
        be directed to pick 3 vertices on the mesh. Once all 3 vertices are selected the meshOrder 
	command will be invoked to reorder the mesh's vertices.
*/

#ifndef _MESH_REORDER_TOOL_H_
#define _MESH_REORDER_TOOL_H_

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
#include "meshReorderCmd.h"

class meshReorderTool : public MPxSelectionContext
{
public:
					meshReorderTool();
	virtual			~meshReorderTool();
	void*			creator();

	void	toolOnSetup( MEvent & event );

	MStatus	doRelease( MEvent& );

private:
	MStatus 			helpStateHasChanged( MEvent&);
	void				reset();

	MSelectionList		fSelectionList;

	MObjectArray 		fSelectedComponentSrc;
	MDagPathArray 		fSelectedPathSrc;

	MIntArray			fSelectVtxSrc;

	MString				fCurrentHelpString;

	int					fSelectedFaceSrc;

	int					fNumSelectedPoints;
};

//////////////////////////////////////////////
// Command to create contexts
//////////////////////////////////////////////

class meshReorderContextCmd : public MPxContextCommand
{
public: 
						meshReorderContextCmd() {};
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
