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
// customAttrManip.cpp
// 
// This plug-in demonstrates how to create user-defined manipulators
// from a user-defined context and apply the manipulator to a custom attribute
// defined on a custom transform node.  The custom transform node has a
// custom attribute defined, RockInX.  A distance base manip is defined as
// the custom manipulator and gets attached to the RockInX attribute when selected.
//
// The attachment of the manipulator is performed by an event callback that
// is registered for PostToolChanged and SelectionChanged events.  
//
////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <maya/M3dView.h>
#include <maya/MArgList.h>
#include <maya/MCursor.h>
#include <maya/MDagPath.h>
#include <maya/MEventMessage.h>
#include <maya/MFn.h>
#include <maya/MFnCamera.h>
#include <maya/MFnDistanceManip.h> 
#include <maya/MFnPlugin.h>
#include <maya/MFnTransform.h>
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>
#include <maya/MItSelectionList.h>
#include <maya/MModelMessage.h>
#include <maya/MPxContext.h>
#include <maya/MPxContextCommand.h>
#include <maya/MPxManipContainer.h> 
#include <maya/MPxNode.h>
#include <maya/MPxSelectionContext.h>
#include <maya/MPxToolCommand.h>
#include <maya/MPlug.h>
#include <maya/MPoint.h>
#include <maya/MQuaternion.h>
#include <maya/MSelectionList.h>
#include <maya/MVector.h>
#include "rockingTransform2.h"
#include "customTriadManip.h"

#define 	customAttributeString	"rockx"

MCallbackId cid1 = 0;
MCallbackId cid2 = 0;
static bool		isSetting = false;

// This constant is used to translate mouse delta values into floating point delta
// values to modify the attached attributes.
static double		scaleFactor = 0.01;

// Callback function for messages.  This callback will be registered for the PostToolChanged
// and SelectionChanged events.
//
static void eventCB(void * data);

/////////////////////////////////////////////////////////////
//
// The customAttr tool command
//
// - This command is used to turn the interactions with the manip
//   or the context into an undoable action.
//
/////////////////////////////////////////////////////////////
#define		ATTR_CMD_NAME "customAttrToolCmd"
#define		DOIT		0
#define		UNDOIT		1
#define		REDOIT		2

class customAttrCmd : public MPxToolCommand
{
public:
	customAttrCmd();
	virtual ~customAttrCmd(); 

	MStatus     doIt( const MArgList& args );
	MStatus     redoIt();
	MStatus     undoIt();
	bool        isUndoable() const;
	MStatus		finalize();
	
public:
	static void* creator();

	void		setDelta(double d);
	void		setDragX() {dragX = true;}

private:
	double	 	delta;
	bool		dragX;
	MStatus 	action( int flag );	// do the work here
	void		parseDrag(int axis);
};

customAttrCmd::customAttrCmd( )
{
	setCommandString( ATTR_CMD_NAME );
	dragX = false;
}

customAttrCmd::~customAttrCmd()
{}

void* customAttrCmd::creator()
{
	return new customAttrCmd;
}

bool customAttrCmd::isUndoable() const
//
// Description
//     Set this command to be undoable.
//
{
	return true;
}

void customAttrCmd::setDelta(double d)
//
// Description
//     This method sets the delta value that will be used when
//     the command is executed.
//
{
	delta = d * scaleFactor;
}

MStatus customAttrCmd::finalize()
//
// Description
//     This method constructs the final command syntax which will be called
//     to execute/undo/redo the action.  The syntax of the generated command
//     will be:
//
//     customAttrToolCmd <deltaVal>
//
//     where <deltaVal> is the most recently set value from the call to
//	   customAttrCmd::setDelta().
//
{
    MArgList command;
    command.addArg( commandString() );
    command.addArg( delta );

	// This call adds the command to the undo queue and sets
	// the journal string for the command.
	//
    return MPxToolCommand::doFinalize( command );
}

MStatus customAttrCmd::doIt( const MArgList& args )
//
// Description
// 	  This method executes the command given the passed arguments.
//    The arguments consist of a delta value and one or more axes
//    along which the delta value will be applied.
//
{
	MStatus stat;

	delta = args.asDouble( 0, &stat );

	return action( DOIT );
}

MStatus customAttrCmd::undoIt( )
//
// Description
// 		Undo last delta value.
//
{
	return action( UNDOIT );
}

MStatus customAttrCmd::redoIt( )
//
// Description
// 		Redo last delta value.
//
{
	return action( REDOIT );
}

MStatus customAttrCmd::action( int flag )
//
// Description
// 	  Do the actual work here to move the objects by the
//    command's delta value.  The objects will come from those
//    on the active selection list.
//
{
	MStatus stat;
	double d = delta;

	switch( flag )
	{
		case UNDOIT:	// undo
			d = -d;
			break;
		case REDOIT:	// redo
			break;
		case DOIT:		// do command
			break;
		default:
			break;
	}

	// Create a selection list iterator
	//
	MSelectionList slist;
 	MGlobal::getActiveSelectionList( slist );
	MItSelectionList iter( slist, MFn::kInvalid, &stat );

	if ( MS::kSuccess == stat ) {
		MDagPath 	mdagPath;		// Item dag path
		MObject 	mComponent;		// Current component

		// Processs all selected objects
		//
		for ( ; !iter.isDone(); iter.next() ) 
		{
			// Get path and possibly a component
			//
			iter.getDagPath( mdagPath, mComponent );

			MFnTransform transFn( mdagPath, &stat );
			if ( MS::kSuccess == stat ) {

				// If the selected object is of type rockingTransform,
				// then set the appropriate plug value depending on which axes
				// the command is operating on.
				if (transFn.typeId() == rockingTransformNode::id)
				{
					MPlug plg = transFn.findPlug(customAttributeString);
					double val;
					plg.getValue(val);
					plg.setValue(val+d);
				}
				continue;
			}

		} // for
	}
	else {
		cerr << "Error creating selection list iterator" << endl;
	}
	return MS::kSuccess;
}


/////////////////////////////////////////////////////////////
//
// The customAttrManip manipulator
//
// - This class defines the manipulator which will be used
//   when the tool becomes the active context.  It consists of
//   three distance base manips aligned along the X, Y, and Z
//   axes of the attached transform's coordinate system.  The
//   internals of the manipulator base class handle the management
//   of command information so that undo/redo are handled.
//
/////////////////////////////////////////////////////////////
class customAttrManip : public MPxManipContainer
{
public:
    customAttrManip();
    virtual ~customAttrManip();
    
    static void * 	creator();
    static MStatus 	initialize();
    virtual MStatus createChildren();
    virtual MStatus connectToDependNode(const MObject &node);

	MVector 		nodeTranslation() const;
	MQuaternion 	nodeRotation() const;

	void 			updateManipLocations();

    MDagPath 		fManip;
	MDagPath 		fNodePath;

public:
    static MTypeId 	id;
};


MTypeId customAttrManip::id( 0x80025 );

customAttrManip::customAttrManip() 
{ 
    // Do not call createChildren from here - 
    // MayaPtr has not been set up yet.
}


customAttrManip::~customAttrManip() 
{
}


void *customAttrManip::creator()
{
     return new customAttrManip();
}


MStatus customAttrManip::initialize()
{ 
    MStatus stat;
    stat = MPxManipContainer::initialize();
    return stat;
}

MStatus customAttrManip::createChildren()
//
// Description
// 	  Create the geometry of the manip.  This consists of a single
//    distance manip.
//
{
    fManip = addDistanceManip("customtManip", "customPoint");

    return MStatus::kSuccess;
}

MQuaternion customAttrManip::nodeRotation() const
//
// Description
// 	  Query and return the rotation values for the attached transform node.
//
{
	MFnDagNode dagFn(fNodePath);
	MDagPath path;
	dagFn.getPath(path);
	MFnTransform transformFn(path);
	MQuaternion q;
	transformFn.getRotation( q, MSpace::kWorld );
	return q;
}

MVector customAttrManip::nodeTranslation() const
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

void customAttrManip::updateManipLocations()
//
// Description
// 	  This method places the manip in the scene according to the information
//    obtained from the attached transform node.  The position and orientation
//    of the distance manip is determined.
//
{
	MVector  trans = nodeTranslation();
	MQuaternion  q = nodeRotation();

	MFnDistanceManip freePointManipFn(fManip);

	MVector vecX(1.0, 0.0, 0.0);

	freePointManipFn.setDirection(vecX);
	freePointManipFn.rotateBy(q);
	freePointManipFn.setTranslation(trans, MSpace::kWorld);
}

MStatus customAttrManip::connectToDependNode(const MObject &node)
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
	MFnDistanceManip distManipFn(fManip);
    MPlug cPlug = nodeFn.findPlug(customAttributeString, &stat);
	if( stat == MStatus::kSuccess )
		distManipFn.connectToDistancePlug(cPlug);

    finishAddingManips();
	updateManipLocations();

    MPxManipContainer::connectToDependNode(node);        
    return stat;
}


/////////////////////////////////////////////////////////////
//
// The customAttrManip Context
//
// - Tool contexts are custom event handlers and are used to 
//   process mouse interactions.  The context subclass 
//   allows you to override press/drag/release events.
//
//	This context contains the customAttrManip defined above and
//  also performs its own mouse processing by handling the middle
//  mouse.  When the middle mouse button is lifted at the end of
//  a drag, a command is constructed for use in undo/redo.
//
/////////////////////////////////////////////////////////////
#define MOVEHELPSTR		"Drag the distance manips to change values on custom attributes"
#define MOVETITLESTR	"customAttrManip"


class customAttrCtx : public MPxSelectionContext
{
public:
    customAttrCtx();

    virtual void    toolOnSetup(MEvent &event);
    virtual void    toolOffCleanup();
    virtual MStatus doEnterRegion(MEvent &event);
	virtual MStatus	doPress(MEvent &event);
	virtual MStatus	doDrag(MEvent &event);
	virtual MStatus	doRelease(MEvent &event);

    customAttrManip * caManip;

private:
	M3dView 		view;
	MEvent::MouseButtonType downButton;
	short 			startPos_x, endPos_x;
	short 			startPos_y, endPos_y;
	double	 		delta;
	customAttrCmd * cmd;
};


void updateManipulators(void * data);
MCallbackId id1;


customAttrCtx::customAttrCtx()
{
    MString str(MOVETITLESTR);
    setTitleString(str);
}

void customAttrCtx::toolOnSetup(MEvent &)
{
    MString str(MOVEHELPSTR);
    setHelpString(str);

	updateManipulators(this);
	MStatus status;
	id1 = MModelMessage::addCallback(MModelMessage::kActiveListModified,
									 updateManipulators, 
									 this, &status);
	if (!status) {
		cerr << "Model addCallback failed\n";
	}
}


void customAttrCtx::toolOffCleanup()
//
// Description
//     This method is called when the context is no longer the current context.
//     The manipulator is removed from the scene.
//
{
    MStatus status;
	status = MModelMessage::removeCallback(id1);
	if (!status) {
		cerr << "Model remove callback failed\n";
	}
	MPxContext::toolOffCleanup();
}

MStatus customAttrCtx::doPress( MEvent & event )
//
// Description
//     This method is called when a mouse button is pressed while this context is
//    the current context.
//
{
	// Let the parent class handle the event first in case there is no object
	// selected yet.  The parent class will perform any necessary selection.
	MStatus stat = MPxSelectionContext::doPress( event );

	// If an object has been selected, then process the event.  Otherwise,
	// ignore it as there is nothing to do.
	if ( !isSelecting() ) {
		if (event.mouseButton() == MEvent::kMiddleMouse)
		{
			setCursor(MCursor::handCursor);
			view = M3dView::active3dView();

			// Create an instance of the customAttrCmd tool command and initialize
			// its delta value to 0.  As the mouse drags, the delta value will change.
			// when the mouse is lifted, a final command will be constructed with the
			// most recently set delta value and axis specifications.
			cmd = (customAttrCmd *)newToolCommand();
			cmd->setDelta(0.0);

			event.getPosition( startPos_x, startPos_y );

			// Determine the channel box attribute which will be operated on by the
			// dragging motion and set the state of the command accordingly.
			unsigned int i;
			MStringArray result;
			MGlobal::executeCommand(
					"channelBox -q -selectedMainAttributes $gChannelBoxName", result);
			for (i=0; i<result.length(); i++)
			{
				if (result[i] == customAttributeString)
				{
					cmd->setDragX();
					break;
				}
			}
		}
	}

	return stat;
}

MStatus customAttrCtx::doDrag( MEvent & event )
//
// Description
//     This method is called when a mouse button is dragged while this context is
//    the current context.
//
{
	MStatus stat;

	// If an object has been selected, then process the drag.  Otherwise, pass the
    // event on up to the parent class.
	if ((!isSelecting()) && (event.mouseButton() == MEvent::kMiddleMouse)) {

		event.getPosition( endPos_x, endPos_y );

		// Undo the command to erase the previously set delta value from the
		// node, set a new delta value in the command and redo the command to
		// set the values in the node.
		cmd->undoIt();
		cmd->setDelta(endPos_x - startPos_x);
		stat = cmd->redoIt();
		view.refresh( true );
	}
	else
		stat = MPxSelectionContext::doDrag( event );

	return stat;
}

MStatus customAttrCtx::doRelease( MEvent & event )
//
// Description
//     This method is called when a mouse button is released while this context is
//    the current context.
//
{
	// Let the parent class handle the event.
	MStatus stat = MPxSelectionContext::doRelease( event );

	// If an object is selected, process the event if the middle mouse button
	// was lifted.
	if ( !isSelecting() ) {
		if (event.mouseButton() == MEvent::kMiddleMouse)
		{
			event.getPosition( endPos_x, endPos_y );

			// Delete the move command if we have moved less then 2 pixels
			// otherwise call finalize to set up the journal and add the
			// command to the undo queue.
			//
			if ( abs(startPos_x - endPos_x) < 2 ) {
				delete cmd;
				view.refresh( true );
			}
			else {
				stat = cmd->finalize();
				view.refresh( true );
			}
			setCursor(MCursor::defaultCursor);
		}
	}

	return stat;
}

void updateManipulators(void * data)
{
//
// Description
//     This callback function is called when the selection changes so that the manip can
//    be reinitialized on the new current selection.
//
    MStatus stat = MStatus::kSuccess;
	
	// Delete any previously existing manipulators.
	customAttrCtx * ctxPtr = (customAttrCtx *) data;
	ctxPtr->deleteManipulators(); 

    // iterate through the selected objects:
    // 
    MSelectionList list;
    stat = MGlobal::getActiveSelectionList(list);
    MItSelectionList iter(list, MFn::kInvalid, &stat);

    if (MS::kSuccess == stat) {
        for (; !iter.isDone(); iter.next()) {
            // create the customAttrManip for each object selected:
            //
            MString manipName ("customAttrManip");
            MObject manipObject;
            ctxPtr->caManip = (customAttrManip *) customAttrManip::newManipulator(manipName, manipObject);

            if (NULL != ctxPtr->caManip) {

                MObject dependNode;
                iter.getDependNode(dependNode);
                MFnDependencyNode dependNodeFn(dependNode);

			    ctxPtr->addManipulator(manipObject);

                dependNodeFn.findPlug(customAttributeString, &stat);
                if (MStatus::kSuccess != stat) {
					ctxPtr->deleteManipulators(); 
					return;
                }

                ctxPtr->caManip->connectToDependNode(dependNode);
            } 
        }
    }
}


MStatus customAttrCtx::doEnterRegion(MEvent &event)
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
#define CREATE_CTX_NAME "customAttrManipContext"


class customAttrCtxCommand : public MPxContextCommand
{
public:
    customAttrCtxCommand() {};
    virtual MPxContext * makeObj();

public:
    static void* creator();
};


MPxContext *customAttrCtxCommand::makeObj()
{
    customAttrCtx *newC = new customAttrCtx();
	return newC;
}


void *customAttrCtxCommand::creator()
{
    return new customAttrCtxCommand;
}


///////////////////////////////////////////////////////////////////////
//
// The following routines are used to register/unregister
// the commands we are creating within Maya
//
///////////////////////////////////////////////////////////////////////
MStatus initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj, PLUGIN_COMPANY, "3.0", "Any");

    status = plugin.registerContextCommand(
			CREATE_CTX_NAME, &customAttrCtxCommand::creator,
			ATTR_CMD_NAME, &customAttrCmd::creator);
    if (!status) 
	{
        status.perror("registerContextCommand");
        return status;
    }

    status = plugin.registerContextCommand(
			CREATE_TRIAD_CTX_NAME, &customTriadCtxCommand::creator);
    if (!status) 
	{
        status.perror("registerContextCommand");
        return status;
    }


	// Classify the node as a transform.  This causes Viewport
	// 2.0 to treat the node the same way it treats a regular
	// transform node.
	const MString classification = "drawdb/geometry/transform/rockingTransform2";
	status = plugin.registerTransform(	"rockingTransform", 
										rockingTransformNode::id, 
						 				&rockingTransformNode::creator, 
										&rockingTransformNode::initialize,
						 				&rockingTransformMatrix::creator,
										rockingTransformMatrix::id,
										&classification);
	if (!status) {
		status.perror("registerNode");
		return status;
	}

    status = plugin.registerNode("customAttrManip", customAttrManip::id, 
								 &customAttrManip::creator, &customAttrManip::initialize,
								 MPxNode::kManipContainer);
    if (!status) 
	{
        status.perror("registerManip");
        return status;
    }

    status = plugin.registerNode("customTriadManip", customTriadManip::id, 
								 &customTriadManip::creator, &customTriadManip::initialize,
								 MPxNode::kManipContainer);
    if (!status) 
	{
        status.perror("registerManip");
        return status;
    }

	MPxManipContainer::addToManipConnectTable(customTriadManip::id); 

	// Register a callback for the PostToolChanged and SelectionChanged events.
	cid1 = MEventMessage::addEventCallback("PostToolChanged", eventCB, NULL, &status);
	cid2 = MEventMessage::addEventCallback("SelectionChanged", eventCB, NULL, &status);

	MGlobal::executeCommandOnIdle("customAttrManipContext myCustomAttrContext");
	MGlobal::executeCommandOnIdle("customTriadManipContext myCustomTriadContext");
    return status;
}


MStatus uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj);

	// Unregister the event callbacks.
	MEventMessage::removeCallback(cid1);
	MEventMessage::removeCallback(cid2);

	MPxManipContainer::removeFromManipConnectTable(customTriadManip::id); 

    status = plugin.deregisterContextCommand(CREATE_CTX_NAME, ATTR_CMD_NAME);
    if (!status) {
        status.perror("deregisterContextCommand");
        return status;
    }

    status = plugin.deregisterContextCommand(CREATE_TRIAD_CTX_NAME);
    if (!status) {
        status.perror("deregisterContextCommand");
        return status;
    }

    status = plugin.deregisterNode(customAttrManip::id);
    if (!status) {
        status.perror("deregisterManip");
        return status;
    }

    status = plugin.deregisterNode(customTriadManip::id);
    if (!status) {
        status.perror("deregisterManip");
        return status;
    }

	status = plugin.deregisterNode( rockingTransformNode::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

    return status;
}

///////////////////////////////////////////////////
//
// Callback functions
//
///////////////////////////////////////////////////

//
// This callback gets called for the PostToolChanged and SelectionChanged events.
// It checks to see if the current context is the dragAttrContext, which is the context
// applied by default when a custom numeric attribute is selected in the channel box.
// In this case, the customAttrManip context is set.
// 
static void eventCB(void * data)
{
	// This check prevents recursion from happening when overriding the manip.
	if (isSetting)
		return;

	MSelectionList selList;
	MGlobal::getActiveSelectionList(selList);

	MString curCtx = "";
	MGlobal::executeCommand("currentCtx", curCtx);

	MDagPath path;
	MObject dependNode;
	for (unsigned int i=0; i<selList.length(); i++)
	{
        if ((selList.getDependNode(i, dependNode)) == MStatus::kSuccess)
		{
			MFnTransform node;
			if (node.hasObj(dependNode))
				node.setObject(dependNode);
			else
				continue;

			if (node.typeId() == rockingTransformNode::id)
			{
				// If the current context is the dragAttrContext, check to see
				// if the custom channel box attributes are selected.  If so,
				// attach the custom manipulator.
				if ((curCtx == "dragAttrContext") || (curCtx == ""))
				{
					// Make sure that the correct channel box attributes are selected
					// before setting the tool context.
					unsigned int c;
					MStringArray cboxAttrs;
					MGlobal::executeCommand(
							"channelBox -q -selectedMainAttributes $gChannelBoxName", cboxAttrs);
					for (c=0; c<cboxAttrs.length(); c++)
					{
						if (cboxAttrs[c] == customAttributeString)
						{
							isSetting = true;
							MGlobal::executeCommand("setToolTo myCustomAttrContext");
							isSetting = false;
							return;
						}
					}
				}
				if ((curCtx == "moveSuperContext") || (curCtx == "manipMoveContext") ||
					(curCtx == ""))
				{
					isSetting = true;
					MGlobal::executeCommand("setToolTo myCustomTriadContext");
					isSetting = false;
					return;
				}
			}
		}
	}
}

