# Copyright 2015 Autodesk, Inc. All rights reserved.
# 
# Use of this software is subject to the terms of the Autodesk
# license agreement provided at the time of installation or download,
# or which otherwise accompanies this software in either electronic
# or hard copy form.

from ctypes import *
import maya.api.OpenMayaRender as omr
import maya.api.OpenMaya as om

# Example plugin: pyCustomPrimitiveGenerator.py
#
# This plug-in is an example of a custom MPxPrimitiveGenerator.
# It provides custom primitives based on shader requirements coming from 
# an MPxShaderOverride.  The name() in the MIndexBufferDescriptor is used 
# to signify a unique identifier for a custom buffer.

# This primitive generator is provided for demonstration purposes only.
# It simply provides a triangle list for mesh objects with no vertex sharing.
# A more sophisticated primitive provider could be used to provide patch primitives
# for GPU tessellation.

# This plugin is meant to be used in conjunction with the d3d11Shader or cgShader or the hwPhongShader plugins.
# The customPrimitiveGeneratorDX11.fx and customPrimitiveGeneratorGL.cgfx files accompanying this sample
# can be loaded using the appropriate shader plugin.
# In any case, the environment variable MAYA_USE_CUSTOMPRIMITIVEGENERATOR needs to be set (any value is fine) for it to be enabled.

def maya_useNewAPI():
	"""
	The presence of this function tells Maya that the plugin produces, and
	expects to be passed, objects created using the Maya Python API 2.0.
	"""
	pass


class MyCustomPrimitiveGenerator(omr.MPxPrimitiveGenerator):
	def __init__(self):
		omr.MPxPrimitiveGenerator.__init__(self)

	def computeIndexCount(self, object, component):
		# get the mesh from the object
		mesh = om.MFnMesh(object)

		return mesh.numFaceVertices()

	def generateIndexing(self, object, component, sourceIndexing, targetIndexing, indexBuffer):
		# get the mesh from the object
		mesh = om.MFnMesh(object)

		for x in range(0, len(targetIndexing)):
			target = targetIndexing[x]
			if target != None and target.componentType() == omr.MComponentDataIndexing.kFaceVertex:
				# Get Triangles
				(triCounts, triVertIDs) = mesh.getTriangleOffsets()
				numTriVerts = len(triVertIDs)
  
				customNumTriVerts = numTriVerts * 2
				indexData = indexBuffer.acquire(customNumTriVerts, True)  # writeOnly = True - we don't need the current buffer values
				if indexData == 0:
					return omr.MGeometry.kInvalidPrimitive

				sharedIndices = target.indices()

				# Crawl the sharedIndices array to find the last
				# The new vertices will be added at the end
				nextNewVertexIndex = 0
				for idx in range(0, len(sharedIndices)):
					if sharedIndices[idx] > nextNewVertexIndex:
						nextNewVertexIndex = sharedIndices[idx]
				nextNewVertexIndex += 1

				dataType = indexBuffer.dataType()

				address = indexData
				uintinc = sizeof(c_uint)
				ushortinc = sizeof(c_ushort)

				newTriId = 0
				triId = 0
				while triId < numTriVerts:
					# split each triangle in two : add new vertex between vertexId1 and vertexId2
					vertexId0 = sharedIndices[triVertIDs[triId]]
					vertexId1 = sharedIndices[triVertIDs[triId+1]]
					vertexId2 = sharedIndices[triVertIDs[triId+2]]
					triId += 3

					newVertexIndex = nextNewVertexIndex
					nextNewVertexIndex += 1

					# Triangle 0 1 2 become two triangles : 0 1 X and 0 X 2
					if dataType == omr.MGeometry.kUnsignedInt32:
						xaddr = address
						yaddr = address+uintinc
						zaddr = address+2*uintinc
						address += 3*uintinc

						c_uint.from_address(xaddr).value = vertexId0
						c_uint.from_address(yaddr).value = vertexId1
						c_uint.from_address(zaddr).value = newVertexIndex

						xaddr = address
						yaddr = address+uintinc
						zaddr = address+2*uintinc
						address += 3*uintinc

						c_uint.from_address(xaddr).value = vertexId0
						c_uint.from_address(yaddr).value = newVertexIndex
						c_uint.from_address(zaddr).value = vertexId2

					elif dataType == omr.MGeometry.kUnsignedChar:
						xaddr = address
						yaddr = address+ushortinc
						zaddr = address+2*ushortinc
						address += 3*ushortinc

						c_ushort.from_address(xaddr).value = vertexId0
						c_ushort.from_address(yaddr).value = vertexId1
						c_ushort.from_address(zaddr).value = newVertexIndex

						xaddr = address
						yaddr = address+ushortinc
						zaddr = address+2*ushortinc
						address += 3*ushortinc

						c_ushort.from_address(xaddr).value = vertexId0
						c_ushort.from_address(yaddr).value = newVertexIndex
						c_ushort.from_address(zaddr).value = vertexId2

				indexBuffer.commit(indexData)

				return (omr.MGeometry.kTriangles, 3)
		return omr.MGeometry.kInvalidPrimitive

# This is the primitive generator creation function registered with the DrawRegistry.
# Used to initialize a custom primitive generator.
def createMyCustomPrimitiveGenerator():
	return MyCustomPrimitiveGenerator()

##########################################################################################
##########################################################################################

class MyCustomPositionBufferGenerator(omr.MPxVertexBufferGenerator):
	def __init__(self):
		omr.MPxVertexBufferGenerator.__init__(self)

	def getSourceIndexing(self, object, sourceIndexing):
		# get the mesh from the object
		mesh = om.MFnMesh(object)

		(vertexCount, vertexList) = mesh.getVertices()
		vertCount = len(vertexList)

		vertices = sourceIndexing.indices()
		vertices.setLength(vertCount)

		for i in range(0, vertCount):
			vertices[i] = vertexList[i]

		# assign the source indexing
		sourceIndexing.setComponentType(omr.MComponentDataIndexing.kFaceVertex)

		return True

	def getSourceStreams(self, object, sourceStreams):
		#No source stream needed
		return False

	def createVertexStream(self, object, vertexBuffer, targetIndexing, sharedIndexing, sourceStreams):
		# get the descriptor from the vertex buffer.  
		# It describes the format and layout of the stream.
		descriptor = vertexBuffer.descriptor()
        
		# we are expecting a float stream.
		if descriptor.dataType != omr.MGeometry.kFloat:
			return

		# we are expecting a dimension of 3
		dimension = descriptor.dimension
		if dimension != 3:
			return

		# we are expecting a position channel
		if descriptor.semantic != omr.MGeometry.kPosition:
			return

		# get the mesh from the current path
		# if it is not a mesh we do nothing.
		mesh = om.MFnMesh(object)

		indices = targetIndexing.indices()

		vertexCount = len(indices)
		if vertexCount <= 0:
			return

		# Keep track of the vertices that will be used to created a new vertex in-between
		extraVertices = []
		# Get Triangles
		(triCounts, triVertIDs) = mesh.getTriangleOffsets()
		numTriVerts = len(triVertIDs)

		sharedIndices = sharedIndexing.indices()

		triId = 0
		while triId < numTriVerts:
			# split each triangle in two : add new vertex between vertexId1 and vertexId2
			#vertexId0 = sharedIndices[triVertIDs[triId]]
			vertexId1 = sharedIndices[triVertIDs[triId+1]]
			vertexId2 = sharedIndices[triVertIDs[triId+2]]
			triId += 3

			extraVertices.append( [vertexId1, vertexId2] )

		newVertexCount = vertexCount + len(extraVertices)
		customBuffer = vertexBuffer.acquire(newVertexCount, True)	# writeOnly = True - we don't need the current buffer values
		if customBuffer != 0:
			
			address = customBuffer
			floatinc = sizeof(c_float)

			# Append 'real' vertices position
			vertId = 0
			while vertId < vertexCount:
				vertexId = indices[vertId]

				point = mesh.getPoint(vertexId)

				c_float.from_address(address + (vertId * dimension) * floatinc).value = point.x
				c_float.from_address(address + (vertId * dimension + 1) * floatinc).value = point.y
				c_float.from_address(address + (vertId * dimension + 2) * floatinc).value = point.z

				vertId += 1

			# Append the new vertices position, interpolated from vert1 and vert2
			for i in range(0, len(extraVertices)):
				vertexId1 = indices[extraVertices[i][0]]
				vertexId2 = indices[extraVertices[i][1]]

				point = mesh.getPoint(vertexId1)
				point += mesh.getPoint(vertexId2)
				point /= 2.0

				c_float.from_address(address + (vertId * dimension) * floatinc).value = point.x
				c_float.from_address(address + (vertId * dimension + 1) * floatinc).value = point.y
				c_float.from_address(address + (vertId * dimension + 2) * floatinc).value = point.z

				vertId += 1

			vertexBuffer.commit(customBuffer)

# This is the buffer generator creation function registered with the DrawRegistry.
# Used to initialize the generator.
def createMyCustomPositionBufferGenerator():
	return MyCustomPositionBufferGenerator()

##########################################################################################
##########################################################################################

class MyCustomNormalBufferGenerator(omr.MPxVertexBufferGenerator):
	def __init__(self):
		omr.MPxVertexBufferGenerator.__init__(self)

	def getSourceIndexing(self, object, sourceIndexing):
		# get the mesh from the object
		mesh = om.MFnMesh(object)

		(vertexCount, vertexList) = mesh.getVertices()
		vertCount = len(vertexList)

		vertices = sourceIndexing.indices()
		vertices.setLength(vertCount)

		for i in range(0, vertCount):
			vertices[i] = vertexList[i]

		# assign the source indexing
		sourceIndexing.setComponentType(omr.MComponentDataIndexing.kFaceVertex)

		return True

	def getSourceStreams(self, object, sourceStreams):
		#No source stream needed
		return False

	def createVertexStream(self, object, vertexBuffer, targetIndexing, sharedIndexing, sourceStreams):
		# get the descriptor from the vertex buffer.  
		# It describes the format and layout of the stream.
		descriptor = vertexBuffer.descriptor()
        
		# we are expecting a float stream.
		if descriptor.dataType != omr.MGeometry.kFloat:
			return

		# we are expecting a dimension of 3
		dimension = descriptor.dimension
		if dimension != 3:
			return

		# we are expecting a normal channel
		if descriptor.semantic != omr.MGeometry.kNormal:
			return

		# get the mesh from the current path
		# if it is not a mesh we do nothing.
		mesh = om.MFnMesh(object)

		indices = targetIndexing.indices()

		vertexCount = len(indices)
		if vertexCount <= 0:
			return

		# Keep track of the vertices that will be used to created a new vertex in-between
		extraVertices = []
		# Get Triangles
		(triCounts, triVertIDs) = mesh.getTriangleOffsets()
		numTriVerts = len(triVertIDs)

		sharedIndices = sharedIndexing.indices()

		triId = 0
		while triId < numTriVerts:
			# split each triangle in two : add new vertex between vertexId1 and vertexId2
			#vertexId0 = sharedIndices[triVertIDs[triId]]
			vertexId1 = sharedIndices[triVertIDs[triId+1]]
			vertexId2 = sharedIndices[triVertIDs[triId+2]]
			triId += 3

			extraVertices.append( [vertexId1, vertexId2] )

		newVertexCount = vertexCount + len(extraVertices)
		customBuffer = vertexBuffer.acquire(newVertexCount, True)	# writeOnly = True - we don't need the current buffer values
		if customBuffer != 0:
			
			address = customBuffer
			floatinc = sizeof(c_float)

			normals = mesh.getNormals()

			# Append 'real' vertices position
			vertId = 0
			while vertId < vertexCount:
				vertexId = indices[vertId]

				normal = normals[vertId]

				c_float.from_address(address + (vertId * dimension) * floatinc).value = normal.x
				c_float.from_address(address + (vertId * dimension + 1) * floatinc).value = normal.y
				c_float.from_address(address + (vertId * dimension + 2) * floatinc).value = normal.z

				vertId += 1

			# Append the new vertices normal, interpolated from vert1 and vert2
			for i in range(0, len(extraVertices)):
				vertexId1 = extraVertices[i][0]
				vertexId2 = extraVertices[i][1]

				normal = normals[vertexId1]
				normal += normals[vertexId2]
				normal /= 2.0

				c_float.from_address(address + (vertId * dimension) * floatinc).value = normal.x
				c_float.from_address(address + (vertId * dimension + 1) * floatinc).value = normal.y
				c_float.from_address(address + (vertId * dimension + 2) * floatinc).value = normal.z

				vertId += 1

			vertexBuffer.commit(customBuffer)

# This is the buffer generator creation function registered with the DrawRegistry.
# Used to initialize the generator.
def createMyCustomNormalBufferGenerator():
	return MyCustomNormalBufferGenerator()

##########################################################################################
##########################################################################################

# The following routines are used to register/unregister
# the vertex mutators with Maya

def initializePlugin(obj):

	omr.MDrawRegistry.registerPrimitiveGenerator("customPrimitiveTest", createMyCustomPrimitiveGenerator)

	omr.MDrawRegistry.registerVertexBufferGenerator("customPositionStream", createMyCustomPositionBufferGenerator)

	omr.MDrawRegistry.registerVertexBufferGenerator("customNormalStream", createMyCustomNormalBufferGenerator)

def uninitializePlugin(obj):

	omr.MDrawRegistry.deregisterPrimitiveGenerator("customPrimitiveTest")

	omr.MDrawRegistry.deregisterVertexBufferGenerator("customPositionStream")

	omr.MDrawRegistry.deregisterVertexBufferGenerator("customNormalStream")

