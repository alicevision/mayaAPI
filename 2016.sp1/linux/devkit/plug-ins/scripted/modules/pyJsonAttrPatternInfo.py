"""
Simple class to hold common information used by the JSON attribute pattern
reader and writer.  To use, make sure that pyJsonAttrPatternInfo.py is in
your Python path then do the following:

from pyJsonAttrPatternInfo import PyJsonAttrPatternInfo as JsonKeys
...
flagKeyword = JsonKeys.kKeyFlags
...
"""
import sys
import maya.api.OpenMaya as omAPI

#======================================================================
def jsonDebug(dbgString):
	"""
	Print out some debugging messages if the flag is turned on.
	Lazily I'm making the location to turn it on be right here as well.
	"""
	debuggingJson = True
	if debuggingJson:
		print 'JSON: %s' % dbgString
		# Ensure the output gets seen immediately
		sys.stdout.flush()

#======================================================================
class PyJsonAttrPatternInfo():
	kPluginPatternFactoryName = 'json'

	# Keywords representing attribute data
	kKeyAcceptedTypes			= 'acceptedTypes'
	kKeyAcceptedNumericTypes	= 'acceptedNumericTypes'
	kKeyAcceptedPluginTypes		= 'acceptedPluginTypes'
	kKeyAttrType		= 'attributeType'
	kKeyFlags			= 'flags'
	kKeyName			= 'name'
	kKeyNiceName		= 'niceName'
	kKeyShortName		= 'shortName'
	kKeyCategories		= 'categories'
	kKeyEnumNames		= 'enumNames'
	kKeyDefault			= 'defaultValue'
	kKeyDisconnect		= 'disconnectBehavior'
	kKeyChildren		= 'children'
	kKeyMin				= 'min'
	kKeyMax				= 'max'
	kKeySoftMin			= 'softMin'
	kKeySoftMax			= 'softMax'
	#
	# Key = Keyword saying what type of number the numerical attribute is
	# Value = [Constant which defines that type in MFnNumericAttribute,
	#          Default value for that number type]
	# Note that the clever juxtaposition of numeric and unit types allows
	# the same code to be used for creating both.
	#
	kNumericTypes = { 'bool'    : omAPI.MFnNumericData.kBoolean,
					  'byte'    : omAPI.MFnNumericData.kByte,
					  'char'    : omAPI.MFnNumericData.kChar,
					  'short'   : omAPI.MFnNumericData.kShort,
					  'short2'  : omAPI.MFnNumericData.k2Short,
					  'short3'  : omAPI.MFnNumericData.k3Short,
					  'long'    : omAPI.MFnNumericData.kLong,
					  'int'     : omAPI.MFnNumericData.kInt,
					  'long2'   : omAPI.MFnNumericData.k2Long,
					  'int2'    : omAPI.MFnNumericData.k2Int,
					  'long3'   : omAPI.MFnNumericData.k3Long,
					  'int3'    : omAPI.MFnNumericData.k3Int,
					  'float'   : omAPI.MFnNumericData.kFloat,
					  'float2'  : omAPI.MFnNumericData.k2Float,
					  'float3'  : omAPI.MFnNumericData.k3Float,
					  'double'  : omAPI.MFnNumericData.kDouble,
					  'double2' : omAPI.MFnNumericData.k2Double,
					  'double3' : omAPI.MFnNumericData.k3Double,
					  'double4' : omAPI.MFnNumericData.k4Double,
					  'addr'    : omAPI.MFnNumericData.kAddr,
					  'angle'   : omAPI.MFnUnitAttribute.kAngle,
					  'distance': omAPI.MFnUnitAttribute.kDistance,
					  'time'    : omAPI.MFnUnitAttribute.kTime }
	#
	# Non-numeric attribute types
	#
	kTypeCompound = 'compound'
	kTypeEnum = 'enum'
	kTypeString = 'string'
	kTypeTyped = 'typed'
	kTypeMatrix = ['floatMatrix', 'doubleMatrix']
	kTypeMatrixTypes = { kTypeMatrix[0] : omAPI.MFnMatrixAttribute.kFloat,
						 kTypeMatrix[1] : omAPI.MFnMatrixAttribute.kDouble }
	kTypeLightData = 'lightData'
	kTypeMessage = 'message'
	# MFnData.kPlugin is deliberately ignored in the API so skip it
	kGenericTypes = { 'numeric'			: omAPI.MFnData.kNumeric,
					  'pluginGeometry'	: omAPI.MFnData.kPluginGeometry,
					  'string'			: omAPI.MFnData.kString,
					  'matrix'			: omAPI.MFnData.kMatrix,
					  'stringArray'		: omAPI.MFnData.kStringArray,
					  'doubleArray'		: omAPI.MFnData.kDoubleArray,
					  'intArray'		: omAPI.MFnData.kIntArray,
					  'pointArray'		: omAPI.MFnData.kPointArray,
					  'vectorArray'		: omAPI.MFnData.kVectorArray,
					  'componentList'	: omAPI.MFnData.kComponentList,
					  'mesh'			: omAPI.MFnData.kMesh,
					  'lattice'			: omAPI.MFnData.kLattice,
					  'nurbsCurve'		: omAPI.MFnData.kNurbsCurve,
					  'nurbsSurface'	: omAPI.MFnData.kNurbsSurface,
					  'sphere'			: omAPI.MFnData.kSphere,
					  'dynArrayAttrs'	: omAPI.MFnData.kDynArrayAttrs,
					  'dynSweptGeometry': omAPI.MFnData.kDynSweptGeometry,
					  'subdSurface'		: omAPI.MFnData.kSubdSurface,
					  'nObject'			: omAPI.MFnData.kNObject,
					  'nId'				: omAPI.MFnData.kNId }

	# List of keywords defining the flag to be set (preceded with "!" to unset
	# it). The values correspond to the attribute names in the MPy object.
	#
	kFlagFunctions = { 'readable'				: 'readable',
					   'writable'				: 'writable',
					   'canconnectassrc'		: 'readable',
					   'canconnectasdst'		: 'writable',
					   'connectable'			: 'connectable',
					   'storable'				: 'storable',
					   'cached'					: 'cached',
					   'array'					: 'array',
					   'indexmatters'			: 'indexMatters',
					   'keyable'				: 'keyable',
					   'channelbox'				: 'channelBox',
					   'hidden'					: 'hidden',
					   'usedascolor'			: 'usedAsColor',
					   'indeterminant'			: 'indeterminant',
					   'rendersource'			: 'renderSource',
					   'worldspace'				: 'worldSpace',
					   'affectsworldspace'		: 'affectsWorldSpace',
					   'usedasfilename'			: 'usedAsFilename',
					   'affectsappearance'		: 'affectsAppearance',
					   'usesarraydatabuilder'	: 'usesArrayDataBuilder',
					   'internal'				: 'internal' }

	# The list of legal disconnect behavior types (see MFnAttribute
	# documentation for what they mean.
	#
	kDisconnectBehaviors = { 'delete'  : omAPI.MFnAttribute.kDelete,
							 'reset'   : omAPI.MFnAttribute.kReset,
							 'nothing' : omAPI.MFnAttribute.kNothing }

#-
# ==========================================================================
# Copyright (C) 2011 Autodesk, Inc. and/or its licensors.  All 
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

