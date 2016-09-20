#-
# ===========================================================================
# Copyright 2015 Autodesk, Inc.  All rights reserved.
#
# Use of this software is subject to the terms of the Autodesk license
# agreement provided at the time of installation or download, or which
# otherwise accompanies this software in either electronic or hard copy form.
# ===========================================================================
#+

import sys, math, ctypes, collections
import maya.api.OpenMaya as om
import maya.api.OpenMayaUI as omui
import maya.api.OpenMayaRender as omr

def maya_useNewAPI():
	"""
	The presence of this function tells Maya that the plugin produces, and
	expects to be passed, objects created using the Maya Python API 2.0.
	"""
	pass

## helper function
def useSelectHighlight(selectedList, path):
	
	displayStatus = omr.MGeometryUtilities.displayStatus(path)
	if (displayStatus & (omr.MGeometryUtilities.kHilite | omr.MGeometryUtilities.kActiveComponent)) != 0:
		return True
	
	pathCopy = om.MDagPath(path)

	while pathCopy.length() > 0:
		if selectedList.hasItem(pathCopy):
			return True
		pathCopy.pop()

	return False

def floatApproxEqual(left, right):
	return abs(left - right) < 0.0001

################################################################################
##
## This class holds the underlying geometry for the shape or data.
## This is where geometry specific data and methods should go.
##
################################################################################
class apiMeshGeomUV:
	def __init__(self, other=None):
		if other:
			self.copy(other)
		else:
			self.reset()

	def uvId(self, fvi):
		return self.faceVertexIndex[fvi]

	def getUV(self, uvId):
		return [ self.ucoord[uvId], self.vcoord[uvId] ]

	def u(self, uvId):
		return self.ucoord[uvId]

	def v(self, uvId):
		return self.vcoord[uvId]

	def uvcount(self):
		return len(self.ucoord)

	def append_uv(self, u, v):
		self.ucoord.append( u ) 
		self.vcoord.append( v ) 

	def reset(self):
		self.ucoord = om.MFloatArray()
		self.vcoord = om.MFloatArray()
		self.faceVertexIndex = om.MIntArray()

	def copy(self, other):
		self.ucoord = om.MFloatArray(other.ucoord)
		self.vcoord = om.MFloatArray(other.vcoord)
		self.faceVertexIndex = om.MIntArray(other.faceVertexIndex)

class apiMeshGeom:
	def __init__(self):
		self.vertices = om.MPointArray()
		self.face_counts = om.MIntArray()
		self.face_connects = om.MIntArray()
		self.normals = om.MVectorArray()
		self.uvcoords = apiMeshGeomUV()
		self.faceCount = 0

	def copy(self, other):
		self.vertices = om.MPointArray(other.vertices)
		self.face_counts = om.MIntArray(other.face_counts)
		self.face_connects = om.MIntArray(other.face_connects)
		self.normals = om.MVectorArray(other.normals)
		self.uvcoords = apiMeshGeomUV(other.uvcoords)
		self.faceCount = other.faceCount

################################################################################
##
## Provides a data type for some arbitrary user geometry.
## 
## A users geometry class can exist in the DAG by creating an
## MPxSurfaceShape (and UI) class for it and can also be passed through
## DG connections by creating an MPxGeometryData class for it.
## 
## MPxGeometryData is the same as MPxData except it provides 
## additional methods to modify the geometry data via an iterator.
##
################################################################################
## Ascii file IO defines
##
kDblQteChar     = "\""
kSpaceChar      = "	"
kWrapString     = "\n\t\t"
kVertexKeyword  = "v"
kNormalKeyword  = "vn"
kTextureKeyword = "vt"
kFaceKeyword    = "face"
kUVKeyword      = "uv" 

class apiMeshGeomIterator(om.MPxGeometryIterator):
	def __init__(self, userGeometry, components):
		om.MPxGeometryIterator.__init__(self, userGeometry, components)
		self.geometry = userGeometry
		self.reset()

	def reset(self):
		## Resets the iterator to the start of the components so that another
		## pass over them may be made.
		##
		om.MPxGeometryIterator.reset(self)
		self.currentPoint = 0
		if self.geometry:
			maxVertex = len(self.geometry.vertices)
			self.maxPoints = maxVertex

	def point(self):
		## Returns the point for the current element in the iteration.
		## This is used by the transform tools for positioning the
		## manipulator in component mode. It is also used by deformers.	 
		##
		pnt = om.MPoint()
		if self.geometry:
			pnt = self.geometry.vertices[ self.index() ]
		return pnt

	def setPoint(self, pnt):
		## Set the point for the current element in the iteration.
		## This is used by deformers.	 
		##
		if self.geometry:
			self.geometry.vertices[ self.index() ] = pnt

	def iteratorCount(self):
		## Return the number of vertices in the iteration.
		## This is used by deformers such as smooth skinning
		##
		if self.geometry:
			return len(self.geometry.vertices)
		return 0

	def hasPoints(self):
		## Returns true since the shape data has points.
		##
		return True

class apiMeshData(om.MPxGeometryData):
	typeName = "apiMeshData"
	id = om.MTypeId(0x80777)

	@staticmethod
	def creator():
		return apiMeshData()

	def __init__(self):
		om.MPxGeometryData.__init__(self)
		self.fGeometry = apiMeshGeom()

	def __del__(self):
		self.fGeometry = None

	def readASCII(self, argList, idx):
		idx = self.readVerticesASCII(argList, idx)
		idx = self.readNormalsASCII(argList, idx)
		idx = self.readFacesASCII(argList, idx)
		idx = self.readUVASCII(argList, idx)
		return idx

	def readBinary(self, inputData, length):
		## not implemented
		return 0

	def writeASCII(self):
		data  = self.writeVerticesASCII()
		data += self.writeNormalsASCII()
		data += self.writeFacesASCII()
		data += writeUVASCII()
		return data

	def writeBinary(self):
		## not implemented
		return bytearray()

	def copy(self, src):
		self.fGeometry.copy(src.fGeometry)
	
	def typeId(self):
		return apiMeshData.id

	def name(self):
		return apiMeshData.typeName

	##################################################################
	##
	## Overrides from MPxGeometryData
	##
	##################################################################

	def iterator(self, componentList, component, useComponents, world=None):
		if useComponents:
			return apiMeshGeomIterator(self.fGeometry, componentList)

		return apiMeshGeomIterator(self.fGeometry, component)
	
	##################################################################
	##
	## Helper methods
	##
	##################################################################

	def readVerticesASCII(self, argList, idx):
		geomStr = ""
		try:
			geomStr = argList.asString(idx)
		except:
			geomStr = ""
			pass

		if geomStr == kVertexKeyword:
			idx = argList.lastArgUsed()+1
			vertexCount = argList.asInt(idx)
			idx = argList.lastArgUsed()+1
			for i in xrange(vertexCount):
				vertex = argList.asPoint(idx)
				idx = argList.lastArgUsed()+1
				self.fGeometry.vertices.append(vertex)

		return idx

	def readNormalsASCII(self, argList, idx):
		geomStr = ""
		try:
			geomStr = argList.asString(idx)
		except:
			geomStr = ""
			pass

		if geomStr == kNormalKeyword:
			idx = argList.lastArgUsed()+1
			normalCount = argList.asInt(idx)
			idx = argList.lastArgUsed()+1
			for i in xrange(normalCount):
				normal = argList.asVector(idx)
				idx = argList.lastArgUsed()+1
				self.fGeometry.normals.append(normal)

		return idx

	def readFacesASCII(self, argList, idx):
		geomStr = ""
		try:
			geomStr = argList.asString(idx)
		except:
			geomStr = ""
			pass

		while geomStr == kFaceKeyword:
			idx = argList.lastArgUsed()+1
			faceCount = argList.asInt(idx)
			idx = argList.lastArgUsed()+1
			self.fGeometry.face_counts.append(faceCount)
			for i in xrange(faceCount):
				vid = argList.asInt(idx)
				idx = argList.lastArgUsed()+1
				self.fGeometry.face_connects.append(vid)

			try:
				geomStr = argList.asString(idx)
			except:
				geomStr = ""
				pass

		self.fGeometry.faceCount = len(self.fGeometry.face_counts)
		return idx

	def readUVASCII(self, argList, idx):
		self.fGeometry.uvcoords.reset()

		geomStr = ""
		try:
			geomStr = argList.asString(idx)
		except:
			geomStr = ""
			pass

		if geomStr == kUVKeyword:
			idx = argList.lastArgUsed()+1
			uvCount = argList.asInt(idx)
			idx = argList.lastArgUsed()+1
			faceVertexListCount = argList.asInt(idx)
			idx = argList.lastArgUsed()+1
			for i in xrange(uvCount):
				u = argList.asDouble(idx)
				idx = argList.lastArgUsed()+1
				v = argList.asDouble(idx)
				idx = argList.lastArgUsed()+1
				self.fGeometry.uvcoords.append_uv(u, v)

			for i in xrange(faceVertexListCount):
				fvi = argList.asInt(idx)
				idx = argList.lastArgUsed()+1
				self.fGeometry.uvcoords.faceVertexIndex.append( fvi )

		return idx

	def writeVerticesASCII(self):
		vertexCount = len(self.fGeometry.vertices)

		data  = "\n"
		data += kWrapString
		data += kDblQteChar + kVertexKeyword + kDblQteChar + kSpaceChar + str(vertexCount)

		for i in xrange(vertexCount):
			vertex = self.fGeometry.vertices[i]

			data += kWrapString
			data += str(vertex[0]) + kSpaceChar + str(vertex[1]) + kSpaceChar + str(vertex[2])

		return data

	def writeNormalsASCII(self):
		normalCount = len(self.fGeometry.normals)

		data  = "\n"
		data += kWrapString
		data += kDblQteChar + kNormalKeyword + kDblQteChar + kSpaceChar + str(normalCount)

		for i in xrange(normalCount):
			normal = self.fGeometry.normals[i]

			data += kWrapString
			data += str(normal[0]) + kSpaceChar + str(normal[1]) + kSpaceChar + str(normal[2])

		return data

	def writeFacesASCII(self):
		numFaces = len(self.fGeometry.face_counts)

		data = ""
		vid = 0

		for i in xrange(numFaces):
			faceVertexCount = self.fGeometry.face_counts[i]

			data += "\n"
			data += kWrapString
			data += kDblQteChar + kFaceKeyword + kDblQteChar + kSpaceChar + str(faceVertexCount)

			data += kWrapString

			for v in xrange(faceVertexCount):
				value = self.fGeometry.face_connects[vid]

				data += str(value) + kSpaceChar
				vid += 1

		return data

	def writeUVASCII(self):
		uvCount = self.fGeometry.uvcoords.uvcount()
		faceVertexCount = len(self.fGeometry.uvcoords.faceVertexIndex)

		data = ""

		if uvCount > 0:
			data  = "\n"
			data += kWrapString
			data += kDblQteChar + kUVKeyword + kDblQteChar + kSpaceChar + str(uvCount) + kSpaceChar + str(faceVertexCount)

			for i in xrange(uvCount):
				uv = self.fGeometry.uvcoords.getUV(i)

				data += kWrapString
				data += uv[0] + kSpaceChar + uv[1] + kSpaceChar

			for i in xrange(faceVertexCount):
				value = self.fGeometry.uvcoords.faceVertexIndex[i]

				data += kWrapString
				data += value + kSpaceChar

		return data


################################################################################
##
## apiMeshShape
##
## Implements a new type of shape node in maya called apiMesh.
##
## INPUTS
##     inputSurface    - input apiMeshData
##     outputSurface   - output apiMeshData
##     worldSurface    - array of world space apiMeshData, each element
##                       represents an istance of the shape
## OUTPUTS
##     mControlPoints  - inherited control vertices for the mesh. These values
##                       are tweaks (offsets) that will be applied to the
##                       vertices of the input shape.
##     bboxCorner1     - bounding box upper left corner
##     bboxCorner2     - bounding box lower right corner
##
################################################################################
class apiMesh(om.MPxSurfaceShape):
	id = om.MTypeId(0x80099)

	##########################################################
	##
	## Attributes
	##
	##########################################################
	inputSurface = None
	outputSurface = None
	worldSurface = None

	useWeightedTransformUsingFunction = None
	useWeightedTweakUsingFunction = None

	## used to support tweaking of points, the inputSurface attribute data is
	## transferred into the cached surface when it is dirty. The control points
	## tweaks are added into it there.
	##
	cachedSurface = None

	bboxCorner1 = None
	bboxCorner2 = None

	@staticmethod
	def creator():
		return apiMesh()

	@staticmethod
	def initialize():
		typedAttr = om.MFnTypedAttribute()
		numericAttr = om.MFnNumericAttribute()

		## ----------------------- INPUTS --------------------------
		apiMesh.inputSurface = typedAttr.create( "inputSurface", "is", apiMeshData.id, om.MObject.kNullObj )
		typedAttr.storable = False
		om.MPxNode.addAttribute( apiMesh.inputSurface )

		apiMesh.useWeightedTransformUsingFunction = numericAttr.create( "useWeightedTransformUsingFunction", "utru", om.MFnNumericData.kBoolean, True )
		numericAttr.keyable = True
		om.MPxNode.addAttribute( apiMesh.useWeightedTransformUsingFunction )

		apiMesh.useWeightedTweakUsingFunction = numericAttr.create( "useWeightedTweakUsingFunction", "utwu", om.MFnNumericData.kBoolean, True )
		numericAttr.keyable = True
		om.MPxNode.addAttribute( apiMesh.useWeightedTweakUsingFunction )

		## ----------------------- OUTPUTS -------------------------

		## bbox attributes
		##
		apiMesh.bboxCorner1 = numericAttr.create( "bboxCorner1", "bb1", om.MFnNumericData.k3Double, 0 )
		numericAttr.array = False
		numericAttr.usesArrayDataBuilder = False
		numericAttr.hidden = False
		numericAttr.keyable = False
		om.MPxNode.addAttribute( apiMesh.bboxCorner1 )

		apiMesh.bboxCorner2 = numericAttr.create( "bboxCorner2", "bb2", om.MFnNumericData.k3Double, 0 )
		numericAttr.array = False
		numericAttr.usesArrayDataBuilder = False
		numericAttr.hidden = False
		numericAttr.keyable = False
		om.MPxNode.addAttribute( apiMesh.bboxCorner2 )

		## local/world output surface attributes
		##
		apiMesh.outputSurface = typedAttr.create( "outputSurface", "os", apiMeshData.id, om.MObject.kNullObj )
		typedAttr.writable = False
		om.MPxNode.addAttribute( apiMesh.outputSurface )

		apiMesh.worldSurface = typedAttr.create( "worldSurface", "ws", apiMeshData.id, om.MObject.kNullObj )
		typedAttr.cached = False
		typedAttr.writable = False
		typedAttr.array = True
		typedAttr.usesArrayDataBuilder = True
		typedAttr.disconnectBehavior = om.MFnAttribute.kDelete
		typedAttr.worldSpace = True
		om.MPxNode.addAttribute( apiMesh.worldSurface )

		## Cached surface used for file IO
		##
		apiMesh.cachedSurface = typedAttr.create( "cachedSurface", "cs", apiMeshData.id, om.MObject.kNullObj )
		typedAttr.readable = True
		typedAttr.writable = True
		typedAttr.storable = True
		om.MPxNode.addAttribute( apiMesh.cachedSurface )

		## ---------- Specify what inputs affect the outputs ----------
		##
		om.MPxNode.attributeAffects( apiMesh.inputSurface, apiMesh.outputSurface )
		om.MPxNode.attributeAffects( apiMesh.inputSurface, apiMesh.worldSurface )
		om.MPxNode.attributeAffects( apiMesh.outputSurface, apiMesh.worldSurface )
		om.MPxNode.attributeAffects( apiMesh.inputSurface, apiMesh.bboxCorner1 )
		om.MPxNode.attributeAffects( apiMesh.inputSurface, apiMesh.bboxCorner2 )
		om.MPxNode.attributeAffects( apiMesh.cachedSurface, apiMesh.outputSurface )
		om.MPxNode.attributeAffects( apiMesh.cachedSurface, apiMesh.worldSurface )

		om.MPxNode.attributeAffects( om.MPxSurfaceShape.mControlPoints, apiMesh.outputSurface )
		om.MPxNode.attributeAffects( om.MPxSurfaceShape.mControlValueX, apiMesh.outputSurface )
		om.MPxNode.attributeAffects( om.MPxSurfaceShape.mControlValueY, apiMesh.outputSurface )
		om.MPxNode.attributeAffects( om.MPxSurfaceShape.mControlValueZ, apiMesh.outputSurface )
		om.MPxNode.attributeAffects( om.MPxSurfaceShape.mControlPoints, apiMesh.cachedSurface )
		om.MPxNode.attributeAffects( om.MPxSurfaceShape.mControlValueX, apiMesh.cachedSurface )
		om.MPxNode.attributeAffects( om.MPxSurfaceShape.mControlValueY, apiMesh.cachedSurface )
		om.MPxNode.attributeAffects( om.MPxSurfaceShape.mControlValueZ, apiMesh.cachedSurface )
		om.MPxNode.attributeAffects( om.MPxSurfaceShape.mControlPoints, apiMesh.worldSurface )
		om.MPxNode.attributeAffects( om.MPxSurfaceShape.mControlValueX, apiMesh.worldSurface )
		om.MPxNode.attributeAffects( om.MPxSurfaceShape.mControlValueY, apiMesh.worldSurface )
		om.MPxNode.attributeAffects( om.MPxSurfaceShape.mControlValueZ, apiMesh.worldSurface )

	def __init__(self):
		om.MPxSurfaceShape.__init__(self)

	##########################################################
	##
	## Overrides
	##
	##########################################################

	## From MPxNode
	##
	def postConstructor(self):
		##
		## Description
		##
		##    When instances of this node are created internally, the MObject associated
		##    with the instance is not created until after the constructor of this class
		##    is called. This means that no member functions of MPxSurfaceShape can
		##    be called in the constructor.
		##    The postConstructor solves this problem. Maya will call this function
		##    after the internal object has been created.
		##    As a general rule do all of your initialization in the postConstructor.
		##

		## This call allows the shape to have shading groups assigned
		##
		self.isRenderable = True

		## Is there input history to this node
		##
		self.fHasHistoryOnCreate = False

		## Is the shape dirty? Used by VP2.0 sub-scene evaluator
		##
		self.fShapeDirty = True
		self.fMaterialDirty = True

	def compute(self, plug, datablock):
		##
		## Description
		##
		##    When input attributes are dirty this method will be called to
		##    recompute the output attributes.
		##
		## Arguments
		##
		##    plug      - the attribute that triggered the compute
		##    datablock - the nodes data
		##

		if plug == apiMesh.outputSurface:
			return self.computeOutputSurface( plug, datablock )

		elif plug == apiMesh.cachedSurface:
			return self.computeOutputSurface( plug, datablock )

		elif plug == apiMesh.worldSurface:
			return self.computeWorldSurface( plug, datablock )

		else:
			return None

	def setDependentsDirty(self, plug, plugArray):
		##
		## Description
		##
		##	Horribly abuse the purpose of this method to notify the Viewport 2.0
		##  renderer that something about this shape has changed and that it should
		##  be retranslated.
		##

		## if the dirty attribute is the output mesh then we need to signal the
		## the renderer that it needs to update the object

		if plug == apiMesh.inputSurface or plug == om.MPxSurfaceShape.mControlPoints or plug == om.MPxSurfaceShape.mControlValueX or plug == om.MPxSurfaceShape.mControlValueY or plug == om.MPxSurfaceShape.mControlValueZ:
			self.signalDirtyToViewport()

	def getInternalValueInContext(self, plug, handle, ctx):
		##
		## Description
		##
		##    Handle internal attributes.
		##
		##    Attributes that require special storage, bounds checking,
		##    or other non-standard behavior can be marked as "Internal" by
		##    using the "MFnAttribute.setInternal" method.
		##
		##    The get/setInternalValueInContext methods will get called for internal
		##    attributes whenever the attribute values are stored or retrieved
		##    using getAttr/setAttr or MPlug getValue/setValue.
		##
		##    The inherited attribute mControlPoints is internal and we want
		##    its values to get stored only if there is input history. Otherwise
		##    any changes to the vertices are stored in the cachedMesh and outputMesh
		##    directly.
		##
		##    If values are retrieved then we want the controlPoints value
		##    returned if there is history, this will be the offset or tweak.
		##    In the case of no history, the vertex position of the cached mesh
		##    is returned.
		##
		isOk = True

		if plug == om.MPxSurfaceShape.mControlPoints or plug == om.MPxSurfaceShape.mControlValueX or plug == om.MPxSurfaceShape.mControlValueY or plug == om.MPxSurfaceShape.mControlValueZ:
			## If there is input history then the control point value is
			## directly returned. This is the tweak or offset that
			## was applied to the vertex.
			##
			## If there is no input history then return the actual vertex
			## position and ignore the controlPoints attribute.
			##
			if self.hasHistory():
				return om.MPxNode.getInternalValueInContext(self, plug, handle, ctx)

			else:
				if plug == om.MPxSurfaceShape.mControlPoints and not plug.isArray():
					index = plug.logicalIndex()
					pnt = self.getPointValue(index)
					handle.set3Double( pnt[0], pnt[1], pnt[2] )

				elif plug == om.MPxSurfaceShape.mControlValueX:
					parentPlug = plug.parent()
					index = parentPlug.logicalIndex()
					val = self.getChannelValue( index, 0 )
					handle.setDouble( val )

				elif plug == om.MPxSurfaceShape.mControlValueY:
					parentPlug = plug.parent()
					index = parentPlug.logicalIndex()
					val = self.getChannelValue( index, 1 )
					handle.setDouble( val )

				elif plug == om.MPxSurfaceShape.mControlValueZ:
					parentPlug = plug.parent()
					index = parentPlug.logicalIndex()
					val = self.getChannelValue( index, 2 )
					handle.setDouble( val )

		## This inherited attribute is used to specify whether or
		## not this shape has history. During a file read, the shape
		## is created before any input history can get connected.
		## This attribute, also called "tweaks", provides a way to
		## for the shape to determine if there is input history
		## during file reads.
		##
		elif plug == om.MPxSurfaceShape.mHasHistoryOnCreate:
			handle.setBool( self.fHasHistoryOnCreate )

		else:
			isOk = om.MPxSurfaceShape.getInternalValueInContext(self, plug, handle, ctx)

		return isOk

	def setInternalValueInContext(self, plug, handle, ctx):
		##
		## Description
		##
		##    Handle internal attributes.
		##
		##    Attributes that require special storage, bounds checking,
		##    or other non-standard behavior can be marked as "Internal" by
		##    using the "MFnAttribute.setInternal" method.
		##
		##    The get/setInternalValueInContext methods will get called for internal
		##    attributes whenever the attribute values are stored or retrieved
		##    using getAttr/setAttr or MPlug getValue/setValue.
		##
		##    The inherited attribute mControlPoints is internal and we want
		##    its values to get stored only if there is input history. Otherwise
		##    any changes to the vertices are stored in the cachedMesh and outputMesh
		##    directly.
		##
		##    If values are retrieved then we want the controlPoints value
		##    returned if there is history, this will be the offset or tweak.
		##    In the case of no history, the vertex position of the cached mesh
		##    is returned.
		##
		isOk = True

		if plug == om.MPxSurfaceShape.mControlPoints or plug == om.MPxSurfaceShape.mControlValueX or plug == om.MPxSurfaceShape.mControlValueY or plug == om.MPxSurfaceShape.mControlValueZ:
			## If there is input history then set the control points value
			## using the normal mechanism. In this case we are setting
			## the tweak or offset that will get applied to the input
			## history.
			##
			## If there is no input history then ignore the controlPoints
			## attribute and set the vertex position directly in the
			## cachedMesh.
			##
			if self.hasHistory():
				self.verticesUpdated()
				return om.MPxNode.setInternalValueInContext(self, plug, handle, ctx)

			else:
				if plug == om.MPxSurfaceShape.mControlPoints and not plug.isArray():
					index = plug.logicalIndex()
					self.setPointValue( index, handle.asDouble3() )

				elif plug == om.MPxSurfaceShape.mControlValueX:
					parentPlug = plug.parent()
					index = parentPlug.logicalIndex()
					self.setChannelValue( index, 0, handle.asDouble() )

				elif plug == om.MPxSurfaceShape.mControlValueY:
					parentPlug = plug.parent()
					index = parentPlug.logicalIndex()
					self.setChannelValue( index, 1, handle.asDouble() )

				elif plug == om.MPxSurfaceShape.mControlValueZ:
					parentPlug = plug.parent()
					index = parentPlug.logicalIndex()
					self.setChannelValue( index, 2, handle.asDouble() )

		## This inherited attribute is used to specify whether or
		## not this shape has history. During a file read, the shape
		## is created before any input history can get connected.
		## This attribute, also called "tweaks", provides a way to
		## for the shape to determine if there is input history
		## during file reads.
		##
		elif plug == om.MPxSurfaceShape.mHasHistoryOnCreate:
			self.fHasHistoryOnCreate = handle.asBool()

		else:
			isOk = om.MPxSurfaceShape.setInternalValueInContext(self, plug, handle, ctx)

		return isOk

	def connectionMade(self, plug, otherPlug, asSrc):
		##
		## Description
		##
		##    Whenever a connection is made to this node, this method
		##    will get called.
		##

		if plug == apiMesh.inputSurface:
			thisObj = self.thisMObject()
			historyPlug = om.MPlug( thisObj, om.MPxSurfaceShape.mHasHistoryOnCreate )
			historyPlug.setBool( True )
		else:
			thisObj = self.thisMObject()
			dgNode = om.MFnDependencyNode( thisObj )
			instObjGroups = dgNode.findPlug("instObjGroups", True)
			if plug == instObjGroups:
				self.setMaterialDirty(True)

		return om.MPxNode.connectionMade(self, plug, otherPlug, asSrc )

	def connectionBroken(self, plug, otherPlug, asSrc):
		##
		## Description
		##
		##    Whenever a connection to this node is broken, this method
		##    will get called.
		##

		if plug == apiMesh.inputSurface:
			thisObj = self.thisMObject()
			historyPlug = om.MPlug( thisObj, om.MPxSurfaceShape.mHasHistoryOnCreate )
			historyPlug.setBool( False )
		else:
			thisObj = self.thisMObject()
			dgNode = om.MFnDependencyNode( thisObj )
			instObjGroups = dgNode.findPlug("instObjGroups", True)
			if plug == instObjGroups:
				self.setMaterialDirty(True)

		return om.MPxNode.connectionBroken(self, plug, otherPlug, asSrc )

	def shouldSave(self, plug):
		##
		## Description
		##
		##    During file save this method is called to determine which
		##    attributes of this node should get written. The default behavior
		##    is to only save attributes whose values differ from the default.
		##

		result = True

		if plug == om.MPxSurfaceShape.mControlPoints or plug == om.MPxSurfaceShape.mControlValueX or plug == om.MPxSurfaceShape.mControlValueY or plug == om.MPxSurfaceShape.mControlValueZ:
			if self.hasHistory():
				## Calling this will only write tweaks if they are
				## different than the default value.
				##
				result = om.MPxNode.shouldSave(self, plug)

			else:
				result = False

		elif plug == apiMesh.cachedSurface:
			if self.hasHistory():
				result = False

			else:
				data = plug.asMObject()
				result = not data.isNull()

		else:
			result = om.MPxNode.shouldSave(self, plug)

		return result

	## Attribute to component (components)
	##
	def componentToPlugs(self, component, list):
		##
		## Description
		##
		##    Converts the given component values into a selection list of plugs.
		##    This method is used to map components to attributes.
		##
		## Arguments
		##
		##    component - the component to be translated to a plug/attribute
		##    list      - a list of plugs representing the passed in component
		##

		if component.hasFn(om.MFn.kSingleIndexedComponent):
			fnVtxComp = om.MFnSingleIndexedComponent( self.convertToVertexComponent(component) )
			thisNode = self.thisMObject()
			plug = om.MPlug( thisNode, om.MPxSurfaceShape.mControlPoints )
			## If this node is connected to a tweak node, reset the
			## plug to point at the tweak node.
			##
			self.convertToTweakNodePlug(plug)

			for i in xrange(fnVtxComp.elementCount):
				plug.selectAncestorLogicalIndex(fnVtxComp.element(i), plug.attribute())
				list.add(plug)

	def matchComponent(self, item, spec, list):
		##
		## Description:
		##
		##    Component/attribute matching method.
		##    This method validates component names and indices which are
		##    specified as a string and adds the corresponding component
		##    to the passed in selection list.
		##
		##    For instance, select commands such as "select shape1.vtx[0:7]"
		##    are validated with this method and the corresponding component
		##    is added to the selection list.
		##
		## Arguments
		##
		##    item - DAG selection item for the object being matched
		##    spec - attribute specification object
		##    list - list to add components to
		##
		## Returns
		##
		##    the result of the match
		##

		result = om.MPxSurfaceShape.kMatchOk
		attrSpec = spec[0]
		dim = attrSpec.dimensions

		## Look for attributes specifications of the form :
		##     vtx[ index ]
		##     vtx[ lower:upper ]
		##
		if len(spec) == 1 and dim > 0 and attrSpec.name == "vtx":
			numVertices = len(self.meshGeom().vertices)
			attrIndex = attrSpec[0]

			upper = 0
			lower = 0
			if attrIndex.hasLowerBound():
				lower = attrIndex.getLower()
			if attrIndex.hasUpperBound():
				upper = attrIndex.getUpper()

			## Check the attribute index xrange is valid
			##
			if lower > upper or upper >= numVertices:
				result = om.MPxSurfaceShape.kMatchInvalidAttributeRange

			else:
				path = item.getDagPath(0)
				fnVtxComp = om.MFnSingleIndexedComponent()
				vtxComp = fnVtxComp.create( om.MFn.kMeshVertComponent )

				for i in xrange(lower, upper+1):
					fnVtxComp.addElement( i )

				list.add( path, vtxComp )

		else:
			## Pass this to the parent class
			result = om.MPxSurfaceShape.matchComponent(self, item, spec, list )

		return result

	def match(self, mask, componentList):
		##
		## Description:
		##
		##		Check for matches between selection type / component list, and
		##		the type of this shape / or it's components
		##
		##      This is used by sets and deformers to make sure that the selected
		##      components fall into the "vertex only" category.
		##
		## Arguments
		##
		##		mask          - selection type mask
		##		componentList - possible component list
		##
		## Returns
		##		True if matched any
		##

		result = False

		if len(componentList) == 0:
			result = mask.intersects( om.MSelectionMask.kSelectMeshes )

		else:
			for comp in componentList:
				if comp.apiType() == om.MFn.kMeshVertComponent and mask.intersects(om.MSelectionMask.kSelectMeshVerts):
					result = True
					break

		return result

	## Support deformers (components)
	##
	def createFullVertexGroup(self):
		##
		## Description
		##     This method is used by maya when it needs to create a component
		##     containing every vertex (or control point) in the shape.
		##     This will get called if you apply some deformer to the whole
		##     shape, i.e. select the shape in object mode and add a deformer to it.
		##
		## Returns
		##
		##    A "complete" component representing all vertices in the shape.
		##

		## Create a vertex component
		##
		fnComponent = om.MFnSingleIndexedComponent()
		fullComponent = fnComponent.create( om.MFn.kMeshVertComponent )

		## Set the component to be complete, i.e. the elements in
		## the component will be [0:numVertices-1]
		##
		numVertices = len(self.meshGeom().vertices)
		fnComponent.setCompleteData( numVertices )

		return fullComponent

	def getShapeSelectionMask(self):
		##
		## Description
		##     This method is overriden to support interactive object selection in Viewport 2.0
		##
		## Returns
		##
		##    The selection mask of the shape
		##

		selType = om.MSelectionMask.kSelectMeshes
		return om.MSelectionMask( selType )

	def getComponentSelectionMask(self):
		##
		## Description
		##     This method is overriden to support interactive component selection in Viewport 2.0
		##
		## Returns
		##
		##    The mask of the selectable components of the shape
		##

		selMask = om.MSelectionMask(om.MSelectionMask.kSelectMeshVerts)
		selMask.addMask(om.MSelectionMask.kSelectMeshEdges)
		selMask.addMask(om.MSelectionMask.kSelectMeshFaces)
		return selMask

	def localShapeInAttr(self):
		##
		## Description
		##
		##    Returns the input attribute of the shape. This is used by
		##    maya to establish input connections for deformers etc.
		##    This attribute must be data of type kGeometryData.
		##
		## Returns
		##
		##    input attribute for the shape
		##

		return apiMesh.inputSurface

 	def localShapeOutAttr(self):
		##
		## Description
		##
		##    Returns the output attribute of the shape. This is used by
		##    maya to establish out connections for deformers etc.
		##    This attribute must be data of type kGeometryData.
		##
		## Returns
		##
		##    output attribute for the shape
		##
		##

		return apiMesh.outputSurface

 	def worldShapeOutAttr(self):
		##
		## Description
		##
		##    Returns the output attribute of the shape. This is used by
		##    maya to establish out connections for deformers etc.
		##    This attribute must be data of type kGeometryData.
		##
		## Returns
		##
		##    output attribute for the shape
		##
		##

		return apiMesh.outputSurface

 	def cachedShapeAttr(self):
		##
		## Description
		##
		##    Returns the cached shape attribute of the shape.
		##    This attribute must be data of type kGeometryData.
		##
		## Returns
		##
		##    cached shape attribute
		##

		return apiMesh.cachedSurface


	def geometryData(self):
		##
		## Description
		##
		##    Returns the data object for the surface. This gets
		##    called internally for grouping (set) information.
		##

		datablock = self.forceCache()
		handle = datablock.inputValue( apiMesh.inputSurface )
		return handle.data()

	def closestPoint(self, toThisPoint, theClosestPoint, tolerance):
		##
		## Description
		##
		##		Returns the closest point to the given point in space.
		##		Used for rigid bind of skin.  Currently returns wrong results;
		##		override it by implementing a closest point calculation.

		## Iterate through the geometry to find the closest point within
		## the given tolerance.
		##
		geometry = self.meshGeom()
		numVertices = len(geometry.vertices)
		for i in xrange(numVertices):
			tryThisOne = geometry.vertices[i]

		## Set the output point to the result (hardcode for debug just now)
		##
		theClosestPoint = geometry.vertices[0]

	## Support the translate/rotate/scale tool (components)
	##
	def transformUsing(self, mat, componentList, cachingMode=om.MPxSurfaceShape.kNoPointCaching, pointCache=None):
		##
		## Description
		##
		##    Transforms the given components. This method is used by
		##    the move, rotate, and scale tools in component mode.
		##    The bounding box has to be updated here, so do the normals and
		##    any other attributes that depend on vertex positions.
		##
		## Arguments
		##    mat           - matrix to tranform the components by
		##    componentList - list of components to be transformed,
		##                    or an empty list to indicate the whole surface
		##    cachingMode   - how to use the supplied pointCache (kSavePoints, kRestorePoints)
		##    pointCache    - if non-None, save or restore points from this list base
		##					  on the cachingMode
		##

		geometry = self.meshGeom()

		## Create cachingMode boolean values for clearer reading of conditional code below
		##
		savePoints    = (cachingMode == om.MPxSurfaceShape.kSavePoints and pointCache is not None)
		restorePoints = (cachingMode == om.MPxSurfaceShape.kRestorePoints and pointCache is not None)

		cacheIndex = 0
		cacheLen = 0
		if pointCache:
			cacheLen = len(pointCache)

		if restorePoints:
			## restore the points based on the data provided in the pointCache attribute
			##
			if len(componentList) > 0:
				## traverse the component list
				##
				for comp in componentList:
					fnComp = om.MFnSingleIndexedComponent( self.convertToVertexComponent(comp) )
					elemCount = fnComp.elementCount

					for idx in xrange(elementCount):
						elemIndex = fnComp.element(idx)
						geometry.vertices[elemIndex] = pointCache[cacheIndex]
						cacheIndex += 1
						if cacheIndex >= cacheLen:
							break

					if cacheIndex >= cacheLen:
						break

			else:
				## if the component list is of zero-length, it indicates that we
				## should transform the entire surface
				##
				vertLen = len(geometry.vertices)
				for idx in xrange(vertLen):
					geometry.vertices[idx] = pointCache[cacheIndex]
					cacheIndex += 1
					if cacheIndex >= cacheLen:
						break

		else:
			## Transform the surface vertices with the matrix.
			## If savePoints is True, save the points to the pointCache.
			##
			if len(componentList) > 0:
				## Traverse the componentList
				##
				setSizeIncrement = True
				for comp in componentList:
					fnComp = om.MFnSingleIndexedComponent( self.convertToVertexComponent(comp) )
					elemCount = fnComp.elementCount

					if savePoints and setSizeIncrement:
						pointCache.sizeIncrement = elemCount
						setSizeIncrement = False

					for idx in xrange(elemCount):
						elemIndex = fnComp.element(idx)
						if savePoints:
							pointCache.append(geometry.vertices[elemIndex])

						geometry.vertices[elemIndex] *= mat
						geometry.normals[idx] = geometry.normals[idx].transformAsNormal( mat )

			else:
				## If the component list is of zero-length, it indicates that we
				## should transform the entire surface
				##
				vertLen = len(geometry.vertices)
				if savePoints:
					pointCache.sizeIncrement = vertLen

				for idx in xrange(vertLen):
					if savePoints:
						pointCache.append(geometry.vertices[idx])

					geometry.vertices[idx] *= mat
					geometry.normals[idx] = geometry.normals[idx].transformAsNormal( mat )

		## Update the surface
		self.updateCachedSurface( geometry, componentList )

	def tweakUsing(self, mat, componentList, cachingMode, pointCache, handle):
		##
		## Description
		##
		##    Transforms the given components. This method is used by
		##    the move, rotate, and scale tools in component mode when the
		##    tweaks for the shape are stored on a separate tweak node.
		##    The bounding box has to be updated here, so do the normals and
		##    any other attributes that depend on vertex positions.
		##
		## Arguments
		##    mat           - matrix to tranform the components by
		##    componentList - list of components to be transformed,
		##                    or an empty list to indicate the whole surface
		##    cachingMode   - how to use the supplied pointCache (kSavePoints, kRestorePoints, kUpdatePoints)
		##    pointCache    - if non-null, save or restore points from this list base
		##					  on the cachingMode
		##    handle	    - handle to the attribute on the tweak node where the
		##					  tweaks should be stored
		##

		geometry = self.meshGeom()

		## Create cachingMode boolean values for clearer reading of conditional code below
		##
		savePoints    = (cachingMode == om.MPxSurfaceShape.kSavePoints and pointCache is not None)
		updatePoints  = (cachingMode == om.MPxSurfaceShape.kUpdatePoints and pointCache is not None)
		restorePoints = (cachingMode == om.MPxSurfaceShape.kRestorePoints and pointCache is not None)

		builder = handle.builder()

		cacheIndex = 0
		cacheLen = 0
		if pointCache:
			cacheLen = len(pointCache)

		if restorePoints:
			## restore points from the pointCache
			##
			if len(componentList) > 0:
				## traverse the component list
				##
				for comp in componentList:
					fnComp = om.MFnSingleIndexedComponent( self.convertToVertexComponent(comp) )
					elemCount = fnComp.elementCount

					for idx in xrange(elementCount):
						elemIndex = fnComp.element(idx)
						cachePt = pointCache[cacheIndex]
						elem = builder.addElement( elemIndex )
						elem.set3Double(cachePt.x, cachePt.y, cachePt.z)
						cacheIndex += 1
						if cacheIndex >= cacheLen:
							break

					if cacheIndex >= cacheLen:
						break

			else:
				## if the component list is of zero-length, it indicates that we
				## should transform the entire surface
				##
				vertLen = len(geometry.vertices)
				for idx in xrange(vertLen):
					cachePt = pointCache[cacheIndex]
					elem = builder.addElement( idx )
					elem.set3Double(cachePt.x, cachePt.y, cachePt.z)
					cacheIndex += 1
					if cacheIndex >= cacheLen:
						break

		else:
			## Tweak the points. If savePoints is True, also save the tweaks in the
			## pointCache. If updatePoints is True, add the new tweaks to the existing
			## data in the pointCache.
			##
			if len(componentList) > 0:
				setSizeIncrement = True
				for comp in componentList:
					fnComp = om.MFnSingleIndexedComponent( self.convertToVertexComponent(comp) )
					elemCount = fnComp.elementCount

					if savePoints and setSizeIncrement:
						pointCache.sizeIncrement = elemCount
						setSizeIncrement = False

					for idx in xrange(elementCount):
						elemIndex = fnComp.element(idx)
						currPt = newPt = geometry.vertices[elemIndex]
						newPt *= mat
						delta = newPt - currPt
						elem = builder.addElement( elemIndex )
						elem.set3Double(delta.x, delta.y, delta.z)

						if savePoints:
							## store the points in the pointCache for undo
							##
							pointCache.append(delta*(-1.0))

						elif updatePoints and cacheIndex < cacheLen:
							pointCache[cacheIndex] = pointCache[cacheIndex] - delta
							cacheIndex += 1

			else:
				## if the component list is of zero-length, it indicates that we
				## should transform the entire surface
				##
				vertLen = len(geometry.vertices)
				if savePoints:
					pointCache.sizeIncrement = vertLen

				for idx in xrange(vertLen):
					currPt = newPt = geometry.vertices[idx]
					newPt *= mat
					delta = newPt - currPt
					elem = builder.addElement( idx )
					elem.set3Double(delta.x, delta.y, delta.z)

					if savePoints:
						## store the points in the pointCache for undo
						##
						pointCache.append(delta*(-1.0))

					elif updatePoints and idx < cacheLen:
						pointCache[cacheIndex] = pointCache[cacheIndex] - delta
						cacheIndex += 1

		## Set the builder into the handle.
		##
		handle.set(builder)

		## Tell maya the bounding box for this object has changed
		## and thus "boundingBox()" needs to be called.
		##
		self.childChanged( om.MPxSurfaceShape.kBoundingBoxChanged )

		## Signal to the viewport that it needs to update the object
		self.signalDirtyToViewport()

	## Support the soft-select translate/rotate/scale tool (components)
	##
	def weightedTransformUsing(self, xform, space, componentList, cachingMode, pointCache, freezePlane):
		##
		## Description
		##
		##    Transforms the given soft-selected components interpolated using the specified weights.
		##    This method is used by the move, rotate, and scale tools in component mode.
		##    The bounding box has to be updated here, so do the normals and
		##    any other attributes that depend on vertex positions.
		##    It is similar to the transformUsing() virtual function.
		##
		## Arguments
		##
		##    xform           the matrix representing the transformation that is to be applied to the components
		##    space           the matrix representing the transformation space to perform the interpolated transformation.
		##                    A value of None indicates it should be ignored.
		##    componentList   a list of components to be transformed and their weights.  This list will not be empty.
		##    cachingMode     whether the points should be added/updated in the pointCache, or restored from
		##                    the pointCache, or transform using the original values in the pointCache.
		##    pointCache      used to store for undo and restore points during undo
		##    freezePlane     used for symmetric transformation of components.  A value of None indicates
		##                    it is not used and there is no symmetric transformation.
		##

		## For example purposes only, use the default MPxSurfaceShape.weightedTransformUsing() if the
		## useWeightedTransformUsingFunction is False
		##
		plg_useWeightedTransformUsingFunction = om.MPlug( self.thisMObject(), apiMesh.useWeightedTransformUsingFunction )
		val_useWeightedTransformUsingFunction = plg_useWeightedTransformUsingFunction.asBool()
		if not val_useWeightedTransformUsingFunction:
			om.MPxSurfaceShape.weightedTransformUsing(self, xform, space, componentList, cachingMode, pointCache, freezePlane)
			self.signalDirtyToViewport()
			return

		## Create cachingMode boolean values for clearer reading of conditional code below
		##
		savePoints          = (cachingMode == om.MPxSurfaceShape.kSavePoints and pointCache is not None)
		updatePoints        = (cachingMode == om.MPxSurfaceShape.kUpdatePoints and pointCache is not None)
		restorePoints       = (cachingMode == om.MPxSurfaceShape.kRestorePoints and pointCache is not None)
		transformOrigPoints = (cachingMode == om.MPxSurfaceShape.kTransformOriginalPoints and pointCache is not None)

		## Pre-calculate parameters
		spaceInv = om.MMatrix()
		if space:
			spaceInv = space.inverse()

		## Traverse the componentList and modify the control points
		##
		geometry = self.meshGeom()
		almostZero = 1.0e-5 ## Hardcoded tolerance
		pointCacheIndex = 0
		setSizeIncrement = True

		for comp in componentList:
			fnComp = om.MFnSingleIndexedComponent( self.convertToVertexComponent(comp) )
			elemCount = fnComp.elementCount
			hasWeights = fnComp.hasWeights
			hasSeam = (freezePlane is not None)

			if savePoints and setSizeIncrement:
				pointCache.sizeIncrement = elemCount
				setSizeIncrement = False

			for idx in xrange(elementCount):
				elemIndex = fnComp.element( idx )
				perc = 1.0
				if hasWeights:
			   		perc = fnComp.weight(idx).influence()	## get the weight for the component

				## Only act upon points (store in pointCache, transform, etc) that have a non-zero weight
				if perc > almostZero:	## if the point has enough weight to be transformed
					if restorePoints:
						## restore the original point from the point cache
						geometry.vertices[elemIndex] = om.MVector( pointCache[pointCacheIndex] )
						pointCacheIndex += 1

					else:	## perform point transformation
						## Update the pointCache with the original value
						if savePoints:
							pointCache.append( geometry.vertices[elemIndex] )

						elif transformOrigPoints:	## start by reverting points back to their original values stored in the pointCache for the transformation
							geometry.vertices[elemIndex] = om.MVector( pointCache[pointCacheIndex] )

						elif updatePoints:	## update the pointCache with the current values
							pointCache[pointCacheIndex] = geometry.vertices[elemIndex]

						## Compute interpolated transformation matrix
						mat = om.MMatrix()
						if perc == 1.0:
							mat = xform.asMatrix()
						elif space:
							mat = space * xform.asMatrix(perc) * spaceInv
						else:
							mat = xform.asMatrix(perc)

						## transform to new position
						currPt = newPt = geometry.vertices[elemIndex]
						newPt *= mat

						## handle symmetry and reflection
						if hasSeam and fnComp.weight(idx).seam() > 0.0:
							newPt += freezePlane.normal() * (fnComp.weight(idx).seam() * (freezePlane.directedDistance(currPt) - freezePlane.directedDistance(newPt)))

						## Update the geometry with the new point
						geometry.vertices[elemIndex] = newPt
						pointCacheIndex += 1

		## Update the surface
		self.updateCachedSurface( geometry, componentList )

	def weightedTweakUsing(self, xform, space, componentList, cachingMode, pointCache, freezePlane, handle):
		##
		## Description
		##
		##    Transforms the given soft-selected components interpolated using the specified weights.
		##    This method is used by the move, rotate, and scale tools in component mode when the
		##    tweaks for the shape are stored on a separate tweak node.
		##    The bounding box has to be updated here, so do the normals and
		##    any other attributes that depend on vertex positions.
		##
		##    It is similar to the tweakUsing() virtual function and is based on apiMesh.tweakUsing().
		##
		##
		## Arguments
		##
		##    xform           the matrix representing the transformation that is to be applied to the components
		##    space           the matrix representing the transformation space to perform the interpolated transformation.
		##                    A value of None indicates it should be ignored.
		##    componentList   a list of components to be transformed and their weights.  This list will not be empty.
		##    cachingMode     whether the points should be added/updated in the pointCache, or restored from
		##                    the pointCache, or transform using use the original values in the pointCache.
		##    pointCache      used to store for undo and restore points during undo
		##    freezePlane     used for symmetric transformation of components.  A value of None indicates
		##                    it is not used and there is no symmetric transformation.
		##    handle	    - handle to the attribute on the tweak node where the
		##					  tweaks should be stored
		##
	
		## For example purposes only, use the default MPxSurfaceShape.weightedTweakUsing() if the
		## useWeightedTweakUsingFunction is False
		##
		plg_useWeightedTweakUsingFunction = om.MPlug( self.thisMObject(), apiMesh.useWeightedTweakUsingFunction )
		val_useWeightedTweakUsingFunction = plg_useWeightedTweakUsingFunction.asBool()
		if not val_useWeightedTweakUsingFunction:
			om.MPxSurfaceShape.weightedTweakUsing(self, xform, space, componentList, cachingMode, pointCache, freezePlane, handle)
			return

		geometry = self.meshGeom()

		## Create cachingMode boolean values for clearer reading of conditional code below
		##
		savePoints          = (cachingMode == om.MPxSurfaceShape.kSavePoints and pointCache is not None)
		updatePoints        = (cachingMode == om.MPxSurfaceShape.kUpdatePoints and pointCache is not None)
		restorePoints       = (cachingMode == om.MPxSurfaceShape.kRestorePoints and pointCache is not None)
		transformOrigPoints = (cachingMode == om.MPxSurfaceShape.kTransformOriginalPoints and pointCache is not None)

		builder = handle.builder()

		cacheIndex = 0
		cacheLen = 0
		if pointCache:
			cacheLen = len(pointCache)

		if restorePoints:
			## restore points from the pointCache
			##
			## traverse the component list
			##
			for comp in componentList:
				fnComp = om.MFnSingleIndexedComponent( self.convertToVertexComponent(comp) )
				elemCount = fnComp.elementCount

				for idx in xrange(elementCount):
					elemIndex = fnComp.element( idx )
					cachePt = pointCache[cacheIndex]
					elem = builder.addElement( elemIndex )
					elem.set3Double(cachePt.x, cachePt.y, cachePt.z)
					cacheIndex += 1
					if cacheIndex >= cacheLen:
						break

		else:
			## Tweak the points. If savePoints is True, also save the tweaks in the
			## pointCache. If updatePoints is True, add the new tweaks to the existing
			## data in the pointCache.
			##

			## Specify a few parameters (for weighted transformation)
			almostZero = 1.0e-5 ## Hardcoded tolerance
			setSizeIncrement = True
			spaceInv = om.MMatrix()
			if space:
				spaceInv = space.inverse()

			for comp in componentList:
				fnComp = om.MFnSingleIndexedComponent( self.convertToVertexComponent(comp) )
				elemCount = fnComp.elementCount
				hasWeights = fnComp.hasWeights ## (for weighted transformation)
				hasSeam = (freezePlane is not None)  ## (for weighted transformation)

				if savePoints and setSizeIncrement:
					pointCache.sizeIncrement = elemCount
					setSizeIncrement = False

				for idx in xrange(elementCount):
					elemIndex = fnComp.element( idx )
					perc = 1.0
					if hasWeights:
						perc = fnComp.weight(idx).influence()	## get the weight for the component

					## Only act upon points (store in pointCache, transform, etc) that have a non-zero weight
					if perc > almostZero:	## if the point has enough weight to be transformed (for weighted transformation)

						## Compute interpolated transformation matrix (for weighted transformation)
						##
						mat = om.MMatrix()
						if perc == 1.0:
							mat = xform.asMatrix()
						elif space:
							mat = space * xform.asMatrix(perc) * spaceInv
						else:
							mat = xform.asMatrix(perc)

						## Start by reverting points back to their original values stored in
						## the pointCache for the transformation
						##
						if transformOrigPoints:
							geometry.vertices[elemIndex] = om.MVector( pointCache[cacheIndex] )

						## Perform transformation of the point
						##
						currPt = newPt = geometry.vertices[elemIndex]
						newPt *= mat

						## Handle symmetry and reflection (for weighted transformation)
						##
						if hasSeam and fnComp.weight(idx).seam() > 0.0:
							newPt += freezePlane.normal() * (fnComp.weight(idx).seam() * (freezePlane.directedDistance(currPt) - freezePlane.directedDistance(newPt)))

						## Calculate deltas and final positions
						delta = newPt - currPt

						elem = builder.addElement( elemIndex )
						elem.set3Double(delta.x, delta.y, delta.z)

						if savePoints:
							## store the points in the pointCache for undo
							##
							pointCache.append(delta*(-1.0))
						elif updatePoints and cacheIndex < cacheLen:
							pointCache[cacheIndex] = pointCache[cacheIndex] - delta
							cacheIndex += 1

		## Set the builder into the handle.
		##
		handle.set(builder)

		## Tell maya the bounding box for this object has changed
		## and thus "boundingBox()" needs to be called.
		##
		self.childChanged( om.MPxSurfaceShape.kBoundingBoxChanged )

	## Support the move tools normal/u/v mode (components)
	##
	def vertexOffsetDirection(self, component, direction, mode, normalize):
		##
		## Description
		##
		##    Returns offsets for the given components to be used my the
		##    move tool in normal/u/v mode.
		##
		## Arguments
		##
		##    component - components to calculate offsets for
		##    direction - array of offsets to be filled
		##    mode      - the type of offset to be calculated
		##    normalize - specifies whether the offsets should be normalized
		##
		## Returns
		##
		##    True if the offsets could be calculated, False otherwise
		##

		fnComp = om.MFnSingleIndexedComponent( self.convertToVertexComponent(component) )
		if component.apiType() != om.MFn.kMeshVertComponent:
			return False

		geometry = self.meshGeom()
		if not geometry:
			return False

		## For each vertex add the appropriate offset
		##
		for idx in fnComp:
			normal = geometry.normals[ idx ]

			if mode == om.MPxSurfaceShape.kNormal:
				if normalize:
					normal.normalize()
				direction.append( normal )

			else:
				## Construct an orthonormal basis from the normal
				## uAxis, and vAxis are the new vectors.
				##
				normal.normalize()

				i = 0
				a = math.abs(normal[0])

				if a < math.abs(normal[1]):
				   i = 1
				   a = fabs(normal[1])

				if a < math.abs(normal[2]):
				   i = 2

				j = (i+1)%3
				k = (j+1)%3

				a = math.sqrt(normal[i]*normal[i] + normal[j]*normal[j])
				uAxis[i] = -normal[j]/a
				uAxis[j] =  normal[i]/a
			   	uAxis[k] = 0.0
				vAxis = normal^uAxis

				if mode == om.MPxSurfaceShape.kUTangent or mode == om.MPxSurfaceShape.kUVNTriad:
					if normalize:
						uAxis.normalize()
					direction.append( uAxis )

				if mode == om.MPxSurfaceShape.kVTangent or mode == om.MPxSurfaceShape.kUVNTriad:
					if normalize:
						vAxis.normalize()
					direction.append( vAxis )

				if mode == om.MPxSurfaceShape.kUVNTriad:
					if normalize:
						normal.normalize()
					direction.append( normal )

		return True

	## Bounding box methods
	##
	def isBounded(self):
		##
		## Description
		##
		##    Specifies that this object has a boundingBox.
		##

		return True

	def boundingBox(self):
		##
		## Description
		##
		##    Returns the bounding box for this object.
		##    It is a good idea not to recompute here as this funcion is called often.
		##
		if self.fShapeDirty:
			# Update:
			self.meshObject()

		thisNode = self.thisMObject()
		c1Plug = om.MPlug( thisNode, apiMesh.bboxCorner1 )
		c2Plug = om.MPlug( thisNode, apiMesh.bboxCorner2 )
		corner1Object = c1Plug.asMObject()
		corner2Object = c2Plug.asMObject()

		fnData = om.MFnNumericData()
		fnData.setObject( corner1Object )
		corner1 = fnData.getData()

		fnData.setObject( corner2Object )
		corner2 = fnData.getData()

		corner1Point = om.MPoint( corner1[0], corner1[1], corner1[2] )
		corner2Point = om.MPoint( corner2[0], corner2[1], corner2[2] )

		return om.MBoundingBox( corner1Point, corner2Point )

	## Associates a user defined iterator with the shape (components)
	##
	def geometryIteratorSetup(self, componentList, components, forReadOnly=False):
		##
		## Description
		##
		##    Creates a geometry iterator compatible with his shape.
		##
		## Arguments
		##
		##    componentList - list of components to be iterated
		##    components    - component to be iterator
		##    forReadOnly   -
		##
		## Returns
		##
		##    An iterator for the components
		##

		if components.isNull():
			vtxComponents = om.MObjectArray([self.convertToVertexComponent(c) for c in componentList])
			return apiMeshGeomIterator( self.meshGeom(), vtxComponents )

		return apiMeshGeomIterator( self.meshGeom(), self.convertToVertexComponent(components) )

	def acceptsGeometryIterator(self, arg0, arg1=None, arg2=None):
		##
		## Description
		##
		##    Specifies that this shape can provide an iterator for getting/setting
		##    control point values.
		##
		## Arguments
		##
		##    writable   - maya asks for an iterator that can set points if this is True
		##
		##	  OR
		##
		##    component   - the component
		##    writable    - maya asks for an iterator that can set points if this is True
		##    forReadOnly - maya asking for an iterator for querying only
		##

		return True

	##########################################################
	##
	## Helper methods
	##
	##########################################################

	def hasHistory(self):
		##
		## Description
		##
		##    Returns True if the shape has input history, False otherwise.
		##
		return self.fHasHistoryOnCreate

	def shapeDirty(self):
		##
		## Description
		##
		##    Returns True if the input surface of the shape has been dirtied since
		##    the last reset of the flag
		##
		return self.fShapeDirty

	def resetShapeDirty(self):
		##
		## Description
		##
		##    Reset the shape dirty state of the node
		##
		self.fShapeDirty = False

	def materialDirty(self):
		##
		## Description
		##
		##    Returns true if the shading group of the shape has been changed since
		##    the last reset of the flag
		##
		return self.fMaterialDirty

	def setMaterialDirty(self, dirty):
		##
		## Description
		##
		##    Reset the material dirty state of the node
		##
		self.fMaterialDirty = dirty

	def computeInputSurface(self, plug, datablock):
		##
		## Description
		##
		##    If there is input history, evaluate the input attribute
		##

		## Get the input surface if there is history
		##
		if self.hasHistory():
			inputHandle = datablock.inputValue( apiMesh.inputSurface )

			surf = inputHandle.asPluginData()
			if not isinstance(surf, apiMeshData):
				raise RuntimeError("computeInputSurface : invalid inputSurface data found")

			## Create the cachedSurface and copy the input surface into it
			##
			fnDataCreator = om.MFnPluginData()
			fnDataCreator.create( apiMeshData.id )

			newCachedData = fnDataCreator.data()
			if not isinstance(newCachedData, apiMeshData):
				raise RuntimeError("computeInputSurface : invalid proxy cached apiMeshData object")

			newCachedData.fGeometry.copy(surf.fGeometry)

			cachedHandle = datablock.outputValue( apiMesh.cachedSurface )
			if not isinstance(cachedHandle, om.MDataHandle):
				raise RuntimeError("computeInputSurface : invalid cachedSurface")

			cachedHandle.setMPxData( newCachedData )

	def computeOutputSurface(self, plug, datablock):
		##
		## Description
		##
		##    Compute the outputSurface attribute.
		##
		##    If there is no history, use cachedSurface as the
		##    input surface. All tweaks will get written directly
		##    to it. Output is just a copy of the cached surface
		##    that can be connected etc.
		##

		## Check for an input surface. The input surface, if it
		## exists, is copied to the cached surface.
		##
		self.computeInputSurface( plug, datablock )

		## Get a handle to the cached data
		##
		cachedHandle = datablock.outputValue( apiMesh.cachedSurface )
		if not isinstance(cachedHandle, om.MDataHandle):
			raise RuntimeError("computeOutputSurface : invalid cachedSurface")

		cached = cachedHandle.asPluginData()
		if not isinstance(cached, apiMeshData):
			raise RuntimeError("computeOutputSurface : invalid cachedSurface data found")

		datablock.setClean( plug )

		## Apply any vertex offsets.
		##
		if self.hasHistory():
			self.applyTweaks( datablock, cached.fGeometry )

		else:
			cpHandle = datablock.inputArrayValue( om.MPxSurfaceShape.mControlPoints )
			cpHandle.setAllClean()

		## Create some output data
		##
		fnDataCreator = om.MFnPluginData()
		fnDataCreator.create( apiMeshData.id )

		newData = fnDataCreator.data()
		if not isinstance(newData, apiMeshData):
			raise RuntimeError("computeOutputSurface : invalid proxy cached apiMeshData object")

		## Copy the data
		##
		newData.fGeometry.copy(cached.fGeometry)

		## Assign the new data to the outputSurface handle
		##
		outHandle = datablock.outputValue( apiMesh.outputSurface )
		outHandle.setMPxData( newData )

		## Update the bounding box attributes
		##
		self.computeBoundingBox( datablock )

	def computeWorldSurface(self, plug, datablock):
		##
		## Description
		##
		##    Compute the worldSurface attribute.
		##

		self.computeOutputSurface( plug, datablock )
		inHandle = datablock.outputValue( apiMesh.outputSurface )
		outSurf = inHandle.asPluginData()
		if not isinstance(outSurf, apiMeshData):
			raise RuntimeError("computeWorldSurface : invalid outSurf")

		## Create some output data
		##
		fnDataCreator = om.MFnPluginData()
		fnDataCreator.create( apiMeshData.id )

		newData = fnDataCreator.data()
		if not isinstance(newData, apiMeshData):
			raise RuntimeError("computeWorldSurface : invalid proxy cached apiMeshData object")

		## Get worldMatrix from MPxSurfaceShape and set it to MPxGeometryData
		worldMat = self.getWorldMatrix(datablock, 0)
		newData.matrix = worldMat

		## Copy the data
		##
		newData.fGeometry.copy( outSurf.fGeometry )

		## Assign the new data to the outputSurface handle
		##
		arrayIndex = plug.logicalIndex()

		worldHandle = datablock.outputArrayValue( apiMesh.worldSurface )
		builder = worldHandle.builder()
		outHandle = builder.addElement( arrayIndex )

		outHandle.setMPxData( newData )

	def computeBoundingBox(self, datablock):
		##
		## Description
		##
		##    Use the larges/smallest vertex positions to set the corners
		##    of the bounding box.
		##

		## Update bounding box
		##
		lowerHandle = datablock.outputValue( apiMesh.bboxCorner1 )
		upperHandle = datablock.outputValue( apiMesh.bboxCorner2 )

		geometry = self.meshGeom()
		cnt = len(geometry.vertices)
		if cnt == 0:
			return

		## This clears any old bbox values
		##
		tmppnt = geometry.vertices[0]
		lower = [ tmppnt[0], tmppnt[1], tmppnt[2] ]
		upper = [ tmppnt[0], tmppnt[1], tmppnt[2] ]

		for i in xrange(cnt):
			pnt = geometry.vertices[i]

			if pnt[0] < lower[0]:	lower[0] = pnt[0]
			if pnt[1] < lower[1]:	lower[1] = pnt[1]
			if pnt[2] < lower[2]:	lower[2] = pnt[2]

			if pnt[0] > upper[0]:	upper[0] = pnt[0]
			if pnt[1] > upper[1]:	upper[1] = pnt[1]
			if pnt[2] > upper[2]:	upper[2] = pnt[2]

		lowerHandle.set3Double(lower[0], lower[1], lower[2])
		upperHandle.set3Double(upper[0], upper[1], upper[2])

		lowerHandle.setClean()
		upperHandle.setClean()

		## Signal that the bounding box has changed.
		##
		self.childChanged( om.MPxSurfaceShape.kBoundingBoxChanged )
		
	def convertToVertexComponent(self, components):
		"""
			Converts edge and face components into vertex components. This
			allows applying transform offsets to the vertex when edge or faces
			are selected.
		"""
		retVal = components

		try:
			srcComponent = om.MFnSingleIndexedComponent(components)
			srcComponentType = srcComponent.componentType
		except:
			return components
		
		if srcComponentType != om.MFn.kMeshVertComponent:
			srcIndices = set(srcComponent.getElements())
			retVal = srcComponent.create(om.MFn.kMeshVertComponent)
			vtxComponent = om.MFnSingleIndexedComponent(retVal)
			
			geomPtr = self.meshGeom()
			
			base = 0
			edgeId = 0
			for faceIdx in xrange(0, geomPtr.faceCount):
				# ignore degenerate faces
				numVerts = geomPtr.face_counts[faceIdx]
				if numVerts > 2:
					for v in xrange(0, numVerts):
						if srcComponentType == om.MFn.kMeshEdgeComponent:
							if edgeId in srcIndices:
								vindex1 = base + (v % numVerts)
								vindex2 = base + ((v+1) % numVerts)

								vertexId1 = geomPtr.face_connects[vindex1]
								vertexId2 = geomPtr.face_connects[vindex2]

								vtxComponent.addElement(vertexId1)
								vtxComponent.addElement(vertexId2)

							edgeId += 1
						else:
							# Face component:
							if faceIdx in srcIndices:
								vindex = base + (v % numVerts)
								vertexId = geomPtr.face_connects[vindex]
								vtxComponent.addElement(vertexId)

					base += numVerts;

		return retVal

	def applyTweaks(self, datablock, geometry):
		##
		## Description
		##
		##    If the shape has history, apply any tweaks (offsets) made
		##    to the control points.
		##

		cpHandle = datablock.inputArrayValue( om.MPxSurfaceShape.mControlPoints )

		## Loop through the component list and transform each vertex.
		##
		while not cpHandle.isDone():
			elemIndex = cpHandle.elementLogicalIndex()
			pntHandle = cpHandle.outputValue()

			offset = pntHandle.asDouble3()

			## Apply the tweaks to the output surface
			##
			geometry.vertices[elemIndex] += om.MPoint(offset[0],offset[1],offset[2])

			cpHandle.next()


	def updateCachedSurface(self, geometry, componentList):
		##
		## Description
		##
		##    Update the cached surface attribute, handle the tweak history as appropriate,
		##    and trigger a bounding box change calculation.
		##
		## Arguments
		##    geometry       - the modified geometry to apply to the cached surface attribute
		##

		## Retrieve the value of the cached surface attribute.
		## We will set the new geometry data into the cached surface attribute
		##
		## Access the datablock directly. This code has to be efficient
		## and so we bypass the compute mechanism completely.
		## NOTE: In general we should always go though compute for getting
		## and setting attributes.
		##
		datablock = self.forceCache()

		cachedHandle = datablock.outputValue( apiMesh.cachedSurface )
		cached = cachedHandle.asPluginData()

		dHandle = datablock.outputValue( om.MPxSurfaceShape.mControlPoints )

		## If there is history then calculate the tweaks necessary for
		## setting the final positions of the vertices.
		##
		if self.hasHistory() and cached:
			## Since the shape has history, we need to store the tweaks (deltas)
			## between the input shape and the tweaked shape in the control points
			## attribute.
			##
			self.buildControlPoints( datablock, len(geometry.vertices) )

			cpHandle = om.MArrayDataHandle( dHandle )

			## Loop through the component list and transform each vertex.
			##
			for comp in componentList:
				fnComp = om.MFnSingleIndexedComponent( self.convertToVertexComponent(comp) )
				for elemIndex in fnComp.getElements():
					cpHandle.jumpToLogicalElement( elemIndex )
					pntHandle = cpHandle.outputValue()

					pnt = pntHandle.asDouble3()

					oldPnt = cached.fGeometry.vertices[elemIndex]
					newPnt = geometry.vertices[elemIndex]
					offset = newPnt - oldPnt

					pnt[0] += offset[0]
					pnt[1] += offset[1]
					pnt[2] += offset[2]

					pntHandle.set3Double(pnt[0], pnt[1], pnt[2])

		## Copy outputSurface to cachedSurface
		##
		if cached:
			cached.fGeometry.copy(geometry)

		pCPs = om.MPlug( self.thisMObject(), om.MPxSurfaceShape.mControlPoints)
		pCPs.setMDataHandle(dHandle)

		## Moving vertices will likely change the bounding box.
		##
		self.computeBoundingBox( datablock )

		## Tell maya the bounding box for this object has changed
		## and thus "boundingBox()" needs to be called.
		##
		self.childChanged( om.MPxSurfaceShape.kBoundingBoxChanged )

		## Signal to the viewport that it needs to update the object
		self.signalDirtyToViewport()

	def getPointValue(self, pntInd):
		##
		## Description
		##
		##	  Helper function to return the value of a given vertex
		##    from the cachedMesh.
		##
		geometry = self.cachedGeom()
		if geometry:
			return geometry.vertices[ pntInd ]

		return om.MPoint()

	def getChannelValue(self, pntInd, vlInd):
		##
		## Description
		##
		##	  Helper function to return the value of a given vertex
		##    from the cachedMesh.
		##
		geometry = self.cachedGeom()
		if geometry:
			return geometry.vertices[ pntInd ][ vlInd ]

		return 0

	def setPointValue(self, pntInd, val):
		##
		## Description
		##
		##	  Helper function to set the value of a given vertex
		##    in the cachedMesh.
		##
		geometry = self.cachedGeom()
		if geometry:
			geometry.vertices[ pntInd ] = om.MPoint(val)
	
		self.verticesUpdated()

	def setChannelValue(self, pntInd, vlInd, val):
		##
		## Description
		##
		##	  Helper function to set the value of a given vertex
		##    in the cachedMesh.
		##
		geometry = self.cachedGeom()
		if geometry:
			geometry.vertices[ pntInd ][ vlInd ] = val
	
		self.verticesUpdated()

	def meshObject(self):
		##
		## Description
		##
		##    Get a reference to the mesh data (outputSurface)
		##    from the datablock. If dirty then an evaluation is
		##    triggered.
		##

		## Get the datablock for this node
		##
		datablock = self.forceCache()

		## Calling inputValue will force a recompute if the
		## connection is dirty. This means the most up-to-date
		## mesh data will be returned by this method.
		##
		handle = datablock.inputValue( apiMesh.outputSurface )
		return handle.data()

	def meshGeom(self):
		##
		## Description
		##
		##    Returns the apiMeshGeom of the outputSurface.
		##

		fnData = om.MFnPluginData( self.meshObject() )
		data = fnData.data()
		if not isinstance(data, apiMeshData):
			raise RuntimeError("meshGeom : failed to get apiMeshData")

		return data.fGeometry

	def cachedObject(self):
		##
		## Description
		##
		##    Get a reference to the mesh data (cachedSurface)
		##    from the datablock. No evaluation is triggered.
		##

		## Get the datablock for this node
		##
		datablock = self.forceCache()
		handle = datablock.outputValue( apiMesh.cachedSurface )
		return handle.data()

	def cachedGeom(self):
		##
		## Description
		##
		##    Returns the apiMeshGeom of the cachedSurface.
		##

		fnData = om.MFnPluginData( self.cachedObject() )
		data = fnData.data()
		if not isinstance(data, apiMeshData):
			raise RuntimeError("cachedGeom : failed to get apiMeshData")

		return data.fGeometry

	def buildControlPoints(self, datablock, count):
		##
		## Description
		##
		##    Check the controlPoints array. If there is input history
		##    then we will use this array to store tweaks (vertex movements).
		##

		cpH = datablock.outputArrayValue( om.MPxSurfaceShape.mControlPoints )

		oldBuilder = cpH.builder()
		if count != len(oldBuilder):
			## Make and set the new builder based on the
			## info from the old builder.
			builder = om.MArrayDataBuilder( oldBuilder )

			for vtx in xrange(count):
			  	builder.addElement( vtx )

			cpH.set( builder )

		cpH.setAllClean()

	def verticesUpdated(self):
		##
		## Description
		##
		##    Helper function to tell maya that this shape's
		##    vertices have updated and that the bbox needs
		##    to be recalculated and the shape redrawn.
		##
		self.childChanged( om.MPxSurfaceShape.kBoundingBoxChanged )
		self.childChanged( om.MPxSurfaceShape.kObjectChanged )

	def signalDirtyToViewport(self):
		self.fShapeDirty = True
		omr.MRenderer.setGeometryDrawDirty(self.thisMObject())

################################################################################
##
## apiMeshShapeUI
##
## Encapsulates the UI portion of a user defined shape. All of the
## drawing and selection code goes here.
##
################################################################################
class apiMeshUI(omui.MPxSurfaceShapeUI):

	@staticmethod
	def creator():
		return apiMeshUI()

	def __init__(self):
		omui.MPxSurfaceShapeUI.__init__(self)

	#####################################################################
	##
	## Overrides
	##
	#####################################################################

	## Main draw routine for UV editor. This is called by maya when the 
	## shape is selected and the UV texture window is visible. 
	## 
	def drawUV(self, view, info):
		## 
		## Description: 
		##   Main entry point for UV drawing. This method is called by the UV 
		##   texture editor when the shape is 'active'. 
		## 
		## Input: 
		##   A 3dView. 
		##

		meshNode = self.surfaceShape()
		geom = meshNode.meshGeom()

		if geom.uvcoords.uvcount() > 0:
			view.setDrawColor( om.MColor( (1.0, 0.0, 0.0) ) )
		
			if info.drawingFunction == omui.MTextureEditorDrawInfo.kDrawWireframe: 
				self.drawUVWireframe( geom, view, info )

			elif info.drawingFunction == omui.MTextureEditorDrawInfo.kDrawEverything or info.drawingFunction == omui.MTextureEditorDrawInfo.kDrawUVForSelect:
				self.drawUVWireframe( geom, view, info ) 
				self.drawUVMapCoordNum( geom, view, info, True )

			else:
				self.drawUVWireframe( geom, view, info )

	def canDrawUV(self):
		##
		## Description: 
		##  Tells Maya that this surface shape supports uv drawing. 
		##

		meshNode = self.surfaceShape()
		geom = meshNode.meshGeom()
		return (geom.uvcoords.uvcount() > 0)

	## Main selection routine
	##
	def select(self, selectInfo, selectionList, worldSpaceSelectPts):
		##
		## Description:
		##
		##     Main selection routine
		##
		## Arguments:
		##
		##     selectInfo           - the selection state information
		##     selectionList        - the list of selected items to add to
		##     worldSpaceSelectPts  -
		##

		selected = False
		componentSelected = False
		hilited = False

		hilited = (selectInfo.displayStatus() == omui.M3dView.kHilite)
		if hilited:
			componentSelected = self.selectVertices( selectInfo, selectionList, worldSpaceSelectPts )
			selected = selected or componentSelected

		if not selected:
			meshNode = self.surfaceShape()

			## NOTE: If the geometry has an intersect routine it should
			## be called here with the selection ray to determine if the
			## the object was selected.

			selected = True
			priorityMask = om.MSelectionMask( om.MSelectionMask.kSelectNurbsSurfaces )

			item = om.MSelectionList()
			item.add( selectInfo.selectPath() )

			xformedPt = om.MPoint()
			if selectInfo.singleSelection():
				center = meshNode.boundingBox().center
				xformedPt = center
				xformedPt *= selectInfo.selectPath().inclusiveMatrix()

			selectInfo.addSelection( item, xformedPt, selectionList, worldSpaceSelectPts, priorityMask, False )

		return selected

	#####################################################################
	##
	## Helper routines
	##
	#####################################################################

	def selectVertices(self, selectInfo, selectionList, worldSpaceSelectPts):
		##
		## Description:
		##
		##     Vertex selection.
		##
		## Arguments:
		##
		##     selectInfo           - the selection state information
		##     selectionList        - the list of selected items to add to
		##     worldSpaceSelectPts  -
		##

		selected = False
		view = selectInfo.view()
		path = selectInfo.multiPath()
		singleSelection = selectInfo.singleSelection()

		## Create a component that will store the selected vertices
		##
		fnComponent = om.MFnSingleIndexedComponent()
		surfaceComponent = fnComponent.create( om.MFn.kMeshVertComponent )

		## if the user did a single mouse click and we find > 1 selection
		## we will use the alignmentMatrix to find out which is the closest
		##
		alignmentMatrix = om.MMatrix()
		if singleSelection:
			alignmentMatrix = selectInfo.getAlignmentMatrix()

		singlePoint = om.MPoint()
		selectionPoint = om.MPoint()
		closestPointVertexIndex = -1
		previousZ = 0

		## Get the geometry information
		##
		meshNode = self.surfaceShape()
		geom = meshNode.meshGeom()

		## Loop through all vertices of the mesh and
		## see if they lie withing the selection area
		##
		for currentPoint in geom.vertices:
			## Sets OpenGL's render mode to select and stores
			## selected items in a pick buffer
			##
			view.beginSelect()

			glBegin( GL_POINTS )
			glVertex3f( currentPoint[0], currentPoint[1], currentPoint[2] )
			glEnd()

			if view.endSelect() > 0: ## Hit count > 0
				selected = True

				if singleSelection:
					xformedPoint = currentPoint
					xformedPoint.homogenize()
					xformedPoint *= alignmentMatrix
					z = xformedPoint.z
					if closestPointVertexIndex < 0 or z > previousZ:
						closestPointVertexIndex = vertexIndex
						singlePoint = currentPoint
						previousZ = z

				else:
					## multiple selection, store all elements
					##
					fnComponent.addElement( vertexIndex )

		## If single selection, insert the closest point into the array
		##
		if selected and singleSelection:
			fnComponent.addElement(closestPointVertexIndex)

			## need to get world space position for this vertex
			##
			selectionPoint = singlePoint
			selectionPoint *= path.inclusiveMatrix()

		## Add the selected component to the selection list
		##
		if selected:
			selectionItem = om.MSelectionList()
			selectionItem.add( path, surfaceComponent )

			mask = om.MSelectionMask( om.MSelectionMask.kSelectComponentsMask )
			selectInfo.addSelection( selectionItem, selectionPoint, selectionList, worldSpaceSelectPts, mask, True )

		return selected

	def drawUVWireframe(self, geom, view, info):
		##
		## Description: 
		##  Draws the UV layout in wireframe mode. 
		## 

		view.beginGL()
		
		## Draw the polygons
		##
		vid = 0
		vid_start = vid
		for i in xrange(geom.faceCount):
			glBegin( GL_LINES )

			vid_start = vid 
			for v in xrange(geom.face_counts[i]-1):
				uvId1 = geom.uvcoords.uvId(vid)
				uvId2 = geom.uvcoords.uvId(vid+1)

				uv1 = geom.uvcoords.getUV(uvId1)
				uv2 = geom.uvcoords.getUV(uvId2)

				glVertex3f( uv1[0], uv1[1], 0.0 ) 
				glVertex3f( uv2[0], uv2[1], 0.0 ) 
				vid += 1
			
			uvId1 = geom.uvcoords.uvId(vid)
			uvId2 = geom.uvcoords.uvId(vid_start)

			uv1 = geom.uvcoords.getUV(uvId1)
			uv2 = geom.uvcoords.getUV(uvId2)

			glVertex3f( uv1[0], uv1[1], 0.0 ) 
			glVertex3f( uv2[0], uv2[1], 0.0 ) 
			vid += 1

			glEnd()
		
		view.endGL()

	def drawUVMapCoord(self, view, uvId, uv, drawNumbers):
		##
		## Description: 
		##  Draw the specified uv value into the port view. If drawNumbers is True 
		##  It will also draw the UV id for the the UV.  
		##

		if drawNumbers:
			view.drawText( str(uvId), om.MPoint( uv[0], uv[1], 0 ), omui.M3dView.kCenter )

		glVertex3f( uv[0], uv[1], 0.0 )

	def drawUVMapCoordNum(self, geom, view, info, drawNumbers):
		##
		## Description: 
		##  Draw the UV points for all uvs on this surface shape. 
		##

		view.beginGL() 

		ptSize = glGetFloatv( GL_POINT_SIZE )
		glPointSize( 4.0 )

		for uvId in xrange(geom.uvcoords.uvcount()):
			uv = geom.uvcoords.getUV( uvId )
			self.drawUVMapCoord( view, uvId, uv, drawNumbers )

		glPointSize( ptSize )

		view.endGL() 
		
################################################################################
##
## apiMeshCreator
##
## A DG node that takes a maya mesh as input and outputs apiMeshData.
## If there is no input then the node creates a cube or sphere
## depending on what the shapeType attribute is set to.
##
################################################################################
class apiMeshCreator(om.MPxNode):
	id = om.MTypeId(0x80089)

	##########################################################
	##
	## Attributes
	##
	##########################################################
	size = None
	shapeType = None
	inputMesh = None
	outputSurface = None

	@staticmethod
	def creator():
		return apiMeshCreator()

	@staticmethod
	def initialize():
		typedAttr = om.MFnTypedAttribute()
		numericAttr = om.MFnNumericAttribute()
		enumAttr = om.MFnEnumAttribute()

		## ----------------------- INPUTS -------------------------
		apiMeshCreator.size = numericAttr.create( "size", "sz", om.MFnNumericData.kDouble, 1 )
		numericAttr.array = False
		numericAttr.usesArrayDataBuilder = False
		numericAttr.hidden = False
		numericAttr.keyable = True
		om.MPxNode.addAttribute( apiMeshCreator.size )

		apiMeshCreator.shapeType = enumAttr.create( "shapeType", "st", 0 )
		enumAttr.addField( "cube", 0 )
		enumAttr.addField( "sphere", 1 )
		enumAttr.hidden = False
		enumAttr.keyable = True
		om.MPxNode.addAttribute( apiMeshCreator.shapeType )
		
		apiMeshCreator.inputMesh = typedAttr.create( "inputMesh", "im", om.MFnData.kMesh, om.MObject.kNullObj )
		typedAttr.hidden = True
		om.MPxNode.addAttribute( apiMeshCreator.inputMesh )

		## ----------------------- OUTPUTS -------------------------
		apiMeshCreator.outputSurface = typedAttr.create( "outputSurface", "os", apiMeshData.id, om.MObject.kNullObj )
		typedAttr.writable = False
		om.MPxNode.addAttribute( apiMeshCreator.outputSurface )

		## ---------- Specify what inputs affect the outputs ----------
		##
		om.MPxNode.attributeAffects( apiMeshCreator.inputMesh, apiMeshCreator.outputSurface )
		om.MPxNode.attributeAffects( apiMeshCreator.size, apiMeshCreator.outputSurface )
		om.MPxNode.attributeAffects( apiMeshCreator.shapeType, apiMeshCreator.outputSurface )

	def __init__(self):
		om.MPxNode.__init__(self)

    ##########################################################
	##
	## Overrides
	##
    ##########################################################

	def compute(self, plug, datablock):
		##
		## Description
		##
		##    When input attributes are dirty this method will be called to
		##    recompute the output attributes.
		##

		if plug == apiMeshCreator.outputSurface:
			## Create some user defined geometry data and access the
			## geometry so we can set it
			##
			fnDataCreator = om.MFnPluginData()
			fnDataCreator.create( apiMeshData.id )

			newData = fnDataCreator.data()
			if not isinstance(newData, apiMeshData):
				raise RuntimeError("compute : invalid proxy cached apiMeshData object")

			geometry = newData.fGeometry

			## If there is an input mesh then copy it's values
			## and construct some apiMeshGeom for it.
			##
			hasHistory = self.computeInputMesh( plug, datablock, geometry )
												
			## There is no input mesh so check the shapeType attribute
			## and create either a cube or a sphere.
			##
			if not hasHistory:
				sizeHandle = datablock.inputValue( apiMeshCreator.size )
				shape_size = sizeHandle.asDouble()
				typeHandle = datablock.inputValue( apiMeshCreator.shapeType )
				shape_type = typeHandle.asShort()

				if shape_type == 0: ## build a cube
					self.buildCube( shape_size, geometry )
				elif shape_type == 1: ## build a sphere
					self.buildSphere( shape_size, 32, geometry )

			geometry.faceCount = len(geometry.face_counts)

			## Assign the new data to the outputSurface handle
			##
			outHandle = datablock.outputValue( apiMeshCreator.outputSurface )
			outHandle.setMPxData( newData )
			datablock.setClean( plug )

    ##########################################################
	##
	## Helper methods
	##
    ##########################################################

	def computeInputMesh(self, plug, datablock, geometry):
		##
		## Description
		##
		##     This function takes an input surface of type kMeshData and converts
		##     the geometry into this nodes attributes.
		##     Returns kFailure if nothing is connected.
		##

		## Get the input subdiv
		##        
		inputData = datablock.inputValue( apiMeshCreator.inputMesh )
		surf = inputData.asMesh()

		## Check if anything is connected
		##
		thisObj = self.thisMObject()
		surfPlug = om.MPlug( thisObj, apiMeshCreator.inputMesh )
		if not surfPlug.isConnected:
			datablock.setClean( plug )
			return False

		## Extract the mesh data
		##
		surfFn = om.MFnMesh(surf)
		geometry.vertices = surfFn.getPoints(om.MSpace.kObject)

		## Check to see if we have UVs to copy. 
		##
		hasUVs = surfFn.numUVs() > 0
		uvs = surfFn.getUVs()
		geometry.uvcoords.ucoord = uvs[0]
		geometry.uvcoords.vcoord = uvs[1]

		for i in xrange(surfFn.numPolygons()):
			polyVerts = surfFn.getPolygonVertices(i)

			pvc = len(polyVerts)
			geometry.face_counts.append( pvc )

			for v in xrange(pvc):
				if hasUVs:
					uvId = surfFn.getPolygonUVid(i, v)
					geometry.uvcoords.faceVertexIndex.append( uvId )
				geometry.face_connects.append( polyVerts[v] )

		for n in xrange(len(geometry.vertices)):
			normal = surfFn.getVertexNormal(n)
			geometry.normals.append( normal )

		return True

	def buildCube(self, cube_size, geometry):
		##
		## Description
		##
		##    Constructs a cube
		##

		geometry.vertices.clear()
		geometry.normals.clear()
	   	geometry.face_counts.clear()
		geometry.face_connects.clear()
		geometry.uvcoords.reset()

		geometry.vertices.append( om.MPoint( -cube_size, -cube_size, -cube_size ) )
		geometry.vertices.append( om.MPoint(  cube_size, -cube_size, -cube_size ) )
		geometry.vertices.append( om.MPoint(  cube_size, -cube_size, cube_size ) )
		geometry.vertices.append( om.MPoint( -cube_size, -cube_size, cube_size ) )
		geometry.vertices.append( om.MPoint( -cube_size, cube_size, -cube_size ) )
		geometry.vertices.append( om.MPoint( -cube_size, cube_size, cube_size ) )
		geometry.vertices.append( om.MPoint(  cube_size, cube_size, cube_size ) )
		geometry.vertices.append( om.MPoint(  cube_size, cube_size, -cube_size ) )

		normal_value = 0.5775
		geometry.normals.append( om.MVector( -normal_value, -normal_value, -normal_value ) )
		geometry.normals.append( om.MVector(  normal_value, -normal_value, -normal_value ) )
		geometry.normals.append( om.MVector(  normal_value, -normal_value, normal_value ) )
		geometry.normals.append( om.MVector( -normal_value, -normal_value, normal_value ) )
		geometry.normals.append( om.MVector( -normal_value, normal_value, -normal_value ) )
		geometry.normals.append( om.MVector( -normal_value, normal_value, normal_value ) )
		geometry.normals.append( om.MVector(  normal_value, normal_value, normal_value ) )
		geometry.normals.append( om.MVector(  normal_value, normal_value, -normal_value ) )

		## Define the UVs for the cube. 
		##
		uv_count = 14
		uv_pts = [	[ 0.375, 0.0  ],
					[ 0.625, 0.0  ],
					[ 0.625, 0.25 ],
					[ 0.375, 0.25 ],
					[ 0.625, 0.5  ],
					[ 0.375, 0.5  ],
					[ 0.625, 0.75 ],
					[ 0.375, 0.75 ],
					[ 0.625, 1.0  ],
					[ 0.375, 1.0  ],
					[ 0.875, 0.0  ],
					[ 0.875, 0.25 ],
					[ 0.125, 0.0  ],
					[ 0.125, 0.25 ] ]
		
		## UV Face Vertex Id. 
		##
		num_face_connects = 24
		uv_fvid = [ 0, 1, 2, 3, 
					3, 2, 4, 5, 
					5, 4, 6, 7, 
					7, 6, 8, 9, 
					1, 10, 11, 2, 
					12, 0, 3, 13 ]

		for i in xrange(uv_count):
			geometry.uvcoords.append_uv( uv_pts[i][0], uv_pts[i][1] )
		
		for i in xrange(num_face_connects):
			geometry.uvcoords.faceVertexIndex.append( uv_fvid[i] ) 

		## Set up an array containing the number of vertices
		## for each of the 6 cube faces (4 verticies per face)
		##
		num_faces = 6
		face_counts = [ 4, 4, 4, 4, 4, 4 ]

		for i in xrange(num_faces):
			geometry.face_counts.append( face_counts[i] )

		## Set up and array to assign vertices from vertices to each face 
		##
		face_connects = [ 0, 1, 2, 3,
						  4, 5, 6, 7,
						  3, 2, 6, 5,
						  0, 3, 5, 4,
						  0, 4, 7, 1,
						  1, 7, 6, 2 ]

		for i in xrange(num_face_connects):
			geometry.face_connects.append( face_connects[i] )

	def buildSphere(self, radius, divisions, geometry):
		##
		## Description
		##
		##    Create circles of vertices starting with 
		##    the top pole ending with the botton pole
		##

		geometry.vertices.clear()
		geometry.normals.clear()
	   	geometry.face_counts.clear()
		geometry.face_connects.clear()
		geometry.uvcoords.reset()

		u = -math.pi / 2.
		v = -math.pi
		u_delta = math.pi / divisions
		v_delta = 2 * math.pi / divisions

		topPole = om.MPoint( 0.0, radius, 0.0 )
		botPole = om.MPoint( 0.0, -radius, 0.0 )

		## Build the vertex and normal table
		##
		geometry.vertices.append( botPole )
		geometry.normals.append( botPole - om.MPoint.kOrigin )

		for i in xrange(divisions-1):
			u += u_delta
			v = -math.pi

			for j in xrange(divisions):
				x = radius * math.cos(u) * math.cos(v)
				y = radius * math.sin(u)
				z = radius * math.cos(u) * math.sin(v)

				pnt = om.MPoint( x, y, z )
				geometry.vertices.append( pnt )
				geometry.normals.append( pnt - om.MPoint.kOrigin )
				v += v_delta

		geometry.vertices.append( topPole )
		geometry.normals.append( topPole - om.MPoint.kOrigin )

		## Create the connectivity lists
		##
		vid = 1
		numV = 0
		for i in xrange(divisions):
			for j in xrange(divisions):
				if i == 0:
					geometry.face_counts.append( 3 )

					geometry.face_connects.append( 0 )
					geometry.face_connects.append( j + vid )
					if j == divisions-1:
						geometry.face_connects.append( vid )
					else:
						geometry.face_connects.append( j + vid + 1 )

				elif i == divisions-1:
					geometry.face_counts.append( 3 )

					geometry.face_connects.append( j + vid + 1 - divisions )
					geometry.face_connects.append( vid + 1 )
					if j == divisions-1:
						geometry.face_connects.append( vid + 1 - divisions )
					else:
						geometry.face_connects.append( j + vid + 2 - divisions )

				else:
					geometry.face_counts.append( 4 )

					geometry.face_connects.append( j + vid + 1 - divisions )
					geometry.face_connects.append( j + vid + 1 )
					if j == divisions-1:
						geometry.face_connects.append( vid + 1 )
						geometry.face_connects.append( vid + 1 - divisions )
					else:
						geometry.face_connects.append( j + vid + 2 )
						geometry.face_connects.append( j + vid + 2 - divisions )

				numV += 1

			vid = numV

## Helper class for link lost callback
class ShadedItemUserData(om.MUserData):
	def __init__(self, override):
		om.MUserData.__init__(self, False)
		self.fOverride = override

## Custom user data class to attach to face selection render item
## to help with viewport 2.0 selection
class apiMeshHWSelectionUserData(om.MUserData):
	def __init__(self):
		om.MUserData.__init__(self, True)	## let Maya clean up
		self.fMeshGeom = None

################################################################################
##
## apiMeshSubSceneOverride
##
## Handles vertex data preparation for drawing the user defined shape in
## Viewport 2.0.
##
################################################################################
## Custom component converter to select components
## Attached to the vertex, edge and face selection render items
## respectively apiMeshSubSceneOverride.sVertexSelectionName, apiMeshSubSceneOverride.sEdgeSelectionName
## and apiMeshSubSceneOverride.sFaceSelectionName
class simpleComponentConverter_subsceneOverride(omr.MPxComponentConverter):
	def __init__(self, componentType, selectionType):
		omr.MPxComponentConverter.__init__(self)

		self.fComponentType = componentType
		self.fSelectionType = selectionType

		self.fComponent = om.MFnSingleIndexedComponent()
		self.fComponentObject = om.MObject.kNullObj
		self.fLookupTable = []

	def initialize(self, renderItem):
		## Create the component selection object
		self.fComponentObject = self.fComponent.create( self.fComponentType )

		## For face selection, 
		## create a lookup table to match triangle intersection with face id :
		## One face may contains more than one triangle
		if self.fComponentType == om.MFn.kMeshPolygonComponent:
			selectionData = renderItem.customData()
			if isinstance(selectionData, apiMeshHWSelectionUserData):
				meshGeom = selectionData.fMeshGeom

				## Allocate faces lookup table
				numTriangles = 0
				for i in xrange(meshGeom.faceCount):
					numVerts = meshGeom.face_counts[i]
					if numVerts > 2:
						numTriangles += numVerts - 2
				self.fLookupTable = [0]*numTriangles

				## Fill faces lookup table
				triId = 0
				for faceId in xrange(meshGeom.faceCount):
					## ignore degenerate faces
					numVerts = meshGeom.face_counts[faceId]
					if numVerts > 2:
						for v in xrange(1, numVerts-1):
							self.fLookupTable[triId] = faceId
							triId += 1

	def addIntersection(self, intersection):
		## Convert the intersection index, which represent the primitive position in the
		## index buffer, to the correct component id

		## For vertex and edge: the primitive index value is the same as the component id
		## For face: get the face id that matches the triangle index from the lookup table

		if self.fComponentType == om.MFn.kMeshEdgeComponent:
			# Only accept edge selection intersection on draw instance #2 -- scaled by 2
			# and instance #-1 (when useDrawInstancingOnEdgeSelectionItem is False)
			if intersection.instanceID == 1 or intersection.instanceID == 3:
				return

		idx = intersection.index

		if self.fComponentType == om.MFn.kMeshPolygonComponent:
			if idx >= 0 and idx < len(self.fLookupTable):
				idx = self.fLookupTable[idx]

		self.fComponent.addElement(idx)

	def component(self):
		## Return the component object that contains the ids of the selected components
		return self.fComponentObject

	def selectionMask(self):
		## This converter is only valid for specified selection type
		return self.fSelectionType

	## creator function to instanciate a component converter for vertex selection
	@staticmethod
	def creatorVertexSelection():
		mask = om.MSelectionMask(om.MSelectionMask.kSelectMeshVerts)
		mask.addMask(om.MSelectionMask.kSelectPointsForGravity)
		return simpleComponentConverter_subsceneOverride(om.MFn.kMeshVertComponent, mask)

	## creator function to instanciate a component converter for edge selection
	@staticmethod
	def creatorEdgeSelection():
		return simpleComponentConverter_subsceneOverride(om.MFn.kMeshEdgeComponent, om.MSelectionMask.kSelectMeshEdges)

	## creator function to instanciate a component converter for face selection
	@staticmethod
	def creatorFaceSelection():
		return simpleComponentConverter_subsceneOverride(om.MFn.kMeshPolygonComponent, om.MSelectionMask.kSelectMeshFaces)

class apiMeshSubSceneOverride(omr.MPxSubSceneOverride):
	sWireName        = "apiMeshWire"
	sSelectName      = "apiMeshSelection"
	sBoxName         = "apiMeshBox"
	sSelectedBoxName = "apiMeshBoxSelection"
	sShadedName      = "apiMeshShaded"

	sVertexSelectionName = "apiMeshVertexSelection"
	sEdgeSelectionName   = "apiMeshEdgeSelection"
	sFaceSelectionName   = "apiMeshFaceSelection"

	sActiveVertexName = "apiMeshActiveVertex"
	sActiveEdgeName   = "apiMeshActiveEdge"
	sActiveFaceName   = "apiMeshActiveFace"

	class InstanceInfo:
		def __init__(self, transform, isSelected):
			self.fTransform = transform
			self.fIsSelected = isSelected

	@staticmethod
	def creator(obj):
		return apiMeshSubSceneOverride(obj)

	@staticmethod
	def shadedItemLinkLost(userData):
		if not userData is None and not userData.fOverride is None:
			if not userData.fOverride.fMesh is None:
				userData.fOverride.fMesh.setMaterialDirty(True)
			userData.fOverride = None
		userData = None

	def __init__(self, obj):
		omr.MPxSubSceneOverride.__init__(self, obj)
		
		node = om.MFnDependencyNode(obj)
		self.fMesh = node.userNode()
		self.fObject = om.MObject(obj)
		self.fWireShader = None
		self.fThickWireShader = None
		self.fSelectShader = None
		self.fThickSelectShader = None
		self.fShadedShader = None
		self.fVertexComponentShader = None
		self.fEdgeComponentShader = None
		self.fFaceComponentShader = None
		self.fPositionBuffer = None
		self.fNormalBuffer = None
		self.fBoxPositionBuffer = None
		self.fWireIndexBuffer = None
		self.fVertexIndexBuffer = None
		self.fBoxIndexBuffer = None
		self.fShadedIndexBuffer = None
		self.fActiveVerticesIndexBuffer = None
		self.fActiveEdgesIndexBuffer = None
		self.fActiveFacesIndexBuffer = None
		self.fThickLineWidth = -1.0
		self.fQueuedLineWidth = -1.0
		self.fNumInstances = 0
		self.fIsInstanceMode = False
		self.fQueueUpdate = False
		self.fUseQueuedLineUpdate = False ## Set to True to run sample line width update code

		self.fInstanceInfoCache = collections.defaultdict(set)

		self.fActiveVerticesSet = set()
		self.fActiveEdgesSet = set()
		self.fActiveFacesSet = set()

	def __del__(self):
		self.fMesh = None

		shaderMgr = omr.MRenderer.getShaderManager()
		if shaderMgr:
			if self.fWireShader:
				shaderMgr.releaseShader(self.fWireShader)
				self.fWireShader = None

			if self.fThickWireShader:
				shaderMgr.releaseShader(self.fThickWireShader)
				self.fThickWireShader = None

			if self.fSelectShader:
				shaderMgr.releaseShader(self.fSelectShader)
				self.fSelectShader = None

			if self.fThickSelectShader:
				shaderMgr.releaseShader(self.fThickSelectShader)
				self.fThickSelectShader = None

			if self.fShadedShader:
				shaderMgr.releaseShader(self.fShadedShader)
				self.fShadedShader = None

			if self.fVertexComponentShader:
				shaderMgr.releaseShader(self.fVertexComponentShader)
				self.fVertexComponentShader = None

			if self.fEdgeComponentShader:
				shaderMgr.releaseShader(self.fEdgeComponentShader)
				self.fEdgeComponentShader = None

			if self.fFaceComponentShader:
				shaderMgr.releaseShader(self.fFaceComponentShader)
				self.fFaceComponentShader = None

		self.clearBuffers()

	def supportedDrawAPIs(self):
		## this plugin supports both GL and DX
		return omr.MRenderer.kOpenGL | omr.MRenderer.kDirectX11 | omr.MRenderer.kOpenGLCoreProfile

	def requiresUpdate(self, container, frameContext):
		# Nothing in the container, definitely need to update
		if len(container) == 0:
			return True

		# Update always. This could be optimized to only update when the
		# actual shape node detects a change.
		return True

	def update(self, container, frameContext):
		# Update render items based on current set of instances
		self.manageRenderItems(container, frameContext, self.fMesh.shapeDirty() or len(container) == 0)

		# Always reset shape dirty flag
		self.fMesh.resetShapeDirty()

	def furtherUpdateRequired(self, frameContext):
		if self.fUseQueuedLineUpdate:
			if not frameContext.inUserInteraction() and not frameContext.userChangingViewContext():
				return self.fQueueUpdate

		return False

	def manageRenderItems(self, container, frameContext, updateGeometry):
		## Preamble
		if not self.fMesh or self.fObject.isNull():
			return

		shaderMgr = omr.MRenderer.getShaderManager()
		if not shaderMgr:
			return

		node = om.MFnDagNode(self.fObject)
		instances = node.getAllPaths()
		if len(instances) == 0:
			return

		## Constants
		sRed     = [1.0, 0.0, 0.0, 1.0]
		sGreen   = [0.0, 1.0, 0.0, 1.0]
		sWhite   = [1.0, 1.0, 1.0, 1.0]

		## Set up shared shaders if needed
		if not self.fWireShader:
			self.fWireShader = shaderMgr.getStockShader(omr.MShaderManager.k3dSolidShader)
			self.fWireShader.setParameter("solidColor", sRed)

		if not self.fThickWireShader:
			self.fThickWireShader = shaderMgr.getStockShader(omr.MShaderManager.k3dThickLineShader)
			self.fThickWireShader.setParameter("solidColor", sRed)

		if not self.fSelectShader:
			self.fSelectShader = shaderMgr.getStockShader(omr.MShaderManager.k3dSolidShader)
			self.fSelectShader.setParameter("solidColor", sGreen)

		if not self.fThickSelectShader:
			self.fThickSelectShader = shaderMgr.getStockShader(omr.MShaderManager.k3dThickLineShader)
			self.fThickSelectShader.setParameter("solidColor", sGreen)

		if not self.fVertexComponentShader:
			self.fVertexComponentShader = shaderMgr.getStockShader(omr.MShaderManager.k3dFatPointShader)
			self.fVertexComponentShader.setParameter("solidColor", sWhite)
			self.fVertexComponentShader.setParameter("pointSize", [5.0, 5.0])

		if not self.fEdgeComponentShader:
			self.fEdgeComponentShader = shaderMgr.getStockShader(omr.MShaderManager.k3dThickLineShader)
			self.fEdgeComponentShader.setParameter("solidColor", sWhite)
			self.fEdgeComponentShader.setParameter("lineWidth", [2.0, 2.0])

		if not self.fFaceComponentShader:
			self.fFaceComponentShader = shaderMgr.getStockShader(omr.MShaderManager.k3dSolidShader)
			self.fFaceComponentShader.setParameter("solidColor", sWhite)

		## Set up shared geometry if necessary
		if updateGeometry:
			self.rebuildGeometryBuffers()

		if not all((self.fPositionBuffer, self.fNormalBuffer, self.fBoxPositionBuffer, self.fWireIndexBuffer, self.fVertexIndexBuffer, self.fBoxIndexBuffer, self.fShadedIndexBuffer)):
			print "At least one buffer is not set"
			return

		selectedList = om.MGlobal.getActiveSelectionList()

		anyMatrixChanged = False
		itemsChanged = False
		instanceArrayLength = len(instances)
		numInstanceSelected = 0
		numInstanceUnselected = 0
		numInstances = 0

		instanceMatrixArray = om.MMatrixArray(instanceArrayLength)
		selectedInstanceMatrixArray = om.MMatrixArray(instanceArrayLength)
		unselectedInstanceMatrixArray = om.MMatrixArray(instanceArrayLength)

		for instance in instances:
			## If expecting large numbers of instances, then walking through the whole
			## list of instances every time to look for changes is not efficient
			## enough.  Watching for change events and changing only the required
			## instances should be done instead.  This method of checking for selection
			## status is also not fast.
			if not instance.isValid or not instance.isVisible:
				continue

			instanceNum = instance.instanceNumber()

			instanceInfo = apiMeshSubSceneOverride.InstanceInfo(instance.inclusiveMatrix(), useSelectHighlight(selectedList, instance))

			if instanceNum not in self.fInstanceInfoCache or self.fInstanceInfoCache[instanceNum].fIsSelected != instanceInfo.fIsSelected or not self.fInstanceInfoCache[instanceNum].fTransform.isEquivalent(instanceInfo.fTransform):
				self.fInstanceInfoCache[instanceNum] = instanceInfo
				anyMatrixChanged = True

			instanceMatrixArray[numInstances] = instanceInfo.fTransform
			numInstances += 1
			if instanceInfo.fIsSelected:
				selectedInstanceMatrixArray[numInstanceSelected] = instanceInfo.fTransform
				numInstanceSelected += 1
			else:
				unselectedInstanceMatrixArray[numInstanceUnselected] = instanceInfo.fTransform
				numInstanceUnselected += 1

		instanceMatrixArray.setLength(numInstances)  ## collapse to correct length
		selectedInstanceMatrixArray.setLength(numInstanceSelected)
		unselectedInstanceMatrixArray.setLength(numInstanceUnselected)
		if self.fNumInstances != numInstances:
			anyMatrixChanged = True
			self.fNumInstances = numInstances

		anyInstanceSelected = numInstanceSelected > 0
		anyInstanceUnselected = numInstanceUnselected > 0

		activeVerticesSet = set()
		activeEdgesSet = set()
		activeFacesSet = set()

		meshGeom = self.fMesh.meshGeom()
		if meshGeom and self.fMesh.hasActiveComponents():
			activeComponents = self.fMesh.activeComponents()
			if len(activeComponents) > 0:
				fnComponent = om.MFnSingleIndexedComponent( activeComponents[0] )
				if fnComponent.elementCount > 0:
					activeIds = fnComponent.getElements()

					if fnComponent.componentType == om.MFn.kMeshVertComponent:
						activeVerticesSet = set(activeIds)

					elif fnComponent.componentType == om.MFn.kMeshEdgeComponent:
						activeEdgesSet = set(activeIds)

					elif fnComponent.componentType == om.MFn.kMeshPolygonComponent:
						activeFacesSet = set(activeIds)

		## Update index buffer of active items if necessary
		updateActiveItems = updateGeometry or self.fActiveVerticesSet != activeVerticesSet or self.fActiveEdgesSet != activeEdgesSet or self.fActiveFacesSet != activeFacesSet
		self.fActiveVerticesSet = activeVerticesSet
		self.fActiveEdgesSet = activeEdgesSet
		self.fActiveFacesSet = activeFacesSet

		if updateActiveItems:
			self.rebuildActiveComponentIndexBuffers()

		anyVertexSelected = bool(self.fActiveVerticesSet)
		anyEdgeSelected = bool(self.fActiveEdgesSet)
		anyFaceSelected = bool(self.fActiveFacesSet)

		if (anyVertexSelected and not self.fActiveVerticesIndexBuffer) or (anyEdgeSelected and not self.fActiveEdgesIndexBuffer) or (anyFaceSelected and not self.fActiveFacesIndexBuffer):
			return

		## Add render items if necessary.  Remove any pre-existing render items
		## that are no longer needed.
		wireItem = container.find(self.sWireName)
		if not wireItem and anyInstanceUnselected:
			wireItem = omr.MRenderItem.create( self.sWireName, omr.MRenderItem.DecorationItem, omr.MGeometry.kLines)
			wireItem.setDrawMode(omr.MGeometry.kWireframe)
			wireItem.setDepthPriority(omr.MRenderItem.sActiveWireDepthPriority)
			wireItem.setShader(self.fWireShader)
			container.add(wireItem)
			itemsChanged = True

		elif wireItem and not anyInstanceUnselected:
			container.remove(self.sWireName)
			wireItem = None
			itemsChanged = True

		selectItem = container.find(self.sSelectName)
		if not selectItem and anyInstanceSelected:
			selectItem = omr.MRenderItem.create( self.sSelectName, omr.MRenderItem.DecorationItem, omr.MGeometry.kLines)
			selectItem.setDrawMode(omr.MGeometry.kWireframe | omr.MGeometry.kShaded | omr.MGeometry.kTextured)
			selectItem.setDepthPriority(omr.MRenderItem.sActiveWireDepthPriority)
			selectItem.setShader(self.fSelectShader)
			container.add(selectItem)
			itemsChanged = True

		elif selectItem and not anyInstanceSelected:
			container.remove(self.sSelectName)
			selectItem = None
			itemsChanged = True

		boxItem = container.find(self.sBoxName)
		if not boxItem and anyInstanceUnselected:
			boxItem = omr.MRenderItem.create( self.sBoxName, omr.MRenderItem.NonMaterialSceneItem, omr.MGeometry.kLines)
			boxItem.setDrawMode(omr.MGeometry.kBoundingBox)
			boxItem.setShader(self.fWireShader)
			container.add(boxItem)
			itemsChanged = True

		elif boxItem and not anyInstanceUnselected:
			container.remove(self.sBoxName)
			boxItem = None
			itemsChanged = True

		selectedBoxItem = container.find(self.sSelectedBoxName)
		if not selectedBoxItem and anyInstanceSelected:
			selectedBoxItem = omr.MRenderItem.create( self.sSelectedBoxName, omr.MRenderItem.NonMaterialSceneItem, omr.MGeometry.kLines)
			selectedBoxItem.setDrawMode(omr.MGeometry.kBoundingBox)
			selectedBoxItem.setShader(self.fSelectShader)
			container.add(selectedBoxItem)
			itemsChanged = True

		elif selectedBoxItem and not anyInstanceSelected:
			container.remove(self.sSelectedBoxName)
			selectedBoxItem = None
			itemsChanged = True

		shadedItem = container.find(self.sShadedName)
		if not shadedItem:
			## We always want a shaded item
			shadedItem = omr.MRenderItem.create( self.sShadedName, omr.MRenderItem.MaterialSceneItem, omr.MGeometry.kTriangles)
			shadedItem.setDrawMode(omr.MGeometry.kShaded | omr.MGeometry.kTextured)
			shadedItem.setExcludedFromPostEffects(False)
			shadedItem.setCastsShadows(True)
			shadedItem.setReceivesShadows(True)
			container.add(shadedItem)
			itemsChanged = True

		## Update shader for shaded item
		if self.fMesh.materialDirty() or (self.fShadedShader is None and not shadedItem.isShaderFromNode()):
			hasSetShaderFromNode = False

			## Grab shading node from first component of first instance of the
			## object and use it to get an MShaderInstance. This could be expanded
			## to support full instancing and components if necessary.
			try:
				(sets, comps) = node.getConnectedSetsAndMembers(0, True)
				for obj in sets:
					dn = om.MFnDependencyNode(obj)
					shaderPlug = dn.findPlug("surfaceShader", True)
					connectedPlugs = shaderPlug.connectedTo(True, False)

					if len(connectedPlugs) > 0:
						userData = ShadedItemUserData(self)

						if shadedItem.setShaderFromNode(connectedPlugs[0].node(), instances[0], apiMeshSubSceneOverride.shadedItemLinkLost, userData):
							hasSetShaderFromNode = True
							break;
			except:
				pass

			if not hasSetShaderFromNode:
				if not self.fShadedShader:
					self.fShadedShader = shaderMgr.getStockShader(omr.MShaderManager.k3dBlinnShader)

				shadedItem.setShader(self.fShadedShader)

			self.fMesh.setMaterialDirty(False)

		# render item for vertex selection
		vertexSelectionItem = container.find(self.sVertexSelectionName)
		if not vertexSelectionItem:
			vertexSelectionItem = omr.MRenderItem.create( self.sVertexSelectionName, omr.MRenderItem.DecorationItem, omr.MGeometry.kPoints)
			# use for selection only : not visible in viewport
			vertexSelectionItem.setDrawMode(omr.MGeometry.kSelectionOnly)
			# set the selection mask to kSelectMeshVerts : we want the render item to be used for Vertex Components selection
			mask = om.MSelectionMask(om.MSelectionMask.kSelectMeshVerts)
			mask.addMask(om.MSelectionMask.kSelectPointsForGravity)
			vertexSelectionItem.setSelectionMask( mask )
			# set selection priority : on top
			vertexSelectionItem.setDepthPriority(omr.MRenderItem.sSelectionDepthPriority)
			vertexSelectionItem.setShader(self.fVertexComponentShader)
			container.add(vertexSelectionItem)
			itemsChanged = True

		# change this value to enable item instancing : same item will be rendered multiple times
		# the edge selection item will be visible in the viewport and rendered 3 times (with different scaling)
		# only the second instance (scale 1.5) will be selectable (see simpleComponentConverter_subsceneOverride)
		useDrawInstancingOnEdgeSelectionItem = False

		# render item for edge selection
		edgeSelectionItem = container.find(self.sEdgeSelectionName)
		if not edgeSelectionItem:
			# use for selection only : not visible in viewport
			drawMode = omr.MGeometry.kSelectionOnly
			depthPriority = omr.MRenderItem.sSelectionDepthPriority
			if useDrawInstancingOnEdgeSelectionItem:
				# display in viewport and in selection
				drawMode = omr.MGeometry.kAll
				# reduce priority so we can see the active item
				depthPriority = omr.MRenderItem.sActiveWireDepthPriority-1

			edgeSelectionItem = omr.MRenderItem.create( self.sEdgeSelectionName, omr.MRenderItem.DecorationItem, omr.MGeometry.kLines)
			edgeSelectionItem.setDrawMode(drawMode)
			# set selection priority : on top
			edgeSelectionItem.setDepthPriority(depthPriority)
			# set the selection mask to kSelectMeshEdges : we want the render item to be used for Edge Components selection
			edgeSelectionItem.setSelectionMask( om.MSelectionMask.kSelectMeshEdges )
			edgeSelectionItem.setShader(self.fWireShader)
			container.add(edgeSelectionItem)
			itemsChanged = True

		# render item for face selection - not visible in viewport
		faceSelectionItem = container.find(self.sFaceSelectionName)
		if not faceSelectionItem:
			faceSelectionItem = omr.MRenderItem.create( self.sFaceSelectionName, omr.MRenderItem.DecorationItem, omr.MGeometry.kTriangles)
			# use for selection only : not visible in viewport
			faceSelectionItem.setDrawMode(omr.MGeometry.kSelectionOnly)
			# set the selection mask to kSelectMeshFaces : we want the render item to be used for Face Components selection
			faceSelectionItem.setSelectionMask( om.MSelectionMask.kSelectMeshFaces )
			# set selection priority : on top
			faceSelectionItem.setDepthPriority(omr.MRenderItem.sSelectionDepthPriority)
			faceSelectionItem.setShader(self.fFaceComponentShader)
			container.add(faceSelectionItem)
			itemsChanged = True

		## create and add a custom data to help the face component converter
		if updateGeometry:
			mySelectionData = apiMeshHWSelectionUserData()
			mySelectionData.fMeshGeom = self.fMesh.meshGeom()
			faceSelectionItem.setCustomData(mySelectionData)

		# render item to display active (selected) vertices
		activeVertexItem = container.find(self.sActiveVertexName)
		if not activeVertexItem and anyVertexSelected:
			activeVertexItem = omr.MRenderItem.create( self.sActiveVertexName, omr.MRenderItem.DecorationItem, omr.MGeometry.kPoints)
			activeVertexItem.setDrawMode(omr.MGeometry.kAll)
			activeVertexItem.setDepthPriority(omr.MRenderItem.sActivePointDepthPriority)
			activeVertexItem.setShader(self.fVertexComponentShader)
			container.add(activeVertexItem)
			itemsChanged = True

		elif activeVertexItem and not anyVertexSelected:
			container.remove(self.sActiveVertexName)
			activeVertexItem = None
			itemsChanged = True

		# render item to display active (selected) edges
		activeEdgeItem = container.find(self.sActiveEdgeName)
		if not activeEdgeItem and anyEdgeSelected:
			activeEdgeItem = omr.MRenderItem.create( self.sActiveEdgeName, omr.MRenderItem.DecorationItem, omr.MGeometry.kLines)
			activeEdgeItem.setDrawMode(omr.MGeometry.kAll)
			activeEdgeItem.setDepthPriority(omr.MRenderItem.sActiveLineDepthPriority)
			activeEdgeItem.setShader(self.fEdgeComponentShader)
			container.add(activeEdgeItem)
			itemsChanged = True

		elif activeEdgeItem and not anyEdgeSelected:
			container.remove(self.sActiveEdgeName)
			activeEdgeItem = None
			itemsChanged = True

		# render item to display active (selected) faces
		activeFaceItem = container.find(self.sActiveFaceName)
		if not activeFaceItem and anyFaceSelected:
			activeFaceItem = omr.MRenderItem.create( self.sActiveFaceName, omr.MRenderItem.DecorationItem, omr.MGeometry.kTriangles)
			activeFaceItem.setDrawMode(omr.MGeometry.kAll)
			activeFaceItem.setDepthPriority(omr.MRenderItem.sActiveLineDepthPriority)
			activeFaceItem.setShader(self.fFaceComponentShader)
			container.add(activeFaceItem)
			itemsChanged = True

		elif activeFaceItem and not anyFaceSelected:
			container.remove(self.sActiveFaceName)
			activeFaceItem = None
			itemsChanged = True

		## update the line width on the shader for our wire items if it changed
		lineWidth = frameContext.getGlobalLineWidth()
		userWidthChange = not floatApproxEqual(lineWidth, self.fThickLineWidth)

		doUpdate = False
		targetRefinedLineWidth = 50.0
		if userWidthChange:
			self.fThickLineWidth = lineWidth
			doUpdate = True

			## First user change will trigger an update requirement
			if self.fUseQueuedLineUpdate:
				self.fQueuedLineWidth = lineWidth
				if self.fQueuedLineWidth < targetRefinedLineWidth:
					self.fQueueUpdate = True

		else:
			## Increment by 1 until we reach the target width.
			## If we haven't reached it yet then queue another update
			## so we can increment and retest against the target width.
			if self.fUseQueuedLineUpdate and self.fQueueUpdate:
				if self.fQueuedLineWidth < targetRefinedLineWidth:
					lineWidth = self.fQueuedLineWidth
					self.fQueuedLineWidth += 1
					self.fQueueUpdate = True
					doUpdate = True

				else:
					## Reached target. Stop asking for more refinement
					self.fQueueUpdate = False
		
		if doUpdate:
			if not floatApproxEqual(lineWidth, 1.0):
				## Only set the shader if the line width changes (or the first time)
				lineWidthArray = [ lineWidth, lineWidth ]
				self.fThickWireShader.setParameter("lineWidth", lineWidthArray)
				self.fThickSelectShader.setParameter("lineWidth", lineWidthArray)
				if wireItem:
					wireItem.setShader(self.fThickWireShader)
				if selectItem:
					selectItem.setShader(self.fThickSelectShader)

			else:
				if wireItem:
					wireItem.setShader(self.fWireShader)
				if selectItem:
					selectItem.setShader(self.fSelectShader)

		## Update geometry if required
		if itemsChanged or updateGeometry:
			bounds = self.fMesh.boundingBox()

			wireBuffers = omr.MVertexBufferArray()
			wireBuffers.append(self.fPositionBuffer, "positions")
			if wireItem:
				self.setGeometryForRenderItem(wireItem, wireBuffers, self.fWireIndexBuffer, bounds)
			if selectItem:
				self.setGeometryForRenderItem(selectItem, wireBuffers, self.fWireIndexBuffer, bounds)

			boxBuffers = omr.MVertexBufferArray()
			boxBuffers.append(self.fBoxPositionBuffer, "positions")
			if boxItem:
				self.setGeometryForRenderItem(boxItem, boxBuffers, self.fBoxIndexBuffer, bounds)
			if selectedBoxItem:
				self.setGeometryForRenderItem(selectedBoxItem, boxBuffers, self.fBoxIndexBuffer, bounds)

			shadedBuffers = omr.MVertexBufferArray()
			shadedBuffers.append(self.fPositionBuffer, "positions")
			shadedBuffers.append(self.fNormalBuffer, "normals")
			self.setGeometryForRenderItem(shadedItem, shadedBuffers, self.fShadedIndexBuffer, bounds)
			
			self.setGeometryForRenderItem(vertexSelectionItem, wireBuffers, self.fVertexIndexBuffer, bounds)
			self.setGeometryForRenderItem(edgeSelectionItem, wireBuffers, self.fWireIndexBuffer, bounds)
			self.setGeometryForRenderItem(faceSelectionItem, wireBuffers, self.fShadedIndexBuffer, bounds)

		## Update active component items if required
		if itemsChanged or updateActiveItems:
			bounds = self.fMesh.boundingBox()

			vertexBuffer = omr.MVertexBufferArray()
			vertexBuffer.append(self.fPositionBuffer, "positions")

			if activeVertexItem:
				self.setGeometryForRenderItem(activeVertexItem, vertexBuffer, self.fActiveVerticesIndexBuffer, bounds)
			if activeEdgeItem:
				self.setGeometryForRenderItem(activeEdgeItem, vertexBuffer, self.fActiveEdgesIndexBuffer, bounds)
			if activeFaceItem:
				self.setGeometryForRenderItem(activeFaceItem, vertexBuffer, self.fActiveFacesIndexBuffer, bounds)

		## Update render item matrices if necessary
		if itemsChanged or anyMatrixChanged:
			if not self.fIsInstanceMode and numInstances == 1:
				## When not dealing with multiple instances, don't convert the render items into instanced
				## mode.  Set the matrices on them directly.
				objToWorld = instanceMatrixArray[0]

				if wireItem:
					wireItem.setMatrix(objToWorld)
				if selectItem:
					selectItem.setMatrix(objToWorld)
				if boxItem:
					boxItem.setMatrix(objToWorld)
				if selectedBoxItem:
					selectedBoxItem.setMatrix(objToWorld)
				shadedItem.setMatrix(objToWorld)

				vertexSelectionItem.setMatrix(objToWorld)
				edgeSelectionItem.setMatrix(objToWorld)
				faceSelectionItem.setMatrix(objToWorld)

				if useDrawInstancingOnEdgeSelectionItem:
					## create/update draw instances
					## first instance : no scaling - won't be selectable see simpleComponentConverter_subsceneOverride.addIntersection
					transform1 = objToWorld
					transform1[15] = 1  # make sure we don't scale the w
					## second instance : scaled by 2
					transform2 = objToWorld * 2
					transform2[15] = 1  # make sure we don't scale the w
					## third instance : scaled by 3 - won't be selectable see simpleComponentConverter_subsceneOverride.addIntersection
					transform3 = objToWorld * 3
					transform3[15] = 1  # make sure we don't scale the w

					if True:
						transforms = om.MMatrixArray((transform1, transform2, transform3))
						self.setInstanceTransformArray(edgeSelectionItem, transforms)
					else:
						## another way to set up the instances
						## useful to get the instance ID
						try:
							self.removeAllInstances(edgeSelectionItem)
						except:
							# removeAllInstances raise an error if the item is not set up for instancing
							pass
						newInstanceId = self.addInstanceTransform(edgeSelectionItem, transform1)
						print "newInstanceId " + str(newInstanceId)
						newInstanceId = self.addInstanceTransform(edgeSelectionItem, transform2)
						print "newInstanceId " + str(newInstanceId)
						newInstanceId = self.addInstanceTransform(edgeSelectionItem, transform3)
						print "newInstanceId " + str(newInstanceId)

				if activeVertexItem:
					activeVertexItem.setMatrix(objToWorld)
				if activeEdgeItem:
					activeEdgeItem.setMatrix(objToWorld)
				if activeFaceItem:
					activeFaceItem.setMatrix(objToWorld)

			else:
				## If we have DAG instances of this shape then use the MPxSubSceneOverride instance
				## transform API to set up instance copies of the render items.  This will be faster
				## to render than creating render items for each instance, especially for large numbers
				## of instances.
				## Note: for simplicity, this code is not tracking the actual shaded material binding
				## of the shape.  MPxSubSceneOverride render item instances require that the shader
				## and other properties of the instances all match.  So if we were to bind a shader based
				## on the DAG shape material binding, then we would need one render item per material. We
				## could then group up the instance transforms based matching materials.

				## Note this has to happen after the geometry and shaders are set, otherwise it will fail.
				if wireItem:
					self.setInstanceTransformArray(wireItem, unselectedInstanceMatrixArray)
				if selectItem:
					self.setInstanceTransformArray(selectItem, selectedInstanceMatrixArray)
				if boxItem:
					self.setInstanceTransformArray(boxItem, unselectedInstanceMatrixArray)
				if selectedBoxItem:
					self.setInstanceTransformArray(selectedBoxItem, selectedInstanceMatrixArray)
				self.setInstanceTransformArray(shadedItem, instanceMatrixArray)

				self.setInstanceTransformArray(vertexSelectionItem, instanceMatrixArray)
				self.setInstanceTransformArray(edgeSelectionItem, instanceMatrixArray)
				self.setInstanceTransformArray(faceSelectionItem, instanceMatrixArray)

				if activeVertexItem:
					self.setInstanceTransformArray(activeVertexItem, instanceMatrixArray)
				if activeEdgeItem:
					self.setInstanceTransformArray(activeEdgeItem, instanceMatrixArray)
				if activeFaceItem:
					self.setInstanceTransformArray(activeFaceItem, instanceMatrixArray)

				## Once we change the render items into instance rendering they can't be changed back without
				## being deleted and re-created.  So if instances are deleted to leave only one remaining,
				## just keep treating them the instance way.
				self.fIsInstanceMode = True

		if itemsChanged or anyMatrixChanged or updateGeometry:
			## On transform or geometry change, force recalculation of shadow maps
			omr.MRenderer.setLightsAndShadowsDirty()

	def rebuildGeometryBuffers(self):
		## Preamble
		meshGeom = self.fMesh.meshGeom()
		if not meshGeom:
			return
		bounds = self.fMesh.boundingBox()

		## Clear old
		self.clearGeometryBuffers()

		## Compute mesh data size
		numTriangles = 0
		totalVerts = 0
		totalPoints = len(meshGeom.vertices)
		for i in xrange(meshGeom.faceCount):
			numVerts = meshGeom.face_counts[i]
			if numVerts > 2:
				numTriangles += numVerts - 2
				totalVerts += numVerts

		## Acquire vertex buffer resources
		posDesc = omr.MVertexBufferDescriptor("", omr.MGeometry.kPosition, omr.MGeometry.kFloat, 3)
		normalDesc = omr.MVertexBufferDescriptor("", omr.MGeometry.kNormal, omr.MGeometry.kFloat, 3)

		self.fPositionBuffer = omr.MVertexBuffer(posDesc)
		self.fNormalBuffer = omr.MVertexBuffer(normalDesc)
		self.fBoxPositionBuffer = omr.MVertexBuffer(posDesc)

		positionDataAddress = self.fPositionBuffer.acquire(totalPoints, True)
		normalDataAddress = self.fNormalBuffer.acquire(totalPoints, True)
		boxPositionDataAddress = self.fBoxPositionBuffer.acquire(8, True)

		## Acquire index buffer resources
		self.fWireIndexBuffer = omr.MIndexBuffer(omr.MGeometry.kUnsignedInt32)
		self.fVertexIndexBuffer = omr.MIndexBuffer(omr.MGeometry.kUnsignedInt32)
		self.fBoxIndexBuffer = omr.MIndexBuffer(omr.MGeometry.kUnsignedInt32)
		self.fShadedIndexBuffer = omr.MIndexBuffer(omr.MGeometry.kUnsignedInt32)

		wireBufferDataAddress = self.fWireIndexBuffer.acquire(2*totalVerts, True)
		vertexBufferDataAddress = self.fVertexIndexBuffer.acquire(totalPoints, True)
		boxBufferDataAddress = self.fBoxIndexBuffer.acquire(24, True)
		shadedBufferDataAddress = self.fShadedIndexBuffer.acquire(3*numTriangles, True)

		## Sanity check
		if not all((positionDataAddress, normalDataAddress, boxPositionDataAddress, wireBufferDataAddress, vertexBufferDataAddress, boxBufferDataAddress, shadedBufferDataAddress)):
			print "At least one buffer data is not valid"
			self.clearGeometryBuffers()
			return

		positionData = ((ctypes.c_float * 3)*totalPoints).from_address(positionDataAddress)
		normalData = ((ctypes.c_float * 3)*totalPoints).from_address(normalDataAddress)
		boxPositionData = ((ctypes.c_float * 3)*8).from_address(boxPositionDataAddress)

		wireBufferData = (ctypes.c_uint * (2*totalVerts)).from_address(wireBufferDataAddress)
		vertexBufferData = (ctypes.c_uint * totalPoints).from_address(vertexBufferDataAddress)
		boxBufferData = (ctypes.c_uint * 24).from_address(boxBufferDataAddress)
		shadedBufferData = ((ctypes.c_uint * 3)*numTriangles).from_address(shadedBufferDataAddress)

		## Fill vertex data for shaded/wireframe
		for vid,position in enumerate(meshGeom.vertices):
			positionData[vid][0] = position[0]
			positionData[vid][1] = position[1]
			positionData[vid][2] = position[2]

		for vid,normal in enumerate(meshGeom.normals):
			normalData[vid][0] = normal[0]
			normalData[vid][1] = normal[1]
			normalData[vid][2] = normal[2]

		self.fPositionBuffer.commit(positionDataAddress)
		positionDataAddress = None
		self.fNormalBuffer.commit(normalDataAddress)
		normalDataAddress = None

		## Fill vertex data for bounding box
		bbmin = bounds.min
		bbmax = bounds.max
		boxPositionData[0][0] = bbmin.x
		boxPositionData[0][1] = bbmin.y
		boxPositionData[0][2] = bbmin.z

		boxPositionData[1][0] = bbmin.x
		boxPositionData[1][1] = bbmin.y
		boxPositionData[1][2] = bbmax.z

		boxPositionData[2][0] = bbmax.x
		boxPositionData[2][1] = bbmin.y
		boxPositionData[2][2] = bbmax.z

		boxPositionData[3][0] = bbmax.x
		boxPositionData[3][1] = bbmin.y
		boxPositionData[3][2] = bbmin.z

		boxPositionData[4][0] = bbmin.x
		boxPositionData[4][1] = bbmax.y
		boxPositionData[4][2] = bbmin.z

		boxPositionData[5][0] = bbmin.x
		boxPositionData[5][1] = bbmax.y
		boxPositionData[5][2] = bbmax.z

		boxPositionData[6][0] = bbmax.x
		boxPositionData[6][1] = bbmax.y
		boxPositionData[6][2] = bbmax.z

		boxPositionData[7][0] = bbmax.x
		boxPositionData[7][1] = bbmax.y
		boxPositionData[7][2] = bbmin.z

		self.fBoxPositionBuffer.commit(boxPositionDataAddress)
		boxPositionDataAddress = None

		## Fill index data for wireframe
		vid = 0
		first = 0
		idx = 0
		for i in xrange(meshGeom.faceCount):
			## Ignore degenerate faces
			numVerts = meshGeom.face_counts[i]
			if numVerts > 2:
				first = vid
				for v in xrange(numVerts-1):
					wireBufferData[idx] = meshGeom.face_connects[vid]
					idx += 1
					vid += 1
					wireBufferData[idx] = meshGeom.face_connects[vid]
					idx += 1

				wireBufferData[idx] = meshGeom.face_connects[vid]
				idx += 1
				vid += 1
				wireBufferData[idx] = meshGeom.face_connects[first]
				idx += 1

			else:
				vid += numVerts

		self.fWireIndexBuffer.commit(wireBufferDataAddress)
		wireBufferDataAddress = None

		## Fill index data for vertices
		for i in xrange(totalPoints):
			vertexBufferData[i] = i

		self.fVertexIndexBuffer.commit(vertexBufferDataAddress)
		vertexBufferDataAddress = None

		## Fill index data for bounding box
		indexData = [ 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7 ]
		for i in xrange(24):
			boxBufferData[i] = indexData[i]

		self.fBoxIndexBuffer.commit(boxBufferDataAddress)
		boxBufferDataAddress = None

		## Fill index data for shaded
		base = 0
		idx = 0
		for i in xrange(meshGeom.faceCount):
			## Ignore degenerate faces
			numVerts = meshGeom.face_counts[i]
			if numVerts > 2:
				for v in xrange(1, numVerts-1):
					shadedBufferData[idx][0] = meshGeom.face_connects[base]
					shadedBufferData[idx][1] = meshGeom.face_connects[base+v]
					shadedBufferData[idx][2] = meshGeom.face_connects[base+v+1]
					idx += 1

				base += numVerts
		
		self.fShadedIndexBuffer.commit(shadedBufferDataAddress)
		shadedBufferDataAddress = None

	def rebuildActiveComponentIndexBuffers(self):
		## Preamble
		meshGeom = self.fMesh.meshGeom()
		if not meshGeom:
			return

		## Clear old
		self.clearActiveComponentIndexBuffers()

		## Acquire and fill index buffer for active vertices
		numActiveVertices = len(self.fActiveVerticesSet)
		if numActiveVertices > 0:
			self.fActiveVerticesIndexBuffer = omr.MIndexBuffer(omr.MGeometry.kUnsignedInt32)
			activeVerticesDataAddress = self.fActiveVerticesIndexBuffer.acquire(numActiveVertices, True)
			if activeVerticesDataAddress:
				activeVerticesData = (ctypes.c_uint*numActiveVertices).from_address(activeVerticesDataAddress)

				idx = 0
				for vid in self.fActiveVerticesSet:
					activeVerticesData[idx] = vid 
					idx += 1

				self.fActiveVerticesIndexBuffer.commit(activeVerticesDataAddress)
				activeVerticesDataAddress = None

		## Acquire and fill index buffer for active edges
		numActiveEdges = len(self.fActiveEdgesSet)
		if numActiveEdges > 0:
			self.fActiveEdgesIndexBuffer = omr.MIndexBuffer(omr.MGeometry.kUnsignedInt32)
			activeEdgesDataAddress = self.fActiveEdgesIndexBuffer.acquire(2*numActiveEdges, True)
			if activeEdgesDataAddress:
				activeEdgesData = ((ctypes.c_uint * 2)*numActiveEdges).from_address(activeEdgesDataAddress)

				eid = 0
				first = 0
				vid = 0
				idx = 0
				for i in xrange(meshGeom.faceCount):
					## Ignore degenerate faces
					numVerts = meshGeom.face_counts[i]
					if numVerts > 2:
						first = vid
						for v in xrange(numVerts-1):
							if eid in self.fActiveEdgesSet:
								activeEdgesData[idx][0] = meshGeom.face_connects[vid]
								activeEdgesData[idx][1] = meshGeom.face_connects[vid + 1]
								idx += 1
							vid += 1
							eid += 1

						if eid in self.fActiveEdgesSet:
							activeEdgesData[idx][0] = meshGeom.face_connects[vid]
							activeEdgesData[idx][1] = meshGeom.face_connects[first]
							idx += 1
						vid += 1
						eid += 1
					else:
						vid += numVerts

				self.fActiveEdgesIndexBuffer.commit(activeEdgesDataAddress)
				activeEdgesDataAddress = None

		## Acquire and fill index buffer for active faces
		numActiveFaces = len(self.fActiveFacesSet)
		if numActiveFaces > 0:
			numActiveFacesTriangles = 0
			for i in xrange(meshGeom.faceCount):
				if i in self.fActiveFacesSet:
					numVerts = meshGeom.face_counts[i]
					if numVerts > 2:
						numActiveFacesTriangles += numVerts - 2

			self.fActiveFacesIndexBuffer = omr.MIndexBuffer(omr.MGeometry.kUnsignedInt32)
			activeFacesDataAddress = self.fActiveFacesIndexBuffer.acquire(3*numActiveFacesTriangles, True)
			if activeFacesDataAddress:
				activeFacesData = ((ctypes.c_uint * 3)*numActiveFacesTriangles).from_address(activeFacesDataAddress)

				idx = 0
				vid = 0
				for i in xrange(meshGeom.faceCount):
					numVerts = meshGeom.face_counts[i]
					if numVerts > 2:
						if i in self.fActiveFacesSet:
							for v in xrange(1, numVerts-1):
								activeFacesData[idx][0] = meshGeom.face_connects[vid]
								activeFacesData[idx][1] = meshGeom.face_connects[vid+v]
								activeFacesData[idx][2] = meshGeom.face_connects[vid+v+1]
								idx += 1

						vid += numVerts

				self.fActiveFacesIndexBuffer.commit(activeFacesDataAddress)
				activeFacesDataAddress = None

	def clearBuffers(self):
		self.clearGeometryBuffers()
		self.clearActiveComponentIndexBuffers()

	def clearGeometryBuffers(self):
		self.fPositionBuffer = None
		self.fNormalBuffer = None
		self.fBoxPositionBuffer = None
		self.fWireIndexBuffer = None
		self.fBoxIndexBuffer = None
		self.fShadedIndexBuffer = None

	def clearActiveComponentIndexBuffers(self):
		self.fActiveVerticesIndexBuffer = None
		self.fActiveEdgesIndexBuffer = None
		self.fActiveFacesIndexBuffer = None

	def updateSelectionGranularity(self, path, selectionContext):
		## This is method is called during the pre-filtering phase of the viewport 2.0 selection
		## and is used to setup the selection context of the given DAG object.

		## We want the whole shape to be selectable, so we set the selection level to kObject so that the shape
		## will be processed by the selection.

		## In case we are currently in component selection mode (vertex, edge or face),
		## since we have created render items that can be use in the selection phase (kSelectionOnly draw mode)
		## and we also registered component converters to handle this render items,
		## we can set the selection level to kComponent so that the shape will also be processed by the selection.
		displayStatus = omr.MGeometryUtilities.displayStatus(path)
		if (displayStatus & omr.MGeometryUtilities.kHilite) != 0:
	
			globalComponentMask = om.MGlobal.objectSelectionMask()
			if om.MGlobal.selectionMode() == om.MGlobal.kSelectComponentMode:
				globalComponentMask = om.MGlobal.componentSelectionMask()

			supportedComponentMask = om.MSelectionMask( om.MSelectionMask.kSelectMeshVerts )
			supportedComponentMask.addMask( om.MSelectionMask.kSelectMeshEdges )
			supportedComponentMask.addMask( om.MSelectionMask.kSelectMeshFaces )
			supportedComponentMask.addMask( om.MSelectionMask.kSelectPointsForGravity )
			
			if globalComponentMask.intersects(supportedComponentMask):
				selectionContext.selectionLevel = omr.MSelectionContext.kComponent
		elif omr.MPxSubSceneOverride.pointSnappingActive():
			selectionContext.selectionLevel = omr.MSelectionContext.kComponent


################################################################################
##
## apiMeshGeometryOverride
##
## Handles vertex data preparation for drawing the user defined shape in
## Viewport 2.0.
##
################################################################################
## Custom user data class to attach to render items
class apiMeshUserData(om.MUserData):
	def __init__(self):
		om.MUserData.__init__(self, True)	## let Maya clean up
		self.fMessage = ""
		self.fNumModifications = 0

## Pre/Post callback helper
def callbackDataPrint(context, renderItemList):
	for item in renderItemList:
		if item:
			path = item.sourceDagPath()
			print "\tITEM: '" + item.name() + "' -- SOURCE: '" + path.fullPathName() + "'"

	passCtx = context.getPassContext()
	passId = passCtx.passIdentifier()
	passSem = passCtx.passSemantics()
	print "\tAPI mesh drawing in pass[" + passId + "], semantic[" + passSem + "]"

## Custom pre-draw callback
def apiMeshPreDrawCallback(context, renderItemList, shaderInstance):
	print "PRE-draw callback triggered for render item list with data:"
	callbackDataPrint(context, renderItemList)

## Custom post-draw callback
def apiMeshPostDrawCallback(context, renderItemList, shaderInstance):
	print "POST-draw callback triggered for render item list with data:"
	callbackDataPrint(context, renderItemList)

## Custom component converter to select vertices
## Attached to the dormant vertices render item (apiMeshGeometryOverride.sVertexItemName)
class meshVertComponentConverter_geometryOverride(omr.MPxComponentConverter):
	def __init__(self):
		omr.MPxComponentConverter.__init__(self)

		self.fComponent = om.MFnSingleIndexedComponent()
		self.fComponentObject = om.MObject.kNullObj
		self.fVertices = []

	def initialize(self, renderItem):
		## Create the component selection object .. here we are selecting vertex component
		self.fComponentObject = self.fComponent.create( om.MFn.kMeshVertComponent )

		## Build a lookup table to match each primitive position in the index buffer of the render item geometry
		## to the correponding vertex component of the object
		## Use same algorithm as in apiMeshGeometryOverride.updateIndexingForDormantVertices

		selectionData = renderItem.customData()
		if isinstance(selectionData, apiMeshHWSelectionUserData):
			meshGeom = selectionData.fMeshGeom

			## Allocate vertices lookup table
			numTriangles = 0
			for i in xrange(meshGeom.faceCount):
				numVerts = meshGeom.face_counts[i]
				if numVerts > 2:
					numTriangles += numVerts - 2
			self.fVertices = [0]*(3*numTriangles)

			## Fill vertices lookup table
			base = 0
			idx = 0
			for faceIdx in xrange(meshGeom.faceCount):
				## ignore degenerate faces
				numVerts = meshGeom.face_counts[faceIdx]
				if numVerts > 2:
					for v in xrange(1, numVerts-1):
						self.fVertices[idx] = meshGeom.face_connects[base]
						self.fVertices[idx+1] = meshGeom.face_connects[base+v]
						self.fVertices[idx+2] = meshGeom.face_connects[base+v+1]
						idx += 3
					base += numVerts

	def addIntersection(self, intersection):
		## Convert the intersection index, which represent the primitive position in the
		## index buffer, to the correct vertex component
		rawIdx = intersection.index
		idx = 0
		if rawIdx >= 0 and rawIdx < len(self.fVertices):
			idx = self.fVertices[rawIdx]
		self.fComponent.addElement(idx)

	def component(self):
		## Return the component object that contains the ids of the selected vertices
		return self.fComponentObject

	def selectionMask(self):
		## This converter is only valid for vertex selection
		mask = om.MSelectionMask(om.MSelectionMask.kSelectMeshVerts)
		mask.addMask(om.MSelectionMask.kSelectPointsForGravity)
		return mask

	@staticmethod
	def creator():
		return meshVertComponentConverter_geometryOverride()

## Custom component converter to select edges
## Attached to the edge selection render item (apiMeshGeometryOverride.sEdgeSelectionItemName)
class meshEdgeComponentConverter_geometryOverride(omr.MPxComponentConverter):
	def __init__(self):
		omr.MPxComponentConverter.__init__(self)

		self.fComponent = om.MFnSingleIndexedComponent()
		self.fComponentObject = om.MObject.kNullObj
		self.fEdges = []

	def initialize(self, renderItem):
		## Create the component selection object .. here we are selecting edge component
		self.fComponentObject = self.fComponent.create( om.MFn.kMeshEdgeComponent )

		## Build a lookup table to match each primitive position in the index buffer of the render item geometry
		## to the correponding edge component of the object
		## Use same algorithm as in apiMeshGeometryOverride.updateIndexingForEdges

		## in updateIndexingForEdges the index buffer is allocated with "totalEdges = 2*totalVerts"
		## but since we are drawing lines, we'll get only half of the data as primitive positions :
		## indices 0 & 1 : primitive #0
		## indices 2 & 3 : primitive #1
		## ...

		selectionData = renderItem.customData()
		if isinstance(selectionData, apiMeshHWSelectionUserData):
			meshGeom = selectionData.fMeshGeom

			## Allocate edges lookup table
			totalVerts = 0
			for i in xrange(meshGeom.faceCount):
				numVerts = meshGeom.face_counts[i]
				if numVerts > 2:
					totalVerts += numVerts
			self.fEdges = [0]*(totalVerts)

			## Fill edges lookup table
			idx = 0
			edgeId = 0
			for faceIdx in xrange(meshGeom.faceCount):
				## ignore degenerate faces
				numVerts = meshGeom.face_counts[faceIdx]
				if numVerts > 2:
					for v in xrange(numVerts):
						self.fEdges[idx] = edgeId
						edgeId += 1
						idx += 1

	def addIntersection(self, intersection):
		## Convert the intersection index, which represent the primitive position in the
		## index buffer, to the correct edge component
		rawIdx = intersection.index
		idx = 0
		if rawIdx >= 0 and rawIdx < len(self.fEdges):
			idx = self.fEdges[rawIdx]
		self.fComponent.addElement(idx)

	def component(self):
		## Return the component object that contains the ids of the selected edges
		return self.fComponentObject

	def selectionMask(self):
		## This converter is only valid for edge selection
		return om.MSelectionMask(om.MSelectionMask.kSelectMeshEdges)

	@staticmethod
	def creator():
		return meshEdgeComponentConverter_geometryOverride()

## Custom component converter to select faces
## Attached to the face selection render item (apiMeshGeometryOverride.sFaceSelectionItemName)
class meshFaceComponentConverter_geometryOverride(omr.MPxComponentConverter):
	def __init__(self):
		omr.MPxComponentConverter.__init__(self)

		self.fComponent = om.MFnSingleIndexedComponent()
		self.fComponentObject = om.MObject.kNullObj
		self.fFaces = []

	def initialize(self, renderItem):
		## Create the component selection object .. here we are selecting face component
		self.fComponentObject = self.fComponent.create( om.MFn.kMeshPolygonComponent )

		## Build a lookup table to match each primitive position in the index buffer of the render item geometry
		## to the correponding face component of the object
		## Use same algorithm as in apiMeshGeometryOverride.updateIndexingForFaces

		## in updateIndexingForFaces the index buffer is allocated with "numTriangleVertices = 3*numTriangles"
		## but since we are drawing triangles, we'll get only a third of the data as primitive positions :
		## indices 0, 1 & 2 : primitive #0
		## indices 3, 4 & 5 : primitive #1
		## ...

		selectionData = renderItem.customData()
		if isinstance(selectionData, apiMeshHWSelectionUserData):
			meshGeom = selectionData.fMeshGeom

			## Allocate faces lookup table
			numTriangles = 0
			for i in xrange(meshGeom.faceCount):
				numVerts = meshGeom.face_counts[i]
				if numVerts > 2:
					numTriangles += numVerts - 2
			self.fFaces = [0]*numTriangles

			## Fill faces lookup table
			idx = 0
			for faceIdx in xrange(meshGeom.faceCount):
				## ignore degenerate faces
				numVerts = meshGeom.face_counts[faceIdx]
				if numVerts > 2:
					for v in xrange(1, numVerts-1):
						self.fFaces[idx] = faceIdx
						idx += 1

	def addIntersection(self, intersection):
		## Convert the intersection index, which represent the primitive position in the
		## index buffer, to the correct face component
		rawIdx = intersection.index
		idx = 0
		if rawIdx >= 0 and rawIdx < len(self.fFaces):
			idx = self.fFaces[rawIdx]
		self.fComponent.addElement(idx)

	def component(self):
		## Return the component object that contains the ids of the selected faces
		return self.fComponentObject

	def selectionMask(self):
		## This converter is only valid for face selection
		return om.MSelectionMask(om.MSelectionMask.kSelectMeshFaces)

	@staticmethod
	def creator():
		return meshFaceComponentConverter_geometryOverride()

class apiMeshGeometryOverride(omr.MPxGeometryOverride):
	## Render item names
	sWireframeItemName = "apiMeshWire"
	sShadedTemplateItemName = "apiMeshShadedTemplateWire"
	sSelectedWireframeItemName = "apiMeshSelectedWireFrame"
	sVertexItemName = "apiMeshVertices"
	sEdgeSelectionItemName = "apiMeshEdgeSelection"
	sFaceSelectionItemName = "apiMeshFaceSelection"
	sActiveVertexItemName = "apiMeshActiveVertices"
	sVertexIdItemName = "apiMeshVertexIds"
	sVertexPositionItemName = "apiMeshVertexPositions"
	sShadedModeFaceCenterItemName = "apiMeshFaceCenterInShadedMode"
	sWireframeModeFaceCenterItemName = "apiMeshFaceCenterInWireframeMode"
	sShadedProxyItemName = "apiShadedProxy"
	sAffectedEdgeItemName = "apiMeshAffectedEdges"
	sAffectedFaceItemName = "apiMeshAffectedFaces"
	sActiveVertexStreamName = "apiMeshSharedVertexStream"
	sFaceCenterStreamName = "apiMeshFaceCenterStream"

	@staticmethod
	def creator(obj):
		return apiMeshGeometryOverride(obj)

	def __init__(self, obj):
		omr.MPxGeometryOverride.__init__(self, obj)

		node = om.MFnDependencyNode(obj)
		self.fMesh = node.userNode()
		self.fMeshGeom = None
		self.fColorRemapTexture = None

		self.fActiveVertices = om.MIntArray()
		self.fActiveVerticesSet = set()
		self.fActiveEdgesSet = set()
		self.fActiveFacesSet = set()
		self.fCastsShadows = False
		self.fReceivesShadows = False

		## Stream names used for filling in different data
		## for different streams required for different render items,
		## and toggle to choose whether to use name streams
		##
		self.fDrawSharedActiveVertices = True
		## Turn on to view active vertices with colours lookedup from a 1D texture.
		self.fDrawActiveVerticesWithRamp = False
		self.fLinearSampler = None

		##Vertex stream for face centers which is calculated from faces.
		self.fDrawFaceCenters = True

		if self.fDrawActiveVerticesWithRamp:
			self.fDrawFaceCenters = False	## Too cluttered showing the face centers at the same time.

		## Can set the following to True, but then the logic to
		## determine what color to set is the responsibility of the plugin.
		##
		self.fUseCustomColors = False

		## Can change this to choose a different shader to use when
		## no shader node is assigned to the object.
		## self.fProxyShader = omr.MShaderManager.k3dSolidShader ## - Basic line shader
		## self.fProxyShader = omr.MShaderManager.k3dStippleShader ## - For filled stipple faces (triangles)
		## self.fProxyShader = omr.MShaderManager.k3dThickLineShader ## For thick solid colored lines
		## self.fProxyShader = omr.MShaderManager.k3dCPVThickLineShader ## For thick colored lines. Black if no CPV
		## self.fProxyShader = omr.MShaderManager.k3dDashLineShader ## - For dashed solid color lines
		## self.fProxyShader = omr.MShaderManager.k3dCPVDashLineShader ##- For dashed coloured lines. Black if no CPV
		## self.fProxyShader = omr.MShaderManager.k3dThickDashLineShader ## For thick dashed solid color lines. black if no cpv
		self.fProxyShader = omr.MShaderManager.k3dCPVThickDashLineShader ##- For thick, dashed and coloured lines

		## Set to True to test that overriding internal items has no effect
		## for shadows and effects overrides
		self.fInternalItems_NoShadowCast = False
		self.fInternalItems_NoShadowReceive = False
		self.fInternalItems_NoPostEffects = False

		## Use the existing shadow casts / receives flags on the shape
		## to drive settings for applicable render items.
		self.fExternalItems_NoShadowCast = False
		self.fExternalItems_NoShadowReceive = False
		self.fExternalItemsNonTri_NoShadowCast = False
		self.fExternalItemsNonTri_NoShadowReceive = False

		## Set to True to ignore post-effects for wireframe items.
		## Shaded items will still have effects applied.
		self.fExternalItems_NoPostEffects = True
		self.fExternalItemsNonTri_NoPostEffects = True

	def __del__(self):
		self.fMesh = None
		self.fMeshGeom = None

		if self.fColorRemapTexture:
			textureMgr = omr.MRenderer.getTextureManager()
			if textureMgr:
				textureMgr.releaseTexture(self.fColorRemapTexture)
			self.fColorRemapTexture = None

		if self.fLinearSampler:
			omr.MStateManager.releaseSamplerState(self.fLinearSampler)
			self.fLinearSampler = None

	def supportedDrawAPIs(self):
		## this plugin supports both GL and DX
		return omr.MRenderer.kOpenGL | omr.MRenderer.kDirectX11 | omr.MRenderer.kOpenGLCoreProfile

	def updateDG(self):
		## Pull the actual outMesh from the shape, as well
		## as any active components
		self.fActiveVertices.clear()
		self.fActiveVerticesSet = set()
		self.fActiveEdgesSet = set()
		self.fActiveFacesSet = set()
		if self.fMesh:
			self.fMeshGeom = self.fMesh.meshGeom()

			if self.fMeshGeom and self.fMesh.hasActiveComponents():
				activeComponents = self.fMesh.activeComponents()
				if len(activeComponents) > 0:
					fnComponent = om.MFnSingleIndexedComponent( activeComponents[0] )
					if fnComponent.elementCount > 0:
						activeIds = fnComponent.getElements()

						if fnComponent.componentType == om.MFn.kMeshVertComponent:
							self.fActiveVertices = activeIds
							self.fActiveVerticesSet = set(activeIds)

						elif fnComponent.componentType == om.MFn.kMeshEdgeComponent:
							self.fActiveEdgesSet = set(activeIds)

						elif fnComponent.componentType == om.MFn.kMeshPolygonComponent:
							self.fActiveFacesSet = set(activeIds)

	def updateRenderItems(self, path, list):
		## Update render items. Shaded render item is provided so this
		## method will be adding and updating UI render items only.

		shaderMgr = omr.MRenderer.getShaderManager()
		if not shaderMgr:
			return

		dagNode = om.MFnDagNode(path)
		castsShadowsPlug = dagNode.findPlug("castsShadows", False)
		self.fCastsShadows = castsShadowsPlug.asBool()
		receiveShadowsPlug = dagNode.findPlug("receiveShadows", False)
		self.fReceivesShadows = receiveShadowsPlug.asBool()

		##1 Update wireframe render items
		self.updateDormantAndTemplateWireframeItems(path, list, shaderMgr)
		self.updateActiveWireframeItem(path, list, shaderMgr)

		## Update vertex render items
		self.updateDormantVerticesItem(path, list, shaderMgr)
		self.updateActiveVerticesItem(path, list, shaderMgr)

		## Update vertex numeric render items
		self.updateVertexNumericItems(path, list, shaderMgr)

		## Update face center item
		if self.fDrawFaceCenters:
			self.updateWireframeModeFaceCenterItem(path, list, shaderMgr)
			self.updateShadedModeFaceCenterItem(path, list, shaderMgr)

		## Update "affected" edge and face render items
		self.updateAffectedComponentItems(path, list, shaderMgr)

		## Update faces and edges selection items
		self.updateSelectionComponentItems(path, list, shaderMgr)

		## Update proxy shaded render item
		self.updateProxyShadedItem(path, list, shaderMgr)

		## Test overrides on existing shaded items.
		## In this case it is not valid to override these states
		## so there should be no change in behaviour.
		##
		testShadedOverrides = self.fInternalItems_NoShadowCast or self.fInternalItems_NoShadowReceive or self.fInternalItems_NoPostEffects
		if testShadedOverrides:
			for item in list:
				if not item:
					continue

				drawMode = item.drawMode()
				if drawMode == omr.MGeometry.kShaded or drawMode == omr.MGeometry.kTextured:
					if item.name() != self.sShadedTemplateItemName:
						item.setCastsShadows( not self.fInternalItems_NoShadowCast and self.fCastsShadows )
						item.setReceivesShadows( not self.fInternalItems_NoShadowReceive and self.fReceivesShadows )
						item.setExcludedFromPostEffects( self.fInternalItems_NoPostEffects )

	def populateGeometry(self, requirements, renderItems, data):
		## Fill in data and index streams based on the requirements passed in.
		## Associate indexing with the render items passed in.
		## 
		## Note that we leave both code paths to either draw shared or non-shared active vertices.
		## The choice of which to use is up to the circumstances per plug-in.
		## When drawing shared vertices, this requires an additional position buffer to be
		## created so will use more memory. If drawing unshared vertices redundent extra
		## vertices are drawn but will use less memory. The data member fDrawSharedActiveVertices
		## can be set to decide on which implementation to use.
	
		debugPopulateGeometry = False
		if debugPopulateGeometry:
			print "> Begin populate geometry"

		## Get the active vertex count
		activeVertexCount = len(self.fActiveVertices)

		## Compute the number of triangles, assume polys are always convex
		numTriangles = 0
		totalVerts = 0
		for i in xrange(self.fMeshGeom.faceCount):
			numVerts = self.fMeshGeom.face_counts[i]
			if numVerts > 2:
				numTriangles += numVerts - 2
				totalVerts += numVerts

		## Update data streams based on geometry requirements
		self.updateGeometryRequirements(requirements, data, activeVertexCount, totalVerts, debugPopulateGeometry)

		## Update indexing data for all appropriate render items
		wireIndexBuffer = None ## reuse same index buffer for both wireframe and selected

		for item in renderItems:
			if not item:
				continue

			## Enable to debug vertex buffers that are associated with each render item.
			## Can also use to generate indexing better, but we don't need that here.
			## Also debugs custom data on the render item.
			debugStuff = False
			if debugStuff:
				itemBuffers = item.requiredVertexBuffers()
				for desc in itemBuffers:
					print "Buffer Required for Item '" + item.name() + "':"
					print "\tBufferName: " + desc.name
					print "\tDataType: " + omr.MGeometry.dataTypeString(desc.dataType) + " (dimension " + str(desc.dimension) + ")"
					print "\tSemantic: " + omr.MGeometry.semanticString(desc.semantic)
					print ""

				## Just print a message for illustration purposes. Note that the custom data is also
				## accessible from the MRenderItem in MPxShaderOverride::draw().
				myCustomData = item.customData()
				if isinstance(myCustomData, apiMeshUserData):
					print "Custom data '" + myCustomData.fMessage + "', modified count='" + str(myCustomData.fNumModifications) + "'"
				else:
					print "No custom data"

			## Update indexing for active vertex item
			##
			if item.name() == self.sActiveVertexItemName:
				self.updateIndexingForVertices( item, data, numTriangles, activeVertexCount, debugPopulateGeometry)

			## Update indexing for face center item in wireframe mode and shaded mode
			##
			if self.fDrawFaceCenters and (item.name() == self.sShadedModeFaceCenterItemName or item.name() == self.sWireframeModeFaceCenterItemName):
				self.updateIndexingForFaceCenters( item, data, debugPopulateGeometry)

			## Create indexing for dormant and numeric vertex render items
			##
			elif item.name() == self.sVertexItemName or item.name() == self.sVertexIdItemName or item.name() == self.sVertexPositionItemName:
				self.updateIndexingForDormantVertices( item, data, numTriangles )

			## Create indexing for wireframe render items
			##
			elif item.name() == self.sWireframeItemName or item.name() == self.sShadedTemplateItemName or item.name() == self.sSelectedWireframeItemName or (item.primitive() != omr.MGeometry.kTriangles and item.name() == self.sShadedProxyItemName):
				self.updateIndexingForWireframeItems(wireIndexBuffer, item, data, totalVerts)

			## Handle indexing for affected edge render items
			## For each face we check the edges. If the edges are in the active vertex
			## list we add indexing for the 2 vertices on the edge to the index buffer.
			##
			elif item.name() == self.sAffectedEdgeItemName:
				self.updateIndexingForEdges(item, data, totalVerts, True) ## Filter edges using active edges or actives vertices set
			elif item.name() == self.sEdgeSelectionItemName:
				self.updateIndexingForEdges(item, data, totalVerts, False) ## No filter : all edges

			## Handle indexing for affected edge render items
			## For each triangle we check the vertices. If any of the vertices are in the active vertex
			## list we add indexing for the triangle to the index buffer.
			##
			elif item.name() == self.sAffectedFaceItemName:
				self.updateIndexingForFaces(item, data, numTriangles, True) ## Filter faces using active faces or actives vertices set
			elif item.name() == self.sFaceSelectionItemName:
				self.updateIndexingForFaces(item, data, numTriangles, False) ## No filter : all faces

			## Create indexing for filled (shaded) render items
			##
			elif item.primitive() == omr.MGeometry.kTriangles:
				self.updateIndexingForShadedTriangles(item, data, numTriangles)

		if debugPopulateGeometry:
			print "> End populate geometry"

	def cleanUp(self):
		self.fMeshGeom = None
		self.fActiveVertices.clear()
		self.fActiveVerticesSet = set()
		self.fActiveEdgesSet = set()
		self.fActiveFacesSet = set()

	def updateSelectionGranularity(self, path, selectionContext):
		## This is method is called during the pre-filtering phase of the viewport 2.0 selection
		## and is used to setup the selection context of the given DAG object.

		## We want the whole shape to be selectable, so we set the selection level to kObject so that the shape
		## will be processed by the selection.

		## In case we are currently in component selection mode (vertex, edge or face),
		## since we have created render items that can be use in the selection phase (kSelectionOnly draw mode)
		## and we also registered component converters to handle this render items,
		## we can set the selection level to kComponent so that the shape will also be processed by the selection.

		displayStatus = omr.MGeometryUtilities.displayStatus(path)
		if (displayStatus & omr.MGeometryUtilities.kHilite) != 0:
	
			globalComponentMask = om.MGlobal.objectSelectionMask()
			if om.MGlobal.selectionMode() == om.MGlobal.kSelectComponentMode:
				globalComponentMask = om.MGlobal.componentSelectionMask()

			supportedComponentMask = om.MSelectionMask( om.MSelectionMask.kSelectMeshVerts )
			supportedComponentMask.addMask( om.MSelectionMask.kSelectMeshEdges )
			supportedComponentMask.addMask( om.MSelectionMask.kSelectMeshFaces )
			supportedComponentMask.addMask( om.MSelectionMask.kSelectPointsForGravity )
			
			if globalComponentMask.intersects(supportedComponentMask):
				selectionContext.selectionLevel = omr.MSelectionContext.kComponent
		elif omr.MPxGeometryOverride.pointSnappingActive():
			selectionContext.selectionLevel = omr.MSelectionContext.kComponent

	def printShader(self, shader):
		## Some example code to print out shader parameters
		if not shader:
			return

		params = shader.parameterList()
		print "DEBUGGING SHADER, BEGIN PARAM LIST OF LENGTH " + str(len(params))

		for param in params:
			paramType = shader.parameterType(param)
			isArray = shader.isArrayParameter(param)

			typeAsStr = "Unknown"
			if paramType == omr.MShaderInstance.kInvalid:
				typeAsStr = "Invalid"
			elif paramType == omr.MShaderInstance.kBoolean:
				typeAsStr = "Boolean"
			elif paramType == omr.MShaderInstance.kInteger:
				typeAsStr = "Integer"
			elif paramType == omr.MShaderInstance.kFloat:
				typeAsStr = "Float"
			elif paramType == omr.MShaderInstance.kFloat2:
				typeAsStr = "Float2"
			elif paramType == omr.MShaderInstance.kFloat3:
				typeAsStr = "Float3"
			elif paramType == omr.MShaderInstance.kFloat4:
				typeAsStr = "Float4"
			elif paramType == omr.MShaderInstance.kFloat4x4Row:
				typeAsStr = "Float4x4Row"
			elif paramType == omr.MShaderInstance.kFloat4x4Col:
				typeAsStr = "Float4x4Col"
			elif paramType == omr.MShaderInstance.kTexture1:
				typeAsStr = "1D Texture"
			elif paramType == omr.MShaderInstance.kTexture2:
				typeAsStr = "2D Texture"
			elif paramType == omr.MShaderInstance.kTexture3:
				typeAsStr = "3D Texture"
			elif paramType == omr.MShaderInstance.kTextureCube:
				typeAsStr = "Cube Texture"
			elif paramType == omr.MShaderInstance.kSampler:
				typeAsStr = "Sampler"

			print "ParamName='" + param + "', ParamType='" + typeAsStr + "', IsArrayParameter:'" + str(isArray) + "'"

		print "END PARAM LIST"

	def setSolidColor(self, shaderInstance, defaultColor, customColor=None):
		## Set the solid color for solid color shaders
		if shaderInstance:
			color = defaultColor
			if self.fUseCustomColors and customColor:
				color = customColor
			try:
				shaderInstance.setParameter("solidColor", color)
			except:
				pass

	def setSolidPointSize(self, shaderInstance, size):
		## Set the point size for solid color shaders
		if shaderInstance:
			try:
				shaderInstance.setParameter("pointSize", [size, size])
			except:
				pass

	def setLineWidth(self, shaderInstance, width):
		## Set the line width for solid color shaders
		if shaderInstance:
			try:
				shaderInstance.setParameter("lineWidth", [width, width])
			except:
				pass

	def enableActiveComponentDisplay(self, path):
		## Test to see if active components should be enabled.
		## Based on active vertices + non-template state

		enable = True

		## If there are components then we need to check
		## either the display status of the object, or
		## in the case of a templated object make sure
		## to hide components to be consistent with how
		## internal objects behave
		##
		displayStatus = omr.MGeometryUtilities.displayStatus(path)
		if (displayStatus & (omr.MGeometryUtilities.kHilite | omr.MGeometryUtilities.kActiveComponent)) == 0:
			enable = False
		
		else:
			## Do an explicit path test for templated
			## since display status does not indicate this.
			if path.isTemplated():
				enable = False

		return enable

	## Render item handling methods
	def updateDormantAndTemplateWireframeItems(self, path, list, shaderMgr):
		## Update render items for dormant and template wireframe drawing.
		## 
		## 1) If the object is dormant and not templated then we require
		## a render item to display when wireframe drawing is required (display modes
		## is wire or wire-on-shaded)
		## 
		## 2a) If the object is templated then we use the same render item as in 1)
		## when in wireframe drawing is required.
		## 2b) However we also require a render item to display when in shaded mode.
	
		## Stock colors
		dormantColor = [ 0.0, 0.0, 1.0, 1.0 ]
		templateColor = [ 0.45, 0.45, 0.45, 1.0 ]
		activeTemplateColor = [ 1.0, 0.5, 0.5, 1.0 ]

		## Some local options to show debug interface
		##
		debugShader = False

		## Get render item used for draw in wireframe mode
		## (Mode to draw in is omr.MGeometry.kWireframe)
		##
		wireframeItem = None
		index = list.indexOf(self.sWireframeItemName)
		if index < 0:
			wireframeItem = omr.MRenderItem.create( self.sWireframeItemName, omr.MRenderItem.DecorationItem, omr.MGeometry.kLines)
			wireframeItem.setDrawMode(omr.MGeometry.kWireframe)

			## Set dormant wireframe with appropriate priority to not clash with
			## any active wireframe which may overlap in depth.
			wireframeItem.setDepthPriority( omr.MRenderItem.sDormantWireDepthPriority )

			list.append(wireframeItem)

			preCb = None
			postCb = None
			if debugShader:
				preCb = apiMeshPreDrawCallback
				postCb = apiMeshPostDrawCallback

			shader = shaderMgr.getStockShader(omr.MShaderManager.k3dSolidShader, preCb, postCb)
			if shader:
				## assign shader
				wireframeItem.setShader(shader)

				## sample debug code
				if debugShader:
					self.printShader( shader )

				## once assigned, no need to hold on to shader instance
				shaderMgr.releaseShader(shader)
		else:
			wireframeItem = list[index]

		## Get render item for handling mode shaded template drawing
		##
		shadedTemplateItem = None
		index = list.indexOf(self.sShadedTemplateItemName)
		if index < 0:
			shadedTemplateItem = omr.MRenderItem.create( self.sShadedTemplateItemName, omr.MRenderItem.DecorationItem, omr.MGeometry.kLines)
			shadedTemplateItem.setDrawMode(omr.MGeometry.kAll)

			## Set shaded item as being dormant wire since it should still be raised
			## above any shaded items, but not as high as active items.
			shadedTemplateItem.setDepthPriority( omr.MRenderItem.sDormantWireDepthPriority )

			list.append(shadedTemplateItem)

			shader = shaderMgr.getStockShader(omr.MShaderManager.k3dSolidShader)
			if shader:
				## assign shader
				shadedTemplateItem.setShader(shader)

				## sample debug code
				if debugShader:
					self.printShader( shader )

				## once assigned, no need to hold on to shader instance
				shaderMgr.releaseShader(shader)
		else:
			shadedTemplateItem = list[index]

		## Sample code to disable cast, receives shadows, and post effects.
		shadedTemplateItem.setCastsShadows( not self.fExternalItemsNonTri_NoShadowCast )
		shadedTemplateItem.setReceivesShadows( not self.fExternalItemsNonTri_NoShadowReceive )
		shadedTemplateItem.setExcludedFromPostEffects( self.fExternalItemsNonTri_NoPostEffects )

		displayStatus = omr.MGeometryUtilities.displayStatus(path)
		wireColor = omr.MGeometryUtilities.wireframeColor(path)

		## Enable / disable wireframe item and update the shader parameters
		##
		if wireframeItem:
			shader = wireframeItem.getShader()

			if displayStatus == omr.MGeometryUtilities.kTemplate:
				self.setSolidColor( shader, wireColor, templateColor)
				wireframeItem.enable(True)

			elif displayStatus == omr.MGeometryUtilities.kActiveTemplate:
				self.setSolidColor( shader, wireColor, activeTemplateColor)
				wireframeItem.enable(True)

			elif displayStatus == omr.MGeometryUtilities.kDormant:
				self.setSolidColor( shader, wireColor, dormantColor)
				wireframeItem.enable(True)

			elif displayStatus == omr.MGeometryUtilities.kActiveAffected:
				theColor = [ 0.5, 0.0, 1.0, 1.0 ]
				self.setSolidColor( shader, wireColor, theColor)
				wireframeItem.enable(True)

			else:
				wireframeItem.enable(False)

		## Enable / disable shaded/template item and update the shader parameters
		##
		if shadedTemplateItem:
			isTemplate = path.isTemplated()
			shader = shadedTemplateItem.getShader()

			if displayStatus == omr.MGeometryUtilities.kTemplate:
				self.setSolidColor( shader, wireColor, templateColor)
				shadedTemplateItem.enable(isTemplate)

			elif displayStatus == omr.MGeometryUtilities.kActiveTemplate:
				self.setSolidColor( shader, wireColor, activeTemplateColor)
				shadedTemplateItem.enable(isTemplate)

			elif displayStatus == omr.MGeometryUtilities.kDormant:
				self.setSolidColor( shader, wireColor, dormantColor)
				shadedTemplateItem.enable(isTemplate)

			else:
				shadedTemplateItem.enable(False)

	def updateActiveWireframeItem(self, path, list, shaderMgr):
		## Create a render item for active wireframe if it does not exist. Updating
		## shading parameters as necessary.

		selectItem = None
		index = list.indexOf(self.sSelectedWireframeItemName)
		if index < 0:
			selectItem = omr.MRenderItem.create(self.sSelectedWireframeItemName, omr.MRenderItem.DecorationItem, omr.MGeometry.kLines)
			selectItem.setDrawMode(omr.MGeometry.kAll)
			## This is the same as setting the argument raiseAboveShaded = True,/
			## since it sets the priority value to be the same. This line is just
			## an example of another way to do the same thing after creation of
			## the render item.
			selectItem.setDepthPriority( omr.MRenderItem.sActiveWireDepthPriority )
			list.append(selectItem)

			## For active wireframe we will use a shader which allows us to draw thick lines
			##
			drawThick = False
			shaderId = omr.MShaderManager.k3dSolidShader
			if drawThick:
				shaderId = omr.MShaderManager.k3dThickLineShader

			shader = shaderMgr.getStockShader(shaderId)
			if shader:
				## assign shader
				selectItem.setShader(shader)
				## once assigned, no need to hold on to shader instance
				shaderMgr.releaseShader(shader)
		else:
			selectItem = list[index]

		shader = None
		if selectItem:
			shader = selectItem.getShader()

		displayStatus = omr.MGeometryUtilities.displayStatus(path)
		wireColor = omr.MGeometryUtilities.wireframeColor(path)

		if displayStatus == omr.MGeometryUtilities.kLead:
			theColor = [ 0.0, 0.8, 0.0, 1.0 ]
			self.setSolidColor( shader, wireColor, theColor)
			selectItem.enable(True)

		elif displayStatus == omr.MGeometryUtilities.kActive:
			theColor = [ 1.0, 1.0, 1.0, 1.0 ]
			self.setSolidColor( shader, wireColor, theColor)
			selectItem.enable(True)

		elif displayStatus == omr.MGeometryUtilities.kHilite or displayStatus == omr.MGeometryUtilities.kActiveComponent:
			theColor = [ 0.0, 0.5, 0.7, 1.0 ]
			self.setSolidColor( shader, wireColor, theColor)
			selectItem.enable(True)

		else:
			selectItem.enable(False)

		## Add custom user data to selection item
		myCustomData = selectItem.customData()
		if not myCustomData:
			## create the custom data
			myCustomData = apiMeshUserData()
			myCustomData.fMessage = "I'm custom data!"
			selectItem.setCustomData(myCustomData)
		else:
			## modify the custom data
			myCustomData.fNumModifications += 1

	def updateWireframeModeFaceCenterItem(self, path, list, shaderMgr):
		## Add render item for face centers in wireframe mode, always show face centers
	   	## in wireframe mode except it is drawn as template.

		wireframeModeFaceCenterItem = None
		index = list.indexOf(self.sWireframeModeFaceCenterItemName)
		if index < 0:
			wireframeModeFaceCenterItem = omr.MRenderItem.create(self.sWireframeModeFaceCenterItemName, omr.MRenderItem.DecorationItem, omr.MGeometry.kPoints)
			wireframeModeFaceCenterItem.setDrawMode(omr.MGeometry.kWireframe)
			wireframeModeFaceCenterItem.setDepthPriority( omr.MRenderItem.sActiveWireDepthPriority )

			list.append(wireframeModeFaceCenterItem)

			shader = shaderMgr.getStockShader( omr.MShaderManager.k3dFatPointShader )
			if shader:
				## Set the point size parameter. Make it slightly larger for face centers
				pointSize = 5.0
				self.setSolidPointSize( shader, pointSize )

				wireframeModeFaceCenterItem.setShader(shader, self.sFaceCenterStreamName )

				## once assigned, no need to hold on to shader instance
				shaderMgr.releaseShader(shader)
			else:
				wireframeModeFaceCenterItem = list[index]

		if wireframeModeFaceCenterItem:
			shader = wireframeModeFaceCenterItem.getShader()
			if shader:
				## Set face center color in wireframe mode
				theColor = [ 0.0, 0.0, 1.0, 1.0 ]
				self.setSolidColor( shader, theColor )

			## disable the face center item when template
			isTemplate = path.isTemplated()
			wireframeModeFaceCenterItem.enable( not isTemplate )

	def updateShadedModeFaceCenterItem(self, path, list, shaderMgr):
		##Add render item for face centers in shaded mode. If the geometry is not selected,
		## face centers are not drawn.

		shadedModeFaceCenterItem = None
		index = list.indexOf(self.sShadedModeFaceCenterItemName)
		if index < 0:
			shadedModeFaceCenterItem = omr.MRenderItem.create( self.sShadedModeFaceCenterItemName, omr.MRenderItem.DecorationItem, omr.MGeometry.kPoints)
			shadedModeFaceCenterItem.setDrawMode(omr.MGeometry.kShaded | omr.MGeometry.kTextured)

			shadedModeFaceCenterItem.setDepthPriority(omr.MRenderItem.sActivePointDepthPriority)

			list.append(shadedModeFaceCenterItem)

			shader = shaderMgr.getStockShader( omr.MShaderManager.k3dFatPointShader )
			if shader:
				## Set the point size parameter. Make it slightly larger for face centers
				pointSize = 5.0
				self.setSolidPointSize( shader, pointSize )

				shadedModeFaceCenterItem.setShader(shader, self.sFaceCenterStreamName )

				## once assigned, no need to hold on to shader instance
				shaderMgr.releaseShader(shader)
		else:
			shadedModeFaceCenterItem = list[index]

		if shadedModeFaceCenterItem:
			shadedModeFaceCenterItem.setExcludedFromPostEffects(True)

			shader = shadedModeFaceCenterItem.getShader()
			wireColor = omr.MGeometryUtilities.wireframeColor(path)

			if shader:
				## Set face center color in shaded mode
				self.setSolidColor( shader, wireColor )

			displayStatus = omr.MGeometryUtilities.displayStatus(path)
			if displayStatus == omr.MGeometryUtilities.kActive or displayStatus == omr.MGeometryUtilities.kLead or displayStatus == omr.MGeometryUtilities.kActiveComponent or displayStatus == omr.MGeometryUtilities.kLive or displayStatus == omr.MGeometryUtilities.kHilite:
				shadedModeFaceCenterItem.enable(True)

			else:
				shadedModeFaceCenterItem.enable(False)

	def updateDormantVerticesItem(self, path, list, shaderMgr):
		## Create a render item for dormant vertices if it does not exist. Updating
		## shading parameters as necessary.

		vertexItem = None
		index = list.indexOf(self.sVertexItemName)
		if index < 0:
			vertexItem = omr.MRenderItem.create(self.sVertexItemName, omr.MRenderItem.DecorationItem, omr.MGeometry.kPoints)

			## Set draw mode to kAll : the item will be visible in the viewport and also during viewport 2.0 selection
			vertexItem.setDrawMode(omr.MGeometry.kAll)

			## Set the selection mask to kSelectMeshVerts : we want the render item to be used for Vertex Components selection
			mask = om.MSelectionMask(om.MSelectionMask.kSelectMeshVerts)
			mask.addMask(om.MSelectionMask.kSelectPointsForGravity)
			vertexItem.setSelectionMask( mask )

			## Set depth priority higher than wireframe and shaded render items,
			## but lower than active points.
			## Raising higher than wireframe will make them not seem embedded into the surface
			vertexItem.setDepthPriority( omr.MRenderItem.sDormantPointDepthPriority )
			list.append(vertexItem)

			shader = shaderMgr.getStockShader( omr.MShaderManager.k3dFatPointShader )
			if shader:
				## Set the point size parameter
				pointSize = 3.0
				self.setSolidPointSize( shader, pointSize )

				## assign shader
				vertexItem.setShader(shader)

				## once assigned, no need to hold on to shader instance
				shaderMgr.releaseShader(shader)
		else:
			vertexItem = list[index]

		if vertexItem:
			shader = vertexItem.getShader()
			## set color
			theColor = [ 0.0, 0.0, 1.0, 1.0 ]
			self.setSolidColor( shader, theColor )

			displayStatus = omr.MGeometryUtilities.displayStatus(path)

			## Generally if the display status is hilite then we
			## draw components.
			if displayStatus == omr.MGeometryUtilities.kHilite or omr.MPxGeometryOverride.pointSnappingActive():
				## In case the object is templated
				## we will hide the components to be consistent
				## with how internal objects behave.
				if path.isTemplated():
					vertexItem.enable(False)
				else:
					vertexItem.enable(True)
			else:
				vertexItem.enable(False)

			mySelectionData = vertexItem.customData()
			if not isinstance(mySelectionData, apiMeshHWSelectionUserData):
				## create the custom data
				mySelectionData = apiMeshHWSelectionUserData()
				vertexItem.setCustomData(mySelectionData)
			## update the custom data
			mySelectionData.fMeshGeom = self.fMeshGeom

	def updateActiveVerticesItem(self, path, list, shaderMgr):
		## Create a render item for active vertices if it does not exist. Updating
		## shading parameters as necessary.

		activeItem = None
		index = list.indexOf(self.sActiveVertexItemName)
		if index < 0:
			activeItem = omr.MRenderItem.create(self.sActiveVertexItemName, omr.MRenderItem.DecorationItem, omr.MGeometry.kPoints)
			activeItem.setDrawMode(omr.MGeometry.kAll)
			## Set depth priority to be active point. This should offset it
			## to be visible above items with "dormant point" priority.
			activeItem.setDepthPriority( omr.MRenderItem.sActivePointDepthPriority )
			list.append(activeItem)

			shaderId = omr.MShaderManager.k3dFatPointShader
			if self.fDrawActiveVerticesWithRamp:
				shaderId = omr.MShaderManager.k3dColorLookupFatPointShader

			shader = shaderMgr.getStockShader( shaderId )
			if shader:
				## Set the point size parameter. Make it slightly larger for active vertices
				pointSize = 5.0
				self.setSolidPointSize( shader, pointSize )

				## 1D Ramp color lookup option
				##
				if self.fDrawActiveVerticesWithRamp:
					textureMgr = omr.MRenderer.getTextureManager()

					## Assign dummy ramp lookup
					if not self.fColorRemapTexture:
						## Sample 3 colour ramp
						colorArray = [	1.0, 0.0, 0.0, 1.0,
										0.0, 1.0, 0.0, 1.0,
										0.0, 0.0, 1.0, 1.0 ]

						arrayLen = 3
						textureDesc = omr.MTextureDescription()
						textureDesc.setToDefault2DTexture()
						textureDesc.fWidth = arrayLen
						textureDesc.fHeight = 1
						textureDesc.fDepth = 1
						textureDesc.fBytesPerSlice = textureDesc.fBytesPerRow = 24*arrayLen
						textureDesc.fMipmaps = 1
						textureDesc.fArraySlices = 1
						textureDesc.fTextureType = omr.MRenderer.kImage1D
						textureDesc.fFormat = omr.MRenderer.kR32G32B32A32_FLOAT
						self.fColorRemapTexture = textureMgr.acquireTexture("", textureDesc, colorArray, False)

					if not self.fLinearSampler:
						samplerDesc = omr.MSamplerStateDesc()
						samplerDesc.addressU = omr.MSamplerState.kTexClamp
						samplerDesc.addressV = omr.MSamplerState.kTexClamp
						samplerDesc.addressW = omr.MSamplerState.kTexClamp
						samplerDesc.filter = omr.MSamplerState.kMinMagMipLinear
						fLinearSampler = omr.MStateManager.acquireSamplerState(samplerDesc)

					if self.fColorRemapTexture and self.fLinearSampler:
						## Set up the ramp lookup
						shader.setParameter("map", self.fColorRemapTexture)
						shader.setParameter("samp", self.fLinearSampler)

						## No remapping. The initial data created in the xrange 0...1
						##
						rampValueRange = om.MFloatVector(0.0, 1.0)
						shader.setParameter("UVRange", rampValueRange)

				## Assign shader. Use a named stream if we want to supply a different
				## set of "shared" vertices for drawing active vertices
				if self.fDrawSharedActiveVertices:
					activeItem.setShader(shader, self.sActiveVertexStreamName)
				else:
					activeItem.setShader(shader, None)

				## once assigned, no need to hold on to shader instance
				shaderMgr.releaseShader(shader)

		else:
			activeItem = list[index]

		if activeItem:
			shader = activeItem.getShader()
			if shader:
				## Set active color
				theColor = [ 1.0, 1.0, 0.0, 1.0 ]
				self.setSolidColor( shader, theColor )

			enable = (bool(self.fActiveVerticesSet) and self.enableActiveComponentDisplay(path))
			activeItem.enable( enable )

	def updateVertexNumericItems(self, path, list, shaderMgr):
		## Create render items for numeric display, and update shaders as necessary

		## Enable to show numeric render items
		enableNumericDisplay = False

		## Vertex id item
		##
		vertexItem = None
		index = list.indexOf(self.sVertexIdItemName)
		if index < 0:
			vertexItem = omr.MRenderItem.create( self.sVertexIdItemName, omr.MRenderItem.DecorationItem, omr.MGeometry.kPoints)
			vertexItem.setDrawMode(omr.MGeometry.kAll)
			vertexItem.setDepthPriority( omr.MRenderItem.sDormantPointDepthPriority )
			list.append(vertexItem)

			## Use single integer numeric shader
			shader = shaderMgr.getStockShader( omr.MShaderManager.k3dIntegerNumericShader )
			if shader:
				## Label the fields so that they can be found later on.
				vertexItem.setShader(shader, self.sVertexIdItemName)
				shaderMgr.releaseShader(shader)
		else:
			vertexItem = list[index]

		if vertexItem:
			shader = vertexItem.getShader()
			if shader:
				## set color
				theColor = [ 1.0, 1.0, 0.0, 1.0 ]
				self.setSolidColor( shader, theColor )

			vertexItem.enable(enableNumericDisplay)

		## Vertex position numeric render item
		##
		vertexItem = None
		index = list.indexOf(self.sVertexPositionItemName)
		if index < 0:
			vertexItem = omr.MRenderItem.create( self.sVertexPositionItemName, omr.MRenderItem.DecorationItem, omr.MGeometry.kPoints)
			vertexItem.setDrawMode(omr.MGeometry.kAll)
			vertexItem.setDepthPriority( omr.MRenderItem.sDormantPointDepthPriority )
			list.append(vertexItem)

			## Use triple float numeric shader
			shader = shaderMgr.getStockShader( omr.MShaderManager.k3dFloat3NumericShader )
			if shader:
				##vertexItem.setShader(shader)
				vertexItem.setShader(shader, self.sVertexPositionItemName)
				shaderMgr.releaseShader(shader)
		else:
			vertexItem = list[index]

		if vertexItem:
			shader = vertexItem.getShader()
			if shader:
				## set color
				theColor = [ 0.0, 1.0, 1.0, 1.0 ]
				self.setSolidColor( shader, theColor)

			vertexItem.enable(enableNumericDisplay)

	def updateAffectedComponentItems(self, path, list, shaderMgr):
		## Example of adding in items to hilite edges and faces. In this
		## case these are edges and faces which are connected to vertices
		## and we thus call them "affected" components.

		## Create / update "affected/active" edges component render item.
		##
		componentItem = None
		index = list.indexOf(self.sAffectedEdgeItemName)
		if index < 0:
			componentItem = omr.MRenderItem.create( self.sAffectedEdgeItemName, omr.MRenderItem.DecorationItem, omr.MGeometry.kLines)
			componentItem.setDrawMode(omr.MGeometry.kAll)

			## Set depth priority to be active line so that it is above wireframe
			## but below dormant and active points.
			componentItem.setDepthPriority( omr.MRenderItem.sActiveLineDepthPriority )
			list.append(componentItem)

			shader = shaderMgr.getStockShader( omr.MShaderManager.k3dThickLineShader )
			if shader:
				## Set lines a bit thicker to stand out
				lineSize = 1.0
				self.setLineWidth( shader, lineSize )

				## Assign shader.
				componentItem.setShader(shader, None)

				## once assigned, no need to hold on to shader instance
				shaderMgr.releaseShader(shader)
		else:
			componentItem = list[index]

		if componentItem:
			shader = componentItem.getShader()
			if shader:
				## Set affected color
				theColor = [ 1.0, 1.0, 1.0, 1.0 ]
				self.setSolidColor( shader, theColor )

			enable = ((bool(self.fActiveVerticesSet) or bool(self.fActiveEdgesSet)) and self.enableActiveComponentDisplay(path))
			componentItem.enable( enable )

		################################################################################

		## Create / update "affected/active" faces component render item
		##
		componentItem = None
		index = list.indexOf(self.sAffectedFaceItemName)
		if index < 0:
			componentItem = omr.MRenderItem.create( self.sAffectedFaceItemName, omr.MRenderItem.DecorationItem, omr.MGeometry.kTriangles)
			componentItem.setDrawMode(omr.MGeometry.kAll)
			## Set depth priority to be dormant wire so that edge and vertices
			## show on top.
			componentItem.setDepthPriority( omr.MRenderItem.sDormantWireDepthPriority )
			list.append(componentItem)

			shader = shaderMgr.getStockShader( omr.MShaderManager.k3dStippleShader )
			if shader:
				## Assign shader.
				componentItem.setShader(shader, None)

				## once assigned, no need to hold on to shader instance
				shaderMgr.releaseShader(shader)
		else:
			componentItem = list[index]

		if componentItem:
			shader = componentItem.getShader()
			if shader:
				## Set affected color
				theColor = [ 1.0, 1.0, 1.0, 1.0 ]
				self.setSolidColor( shader, theColor )

			enable = ((bool(self.fActiveVerticesSet) or bool(self.fActiveFacesSet)) and self.enableActiveComponentDisplay(path))
			componentItem.enable( enable )

	def updateSelectionComponentItems(self, path, list, shaderMgr):
		## Example of adding in items for edges and faces selection.

		## For the vertex selection, we already have a render item that display all the vertices (sVertexItemName)
		## we could use it for the selection as well.
	
		## But we have none that display the complete edges or faces,
		## sAffectedEdgeItemName and sAffectedFaceItemName only display a subset of the edges and faces
		## that are active or affected (one of their vertices is selected).

		## In order to be able to perform the selection of this components we'll create new render items
		## that will only be used for the selection (they will not be visible in the viewport)

		## Create / update selection edges component render item.
		##
		selectionItem = None
		index = list.indexOf(self.sEdgeSelectionItemName)
		if index < 0:
			selectionItem = omr.MRenderItem.create( self.sEdgeSelectionItemName, omr.MRenderItem.DecorationItem, omr.MGeometry.kLines)

			## Set draw mode to kSelectionOnly : the item will not be visible in the viewport, but during viewport 2.0 selection
			selectionItem.setDrawMode(omr.MGeometry.kSelectionOnly)

			## Set the selection mask to kSelectMeshEdges : we want the render item to be used for Edge Components selection
			selectionItem.setSelectionMask( om.MSelectionMask.kSelectMeshEdges )

			## Set depth priority to be selection so that it is above everything
			selectionItem.setDepthPriority( omr.MRenderItem.sSelectionDepthPriority )
			list.append(selectionItem)

			shader = shaderMgr.getStockShader(omr.MShaderManager.k3dThickLineShader)
			if shader:
				## Assign shader.
				selectionItem.setShader(shader, None)

				## once assigned, no need to hold on to shader instance
				shaderMgr.releaseShader(shader)
		else:
			selectionItem = list[index]

		if selectionItem:
			selectionItem.enable(True)

			mySelectionData = selectionItem.customData()
			if not isinstance(mySelectionData, apiMeshHWSelectionUserData):
				## create the custom data
				mySelectionData = apiMeshHWSelectionUserData()
				selectionItem.setCustomData(mySelectionData)
			## update the custom data
			mySelectionData.fMeshGeom = self.fMeshGeom

		## Create / update selection faces component render item.
		##
		index = list.indexOf(self.sFaceSelectionItemName)
		if index < 0:
			selectionItem = omr.MRenderItem.create( self.sFaceSelectionItemName, omr.MRenderItem.DecorationItem, omr.MGeometry.kTriangles)

			## Set draw mode to kSelectionOnly : the item will not be visible in the viewport, but during viewport 2.0 selection
			selectionItem.setDrawMode(omr.MGeometry.kSelectionOnly)

			## Set the selection mask to kSelectMeshFaces : we want the render item to be used for Face Components selection
			selectionItem.setSelectionMask( om.MSelectionMask.kSelectMeshFaces )

			## Set depth priority to be selection so that it is above everything
			selectionItem.setDepthPriority( omr.MRenderItem.sSelectionDepthPriority )
			list.append(selectionItem)

			shader = shaderMgr.getStockShader(omr.MShaderManager.k3dSolidShader)
			if shader:
				## Assign shader.
				selectionItem.setShader(shader, None)

				## once assigned, no need to hold on to shader instance
				shaderMgr.releaseShader(shader)
		else:
			selectionItem = list[index]

		if selectionItem:
			selectionItem.enable(True)

			mySelectionData = selectionItem.customData()
			if not isinstance(mySelectionData, apiMeshHWSelectionUserData):
				## create the custom data
				mySelectionData = apiMeshHWSelectionUserData()
				selectionItem.setCustomData(mySelectionData)
			## update the custom data
			mySelectionData.fMeshGeom = self.fMeshGeom

	def updateProxyShadedItem(self, path, list, shaderMgr):
		## In the event there are no shaded items we create a proxy
		## render item so we can still see where the object is.

		## Stock colors
		dormantColor = [ 0.0, 0.0, 1.0, 1.0 ]
		templateColor = [ 0.45, 0.45, 0.45, 1.0 ]
		activeTemplateColor = [ 1.0, 0.5, 0.5, 1.0 ]

		## Note that we still want to raise it above shaded even though
		## we don't have a shaded render item for this override.
		## This will handle in case where there is another shaded object
		## which overlaps this object in depth
		##
		raiseAboveShaded = True
		shadedDrawMode = omr.MGeometry.kShaded | omr.MGeometry.kTextured
		## Mark proxy item as wireframe if not using a material shader
		##
		useFragmentShader = self.fProxyShader < 0
		if not useFragmentShader:
			shadedDrawMode |= omr.MGeometry.kWireframe

		## Fragment + stipple shaders required triangles. All others
		## in the possible list requires lines
		##
		itemType = omr.MRenderItem.NonMaterialSceneItem
		primitive = omr.MGeometry.kLines
		filledProxy = useFragmentShader or self.fProxyShader == omr.MShaderManager.k3dStippleShader
		if filledProxy:
			itemType = omr.MRenderItem.MaterialSceneItem
			primitive = omr.MGeometry.kTriangles

		depthPriority = omr.MRenderItem.sDormantWireDepthPriority
		if raiseAboveShaded:
			depthPriority = omr.MRenderItem.sActiveWireDepthPriority

		proxyItem = None
		index = list.indexOf(self.sShadedProxyItemName)
		if index < 0:
			proxyItem = omr.MRenderItem.create( self.sShadedProxyItemName, itemType, primitive)
			proxyItem.setDrawMode(shadedDrawMode)
			proxyItem.setDepthPriority( depthPriority )

			proxyItem.setCastsShadows( not self.fExternalItems_NoShadowCast and self.fCastsShadows )
			proxyItem.setReceivesShadows( not self.fExternalItems_NoShadowReceive and self.fReceivesShadows )
			proxyItem.setExcludedFromPostEffects( self.fExternalItems_NoPostEffects )

			list.append(proxyItem)

			## We'll draw the proxy with a proxy shader as a visual cue
			##
			shader = None
			if useFragmentShader:
				shader = shaderMgr.getFragmentShader("mayaLambertSurface", "outSurfaceFinal", True)
				sBlue = [ 0.4, 0.4, 1.0 ]
				shader.setParameter("color", sBlue)
				shader.setIsTransparent(False)
			else:
				shader = shaderMgr.getStockShader( self.fProxyShader )

			if shader:
				if not filledProxy:
					self.setLineWidth(shader, 10.0)

				## assign shader
				proxyItem.setShader(shader)
				## once assigned, no need to hold on to shader instance
				shaderMgr.releaseShader(shader)
		else:
			proxyItem = list[index]

		## As this is a shaded item it is up to the plug-in to determine
		## on each update how to handle shadowing and effects.
		## Especially note that shadowing changes on the DAG object will trigger
		## a call to updateRenderItems()
		##
		proxyItem.setCastsShadows( not self.fExternalItems_NoShadowCast and self.fCastsShadows )
		proxyItem.setReceivesShadows( not self.fExternalItems_NoShadowReceive and self.fReceivesShadows )
		proxyItem.setExcludedFromPostEffects( self.fExternalItems_NoPostEffects )

		## Check for any shaded render items. A lack of one indicates
		## there is no shader assigned to the object.
		##
		haveShadedItems = False
		for item in list:
			if not item:
				continue
			drawMode = item.drawMode()
			if drawMode == omr.MGeometry.kShaded or drawMode == omr.MGeometry.kTextured:
				if item.name() != self.sShadedTemplateItemName:
					haveShadedItems = True
					break

		displayStatus = omr.MGeometryUtilities.displayStatus(path)
		wireColor = omr.MGeometryUtilities.wireframeColor(path)

		## Note that we do not toggle the item on and off just based on
		## display state. If this was so then call to MRenderer::setLightsAndShadowsDirty()
		## would be required as shadow map update does not monitor display state.
		##
		if proxyItem:
			shader = proxyItem.getShader()

			if displayStatus == omr.MGeometryUtilities.kTemplate:
				self.setSolidColor( shader, wireColor, templateColor )

			elif displayStatus == omr.MGeometryUtilities.kActiveTemplate:
				self.setSolidColor( shader, wireColor, activeTemplateColor )

			elif displayStatus == omr.MGeometryUtilities.kDormant:
				self.setSolidColor( shader, wireColor, dormantColor )

			## If we are missing shaded render items then enable
			## the proxy. Otherwise disable it.
			##
			if filledProxy:
				## If templated then hide filled proxy
				if path.isTemplated():
					proxyItem.enable(False)
				else:
					proxyItem.enable(not haveShadedItems)
			else:
				proxyItem.enable(not haveShadedItems)

	## Data stream (geometry requirements) handling
	def updateGeometryRequirements(self, requirements, data, activeVertexCount, totalVerts,	debugPopulateGeometry):
		## Examine the geometry requirements and create / update the
		## appropriate data streams. As render items specify both named and
		## unnamed data streams, both need to be handled here.

		## Vertex data
		positionBuffer = None
		positionDataAddress = None
		positionData = None

		vertexNumericIdBuffer = None
		vertexNumericIdDataAddress = None
		vertexNumericIdData = None

		vertexNumericIdPositionBuffer = None
		vertexNumericIdPositionDataAddress = None
		vertexNumericIdPositionData = None

		vertexNumericLocationBuffer = None
		vertexNumericLocationDataAddress = None
		vertexNumericLocationData = None

		vertexNumericLocationPositionBuffer = None
		vertexNumericLocationPositionDataAddress = None
		vertexNumericLocationPositionData = None

		activeVertexPositionBuffer = None
		activeVertexPositionDataAddress = None
		activeVertexPositionData = None

		activeVertexUVBuffer = None
		activeVertexUVDataAddress = None
		activeVertexUVData = None

		faceCenterPositionBuffer = None
		faceCenterPositionDataAddress = None
		faceCenterPositionData = None

		normalBuffer = None
		normalDataAddress = None
		normalData = None

		cpvBuffer = None
		cpvDataAddress = None
		cpvData = None

		uvBuffer = None
		uvDataAddress = None
		uvData = None

		numUVs = self.fMeshGeom.uvcoords.uvcount()

		descList = requirements.vertexRequirements()
		satisfiedRequirements = [False,] * len(descList)
		for i in xrange(len(descList)):
			desc = descList[i]
			## Fill in vertex data for drawing active vertex components (if drawSharedActiveVertices=True)
			##
			if self.fDrawSharedActiveVertices and (desc.name == self.sActiveVertexStreamName):
				if desc.semantic == omr.MGeometry.kPosition:
					if not activeVertexPositionBuffer:
						activeVertexPositionBuffer = data.createVertexBuffer(desc)
						if activeVertexPositionBuffer:
							satisfiedRequirements[i] = True
							if debugPopulateGeometry:
								print ">>> Fill in data for active vertex requirement '" + desc.name + "'. Semantic = kPosition"
							activeVertexPositionDataAddress = activeVertexPositionBuffer.acquire(activeVertexCount, True) ## writeOnly - we don't need the current buffer values
							if activeVertexPositionDataAddress:
								activeVertexPositionData = ((ctypes.c_float * 3)*activeVertexCount).from_address(activeVertexPositionDataAddress)

				elif desc.semantic == omr.MGeometry.kTexture:
					if not activeVertexUVBuffer:
						activeVertexUVBuffer = data.createVertexBuffer(desc)
						if activeVertexUVBuffer:
							satisfiedRequirements[i] = True
							if debugPopulateGeometry:
								print ">>> Fill in data for active vertex requirement '" + desc.name + "'. Semantic = kTexture"
							activeVertexUVDataAddress = activeVertexUVBuffer.acquire(activeVertexCount, True) ## writeOnly - we don't need the current buffer values
							if activeVertexUVDataAddress:
								activeVertexUVData = ((ctypes.c_float * 3)*activeVertexCount).from_address(activeVertexUVDataAddress)
				else:
					## do nothing for stuff we don't understand
					pass

			## Fill in vertex data for drawing face center components (if fDrawFaceCenters=True)
			##
			elif self.fDrawFaceCenters and desc.name == self.sFaceCenterStreamName:
				if desc.semantic == omr.MGeometry.kPosition:
					if not faceCenterPositionBuffer:
						faceCenterPositionBuffer = data.createVertexBuffer(desc)
						if faceCenterPositionBuffer:
							satisfiedRequirements[i] = True
							if debugPopulateGeometry:
								print ">>> Fill in data for face center vertex requirement '" + desc.name + "'. Semantic = kPosition"
							faceCenterPositionDataAddress = faceCenterPositionBuffer.acquire(self.fMeshGeom.faceCount, True) ## writeOnly - we don't need the current buffer values
							if faceCenterPositionDataAddress:
								faceCenterPositionData = ((ctypes.c_float * 3)*self.fMeshGeom.faceCount).from_address(faceCenterPositionDataAddress)

				else:
					## do nothing for stuff we don't understand
					pass

			## Fill vertex stream data used for dormant vertex, wireframe and shaded drawing.
			## Fill also for active vertices if (fDrawSharedActiveVertices=False)
			else:
				if desc.semantic == omr.MGeometry.kPosition:
					if desc.name == self.sVertexIdItemName:
						if not vertexNumericIdPositionBuffer:
							vertexNumericIdPositionBuffer = data.createVertexBuffer(desc)
							if vertexNumericIdPositionBuffer:
								satisfiedRequirements[i] = True
								if debugPopulateGeometry:
									print ">>> Fill in data for requirement '" + desc.name + "'. Semantic = kPosition"
									print "Acquire 3loat-numeric position buffer"
								vertexNumericIdPositionDataAddress = vertexNumericIdPositionBuffer.acquire(totalVerts, True) ## writeOnly - we don't need the current buffer values
								if vertexNumericIdPositionDataAddress:
									vertexNumericIdPositionData = ((ctypes.c_float * 3)*totalVerts).from_address(vertexNumericIdPositionDataAddress)

					elif desc.name == self.sVertexPositionItemName:
						if not vertexNumericLocationPositionBuffer:
							vertexNumericLocationPositionBuffer = data.createVertexBuffer(desc)
							if vertexNumericLocationPositionBuffer:
								satisfiedRequirements[i] = True
								if debugPopulateGeometry:
									print ">>> Fill in data for requirement '" + desc.name + "'. Semantic = kPosition"
									print "Acquire 3loat-numeric position buffer"
								vertexNumericLocationPositionDataAddress = vertexNumericLocationPositionBuffer.acquire(totalVerts, True) ## writeOnly - we don't need the current buffer values
								if vertexNumericLocationPositionDataAddress:
									vertexNumericLocationPositionData = ((ctypes.c_float * 3)*totalVerts).from_address(vertexNumericLocationPositionDataAddress)

					else:
						if not positionBuffer:
							positionBuffer = data.createVertexBuffer(desc)
							if positionBuffer:
								satisfiedRequirements[i] = True
								if debugPopulateGeometry:
									print ">>> Fill in data for requirement '" + desc.name + "'. Semantic = kPosition"
									print "Acquire unnamed position buffer"
								positionDataAddress = positionBuffer.acquire(totalVerts, True) ## writeOnly - we don't need the current buffer values
								if positionDataAddress:
									positionData = ((ctypes.c_float * 3)*totalVerts).from_address(positionDataAddress)

				elif desc.semantic == omr.MGeometry.kNormal:
					if not normalBuffer:
						normalBuffer = data.createVertexBuffer(desc)
						if normalBuffer:
							satisfiedRequirements[i] = True
							if debugPopulateGeometry:
								print ">>> Fill in data for requirement '" + desc.name + "'. Semantic = kNormal"
							normalDataAddress = normalBuffer.acquire(totalVerts, True) ## writeOnly - we don't need the current buffer values
							if normalDataAddress:
								normalData = ((ctypes.c_float * 3)*totalVerts).from_address(normalDataAddress)

				elif desc.semantic == omr.MGeometry.kTexture:
					numericValue = "numericvalue"
					numeric3Value ="numeric3value"

					## Fill in single numeric field
					if desc.semanticName.lower() == numericValue and desc.name == self.sVertexIdItemName:
						if not vertexNumericIdBuffer:
							vertexNumericIdBuffer = data.createVertexBuffer(desc)
							if vertexNumericIdBuffer:
								satisfiedRequirements[i] = True
								if debugPopulateGeometry:
									print ">>> Fill in data for requirement '" + desc.name + "'. Semantic = kTexture"
									print "Acquire 1loat numeric buffer"
								vertexNumericIdDataAddress = vertexNumericIdBuffer.acquire(totalVerts, True) ## writeOnly - we don't need the current buffer values
								if vertexNumericIdDataAddress:
									vertexNumericIdData = ((ctypes.c_float * 1)*totalVerts).from_address(vertexNumericIdDataAddress)

					## Fill in triple numeric field
					elif desc.semanticName.lower() == numeric3Value and desc.name == self.sVertexPositionItemName:
						if not vertexNumericLocationBuffer:
							vertexNumericLocationBuffer = data.createVertexBuffer(desc)
							if vertexNumericLocationBuffer:
								satisfiedRequirements[i] = True
								if debugPopulateGeometry:
									print ">>> Fill in data for requirement '" + desc.name + "'. Semantic = kTexture"
									print "Acquire 3loat numeric location buffer"
								vertexNumericLocationDataAddress = vertexNumericLocationBuffer.acquire(totalVerts, True) ## writeOnly - we don't need the current buffer values
								if vertexNumericLocationDataAddress:
									vertexNumericLocationData = ((ctypes.c_float * 3)*totalVerts).from_address(vertexNumericLocationDataAddress)

					## Fill in uv values
					elif desc.name != self.sVertexIdItemName and desc.name != self.sVertexPositionItemName:
						if not uvBuffer:
							uvBuffer = data.createVertexBuffer(desc)
							if uvBuffer:
								satisfiedRequirements[i] = True
								if debugPopulateGeometry:
									print ">>> Fill in data for requirement '" + desc.name + "'. Semantic = kTexture"
									print "Acquire a uv buffer"
								uvDataAddress = uvBuffer.acquire(totalVerts, True) ## writeOnly - we don't need the current buffer values
								if uvDataAddress:
									uvData = ((ctypes.c_float * 2)*totalVerts).from_address(uvDataAddress)

				elif desc.semantic == omr.MGeometry.kColor:
					if not cpvBuffer:
						cpvBuffer = data.createVertexBuffer(desc)
						if cpvBuffer:
							satisfiedRequirements[i] = True
							if debugPopulateGeometry:
								print ">>> Fill in data for requirement '" + desc.name + "'. Semantic = kColor"
							cpvDataAddress = cpvBuffer.acquire(totalVerts, True) ## writeOnly - we don't need the current buffer values
							if cpvDataAddress:
								cpvData = ((ctypes.c_float * 4)*totalVerts).from_address(cpvDataAddress)

				else:
					## do nothing for stuff we don't understand
					pass

		vid = 0
		pid = 0
		nid = 0
		uvid = 0
		cid = 0
		for i in xrange(self.fMeshGeom.faceCount):
			## ignore degenerate faces
			numVerts = self.fMeshGeom.face_counts[i]
			if numVerts > 2:
				for v in xrange(numVerts):
					if any((positionData, vertexNumericIdPositionData, vertexNumericLocationPositionData, vertexNumericLocationData)):
						position = self.fMeshGeom.vertices[self.fMeshGeom.face_connects[vid]]
						## Position used as position
						if positionData:
							positionData[pid][0] = position[0]
							positionData[pid][1] = position[1]
							positionData[pid][2] = position[2]

						## Move the id's a bit to avoid overlap. Position used as position.
						if vertexNumericIdPositionData:
							vertexNumericIdPositionData[pid][0] = position[0]+1.0
							vertexNumericIdPositionData[pid][1] = position[1]+1.0
							vertexNumericIdPositionData[pid][2] = position[2]+1.0

						## Move the locations a bit to avoid overlap. Position used as position.
						if vertexNumericLocationPositionData:
							vertexNumericLocationPositionData[pid][0] = position[0]+3.0
							vertexNumericLocationPositionData[pid][1] = position[1]+3.0
							vertexNumericLocationPositionData[pid][2] = position[2]+3.0

						## Position used as numeric display.
						if vertexNumericLocationData:
							vertexNumericLocationData[pid][0] = position[0]
							vertexNumericLocationData[pid][1] = position[1]
							vertexNumericLocationData[pid][2] = position[2]

						pid += 1

					if normalData:
						normal = self.fMeshGeom.normals[self.fMeshGeom.face_connects[vid]]
						normalData[nid][0] = normal[0]
						normalData[nid][1] = normal[1]
						normalData[nid][2] = normal[2]

						nid += 1

					if uvData:
						uv = [0, 0]
						if numUVs > 0:
							uvNum = self.fMeshGeom.uvcoords.uvId(vid)
							uv = self.fMeshGeom.uvcoords.getUV(uvNum)
						uvData[uvid][0] = uv[0]
						uvData[uvid][1] = uv[0]

						uvid += 1

					## Just same fake colors to show filling in requirements for
					## color-per-vertex (CPV)
					if cpvData:
						position = self.fMeshGeom.vertices[self.fMeshGeom.face_connects[vid]]
						cpvData[cid][0] = position[0]
						cpvData[cid][1] = position[1]
						cpvData[cid][2] = position[2]
						cpvData[cid][3] = 1.0

						cid += 1

					## Vertex id's used for numeric display
					if vertexNumericIdData:
						vertexNumericIdData[vid] = fMeshGeom.face_connects[vid]
						pass

					vid += 1

			elif numVerts > 0:
				vid += numVerts

		if positionDataAddress:
			positionBuffer.commit(positionDataAddress)

		if normalDataAddress:
			normalBuffer.commit(normalDataAddress)

		if uvDataAddress:
			uvBuffer.commit(uvDataAddress)

		if cpvDataAddress:
			cpvBuffer.commit(cpvDataAddress)
		
		if vertexNumericIdDataAddress:
			vertexNumericIdBuffer.commit(vertexNumericIdDataAddress)

		if vertexNumericIdPositionDataAddress:
			vertexNumericIdPositionBuffer.commit(vertexNumericIdPositionDataAddress)

		if vertexNumericLocationDataAddress:
			vertexNumericLocationBuffer.commit(vertexNumericLocationDataAddress)

		if vertexNumericLocationPositionDataAddress:
			vertexNumericLocationPositionBuffer.commit(vertexNumericLocationPositionDataAddress)

		## Fill in active vertex data buffer (only when fDrawSharedActiveVertices=True
		## which results in activeVertexPositionDataAddress and activeVertexPositionBuffer being non-NULL)
		##
		if activeVertexPositionData:
			if debugPopulateGeometry:
				print ">>> Fill in the data for active vertex position buffer base on component list"

			## Fill in position buffer with positions based on active vertex indexing list
			##
			if activeVertexCount > len(self.fMeshGeom.vertices):
				activeVertexCount = len(self.fMeshGeom.vertices)

			for i in xrange(activeVertexCount):
				position = self.fMeshGeom.vertices[ self.fActiveVertices[i] ]
				activeVertexPositionData[i][0] = position[0]
				activeVertexPositionData[i][1] = position[1]
				activeVertexPositionData[i][2] = position[2]

			activeVertexPositionBuffer.commit(activeVertexPositionDataAddress)

		if activeVertexUVData:
			if debugPopulateGeometry:
				print ">>> Fill in the data for active vertex uv buffer base on component list"

			## Fill in position buffer with positions based on active vertex indexing list
			##
			if activeVertexCount > len(self.fMeshGeom.vertices):
				activeVertexCount = len(self.fMeshGeom.vertices)

			for i in xrange(activeVertexCount):
				activeVertexUVData[i] = i / activeVertexCount

			activeVertexUVBuffer.commit(activeVertexUVDataAddress)

		## Fill in face center data buffer (only when fDrawFaceCenter=True
		## which results in faceCenterPositionDataAddress and faceCenterPositionBuffer being non-NULL)
		##
		if faceCenterPositionData:
			if debugPopulateGeometry:
				print ">>> Fill in the data for face center position buffer"

			## Fill in face center buffer with positions based on realtime calculations.
			##
			pid = 0
			vid = 0
			for faceId in xrange(self.fMeshGeom.faceCount):
				##tmp variables for calculating the face center position.
				x = 0.0
				y = 0.0
				z = 0.0

				faceCenterPosition = om.MPoint()

				## ignore degenerate faces
				numVerts = self.fMeshGeom.face_counts[faceId]
				if numVerts > 2:
					for v in xrange(numVerts):
						face_vertex_position = self.fMeshGeom.vertices[self.fMeshGeom.face_connects[vid]]
						x += face_vertex_position[0]
						y += face_vertex_position[1]
						z += face_vertex_position[2]

						vid += 1

					faceCenterPosition = om.MPoint(x, y, z) / numVerts

				elif numVerts > 0:
					vid += numVerts

				faceCenterPositionData[faceId][0] = faceCenterPosition[0]
				faceCenterPositionData[faceId][1] = faceCenterPosition[1]
				faceCenterPositionData[faceId][2] = faceCenterPosition[2]

			faceCenterPositionBuffer.commit(faceCenterPositionDataAddress)

		## Run around a second time and handle duplicate buffers and unknown buffers
		##
		for i in xrange(len(descList)):
			if satisfiedRequirements[i]:
				continue
			desc = descList[i]
			if self.fDrawSharedActiveVertices and (desc.name == self.sActiveVertexStreamName):
				if desc.semantic == omr.MGeometry.kPosition:
					satisfiedRequirements[i] = True
					self.cloneVertexBuffer(activeVertexPositionBuffer, data, desc, activeVertexCount, debugPopulateGeometry)
				elif desc.semantic == omr.MGeometry.kTexture:
					satisfiedRequirements[i] = True
					self.cloneVertexBuffer(activeVertexUVBuffer, data, desc, activeVertexCount, debugPopulateGeometry)
			elif self.fDrawFaceCenters and desc.name == self.sFaceCenterStreamName:
				if desc.semantic == omr.MGeometry.kPosition:
					satisfiedRequirements[i] = True
					self.cloneVertexBuffer(faceCenterPositionBuffer, data, desc, self.fMeshGeom.faceCount, debugPopulateGeometry)
			else:
				if desc.semantic == omr.MGeometry.kPosition:
					if desc.name == self.sVertexIdItemName:
						satisfiedRequirements[i] = True
						self.cloneVertexBuffer(vertexNumericIdPositionBuffer, data, desc, totalVerts, debugPopulateGeometry)
					elif desc.name == self.sVertexPositionItemName:
						satisfiedRequirements[i] = True
						self.cloneVertexBuffer(vertexNumericLocationPositionBuffer, data, desc, totalVerts, debugPopulateGeometry)
					else:
						satisfiedRequirements[i] = True
						self.cloneVertexBuffer(positionBuffer, data, desc, totalVerts, debugPopulateGeometry)
				elif desc.semantic == omr.MGeometry.kNormal:
					satisfiedRequirements[i] = True
					self.cloneVertexBuffer(normalBuffer, data, desc, totalVerts, debugPopulateGeometry)
				elif desc.semantic == omr.MGeometry.kTexture:
					numericValue = "numericvalue"
					numeric3Value ="numeric3value"
					if desc.semanticName.lower() == numericValue and desc.name == self.sVertexIdItemName:
						satisfiedRequirements[i] = True
						self.cloneVertexBuffer(vertexNumericIdBuffer, data, desc, totalVerts, debugPopulateGeometry)
					elif desc.semanticName.lower() == numeric3Value and desc.name == self.sVertexPositionItemName:
						satisfiedRequirements[i] = True
						self.cloneVertexBuffer(vertexNumericLocationBuffer, data, desc, totalVerts, debugPopulateGeometry)
					elif desc.name != self.sVertexIdItemName and desc.name != self.sVertexPositionItemName:
						satisfiedRequirements[i] = True
						self.cloneVertexBuffer(uvBuffer, data, desc, totalVerts, debugPopulateGeometry)
				elif desc.semantic == omr.MGeometry.kColor:
					satisfiedRequirements[i] = True
					self.cloneVertexBuffer(cpvBuffer, data, desc, totalVerts, debugPopulateGeometry)
			
			if not satisfiedRequirements[i]:
				## We have a strange buffer request we do not understand. Provide a set of Zeros sufficient to cover
				## totalVerts:
				destBuffer = data.createVertexBuffer(desc)
				if destBuffer:
					satisfiedRequirements[i] = True
					if debugPopulateGeometry:
						print ">>> Fill in dummy requirement '%s'" % (desc.name, )
					destBufferDataAddress = destBuffer.acquire(totalVerts, True) ## writeOnly - we don't need the current buffer values
					if destBufferDataAddress:
						destBufferData = ((ctypes.c_float * desc.dimension)*totalVerts).from_address(destBufferDataAddress)
						if destBufferData:
							for j in xrange(totalVerts):
								if desc.dimension == 4:
									destBufferData[j] = (1.0, 0.0, 0.0, 1.0)
								elif desc.dimension == 3:
									destBufferData[j] = (1.0, 0.0, 0.0)
								else:
									for k in xrange(desc.dimension):
										destBufferData[j][k] = 0.0
						destBuffer.commit(destBufferDataAddress)

	## 	Clone a vertex buffer to fulfill a duplicate requirement.
	##   Can happen for effects asking for multiple UV streams by
	##   name.
	def cloneVertexBuffer(self, srcBuffer, data, desc, dataSize, debugPopulateGeometry):
		if srcBuffer:
			destBuffer = data.createVertexBuffer(desc)
			if destBuffer:
				if debugPopulateGeometry:
					print ">>> Cloning requirement '%s'" % (desc.name, )
				destBufferDataAddress = destBuffer.acquire(dataSize, True) ## writeOnly - we don't need the current buffer values
				srcBufferDataAddress = srcBuffer.map()
				if destBufferDataAddress and srcBufferDataAddress:
					destBufferData = ((ctypes.c_float * desc.dimension)*dataSize).from_address(destBufferDataAddress)
					srcBufferData = ((ctypes.c_float * desc.dimension)*dataSize).from_address(srcBufferDataAddress)
					if destBufferData and srcBufferData:
						for j in xrange(dataSize):
							for k in xrange(desc.dimension):
								destBufferData[j][k] = srcBufferData[j][k]
					destBuffer.commit(destBufferDataAddress)
				srcBuffer.unmap()

	## Indexing for render item handling methods
	def updateIndexingForWireframeItems(self, wireIndexBuffer, item, data, totalVerts):
		## Create / update indexing required to draw wireframe render items.
		## There can be more than one render item using the same wireframe indexing
		## so it is passed in as an argument. If it is not null then we can
		## reuse it instead of creating new indexing.
	
		## Wireframe index buffer is same for both wireframe and selected render item
		## so we only compute and allocate it once, but reuse it for both render items
		if not wireIndexBuffer:
			wireIndexBuffer = data.createIndexBuffer(omr.MGeometry.kUnsignedInt32)
			if wireIndexBuffer:
				dataAddress = wireIndexBuffer.acquire(2*totalVerts, True) ## writeOnly - we don't need the current buffer values
				if dataAddress:
					data = (ctypes.c_uint * (2*totalVerts)).from_address(dataAddress)
					vid = 0
					first = 0
					idx = 0
					for faceIdx in xrange(self.fMeshGeom.faceCount):
						## ignore degenerate faces
						numVerts = self.fMeshGeom.face_counts[faceIdx]
						if numVerts > 2:
							first = vid
							for v in xrange(numVerts-1):
								data[idx] = vid
								vid += 1
								idx += 1
								data[idx] = vid
								idx += 1

							data[idx] = vid
							vid += 1
							idx += 1
							data[idx] = first
							idx += 1

						else:
							vid += numVerts

					wireIndexBuffer.commit(dataAddress)

		## Associate same index buffer with either render item
		if wireIndexBuffer:
			item.associateWithIndexBuffer(wireIndexBuffer)

	def updateIndexingForDormantVertices(self, item, data, numTriangles):
		## Create / update indexing for render items which draw dormant vertices

		indexBuffer = data.createIndexBuffer(omr.MGeometry.kUnsignedInt32)
		if indexBuffer:
			dataAddress = indexBuffer.acquire(3*numTriangles, True) ## writeOnly - we don't need the current buffer values
			if dataAddress:
				data = (ctypes.c_uint*(3*numTriangles)).from_address(dataAddress)
				## compute index data for triangulated convex polygons sharing
				## poly vertex data among triangles
				base = 0
				idx = 0
				for faceIdx in xrange(self.fMeshGeom.faceCount):
					## ignore degenerate faces
					numVerts = self.fMeshGeom.face_counts[faceIdx]
					if numVerts > 2:
						for v in xrange(1, numVerts-1):
							data[idx] = base
							data[idx+1] = base+v
							data[idx+2] = base+v+1
							idx += 3

						base += numVerts

				indexBuffer.commit(dataAddress)

			item.associateWithIndexBuffer(indexBuffer)

	def updateIndexingForFaceCenters(self, item, data, debugPopulateGeometry):
		## Create / update indexing for render items which draw face centers

		indexBuffer = data.createIndexBuffer(omr.MGeometry.kUnsignedInt32)
		if indexBuffer:
			dataAddress = indexBuffer.acquire(self.fMeshGeom.faceCount, True) ## writeOnly - we don't need the current buffer values
			if dataAddress:
				data = (ctypes.c_uint * self.fMeshGeom.faceCount).from_address(dataAddress)
				if debugPopulateGeometry:
					print ">>> Set up indexing for face centers"

				for i in xrange(self.fMeshGeom.faceCount):
					data[i] = 0
					pass

				idx = 0
				for i in xrange(self.fMeshGeom.faceCount):
					## ignore degenerate faces
					numVerts = self.fMeshGeom.face_counts[i]
					if numVerts > 2:
						data[idx] = idx
						idx += 1

				indexBuffer.commit(dataAddress)

			item.associateWithIndexBuffer(indexBuffer)

	def updateIndexingForVertices(self, item, data, numTriangles, activeVertexCount, debugPopulateGeometry):
		## Create / update indexing for render items which draw active vertices

		indexBuffer = data.createIndexBuffer(omr.MGeometry.kUnsignedInt32)
		if indexBuffer:
			dataAddress = None

			## If drawing shared active vertices then the indexing degenerates into
			## a numerically increasing index value. Otherwise a remapping from
			## the active vertex list indexing to the unshared position stream is required.
			##
			## 1. Create indexing for shared positions. In this case it
			## is a degenerate list since the position buffer was created
			## in linear ascending order.
			##
			if self.fDrawSharedActiveVertices:
				dataAddress = indexBuffer.acquire(activeVertexCount, True) ## writeOnly - we don't need the current buffer values
				if dataAddress:
					data = (ctypes.c_uint*activeVertexCount).from_address(dataAddress)
					if debugPopulateGeometry:
						print ">>> Set up indexing for shared vertices"
 
					for i in xrange(activeVertexCount):
						data[i] = i

			## 2. Create indexing to remap to unshared positions
			##
			else:
				if debugPopulateGeometry:
					print ">>> Set up indexing for unshared vertices"

				vertexCount = 3*numTriangles
				dataAddress = indexBuffer.acquire(vertexCount, True) ## writeOnly - we don't need the current buffer values
				if dataAddress:
					data = (ctypes.c_uint*vertexCount).from_address(dataAddress)
					for i in xrange(vertexCount):
						data[i] = vertexCount+1

					selectionIdSet = self.fActiveVerticesSet

					## compute index data for triangulated convex polygons sharing
					## poly vertex data among triangles
					base = 0
					lastFound = 0
					idx = 0
					for faceIdx in xrange(self.fMeshGeom.faceCount):
						## ignore degenerate faces
						numVerts = self.fMeshGeom.face_counts[faceIdx]
						if numVerts > 2:
							for v in xrange(1, numVerts-1):
								vertexId = self.fMeshGeom.face_connects[base]
								if vertexId in selectionIdSet:
									lastFound = base
									data[idx] = lastFound
									idx += 1

								vertexId = self.fMeshGeom.face_connects[base+v]
								if vertexId in selectionIdSet:
									lastFound = base+v
									data[idx] = lastFound
									idx += 1

								vertexId = self.fMeshGeom.face_connects[base+v+1]
								if vertexId in selectionIdSet:
									lastFound = base+v+1
									data[idx] = lastFound
									idx +1

							base += numVerts

					for i in xrange(vertexCount):
						if data[i] == vertexCount+1:
							 data[i] = lastFound

			if dataAddress:
				indexBuffer.commit(dataAddress)

			item.associateWithIndexBuffer(indexBuffer)

	def updateIndexingForEdges(self, item, data, totalVerts, fromSelection):
		## Create / update indexing for render items which draw affected edges

		indexBuffer = data.createIndexBuffer(omr.MGeometry.kUnsignedInt32)
		if indexBuffer:
			totalEdges = 2*totalVerts
			totalEdgesP1 = 2*totalVerts+1
			dataAddress = indexBuffer.acquire(totalEdges, True) ## writeOnly - we don't need the current buffer values
			if dataAddress:
				data = (ctypes.c_uint*totalEdges).from_address(dataAddress)
				for i in xrange(totalEdges):
					data[i] = totalEdgesP1
					pass

				displayAll = not fromSelection
				displayActives = (not displayAll and bool(self.fActiveEdgesSet))
				displayAffected = (not displayAll and not displayActives)

				selectionIdSet = None
				if displayActives:
					selectionIdSet = self.fActiveEdgesSet
				elif displayAffected:
					selectionIdSet = self.fActiveVerticesSet

				base = 0
				lastFound = 0
				idx = 0
				edgeId = 0
				for faceIdx in xrange(self.fMeshGeom.faceCount):
					## ignore degenerate faces
					numVerts = self.fMeshGeom.face_counts[faceIdx]
					if numVerts > 2:
						for v in xrange(numVerts):
							enableEdge = displayAll
							vindex1 = base + (v % numVerts)
							vindex2 = base + ((v+1) % numVerts)

							if displayAffected:
								## Check either ends of an "edge" to see if the
								## vertex is in the active vertex list
								##
								vertexId = self.fMeshGeom.face_connects[vindex1]
								if vertexId in selectionIdSet:
									enableEdge = True
									lastFound = vindex1

								if not enableEdge:
									vertexId2 = self.fMeshGeom.face_connects[vindex2]
									if vertexId2 in selectionIdSet:
										enableEdge = True
										lastFound = vindex2

							elif displayActives:
								## Check if the edge is active
								##
								if edgeId in selectionIdSet:
									enableEdge = True
									lastFound = vindex1

							## Add indices for "edge"
							if enableEdge:
								data[idx] = vindex1
								idx += 1
								data[idx] = vindex2
								idx += 1
							edgeId += 1

						base += numVerts

				if not displayAll:
					for i in xrange(totalEdges):
						if data[i] == totalEdgesP1:
							data[i] = lastFound

				indexBuffer.commit(dataAddress)

			item.associateWithIndexBuffer(indexBuffer)

	def updateIndexingForFaces(self, item, data, numTriangles, fromSelection):
		## Create / update indexing for render items which draw affected/active faces

		indexBuffer = data.createIndexBuffer(omr.MGeometry.kUnsignedInt32)
		if indexBuffer:
			numTriangleVertices = 3*numTriangles
			dataAddress = indexBuffer.acquire(numTriangleVertices, True) ## writeOnly - we don't need the current buffer values
			if dataAddress:
				data = (ctypes.c_uint*numTriangleVertices).from_address(dataAddress)
				for i in xrange(numTriangleVertices):
					data[i] = numTriangleVertices+1
					pass

				displayAll = not fromSelection
				displayActives = (not displayAll and bool(self.fActiveFacesSet))
				displayAffected = (not displayAll and not displayActives)

				selectionIdSet = None
				if displayActives:
					selectionIdSet = self.fActiveFacesSet
				elif displayAffected:
					selectionIdSet = self.fActiveVerticesSet

				base = 0
				lastFound = 0
				idx = 0
				for faceIdx in xrange(self.fMeshGeom.faceCount):
					## ignore degenerate faces
					numVerts = self.fMeshGeom.face_counts[faceIdx]
					if numVerts > 2:
						enableFace = displayAll

						if displayAffected:
							## Scan for any vertex in the active list
							##
							for v in xrange(1, numVerts-1):
								vertexId = self.fMeshGeom.face_connects[base]
								if vertexId in selectionIdSet:
									enableFace = True
									lastFound = base

								if not enableFace:
									vertexId2 = self.fMeshGeom.face_connects[base+v]
									if vertexId2 in selectionIdSet:
										enableFace = True
										lastFound = base+v

								if not enableFace:
									vertexId3 = self.fMeshGeom.face_connects[base+v+1]
									if vertexId3 in selectionIdSet:
										enableFace = True
										lastFound = base+v+1

						elif displayActives:
							## Check if the face is active
							##
							if faceIdx in selectionIdSet:
								enableFace = True
								lastFound = base

						## Found an active face
						## or one active vertex on the triangle so add indexing for the entire triangle.
						##
						if enableFace:
							for v in xrange(1, numVerts-1):
								data[idx] = base
								data[idx+1] = base+v
								data[idx+2] = base+v+1
								idx += 3

						base += numVerts

				if not displayAll:
					for i in xrange(numTriangleVertices):
						if data[i] == numTriangleVertices+1:
							data[i] = lastFound

				indexBuffer.commit(dataAddress)

			item.associateWithIndexBuffer(indexBuffer)

	def updateIndexingForShadedTriangles(self, item, data, numTriangles):
		## Create / update indexing for render items which draw filled / shaded
		## triangles.

		indexBuffer = data.createIndexBuffer(omr.MGeometry.kUnsignedInt32)
		if indexBuffer:
			dataAddress = indexBuffer.acquire(3*numTriangles, True) ## writeOnly - we don't need the current buffer values
			if dataAddress:
				data = (ctypes.c_uint * (3*numTriangles)).from_address(dataAddress)
				## compute index data for triangulated convex polygons sharing
				## poly vertex data among triangles
				base = 0
				idx = 0
				for faceIdx in xrange(self.fMeshGeom.faceCount):
					## ignore degenerate faces
					numVerts = self.fMeshGeom.face_counts[faceIdx]
					if numVerts > 2:
						for v in xrange(1, numVerts-1):
							data[idx] = base
							data[idx+1] = base+v
							data[idx+2] = base+v+1
							idx += 3

						base += numVerts

				indexBuffer.commit(dataAddress)

			item.associateWithIndexBuffer(indexBuffer)

################################################################################
##
## Node registry
##
## Registers/Deregisters apiMeshData geometry data,
## apiMeshCreator DG node, and apiMeshShape user defined shape.
##
################################################################################
##
## Strings for registering vp2 draw overrides. Plugin includes implementations
## of MPxSubSceneOverride and MPxGeometryOverride, set the boolean flag below
## to choose which is used.
sUseSubSceneOverride = False
sDrawDbClassification = "drawdb/geometry/apiMesh"
if sUseSubSceneOverride:
	sDrawDbClassification = "drawdb/subscene/apiMesh"
sDrawRegistrantId = "apiMeshPlugin"


def initializePlugin(obj):
	plugin = om.MFnPlugin(obj, "Autodesk", "3.0", "Any")

	global sUseSubSceneOverride, sDrawDbClassification, sDrawRegistrantId

	try:
		plugin.registerData("apiMeshData", apiMeshData.id, apiMeshData.creator, om.MPxData.kGeometryData)
	except:
		sys.stderr.write("Failed to register data\n")
		raise

	try:
		plugin.registerShape("apiMesh", apiMesh.id, apiMesh.creator, apiMesh.initialize, apiMeshUI.creator, sDrawDbClassification)
	except:
		sys.stderr.write("Failed to register node\n")
		raise

	try:
		plugin.registerNode("apiMeshCreator", apiMeshCreator.id, apiMeshCreator.creator, apiMeshCreator.initialize)
	except:
		sys.stderr.write("Failed to register node\n")
		raise

	try:
		if sUseSubSceneOverride:
			omr.MDrawRegistry.registerSubSceneOverrideCreator(sDrawDbClassification, sDrawRegistrantId, apiMeshSubSceneOverride.creator)
		else:
			omr.MDrawRegistry.registerGeometryOverrideCreator(sDrawDbClassification, sDrawRegistrantId, apiMeshGeometryOverride.creator)
	except:
		sys.stderr.write("Failed to register override\n")
		raise

	try:
		if sUseSubSceneOverride:
			omr.MDrawRegistry.registerComponentConverter(apiMeshSubSceneOverride.sVertexSelectionName, simpleComponentConverter_subsceneOverride.creatorVertexSelection)
			omr.MDrawRegistry.registerComponentConverter(apiMeshSubSceneOverride.sEdgeSelectionName, simpleComponentConverter_subsceneOverride.creatorEdgeSelection)
			omr.MDrawRegistry.registerComponentConverter(apiMeshSubSceneOverride.sFaceSelectionName, simpleComponentConverter_subsceneOverride.creatorFaceSelection)
		else:
			omr.MDrawRegistry.registerComponentConverter(apiMeshGeometryOverride.sVertexItemName, meshVertComponentConverter_geometryOverride.creator)
			omr.MDrawRegistry.registerComponentConverter(apiMeshGeometryOverride.sEdgeSelectionItemName, meshEdgeComponentConverter_geometryOverride.creator)
			omr.MDrawRegistry.registerComponentConverter(apiMeshGeometryOverride.sFaceSelectionItemName, meshFaceComponentConverter_geometryOverride.creator)
	except:
		sys.stderr.write("Failed to register component converters\n")
		raise

def uninitializePlugin(obj):
	plugin = om.MFnPlugin(obj)

	global sUseSubSceneOverride, sDrawDbClassification, sDrawRegistrantId

	try:
		if sUseSubSceneOverride:
			omr.MDrawRegistry.deregisterComponentConverter(apiMeshSubSceneOverride.sVertexSelectionName)
			omr.MDrawRegistry.deregisterComponentConverter(apiMeshSubSceneOverride.sEdgeSelectionName)
			omr.MDrawRegistry.deregisterComponentConverter(apiMeshSubSceneOverride.sFaceSelectionName)
		else:
			omr.MDrawRegistry.deregisterComponentConverter(apiMeshGeometryOverride.sVertexItemName)
			omr.MDrawRegistry.deregisterComponentConverter(apiMeshGeometryOverride.sEdgeSelectionItemName)
			omr.MDrawRegistry.deregisterComponentConverter(apiMeshGeometryOverride.sFaceSelectionItemName)
	except:
		sys.stderr.write("Failed to deregister component converters\n")
		pass

	try:
		if sUseSubSceneOverride:
			omr.MDrawRegistry.deregisterSubSceneOverrideCreator(sDrawDbClassification, sDrawRegistrantId)
		else:
			omr.MDrawRegistry.deregisterGeometryOverrideCreator(sDrawDbClassification, sDrawRegistrantId)
	except:
		sys.stderr.write("Failed to deregister override\n")
		pass

	try:
		plugin.deregisterNode(apiMeshCreator.id)
	except:
		sys.stderr.write("Failed to deregister node\n")
		pass

	try:
		plugin.deregisterNode(apiMesh.id)
	except:
		sys.stderr.write("Failed to deregister node\n")
		pass

	try:
		plugin.deregisterData(apiMeshData.id)
	except:
		sys.stderr.write("Failed to deregister data\n")
		pass

