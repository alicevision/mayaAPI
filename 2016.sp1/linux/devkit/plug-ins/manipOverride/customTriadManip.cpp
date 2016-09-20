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

////////////////////////////////////////////////////////////////////////
// 
// customTriadManip.cpp
// 
// This module demonstrates how to create user-defined manipulators
// from a user-defined context and apply the manipulator to the translation
// attribute of a transform node.
//
////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <maya/MArgList.h>
#include <maya/MCursor.h>
#include <maya/MDistance.h>
#include <maya/MEulerRotation.h>
#include <maya/MFn.h>
#include <maya/MFnCamera.h>
#include <maya/MFnNumericData.h>
#include <maya/MFnFreePointTriadManip.h> 
#include <maya/MFnTransform.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MGlobal.h>
#include <maya/MItSelectionList.h>
#include <maya/MModelMessage.h>
#include <maya/MIOStream.h>
#include <maya/MPlug.h>
#include <maya/MPoint.h>
#include <maya/MPxNode.h>
#include <maya/MSelectionList.h>
#include "customTriadManip.h"

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

MTypeId customTriadManip::id( 0x80026 );

customTriadManip::customTriadManip() 
{ 
    // Do not call createChildren from here - 
    // MayaPtr has not been set up yet.
}


customTriadManip::~customTriadManip() 
{
}


void *customTriadManip::creator()
{
     return new customTriadManip();
}


MStatus customTriadManip::initialize()
{ 
    MStatus stat;
    stat = MPxManipContainer::initialize();
    return stat;
}

MStatus customTriadManip::createChildren()
//
// Description
// 	  Create the geometry of the manip.  This consists of a single
//    triad manip.
//
{
    fTriadManip = addFreePointTriadManip("customtManip", "customPoint");

    return MStatus::kSuccess;
}

MVector customTriadManip::nodeTranslation() const
//
// Description
// 	  Query and return the translation values for the attached transform node.
//
{
	MFnDagNode dagFn(fNodePath);
	MDagPath path;
	dagFn.getPath(path);
	MFnTransform transformFn(path);
	return transformFn.translation(MSpace::kWorld);
}

void customTriadManip::updateManipLocations()
//
// Description
// 	  This method places the manip in the scene according to the information
//    obtained from the attached transform node.  The position and orientation
//    of the three distance manips are determined.
//
{
	MFnFreePointTriadManip manipFn(fTriadManip);

	MVector offset = nodeTranslation();
	manipFn.setPoint(offset);
}

MStatus customTriadManip::connectToDependNode(const MObject &node)
//
// Description
// 	  This method activates the manip on the given transform node.
//
{
    MStatus stat = MStatus::kSuccess;

	// Get the DAG path
	//
	MFnDagNode dagNodeFn(node);
	dagNodeFn.getPath(fNodePath);

    // Connect the plugs to the manip.
    MFnDependencyNode nodeFn(node);    
	MFnFreePointTriadManip manipFn(fTriadManip);

    MPlug cPlug = nodeFn.findPlug("translate", &stat);
	if( stat == MStatus::kSuccess )
		manipFn.connectToPointPlug(cPlug);
    finishAddingManips();
	updateManipLocations();

    MPxManipContainer::connectToDependNode(node);        
    return stat;
}

/////////////////////////////////////////////////////////////
//
// The customTriadManip Context
//
// - Tool contexts are custom event handlers and are used to 
//   process mouse interactions.  The context subclass 
//   allows you to override press/drag/release events.
//
//	This context contains the customTriadManip defined above.
//
/////////////////////////////////////////////////////////////
#define MOVEHELPSTR		"Drag the triad manip to change the translation values"
#define MOVETITLESTR	"customTriadManip"


static void updateTriadManipulator(void * data);
static MCallbackId mid = 0;


customTriadCtx::customTriadCtx()
{
    MString str(MOVETITLESTR);
    setTitleString(str);
}

void customTriadCtx::toolOnSetup(MEvent &)
{
    MString str(MOVEHELPSTR);
    setHelpString(str);

	updateTriadManipulator(this);
	MStatus status;
	mid = MModelMessage::addCallback(MModelMessage::kActiveListModified,
									 updateTriadManipulator, 
									 this, &status);
	if (!status) {
		cerr << "Model addCallback failed\n";
	}
}


void customTriadCtx::toolOffCleanup()
//
// Description
//     This method is called when the context is no longer the current context.
//     The manipulator is removed from the scene.
//
{
    MStatus status;
	status = MModelMessage::removeCallback(mid);
	if (!status) {
		cerr << "Model remove callback failed\n";
	}
	MPxContext::toolOffCleanup();
}


static void updateTriadManipulator(void * data)
//
// Description
//     This callback function is called when the selection changes so that the manip can
//    be reinitialized on the new current selection.
//
{
    MStatus stat = MStatus::kSuccess;
	
	// Delete any previously existing manipulators
	customTriadCtx * ctxPtr = (customTriadCtx *) data;
	ctxPtr->deleteManipulators(); 

    // iterate through the selected objects:
    // 
    MSelectionList list;
    stat = MGlobal::getActiveSelectionList(list);
    MItSelectionList iter(list, MFn::kInvalid, &stat);

    if (MS::kSuccess == stat) {
        for (; !iter.isDone(); iter.next()) {

            // Create the customTriadManip for each object selected.
            MString manipName ("customTriadManip");
            MObject manipObject;
            ctxPtr->caManip = (customTriadManip *) customTriadManip::newManipulator(manipName, manipObject);

            if (NULL != ctxPtr->caManip) {
                MObject dependNode;
                iter.getDependNode(dependNode);
                MFnDependencyNode dependNodeFn(dependNode);

			    ctxPtr->addManipulator(manipObject);

                dependNodeFn.findPlug("translate", &stat);
                if (MStatus::kSuccess != stat) {
					ctxPtr->deleteManipulators(); 
					return;
                }

                ctxPtr->caManip->connectToDependNode(dependNode);
            }
        }
    }
}


MStatus customTriadCtx::doEnterRegion(MEvent &event)
//
// Print the tool description in the help line.
//
{
    MString str(MOVEHELPSTR);
    return setHelpString(str);
}

/////////////////////////////////////////////////////////////
//
// Context creation command
//
//  This is the command that will be used to create instances
//  of our context.
//
/////////////////////////////////////////////////////////////

MPxContext *customTriadCtxCommand::makeObj()
{
    customTriadCtx *newC = new customTriadCtx();
	return newC;
}


void *customTriadCtxCommand::creator()
{
    return new customTriadCtxCommand;
}


