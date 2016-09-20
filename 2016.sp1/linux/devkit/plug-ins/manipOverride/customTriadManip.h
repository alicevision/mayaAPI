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

#define	TRIAD_CMD_NAME 			"customTriadToolCmd"
#define CREATE_TRIAD_CTX_NAME 	"customTriadManipContext"

////////////////////////////////////////////////////////////////////////
// 
// customTriadManip.h
// 
// This plug-in demonstrates how to create user-defined manipulators
// from a user-defined context and apply the manipulator to custom attributes
// defined on a custom transform node.  The custom transform node has three
// custom attributes define, TnoiseX, TnoiseY, and TnoiseZ.  Three distance
// base manips are defined as the custom manipulator and get attached to the
// noise attributes when selected.
//
// The attachment of the manipulator is performed by an event callback that
// is registered for PostToolChanged and SelectionChanged events.  
//
////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <maya/M3dView.h>
#include <maya/MDagPath.h>
#include <maya/MEventMessage.h>
#include <maya/MManipData.h>
#include <maya/MPxContextCommand.h>
#include <maya/MPxManipContainer.h> 
#include <maya/MPxSelectionContext.h>
#include <maya/MPxToolCommand.h>
#include <maya/MVector.h>

class MArgList;
class MPxContext;

/////////////////////////////////////////////////////////////
//
// The customTriadManip manipulator
//
// - This class defines the manipulator which will be used
//   when the tool becomes the active context.  It consists of
//   a single triad manip aligned with the axes of the attached
//   transform's coordinate system.  The internals of the
//   manipulator base class handle the management of command
//   information so that undo/redo are handled.
//
/////////////////////////////////////////////////////////////
class customTriadManip : public MPxManipContainer
{
public:
    customTriadManip();
    virtual ~customTriadManip();
    
    static void * 	creator();
    static MStatus 	initialize();
    virtual MStatus createChildren();
    virtual MStatus connectToDependNode(const MObject &node);

	void 			updateManipLocations();
	MVector 		nodeTranslation() const;

    MDagPath 		fTriadManip;
	MDagPath 		fNodePath;

public:
    static MTypeId 	id;
};


/////////////////////////////////////////////////////////////
//
// The customTriadManip Context
//
// - Tool contexts are custom event handlers and are used to 
//   process mouse interactions.  The context subclass 
//   allows you to override press/drag/release events.
//
//	This context contains the customTriadManip defined above and
//  also performs its own mouse processing by handling the middle
//  mouse.  When the middle mouse button is lifted at the end of
//  a drag, a command is constructed for use in undo/redo.
//
/////////////////////////////////////////////////////////////

class customTriadCtx : public MPxSelectionContext
{
public:
    customTriadCtx();

    virtual void    toolOnSetup(MEvent &event);
    virtual void    toolOffCleanup();
    virtual MStatus doEnterRegion(MEvent &event);

    customTriadManip * caManip;
};

/////////////////////////////////////////////////////////////
//
// Context creation command
//
//  This is the command that will be used to create instances
//  of our context.
//
/////////////////////////////////////////////////////////////

class customTriadCtxCommand : public MPxContextCommand
{
public:
    customTriadCtxCommand() {};
    virtual MPxContext * makeObj();

public:
    static void* creator();
};

