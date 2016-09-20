#-
# ===========================================================================
# Copyright 2015 Autodesk, Inc.  All rights reserved.
#
# Use of this software is subject to the terms of the Autodesk license
# agreement provided at the time of installation or download, or which
# otherwise accompanies this software in either electronic or hard copy form.
# ===========================================================================
#+

import sys
import maya.api.OpenMaya as om
import maya.api.OpenMayaUI as omui
import maya.api.OpenMayaRender as omr

def maya_useNewAPI():
	"""
	The presence of this function tells Maya that the plugin produces, and
	expects to be passed, objects created using the Maya Python API 2.0.
	"""
	pass


class geometryReplicator(om.MPxSurfaceShape):
	id = om.MTypeId(0x00080029)
	drawDbClassification = "drawdb/geometry/geometryReplicator"
	drawRegistrantId = "geometryReplicatorPlugin"

	# attributes
	aShowCPV = None
	aBaseMesh = None

	def __init__(self):
		om.MPxSurfaceShape.__init__(self)

	@staticmethod
	def creator():
		return geometryReplicator()

	@staticmethod
	def initialize():
		nAttr = om.MFnNumericAttribute()

		geometryReplicator.aShowCPV = nAttr.create("showCPV", "sc", om.MFnNumericData.kBoolean, 0)
		om.MPxNode.addAttribute(geometryReplicator.aShowCPV)

		geometryReplicator.aBaseMesh = nAttr.create("isBaseMesh", "bm", om.MFnNumericData.kBoolean, 0)
		om.MPxNode.addAttribute(geometryReplicator.aBaseMesh)

	def postConstructor(self):
		self.isRenderable = True

	def isBounded(self):
		return True

	def boundingBox(self):
		corner1 = om.MPoint( -0.5, 0.0, -0.5 )
		corner2 = om.MPoint( 0.5, 0.0, 0.5 )

		return om.MBoundingBox( corner1, corner2 )


###############################################################################
##
## geometryReplicatorShapeUI
##
## There's no need to draw and select this node in vp1.0, so this class
## doesn't override draw(), select(), etc. But the creator() is needed for
## plugin registration and avoid crash in cases (e.g., RB pop up menu of this node).
##
###############################################################################
class geometryReplicatorShapeUI(omui.MPxSurfaceShapeUI):
	def __init__(self):
		omui.MPxSurfaceShapeUI.__init__(self)

	@staticmethod
	def creator():
		return geometryReplicatorShapeUI()


###############################################################################
##
## geometryReplicatorGeometryOverride
##
## Handles vertex data preparation for drawing the user defined shape in
## Viewport 2.0.
##
###############################################################################
class geometryReplicatorGeometryOverride(omr.MPxGeometryOverride):
	def __init__(self, obj):
		omr.MPxGeometryOverride.__init__(self, obj)
		self.fThisNode = om.MObject(obj)
		self.fType = om.MFn.kInvalid	# the type of the associated object.
		self.fPath = om.MDagPath()		# the associated object path.

	@staticmethod
	def creator(obj):
		return geometryReplicatorGeometryOverride(obj)

	def supportedDrawAPIs(self):
		# this plugin supports both GL and DX
		return (omr.MRenderer.kOpenGL | omr.MRenderer.kDirectX11)

	def isCPVShown(self):
		res = False
		plug = om.MPlug(self.fThisNode, geometryReplicator.aShowCPV)
		if not plug.isNull:
			res = plug.asBool()
		return res

	def isBaseMesh(self):
		res = False
		plug = om.MPlug(self.fThisNode, geometryReplicator.aBaseMesh)
		if not plug.isNull:
			res = plug.asBool()
		return res

	def updateDG(self):
		if not self.fPath.isValid():
			fnThisNode = om.MFnDependencyNode(self.fThisNode)

			messageAttr = fnThisNode.attribute("message")
			messagePlug = om.MPlug(self.fThisNode, messageAttr)

			connections = messagePlug.connectedTo(False, True)
			for plug in connections:
				node = plug.node()
				if node.hasFn(om.MFn.kMesh) or node.hasFn(om.MFn.kNurbsSurface) or node.hasFn(om.MFn.kNurbsCurve) or node.hasFn(om.MFn.kBezierCurve):
					path = om.MDagPath.getAPathTo(node)
					self.fPath = path
					self.fType = path.apiType()
					break

	def updateRenderItems(self, path, list):
		if not self.fPath.isValid():
			return
		shaderManager = omr.MRenderer.getShaderManager()
		if shaderManager is None:
			return

		if self.fType == om.MFn.kNurbsCurve or self.fType == om.MFn.kBezierCurve:
			## add render items for drawing curve
			curveItem = None
			index = list.indexOf("geometryReplicatorCurve")
			if index < 0:
				curveItem = omr.MRenderItem.create("geometryReplicatorCurve", omr.MRenderItem.NonMaterialSceneItem, omr.MGeometry.kLines)
				curveItem.setDrawMode(omr.MGeometry.kAll)
				list.append(curveItem)

				shader = shaderManager.getStockShader(omr.MShaderManager.k3dSolidShader)
				if shader is not None:
					theColor = [1.0, 0.0, 0.0, 1.0]
					shader.setParameter("solidColor", theColor)

					curveItem.setShader(shader)
					shaderManager.releaseShader(shader)

			else:
				curveItem = list[index]

			if curveItem is not None:
				curveItem.enable(True)

		elif self.fType == om.MFn.kMesh:
			## add render item for drawing wireframe on the mesh
			wireframeItem = None
			index = list.indexOf("geometryReplicatorWireframe")
			if index < 0:
				wireframeItem = omr.MRenderItem.create("geometryReplicatorWireframe", omr.MRenderItem.DecorationItem, omr.MGeometry.kLines)
				wireframeItem.setDrawMode(omr.MGeometry.kWireframe)
				wireframeItem.setDepthPriority(omr.MRenderItem.sActiveWireDepthPriority)
				list.append(wireframeItem)

				shader = shaderManager.getStockShader(omr.MShaderManager.k3dSolidShader)
				if shader is not None:
					theColor = [1.0, 0.0, 0.0, 1.0]
					shader.setParameter("solidColor", theColor)

					wireframeItem.setShader(shader)
					shaderManager.releaseShader(shader)

			else:
				wireframeItem = list[index]

			if wireframeItem is not None:
				wireframeItem.enable(True)

			## disable StandardShadedItem if CPV is shown.
			showCPV = self.isCPVShown()
			index = list.indexOf("StandardShadedItem", omr.MGeometry.kTriangles, omr.MGeometry.kShaded)
			if index >= 0:
				shadedItem = list[index]
				if shadedItem is not None:
					shadedItem.enable(not showCPV)

			index = list.indexOf("StandardShadedItem", omr.MGeometry.kTriangles, omr.MGeometry.kTextured)
			if index >= 0:
				shadedItem = list[index]
				if shadedItem is not None:
					shadedItem.enable(not showCPV)

			## add item for CPV.
			index = list.indexOf("geometryReplicatorCPV")
			if index >= 0:
				cpvItem = list[index]
				if cpvItem is not None:
					cpvItem.enable(showCPV)

			else:
				## if no cpv item and showCPV is true, created the cpv item.
				if showCPV:
					cpvItem = omr.MRenderItem.create("geometryReplicatorCPV", omr.MRenderItem.MaterialSceneItem, omr.MGeometry.kTriangles)
					cpvItem.setDrawMode(omr.MGeometry.kShaded | omr.MGeometry.kTextured)
					list.append(cpvItem)

					shader = shaderManager.getStockShader(omr.MShaderManager.k3dCPVSolidShader)
					if shader is not None:
						cpvItem.setShader(shader)
						cpvItem.enable(True)
						shaderManager.releaseShader(shader)

	def populateGeometry(self, requirements, renderItems, data):
		if not self.fPath.isValid():
			return

		## here, fPath is the path of the linked object instead of the plugin node; it
		## is used to determine the right type of the geometry shape, e.g., polygon
		## or NURBS surface.
		## The sharing flag (true here) is just for the polygon shape.
		options = omr.MGeometryExtractor.kPolyGeom_Normal
		if self.isBaseMesh():
		   options = options | omr.MGeometryExtractor.kPolyGeom_BaseMesh

		extractor = omr.MGeometryExtractor(requirements, self.fPath, options)
		if extractor is None:
			return

		## fill vertex buffer
		descList = requirements.vertexRequirements()
		for desc in descList:

			if desc.semantic == omr.MGeometry.kPosition or desc.semantic == omr.MGeometry.kPosition or desc.semantic == omr.MGeometry.kNormal or desc.semantic == omr.MGeometry.kTexture or desc.semantic == omr.MGeometry.kTangent or desc.semantic == omr.MGeometry.kBitangent or desc.semantic == omr.MGeometry.kColor:
				vertexBuffer = data.createVertexBuffer(desc)
				if vertexBuffer is not None:
					## MGeometryExtractor.vertexCount and MGeometryExtractor.populateVertexBuffer.
					## since the plugin node has the same vertex data as its linked scene object,
					## call vertexCount to allocate vertex buffer of the same size, and then call
					## populateVertexBuffer to copy the data.
					vertexCount = extractor.vertexCount()
					vertices = vertexBuffer.acquire(vertexCount, True) ## writeOnly - we don't need the current buffer values
					if vertices is not None:
						extractor.populateVertexBuffer(vertices, vertexCount, desc)
						vertexBuffer.commit(vertices)

		## fill index buffer
		for item in renderItems:
			indexBuffer = data.createIndexBuffer(omr.MGeometry.kUnsignedInt32)
			if indexBuffer is None:
				continue

			## MGeometryExtractor.primitiveCount and MGeometryExtractor.populateIndexBuffer.
			## since the plugin node has the same index data as its linked scene object,
			## call primitiveCount to allocate index buffer of the same size, and then call
			## populateIndexBuffer to copy the data.
			if item.primitive() == omr.MGeometry.kTriangles:
				triangleDesc = omr.MIndexBufferDescriptor(omr.MIndexBufferDescriptor.kTriangle, "", omr.MGeometry.kTriangles, 3)
				numTriangles = extractor.primitiveCount(triangleDesc)

				indices = indexBuffer.acquire(3 * numTriangles, True) ## writeOnly - we don't need the current buffer values
				extractor.populateIndexBuffer(indices, numTriangles, triangleDesc)
				indexBuffer.commit(indices)

			elif item.primitive() == omr.MGeometry.kLines:
				edgeDesc = omr.MIndexBufferDescriptor(omr.MIndexBufferDescriptor.kEdgeLine, "", omr.MGeometry.kLines, 2)
				numEdges = extractor.primitiveCount(edgeDesc)

				indices = indexBuffer.acquire(2 * numEdges, True) ## writeOnly - we don't need the current buffer values
				extractor.populateIndexBuffer(indices, numEdges, edgeDesc)
				indexBuffer.commit(indices)

			item.associateWithIndexBuffer(indexBuffer)
	
	def cleanUp(self):
		# no-op
		pass


##---------------------------------------------------------------------------
##---------------------------------------------------------------------------
## Plugin Registration
##---------------------------------------------------------------------------
##---------------------------------------------------------------------------

def initializePlugin(obj):
	plugin = om.MFnPlugin(obj, "Autodesk", "3.0", "Any")
	try:
		plugin.registerShape("geometryReplicator", geometryReplicator.id, geometryReplicator.creator, geometryReplicator.initialize, geometryReplicatorShapeUI.creator, geometryReplicator.drawDbClassification)
	except:
		sys.stderr.write("Failed to register shape\n")
		raise

	try:
		omr.MDrawRegistry.registerGeometryOverrideCreator(geometryReplicator.drawDbClassification, geometryReplicator.drawRegistrantId, geometryReplicatorGeometryOverride.creator)
	except:
		sys.stderr.write("Failed to register override\n")
		raise

def uninitializePlugin(obj):
	plugin = om.MFnPlugin(obj)
	try:
		omr.MDrawRegistry.deregisterGeometryOverrideCreator(geometryReplicator.drawDbClassification, geometryReplicator.drawRegistrantId)
	except:
		sys.stderr.write("Failed to deregister override\n")
		raise

	try:
		plugin.deregisterNode(geometryReplicator.id)
	except:
		sys.stderr.write("Failed to deregister shape\n")
		raise

