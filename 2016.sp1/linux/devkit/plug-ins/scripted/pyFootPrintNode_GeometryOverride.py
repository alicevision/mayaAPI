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
import ctypes
import maya.api.OpenMaya as om
import maya.api.OpenMayaUI as omui
import maya.api.OpenMayaAnim as oma
import maya.api.OpenMayaRender as omr

def maya_useNewAPI():
	"""
	The presence of this function tells Maya that the plugin produces, and
	expects to be passed, objects created using the Maya Python API 2.0.
	"""
	pass

def matrixAsArray(matrix):
	array = []
	for i in xrange(16):
		array.append(matrix[i])
	return array

## Foot Data
##
sole = [ [  0.00, 0.0, -0.70 ],
				 [  0.04, 0.0, -0.69 ],
				 [  0.09, 0.0, -0.65 ],
				 [  0.13, 0.0, -0.61 ],
				 [  0.16, 0.0, -0.54 ],
				 [  0.17, 0.0, -0.46 ],
				 [  0.17, 0.0, -0.35 ],
				 [  0.16, 0.0, -0.25 ],
				 [  0.15, 0.0, -0.14 ],
				 [  0.13, 0.0,  0.00 ],
				 [  0.00, 0.0,  0.00 ],
				 [ -0.13, 0.0,  0.00 ],
				 [ -0.15, 0.0, -0.14 ],
				 [ -0.16, 0.0, -0.25 ],
				 [ -0.17, 0.0, -0.35 ],
				 [ -0.17, 0.0, -0.46 ],
				 [ -0.16, 0.0, -0.54 ],
				 [ -0.13, 0.0, -0.61 ],
				 [ -0.09, 0.0, -0.65 ],
				 [ -0.04, 0.0, -0.69 ],
				 [ -0.00, 0.0, -0.70 ] ]
heel = [ [  0.00, 0.0,  0.06 ],
				 [  0.13, 0.0,  0.06 ],
				 [  0.14, 0.0,  0.15 ],
				 [  0.14, 0.0,  0.21 ],
				 [  0.13, 0.0,  0.25 ],
				 [  0.11, 0.0,  0.28 ],
				 [  0.09, 0.0,  0.29 ],
				 [  0.04, 0.0,  0.30 ],
				 [  0.00, 0.0,  0.30 ],
				 [ -0.04, 0.0,  0.30 ],
				 [ -0.09, 0.0,  0.29 ],
				 [ -0.11, 0.0,  0.28 ],
				 [ -0.13, 0.0,  0.25 ],
				 [ -0.14, 0.0,  0.21 ],
				 [ -0.14, 0.0,  0.15 ],
				 [ -0.13, 0.0,  0.06 ],
				 [ -0.00, 0.0,  0.06 ] ]
soleCount = 21
heelCount = 17


#############################################################################
##
## Node implementation with standard viewport draw
##
#############################################################################
class footPrint(omui.MPxLocatorNode):
	id = om.MTypeId( 0x80007 )
	drawDbClassification = "drawdb/geometry/footPrint"
	drawRegistrantId = "FootprintNodePlugin"

	size = None	## The size of the foot

	@staticmethod
	def creator():
		return footPrint()

	@staticmethod
	def initialize():
		unitFn = om.MFnUnitAttribute()

		footPrint.size = unitFn.create( "size", "sz", om.MFnUnitAttribute.kDistance )
		unitFn.default = om.MDistance(1.0)

		om.MPxNode.addAttribute( footPrint.size )

	def __init__(self):
		omui.MPxLocatorNode.__init__(self)

	def compute(self, plug, data):
		return None

	def draw(self, view, path, style, status):
		## Get the size
		##
		thisNode = self.thisMObject()
		plug = om.MPlug( thisNode, footPrint.size )
		sizeVal = plug.asMDistance()
		multiplier = sizeVal.asCentimeters()

		global sole, soleCount
		global heel, heelCount

		view.beginGL()

		## drawing in VP1 views will be done using V1 Python APIs:
		import maya.OpenMayaRender as v1omr
		glRenderer = v1omr.MHardwareRenderer.theRenderer()
		glFT = glRenderer.glFunctionTable()

		if ( style == omui.M3dView.kFlatShaded ) or ( style == omui.M3dView.kGouraudShaded ):
			## Push the color settings
			##
			glFT.glPushAttrib( v1omr.MGL_CURRENT_BIT )
			
			# Show both faces
			glFT.glDisable( v1omr.MGL_CULL_FACE )

			if status == omui.M3dView.kActive:
				view.setDrawColor( 13, omui.M3dView.kActiveColors )
			else:
				view.setDrawColor( 13, omui.M3dView.kDormantColors )

			glFT.glBegin( v1omr.MGL_TRIANGLE_FAN )
			for i in xrange(soleCount-1):
				glFT.glVertex3f( sole[i][0] * multiplier, sole[i][1] * multiplier, sole[i][2] * multiplier )
			glFT.glEnd()

			glFT.glBegin( v1omr.MGL_TRIANGLE_FAN )
			for i in xrange(heelCount-1):
				glFT.glVertex3f( heel[i][0] * multiplier, heel[i][1] * multiplier, heel[i][2] * multiplier )
			glFT.glEnd()

			glFT.glPopAttrib()

		## Draw the outline of the foot
		##
		glFT.glBegin( v1omr.MGL_LINES )
		for i in xrange(soleCount-1):
			glFT.glVertex3f( sole[i][0] * multiplier, sole[i][1] * multiplier, sole[i][2] * multiplier )
			glFT.glVertex3f( sole[i+1][0] * multiplier, sole[i+1][1] * multiplier, sole[i+1][2] * multiplier )

		for i in xrange(heelCount-1):
			glFT.glVertex3f( heel[i][0] * multiplier, heel[i][1] * multiplier, heel[i][2] * multiplier )
			glFT.glVertex3f( heel[i+1][0] * multiplier, heel[i+1][1] * multiplier, heel[i+1][2] * multiplier )
		glFT.glEnd()

		view.endGL()

	def isBounded(self):
		return True

	def boundingBox(self):
		## Get the size
		##
		thisNode = self.thisMObject()
		plug = om.MPlug( thisNode, footPrint.size )
		sizeVal = plug.asMDistance()
		multiplier = sizeVal.asCentimeters()

		corner1 = om.MPoint( -0.17, 0.0, -0.7 )
		corner2 = om.MPoint( 0.17, 0.0, 0.3 )

		corner1 *= multiplier
		corner2 *= multiplier

		return om.MBoundingBox( corner1, corner2 )

#############################################################################
##
## Viewport 2.0 override implementation
##
#############################################################################

class footPrintGeometryOverride(omr.MPxGeometryOverride):
	colorParameterName_ = "solidColor"
	wireframeHeelItemName_ = "heelLocatorWires"
	wireframeSoleItemName_ = "soleLocatorWires"
	shadedHeelItemName_ = "heelLocatorTriangles"
	shadedSoleItemName_ = "soleLocatorTriangles"
	
	@staticmethod
	def creator(obj):
		return footPrintGeometryOverride(obj)

	def __init__(self, obj):
		omr.MPxGeometryOverride.__init__(self, obj)
		self. mSolidUIShader = None
		self.mLocatorNode = obj
		self.mMultiplier = 0.0
		self.mMultiplierChanged = True
		
		shaderMgr = omr.MRenderer.getShaderManager()
		if shaderMgr:
			self.mSolidUIShader = shaderMgr.getStockShader(omr.MShaderManager.k3dSolidShader)

	def __del__(self):
		if self.mSolidUIShader:
			shaderMgr = omr.MRenderer.getShaderManager()
			if shaderMgr:
				shaderMgr.releaseShader(self.mSolidUIShader)

	def supportedDrawAPIs(self):
		# this plugin supports all modes
		return omr.MRenderer.kOpenGL | omr.MRenderer.kOpenGLCoreProfile | omr.MRenderer.kDirectX11

	def hasUIDrawables(self):
		return False

	def updateDG(self):
		plug = om.MPlug(self.mLocatorNode, footPrint.size)
		newScale = 1.0
		if not plug.isNull:
			sizeVal = plug.asMDistance()
			newScale = sizeVal.asCentimeters()
		
		if newScale != self.mMultiplier:
			self.mMultiplier = newScale
			self.mMultiplierChanged = True
	
	def cleanUp(self):
		pass

	def isIndexingDirty(self, item):
		return False
		
	def isStreamDirty(self, desc):
		return self.mMultiplierChanged
		
	def updateRenderItems(self, dagPath, renderList ):
		
		fullItemList = ( 
					# Wireframe sole and heel:
					( (footPrintGeometryOverride.wireframeHeelItemName_, 
					   footPrintGeometryOverride.wireframeSoleItemName_),
					omr.MGeometry.kLineStrip,
					omr.MGeometry.kWireframe),
					
					# Shaded sole and heel
					( (footPrintGeometryOverride.shadedHeelItemName_, 
					    footPrintGeometryOverride.shadedSoleItemName_),
					omr.MGeometry.kTriangleStrip,
					omr.MGeometry.kShaded)
				)

		for itemNameList, geometryType, drawMode in fullItemList:
			for itemName in itemNameList:
				renderItem = None
				index = renderList.indexOf(itemName)
				if index < 0:
					renderItem = omr.MRenderItem.create(
						itemName,
						omr.MRenderItem.DecorationItem,
						geometryType)
					renderItem.setDrawMode(drawMode)
					renderItem.setDepthPriority(5)

					renderList.append(renderItem)
				else:
					renderItem = renderList[index]

				if renderItem:
					if self.mSolidUIShader:
						self.mSolidUIShader.setParameter(footPrintGeometryOverride.colorParameterName_, omr.MGeometryUtilities.wireframeColor(dagPath))
						renderItem.setShader(self.mSolidUIShader)
					renderItem.enable(True)

	def populateGeometry(self, requirements, renderItems, data):
		vertexBufferDescriptorList = requirements.vertexRequirements()
		
		for vertexBufferDescriptor in vertexBufferDescriptorList:
			if vertexBufferDescriptor.semantic == omr.MGeometry.kPosition:
				verticesCount = soleCount+heelCount
				verticesBuffer = data.createVertexBuffer(vertexBufferDescriptor)
				verticesPositionDataAddress = verticesBuffer.acquire(verticesCount, True)
				verticesPositionData = ((ctypes.c_float * 3)*verticesCount).from_address(verticesPositionDataAddress)	
				
				verticesPointerOffset = 0
				
				# We concatenate the heel and sole positions into a single vertex buffer.
				# The index buffers will decide which positions will be selected for each render items.
				for vtxList in (heel, sole):
					for vtx in vtxList:
						verticesPositionData[verticesPointerOffset][0] = vtx[0] * self.mMultiplier
						verticesPositionData[verticesPointerOffset][1] = vtx[1] * self.mMultiplier
						verticesPositionData[verticesPointerOffset][2] = vtx[2] * self.mMultiplier
						verticesPointerOffset += 1
						
				verticesBuffer.commit(verticesPositionDataAddress)
				
				break
					
		for item in renderItems:
			if not item:
				continue

			# Start position in the index buffer (start of heel or sole positions):
			startIndex = 0
			endIndex = 0
			# Number of index to generate (for line strip, or triangle list)
			numIndex = 0
			isWireFrame = True

			if item.name() == footPrintGeometryOverride.wireframeHeelItemName_:
				numIndex = heelCount
			elif item.name() == footPrintGeometryOverride.wireframeSoleItemName_:
				startIndex = heelCount
				numIndex = soleCount
			elif item.name() == footPrintGeometryOverride.shadedHeelItemName_:
				numIndex = heelCount - 2
				startIndex = 1
				endIndex = heelCount - 2
				isWireFrame = False
			elif item.name() == footPrintGeometryOverride.shadedSoleItemName_:
				startIndex = heelCount
				endIndex = heelCount + soleCount - 2
				numIndex = soleCount - 2
				isWireFrame = False

			if numIndex:
				indexBuffer = data.createIndexBuffer(omr.MGeometry.kUnsignedInt32)
				indicesAddress = indexBuffer.acquire(numIndex, True)
				indices = (ctypes.c_uint * numIndex).from_address(indicesAddress)
				
				i = 0
				while i < numIndex:
					if isWireFrame:
						# Line strip starts at first position and iterates thru all vertices:
						indices[i] = startIndex + i
						i += 1
					else:
						# Triangle strip
						indices[i] = startIndex + i/2
						if i+1 < numIndex:
							indices[i+1] = endIndex - i/2
						i += 2

				indexBuffer.commit(indicesAddress)
				item.associateWithIndexBuffer(indexBuffer)

		mMultiplierChanged = False

def initializePlugin(obj):
	plugin = om.MFnPlugin(obj, "Autodesk", "3.0", "Any")

	try:
		plugin.registerNode("footPrint", footPrint.id, footPrint.creator, footPrint.initialize, om.MPxNode.kLocatorNode, footPrint.drawDbClassification)
	except:
		sys.stderr.write("Failed to register node\n")
		raise

	try:
		omr.MDrawRegistry.registerGeometryOverrideCreator(footPrint.drawDbClassification, footPrint.drawRegistrantId, footPrintGeometryOverride.creator)
	except:
		sys.stderr.write("Failed to register override\n")
		raise

def uninitializePlugin(obj):
	plugin = om.MFnPlugin(obj)

	try:
		plugin.deregisterNode(footPrint.id)
	except:
		sys.stderr.write("Failed to deregister node\n")
		pass

	try:
		omr.MDrawRegistry.deregisterGeometryOverrideCreator(footPrint.drawDbClassification, footPrint.drawRegistrantId)
	except:
		sys.stderr.write("Failed to deregister override\n")
		pass

