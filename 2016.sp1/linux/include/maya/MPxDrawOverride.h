#ifndef _MPxDrawOverride
#define _MPxDrawOverride
//-
// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//+
//
// CLASS:    MPxDrawOverride
//
// ****************************************************************************
//
// CLASS DESCRIPTION (MPxDrawOverride)
//
//  MPxDrawOverride allows the user to define custom draw code to be
//  used to draw all instances of a specific DAG object type in Maya when using
//  Viewport 2.0.
//
// ****************************************************************************

#if defined __cplusplus

// ****************************************************************************
// INCLUDED HEADER FILES

#include <maya/MBoundingBox.h>
#include <maya/MMatrix.h>
#include <maya/MUserData.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MUIDrawManager.h>
#include <maya/MFrameContext.h>

// ****************************************************************************
// DECLARATIONS

class MObject;
class MDagPath;
class MSelectionMask;

// ****************************************************************************
// NAMESPACE

namespace MHWRender
{

class MDrawContext;
class MRenderItem;
class MVertexBufferArray;
class MIndexBuffer;
class MSelectionContext;
class MSelectionInfo;

// ****************************************************************************
// CLASS DECLARATION (MPxDrawOverride)

//! \ingroup OpenMayaRender MPx
//! \brief Base class for user defined drawing of nodes.
/*!
MPxDrawOverride allows the user to define custom draw code to be used
to draw all instances of a specific DAG object type in Maya when using
Viewport 2.0.

transform() should always return the world space transformation matrix for the
object to be drawn in the custom draw code. The default implementation simply
returns the transformation defined by the parent transform nodes.

isBounded() can return true or false to indicate whether the object is bounded
or not respectively. If a false value is returned
then camera frustum culling will not be performed against the bounding box.
In this situation the boundingBox() method will not be called since the bounding box 
is not required.

boundingBox() should always return the object space bounding box for whatever
is to be drawn in the custom draw code. If the bounding box is incorrect the
node may be culled at the wrong time and the custom draw method might not be
called.

When the object associated with the draw override changes, prepareForDraw() is
called which allows the user to pull data from Maya to be used in the draw
phase. It is invalid to query attribute values from Maya nodes during the draw
callback and doing so may result in instability.

addUIDrawables() provides access to the MUIDrawManager, which can be used
to queue up operations for drawing simple UI elements such as lines, circles and
text. It is called just after prepareForDraw() and carries the same restrictions on
the sorts of operations it can perform. To enable addUIDrawables(), override
hasUIDrawables() and make it return true.


At draw time, the user defined callback will be invoked at which point any
custom OpenGL drawing may occur. Data needed from the Maya dependency graph
must have been cached in the prepareForDraw() stage as it is invalid to query
such data during the draw callback.

Implementations of MPxDrawOverride must be registered with Maya through
MDrawRegistry.

When using the MHWRender::MRenderer::setGeometryDrawDirty() interface to mark a
DAG object associated with an MPxDrawOverride object dirty, the optional 
topologyChanged parameter must be set to true.
*/
class OPENMAYARENDER_EXPORT MPxDrawOverride
{
public:
	//! User draw callback definition, draw context and blind user data are parameters
	typedef void (*GeometryDrawOverrideCb)(const MDrawContext&, const MUserData*);

	MPxDrawOverride(
		const MObject& obj,
		GeometryDrawOverrideCb callback);

	virtual ~MPxDrawOverride();

	virtual MHWRender::DrawAPI supportedDrawAPIs() const;
	virtual bool hasUIDrawables() const;

	virtual MMatrix transform(
		const MDagPath& objPath,
		const MDagPath& cameraPath) const;

	virtual MBoundingBox boundingBox(
		const MDagPath& objPath,
		const MDagPath& cameraPath) const;

	virtual bool isBounded(
		const MDagPath& objPath,
		const MDagPath& cameraPath) const;

	virtual bool disableInternalBoundingBoxDraw() const;

	virtual MUserData* prepareForDraw(
		const MDagPath& objPath,
		const MDagPath& cameraPath,
		const MFrameContext& frameContext,
		MUserData* oldData) = 0;

	virtual void addUIDrawables(
		const MDagPath& objPath,
		MUIDrawManager& drawManager,
		const MFrameContext& frameContext,
		const MUserData* data);

	virtual bool refineSelectionPath(const MSelectionInfo& selectInfo,
									 const MRenderItem& hitItem, 
									 MDagPath& path,
									 MObject& geomComponents,
									 MSelectionMask& objectMask);

	virtual void updateSelectionGranularity(
		const MDagPath& path,
		MSelectionContext& selectionContext);

	static const char* className();

protected:
	MStatus setGeometryForRenderItem(
		MRenderItem& renderItem,
		const MVertexBufferArray& vertexBuffers,
		const MIndexBuffer* indexBuffer,
		const MBoundingBox* objectBox) const;

	static MStatus drawRenderItem(
		const MHWRender::MDrawContext& context, MRenderItem& item);

private:
	GeometryDrawOverrideCb fCallback;

};

} // namespace MHWRender

#endif /* __cplusplus */
#endif /* _MPxDrawOverride */
