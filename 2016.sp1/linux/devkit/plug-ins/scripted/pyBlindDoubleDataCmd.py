#-
# ==========================================================================
# Copyright 2015 Autodesk, Inc.  All rights reserved.
#
# Use of this software is subject to the terms of the Autodesk
# license agreement provided at the time of installation or download,
# or which otherwise accompanies this software in either electronic
# or hard copy form.
# ==========================================================================
#+

import sys
import io
import pickle
import maya.api.OpenMaya as om

def maya_useNewAPI():
	"""
	The presence of this function tells Maya that the plugin produces, and
	expects to be passed, objects created using the Maya Python API 2.0.
	"""
	pass


##############################################################################
##
## Proxy data class implementation
##
##############################################################################
class blindDoubleData(om.MPxData):
	s_id = om.MTypeId( 0x80003 )
	s_name = "blindDoubleData"
	fValue = 0

	def __init__(self):
		om.MPxData.__init__(self)

	@staticmethod
	def creator():
		return blindDoubleData()

	def readASCII(self, args, lastParsedElement):
		if len(args) > 0:
			self.fValue = args.asDouble(lastParsedElement)
			lastParsedElement = lastParsedElement+1
		return lastParsedElement

	def readBinary(self, istream, length):
		rawData = io.BytesIO(istream)
		reader = pickle.Unpickler(rawData)

		self.fValue = reader.load()

		return rawData.tell()

	def writeASCII(self, ostream):
		data = str(self.fValue)
		data += " "

		ostream[:] = bytearray(data, "ascii")

	def writeBinary(self, ostream):
		rawData = io.BytesIO()
		writer = pickle.Pickler(rawData)

		writer.dump( self.fValue )

		ostream[:] = rawData.getvalue()

	def copy(self, other):
		self.fValue = other.fValue

	def typeId(self):
		return blindDoubleData.s_id

	def name(self):
		return blindDoubleData.s_name

	def setValue(self, newValue):
		self.fValue = newValue

##############################################################################
##
## Command class implementation
##
##############################################################################
class blindDoubleDataCmd(om.MPxCommand):
	s_name = "blindDoubleData"
	iter = None

	def __init__(self):
		om.MPxCommand.__init__(self)

	@staticmethod
	def creator():
		return blindDoubleDataCmd()

	def doIt(self, args):
		sList = om.MGlobal.getActiveSelectionList()
		self.iter = om.MItSelectionList(sList, om.MFn.kInvalid)
		self.redoIt()

	def redoIt(self):
		# Iterate over all selected dependency nodes
		#
		while not self.iter.isDone():
			# Get the selected dependency node and create
			# a function set for it
			#
			dependNode = self.iter.getDependNode()
			self.iter.next()

			fnDN = om.MFnDependencyNode(dependNode)

			fullName = "blindDoubleData"
			try:
				fnDN.findPlug(fullName, True)
				# already have the attribute
				continue
			except:
				pass

			# Create a new attribute for our blind data
			#
			fnAttr = om.MFnTypedAttribute()
			briefName = "BDD"
			newAttr = fnAttr.create( fullName, briefName, blindDoubleData.s_id )

			# Now add the new attribute to the current dependency node
			#
			fnDN.addAttribute( newAttr )

			# Create a plug to set and retrive value off the node.
			#
			plug = om.MPlug( dependNode, newAttr )

			# Instantiate blindDoubleData and set its value.
			#
			newData = blindDoubleData()
			newData.setValue( 3.2 )

			# Set the value for the plug.
			#
			plug.setMPxData( newData )

			# Now try to retrieve the value off the plug as an MObject.
			#
			sData = plug.asMObject()

			# Convert the data back to MPxData.
			#
			pdFn = om.MFnPluginData( sData )
			data = pdFn.data()
			assert(isinstance(data, blindDoubleData))

	def undoIt(self):
		return

	def isUndoable(self):
		return True

##############################################################################
##
## The following routines are used to register/unregister
## the command we are creating within Maya
##
##############################################################################
def initializePlugin(obj):
	plugin = om.MFnPlugin(obj, "Autodesk", "3.0", "Any")
	try:
		plugin.registerData(blindDoubleData.s_name, blindDoubleData.s_id, blindDoubleData.creator)
	except:
		sys.stderr.write("Failed to register data\n")
		raise

	try:
		plugin.registerCommand(blindDoubleDataCmd.s_name, blindDoubleDataCmd.creator)
	except:
		sys.stderr.write("Failed to register command\n")
		raise

def uninitializePlugin(obj):
	plugin = om.MFnPlugin(obj)
	try:
		plugin.deregisterCommand(blindDoubleDataCmd.s_name)
	except:
		sys.stderr.write("Failed to deregister command\n")
		raise

	try:
		plugin.deregisterData(blindDoubleData.s_id)
	except:
		sys.stderr.write("Failed to deregister data\n")
		raise

