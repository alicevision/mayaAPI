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
try:
	from OpenGL.GL import *
except:
	pass
import maya.api.OpenMaya as om
import maya.api.OpenMayaUI as omui
import maya.api.OpenMayaRender as omr

def maya_useNewAPI():
	"""
	The presence of this function tells Maya that the plugin produces, and
	expects to be passed, objects created using the Maya Python API 2.0.
	"""
	pass


def matrixAsArray(matrix):
	array = []
	for i in range(16):
		array.append(matrix[i])
	return array

def drawBoundingBox(box, color):
	min = om.MFloatPoint( box.min )
	max = om.MFloatPoint( box.max )

	glPushAttrib(GL_ALL_ATTRIB_BITS)

	glDisable( GL_LIGHTING )
	glColor3f( color.r, color.g, color.b)
	glBegin( GL_LINE_STRIP )
	glVertex3f( min.x, min.y, min.z )
	glVertex3f( max.x, min.y, min.z )
	glVertex3f( max.x, max.y, min.z )
	glVertex3f( min.x, max.y, min.z )
	glVertex3f( min.x, min.y, min.z )
	glVertex3f( min.x, min.y, max.z )
	glVertex3f( min.x, max.y, max.z )
	glVertex3f( min.x, max.y, min.z )
	glVertex3f( max.x, max.y, min.z )
	glVertex3f( max.x, max.y, max.z )
	glVertex3f( max.x, min.y, max.z )
	glVertex3f( max.x, min.y, min.z )
	glVertex3f( max.x, min.y, max.z )
	glVertex3f( min.x, min.y, max.z )
	glVertex3f( min.x, max.y, max.z )
	glVertex3f( max.x, max.y, max.z )
	glEnd()

	glPopAttrib()

class hwColorPerVertexShader(omui.MPxHwShaderNode):
	id = om.MTypeId(0x00105450)

	# Attributes
	aColorGain = None
	aColorBias = None
	aTranspGain = None
	aTranspBias = None
	aNormalsPerVertex = None
	aColorsPerVertex = None
	aColorSetName = None
	aTexRotateX = None
	aTexRotateY = None
	aTexRotateZ = None
	aDrawBoundingBox = None

	def __init__(self):
		omui.MPxHwShaderNode.__init__(self)

		# Cached internal values
		self.mColorGain = [1.0, 0.0, 0.0]
		self.mColorBias = [0.0, 0.0, 0.0]
		self.mTranspGain = 1.0
		self.mTranspBias = 0.0

		self.mNormalsPerVertex = 0
		self.mColorsPerVertex = 0
		self.mColorSetName = ""
		self.mTexRotateX = 0.0
		self.mTexRotateY = 0.0
		self.mTexRotateZ = 0.0

		self.mDrawBoundingBox = False

		self.mAttributesChanged = False

		self.mSwatchShader = None

	def __del__(self):
		if self.mSwatchShader is not None:
			shaderMgr = omr.MRenderer.getShaderManager()
			if shaderMgr is not None:
				shaderMgr.releaseShader(self.mSwatchShader)
			self.mSwatchShader = None

	## The creator method creates an instance of the
	## hwColorPerVertexShader class and is the first method called by Maya
	## when a hwColorPerVertexShader needs to be created.
	##
	@staticmethod
	def creator():
		return hwColorPerVertexShader()

	## The initialize routine is called after the node has been created.
	## It sets up the input and output attributes and adds them to the node.
	## Finally the dependencies are arranged so that when the inputs
	## change Maya knowns to call compute to recalculate the output values.
	##
	@staticmethod
	def initialize():
		nAttr = om.MFnNumericAttribute()
		tAttr = om.MFnTypedAttribute()

		## Create input attributes.
		## All attributes are cached internal
		##
		hwColorPerVertexShader.aColorGain = nAttr.createColor( "colorGain", "cg")
		nAttr.storable = True
		nAttr.keyable = True
		nAttr.default = (1.0, 1.0, 1.0)
		nAttr.cached = True
		nAttr.internal = True
		nAttr.affectsAppearance = True

		hwColorPerVertexShader.aTranspGain = nAttr.create("transparencyGain", "tg", om.MFnNumericData.kFloat, 1.0)
		nAttr.storable = True
		nAttr.keyable = True
		nAttr.default = 1.0
		nAttr.setSoftMin(0.0)
		nAttr.setSoftMax(2.0)
		nAttr.cached = True
		nAttr.internal = True
		nAttr.affectsAppearance = True

		hwColorPerVertexShader.aColorBias = nAttr.createColor( "colorBias", "cb")
		nAttr.storable = True
		nAttr.keyable = True
		nAttr.default = (0.0, 0.0, 0.0)
		nAttr.cached = True
		nAttr.internal = True
		nAttr.affectsAppearance = True

		hwColorPerVertexShader.aTranspBias = nAttr.create( "transparencyBias", "tb", om.MFnNumericData.kFloat, 0.0)
		nAttr.storable = True
		nAttr.keyable = True
		nAttr.default = 0.0
		nAttr.setSoftMin(-1.0)
		nAttr.setSoftMax(1.0)
		nAttr.cached = True
		nAttr.internal = True
		nAttr.affectsAppearance = True

		hwColorPerVertexShader.aNormalsPerVertex = nAttr.create("normalsPerVertex", "nv", om.MFnNumericData.kInt, 0)
		nAttr.storable = True
		nAttr.keyable = False
		nAttr.default = 0
		nAttr.setSoftMin(0)
		nAttr.setSoftMax(3)
		nAttr.cached = True
		nAttr.internal = True
		nAttr.affectsAppearance = True

		hwColorPerVertexShader.aColorsPerVertex = nAttr.create("colorsPerVertex", "cv", om.MFnNumericData.kInt, 0)
		nAttr.storable = True
		nAttr.keyable = False
		nAttr.default = 0
		nAttr.setSoftMin(0)
		nAttr.setSoftMax(5)
		nAttr.cached = True
		nAttr.internal = True
		nAttr.affectsAppearance = True

		hwColorPerVertexShader.aColorSetName = tAttr.create("colorSetName", "cs", om.MFnData.kString, om.MObject.kNullObj)
		tAttr.storable = True
		tAttr.keyable = False
		tAttr.cached = True
		tAttr.internal = True
		tAttr.affectsAppearance = True

		hwColorPerVertexShader.aTexRotateX = nAttr.create( "texRotateX", "tx", om.MFnNumericData.kFloat, 0.0)
		nAttr.storable = True
		nAttr.keyable = True
		nAttr.default = 0.0
		nAttr.cached = True
		nAttr.internal = True
		nAttr.affectsAppearance = True

		hwColorPerVertexShader.aTexRotateY = nAttr.create( "texRotateY", "ty", om.MFnNumericData.kFloat, 0.0)
		nAttr.storable = True
		nAttr.keyable = True
		nAttr.default = 0.0
		nAttr.cached = True
		nAttr.internal = True
		nAttr.affectsAppearance = True

		hwColorPerVertexShader.aTexRotateZ = nAttr.create( "texRotateZ", "tz", om.MFnNumericData.kFloat, 0.0)
		nAttr.storable = True
		nAttr.keyable = True
		nAttr.default = 0.0
		nAttr.cached = True
		nAttr.internal = True
		nAttr.affectsAppearance = True

		hwColorPerVertexShader.aDrawBoundingBox = nAttr.create( "drawBoundingBox", "dbb", om.MFnNumericData.kBoolean, False)
		nAttr.storable = True
		nAttr.keyable = True
		nAttr.default = False
		nAttr.cached = True
		nAttr.internal = True
		nAttr.affectsAppearance = True

		## create output attributes here
		## outColor is the only output attribute and it is inherited
		## so we do not need to create or add it.
		##

		## Add the attributes here

		om.MPxNode.addAttribute(hwColorPerVertexShader.aColorGain)
		om.MPxNode.addAttribute(hwColorPerVertexShader.aTranspGain)
		om.MPxNode.addAttribute(hwColorPerVertexShader.aColorBias)
		om.MPxNode.addAttribute(hwColorPerVertexShader.aTranspBias)
		om.MPxNode.addAttribute(hwColorPerVertexShader.aNormalsPerVertex)
		om.MPxNode.addAttribute(hwColorPerVertexShader.aColorsPerVertex)
		om.MPxNode.addAttribute(hwColorPerVertexShader.aColorSetName)
		om.MPxNode.addAttribute(hwColorPerVertexShader.aTexRotateX)
		om.MPxNode.addAttribute(hwColorPerVertexShader.aTexRotateY)
		om.MPxNode.addAttribute(hwColorPerVertexShader.aTexRotateZ)
		om.MPxNode.addAttribute(hwColorPerVertexShader.aDrawBoundingBox)

		om.MPxNode.attributeAffects (hwColorPerVertexShader.aColorGain,  omui.MPxHwShaderNode.outColor)
		om.MPxNode.attributeAffects (hwColorPerVertexShader.aTranspGain, omui.MPxHwShaderNode.outColor)
		om.MPxNode.attributeAffects (hwColorPerVertexShader.aColorBias,  omui.MPxHwShaderNode.outColor)
		om.MPxNode.attributeAffects (hwColorPerVertexShader.aTranspBias, omui.MPxHwShaderNode.outColor)
		om.MPxNode.attributeAffects (hwColorPerVertexShader.aNormalsPerVertex, omui.MPxHwShaderNode.outColor)
		om.MPxNode.attributeAffects (hwColorPerVertexShader.aColorsPerVertex, omui.MPxHwShaderNode.outColor)
		om.MPxNode.attributeAffects (hwColorPerVertexShader.aColorSetName, omui.MPxHwShaderNode.outColor)
		om.MPxNode.attributeAffects (hwColorPerVertexShader.aTexRotateX, omui.MPxHwShaderNode.outColor)
		om.MPxNode.attributeAffects (hwColorPerVertexShader.aTexRotateY, omui.MPxHwShaderNode.outColor)
		om.MPxNode.attributeAffects (hwColorPerVertexShader.aTexRotateZ, omui.MPxHwShaderNode.outColor)
		om.MPxNode.attributeAffects (hwColorPerVertexShader.aDrawBoundingBox, omui.MPxHwShaderNode.outColor)

	## The compute method is called by Maya when the input values
	## change and the output values need to be recomputed.
	##
	## In this case this is only used for software shading, when the to
	## compute the rendering swatches.
	def compute(self, plug, data):
		## Check that the requested recompute is one of the output values
		##
		if (plug != omui.MPxHwShaderNode.outColor) and (plug.parent() != omui.MPxHwShaderNode.outColor):
			return None

		inputData = data.inputValue(hwColorPerVertexShader.aColorGain)
		color = inputData.asFloatVector()

		outColorHandle = data.outputValue(hwColorPerVertexShader.outColor)
		outColorHandle.setMFloatVector(color)

		data.setClean(plug)

	def postConstructor(self):
		self.setMPSafe(False)

	## Internally cached attribute handling routines
	def getInternalValueInContext(self, plug, handle, ctx):
		handledAttribute = False

		if (plug == hwColorPerVertexShader.aColorGain):
			handledAttribute = True
			handle.set3Float( self.mColorGain[0], self.mColorGain[1], self.mColorGain[2] )

		elif (plug == hwColorPerVertexShader.aColorBias):
			handledAttribute = True
			handle.set3Float( self.mColorBias[0], self.mColorBias[1], self.mColorBias[2] )

		elif (plug == hwColorPerVertexShader.aTranspGain):
			handledAttribute = True
			handle.setFloat( self.mTranspGain )

		elif (plug == hwColorPerVertexShader.aTranspBias):
			handledAttribute = True
			handle.setFloat( self.mTranspBias )

		elif (plug == hwColorPerVertexShader.aNormalsPerVertex):
			handledAttribute = True
			handle.setInt( self.mNormalsPerVertex )

		elif (plug == hwColorPerVertexShader.aColorsPerVertex):
			handledAttribute = True
			handle.setInt( self.mColorsPerVertex )

		elif (plug == hwColorPerVertexShader.aColorSetName):
			handledAttribute = True
			handle.setString( self.mColorSetName )

		elif (plug == hwColorPerVertexShader.aTexRotateX):
			handledAttribute = True
			handle.setFloat( self.mTexRotateX )

		elif (plug == hwColorPerVertexShader.aTexRotateY):
			handledAttribute = True
			handle.setFloat( self.mTexRotateY )

		elif (plug == hwColorPerVertexShader.aTexRotateZ):
			handledAttribute = True
			handle.setFloat( self.mTexRotateZ )

		elif (plug == hwColorPerVertexShader.aDrawBoundingBox):
			handledAttribute = True
			handle.setBool( self.mDrawBoundingBox )

		return handledAttribute

	def setInternalValueInContext(self, plug, handle, ctx):
		handledAttribute = False

		if (plug == hwColorPerVertexShader.aNormalsPerVertex):
			handledAttribute = True
			self.mNormalsPerVertex = handle.asInt()

		elif (plug == hwColorPerVertexShader.aColorsPerVertex):
			handledAttribute = True
			self.mColorsPerVertex = handle.asInt()

		elif (plug == hwColorPerVertexShader.aColorSetName):
			handledAttribute = True
			self.mColorSetName = handle.asString()

		elif (plug == hwColorPerVertexShader.aTexRotateX):
			handledAttribute = True
			self.mTexRotateX = handle.asFloat()

		elif (plug == hwColorPerVertexShader.aTexRotateY):
			handledAttribute = True
			self.mTexRotateY = handle.asFloat()

		elif (plug == hwColorPerVertexShader.aTexRotateZ):
			handledAttribute = True
			self.mTexRotateZ = handle.asFloat()

		elif (plug == hwColorPerVertexShader.aColorGain):
			handledAttribute = True
			val = handle.asFloat3()
			if val != self.mColorGain:
				self.mColorGain = val
				self.mAttributesChanged = True

		elif (plug == hwColorPerVertexShader.aColorBias):
			handledAttribute = True
			val = handle.asFloat3()
			if val != self.mColorBias:
				self.mColorBias = val
				self.mAttributesChanged = True

		elif (plug == hwColorPerVertexShader.aTranspGain):
			handledAttribute = True
			val = handle.asFloat()
			if val != self.mTranspGain:
				self.mTranspGain = val
				self.mAttributesChanged = True

		elif (plug == hwColorPerVertexShader.aTranspBias):
			handledAttribute = True
			val = handle.asFloat()
			if val != self.mTranspBias:
				self.mTranspBias = val
				self.mAttributesChanged = True

		elif (plug == hwColorPerVertexShader.aDrawBoundingBox):
			handledAttribute = True
			val = handle.asBool()
			if val != self.mDrawBoundingBox:
				self.mDrawBoundingBox = val
				self.mAttributesChanged = True

		return handledAttribute

	def bind(self, request, view):
		return

	def unbind(self, request, view):
		return

	def geometry(self, request, view, prim, writable, indexCount, indexArray, vertexCount, vertexIDs, vertexArray, normalCount, normalArrays, colorCount, colorArrays, texCoordCount, texCoordArrays):

		if self.mDrawBoundingBox:
			points = ((ctypes.c_float * 3)*vertexCount).from_buffer(vertexArray)

			# Compute the bounding box.
			box = om.MBoundingBox()
			for i in range(vertexCount):
				point = points[i]
				box.expand( om.MPoint(point[0], point[1], point[2]) )

			# Draw the bounding box.
			wireColor = om.MColor( (0.1, 0.15, 0.35) )
			drawBoundingBox( box, wireColor )

		## If we've received a color, that takes priority
		##
		if colorCount > 0 and colorArrays[ colorCount - 1] is not None:
			indexArray  = (ctypes.c_uint).from_buffer(indexArray)
			vertexArray = (ctypes.c_float * 3).from_buffer(vertexArray)
			colorArray  = (ctypes.c_float * 4).from_buffer(colorArrays[colorCount - 1])

			glPushAttrib(GL_ALL_ATTRIB_BITS)
			glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT)
			glDisable(GL_LIGHTING)

			glEnableClientState(GL_COLOR_ARRAY)
			glColorPointer( 4, GL_FLOAT, 0, colorArray )

			glEnableClientState(GL_VERTEX_ARRAY)
			glVertexPointer ( 3, GL_FLOAT, 0, vertexArray )
			glDrawElements ( prim, indexCount, GL_UNSIGNED_INT, indexArray )

			glDisableClientState(GL_COLOR_ARRAY)

			glPopClientAttrib()
			glPopAttrib()
			return

		## If this attribute is enabled, normals, tangents,
		## and binormals can be visualized using false coloring.
		## Negative values will clamp to black however.
		if normalCount > self.mNormalsPerVertex:
			normalCount = self.mNormalsPerVertex

		if normalCount > 0:
			indexArray  = (ctypes.c_uint).from_buffer(indexArray)
			vertexArray = (ctypes.c_float * 3).from_buffer(vertexArray)
			normalArray = (ctypes.c_float * 3).from_buffer(normalArrays[normalCount - 1])

			glPushAttrib(GL_ALL_ATTRIB_BITS)
			glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT)
			glDisable(GL_LIGHTING)

			glEnableClientState(GL_COLOR_ARRAY)
			glColorPointer(3, GL_FLOAT, 0, normalArray )

			glEnableClientState(GL_VERTEX_ARRAY)
			glVertexPointer ( 3, GL_FLOAT, 0, vertexArray )
			glDrawElements ( prim, indexCount, GL_UNSIGNED_INT, indexArray )

			glDisableClientState(GL_COLOR_ARRAY)

			glPopClientAttrib()
			glPopAttrib()
			return

		self.draw( prim, writable, indexCount, indexArray, vertexCount, vertexArray, colorCount, colorArrays )

	## Batch overrides
	def glBind(self, shapePath):
		return

	def glUnbind(self, shapePath):
		return

	def glGeometry(self, path, prim, writable, indexCount, indexArray, vertexCount, vertexIDs, vertexArray, normalCount, normalArrays, colorCount, colorArrays, texCoordCount, texCoordArrays):
		## If this attribute is enabled, normals, tangents,
		## and binormals can be visualized using false coloring.
		## Negative values will clamp to black however.
		if normalCount > self.mNormalsPerVertex:
			normalCount = self.mNormalsPerVertex

		if normalCount > 0:
			indexArray  = (ctypes.c_uint).from_buffer(indexArray)
			vertexArray = (ctypes.c_float * 3).from_buffer(vertexArray)
			normalArray = (ctypes.c_float * 3).from_buffer(normalArrays[normalCount - 1])

			glPushAttrib(GL_ALL_ATTRIB_BITS)
			glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT)
			glDisable(GL_LIGHTING)
			glDisable(GL_BLEND)

			glEnableClientState(GL_COLOR_ARRAY)
			glColorPointer(3, GL_FLOAT, 0, normalArray )

			glEnableClientState(GL_VERTEX_ARRAY)
			glVertexPointer ( 3, GL_FLOAT, 0, vertexArray )
			glDrawElements ( prim, indexCount, GL_UNSIGNED_INT, indexArray )

			glDisableClientState(GL_COLOR_ARRAY)

			glPopClientAttrib()
			glPopAttrib()
			return

		self.draw( prim, writable, indexCount, indexArray, vertexCount, vertexArray, colorCount, colorArrays )

	## Overridden to draw an image for swatch rendering.
	##
	def renderSwatchImage(self, outImage):
		return omr.MRenderUtilities.renderMaterialViewerGeometry("meshSphere",
																 self.thisMObject(), 
																 outImage,
																 lightRig=omr.MRenderUtilities.kSwatchLight )

	def	draw(self, prim, writable, indexCount, indexArray, vertexCount, vertexArray, colorCount, colorArrays):
		## Should check this value to allow caching
		## of color values.
		self.mAttributesChanged = False

		## We assume triangles here.
		##
		if (vertexCount == 0) or not ((prim == GL_TRIANGLES) or (prim == GL_TRIANGLE_STRIP)):
			raise ValueError("kFailure")

		indexArray  = (ctypes.c_uint).from_buffer(indexArray)
		vertexArray = (ctypes.c_float * 3).from_buffer(vertexArray)

		glPushAttrib( GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT )
		glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT)
		glDisable(GL_LIGHTING)

		glEnableClientState(GL_VERTEX_ARRAY)
		glVertexPointer ( 3, GL_FLOAT, 0, vertexArray )

		## Do "cheesy" multi-pass here for more than one color set
		##
		if colorCount <= 1:
			glDisable(GL_BLEND)
			if colorCount > 0 and colorArrays[0] is not None:
				colorArray = (ctypes.c_float * 4).from_buffer(colorArrays[0])
				glEnableClientState(GL_COLOR_ARRAY)
				glColorPointer(4, GL_FLOAT, 0, colorArray)
			else:
				glColor4f(1.0, 0.5, 1.0, 1.0)
			glDrawElements( prim, indexCount, GL_UNSIGNED_INT, indexArray )

		else:
			## Do a 1:1 Blend if we have more than on color set available.
			glEnable(GL_BLEND)
			glBlendFunc(GL_ONE, GL_ONE)
			glEnableClientState(GL_COLOR_ARRAY)

			for i in range(colorCount):
				if colorArrays[i] is not None:
					## Apply gain and bias
					##
					colorArray = (ctypes.c_float * 4).from_buffer(colorArrays[i])

					if self.mColorGain != [1.0, 1.0, 1.0] or self.mColorBias != [0.0, 0.0, 0.0] or self.mTranspGain != 1.0 or self.mTranspBias != 0.0:
						## This sample code is a CPU bottlneck. It could be
						## replaced with a vertex program or color matrix
						## operator.
						##

						## We really want to scale 1-transp.
						## T = 1 - ((1-T)*gain + bias) =
						##   = T * gain + 1 - gain - bias
						biasT = 1 - self.mTranspGain - self.mTranspBias

						## Either make a copy or read directly colors.
						##
						if not (writable & omui.MPxHwShaderNode.kWriteColorArrays):
							colorArray = []
							origColors = ((ctypes.c_float * 4)*vertexCount).from_buffer(colorArrays[i])
							for ii in range(vertexCount):
								origColor = origColors[ii]
								colorArray.append( (origColor[0] * self.mColorGain[0]) + self.mColorBias[0] )
								colorArray.append( (origColor[1] * self.mColorGain[1]) + self.mColorBias[1] )
								colorArray.append( (origColor[2] * self.mColorGain[2]) + self.mColorBias[2] )
								colorArray.append( (origColor[3] * self.mTranspGain) + biasT )
						else:
							origColors = ((ctypes.c_float * 4)*vertexCount).from_buffer(colorArrays[i])
							for ii in range(vertexCount):
								origColor = origColors[ii]
								origColor[0] = (origColor[0] * self.mColorGain[0]) + self.mColorBias[0]
								origColor[1] = (origColor[1] * self.mColorGain[1]) + self.mColorBias[1]
								origColor[2] = (origColor[2] * self.mColorGain[2]) + self.mColorBias[2]
								origColor[3] = (origColor[3] * self.mTranspGain) + biasT

					glDisable(GL_BLEND)
					glEnableClientState(GL_COLOR_ARRAY)
					glColorPointer(4, GL_FLOAT, 0, colorArray)

				else:
					glDisable(GL_BLEND)
					glDisableClientState(GL_COLOR_ARRAY)
					glColor4f(1.0, 1.0, 1.0, 1.0)

				glDrawElements ( prim, indexCount, GL_UNSIGNED_INT, indexArray )

		glDisable(GL_BLEND)
		glPopClientAttrib()
		glPopAttrib()


	## Tells maya that color per vertex will be needed.
	def colorsPerVertex(self):
		## Going to be displaying false coloring,
		## so skip getting internal colors.
		if self.mNormalsPerVertex:
			return 0

		numColors = 0

		path = self.currentPath()
		if path.hasFn( om.MFn.kMesh ):
			fnMesh = om.MFnMesh( path.node() )
			numColorSets = fnMesh.numColorSets
			if numColorSets < 2:
				numColors = numColorSets
			else:
				numColors = 2

		return numColors

	## Tells maya that color per vertex will be needed.
	def hasTransparency(self):
		return True

	## Tells maya that color per vertex will be needed.
	def normalsPerVertex(self):
		numNormals = self.mNormalsPerVertex

		path = self.currentPath()
		if path.hasFn( om.MFn.kMesh ):
			fnMesh = om.MFnMesh( path.node() )
			## Check the # of uvsets. If no uvsets
			## then can't return tangent or binormals
			##
			if fnMesh.numUVSets == 0:
				## Put out a warnnig if we're asking for too many uvsets.
				dispWarn  = "Asking for more uvsets then available for shape: "
				dispWarn += path.fullPathName()
				om.MGlobal.displayWarning( dispWarn )

				if self.mNormalsPerVertex:
					numNormals = 1
				else:
					numNormals = 0

		return numNormals

	## Tells maya that texCoords per vertex will be needed.
	def texCoordsPerVertex(self):
		return 0

	def colorSetName(self):
		return self.mColorSetName

	def wantDrawBoundingBox(self):
		return self.mDrawBoundingBox


class hwCPVShaderOverride(omr.MPxShaderOverride):
	## Current hwColorPerVertexShader node associated with the shader override.
	## Updated during doDG() time.
	fShaderNode = None

	## Blend state will be used in the draw
	fBlendState = None

	def __init__(self, obj):
		omr.MPxShaderOverride.__init__(self, obj)

	def __del__(self):
		self.fShaderNode = None
		self.fBlendState = None

	@staticmethod
	def creator(obj):
		return hwCPVShaderOverride(obj)

	# 1) Initialize phase
	def initialize(self, shader):
		# Retrieve and cache the actual node pointer
		if not shader.isNull():
			self.fShaderNode = omui.MPxHwShaderNode.getHwShaderNode(shader)

		# Set position requirement
		reqName = ""
		self.addGeometryRequirement( omr.MVertexBufferDescriptor(reqName, omr.MGeometry.kPosition, omr.MGeometry.kFloat, 3) )

		# Set correct color requirement
		if self.fShaderNode is not None:
			reqName = self.fShaderNode.colorSetName()
		self.addGeometryRequirement( omr.MVertexBufferDescriptor(reqName, omr.MGeometry.kColor, omr.MGeometry.kFloat, 4) )

		return "Autodesk Maya hwColorPerVertexShader"

	# 2) Update phase -- Not implemented, nothing to do since we explicitly
	#                    rebuild on every update (thus triggering the
	#                    initialize method each time).

	# 3) Draw phase
	def draw(self, context, renderItemList):
		##--------------------------
		## Matrix set up
		##--------------------------
		## Set world view matrix
		glMatrixMode(GL_MODELVIEW)
		glPushMatrix()
		transform = context.getMatrix(omr.MFrameContext.kWorldViewMtx)
		glLoadMatrixd( matrixAsArray(transform) )

		## Set projection matrix
		glMatrixMode(GL_PROJECTION)
		glPushMatrix()
		projection = context.getMatrix(omr.MFrameContext.kProjectionMtx)
		glLoadMatrixd( matrixAsArray(projection) )

		##--------------------------
		## State set up
		##--------------------------
		stateMgr = context.getStateManager()

		## Blending
		## Save current blend status
		curBlendState = stateMgr.getBlendState()
		if curBlendState is None:
			return False

		if self.fBlendState is None:
			## If we haven't got a blending state, then acquire a new one.
			## Since it would be expensive to acquire the state, we would
			## store it.
			## Create a new blend status and acquire blend state from the context
			blendDesc = omr.MBlendStateDesc()
			for i in range(omr.MBlendState.kMaxTargets):
				blendDesc.targetBlends[i].blendEnable = True
				blendDesc.targetBlends[i].sourceBlend = omr.MBlendState.kSourceAlpha
				blendDesc.targetBlends[i].destinationBlend = omr.MBlendState.kInvSourceAlpha
				blendDesc.targetBlends[i].blendOperation = omr.MBlendState.kAdd
				blendDesc.targetBlends[i].alphaSourceBlend = omr.MBlendState.kOne
				blendDesc.targetBlends[i].alphaDestinationBlend = omr.MBlendState.kInvSourceAlpha
				blendDesc.targetBlends[i].alphaBlendOperation = omr.MBlendState.kAdd

			blendDesc.blendFactor = (1.0, 1.0, 1.0, 1.0)

			self.fBlendState = stateMgr.acquireBlendState(blendDesc)
			if self.fBlendState is None:
				return False

		## Activate the blend on the device
		stateMgr.setBlendState(self.fBlendState)

		## Bounding box draw
		if self.fShaderNode.wantDrawBoundingBox():
			for renderItem in renderItemList:
				if renderItem is None:
					continue

				## Modelview matrix is already set so just use the object
				## space bbox.
				box = renderItem.boundingBox(om.MSpace.kObject)

				## Draw the bounding box.
				wireColor = om.MColor( (0.1, 0.15, 0.35) )
				drawBoundingBox( box, wireColor )

		##--------------------------
		## geometry draw
		##--------------------------
		useCustomDraw = False
		if useCustomDraw:
			## Custom draw
			## Does not set state, matrix or material
			self.customDraw(context, renderItemList)

		else:
			## Internal standard draw
			## Does not set state, matrix or material
			self.drawGeometry(context)


		##--------------------------
		## Matrix restore
		##--------------------------
		glPopMatrix()
		glMatrixMode(GL_MODELVIEW)
		glPopMatrix()


		##--------------------------
		## State restore
		##--------------------------
		## Restore blending
		stateMgr.setBlendState(curBlendState)

		return True

	# draw helper method
	def customDraw(self, context, renderItemList):
		glPushClientAttrib ( GL_CLIENT_ALL_ATTRIB_BITS )

		for renderItem in renderItemList:
			if renderItem is None:
				continue

			geometry = renderItem.geometry()
			if geometry is not None:
				hwCPVShaderOverride.customDrawGeometry(context, geometry, renderItem.primitive())
				continue

		glPopClientAttrib()

		return True

	# draw helper method
	@staticmethod
	def customDrawGeometry(context, geometry, indexPrimType):
		## Dump out vertex field information for each field
		##
		bufferCount = geometry.vertexBufferCount()

		boundData = True
		for i in range(bufferCount):
			buffer = geometry.vertexBuffer(i)
			if buffer is None:
				boundData = False
				break

			desc = buffer.descriptor()
			dataHandle = buffer.resourceHandle()
			if dataHandle == 0:
				boundData = False
				break

			dataBufferId = (ctypes.c_uint).from_address(dataHandle)
			fieldOffset = ctypes.c_void_p(desc.offset)
			fieldStride = desc.stride

			## Bind each data buffer
			glBindBuffer(GL_ARRAY_BUFFER, dataBufferId)
			currentError = glGetError()
			if currentError != GL_NO_ERROR:
				boundData = False

			if boundData:
				## Set the data pointers
				if desc.semantic == omr.MGeometry.kPosition:
					glEnableClientState(GL_VERTEX_ARRAY)
					glVertexPointer(3, GL_FLOAT, fieldStride*4, fieldOffset)
					currentError = glGetError()
					if currentError != GL_NO_ERROR:
						boundData = False
				elif desc.semantic == omr.MGeometry.kColor:
					glEnableClientState(GL_COLOR_ARRAY)
					glColorPointer(4, GL_FLOAT, fieldStride*4, fieldOffset)
					currentError = glGetError()
					if currentError != GL_NO_ERROR:
						boundData = False

			if not boundData:
				break

		if boundData and geometry.indexBufferCount() > 0:
			## Dump out indexing information
			##
			buffer = geometry.indexBuffer(0)
			indexHandle = buffer.resourceHandle()
			indexBufferCount = 0
			indexBufferId = 0
			if indexHandle != 0:
				indexBufferId = (ctypes.c_uint).from_address(indexHandle)
				indexBufferCount = ctypes.c_int(buffer.size())

			## Bind the index buffer
			if indexBufferId and indexBufferId > 0:
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId)
				currentError = glGetError()
				if currentError == GL_NO_ERROR:
					indexPrimTypeGL = GL_TRIANGLES
					if indexPrimType == omr.MGeometry.kPoints:
						indexPrimTypeGL = GL_POINTS
					elif indexPrimType == omr.MGeometry.kLines:
						indexPrimTypeGL = GL_LINES
					elif indexPrimType == omr.MGeometry.kLineStrip:
						indexPrimTypeGL = GL_LINE_STRIP
					elif indexPrimType == omr.MGeometry.kTriangles:
						indexPrimTypeGL = GL_TRIANGLES
					elif indexPrimType == omr.MGeometry.kTriangleStrip:
						indexPrimTypeGL = GL_TRIANGLE_STRIP
					else:
						boundData = false

					if boundData:
						## Draw the geometry
						indexType = GL_UNSIGNED_SHORT
						if buffer.dataType() == omr.MGeometry.kUnsignedInt32:
							indexType = GL_UNSIGNED_INT
						glDrawElements(indexPrimTypeGL, indexBufferCount, indexType, ctypes.c_void_p(0))


	def rebuildAlways(self):
		# Since color set name changing needs to add a new
		# named requirement to the geometry, so we should
		# return True here to trigger the geometry rebuild.
		return True

	def supportedDrawAPIs(self):
		return omr.MRenderer.kOpenGL

	def isTransparent(self):
		return True


## The initializePlugin method is called by Maya when the
## plugin is loaded. It registers the hwColorPerVertexShader node
## which provides Maya with the creator and initialize methods to be
## called when a hwColorPerVertexShader is created.
##
sHWCPVShaderRegistrantId = "HWCPVShaderRegistrantId"

def initializePlugin(obj):
	plugin = om.MFnPlugin(obj, "Autodesk", "4.5", "Any")
	try:
		swatchName = omui.MHWShaderSwatchGenerator.initialize()
		userClassify = "shader/surface/utility/:drawdb/shader/surface/hwColorPerVertexShader:swatch/" + swatchName
		plugin.registerNode("hwColorPerVertexShader", hwColorPerVertexShader.id, hwColorPerVertexShader.creator, hwColorPerVertexShader.initialize, om.MPxNode.kHwShaderNode, userClassify)
	except:
		sys.stderr.write("Failed to register node\n")
		raise

	try:
		global sHWCPVShaderRegistrantId
		omr.MDrawRegistry.registerShaderOverrideCreator("drawdb/shader/surface/hwColorPerVertexShader", sHWCPVShaderRegistrantId, hwCPVShaderOverride.creator)
	except:
		sys.stderr.write("Failed to register override\n")
		raise


## The unitializePlugin is called when Maya needs to unload the plugin.
## It basically does the opposite of initialize by calling
## the deregisterNode to remove it.
##
def uninitializePlugin(obj):
	plugin = om.MFnPlugin(obj)
	try:
		plugin.deregisterNode(hwColorPerVertexShader.id)
	except:
		sys.stderr.write("Failed to deregister node\n")
		raise

	try:
		global sHWCPVShaderRegistrantId
		omr.MDrawRegistry.deregisterShaderOverrideCreator("drawdb/shader/surface/hwColorPerVertexShader", sHWCPVShaderRegistrantId)
	except:
		sys.stderr.write("Failed to deregister override\n")
		raise

