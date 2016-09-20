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

#include <maya/MTypeId.h>
#include <maya/MTypes.h>
#include <maya/MPxManipulatorNode.h>
#include <maya/MFnPlugin.h>
#include <maya/MObjectHandle.h>
#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MDagModifier.h>
#include <maya/MPxDrawOverride.h>
#include <maya/M3dView.h>
#include <maya/MPoint.h>
#include <maya/MGLFunctionTable.h>

#include "manipulatorMath.h"

//
// Custom manipulator class
//
class squareScaleManipulator : public MPxManipulatorNode
{
public:
	squareScaleManipulator();
	~squareScaleManipulator();

	// The important virtuals to implement
	virtual void draw(
		M3dView& view,
		const MDagPath& path,
		M3dView::DisplayStyle style,
		M3dView::DisplayStatus status);

	virtual MStatus	doPress(M3dView& view);
	virtual MStatus	doDrag(M3dView& view);
	virtual MStatus	doRelease(M3dView& view);

	// Utility methods
	MStatus updateDragInformation();

	// Standard API required methods
	static void* creator();
	static MStatus initialize();

	// Node id
	static MTypeId id;
	static MString classification;
	static MString registrantId;

	// Base points for manip
	static const MPoint topLeft;
	static const MPoint topRight;
	static const MPoint bottomLeft;
	static const MPoint bottomRight;

private:
	bool shouldDraw(const MDagPath& camera) const;

	// GL component names for drawing and picking
	MGLuint activeName, topName, rightName, bottomName, leftName;
	// Final draw points (make accessible to draw override)
	friend class squareScaleManipulatorOverride;
	float tl[4], tr[4], br[4], bl[4];
	// Simple class for plane creation, intersection
	planeMath plane;
	// Modified mouse position used for updating manipulator
	MPoint mousePointGlName;
};

//
// Draw override class for drawing manip in VP2.0
//
class squareScaleManipulatorOverride : public MHWRender::MPxDrawOverride
{
public:
	static MHWRender::MPxDrawOverride* Creator(const MObject& obj)
	{
		return new squareScaleManipulatorOverride(obj);
	}

	virtual ~squareScaleManipulatorOverride();

	virtual MHWRender::DrawAPI supportedDrawAPIs() const;

    virtual bool isBounded(
        const MDagPath& objPath,
        const MDagPath& cameraPath) const;

	virtual MBoundingBox boundingBox(
		const MDagPath& objPath,
		const MDagPath& cameraPath) const;

	virtual bool disableInternalBoundingBoxDraw() const;

	virtual MUserData* prepareForDraw(
		const MDagPath& objPath,
		const MDagPath& cameraPath,
		const MHWRender::MFrameContext& frameContext,
		MUserData* oldData);

	static void draw(
		const MHWRender::MDrawContext& context,
		const MUserData* data);

private:
	squareScaleManipulatorOverride(const MObject& obj);
};

