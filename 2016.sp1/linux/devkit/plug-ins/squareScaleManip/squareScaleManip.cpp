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

//
//	squareScaleManip.cpp
//		Creates manipulator node squareScaleManipulator
//		Creates command squareManipCmd
//
//	This example demonstrates how to use the MPxManipulatorNode
//	class along with a command to create a user defined
//	manipulator.  The manipulator created is a simple square
//	with the 4 sides as OpenGL pickable components.  As you
//	move the pickable component, selected transforms have
//	their scale attribute modified.
//	A corresponding command is used to create and delete
//	the manipulator node and to support undo/redo etc.

/*

// To show this example using MEL, run the following:
loadPlugin squareScaleManip.so;
squareManipCmd -create;

// To delete the manipulator using MEL:
squareManipCmd -delete;

*/

#include "squareScaleManip.h"

#include <maya/MIOStream.h>
#include <maya/MMatrix.h>
#include <maya/MVector.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MFnTransform.h>
#include <maya/MGlobal.h>
#include <maya/MFnCamera.h>
#include <maya/MTemplateCommand.h>
#include <maya/MDrawRegistry.h>
#include <maya/MDrawContext.h>
#include <maya/MUserData.h>
#include <maya/MStateManager.h>

// Statics
MTypeId squareScaleManipulator::id( 0x81046 );
MString squareScaleManipulator::classification("drawdb/geometry/manip/squareScaleManipulator");
MString squareScaleManipulator::registrantId("SquareScaleManipPlugin");
const MPoint squareScaleManipulator::topLeft(-0.5f, 0.5f, 0.0f);
const MPoint squareScaleManipulator::topRight(0.5f, 0.5f, 0.0f);
const MPoint squareScaleManipulator::bottomLeft(-0.5f, -0.5f, 0.0f);
const MPoint squareScaleManipulator::bottomRight(0.5f, -0.5f, 0.0f);

//
// class implementation
//
squareScaleManipulator::squareScaleManipulator()
: MPxManipulatorNode()
, activeName(0)
, topName(0)
, rightName(0)
, bottomName(0)
, leftName(0)
, mousePointGlName(MPoint())
{
	// Populate initial points
	topLeft.get(tl);
	topRight.get(tr);
	bottomLeft.get(bl);
	bottomRight.get(br);
	// Setup the plane with a point on the
	// plane along with a normal
	MPoint pointOnPlane(topLeft);
	// Normal = cross product of two vectors on the plane
	MVector normalToPlane =
		(MVector(topLeft) - MVector(topRight)) ^
		(MVector(topRight) - MVector(bottomRight));
	// Necessary to normalize
	normalToPlane.normalize();
	// Plane defined by a point and a normal
	plane.setPlane(pointOnPlane, normalToPlane);
}

squareScaleManipulator::~squareScaleManipulator()
{
}

// virtual
void squareScaleManipulator::draw(
	M3dView &view,
	const MDagPath &path,
	M3dView::DisplayStyle style,
	M3dView::DisplayStatus status)
{
	// Are we in the right view
	MDagPath dpath;
	view.getCamera(dpath);
	if (!shouldDraw(dpath))
	{
		return;
	}

	// do the draw using common helper method
	view.beginGL();

	// Get the starting value of the pickable items
	MGLuint glPickableItem;
	glFirstHandle(glPickableItem);

	// Top
	topName = glPickableItem;
	// Place before you draw the manipulator component that can
	// be pickable.
	colorAndName(view, glPickableItem, true, mainColor());
	glBegin(GL_LINES);
		glVertex3fv(tl);
		glVertex3fv(tr);
	glEnd();

	// Right
	glPickableItem++;
	rightName = glPickableItem;
	colorAndName(view, glPickableItem, true, mainColor());
	glBegin(GL_LINES);
		glVertex3fv(tr);
		glVertex3fv(br);
	glEnd();

	// Bottom
	glPickableItem++;
	bottomName = glPickableItem;
	colorAndName(view, glPickableItem, true, mainColor());
	glBegin(GL_LINES);
		glVertex3fv(br);
		glVertex3fv(bl);
	glEnd();

	// Left
	glPickableItem++;
	leftName = glPickableItem;
	colorAndName(view, glPickableItem, true, mainColor());
	glBegin(GL_LINES);
		glVertex3fv(bl);
		glVertex3fv(tl);
	glEnd();

	view.endGL();
}

// virtual
MStatus	squareScaleManipulator::doPress(M3dView& view)
{
	// Reset the mousePoint information on a new press
	mousePointGlName = MPoint::origin;
	updateDragInformation();
	return MS::kSuccess;
}

// virtual
MStatus	squareScaleManipulator::doDrag(M3dView& view)
{
	updateDragInformation();
	return MS::kSuccess;
}

// virtual
MStatus squareScaleManipulator::doRelease(M3dView& view)
{
	// Scale nodes on the selection list. Implementation
	// is very simple and will not support undo.
	MSelectionList list;
	MGlobal::getActiveSelectionList(list);
	for (MItSelectionList iter(list); !iter.isDone(); iter.next())
	{
		MObject node;
		MStatus status;
        iter.getDependNode(node);
		MFnTransform xform(node, &status);
		if (MS::kSuccess == status)
		{
			double newScale[3];
			newScale[0] = mousePointGlName.x + 1;
			newScale[1] = mousePointGlName.y + 1;
			newScale[2] = mousePointGlName.z + 1;
			xform.setScale(newScale);
		}
	}
	return MS::kSuccess;
}

MStatus squareScaleManipulator::updateDragInformation()
{
	// Find the mouse point in local space
	MPoint localMousePoint;
	MVector localMouseDirection;
	if (MS::kFailure == mouseRay(localMousePoint, localMouseDirection))
		return MS::kFailure;

	// Find the intersection of the mouse point with the
	// manip plane
	MPoint mouseIntersectionWithManipPlane;
	if (!plane.intersect(localMousePoint, localMouseDirection, mouseIntersectionWithManipPlane))
		return MS::kFailure;

	mousePointGlName = mouseIntersectionWithManipPlane;

	if (glActiveName(activeName))
	{
		// Reset draw points
		topLeft.get(tl);
		topRight.get(tr);
		bottomLeft.get(bl);
		bottomRight.get(br);
		float* start = 0;
		float* end = 0;
		if (activeName == topName)
		{
			start = tl;
			end = tr;
		}
		if (activeName == bottomName)
		{
			start = bl;
			end = br;
		}
		if (activeName == rightName)
		{
			start = tr;
			end = br;
		}
		if (activeName == leftName)
		{
			start = tl;
			end = bl;
		}
		if (start && end)
		{
			// Find a vector on the plane
			lineMath line;
			MPoint a(start[0], start[1], start[2]);
			MPoint b(end[0], end[1], end[2]);
			MPoint vab = a - b;
			// Define line with a point and a vector on the plane
			line.setLine(start, vab);
			MPoint cpt;
			// Find the closest point so that we can get the
			// delta change of the mouse in local space
			if (line.closestPoint(mousePointGlName, cpt))
			{
				mousePointGlName.x -= cpt.x;
				mousePointGlName.y -= cpt.y;
				mousePointGlName.z -= cpt.z;
			}
			// Move draw points
			start[0] += (float)mousePointGlName.x;
			start[1] += (float)mousePointGlName.y;
			start[2] +=(float) mousePointGlName.z;
			end[0] += (float)mousePointGlName.x;
			end[1] += (float)mousePointGlName.y;
			end[2] +=(float) mousePointGlName.z;
		}
	}
	return MS::kSuccess;
}

bool squareScaleManipulator::shouldDraw(const MDagPath& cameraPath) const
{
	MStatus status;
	MFnCamera camera(cameraPath, &status);
	if (!status)
	{
		return false;
	}

	const char* nameBuffer = camera.name().asChar();
	if (0 == nameBuffer)
	{
		return false;
	}

	if ((0 == strstr(nameBuffer,"persp")) &&
		(0 == strstr(nameBuffer,"front")))
	{
		return false;
	}

	return true;
}


//
// Draw override implementation for VP2.0
//
class squareScaleManipulatorData : public MUserData
{
public:
	enum ManipEdge
	{
		kNone,
		kTop,
		kRight,
		kBottom,
		kLeft
	};
	squareScaleManipulatorData()
	: MUserData(false) // don't delete after draw
	, tl(0), tr(0), br(0), bl(0), activeEdge(kNone) {}
	virtual ~squareScaleManipulatorData() {}
	float* tl; float* tr; float* br; float* bl;
	ManipEdge activeEdge;
};

squareScaleManipulatorOverride::squareScaleManipulatorOverride(const MObject& obj)
: MHWRender::MPxDrawOverride(obj, squareScaleManipulatorOverride::draw)
{
}

squareScaleManipulatorOverride::~squareScaleManipulatorOverride()
{
}

MHWRender::DrawAPI squareScaleManipulatorOverride::supportedDrawAPIs() const
{
	return MHWRender::kOpenGL;
}

bool squareScaleManipulatorOverride::isBounded(
	const MDagPath& objPath,
	const MDagPath& cameraPath) const
{
	return true;
}

MBoundingBox squareScaleManipulatorOverride::boundingBox(
	const MDagPath& objPath,
	const MDagPath& cameraPath) const
{
	return MBoundingBox(
		squareScaleManipulator::topLeft,
		squareScaleManipulator::bottomRight);
}

bool squareScaleManipulatorOverride::disableInternalBoundingBoxDraw() const
{
	return true;
}

MUserData* squareScaleManipulatorOverride::prepareForDraw(
	const MDagPath& objPath,
	const MDagPath& cameraPath,
	const MHWRender::MFrameContext& frameContext,
	MUserData* oldData)
{
	// get the node
	MStatus status;
	MFnDependencyNode node(objPath.node(), &status);
	if (!status) return NULL;
	squareScaleManipulator* manipNode =
		dynamic_cast<squareScaleManipulator*>(node.userNode());
	if (!manipNode) return NULL;

	// access/create user data for draw callback
	squareScaleManipulatorData* data =
		dynamic_cast<squareScaleManipulatorData*>(oldData);
	if (!data)
	{
		data = new squareScaleManipulatorData();
	}

	// populate user data from the node
	data->tl = data->tr = data->br = data->bl = 0;
	data->activeEdge = squareScaleManipulatorData::kNone;
	if (manipNode->shouldDraw(cameraPath))
	{
		data->tl = manipNode->tl;
		data->tr = manipNode->tr;
		data->br = manipNode->br;
		data->bl = manipNode->bl;
		if (manipNode->activeName == manipNode->topName)
		{
			data->activeEdge = squareScaleManipulatorData::kTop;
		}
		else if (manipNode->activeName == manipNode->rightName)
		{
			data->activeEdge = squareScaleManipulatorData::kRight;
		}
		else if (manipNode->activeName == manipNode->bottomName)
		{
			data->activeEdge = squareScaleManipulatorData::kBottom;
		}
		else if (manipNode->activeName == manipNode->leftName)
		{
			data->activeEdge = squareScaleManipulatorData::kLeft;
		}
	}

	return data;
}

void squareScaleManipulatorOverride::draw(
	const MHWRender::MDrawContext& context,
	const MUserData* data)
{
	MHWRender::MStateManager* stateMgr = context.getStateManager();
	const squareScaleManipulatorData* manipData =
		dynamic_cast<const squareScaleManipulatorData*>(data);
	if (stateMgr && manipData &&
		manipData->tl && manipData->tr && manipData->br && manipData->bl)
	{
		// turn off depth to get manip to draw on top of everything
		const MHWRender::MDepthStencilState* oldDepthState = stateMgr->getDepthStencilState();
		if (oldDepthState)
		{
			static const MHWRender::MDepthStencilState* depthState = NULL;
			if (!depthState)
			{
				MHWRender::MDepthStencilStateDesc desc;
				desc.depthEnable = false;
				desc.depthWriteEnable = false;
				depthState = stateMgr->acquireDepthStencilState(desc);
			}
			if (depthState)
			{
				stateMgr->setDepthStencilState(depthState);
			}
		}

		// draw manip
		static const float dormantColor[4] = { 0.39f, 0.94f, 1.0f, 1.0f };
		static const float activeColor[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
		glColor4fv(manipData->activeEdge == squareScaleManipulatorData::kTop ? activeColor : dormantColor);
		glBegin(GL_LINES);
			glVertex3fv(manipData->tl);
			glVertex3fv(manipData->tr);
		glEnd();
		glColor4fv(manipData->activeEdge == squareScaleManipulatorData::kRight ? activeColor : dormantColor);
		glBegin(GL_LINES);
			glVertex3fv(manipData->tr);
			glVertex3fv(manipData->br);
		glEnd();
		glColor4fv(manipData->activeEdge == squareScaleManipulatorData::kBottom ? activeColor : dormantColor);
		glBegin(GL_LINES);
			glVertex3fv(manipData->br);
			glVertex3fv(manipData->bl);
		glEnd();
		glColor4fv(manipData->activeEdge == squareScaleManipulatorData::kLeft ? activeColor : dormantColor);
		glBegin(GL_LINES);
			glVertex3fv(manipData->bl);
			glVertex3fv(manipData->tl);
		glEnd();

		// restore old depth state
		if (oldDepthState)
		{
			stateMgr->setDepthStencilState(oldDepthState);
		}
	}
}


//
// Template command that creates and deletes the manipulator
//
class squareManipCmd;
char cmdName[] = "squareManipCmd";
char nodeName[] = "squareScaleManipulator";

class squareManipCmd : public MTemplateCreateNodeCommand<squareManipCmd,cmdName,nodeName>
{
public:
	//
	squareManipCmd()
	{}
};

static squareManipCmd _squareManipCmd;

//
// Static methods
//
void* squareScaleManipulator::creator()
{
	return new squareScaleManipulator();
}

MStatus squareScaleManipulator::initialize()
{
	// No-op
	return MS::kSuccess;
}

//
//	Entry points
//

MStatus initializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "2009", "Any");

	status = plugin.registerNode(
		nodeName,
		squareScaleManipulator::id,
		squareScaleManipulator::creator,
		squareScaleManipulator::initialize,
		MPxNode::kManipulatorNode,
		&squareScaleManipulator::classification);
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	status = _squareManipCmd.registerCommand( obj );
	if (!status) {
		status.perror("registerCommand");
		return status;
	}

    status = MHWRender::MDrawRegistry::registerDrawOverrideCreator(
		squareScaleManipulator::classification,
		squareScaleManipulator::registrantId,
		squareScaleManipulatorOverride::Creator);
	if (!status) {
		status.perror("registerDrawOverrideCreator");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterNode( squareScaleManipulator::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}
	status = _squareManipCmd.deregisterCommand( obj );
	if (!status) {
		status.perror("deregisterCommand");
		return status;
	}

	status = MHWRender::MDrawRegistry::deregisterDrawOverrideCreator(
		squareScaleManipulator::classification,
		squareScaleManipulator::registrantId);
	if (!status) {
		status.perror("deregisterDrawOverrideCreator");
		return status;
	}

	return status;
}


