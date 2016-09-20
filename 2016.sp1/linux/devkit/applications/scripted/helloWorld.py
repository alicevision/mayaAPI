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

#
# Description: 
#	
#	Running this application will start a Maya standalone python application.
#
# Usage:
#
#	Set the environment variable MAYA_LOCATION to your Maya path and in 
#	a shell on Linux execute:
#	
#	$MAYA_LOCATION/bin/mayapy helloWorld.py
#
#	NOTE: you must use the Python executable that ships
# 	with Maya for this to work.  This executable may be located
#	in a different directory for other platforms.

import maya.standalone
import maya.OpenMaya as OpenMaya

import sys

def main( argv = None ):
	try:
		maya.standalone.initialize( name='python' )
	except:
		sys.stderr.write( "Failed in initialize standalone application" )
		raise

	sys.stderr.write( "Hello world! (script output)\n" )
	OpenMaya.MGlobal().executeCommand( "print \"Hello world! (command script output)\\n\"" )
	maya.standalone.uninitialize()

if __name__ == "__main__":
    main()
