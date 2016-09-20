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
#include <maya/MPxManipContainer.h>
#include <maya/MFnPlugin.h>
#include <maya/MObjectHandle.h>
#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MDagModifier.h>

#include <maya/M3dView.h>
#include <maya/MGLFunctionTable.h>

#include "manipulatorMath.h"

//
// Manipulator class
//
class lineManip : public MPxManipulatorNode
{
public:
	lineManip();
	~lineManip();

	// Important virtuals to implement
	virtual void draw(M3dView &view, const MDagPath &path,
				M3dView::DisplayStyle style, M3dView::DisplayStatus status);

	virtual MStatus	doPress( M3dView& view );
	virtual MStatus	doDrag( M3dView& view );
	virtual MStatus	doRelease( M3dView& view );

	// Standard required methods
	static void* creator();
	static MStatus initialize();

	// Utility method
	MStatus updateDragInformation();

	// Node id
	static MTypeId id;

	// Manipulator changes behaviour based on the setting
	// of these two booleans
	bool affectScale;
	bool affectTranslate;

private:
	// GL component name used for drawing and picking
	MGLuint lineName;
	// Simple plane math class
	planeMath plane;
	// Modified mouse position
	MPoint mousePointGlName;
};

//
// Manipulator container which will hold two lineManip nodes
//
class lineManipContainer : public MPxManipContainer
{
public:
	lineManipContainer();
	virtual ~lineManipContainer();

	// Important virtuals to implement
	virtual MStatus createChildren();
	virtual MStatus connectToDependNode(const MObject & node);
	virtual void draw(M3dView &view, const MDagPath &path,
					M3dView::DisplayStyle style, M3dView::DisplayStatus status);

	// Utility method to return the nodeTranslation of
	// fDagPath
	MVector nodeTranslation() const;

	// Standard required methods
	static void * creator();
	static MStatus initialize();

	// Node id
    static MTypeId id;

public:
	MDagPath fNodePath;
};



