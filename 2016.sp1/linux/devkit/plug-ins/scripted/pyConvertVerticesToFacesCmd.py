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

import maya.cmds as cmds
import maya.api.OpenMaya as om

def maya_useNewAPI():
	"""
	The presence of this function tells Maya that the plugin produces, and
	expects to be passed, objects created using the Maya Python API 2.0.
	"""
	pass

##############################################################################
##
## Command class implementation
##
##############################################################################
class convertVerticesToFacesCmd(om.MPxCommand):
	s_name = "convertVerticesToFaces"
	
	def __init__(self):
		om.MPxCommand.__init__(self)
		self.previousSelectionList = None

	@staticmethod
	def creator():
		return convertVerticesToFacesCmd()

	def doIt(self, args):
		self.previousSelectionList = om.MGlobal.getActiveSelectionList()
		self.redoIt()

	def redoIt(self):
		multiVertexComponent = None
		singleVertexComponent = None
		finalFacesSelection = om.MSelectionList()
		vertexComponentIter = om.MItSelectionList(self.previousSelectionList, om.MFn.kMeshVertComponent)
		while not vertexComponentIter.isDone():
		    meshDagPath, multiVertexComponent = vertexComponentIter.getComponent()
		    meshName = meshDagPath.fullPathName();
		    if multiVertexComponent is not None:
		        itMeshVertex = om.MItMeshVertex(meshDagPath, multiVertexComponent)
		        connectedFacesIndices = itMeshVertex.getConnectedFaces()
		        faceIter = om.MItMeshPolygon(meshDagPath)

		        for i in connectedFacesIndices :
		            #GET THE VERTEX INDICES FOR CURRENT FACE:
		            faceIter.setIndex(i);
		            faceVerticesIndices = faceIter.getVertices()
		            faceIsContained = True
		            for j in faceVerticesIndices :
		                singleVertexList = om.MSelectionList() 
		                singleVertexList.clear()
		                vertexName = meshName 
		                vertexName += ".vtx["
		                vertexName += str(j)
		                vertexName += "]"
		                singleVertexList.add(vertexName)
		                meshDagPath, singleVertexComponent = singleVertexList.getComponent(0)
		                ##SEE WHETHER VERTEX BELONGS TO ORIGINAL SELECTION, AND IF IT DOESN'T, THEN THE WHOLE FACE IS NOT CONTAINED:
		                if not self.previousSelectionList.hasItem((meshDagPath, singleVertexComponent)):
		                    faceIsContained = False
		                    break
		            ##IF FACE IS "CONTAINED", ADD IT TO THE FINAL CONTAINED FACES LIST:
		            if faceIsContained:
		                faceName = meshName 
		                faceName += ".f["
		                faceName += str(i)
		                faceName += "]"
		                finalFacesSelection.add(faceName)
		    vertexComponentIter.next()


		## FINALLY, MAKE THE NEW "CONTAINED FACES", THE CURRENT SELECTION:
		om.MGlobal.setActiveSelectionList(finalFacesSelection, om.MGlobal.kReplaceList)
		## RETURN NEW CONTAINED FACES LIST FROM THE MEL COMMAND, AS AN ARRAY OF STRINGS:
		containedFacesArray = finalFacesSelection.getSelectionStrings()
		om.MPxCommand.setResult(containedFacesArray)
	
	def isUndoable(self):
		return True

	def undoIt(self):
		om.MGlobal.setActiveSelectionList(self.previousSelectionList, om.MGlobal.kReplaceList)

##############################################################################
##
## The following routines are used to register/unregister
## the command we are creating within Maya
##
##############################################################################
def initializePlugin(obj):
	plugin = om.MFnPlugin(obj, "Autodesk", "4.0", "Any")
	try:
		plugin.registerCommand(convertVerticesToFacesCmd.s_name, convertVerticesToFacesCmd.creator)
	except:
		sys.stderr.write("Failed to register command\n")
		raise

def uninitializePlugin(obj):
	plugin = om.MFnPlugin(obj)
	try:
		plugin.deregisterCommand(convertVerticesToFacesCmd.s_name)
	except:
		sys.stderr.write("Failed to deregister command\n")
		raise

