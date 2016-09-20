#-
# ==========================================================================
# Copyright 2010 Autodesk, Inc. All rights reserved.
#
# Use of this software is subject to the terms of the Autodesk
# license agreement provided at the time of installation or download,
# or which otherwise accompanies this software in either electronic
# or hard copy form.
# ==========================================================================
#+

#
# This plugin prints out the child plug information for a multiPlug.
# If the -index flag is used, the logical index values used by the plug
# will be returned.  Otherwise, the plug values will be returned.
#
# import maya.cmds as cmds
# cmds.loadPlugin("multiPlugInfoCmd.py")
# cmds.multiPlugInfo("myObj.myMultiAttr", index=True)
#

import maya.OpenMaya as OpenMaya
import maya.OpenMayaMPx as OpenMayaMPx
import sys
kPluginCmdName  = "multiPlugInfo"
kIndexFlag		= "-i"
kIndexFlagLong	= "-index"

# Wrapper to handle exception when MArrayDataHandle hits the end of the array.
def advance(arrayHdl):
	try:
		arrayHdl.next()
	except:
		return False

	return True


# command
class multiPlugInfo(OpenMayaMPx.MPxCommand):
	def __init__(self):
		OpenMayaMPx.MPxCommand.__init__(self)
		# setup private data members
		self.__isIndex = False

	def doIt(self, args):
		"""
		This method is called from script when this command is called.
		It should set up any class data necessary for redo/undo,
		parse any given arguments, and then call redoIt.
		"""
		argData = OpenMaya.MArgDatabase(self.syntax(), args)

		if argData.isFlagSet(kIndexFlag):
			self.__isIndex = True

		# Get the plug specified on the command line.
		slist = OpenMaya.MSelectionList()
		argData.getObjects(slist)
		if slist.length() == 0:
			print "Must specify an array plug in the form <nodeName>.<multiPlugName>."
			return

		plug = OpenMaya.MPlug()
		slist.getPlug(0, plug)
		if plug.isNull():
			print "Must specify an array plug in the form <nodeName>.<multiPlugName>."
			return

		# Construct a data handle containing the data stored in the plug.
		dh = plug.asMDataHandle()
		adh = None
		try:
			adh = OpenMaya.MArrayDataHandle(dh)
		except:
			print "Could not create the array data handle."
			plug.destructHandle(dh)
			return

		# Iterate over the values in the multiPlug.  If the index flag has been used, just return
		# the logical indices of the child plugs.  Otherwise, return the plug values.
		for i in range(adh.elementCount()):
			try:
				indx = adh.elementIndex()
			except:
				advance(adh)
				continue

			if self.__isIndex:
				self.appendToResult(indx)
			else:
				h = adh.outputValue()
				if h.isNumeric():
					if h.numericType() == OpenMaya.MFnNumericData.kBoolean:
						self.appendToResult(h.asBool())
					elif h.numericType() == OpenMaya.MFnNumericData.kShort:
						self.appendToResult(h.asShort())
					elif h.numericType() == OpenMaya.MFnNumericData.kInt:
						self.appendToResult(h.asInt())
					elif h.numericType() == OpenMaya.MFnNumericData.kFloat:
						self.appendToResult(h.asFloat())
					elif h.numericType() == OpenMaya.MFnNumericData.kDouble:
						self.appendToResult(h.asDouble())
					else:
						print "This sample command only supports boolean, integer, and floating point values."
			advance(adh)

		plug.destructHandle(dh)

# Creator
def cmdCreator():
	return OpenMayaMPx.asMPxPtr(multiPlugInfo())


# Syntax creator
def syntaxCreator():
	syntax = OpenMaya.MSyntax()
	syntax.addFlag(kIndexFlag, kIndexFlagLong, OpenMaya.MSyntax.kNoArg)
	syntax.setObjectType(OpenMaya.MSyntax.kSelectionList, 1, 1)
	return syntax


def initializePlugin(mobject):
	mplugin = OpenMayaMPx.MFnPlugin(mobject, "Autodesk", "1.0", "Any")
	try:
		mplugin.registerCommand(kPluginCmdName, cmdCreator, syntaxCreator)
	except:
		sys.stderr.write( "Failed to register command: %s\n" % kPluginCmdName)
		raise

def uninitializePlugin(mobject):
	mplugin = OpenMayaMPx.MFnPlugin(mobject)
	try:
		mplugin.deregisterCommand(kPluginCmdName)
	except:
		sys.stderr.write("Failed to unregister command: %s\n" % kPluginCmdName)
		raise

