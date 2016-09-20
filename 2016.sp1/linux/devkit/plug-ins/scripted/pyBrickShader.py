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
import math
import maya.api.OpenMaya as om
import maya.api.OpenMayaRender as omr

def maya_useNewAPI():
	"""
	The presence of this function tells Maya that the plugin produces, and
	expects to be passed, objects created using the Maya Python API 2.0.
	"""
	pass

def step(t, c):
	if t < c:
		return 0.0
	return 1.0

def smoothstep(t, a, b):
	if t < a:
		return 0.0
	if t > b:
		return 1.0
	t = (t - a)/(b - a)
	return t*t*(3.0 - 2.0*t)

def linearstep(t, a, b):
	if t < a:
		return 0.0
	if t > b:
		return 1.0
	return (t - a)/(b - a)

##
## Node declaration
#######################################################
class brickTextureNode(om.MPxNode):
	# Id tag for use with binary file format
	id = om.MTypeId( 0x8100d )

	# Input attributes
	aColor1 = None
	aColor2 = None
	aBlurFactor = None
	aUVCoord = None
	aFilterSize = None

	# Output attributes
	aOutColor = None

	@staticmethod
	def creator():
		return brickTextureNode()

	@staticmethod
	def initialize():
		nAttr = om.MFnNumericAttribute()

		# Input attributes

		brickTextureNode.aColor1 = nAttr.createColor("brickColor", "bc")
		nAttr.keyable = True
		nAttr.storable = True
		nAttr.readable = True
		nAttr.writable = True
		nAttr.default = (.75, .3, .1) 			# Brown

		brickTextureNode.aColor2 = nAttr.createColor("jointColor", "jc")
		nAttr.keyable = True
		nAttr.storable = True
		nAttr.readable = True
		nAttr.writable = True
		nAttr.default = (.75, .75, .75) 		# Grey

		brickTextureNode.aBlurFactor = nAttr.create( "blurFactor", "bf", om.MFnNumericData.kFloat)
		nAttr.keyable = True
		nAttr.storable = True
		nAttr.readable = True
		nAttr.writable = True

		# Implicit shading network attributes

		child1 = nAttr.create( "uCoord", "u", om.MFnNumericData.kFloat)
		child2 = nAttr.create( "vCoord", "v", om.MFnNumericData.kFloat)
		brickTextureNode.aUVCoord = nAttr.create( "uvCoord", "uv", child1, child2)
		nAttr.keyable = True
		nAttr.storable = True
		nAttr.readable = True
		nAttr.writable = True
		nAttr.hidden = True

		child1 = nAttr.create( "uvFilterSizeX", "fsx", om.MFnNumericData.kFloat)
		child2 = nAttr.create( "uvFilterSizeY", "fsy", om.MFnNumericData.kFloat)
		brickTextureNode.aFilterSize = nAttr.create("uvFilterSize", "fs", child1, child2)
		nAttr.keyable = True
		nAttr.storable = True
		nAttr.readable = True
		nAttr.writable = True
		nAttr.hidden = True

		# Output attributes
		brickTextureNode.aOutColor = nAttr.createColor("outColor", "oc")
		nAttr.keyable = False
		nAttr.storable = False
		nAttr.readable = True
		nAttr.writable = False

		# Add attributes to the node database.
		om.MPxNode.addAttribute(brickTextureNode.aColor1)
		om.MPxNode.addAttribute(brickTextureNode.aColor2)
		om.MPxNode.addAttribute(brickTextureNode.aBlurFactor)
		om.MPxNode.addAttribute(brickTextureNode.aFilterSize)
		om.MPxNode.addAttribute(brickTextureNode.aUVCoord)

		om.MPxNode.addAttribute(brickTextureNode.aOutColor)

		# All input affect the output color
		om.MPxNode.attributeAffects(brickTextureNode.aColor1,  		brickTextureNode.aOutColor)
		om.MPxNode.attributeAffects(brickTextureNode.aColor2,  		brickTextureNode.aOutColor)
		om.MPxNode.attributeAffects(brickTextureNode.aBlurFactor,  	brickTextureNode.aOutColor)
		om.MPxNode.attributeAffects(brickTextureNode.aFilterSize,	brickTextureNode.aOutColor)
		om.MPxNode.attributeAffects(brickTextureNode.aUVCoord,		brickTextureNode.aOutColor)

	def __init__(self):
		om.MPxNode.__init__(self)

	#######################################################
	## DESCRIPTION:
	## This function gets called by Maya to evaluate the texture.
	##
	## Get color1 and color2 from the input block.
	## Get UV coordinates from the input block.
	## Compute the color of our brick for a given UV coordinate.
	## Put the result into the output plug.
	#######################################################
	def compute(self, plug, block):
		# outColor or individial R, G, B channel
		if (plug != brickTextureNode.aOutColor) and (plug.parent() != brickTextureNode.aOutColor):
			return None # let Maya handle this attribute

		uv = block.inputValue( brickTextureNode.aUVCoord ).asFloat2()
		surfaceColor1 = block.inputValue( brickTextureNode.aColor1 ).asFloatVector()
		surfaceColor2 = block.inputValue( brickTextureNode.aColor2 ).asFloatVector()

		# normalize the UV coords
		uv[0] -= math.floor(uv[0])
		uv[1] -= math.floor(uv[1])

		borderWidth = 0.1
		brickHeight = 0.4
		brickWidth  = 0.9
		blur = block.inputValue( brickTextureNode.aBlurFactor ).asFloat()

		v1 = borderWidth/2
		v2 = v1 + brickHeight
		v3 = v2 + borderWidth
		v4 = v3 + brickHeight
		u1 = borderWidth/2
		u2 = brickWidth/2
		u3 = u2 + borderWidth
		u4 = u1 + brickWidth

		fs = block.inputValue( brickTextureNode.aFilterSize ).asFloat2()
		du = blur*fs[0]/2.0
		dv = blur*fs[1]/2.0

		t = max(min(linearstep(uv[1], v1 - dv, v1 + dv) - linearstep(uv[1], v2 - dv, v2 + dv), max(linearstep(uv[0], u3 - du, u3 + du), 1 - linearstep(uv[0], u2 - du, u2 + du))), min(linearstep(uv[1], v3 - dv, v3 + dv) - linearstep(uv[1], v4 - dv, v4 + dv), linearstep(uv[0], u1 - du, u1 + du) - linearstep(uv[0], u4 - du, u4 + du)))

		resultColor = t*surfaceColor1 + (1.0 - t)*surfaceColor2

		# set ouput color attribute
		outColorHandle = block.outputValue( brickTextureNode.aOutColor )
		outColorHandle.setMFloatVector( resultColor )
		outColorHandle.setClean()

	def postConstructor(self):
		self.setMPSafe(True)

##
## Override declaration
#######################################################
class brickTextureNodeOverride(omr.MPxShadingNodeOverride):
	@staticmethod
	def creator(obj):
		return brickTextureNodeOverride(obj)

	def __init__(self, obj):
		omr.MPxShadingNodeOverride.__init__(self, obj)

		# Register fragments with the manager if needed
		fragmentMgr = omr.MRenderer.getFragmentManager()
		if fragmentMgr != None:
			if not fragmentMgr.hasFragment("brickTextureNodePluginFragment"):
				fragmentBody = "<fragment uiName=\"brickTextureNodePluginFragment\" name=\"brickTextureNodePluginFragment\" type=\"plumbing\" class=\"ShadeFragment\" version=\"1.0\">"
				fragmentBody += "	<description><![CDATA[Brick procedural texture fragment]]></description>"
				fragmentBody += "	<properties>"
				fragmentBody += "		<float3 name=\"brickColor\" />"
				fragmentBody += "		<float3 name=\"jointColor\" />"
				fragmentBody += "		<float name=\"blurFactor\" />"
				fragmentBody += "		<float2 name=\"uvCoord\" semantic=\"mayaUvCoordSemantic\" flags=\"varyingInputParam\" />"
				fragmentBody += "		<float2 name=\"uvFilterSize\" />"
				fragmentBody += "	</properties>"
				fragmentBody += "	<values>"
				fragmentBody += "		<float3 name=\"brickColor\" value=\"0.75,0.3,0.1\" />"
				fragmentBody += "		<float3 name=\"jointColor\" value=\"0.75,0.75,0.75\" />"
				fragmentBody += "	</values>"
				fragmentBody += "	<outputs>"
				fragmentBody += "		<float3 name=\"outColor\" />"
				fragmentBody += "	</outputs>"
				fragmentBody += "	<implementation>"
				fragmentBody += "	<implementation render=\"OGSRenderer\" language=\"Cg\" lang_version=\"2.1\">"
				fragmentBody += "		<function_name val=\"brickTextureNodePluginFragment\" />"
				fragmentBody += "		<source><![CDATA["
				fragmentBody += "float btnplinearstep(float t, float a, float b) \n"
				fragmentBody += "{ \n"
				fragmentBody += "	if (t < a) return 0.0f; \n"
				fragmentBody += "	if (t > b) return 1.0f; \n"
				fragmentBody += "	return (t - a)/(b - a); \n"
				fragmentBody += "} \n"
				fragmentBody += "float3 brickTextureNodePluginFragment(float3 color1, float3 color2, float blur, float2 uv, float2 fs) \n"
				fragmentBody += "{ \n"
				fragmentBody += "	uv -= floor(uv); \n"
				fragmentBody += "	float v1 = 0.05f; float v2 = 0.45f; float v3 = 0.55f; float v4 = 0.95f; \n"
				fragmentBody += "	float u1 = 0.05f; float u2 = 0.45f; float u3 = 0.55f; float u4 = 0.95f; \n"
				fragmentBody += "	float du = blur*fs.x/2.0f; \n"
				fragmentBody += "	float dv = blur*fs.y/2.0f; \n"
				fragmentBody += "	float t = max( \n"
				fragmentBody += "		min(btnplinearstep(uv.y, v1 - dv, v1 + dv) - btnplinearstep(uv.y, v2 - dv, v2 + dv), \n"
				fragmentBody += "			max(btnplinearstep(uv.x, u3 - du, u3 + du), 1.0f - btnplinearstep(uv.x, u2 - du, u2 + du))), \n"
				fragmentBody += "		min(btnplinearstep(uv.y, v3 - dv, v3 + dv) - btnplinearstep(uv.y, v4 - dv, v4 + dv), \n"
				fragmentBody += "			btnplinearstep(uv.x, u1 - du, u1 + du) - btnplinearstep(uv.x, u4 - du, u4 + du))); \n"
				fragmentBody += "	return t*color1 + (1.0f - t)*color2; \n"
				fragmentBody += "} \n]]>"
				fragmentBody += "		</source>"
				fragmentBody += "	</implementation>"
				fragmentBody += "	<implementation render=\"OGSRenderer\" language=\"HLSL\" lang_version=\"11.0\">"
				fragmentBody += "		<function_name val=\"brickTextureNodePluginFragment\" />"
				fragmentBody += "		<source><![CDATA["
				fragmentBody += "float btnplinearstep(float t, float a, float b) \n"
				fragmentBody += "{ \n"
				fragmentBody += "	if (t < a) return 0.0f; \n"
				fragmentBody += "	if (t > b) return 1.0f; \n"
				fragmentBody += "	return (t - a)/(b - a); \n"
				fragmentBody += "} \n"
				fragmentBody += "float3 brickTextureNodePluginFragment(float3 color1, float3 color2, float blur, float2 uv, float2 fs) \n"
				fragmentBody += "{ \n"
				fragmentBody += "	uv -= floor(uv); \n"
				fragmentBody += "	float v1 = 0.05f; float v2 = 0.45f; float v3 = 0.55f; float v4 = 0.95f; \n"
				fragmentBody += "	float u1 = 0.05f; float u2 = 0.45f; float u3 = 0.55f; float u4 = 0.95f; \n"
				fragmentBody += "	float du = blur*fs.x/2.0f; \n"
				fragmentBody += "	float dv = blur*fs.y/2.0f; \n"
				fragmentBody += "	float t = max( \n"
				fragmentBody += "		min(btnplinearstep(uv.y, v1 - dv, v1 + dv) - btnplinearstep(uv.y, v2 - dv, v2 + dv), \n"
				fragmentBody += "			max(btnplinearstep(uv.x, u3 - du, u3 + du), 1.0f - btnplinearstep(uv.x, u2 - du, u2 + du))), \n"
				fragmentBody += "		min(btnplinearstep(uv.y, v3 - dv, v3 + dv) - btnplinearstep(uv.y, v4 - dv, v4 + dv), \n"
				fragmentBody += "			btnplinearstep(uv.x, u1 - du, u1 + du) - btnplinearstep(uv.x, u4 - du, u4 + du))); \n"
				fragmentBody += "	return t*color1 + (1.0f - t)*color2; \n"
				fragmentBody += "} \n]]>"
				fragmentBody += "		</source>"
				fragmentBody += "	</implementation>"
				fragmentBody += "	<implementation render=\"OGSRenderer\" language=\"GLSL\" lang_version=\"3.0\">"
				fragmentBody += "		<function_name val=\"brickTextureNodePluginFragment\" />"
				fragmentBody += "		<source><![CDATA["
				fragmentBody += "float btnplinearstep(float t, float a, float b) \n"
				fragmentBody += "{ \n"
				fragmentBody += "	if (t < a) return 0.0f; \n"
				fragmentBody += "	if (t > b) return 1.0f; \n"
				fragmentBody += "	return (t - a)/(b - a); \n"
				fragmentBody += "} \n"
				fragmentBody += "vec3 brickTextureNodePluginFragment(vec3 color1, vec3 color2, float blur, vec2 uv, vec2 fs) \n"
				fragmentBody += "{ \n"
				fragmentBody += "	uv -= floor(uv); \n"
				fragmentBody += "	float v1 = 0.05f; float v2 = 0.45f; float v3 = 0.55f; float v4 = 0.95f; \n"
				fragmentBody += "	float u1 = 0.05f; float u2 = 0.45f; float u3 = 0.55f; float u4 = 0.95f; \n"
				fragmentBody += "	float du = blur*fs.x/2.0f; \n"
				fragmentBody += "	float dv = blur*fs.y/2.0f; \n"
				fragmentBody += "	float t = max( \n"
				fragmentBody += "		min(btnplinearstep(uv.y, v1 - dv, v1 + dv) - btnplinearstep(uv.y, v2 - dv, v2 + dv), \n"
				fragmentBody += "			max(btnplinearstep(uv.x, u3 - du, u3 + du), 1.0f - btnplinearstep(uv.x, u2 - du, u2 + du))), \n"
				fragmentBody += "		min(btnplinearstep(uv.y, v3 - dv, v3 + dv) - btnplinearstep(uv.y, v4 - dv, v4 + dv), \n"
				fragmentBody += "			btnplinearstep(uv.x, u1 - du, u1 + du) - btnplinearstep(uv.x, u4 - du, u4 + du))); \n"
				fragmentBody += "	return t*color1 + (1.0f - t)*color2; \n"
				fragmentBody += "} \n]]>"
				fragmentBody += "		</source>"
				fragmentBody += "	</implementation>"
				fragmentBody += "	</implementation>"
				fragmentBody += "</fragment>"

				fragmentMgr.addShadeFragmentFromBuffer(fragmentBody, False)

	def supportedDrawAPIs(self):
		return omr.MRenderer.kOpenGL | omr.MRenderer.kOpenGLCoreProfile | omr.MRenderer.kDirectX11

	def fragmentName(self):
		return "brickTextureNodePluginFragment"


##
## Plugin setup
#######################################################
sRegistrantId = "brickTexturePlugin"

def initializePlugin(obj):
	plugin = om.MFnPlugin(obj, "Autodesk", "4.5", "Any")
	try:
		userClassify = "texture/2d:drawdb/shader/texture/2d/brickTexture"
		plugin.registerNode("brickTexture", brickTextureNode.id, brickTextureNode.creator, brickTextureNode.initialize, om.MPxNode.kDependNode, userClassify)
	except:
		sys.stderr.write("Failed to register node\n")
		raise

	try:
		global sRegistrantId
		omr.MDrawRegistry.registerShadingNodeOverrideCreator("drawdb/shader/texture/2d/brickTexture", sRegistrantId, brickTextureNodeOverride.creator)
	except:
		sys.stderr.write("Failed to register override\n")
		raise

def uninitializePlugin(obj):
	plugin = om.MFnPlugin(obj)
	try:
		plugin.deregisterNode(brickTextureNode.id)
	except:
		sys.stderr.write("Failed to deregister node\n")
		raise

	try:
		global sRegistrantId
		omr.MDrawRegistry.deregisterShadingNodeOverrideCreator("drawdb/shader/texture/2d/brickTexture", sRegistrantId)
	except:
		sys.stderr.write("Failed to deregister override\n")
		raise

