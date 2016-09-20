#-
# ===========================================================================
# Copyright 2015 Autodesk, Inc.  All rights reserved.
#
# Use of this software is subject to the terms of the Autodesk license
# agreement provided at the time of installation or download, or which
# otherwise accompanies this software in either electronic or hard copy form.
# ===========================================================================
#+

from string import *
import maya.api.OpenMaya as om
import maya.api.OpenMayaRender as omr

def maya_useNewAPI():
	"""
	The presence of this function tells Maya that the plugin produces, and
	expects to be passed, objects created using the Maya Python API 2.0.
	"""
	pass

##################################################################
## Plugin Depth Shader Class Declaration
##################################################################
class depthShader(om.MPxNode):
	# Id tag for use with binary file format
	id = om.MTypeId( 0x81002 )

	# Input attributes
	aColorNear = None
	aColorFar = None
	aNear = None
	aFar = None
	aPointCamera = None

	# Output attributes
	aOutColor = None

	@staticmethod
	def creator():
		return depthShader()

	@staticmethod
	def initialize():
		nAttr = om.MFnNumericAttribute()

		# Create input attributes

		depthShader.aColorNear = nAttr.createColor("color", "c")
		nAttr.keyable = True
		nAttr.storable = True
		nAttr.readable = True
		nAttr.writable = True
		nAttr.default = (0.0, 1.0, 0.0)			# Green


		depthShader.aColorFar = nAttr.createColor("colorFar", "cf")
		nAttr.keyable = True
		nAttr.storable = True
		nAttr.readable = True
		nAttr.writable = True
		nAttr.default = (0.0, 0.0, 1.0)			# Blue

		depthShader.aNear = nAttr.create("near", "n", om.MFnNumericData.kFloat)
		nAttr.keyable = True
		nAttr.storable = True
		nAttr.readable = True
		nAttr.writable = True
		nAttr.setMin(0.0)
		nAttr.setSoftMax(1000.0)

		depthShader.aFar = nAttr.create("far", "f", om.MFnNumericData.kFloat)
		nAttr.keyable = True
		nAttr.storable = True
		nAttr.readable = True
		nAttr.writable = True
		nAttr.setMin(0.0)
		nAttr.setSoftMax(1000.0)
		nAttr.default = 2.0

		depthShader.aPointCamera = nAttr.createPoint("pointCamera", "p")
		nAttr.keyable = True
		nAttr.storable = True
		nAttr.readable = True
		nAttr.writable = True
		nAttr.hidden = True

		# Create output attributes
		depthShader.aOutColor = nAttr.createColor("outColor", "oc")
		nAttr.keyable = False
		nAttr.storable = False
		nAttr.readable = True
		nAttr.writable = False

		om.MPxNode.addAttribute(depthShader.aColorNear)
		om.MPxNode.addAttribute(depthShader.aColorFar)
		om.MPxNode.addAttribute(depthShader.aNear)
		om.MPxNode.addAttribute(depthShader.aFar)
		om.MPxNode.addAttribute(depthShader.aPointCamera)
		om.MPxNode.addAttribute(depthShader.aOutColor)

		om.MPxNode.attributeAffects(depthShader.aColorNear, depthShader.aOutColor)
		om.MPxNode.attributeAffects(depthShader.aColorFar, depthShader.aOutColor)
		om.MPxNode.attributeAffects(depthShader.aNear, depthShader.aOutColor)
		om.MPxNode.attributeAffects(depthShader.aFar, depthShader.aOutColor)
		om.MPxNode.attributeAffects(depthShader.aPointCamera, depthShader.aOutColor)

	def __init__(self):
		om.MPxNode.__init__(self)

	def compute(self, plug, block):
		# outColor or individial R, G, B channel
		if (plug != depthShader.aOutColor) and (plug.parent() != depthShader.aOutColor):
			return None # let Maya handle this attribute

		# get sample surface shading parameters
		pCamera   = block.inputValue(depthShader.aPointCamera).asFloatVector()
		cNear     = block.inputValue(depthShader.aColorNear).asFloatVector()
		cFar      = block.inputValue(depthShader.aColorFar).asFloatVector()
		nearClip  = block.inputValue(depthShader.aNear).asFloat()
		farClip   = block.inputValue(depthShader.aFar).asFloat()

		# pCamera.z is negative
		ratio = 1.0
		dist = farClip - nearClip
		if dist != 0:
			ratio = (farClip + pCamera.z) / dist
		resultColor = cNear * ratio + cFar*(1.0 - ratio)

		# set ouput color attribute
		outColorHandle = block.outputValue( depthShader.aOutColor )
		outColorHandle.setMFloatVector( resultColor )
		outColorHandle.setClean()

	def postConstructor(self):
		self.setMPSafe(True)

##################################################################
## Plugin Depth Shader Override Class Declaration
##################################################################
class depthShaderOverride(omr.MPxSurfaceShadingNodeOverride):
	@staticmethod
	def creator(obj):
		return depthShaderOverride(obj)

	def __init__(self, obj):
		omr.MPxSurfaceShadingNodeOverride.__init__(self, obj)

		# Register fragments with the manager if needed
		fragmentMgr = omr.MRenderer.getFragmentManager()
		if fragmentMgr != None:
			if not fragmentMgr.hasFragment("depthShaderPluginFragment"):
				fragmentBody  = "<fragment uiName=\"depthShaderPluginFragment\" name=\"depthShaderPluginFragment\" type=\"plumbing\" class=\"ShadeFragment\" version=\"1.0\">"
				fragmentBody += "	<description><![CDATA[Depth shader fragment]]></description>"
				fragmentBody += "	<properties>"
				fragmentBody += "		<float name=\"depthValue\" />"
				fragmentBody += "		<float3 name=\"color\" />"
				fragmentBody += "		<float3 name=\"colorFar\" />"
				fragmentBody += "		<float name=\"near\" />"
				fragmentBody += "		<float name=\"far\" />"
				fragmentBody += "	</properties>"
				fragmentBody += "	<values>"
				fragmentBody += "		<float name=\"depthValue\" value=\"0.0\" />"
				fragmentBody += "		<float3 name=\"color\" value=\"0.0,1.0,0.0\" />"
				fragmentBody += "		<float3 name=\"colorFar\" value=\"0.0,0.0,1.0\" />"
				fragmentBody += "		<float name=\"near\" value=\"0.0\" />"
				fragmentBody += "		<float name=\"far\" value=\"2.0\" />"
				fragmentBody += "	</values>"
				fragmentBody += "	<outputs>"
				fragmentBody += "		<float3 name=\"outColor\" />"
				fragmentBody += "	</outputs>"
				fragmentBody += "	<implementation>"
				fragmentBody += "	<implementation render=\"OGSRenderer\" language=\"Cg\" lang_version=\"2.1\">"
				fragmentBody += "		<function_name val=\"depthShaderPluginFragment\" />"
				fragmentBody += "		<source><![CDATA["
				fragmentBody += "float3 depthShaderPluginFragment(float depthValue, float3 cNear, float3 cFar, float nearClip, float farClip) \n"
				fragmentBody += "{ \n"
				fragmentBody += "	float ratio = (farClip + depthValue)/(farClip - nearClip); \n"
				fragmentBody += "	return cNear*ratio + cFar*(1.0f - ratio); \n"
				fragmentBody += "} \n]]>"
				fragmentBody += "		</source>"
				fragmentBody += "	</implementation>"
				fragmentBody += "	<implementation render=\"OGSRenderer\" language=\"HLSL\" lang_version=\"11.0\">"
				fragmentBody += "		<function_name val=\"depthShaderPluginFragment\" />"
				fragmentBody += "		<source><![CDATA["
				fragmentBody += "float3 depthShaderPluginFragment(float depthValue, float3 cNear, float3 cFar, float nearClip, float farClip) \n"
				fragmentBody += "{ \n"
				fragmentBody += "	float ratio = (farClip + depthValue)/(farClip - nearClip); \n"
				fragmentBody += "	return cNear*ratio + cFar*(1.0f - ratio); \n"
				fragmentBody += "} \n]]>"
				fragmentBody += "		</source>"
				fragmentBody += "	</implementation>"
				fragmentBody += "	</implementation>"
				fragmentBody += "</fragment>"

				fragmentMgr.addShadeFragmentFromBuffer(fragmentBody, False)

			if not fragmentMgr.hasFragment("depthShaderPluginInterpolantFragment"):
				vertexFragmentBody  = "<fragment uiName=\"depthShaderPluginInterpolantFragment\" name=\"depthShaderPluginInterpolantFragment\" type=\"interpolant\" class=\"ShadeFragment\" version=\"1.0\">"
				vertexFragmentBody += "	<description><![CDATA[Depth shader vertex fragment]]></description>"
				vertexFragmentBody += "	<properties>"
				vertexFragmentBody += "		<float3 name=\"Pm\" semantic=\"Pm\" flags=\"varyingInputParam\" />"
				vertexFragmentBody += "		<float4x4 name=\"worldViewProj\" semantic=\"worldviewprojection\" />"
				vertexFragmentBody += "	</properties>"
				vertexFragmentBody += "	<values>"
				vertexFragmentBody += "	</values>"
				vertexFragmentBody += "	<outputs>"
				vertexFragmentBody += "		<float name=\"outDepthValue\" ^1s/>"
				vertexFragmentBody += "	</outputs>"
				vertexFragmentBody += "	<implementation>"
				vertexFragmentBody += "	<implementation render=\"OGSRenderer\" language=\"Cg\" lang_version=\"2.1\">"
				vertexFragmentBody += "		<function_name val=\"depthShaderPluginInterpolantFragment\" />"
				vertexFragmentBody += "		<source><![CDATA["
				vertexFragmentBody += "float depthShaderPluginInterpolantFragment(float depthValue) \n"
				vertexFragmentBody += "{ \n"
				vertexFragmentBody += "	return depthValue; \n"
				vertexFragmentBody += "} \n]]>"
				vertexFragmentBody += "		</source>"
				vertexFragmentBody += "		<vertex_source><![CDATA["
				vertexFragmentBody += "float idepthShaderPluginInterpolantFragment(float3 Pm, float4x4 worldViewProj) \n"
				vertexFragmentBody += "{ \n"
				vertexFragmentBody += "	float4 pCamera = mul(worldViewProj, float4(Pm, 1.0f)); \n"
				vertexFragmentBody += "	return (pCamera.z - pCamera.w*2.0f); \n"
				vertexFragmentBody += "} \n]]>"
				vertexFragmentBody += "		</vertex_source>"
				vertexFragmentBody += "	</implementation>"
				vertexFragmentBody += "	<implementation render=\"OGSRenderer\" language=\"HLSL\" lang_version=\"11.0\">"
				vertexFragmentBody += "		<function_name val=\"depthShaderPluginInterpolantFragment\" />"
				vertexFragmentBody += "		<source><![CDATA["
				vertexFragmentBody += "float depthShaderPluginInterpolantFragment(float depthValue) \n"
				vertexFragmentBody += "{ \n"
				vertexFragmentBody += "	return depthValue; \n"
				vertexFragmentBody += "} \n]]>"
				vertexFragmentBody += "		</source>"
				vertexFragmentBody += "		<vertex_source><![CDATA["
				vertexFragmentBody += "float idepthShaderPluginInterpolantFragment(float3 Pm, float4x4 worldViewProj) \n"
				vertexFragmentBody += "{ \n"
				vertexFragmentBody += "	float4 pCamera = mul(float4(Pm, 1.0f), worldViewProj); \n"
				vertexFragmentBody += "	return (pCamera.z - pCamera.w*2.0f); \n"
				vertexFragmentBody += "} \n]]>"
				vertexFragmentBody += "		</vertex_source>"
				vertexFragmentBody += "	</implementation>"
				vertexFragmentBody += "	</implementation>"
				vertexFragmentBody += "</fragment>"

				# In DirectX, need to specify a semantic for the output of the vertex shader
				if omr.MRenderer.drawAPI() == omr.MRenderer.kDirectX11:
					vertexFragmentBody = replace(vertexFragmentBody, "^1s", "semantic=\"extraDepth\" ")
				else:
					vertexFragmentBody = replace(vertexFragmentBody, "^1s", " ")

				fragmentMgr.addShadeFragmentFromBuffer(vertexFragmentBody, False)

			if not fragmentMgr.hasFragment("depthShaderPluginGraph"):
				fragmentGraphBody  = "<fragment_graph name=\"depthShaderPluginGraph\" ref=\"depthShaderPluginGraph\" class=\"FragmentGraph\" version=\"1.0\">"
				fragmentGraphBody += "	<fragments>"
				fragmentGraphBody += "			<fragment_ref name=\"depthShaderPluginFragment\" ref=\"depthShaderPluginFragment\" />"
				fragmentGraphBody += "			<fragment_ref name=\"depthShaderPluginInterpolantFragment\" ref=\"depthShaderPluginInterpolantFragment\" />"
				fragmentGraphBody += "	</fragments>"
				fragmentGraphBody += "	<connections>"
				fragmentGraphBody += "		<connect from=\"depthShaderPluginInterpolantFragment.outDepthValue\" to=\"depthShaderPluginFragment.depthValue\" />"
				fragmentGraphBody += "	</connections>"
				fragmentGraphBody += "	<properties>"
				fragmentGraphBody += "		<float3 name=\"Pm\" ref=\"depthShaderPluginInterpolantFragment.Pm\" semantic=\"Pm\" flags=\"varyingInputParam\" />"
				fragmentGraphBody += "		<float4x4 name=\"worldViewProj\" ref=\"depthShaderPluginInterpolantFragment.worldViewProj\" semantic=\"worldviewprojection\" />"
				fragmentGraphBody += "		<float3 name=\"color\" ref=\"depthShaderPluginFragment.color\" />"
				fragmentGraphBody += "		<float3 name=\"colorFar\" ref=\"depthShaderPluginFragment.colorFar\" />"
				fragmentGraphBody += "		<float name=\"near\" ref=\"depthShaderPluginFragment.near\" />"
				fragmentGraphBody += "		<float name=\"far\" ref=\"depthShaderPluginFragment.far\" />"
				fragmentGraphBody += "	</properties>"
				fragmentGraphBody += "	<values>"
				fragmentGraphBody += "		<float3 name=\"color\" value=\"0.0,1.0,0.0\" />"
				fragmentGraphBody += "		<float3 name=\"colorFar\" value=\"0.0,0.0,1.0\" />"
				fragmentGraphBody += "		<float name=\"near\" value=\"0.0\" />"
				fragmentGraphBody += "		<float name=\"far\" value=\"2.0\" />"
				fragmentGraphBody += "	</values>"
				fragmentGraphBody += "	<outputs>"
				fragmentGraphBody += "		<float3 name=\"outColor\" ref=\"depthShaderPluginFragment.outColor\" />"
				fragmentGraphBody += "	</outputs>"
				fragmentGraphBody += "</fragment_graph>"

				fragmentMgr.addFragmentGraphFromBuffer(fragmentGraphBody)

	def supportedDrawAPIs(self):
		return omr.MRenderer.kOpenGL | omr.MRenderer.kDirectX11

	def fragmentName(self):
		return "depthShaderPluginGraph"

##
## Plugin setup
#######################################################
sRegistrantId = "depthShaderPlugin"

def initializePlugin(obj):
	plugin = om.MFnPlugin(obj, "Autodesk", "4.5", "Any")
	try:
		userClassify = "shader/surface:drawdb/shader/surface/depthShader"
		plugin.registerNode("depthShader", depthShader.id, depthShader.creator, depthShader.initialize, om.MPxNode.kDependNode, userClassify)
	except:
		sys.stderr.write("Failed to register node\n")
		raise

	try:
		global sRegistrantId
		omr.MDrawRegistry.registerSurfaceShadingNodeOverrideCreator("drawdb/shader/surface/depthShader", sRegistrantId, depthShaderOverride.creator)
	except:
		sys.stderr.write("Failed to register override\n")
		raise

def uninitializePlugin(obj):
	plugin = om.MFnPlugin(obj)
	try:
		plugin.deregisterNode(depthShader.id)
	except:
		sys.stderr.write("Failed to deregister node\n")
		raise

	try:
		global sRegistrantId
		omr.MDrawRegistry.deregisterSurfaceShadingNodeOverrideCreator("drawdb/shader/surface/depthShader", sRegistrantId)
	except:
		sys.stderr.write("Failed to deregister override\n")
		raise

