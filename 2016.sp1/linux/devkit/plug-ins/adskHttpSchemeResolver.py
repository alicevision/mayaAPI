"""
Custom file resolver derived from MPxFileResolver that handles
the URI 'htpp' scheme.
When this resolver is active, URI file paths using the 'http://<domain>/'
scheme will be processed using methods on this class.
Refer to MPxFileResolver for more information about custom file resolvers.

To use, make sure that adskHttpSchemeResolver.py is in
your MAYA_PLUG_IN_PATH then do the following:

# Load the plug-in
import maya.cmds
maya.cmds.loadPlugin("adskHttpSchemeResolver.py")
# Once loaded, Maya will call the resolver methods in this plug-in when
# a URI file path is encountered during file resolution processing
# (file open, setAttr, etc.)
# Unload the plug-in
maya.cmds.unloadPlugin("adskHttpSchemeResolver")
# Maya will no longer have access to this file
# resolver to handle URI file paths using the 'http:///' scheme  

"""

import sys
import maya.OpenMaya as OpenMaya
import maya.OpenMayaMPx as OpenMayaMPx
import maya.cmds as cmds
import urllib 
import os.path


#Specify the location for downloaded file
kTempDir = cmds.internalVar(userTmpDir=True)

# Set status messages on or off 
kWantStatusOutput = True; 

def downloadProgress(data, dataSize, fileSize):
	""" Display download progress - callback invoked by urllib.urlretrieve """
	percent = 100.0*data*dataSize/fileSize
	if percent > 100:
		percent = 100
	printStatus('Download progress: %.2f%%' % (percent))

def printStatus(msg):
	""" Print status output for diagnostic purposes (when enabled) """
	if (kWantStatusOutput):
		sys.stderr.write('%s: %s\n' % (adskHttpSchemeResolver.className(), msg) )

def printWarning(msg):
	""" Print warning messages """
	sys.stderr.write('Warning %s: %s\n' % (adskHttpSchemeResolver.className(), msg) )



# 'http' scheme resolver 
class adskHttpSchemeResolver(OpenMayaMPx.MPxFileResolver):
	"""
	This custom plug-in resolver handles the 'http' uri scheme.
	This resolver will copy the file from its 'http' location 
	using standard url library to a temporary location. This temporary location
	of the file is the fully qualified resolved path.
	It also implements a crude caching system.
	"""

	kPluginURIScheme = "http"
	kPluginResolverName = "adskHttpSchemeResolver"

	def __init__(self):
		OpenMayaMPx.MPxFileResolver.__init__(self)

	def uriScheme(self):
		return(self.kPluginURIScheme)

	def resolveURI(self,URI,mode):
		# Determine temporary location that will be used for this file
		uri = URI.asString()
		tempFile = kTempDir + URI.getFileName()
		result = u''

		# Check resolver mode, since not all modes require the file
		# to be downloaded.

		# When mode is kNone, simply return the resolved path
		if mode & OpenMayaMPx.MPxFileResolver.kNone:
			result = tempFile;
		
		# When mode is kInput, Maya is expecting to use this
		# file for input. Download the file to the temporary storage
		# area if it hasn't already been done. 
		elif mode & OpenMayaMPx.MPxFileResolver.kInput:	
			if not os.path.exists(tempFile):
				# If the file does not exist in the cache then go and
				# download it. At this point we assume we have a well
				# formed URI and that it exists on the web somewhere
				# Any error here and the resolved file is simply not
				# found. Would need to code for all the errors that
				# could go wrong here in a production case (lost
				# connections, server down, etc.)
				printStatus('Downloading URI: %s to location: %s' % (uri, tempFile))

				data = urllib.urlretrieve(uri, tempFile, downloadProgress)
				if os.path.exists(tempFile):
					printStatus('Download complete')
				else:
					printWarning('Download failed for URI: %s to location: %s'
						     % (uri, tempFile))
				result = tempFile
			else:
				printStatus('Download skipped, using cached version of URI: %s at location: %s'
					    % (uri, tempFile))
				result = tempFile

		# Unexpected mode - simply return the resolved path	
		else:
			printWarning('Unexpected resolve mode encountered: %s' % str(mode))
			result = tempFile

		# Return the resolved path	
		return result

	def performAfterSaveURI(self,URI,resolvedFullPath):
		uri = URI.asString()
		printStatus('Uploading local file %s to URI location %s'
					    % (resolvedFullPath, uri))

	@staticmethod
	# Creator for the proxy instance 
	def theCreator():
		return OpenMayaMPx.asMPxPtr( adskHttpSchemeResolver() )

	@staticmethod
	def className():
		return 'adskHttpSchemeResolver'

	

# Initialize the script plug-in
def initializePlugin(plugin):
	pluginFn = OpenMayaMPx.MFnPlugin(plugin)
	try:
		pluginFn.registerURIFileResolver( adskHttpSchemeResolver.kPluginResolverName,
						  adskHttpSchemeResolver.kPluginURIScheme,
						  adskHttpSchemeResolver.theCreator )
	except:
		sys.stderr.write( "Failed to register custom resolver: %s for scheme: %s\n" %
				  (adskHttpSchemeResolver.kPluginResolverName,
				   adskHttpSchemeResolver.kPluginURIScheme ))
		raise

# Uninitialize the script plug-in
def uninitializePlugin(plugin):
	pluginFn = OpenMayaMPx.MFnPlugin(plugin)
	try:
		pluginFn.deregisterURIFileResolver(adskHttpSchemeResolver.kPluginResolverName)
	except:
		sys.stderr.write(
			"Failed to deregister custom file resolver: %s\n" %
			adskHttpSchemeResolver.kPluginResolverName)
		raise

#-
# ==========================================================================
# Copyright (C) 2012 Autodesk, Inc. and/or its licensors.  All
# rights reserved.
#
# The coded instructions, statements, computer programs, and/or related
# material (collectively the "Data") in these files contain unpublished
# information proprietary to Autodesk, Inc. ("Autodesk") and/or its
# licensors, which is protected by U.S. and Canadian federal copyright
# law and by international treaties.
#
# The Data is provided for use exclusively by You. You have the right
# to use, modify, and incorporate this Data into other products for
# purposes authorized by the Autodesk software license agreement,
# without fee.
#
# The copyright notices in the Software and this entire statement,
# including the above license grant, this restriction and the
# following disclaimer, must be included in all copies of the
# Software, in whole or in part, and all derivative works of
# the Software, unless such copies or derivative works are solely
# in the form of machine-executable object code generated by a
# source language processor.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
# AUTODESK DOES NOT MAKE AND HEREBY DISCLAIMS ANY EXPRESS OR IMPLIED
# WARRANTIES INCLUDING, BUT NOT LIMITED TO, THE WARRANTIES OF
# NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR
# PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE, OR
# TRADE PRACTICE. IN NO EVENT WILL AUTODESK AND/OR ITS LICENSORS
# BE LIABLE FOR ANY LOST REVENUES, DATA, OR PROFITS, OR SPECIAL,
# DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES, EVEN IF AUTODESK
# AND/OR ITS LICENSORS HAS BEEN ADVISED OF THE POSSIBILITY
# OR PROBABILITY OF SUCH DAMAGES.
#
# ==========================================================================
#+
