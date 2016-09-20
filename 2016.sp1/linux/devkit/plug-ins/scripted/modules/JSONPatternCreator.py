import sys
import json
import maya.cmds as cmds
import maya.api.OpenMaya as omAPI
from pyJsonAttrPatternInfo import PyJsonAttrPatternInfo as JsonKeys
from pyJsonAttrPatternInfo import jsonDebug as jsonDebug

#======================================================================
def attributeTypeName(node, attr):
	"""
	Find the name of an attribute's type. Some translation is required since
	some construction types don't exactly match the class type as returned
	by the attributeQuery command.

	Returns None if the type is either unknown or not one of the ones this
	script currently supports.
	"""
	attributeType = cmds.attributeQuery( attr, node=node, attributeType=True )

	if attributeType in JsonKeys.kNumericTypes:
		return attributeType

	if attributeType in JsonKeys.kTypeMatrix:
		return attributeType

	if attributeType == JsonKeys.kTypeEnum:
		return attributeType

	if attributeType == JsonKeys.kTypeMessage:
		return attributeType

	if attributeType == JsonKeys.kTypeString:
		return attributeType

	return None

#======================================================================
class JSONPatternCreator:
	"""
	Utility class to build JSON pattern objects from a set of attributes.
	The most common use is to create a set of patterns from a node and
	print them out using:
		import json
		import JSONPatternCreator
		patternCreator = JSONPatternCreator.JSONPatternCreator()
		jsonNodePattern = patternCreator.nodeAsJSON( mayaNodeName )
		json.dumps( jsonNodePattern, sort_keys=True, indent=4 )
	"""

	def __init__(self):
		"""
		Initialize the pattern creator.
		"""
		self.node = None
		self.dgNode = None
		self.nodeType = None

	#======================================================================
	def numericAttributeAsJSON(self, attr):
		"""
		Find the numeric-specific attribute parameters
			attr   = Attribute belonging to the pattern
			RETURN = JSON object representing the numeric-attribute-specific parameters
			         It's best to merge these into the main object one by one, rather
					 than making it a sub-object.
		"""
		pattern = {}

		# Set default value(s)
		defaultValue = cmds.attributeQuery( attr, node=self.node, listDefault=True )
		if defaultValue != None and len(defaultValue) > 0:
			pattern[JsonKeys.kKeyDefault] = defaultValue[0]

		# Set min/max values
		if cmds.attributeQuery( attr, node=self.node, minExists=True ):
			value = cmds.attributeQuery( attr, node=self.node, min=True )
			if value != None:
				if len(value) == 1:
					pattern[JsonKeys.kKeyMin] = value[0]
				elif len(value) > 1:
					pattern[JsonKeys.kKeyMin] = value

		if cmds.attributeQuery( attr, node=self.node, maxExists=True ):
			value = cmds.attributeQuery( attr, node=self.node, max=True )
			if value != None:
				if len(value) == 1:
					pattern[JsonKeys.kKeyMax] = value[0]
				elif len(value) > 1:
					pattern[JsonKeys.kKeyMax] = value

		if cmds.attributeQuery( attr, node=self.node, softMinExists=True ):
			value = cmds.attributeQuery( attr, node=self.node, softMin=True )
			if value != None:
				pattern[JsonKeys.kKeySoftMin] = value[0]

		if cmds.attributeQuery( attr, node=self.node, softMaxExists=True ):
			value = cmds.attributeQuery( attr, node=self.node, softMax=True )
			if value != None:
				pattern[JsonKeys.kKeySoftMax] = value[0]

		return pattern

	#======================================================================
	def compoundAttributeAsJSON(self, attr):
		"""
		Find the compound-specific attribute parameters
			attr   = Attribute belonging to the pattern
			RETURN = JSON object representing the compound-attribute-specific parameters
			         It's best to merge these into the main object one by one, rather
					 than making it a sub-object.
		"""
		pattern = {}
		# TODO:
		return pattern

	#======================================================================
	def lightDataAttributeAsJSON(self, attr):
		"""
		Find the lightData-specific attribute parameters
			attr   = Attribute belonging to the pattern
			RETURN = JSON object representing the lightData-attribute-specific parameters
			         It's best to merge these into the main object one by one, rather
					 than making it a sub-object.
		"""
		pattern = {}
		# TODO:
		return pattern

	#======================================================================
	def stringAttributeAsJSON(self, attr):
		"""
		Find the string-specific attribute parameters
			attr   = Attribute belonging to the pattern
			RETURN = JSON object representing the string-attribute-specific parameters
			         It's best to merge these into the main object one by one, rather
					 than making it a sub-object.
		"""
		pattern = {}
		stringDefault = cmds.attributeQuery( attr, node=self.node, listDefault=True )
		if stringDefault != None and len(stringDefault) > 0:
			pattern[JsonKeys.kKeyDefault] = stringDefault[0]
		return pattern

	#======================================================================
	def matrixAttributeAsJSON(self, attr):
		"""
		Find the matrix-specific attribute parameters
			attr   = Attribute belonging to the pattern
			RETURN = JSON object representing the matrix-attribute-specific parameters
			         It's best to merge these into the main object one by one, rather
					 than making it a sub-object.
		"""
		pattern = {}
		matrixDefault = cmds.attributeQuery( attr, node=self.node, listDefault=True )
		if matrixDefault != None and len(matrixDefault) > 0:
			pattern[JsonKeys.kKeyDefault] = matrixDefault[0]
		return pattern

	#======================================================================
	def typedAttributeAsJSON(self, attr):
		"""
		Find the typed-specific attribute parameters
			attr   = Attribute belonging to the pattern
			RETURN = JSON object representing the typed-attribute-specific parameters
			         It's best to merge these into the main object one by one, rather
					 than making it a sub-object.
		"""
		pattern = {}
		# TODO: 
		return pattern

	#======================================================================
	def enumAttributeAsJSON(self, attr):
		"""
		Find the enum-specific attribute parameters
			attr   = Attribute belonging to the pattern
			RETURN = JSON object representing the enum-attribute-specific parameters
			         It's best to merge these into the main object one by one, rather
					 than making it a sub-object.
		"""
		pattern = {}
		enumDefault = cmds.attributeQuery( attr, node=self.node, listDefault=True )
		if enumDefault != None and len(enumDefault) > 0:
			pattern[JsonKeys.kKeyDefault] = int(enumDefault[0])
		enumList = cmds.attributeQuery( attr, node=self.node, listEnum=True)
		if enumList != None and len(enumList) > 0:
			pattern[JsonKeys.kKeyEnumNames] = enumList[0].split(':')
		return pattern

	#======================================================================
	def attributeAsJSON(self, attr):
		"""
		Convert the attribute into its JSON attribute pattern equivalent.
			attr   = Attribute belonging to the pattern
			RETURN = JSON object representing the attribute
		"""
		jsonDebug( 'attributeAsJSON(%s)' % attr)
		jsonAttr = {}
		jsonAttr['name'] = attr
	
		shortName = cmds.attributeQuery( attr, node=self.node, shortName=True )
		# No need to write it if they two names are the same
		if shortName != attr: jsonAttr['shortName'] = shortName
	
		niceName = cmds.attributeQuery( attr, node=self.node, niceName=True )
		jsonAttr['niceName'] = niceName
	
		attributeType = attributeTypeName(self.node, attr)
		jsonAttr['attributeType'] = attributeType
		jsonDebug( '... type %s' % attributeType )
	
		categories = cmds.attributeQuery( attr, node=self.node, categories=True )
		if categories != None and len(categories) > 0:
			jsonAttr['categories'] = categories
	
		# Some flags default to false, some default to true, some are
		# code-dependent. Force setting of the ones who have changed
		# from their defaults, or whose defaults are unknown.
		#
		# Keep the list alphabetical for convenience
		flagList = []
		if cmds.attributeQuery( attr, node=self.node, affectsAppearance=True ):
			flagList.append( 'affectsAppearance' )
		if cmds.attributeQuery( attr, node=self.node, affectsWorldspace=True ):
			flagList.append( 'affectsWorldspace' )
		if not cmds.attributeQuery( attr, node=self.node, cachedInternally=True ):
			flagList.append( '!cached' )
		if not cmds.attributeQuery( attr, node=self.node, writable=True ):
			flagList.append( '!canConnectAsDst' )
		isReadable = True
		if not cmds.attributeQuery( attr, node=self.node, readable=True ):
			isReadable = False
			flagList.append( '!canConnectAsSrc' )
		if cmds.attributeQuery( attr, node=self.node, multi=True ):
			flagList.append( 'array' )
			# You can't set the indexMatters flag unless the attribute is an
			# unreadable array attribute. (It might be set otherwise, but just
			# by accident and the API doesn't support setting it incorrectly.)
			if cmds.attributeQuery( attr, node=self.node, indexMatters=True ) and not isReadable:
				flagList.append( 'indexMatters' )
		if cmds.attributeQuery( attr, node=self.node, channelBox=True ):
			flagList.append( 'channelBox' )
		if not cmds.attributeQuery( attr, node=self.node, connectable=True ):
			flagList.append( '!connectable' )
		if cmds.attributeQuery( attr, node=self.node, hidden=True ):
			flagList.append( 'hidden' )
		if cmds.attributeQuery( attr, node=self.node, indeterminant=True ):
			flagList.append( 'indeterminant' )
		if cmds.attributeQuery( attr, node=self.node, internalSet=True ):
			flagList.append( 'internal' )
		if cmds.attributeQuery( attr, node=self.node, keyable=True ):
			flagList.append( 'keyable' )
		else:
			flagList.append( '!keyable' )
		if cmds.attributeQuery( attr, node=self.node, renderSource=True ):
			flagList.append( 'renderSource' )
		if not cmds.attributeQuery( attr, node=self.node, storable=True ):
			flagList.append( '!storable' )
		if cmds.attributeQuery( attr, node=self.node, usedAsColor=True ):
			flagList.append( 'usedAsColor' )
		if cmds.attributeQuery( attr, node=self.node, usedAsFilename=True ):
			flagList.append( 'usedAsFilename' )
		if cmds.attributeQuery( attr, node=self.node, usesMultiBuilder=True ):
			flagList.append( 'usesArrayDataBuilder' )
		if cmds.attributeQuery( attr, node=self.node, worldspace=True ):
			flagList.append( 'worldspace' )
	
		# Write out the flag collection
		if len(flagList) > 0:
			jsonAttr['flags'] = flagList

		# Get attribute-type specific JSON parameters
		extraInfo = None
		if attributeType == 'enum':
			jsonDebug( '... decoding enum attribute parameters' )
			extraInfo = self.enumAttributeAsJSON(attr )
		elif attributeType in JsonKeys.kNumericTypes:
			jsonDebug( '... decoding numeric attribute parameters' )
			extraInfo = self.numericAttributeAsJSON(attr )
		elif attributeType in JsonKeys.kTypeMatrix:
			jsonDebug( '... decoding matrix attribute parameters' )
			extraInfo = self.matrixAttributeAsJSON(attr )
		elif attributeType == 'string':
			jsonDebug( '... decoding string attribute parameters' )
			extraInfo = self.stringAttributeAsJSON(attr )
		elif attributeType == 'message':
			jsonDebug( '... decoding message attribute parameters' )
		elif attributeType == 'compound':
			jsonDebug( '... decoding compound attribute parameters' )
			extraInfo = self.compoundAttributeAsJSON(attr )
		elif attributeType == 'lightData':
			jsonDebug( '... decoding lightData attribute parameters' )
			extraInfo = self.lightDataAttributeAsJSON(attr )
		elif attributeType == 'typed':
			jsonDebug( '... decoding typed attribute parameters' )
			extraInfo = self.typedAttributeAsJSON(attr )

		if extraInfo != None:
			for extraKey in extraInfo:
				jsonAttr[extraKey] = extraInfo[extraKey]
	
		return jsonAttr

	#======================================================================
	def attributeListAsJSON(self, patternName, attrs):
		"""
		Convert the list of attributes into a JSON attribute pattern object.
		Any attributes not supported will be written out as an (ignored) JSON property
		"unsupportedAttributes".
			patternName = Name of the new pattern
			attrs   	= Attributes belonging to the pattern
			RETURN		= JSON object containing the pattern for the attribute list
		"""
		# Firewall to ignore bad input
		if patternName == None or len(patternName) == 0:
			raise ValueError( 'Pattern name cannot be empty' )
		if attrs == None or len(attrs) == 0:
			return None

		unsupportedAttrs = []
		supportedAttrs = []
		pattern = {}
		pattern["name"] = patternName
	
		for attr in attrs:
			attrType = attributeTypeName( self.node, attr )
			if attrType != None:
				supportedAttrs.append( attr )
			else:
				unsupportedAttrs.append( '%s:%s' % (attr, cmds.attributeQuery( attr, node=self.node, attributeType=True )) )
		unsupportedAttrs.sort()
		supportedAttrs.sort()
		
		if len(unsupportedAttrs) > 0:
			pattern["unsupportedAttributes"] = unsupportedAttrs
	
		attrPatternList = []
		for attr in supportedAttrs:
			attrPatternList.append( self.attributeAsJSON( attr ) )
		pattern["attributes"] = attrPatternList
	
		return pattern

	#======================================================================
	def nodeAsJSON(self, node):
		"""
		node    = Node based on which the pattern is to be created
		RETURN  = JSON object containing patterns for all node attributes

		Method to walk through the list of attributes in a node and return the
		supported types out in a JSON attribute pattern format. The returned
		object can then be written out to a file or processed directly as a 
		JSON attribute pattern.

		There will be up to three attribute patterns specified in the object,
		one for each of static, dynamic, and extension attributes. Each of the
		patterns is only created if there is at least one attribute of that type.

		The names of the patterns for a node named "NODE" will be:
			dynamic_NODE
			static_NODE
			extension_NODE
		You really don't need me to explain which is which do you?
		"""
		self.node = node	# Need this local for getting attribute info
		self.dgNode = omAPI.MSelectionList().add(node).getDependNode(0)
		jsonDebug( 'Getting node information from %s' % str(self.dgNode) )
		patternList = []

		try:
			dynamicAttributes = cmds.listAttr( node, userDefined=True )
			extensionAttributes = cmds.listAttr( node, extension=True )
			allAttributes = cmds.listAttr( node )
			staticAttributes = [attr for attr in allAttributes if not attr in extensionAttributes and not attr in dynamicAttributes]
			#----------------------------------------------------------------------
			# Add the static attribute pattern to the string if there are any
			newPattern = self.attributeListAsJSON( "static_%s" % node, staticAttributes )
			if newPattern != None:
				patternList.append( newPattern )

			#----------------------------------------------------------------------
			# Add the dynamic attribute pattern to the string if there are any
			newPattern = self.attributeListAsJSON( "dynamic_%s" % node, dynamicAttributes )
			if newPattern != None:
				patternList.append( newPattern )

			#----------------------------------------------------------------------
			# Add the extension attribute pattern to the string if there are any
			newPattern = self.attributeListAsJSON( "extension_%s" % node, extensionAttributes )
			if newPattern != None:
				patternList.append( newPattern )

		except Exception, e:
			print 'ERR: Failed pattern creation on node %s (%s)' % (node, str(e))

		# The pattern should be all built up now
		return patternList

	#======================================================================
	def test(self):
		"""
		Run an internal consistency test on the pattern creator to verify its
		functions are operating correctly.
		"""
		factoryIsLoaded = cmds.pluginInfo('pyJsonAttrPatternFactory.py', query=True, loaded=True)
		if not factoryIsLoaded:
			try:
				cmds.loadPlugin('pyJsonAttrPatternFactory.py', quiet=True)
			except:
				# Repotest environment won't find it so skip it then
				return False
		factoryIsLoaded = cmds.pluginInfo('pyJsonAttrPatternFactory.py', query=True, loaded=True)
		# If the environment isn't set up to find the plugin there's
		# nothing we can do about it. It's not a failure of the test so
		# don't report anything other than a warning that the test could
		# not be run.
		if not factoryIsLoaded:
			print 'Warning: JSON attribute pattern factory could not be loaded, test aborted'
			return False

		patterns = """
		[
			{
				"name": "testPattern",
				"attributes": [
					{
						"name"			: "floatWithRanges",
						"shortName"		: "fwr",
						"defaultValue"	: 0.5,
						"min"			: -10.0,
						"max"			: 20.0,
						"softMin"		: 1.0,
						"softMax"		: 10.0,
						"attributeType"	: "float"
					} ,
					{
						"name"			: "float3WithRanges",
						"shortName"		: "ftwr",
						"defaultValue"	: [7.5, 7.6, 7.7],
						"min"			: [-17.0, -17.1, -17.2],
						"max"			: [27.0, 27.1, 27.2],
						"attributeType"	: "float3"
					} 
				]
			}
		]
		"""
		cmds.createAttrPatterns( patternType='json', patternDefinition=patterns )
		cmds.file( force=True, new=True )
		node = cmds.createNode( 'addMatrix' )
		cmds.applyAttrPattern( node, patternName='testPattern' )

		jsonString = self.nodeAsJSON( node )
		print json.dumps(jsonString, indent=4)

