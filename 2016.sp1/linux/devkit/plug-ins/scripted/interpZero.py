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
import maya.OpenMayaMPx as OpenMayaMPx
import maya.OpenMayaAnim as OpenMayaAnim

interpZeroId = OpenMayaAnim.MFnAnimCurve.kTangentShared3

# Animcurve Type definition
class interpZero(OpenMayaMPx.MPxAnimCurveInterpolator):
	def __init__(self):
		OpenMayaMPx.MPxAnimCurveInterpolator.__init__(self)
	def evaluate(time):
		return 0

# creator
def creator():
	return OpenMayaMPx.asMPxPtr( interpZero() )

# initialize the script plug-in
def initializePlugin(mobject):
	mplugin = OpenMayaMPx.MFnPlugin(mobject)
	try:
		mplugin.registerAnimCurveInterpolator( "InterpZero", interpZeroId, creator )
	except:
		sys.stderr.write( "Failed to register type: InterpZero" )
		raise

# uninitialize the script plug-in
def uninitializePlugin(mobject):
	mplugin = OpenMayaMPx.MFnPlugin(mobject)
	try:
		mplugin.deregisterAnimCurveInterpolator( interpZeroId )
	except:
		sys.stderr.write( "Failed to deregister type: InterpZero" )
		raise