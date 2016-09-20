# Copyright 2015 Autodesk, Inc. All rights reserved.
# 
# Use of this software is subject to the terms of the Autodesk
# license agreement provided at the time of installation or download,
# or which otherwise accompanies this software in either electronic
# or hard copy form.

from ctypes import *
import maya.api.OpenMayaRender as omr
import maya.api.OpenMaya as om

# Example plugin: vertexBufferMutator.cpp
#
# This plug-in is an example of a custom MPxVertexBufferMutator.
# It provides custom vertex streams based on shader requirements coming from 
# an MPxShaderOverride.  The semanticName() in the MVertexBufferDescriptor is used 
# to signify a unique identifier for a custom stream.

# This plugin is meant to be used in conjunction with the hwPhongShader
# where the position geometry is defined with the semantic "swizzlePosition"

def maya_useNewAPI():
	"""
	The presence of this function tells Maya that the plugin produces, and
	expects to be passed, objects created using the Maya Python API 2.0.
	"""
	pass

class MyCustomBufferMutator(omr.MPxVertexBufferMutator):
	def __init__(self):
		omr.MPxVertexBufferMutator.__init__(self)

	def modifyVertexStream(self, object, vertexBuffer, targetIndexing):
		# get the descriptor from the vertex buffer.  
		# It describes the format and layout of the stream.
		descriptor = vertexBuffer.descriptor()
        
		# we are expecting a float stream.
		if descriptor.dataType != omr.MGeometry.kFloat:
			return

		# we are expecting a float3
		if descriptor.dimension != 3:
			return

		# we are expecting a position channel
		if descriptor.semantic != omr.MGeometry.kPosition:
			return

		# get the mesh from the current path
		# if it is not a mesh we do nothing.
		mesh = om.MFnMesh(object)

		vertexCount = vertexBuffer.vertexCount()
		if vertexCount <= 0:
			return

		# acquire the buffer to fill with data.
		buffer = vertexBuffer.acquire(vertexCount, False)	# writeOnly = False : we want the current buffer values

		inc = sizeof(c_float)
		address = buffer

		for i in range(0, vertexCount):
			# Here we swap the x, y and z values
			xaddr = address
			yaddr = address+inc
			zaddr = address+2*inc
			address += 3*inc

			x = c_float.from_address(xaddr).value
			c_float.from_address(xaddr).value = c_float.from_address(yaddr).value	# y --> x
			c_float.from_address(yaddr).value = c_float.from_address(zaddr).value	# z --> y
			c_float.from_address(zaddr).value = x									# x --> z

		# commit the buffer to signal completion.
		vertexBuffer.commit(buffer)

# This is the buffer generator creation function registered with the DrawRegistry.
# Used to initialize the generator.
def createMyCustomBufferMutator():
	return MyCustomBufferMutator()

# The following routines are used to register/unregister
# the vertex mutators with Maya
def initializePlugin(obj):

	omr.MDrawRegistry.registerVertexBufferMutator("swizzlePosition", createMyCustomBufferMutator)

def uninitializePlugin(obj):

	omr.MDrawRegistry.deregisterVertexBufferMutator("swizzlePosition")

