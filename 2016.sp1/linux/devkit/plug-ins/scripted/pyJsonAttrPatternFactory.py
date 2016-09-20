"""
To use, make sure that pyJsonAttrPatternFactory.py is in your MAYA_PLUG_IN_PATH
then do the following:

import maya
maya.cmds.loadPlugin("pyJsonAttrPatternFactory.py")
maya.cmds.listAttrPatterns(patternType=True)
// Return: ["json"]
"""
import os
import sys
import json
import traceback
import maya.api.OpenMaya as omAPI
JsonKeys = None
jsonDebug = None

#======================================================================
def import_helpers():
	""" 
	Equivalent to these import statements but presuming installation into
	a utils/ subdirectory of the current directory that's not in the current
	PYTHONPATH.

		from pyJsonAttrPatternInfo import PyJsonAttrPatternInfo as JsonKeys
		from pyJsonAttrPatternInfo import jsonDebug as jsonDebug
	
	The method is set up to still work even if the module path was already
	visible in the PYTHONPATH so there's no harm in setting it.

	Calling this twice will force re-import of the modules; that's a good
	thing for testing since you can unload the plug-in, load it back in and
	it will pick up the modified modules.
	"""
	global JsonKeys
	global jsonDebug

	# Find the subdirectory of the current directory, that's where modules
	# live. Had to do it this way to prevent modules from being misinterpreted
	# to be plug-ins themselves.
	#    The executable is in ..../runTime/bin
	#    The modules are in ..../runTime/devkit/plug-ins/scripted/modules
	#
	location = os.environ['MAYA_LOCATION']
	moduleDir = os.path.join( location, 'devkit', 'plug-ins', 'scripted', 'modules' )
	sys.path.append(moduleDir)

	# Load the module information, now visible using the updated path.
	module = __import__('pyJsonAttrPatternInfo', globals(), locals(), ["PyJsonAttrPatternInfo", "jsonDebug"], -1)
	reload(module) # Might be out of date

	# Name the interesting module elements
	JsonKeys = module.PyJsonAttrPatternInfo
	jsonDebug = module.jsonDebug

	# Remove the temporary path entry
	del sys.path[-1]

#======================================================================
def maya_useNewAPI():
	"""
	The presence of this function tells Maya that the plugin produces, and
	expects to be passed, objects created using the Maya Python API 2.0.
	"""
	pass

#======================================================================
class PyJsonAttrPatternFactory(omAPI.MPxAttributePatternFactory):
	#----------------------------------------------------------------------
	def __init__(self):
		omAPI.MPxAttributePatternFactory.__init__(self)
		self.currentPattern = None
		self.currentAttribute = None

	#----------------------------------------------------------------------
	@staticmethod
	def patternFactoryCreator():
		return PyJsonAttrPatternFactory()

	#----------------------------------------------------------------------
	def reportWarning(self, warningStr):
		"""
		Report a pattern parsing problem but do not raise a failure exception.
		This is for harmless errors that may or may not indicate a problem
		(e.g. a misspelled flag name)
		"""
		print 'WARN: Pattern %s, attribute %s: %s' % (self.currentPattern, self.currentAttribute, warningStr)

	#----------------------------------------------------------------------
	def reportError(self, errorStr):
		"""
		Report a pattern parsing error and raise a failure exception.
		Unfortunately there doesn't seem to be any way to report the actual
		line of the definition that failed (when it could be known) so
		the pattern name and attribute name will have to suffice to narrow
		down the source of the error.
		"""
		traceback.print_stack()
		raise ValueError( 'ERR: Pattern %s, attribute %s: %s' % (self.currentPattern, self.currentAttribute, errorStr) )

	#----------------------------------------------------------------------
	def parseStandardFlags(self, attr, flags):
		"""
		Apply the JSON-described flags to the given attributes. Only check the
		flags understood by all attribute types; let specific types handle
		their own particular flags.
			attr = Created attribute object
			flags = Array of flags to set
		"""
		jsonDebug( 'Parsing %d standard flags for "%s"' % (len(flags), str(attr)) )
		for flag in flags:
			valueToSet = True
			strippedFlag = flag.strip().rstrip()
			# If the flag starts with "!" then unset it rather than setting it
			if strippedFlag.startswith('!'):
				strippedFlag = strippedFlag[1:]
				valueToSet = False
			# Unrecognized flags will be silently ignored. They might be
			# relevant to derived attribute types.
			if strippedFlag.lower() in [x.lower() for x in JsonKeys.kFlagFunctions]:
				try:
					jsonDebug( '--- Set flag %s' % strippedFlag )
					jsonDebug( '--- Flag function = %s, value = %s' % ( JsonKeys.kFlagFunctions[strippedFlag.lower()], valueToSet ) )
					setattr( attr, JsonKeys.kFlagFunctions[strippedFlag.lower()], valueToSet )
				except Exception, e:
					self.reportError( 'Failed setting flag %s on attribute %s : %s"' % (strippedFlag, attr.name, str(e)) )
			else:
				self.reportWarning( 'Unrecognized attribute flag "%s" is ignored' % strippedFlag )

	#----------------------------------------------------------------------
	def parseCompoundAttribute(self, name, shortName, attrInfo):
		"""
		Given a JSON subsection describing a compound attribute create the
		attribute and all children, and set all of the provided flags/members
		for it.
			name = Attribute long name
			shortName = Attribute short name
			attrInfo = JSON object containing the main attribute information
		"""
		jsonDebug( 'parseCompoundAttribute(%s : %s)' % (name, attrInfo) )
		attr = None
		try:
			cAttr = omAPI.MFnCompoundAttribute()
			attr = cAttr.create( name, shortName )

			# Recursively create all children, and children of children, etc.
			if JsonKeys.kKeyChildren in attrInfo:
				childInfo = attrInfo[JsonKeys.kKeyChildren]
				tmpAttr = self.currentAttribute
				for child in childInfo:
					jsonDebug( 'Add compound child %s' % child )
					childAttr = self.parseAttribute( child )
					cAttr.addChild( childAttr )
				self.currentAttribute = tmpAttr
		except Exception, e:
			self.reportError( 'Error creating compound: %s' % str(e) )
		return attr

	#----------------------------------------------------------------------
	def parseEnumAttribute(self, name, shortName, attrInfo):
		"""
		Given a JSON subsection describing an enum attribute create the
		attribute and set all of the provided flags/members for it.
			name = Attribute long name
			shortName = Attribute short name
			attrInfo = JSON object containing the main attribute information
		"""
		jsonDebug( 'parseEnumAttribute(%s)' % name )
		eAttr = omAPI.MFnEnumAttribute()
		attr = eAttr.create( name, shortName )

		# Look for any fields being specified. (If no field names then the
		# attribute can only accept integer values.)
		if JsonKeys.kKeyEnumNames in attrInfo:
			enumIndex = 0
			try:
				for enumName in attrInfo[JsonKeys.kKeyEnumNames]:
					equalSign = enumName.find('=')
					if equalSign >= 0:
						enumIndex = int(enumName[equalSign+1:])
						enumName = enumName[:equalSign]
					eAttr.addField( enumName, enumIndex )
					enumIndex += 1
			except Exception, e:
				self.reportError( 'Bad enum specification: "%s"' % str(e) )

		# Set default after creation so that we can handle both enum names and
		# index values as a default
		if JsonKeys.kKeyDefault in attrInfo:
			defaultValue = attrInfo[JsonKeys.kKeyDefault]
			jsonDebug( 'Setting the enum default to "%s" of type %s' % (defaultValue, type(defaultValue)) )
			if type(defaultValue) == int or str(defaultValue).isdigit():
				eAttr.default = defaultValue
			else:
				eAttr.setDefaultByName( defaultValue )

		return attr

	#----------------------------------------------------------------------
	def parseTypedAttribute(self, name, shortName, attrInfo):
		"""
		Given a JSON subsection describing a typed attribute create the
		attribute and set all of the provided flags/members for it.
			name = Attribute long name
			shortName = Attribute short name
			attrInfo = JSON object containing the main attribute information
		"""
		jsonDebug( 'parseTypedAttribute(%s)' % name )

		# First convert the list of accepted types into the equivalent Maya
		# enum type values
		acceptedTypeEnums = []
		acceptedNumericEnums = []
		acceptedPluginTypes = []
		hasNumeric = False

		# Plugin data types are identified by name at runtime, they take
		# precedence.
		if JsonKeys.kKeyAcceptedPluginTypes in attrInfo:
			jsonDebug( '...getting accepted plugin types %s' % attrInfo[JsonKeys.kKeyAcceptedPluginTypes] )
			for pluginId in attrInfo[JsonKeys.kKeyAcceptedPluginTypes]:
				pId = omAPI.MTypeId()
				acceptedPluginTypes.append( pId.create( int(pluginId) ) )
		if JsonKeys.kKeyAcceptedTypes in attrInfo:
			if 'any' in attrInfo[JsonKeys.kKeyAcceptedTypes]:
				acceptedTypeEnums.append( omAPI.MFnData.kAny )
			else:
				for typeName in attrInfo[JsonKeys.kKeyAcceptedTypes]:
					if typeName == 'numeric':
						hasNumeric = True
						acceptedTypeEnums.append( JsonKeys.kGenericTypes[typeName] )
					elif typeName in JsonKeys.kGenericTypes:
						jsonDebug( '...getting accepted generic %s' % JsonKeys.kGenericTypes[typeName] )
						acceptedTypeEnums.append( JsonKeys.kGenericTypes[typeName] )
					else:
						self.reportError( 'Bad type name specification: "%s"' % str(typeName) )
		if JsonKeys.kKeyAcceptedNumericTypes in attrInfo:
			for typeName in attrInfo[JsonKeys.kKeyAcceptedNumericTypes]:
				if typeName in JsonKeys.kNumericTypes:
					jsonDebug( '...getting accepted numeric %s' % JsonKeys.kNumericTypes[typeName] )
					acceptedNumericEnums.append( JsonKeys.kNumericTypes[typeName] )
				else:
					self.reportError( 'Bad numeric type name specification: "%s"' % str(typeName) )

		# Numeric types have to be generic, it's just how the attributes are
		if len(acceptedTypeEnums) == 0 and len(acceptedNumericEnums) == 0 and len(acceptedPluginTypes) == 0:
			self.reportError( 'Need at least one accepted type' )
		# Only one data type means it can be an MFnTypedAttribute, except for
		# numeric type which for some reason is not supported in the API
		elif len(acceptedTypeEnums) == 1 and len(acceptedNumericEnums) == 0 and len(acceptedPluginTypes) == 0 and not hasNumeric:
			jsonDebug( '--- Accepts only one type : %s' % acceptedTypeEnums[0] )
			tAttr = omAPI.MFnTypedAttribute()
			attr = tAttr.create( name, shortName, acceptedTypeEnums[0] )
			jsonDebug( '--- created' )
		# One plugin type has a special MFnTypedAttribute constructor
		elif len(acceptedTypeEnums) == 0 and len(acceptedNumericEnums) == 0 and len(acceptedPluginTypes) == 1:
			jsonDebug( '--- Accepts only one plugin : %s' % acceptedPluginTypes[0] )
			tAttr = omAPI.MFnTypedAttribute()
			attr = tAttr.create( name, shortName, acceptedPluginTypes[0] )
			jsonDebug( '--- created' )
		# Every other combination forces a generic attribute
		else:
			jsonDebug( '--- Accepts multiple or base numeric types' )
			tAttr = omAPI.MFnGenericAttribute()
			attr = tAttr.create( name, shortName )
			for typeEnum in acceptedTypeEnums:
				jsonDebug( '--> add data type %s' % typeEnum )
				tAttr.addDataType( typeEnum )
			for numericEnum in acceptedNumericEnums:
				jsonDebug( '--> add numeric type %s' % numericEnum )
				tAttr.addNumericType( numericEnum )
			for pluginId in acceptedPluginTypes:
				jsonDebug( '--> add plugin type %s' % pluginId )
				tAttr.addTypeId( pluginId )
			jsonDebug( '--- created' )

		return attr

	#----------------------------------------------------------------------
	def parseLightDataAttribute(self, name, shortName, attrInfo):
		"""
		Given a JSON subsection describing a light data attribute create the
		attribute and set all of the provided flags/members for it.
			name = Attribute long name
			shortName = Attribute short name
			attrInfo = JSON object containing the main attribute information
		"""
		# List of all child attributes with their numeric type and default values
		cNames =  [ 'direction', 'intensity', 'ambient', 'diffuse', 'specular',
					'shadowFraction', 'preShadowIntensity', 'blindData' ]
		lightChildren = { cNames[0]	: (omAPI.MFnNumericData.k3Float,  [0.0,0.0,0.0]),
						  cNames[1]	: (omAPI.MFnNumericData.k3Float,  [0.0,0.0,0.0]),
						  cNames[2]	: (omAPI.MFnNumericData.kBoolean, 0),
						  cNames[3]	: (omAPI.MFnNumericData.kBoolean, 0),
						  cNames[4]	: (omAPI.MFnNumericData.kBoolean, 0),
						  cNames[5]	: (omAPI.MFnNumericData.kFloat,   0.0),
						  cNames[6]	: (omAPI.MFnNumericData.kFloat,   0.0),
						  cNames[7]	: (omAPI.MFnNumericData.kAddr,    0) }

		jsonDebug( 'parseLightDataAttribute(%s)' % name )
		ldAttr = omAPI.MFnLightDataAttribute()

		missingNames = []
		ldChildren = []
		defaultValues = []
		for child in cNames:
			try:
				jsonDebug( 'Creating light data child %s' % child )
				childInfo = attrInfo[child]
				cName = childInfo[JsonKeys.kKeyName]
				jsonDebug( '--- child name %s' % cName )
				if JsonKeys.kKeyShortName in childInfo:
					cShortName = childInfo[JsonKeys.kKeyShortName]
				else:
					cShortName = cName
				jsonDebug( '--- child short name %s' % cShortName )
				if JsonKeys.kKeyDefault in childInfo and child != 'blindData':
					jsonDebug( '--- Defining a default' )
					defaultValues.append( childInfo[JsonKeys.kKeyDefault] )
				else:
					jsonDebug( '--- Accepting default 0' )
					defaultValues.append( lightChildren[child][1] )
				jsonDebug( '--- child default %s' % defaultValues[-1] )

				nAttr = omAPI.MFnNumericAttribute()
				jsonDebug( '--- created numeric type %s' % lightChildren[child][0] )
				ldChildren.append( nAttr.create( cName, cShortName, lightChildren[child][0] ) )
			except Exception, e:
				jsonDebug( 'Missing data for sub-attribute %s : %s' % (child, str(e)) )
				missingNames.append( child )

		if len(missingNames) > 0:
			self.reportError( 'Not all required subattribute names are present. Add %s' % str(missingNames) )

		# Now make the master attribute
		jsonDebug( 'Creating master light data attribute' )
		attr = ldAttr.create( name, shortName,
							  ldChildren[0], ldChildren[1], ldChildren[2],
							  ldChildren[3], ldChildren[4], ldChildren[5],
							  ldChildren[6], ldChildren[7] )
		jsonDebug( 'Setting master light data defaults' )
		ldAttr.default = defaultValues

		return attr

	#----------------------------------------------------------------------
	def parseMessageAttribute(self, name, shortName, attrInfo):
		"""
		Given a JSON subsection describing a message attribute create the
		attribute and set all of the provided flags/members for it.
			name = Attribute long name
			shortName = Attribute short name
			attrInfo = JSON object containing the main attribute information
		"""
		jsonDebug( 'parseMessageAttribute(%s)' % name )
		mAttr = omAPI.MFnMessageAttribute()

		jsonDebug( 'Creating message attribute' )
		attr = mAttr.create( name, shortName )

		return attr

	#----------------------------------------------------------------------
	def parseStringAttribute(self, name, shortName, attrInfo):
		"""
		Given a JSON subsection describing a string attribute create the
		attribute and set all of the provided flags/members for it.
			name = Attribute long name
			shortName = Attribute short name
			attrInfo = JSON object containing the main attribute information
		"""
		jsonDebug( 'parseStringAttribute(%s)' % name )
		sAttr = omAPI.MFnTypedAttribute()

		if JsonKeys.kKeyDefault in attrInfo:
			jsonDebug( 'Setting the string default to "%s"' % attrInfo[JsonKeys.kKeyDefault] )
			sDefault = omAPI.MFnStringData()
			defaultValue = sDefault.create( attrInfo[JsonKeys.kKeyDefault] )
			attr = sAttr.create( name, shortName, omAPI.MFnData.kString, defaultValue )
		else:
			jsonDebug( 'Creating string attribute with no default' )
			attr = sAttr.create( name, shortName, omAPI.MFnData.kString )

		return attr

	#----------------------------------------------------------------------
	def parseMatrixAttribute(self, name, shortName, attrInfo):
		"""
		Given a JSON subsection describing a matrix attribute create the
		attribute and set all of the provided flags/members for it.
			name = Attribute long name
			shortName = Attribute short name
			attrInfo = JSON object containing the main attribute information
		"""
		jsonDebug( 'parseMatrixAttribute(%s)' % name )

		matrixType = JsonKeys.kTypeMatrixTypes[attrInfo[JsonKeys.kKeyAttrType]]
		if JsonKeys.kKeyDefault in attrInfo:
			jsonDebug( 'Setting the matrix default to "%s"' % attrInfo[JsonKeys.kKeyDefault] )
			mDefault = omAPI.MFnMatrixData()
			defaultValue = mDefault.create( omAPI.MMatrix(attrInfo[JsonKeys.kKeyDefault]) )
			mAttr = omAPI.MFnMatrixAttribute( defaultValue )
			attr = mAttr.create( name, shortName, matrixType )
		else:
			jsonDebug( 'Creating matrix attribute with no default' )
			mAttr = omAPI.MFnMatrixAttribute()
			attr = mAttr.create( name, shortName, matrixType )

		return attr

	#----------------------------------------------------------------------
	def parseNumericAttribute(self, name, shortName, numericType, attrInfo):
		"""
		Given a JSON subsection describing a numeric attribute create the
		attribute and set all of the provided flags/members for it.
			name = Attribute long name
			shortName = Attribute short name
			type = Numeric type
			attrInfo = JSON object containing the main attribute information
		"""
		jsonDebug( 'parseNumericAttribute(%s, type=%s)' % (name, type) )

		if numericType in [ 'angle', 'distance', 'time' ]:
			jsonDebug( '... unit attribute type being set up' )
			nAttr = omAPI.MFnUnitAttribute()
		else:
			jsonDebug( '... regular numeric attribute type being set up' )
			nAttr = omAPI.MFnNumericAttribute()

		jsonDebug( 'Creating numeric attribute' )
		attr = nAttr.create( name, shortName, JsonKeys.kNumericTypes[numericType] )
		jsonDebug( '...creation succeeded' )
		if JsonKeys.kKeyDefault in attrInfo:
			defaultValue = attrInfo[JsonKeys.kKeyDefault]
			jsonDebug( '...setting numeric default to %s - it is a %s' % (str(defaultValue), type(defaultValue)) )
			if type(defaultValue) == list:
				# Internally the array numerics insist on tuples for defaults
				jsonDebug( '...converting to tuple %s' % str(tuple(defaultValue)) )
				nAttr.default = tuple(defaultValue)
			else:
				nAttr.default = defaultValue

		jsonDebug( 'Setting range information on attribute' )

		# Parse the numeric-specific attributes
		if JsonKeys.kKeyMin in attrInfo:
			jsonDebug( '...setting minimum' )
			if type(attrInfo[JsonKeys.kKeyMin]) == list:
				# Internally the array numerics insist on tuples for values
				# but in the JSON it makes more sense to have them as a list
				jsonDebug( '...converting list %s to tuple' % attrInfo[JsonKeys.kKeyMin] )
				nAttr.setMin( tuple(attrInfo[JsonKeys.kKeyMin]) )
			else:
				jsonDebug( '...using %s as-is' % attrInfo[JsonKeys.kKeyMin] )
				nAttr.setMin( attrInfo[JsonKeys.kKeyMin] )
		#
		if JsonKeys.kKeyMax in attrInfo:
			jsonDebug( '...setting maximum' )
			if type(attrInfo[JsonKeys.kKeyMax]) == list:
				# Internally the array numerics insist on tuples for values
				# but in the JSON it makes more sense to have them as a list
				jsonDebug( '...converting list %s to tuple' % attrInfo[JsonKeys.kKeyMax] )
				nAttr.setMax( tuple(attrInfo[JsonKeys.kKeyMax]) )
			else:
				jsonDebug( '...using %s as-is' % attrInfo[JsonKeys.kKeyMax] )
				nAttr.setMax( attrInfo[JsonKeys.kKeyMax] )
		#
		if JsonKeys.kKeySoftMin in attrInfo:
			jsonDebug( '...setting soft minimum to %s' % attrInfo[JsonKeys.kKeySoftMin] )
			nAttr.setSoftMin( attrInfo[JsonKeys.kKeySoftMin] )
		#
		if JsonKeys.kKeySoftMax in attrInfo:
			jsonDebug( '...setting soft maximum to %s' % attrInfo[JsonKeys.kKeySoftMax] )
			nAttr.setSoftMax( attrInfo[JsonKeys.kKeySoftMax] )

		jsonDebug( 'Numeric attribute creation of "%s" complete' % attr )
		return attr

	#----------------------------------------------------------------------
	def parseAttribute(self, jsonInfo):
		"""
		Create an attribute using the JSON parameters to decode the structure
		and values for the attribute. If the attribute is a compound then this
		method will be recursively called so as to create the entire attribute
		tree below it.
			jsonInfo = JSON object containing the attribute's information
		Returns the newly created attribute.
		"""
		if JsonKeys.kKeyName not in jsonInfo:
			self.reportError( 'Missing attribute name' )
		self.currentAttribute = jsonInfo[JsonKeys.kKeyName]
		jsonDebug( 'parseAttribute(%s)' % str(jsonInfo) )
		attr = None

		# Short name must always be present so find or generate one now
		if JsonKeys.kKeyShortName in jsonInfo:
			shortName = jsonInfo[JsonKeys.kKeyShortName]
		else:
			shortName = self.currentAttribute
		jsonDebug( '...got short name %s' % shortName )

		#----------------------------------------
		# Create the specific type of attribute requested and handle the
		# type-specific parameters.
		#
		if JsonKeys.kKeyAttrType not in jsonInfo:
			self.reportError('Required keyword "%s" missing' % JsonKeys.kKeyAttrType)
		elif jsonInfo[JsonKeys.kKeyAttrType] in JsonKeys.kNumericTypes:
			attr = self.parseNumericAttribute( self.currentAttribute, shortName, jsonInfo[JsonKeys.kKeyAttrType], jsonInfo )
		elif jsonInfo[JsonKeys.kKeyAttrType] == JsonKeys.kTypeCompound:
			attr = self.parseCompoundAttribute( self.currentAttribute, shortName, jsonInfo )
		elif jsonInfo[JsonKeys.kKeyAttrType] == JsonKeys.kTypeEnum:
			attr = self.parseEnumAttribute( self.currentAttribute, shortName, jsonInfo )
		elif jsonInfo[JsonKeys.kKeyAttrType] == JsonKeys.kTypeString:
			attr = self.parseStringAttribute( self.currentAttribute, shortName, jsonInfo )
		elif jsonInfo[JsonKeys.kKeyAttrType] in JsonKeys.kTypeMatrix:
			attr = self.parseMatrixAttribute( self.currentAttribute, shortName, jsonInfo )
		elif jsonInfo[JsonKeys.kKeyAttrType] == JsonKeys.kTypeTyped:
			attr = self.parseTypedAttribute( self.currentAttribute, shortName, jsonInfo )
		elif jsonInfo[JsonKeys.kKeyAttrType] == JsonKeys.kTypeLightData:
			attr = self.parseLightDataAttribute( self.currentAttribute, shortName, jsonInfo )
		elif jsonInfo[JsonKeys.kKeyAttrType] == JsonKeys.kTypeMessage:
			attr = self.parseMessageAttribute( self.currentAttribute, shortName, jsonInfo )
		else:
			self.reportError( 'Unknown attribute type "%s"' % jsonInfo[JsonKeys.kKeyAttrType] )
			return None

		jsonDebug( 'Done creating attribute "%s", now setting shared parameters' % str(attr) )

		#----------------------------------------
		# Handle the parameters common to all attribute types
		#
		aBase = omAPI.MFnAttribute( attr )
		jsonDebug( '...handling common attribute flags for "%s"' % str(aBase) )

		# Handle the standard flags
		if JsonKeys.kKeyFlags in jsonInfo:
			self.parseStandardFlags( aBase, jsonInfo[JsonKeys.kKeyFlags] )

		# Look for a nice name override
		if JsonKeys.kKeyNiceName in jsonInfo:
			jsonDebug( '...Overriding nice name with "%s"' % jsonInfo[JsonKeys.kKeyNiceName] )
			aBase.setNiceNameOverride( jsonInfo[JsonKeys.kKeyNiceName] )

		# See if the attribute has been added to any categories
		if JsonKeys.kKeyCategories in jsonInfo:
			for category in jsonInfo[JsonKeys.kKeyCategories]:
				jsonDebug( '...Adding category "%s"' % category )
				aBase.addToCategory( category )
				jsonDebug( '...Done on category "%s"' % category )
		jsonDebug( '...Done the categories' )

		# See if there is any special disconnection behaviour
		if JsonKeys.kKeyDisconnect in jsonInfo:
			behavior = jsonInfo[JsonKeys.kKeyDisconnect]
			jsonDebug( '...Setting disconnect behaviour to "%s"' % behavior )
			if behavior in JsonKeys.kDisconnectBehaviors:
				aBase.disconnectBehavior = JsonKeys.kDisconnectBehaviors[behavior]
			else:
				self.reportError( 'Unknown behavior type "%s"' % behavior )

		return attr

	#----------------------------------------------------------------------
	def parseJsonPatterns(self, jsonObj):
		"""
		The workhorse method. Takes the JSON Python object and deconstructs it
		into one or more pattern descriptions and returns them.

		If any of the patterns fail to create an exception is raised.
			ValueError : When the JSON text is not valid
			ValueError : When the pattern name already exists
		"""
		patternList = []
		try:
			jsonDebug( 'parseJsonPatterns(%d patterns)' % len(jsonObj) )
			for thisPattern in jsonObj:
				jsonDebug( '...Pattern is %s' % thisPattern )
				if JsonKeys.kKeyName not in thisPattern:
					self.reportError( 'Missing pattern name' )
					continue
				self.currentPattern = thisPattern[JsonKeys.kKeyName]
				newPattern = omAPI.MAttributePattern( self.currentPattern )
				jsonDebug( '...Pattern %s has %d attributes' % (self.currentPattern, len(thisPattern["attributes"])) )
				if "attributes" not in thisPattern:
					self.reportError( 'Empty attribute list' )
					continue
				for thisAttribute in thisPattern["attributes"]:
					jsonDebug('Started parsing attribute "%s"' % str(thisAttribute))
					attr = self.parseAttribute( thisAttribute )
					jsonDebug('Completed parsing of attribute "%s"' % str(attr))
					# If the attribute creation succeeded add it to the pattern.
					# If it failed the creation code will have already reported
					# the problem with the attribute's description.
					newPattern.addRootAttr(attr)
					jsonDebug( 'Done adding the attribute to the pattern' )
				patternList.append( newPattern )
		except Exception, e:
			self.reportError( e )
		return patternList

	#----------------------------------------------------------------------
	def createPatternsFromString(self, definition):
		"""
		Decode the input string from JSON format and create a set of
		patterns each containing the set of root attributes described in
		that pattern's data.
		"""
		jsonDebug( 'createPatternsFromString(%d chars, %d lines)' % (len(definition), definition.count('\n')) )
		parsedPattern = None
		try:
			jsonPattern = json.loads( definition )
			jsonDebug( 'Created pattern %s' % str(jsonPattern) )
			parsedPattern = self.parseJsonPatterns( jsonPattern )
		except Exception, e:
			self.reportError( e )
		return parsedPattern

	#----------------------------------------------------------------------
	def createPatternsFromFile(self, fileName):
		"""
		Decode the input file contents from JSON format and create a set of
		patterns each containing the set of root attributes described in
		that pattern's data.

		"""
		jsonDebug( 'createPatternsFromFile(%s)' % fileName )
		parsedPattern = None
		try:
			fd = open(fileName, 'r')
			definition = ""
			for line in fd:
				definition += line
			fd.close()
			parsedPattern = self.createPatternsFromString( definition )
		except Exception, e:
			self.reportError( e )
		return parsedPattern

	#----------------------------------------------------------------------
	def name(self):
		"""
		Get the name of this pattern type.
		"""
		return JsonKeys.kPluginPatternFactoryName


#======================================================================
# Initialize the plug-in
def initializePlugin(plugin):
	import_helpers()
	pluginFn = omAPI.MFnPlugin(plugin)
	try:
		pluginFn.registerAttributePatternFactory(
			JsonKeys.kPluginPatternFactoryName, PyJsonAttrPatternFactory.patternFactoryCreator
		)
	except:
		sys.stderr.write(
			"Failed to register attribute pattern factory: %s\n" % JsonKeys.kPluginPatternFactoryName
		)
		raise

#======================================================================
# Uninitialize the plug-in
def uninitializePlugin(plugin):
	pluginFn = omAPI.MFnPlugin(plugin)
	try:
		pluginFn.deregisterAttributePatternFactory(JsonKeys.kPluginPatternFactoryName)
	except:
		sys.stderr.write(
			"Failed to unregister command: %s\n" % JsonKeys.kPluginPatternFactoryName
		)
		raise

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

