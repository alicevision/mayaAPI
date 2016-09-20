#-
# Copyright 2015 Autodesk, Inc.  All rights reserved.
#
# Use of this software is subject to the terms of the Autodesk license agreement
# provided at the time of installation or download, or which otherwise
# accompanies this software in either electronic or hard copy form.
#+

import sys
from OpenGL.GL import *
import maya.api.OpenMayaRender as omr
import maya.api.OpenMayaUI as omui
import maya.api.OpenMaya as om

def maya_useNewAPI():
	"""
	The presence of this function tells Maya that the plugin produces, and
	expects to be passed, objects created using the Maya Python API 2.0.
	"""
	pass

# Enumerations to identify an operation within a list of operations.
kBackgroundBlit 				= 0
kMaya3dSceneRender				= 1		# 3d scene render to target 1
kMaya3dSceneRenderOpaque		= 2		# 3d opaque scene render to target 1
kMaya3dSceneRenderTransparent	= 3		# 3d transparent scene render to target 1
kThresholdOp					= 4		# Brightness threshold
kHorizBlurOp					= 5		# Down sample to target 2
kVertBlurOp						= 6
kBlendOp						= 7		# Blend target 1 and 2 back to target 1
kPostOperation1					= 8		# Post ops on target 1
kPostOperation2					= 9
kMaya3dSceneRenderUI			= 10	# Post ui draw to target 1
kUserOpNumber					= 11	# User op draw to target 1
kHUDBlit						= 12	# Draw HUD on top
kPresentOp						= 13	# Present
kNumberOfOps					= 14

# Helper to enumerate the target indexing
kMyColorTarget	= 0
kMyDepthTarget	= 1
kMyBlurTarget	= 2
kTargetCount	= 3

#
#	Utilty to print out lighting information from a draw context
#
def printDrawContextLightInfo(drawContext):
	# Get all the lighting information in the scene
	considerAllSceneLights = omr.MDrawContext.kFilteredIgnoreLightLimit
	lightCount = drawContext.numberOfActiveLights(considerAllSceneLights)
	if lightCount == 0:
		return

	positions = om.MFloatPointArray()
	position = om.MFloatPoint(0, 0, 0)
	direction = om.MFloatVector()
	color = om.MColor()
	positionCount = 0

	for i in range(lightCount):
		lightParam = drawContext.getLightParameterInformation( i, considerAllSceneLights )
		if not lightParam is None:
			print "\tLight " + str(i) +"\n\t{"

			for pname in lightParam.parameterList():
				ptype = lightParam.parameterType(pname)
				if ptype == omr.MLightParameterInformation.kBoolean:
					print "\t\tLight parameter " + pname + ". Bool " + str(lightParam.getParameter(pname))
				elif ptype == omr.MLightParameterInformation.kInteger:
					print "\t\tLight parameter " + pname + ". Integer " + str(lightParam.getParameter(pname))
				elif ptype == omr.MLightParameterInformation.kFloat:
					print "\t\tLight parameter " + pname + ". Float " + str(lightParam.getParameter(pname))
				elif ptype == omr.MLightParameterInformation.kFloat2:
					print "\t\tLight parameter " + pname + ". Float " + str(lightParam.getParameter(pname))
				elif ptype == omr.MLightParameterInformation.kFloat3:
					print "\t\tLight parameter " + pname + ". Float3 " + str(lightParam.getParameter(pname))
				elif ptype == omr.MLightParameterInformation.kFloat4:
					print "\t\tLight parameter " + pname + ". Float4 " + str(lightParam.getParameter(pname))
				elif ptype == omr.MLightParameterInformation.kFloat4x4Row:
					print "\t\tLight parameter " + pname + ". Float4x4Row " + str(lightParam.getParameter(pname))
				elif ptype == omr.MLightParameterInformation.kFloat4x4Col:
					print "\t\tLight parameter " + pname + ". kFloat4x4Col " + str(lightParam.getParameter(pname))
				elif ptype == omr.MLightParameterInformation.kTexture2:
					# Get shadow map as a resource handle directly in OpenGL
					print "\t\tLight texture parameter " + pname + ". OpenGL texture id = " + str(lightParam.getParameterTextureHandle(pname))
					# Similar access for DX would look something like this:
					# (ID3D11ShaderResourceView *) lightParam.getParameterTextureHandle(pname)
				elif ptype == omr.MLightParameterInformation.kSampler:
					print "\t\tLight sampler parameter " + pname + ". filter = " + str(lightParam.getParameter(pname).filter)

				# Do some discovery to map stock parameters to usable values
				# based on the semantic
				#
				semantic = lightParam.parameterSemantic(pname)
				if semantic == omr.MLightParameterInformation.kLightEnabled:
					print "\t\t- Parameter semantic : light enabled"
				elif semantic == omr.MLightParameterInformation.kWorldPosition:
					print "\t\t- Parameter semantic : world position"
					floatVals = lightParam.getParameter(pname)
					position += om.MFloatPoint( floatVals[0], floatVals[1], floatVals[2] )
					positionCount = positionCount + 1
				elif semantic == omr.MLightParameterInformation.kWorldDirection:
					print "\t\t- Parameter semantic : world direction"
					direction = lightParam.getParameter(pname)
				elif semantic == omr.MLightParameterInformation.kIntensity:
					print "\t\t- Parameter semantic : intensity"
				elif semantic == omr.MLightParameterInformation.kColor:
					print "\t\t- Parameter semantic : color"
					color = om.MColor( lightParam.getParameter(pname) )
				elif semantic == omr.MLightParameterInformation.kEmitsDiffuse:
					print "\t\t- Parameter semantic : emits-diffuse"
				elif semantic == omr.MLightParameterInformation.kEmitsSpecular:
					print "\t\t- Parameter semantic : emits-specular"
				elif semantic == omr.MLightParameterInformation.kDecayRate:
					print "\t\t- Parameter semantic : decay rate"
				elif semantic == omr.MLightParameterInformation.kDropoff:
					print "\t\t- Parameter semantic : drop-off"
				elif semantic == omr.MLightParameterInformation.kCosConeAngle:
					print "\t\t- Parameter semantic : cosine cone angle"
				elif semantic == omr.MLightParameterInformation.kShadowMap:
					print "\t\t- Parameter semantic : shadow map"
				elif semantic == omr.MLightParameterInformation.kShadowSamp:
					print "\t\t- Parameter semantic : shadow map sampler"
				elif semantic == omr.MLightParameterInformation.kShadowBias:
					print "\t\t- Parameter semantic : shadow map bias"
				elif semantic == omr.MLightParameterInformation.kShadowMapSize:
					print "\t\t- Parameter semantic : shadow map size"
				elif semantic == omr.MLightParameterInformation.kShadowViewProj:
					print "\t\t- Parameter semantic : shadow map view projection matrix"
				elif semantic == omr.MLightParameterInformation.kShadowColor:
					print "\t\t- Parameter semantic : shadow color"
				elif semantic == omr.MLightParameterInformation.kGlobalShadowOn:
					print "\t\t- Parameter semantic : global shadows on"
				elif semantic == omr.MLightParameterInformation.kShadowOn:
					print "\t\t- Parameter semantic : local shadows on"

			# Compute an average position
			if positionCount > 1:
				position /= positionCount
				print "\t\tCompute average position " + str(position)

			# Print by semantic
			print "\t\tSemantic -> Parameter Name Lookups"
			paramNames = lightParam.parameterNames(omr.MLightParameterInformation.kLightEnabled)
			floatVals = ""
			try:
				floatVals = lightParam.getParameter(omr.MLightParameterInformation.kLightEnabled)
			except RuntimeError:
				pass
			print "\t\t\tkLightEnabled -> " + str(paramNames) + " -- " + str(floatVals)
			paramNames = lightParam.parameterNames(omr.MLightParameterInformation.kWorldPosition)
			print "\t\t\tkWorldPosition -> " + str(paramNames)
			paramNames = lightParam.parameterNames(omr.MLightParameterInformation.kWorldDirection)
			print "\t\t\tkWorldDirection -> " + str(paramNames)
			paramNames = lightParam.parameterNames(omr.MLightParameterInformation.kIntensity)
			print "\t\t\tkIntensity -> " + str(paramNames)
			paramNames = lightParam.parameterNames(omr.MLightParameterInformation.kColor)
			print "\t\t\tkColor -> " + str(paramNames)
			paramNames = lightParam.parameterNames(omr.MLightParameterInformation.kEmitsDiffuse)
			print "\t\t\tkEmitsDiffuse -> " + str(paramNames)
			paramNames = lightParam.parameterNames(omr.MLightParameterInformation.kEmitsSpecular)
			print "\t\t\tkEmitsSpecular -> " + str(paramNames)
			paramNames = lightParam.parameterNames(omr.MLightParameterInformation.kDecayRate)
			print "\t\t\tkDecayRate -> " + str(paramNames)
			paramNames = lightParam.parameterNames(omr.MLightParameterInformation.kDropoff)
			print "\t\t\tkDropoff -> " + str(paramNames)
			paramNames = lightParam.parameterNames(omr.MLightParameterInformation.kCosConeAngle)
			print "\t\t\tkCosConeAngle -> " + str(paramNames)
			paramNames = lightParam.parameterNames(omr.MLightParameterInformation.kIrradianceIn)
			print "\t\t\tkIrradianceIn -> " + str(paramNames)
			paramNames = lightParam.parameterNames(omr.MLightParameterInformation.kShadowMap)
			print "\t\t\tkShadowMap -> " + str(paramNames)
			paramNames = lightParam.parameterNames(omr.MLightParameterInformation.kShadowSamp)
			print "\t\t\tkShadowSamp -> " + str(paramNames)
			paramNames = lightParam.parameterNames(omr.MLightParameterInformation.kShadowBias)
			print "\t\t\tkShadowBias -> " + str(paramNames)
			paramNames = lightParam.parameterNames(omr.MLightParameterInformation.kShadowMapSize)
			print "\t\t\tkShadowMapSize -> " + str(paramNames)
			paramNames = lightParam.parameterNames(omr.MLightParameterInformation.kShadowColor)
			print "\t\t\tkShadowColor -> " + str(paramNames)
			paramNames = lightParam.parameterNames(omr.MLightParameterInformation.kGlobalShadowOn)
			print "\t\t\tkGlobalShadowOn -> " + str(paramNames)
			paramNames = lightParam.parameterNames(omr.MLightParameterInformation.kShadowOn)
			print "\t\t\tkShadowOn -> " + str(paramNames)

			print "\t}"


# Shader override helpers:
# As part of a shader override it is possible to attach callbacks which
# are invoked when the shader is to be used. The following are some examples
# of what could be performed.
#
#	Example utility used by a callback to:
#
#	1. Print out the shader parameters for a give MShaderInsrtance
#	2. Examine the list of render items which will be rendered with this MShaderInstance
#	3. Examine the pass context and print out information in the context.
#
def callbackDataPrint(context, renderItemList, shaderInstance):
	if not shaderInstance is None:
		paramNames = shaderInstance.parameterList()
		paramCount = len(paramNames)
		print "\tSHADER: # of parameters = " + str(paramCount)
		for i in range(paramCount):
			print "\t\tPARAM[" + paramNames[i] + "]"

	numItems = len(renderItemList)
	for i in range(numItems):
		item = renderItemList[i]
		if not item is None:
			path = item.sourceDagPath()
			print "\tRENDER ITEM: '" + item.name() + "' -- SOURCE: '" + path.fullPathName() + "'"

	passCtx = context.getPassContext()
	passId = passCtx.passIdentifier()
	passSem = passCtx.passSemantics()
	print "PASS ID[" + passId + "], PASS SEMANTICS[" + str(passSem) + "]"

#
#	Example utility used by callback to bind lighting information to a shader instance.
#
#	This callback works specific with the MayaBlinnDirectionLightShadow shader example.
#	It will explicitly binding lighting and shadowing information to the shader instance.
#
def shaderOverrideCallbackBindLightingInfo(drawContext, renderItemList, shaderInstance):
	if shaderInstance is None:
		return

	# Defaults in case there are no lights
	#
	globalShadowsOn = False
	localShadowsOn = False
	direction = om.MFloatVector(0.0, 0.0, 1.0)
	lightIntensity = 0.0 # If no lights then black out the light
	lightColor = [ 0.0, 0.0, 0.0 ]

	# Scan to find the first light that has a direction component in it
	# It's possible we find no lights.
	#
	considerAllSceneLights = omr.MDrawContext.kFilteredIgnoreLightLimit
	lightCount = drawContext.numberOfActiveLights(considerAllSceneLights)
	if lightCount > 0:
		shadowViewProj = om.MMatrix()
		shadowResourceTexture = None
		samplerDesc = omr.MSamplerStateDesc()
		shadowColor = [ 0.0, 0.0, 0.0 ]

		foundDirectional = False
		for i in range(lightCount):
			if foundDirectional:
				break
			globalShadowsOn = False
			localShadowsOn = False
			direction = om.MFloatVector(0.0, 0.0, 1.0)
			lightIntensity = 0.0
			lightColor = [ 0.0, 0.0, 0.0 ]

			lightParam = drawContext.getLightParameterInformation( i, considerAllSceneLights )
			if not lightParam is None:
				for pname in lightParam.parameterList():
					semantic = lightParam.parameterSemantic( pname )
					# Pick a few light parameters to pick up as an example
					if semantic == omr.MLightParameterInformation.kWorldDirection:
						floatVals = lightParam.getParameter( pname )
						direction = om.MFloatVector( floatVals[0], floatVals[1], floatVals[2] )
						foundDirectional = True
					elif semantic == omr.MLightParameterInformation.kIntensity:
						floatVals = lightParam.getParameter( pname )
						lightIntensity = floatVals[0]
					elif semantic == omr.MLightParameterInformation.kColor:
						lightColor = lightParam.getParameter( pname )

					# Pick up shadowing parameters
					elif semantic == omr.MLightParameterInformation.kGlobalShadowOn:
						intVals = lightParam.getParameter( pname )
						if len(intVals) > 0:
							globalShadowsOn = (intVals[0] != 0)
					elif semantic == omr.MLightParameterInformation.kShadowOn:
						intVals = lightParam.getParameter( pname )
						if len(intVals) > 0:
							localShadowsOn = (intVals[0] != 0)
					elif semantic == omr.MLightParameterInformation.kShadowViewProj:
						shadowViewProj = lightParam.getParameter( pname )
					elif semantic == omr.MLightParameterInformation.kShadowMap:
						shadowResourceTexture = lightParam.getParameter( pname )
					elif semantic == omr.MLightParameterInformation.kShadowSamp:
						samplerDesc = lightParam.getParameter( pname )
					elif semantic == omr.MLightParameterInformation.kShadowColor:
						shadowColor = lightParam.getParameter( pname )

			# Set shadow map and projection if shadows are turned on.
			#
			if foundDirectional and globalShadowsOn and localShadowsOn and not shadowResourceTexture is None:
				resourceHandle = shadowResourceTexture.resourceHandle()
				if resourceHandle > 0:
					debugShadowBindings = False
					try:
						shaderInstance.setParameter("mayaShadowPCF1_shadowMap", shadowResource )
						if debugShadowBindings:
							print "Bound shadow map to shader param mayaShadowPCF1_shadowMap"
					except:
						pass
					try:
						shaderInstance.setParameter("mayaShadowPCF1_shadowViewProj", shadowViewProj )
						if debugShadowBindings:
							print "Bound shadow map transform to shader param mayaShadowPCF1_shadowViewProj"
					except:
						pass
					try:
						shaderInstance.setParameter("mayaShadowPCF1_shadowColor", shadowColor )
						if debugShadowBindings:
							print "Bound shadow map color to shader param mayaShadowPCF1_shadowColor"
					except:
						pass

				textureManager = omr.MRenderer.getTextureManager()
				if not textureManager is None:
					textureManager.releaseTexture(shadowResourceTexture)
				shadowResourceTexture = None

	# Set up parameters which should be set regardless of light existence.
	shaderInstance.setParameter("mayaDirectionalLight_direction", direction)
	shaderInstance.setParameter("mayaDirectionalLight_intensity", lightIntensity)
	shaderInstance.setParameter("mayaDirectionalLight_color", lightColor)
	shaderInstance.setParameter("mayaShadowPCF1_mayaGlobalShadowOn", globalShadowsOn)
	shaderInstance.setParameter("mayaShadowPCF1_mayaShadowOn", localShadowsOn)

#
#	Example pre-render callback attached to a shader instance
#
def shaderOverridePreDrawCallback(context, renderItemList, shaderInstance):
	print "PRE-draw callback triggered for render item list with data:"
	callbackDataPrint(context, renderItemList, shaderInstance)
	print ""

	print "\tLIGHTS"
	printDrawContextLightInfo( context )
	print ""

#
#	Example post-render callback attached to a shader instance
#
def shaderOverridePostDrawCallback(context, renderItemList, shaderInstance):
	print "POST-draw callback triggered for render item list with data:"
	callbackDataPrint(context, renderItemList, shaderInstance)
	print ""

###################################################################
#	Custom HUD operation
#
#
class viewRenderHUDOperation(omr.MHUDRender):
	def __init__(self):
		omr.MHUDRender.__init__(self)

		self.mTargets = None

	# Target override
	def targetOverrideList(self):
		if not self.mTargets is None:
			return [ self.mTargets[kMyColorTarget], self.mTargets[kMyDepthTarget] ]
		return None

	def hasUIDrawables(self):
		return True

	def addUIDrawables(self, drawManager2D, frameContext):
		# Start draw UI
		drawManager2D.beginDrawable()
		# Set font color
		drawManager2D.setColor( om.MColor( (0.455, 0.212, 0.596) ) )
		# Set font size
		drawManager2D.setFontSize( omr.MUIDrawManager.kSmallFontSize )

		# Draw renderer name
		dim = frameContext.getViewportDimensions()
		x=dim[0]
		y=dim[1]
		w=dim[2]
		h=dim[3]
		drawManager2D.text( om.MPoint(w*0.5, h*0.91), "Sample VP2 Renderer Override", omr.MUIDrawManager.kCenter )

		# Draw viewport information
		viewportInfoText = "Viewport information: x= "
		viewportInfoText += str(x)
		viewportInfoText += ", y= "
		viewportInfoText += str(y)
		viewportInfoText += ", w= "
		viewportInfoText += str(w)
		viewportInfoText += ", h= "
		viewportInfoText += str(h)
		drawManager2D.text( om.MPoint(w*0.5, h*0.885), viewportInfoText, omr.MUIDrawManager.kCenter )

		# End draw UI
		drawManager2D.endDrawable()

	def setRenderTargets(self, targets):
		self.mTargets = targets


###################################################################
#
#	Custom present target operation
#
#	Only overrides the targets to present
#
class viewRenderPresentTarget(omr.MPresentTarget):
	def __init__(self, name):
		omr.MPresentTarget.__init__(self, name)

		# Targets used as input parameters to mShaderInstance
		self.mTargets = None

	def targetOverrideList(self):
		if not self.mTargets is None:
			return [ self.mTargets[kMyColorTarget], self.mTargets[kMyDepthTarget] ]
		return None

	def setRenderTargets(self, targets):
		self.mTargets = targets


###################################################################
#	Custom quad operation
#
#	General quad operation which can be instantiated with a few
#	different shaders.
#

class viewRenderQuadRender(omr.MQuadRender):
	# Shader list
	kEffectNone				= 0
	kPost_EffectMonochrome	= 1   # Mono color shader
	kPost_EffectEdgeDetect	= 2   # Edge detect shader
	kPost_EffectInvert		= 3   # Invert color shader
	kScene_Threshold		= 4   # Color threshold shader
	kScene_BlurHoriz		= 5   # Horizontal blur shader
	kScene_BlurVert			= 6   # Vertical blur shader
	kSceneBlur_Blend		= 7   # Blend shader
	kPre_MandelBrot			= 8   # Mandelbrot shader

	def __init__(self, name):
		omr.MQuadRender.__init__(self, name)

		# Shader to use for the quad render
		self.mShaderInstance = None
		# Targets used as input parameters to mShaderInstance
		self.mTargets = None
		# View rectangle
		self.mViewRectangle = om.MFloatPoint()
		# Shader to use for quad rendering
		self.mShader = self.kEffectNone

	def __del__(self):
		if not self.mShaderInstance is None:
			shaderMgr = omr.MRenderer.getShaderManager()
			if not shaderMgr is None:
				shaderMgr.releaseShader(self.mShaderInstance)
			self.mShaderInstance = None

	# Return the appropriate shader instance based on the what
	# we want the quad operation to perform
	def shader(self):
		# Create a new shader instance for this quad render instance
		#
		if self.mShaderInstance is None:
			shaderMgr = omr.MRenderer.getShaderManager()
			if not shaderMgr is None:
				# Note in the following code that we are not specifying the
				# full file name, but relying on the getEffectFileShader() logic
				# to determine the correct file name extension based on the shading language
				# which is appropriate for the drawing API (DirectX or OpenGL).
				#
				# Refer to the documentation for this method to review how the
				# final name on disk is derived.
				#
				# The second argument here is the technique. If desired
				# and effect on disk can hold different techniques. For each unique
				# effect + technique a different shader instance is created.
				#
				if self.mShader == self.kPre_MandelBrot:
					self.mShaderInstance = shaderMgr.getEffectsFileShader( "MandelBrot", "" )
				elif self.mShader == self.kPost_EffectMonochrome:
					self.mShaderInstance = shaderMgr.getEffectsFileShader( "FilterMonochrome", "" )
				elif self.mShader == self.kPost_EffectEdgeDetect:
					self.mShaderInstance = shaderMgr.getEffectsFileShader( "FilterEdgeDetect", "" )
				elif self.mShader == self.kPost_EffectInvert:
					self.mShaderInstance = shaderMgr.getEffectsFileShader( "Invert", "" )
				elif self.mShader == self.kScene_Threshold:
					self.mShaderInstance = shaderMgr.getEffectsFileShader( "Threshold", "" )
				elif self.mShader == self.kScene_BlurHoriz:
					self.mShaderInstance = shaderMgr.getEffectsFileShader( "Blur", "BlurHoriz" )
				elif self.mShader == self.kScene_BlurVert:
					self.mShaderInstance = shaderMgr.getEffectsFileShader( "Blur", "BlurVert" )
				elif self.mShader == self.kSceneBlur_Blend:
					self.mShaderInstance = shaderMgr.getEffectsFileShader( "Blend", "Add" )

		# Set parameters on the shader instance.
		#
		# This is where the input render targets can be specified by binding
		# a render target to the appropriate parameter on the shader instance.
		#
		if not self.mShaderInstance is None:
			if self.mShader == self.kPre_MandelBrot:
				# Example of a simple float parameter setting.
				try:
					self.mShaderInstance.setParameter("gIterate", 50)
				except:
					print "Could not change mandelbrot parameter"
					return None

			elif self.mShader == self.kPost_EffectInvert:
				# Set the input texture parameter 'gInputTex' to use
				# a given color target
				try:
					self.mShaderInstance.setParameter("gInputTex", self.mTargets[kMyColorTarget])
				except:
					print "Could not set input render target / texture parameter on invert shader"
					return None

			elif self.mShader == self.kScene_Threshold:
				# Set the input texture parameter 'gSourceTex' to use
				# a given color target
				try:
					self.mShaderInstance.setParameter("gSourceTex", self.mTargets[kMyColorTarget])
				except:
					print "Could not set input render target / texture parameter on threshold shader"
					return None
				self.mShaderInstance.setParameter("gBrightThreshold", 0.7 )

			elif self.mShader == self.kScene_BlurHoriz:
				# Set the input texture parameter 'gSourceTex' to use
				# a given color target
				try:
					self.mShaderInstance.setParameter("gSourceTex", self.mTargets[kMyBlurTarget])
				except:
					print "Could not set input render target / texture parameter on hblur shader"
					return None

			elif self.mShader == self.kScene_BlurVert:
				# Set the input texture parameter 'gSourceTex' to use
				# a given color target
				try:
					self.mShaderInstance.setParameter("gSourceTex", self.mTargets[kMyBlurTarget])
				except:
					print "Could not set input render target / texture parameter on vblur shader"
					return None

			elif self.mShader == self.kSceneBlur_Blend:
				# Set the first input texture parameter 'gSourceTex' to use
				# one color target.
				try:
					self.mShaderInstance.setParameter("gSourceTex", self.mTargets[kMyColorTarget])
				except:
					return None
				# Set the second input texture parameter 'gSourceTex2' to use
				# a second color target.
				try:
					self.mShaderInstance.setParameter("gSourceTex2", self.mTargets[kMyBlurTarget])
				except:
					return None
				self.mShaderInstance.setParameter("gBlendSrc", 0.3 )

			elif self.mShader == self.kPost_EffectMonochrome:
				# Set the input texture parameter 'gInputTex' to use
				# a given color target
				try:
					self.mShaderInstance.setParameter("gInputTex", self.mTargets[kMyColorTarget])
				except:
					print "Could not set input render target / texture parameter on monochrome shader"
					return None

			elif self.mShader == self.kPost_EffectEdgeDetect:
				# Set the input texture parameter 'gInputTex' to use
				# a given color target
				try:
					self.mShaderInstance.setParameter("gInputTex", self.mTargets[kMyColorTarget])
				except:
					print "Could not set input render target / texture parameter on edge detect shader"
					return None
				self.mShaderInstance.setParameter("gThickness", 1.0 )
				self.mShaderInstance.setParameter("gThreshold", 0.1 )

		return self.mShaderInstance

	# Based on which shader is being used for the quad render
	# we want to render to different targets. For the
	# threshold and two blur shaders the temporary 'blur'
	# target is used. Otherwise rendering should be directed
	# to the custom color and depth target.
	def targetOverrideList(self):
		if not self.mTargets is None:
			# Render to blur target for blur operations
			if self.mShader == self.kScene_Threshold or self.mShader == self.kScene_BlurHoriz or self.mShader == self.kScene_BlurVert:
				return [ self.mTargets[kMyBlurTarget] ]
			else:
				return [ self.mTargets[kMyColorTarget], self.mTargets[kMyDepthTarget] ]
		return None

	# Set the clear override to use.
	def clearOperation(self):
		clearOp = self.mClearOperation
		# Want to clear everything since the quad render is the first operation.
		if self.mShader == self.kPre_MandelBrot:
			clearOp.setClearGradient( False )
			clearOp.setMask( omr.MClearOperation.kClearAll )
		# This is a post processing operation, so we don't want to clear anything
		else:
			clearOp.setClearGradient( False )
			clearOp.setMask( omr.MClearOperation.kClearNone )
		return clearOp

	def setRenderTargets(self, targets):
		self.mTargets = targets

	def setShader(self, shader):
		self.mShader = shader

	def viewRectangle(self):
		return self.mViewRectangle

	def setViewRectangle(self, rect):
		self.mViewRectangle = rect


###################################################################
#	Simple scene operation
#
#	Example of just overriding a few options on the scene render.
#
class simpleViewRenderSceneRender(omr.MSceneRender):
	def __init__(self, name):
		omr.MSceneRender.__init__(self, name)

		self.mViewRectangle = om.MFloatPoint(0.0, 0.0, 1.0, 1.0) # 100 % of target size

	def viewportRectangleOverride(self):
		# Enable this flag to use viewport sizing
		testRectangleSize = False
		if testRectangleSize:
			# 1/3 to the right and 10 % up. 1/2 the target size.
			self.mViewRectangle = om.MFloatPoint(0.33, 0.10, 0.50, 0.50)
		return self.mViewRectangle

	def clearOperation(self):
		# Override to clear to these gradient colors
		val1 = [ 0.0, 0.2, 0.8, 1.0 ]
		val2 = [ 0.5, 0.4, 0.1, 1.0 ]
		clearOp = self.mClearOperation
		clearOp.setClearColor( val1 )
		clearOp.setClearColor2( val2 )
		clearOp.setClearGradient( True )
		return clearOp


###################################################################
#	Custom scene operation
#
#	A scene render which is reused as necessary with different
#	override parameters
#
class viewRenderSceneRender(omr.MSceneRender):
	def __init__(self, name, sceneFilter, clearMask):
		omr.MSceneRender.__init__(self, name)

		self.mSelectionList = om.MSelectionList()

		# 3D viewport panel name, if available
		self.mPanelName = ""
		# Camera override
		self.mCameraOverride = omr.MCameraOverride()
		# Viewport rectangle override
		self.mViewRectangle = om.MFloatPoint(0.0, 0.0, 1.0, 1.0) # 100 % of target size
		# Available render targets
		self.mTargets = None
		# Shader override for surfaces
		self.mShaderOverride = None
		# Scene draw filter override
		self.mSceneFilter = sceneFilter
		# Mask for clear override
		self.mClearMask = clearMask

		# Some sample override flags
		self.mUseShaderOverride = False
		self.mUseStockShaderOverride = False
		self.mAttachPrePostShaderCallback = False
		self.mUseShadowShader = False
		self.mOverrideDisplayMode = True
		self.mOverrideLightingMode = False
		self.mOverrideCullingMode = False
		self.mDebugTargetResourceHandle = False
		self.mOverrrideM3dViewDisplayMode = False
		self.mPrevDisplayStyle = omui.M3dView.kGouraudShaded # Track previous display style of override set
		self.mFilterDrawNothing = False
		self.mFilterDrawSelected = False
		self.mEnableSRGBWrite = False


	def __del__(self):
		if not self.mShaderOverride is None:
			shaderMgr = omr.MRenderer.getShaderManager()
			if not shaderMgr is None:
				shaderMgr.releaseShader(self.mShaderOverride)
			self.mShaderOverride = None

	def setRenderTargets(self, targets):
		self.mTargets = targets

	def targetOverrideList(self):
		if not self.mTargets is None:
			return [ self.mTargets[kMyColorTarget], self.mTargets[kMyDepthTarget] ]
		return None
	
	# Indicate whether to enable SRGB write
	def enableSRGBWrite(self):
		return self.mEnableSRGBWrite

	# Sample of accessing the view to get a camera path and using that as
	# the camera override. Other camera paths or direct matrix setting
	def cameraOverride(self):
		if len(self.mPanelName) > 0:
			view = omui.M3dView.getM3dViewFromModelPanel(self.mPanelName)
			self.mCameraOverride.mCameraPath = view.getCamera()
			return self.mCameraOverride

		print "\t" + self.mName + ": Query custom scene camera override -- no override set"
		return None

	# Depending on what is required either the scene filter will return whether
	# to draw the opaque, transparent of non-shaded (UI) items.
	def renderFilterOverride(self):
		return self.mSceneFilter

	# Example display mode override. In this example we override so that
	# the scene will always be drawn in shaded mode and in bounding box
	# mode (bounding boxes will also be drawn). This is fact not a
	# 'regular' viewport display mode available from the viewport menus.
	def displayModeOverride(self):
		if self.mOverrideDisplayMode:
			return ( omr.MSceneRender.kBoundingBox | omr.MSceneRender.kShaded )
		return omr.MSceneRender.kNoDisplayModeOverride

	# Example Lighting mode override. In this example
	# the override would set to draw with only selected lights.
	def lightModeOverride(self):
		if self.mOverrideLightingMode:
			return omr.MSceneRender.kSelectedLights
		return omr.MSceneRender.kNoLightingModeOverride

	# Return shadow override. For the UI pass we don't want to compute shadows.
	def shadowEnableOverride(self):
		if (self.mSceneFilter & omr.MSceneRender.kRenderShadedItems) == 0:
			return False  # UI doesn't need shadows
		# For all other cases, just use whatever is currently set
		return None

	# Example culling mode override. When enable
	# this example would force to cull backfacing
	# polygons.
	def cullingOverride(self):
		if self.mOverrideCullingMode:
			return omr.MSceneRender.kCullBackFaces
		return omr.MSceneRender.kNoCullingOverride

	# Per scene operation pre-render.
	# In this example the display style for the given panel / view
	# M3dView is set to be consistent with the draw override
	# for the scene operation.
	def preRender(self):
		if self.mOverrrideM3dViewDisplayMode:
			if len(self.mPanelName) > 0:
				view = omui.M3dView.getM3dViewFromModelPanel(self.mPanelName)
				self.mPrevDisplayStyle = view.displayStyle();
				view.setDisplayStyle( omui.M3dView.kGouraudShaded )

	# Post-render example.
	# In this example we can debug the resource handle of the active render target
	# after this operation. The matching for for the pre-render M3dView override
	# also resides here to restore the M3dView state.
	def postRender(self):
		if self.mDebugTargetResourceHandle:
			# Get the id's for the textures which are used as the color and
			# depth render targets. These id's could arbitrarily change
			# so they should not be held on to.
			if not self.mTargets[kMyColorTarget] is None:
				colorResourceHandle = self.mTargets[kMyColorTarget].resourceHandle()
				print "\t - Color target resource handle = " + str(colorResourceHandle)

			if not self.mTargets[kMyDepthTarget] is None:
				depthStencilResourceHandle = self.mTargets[kMyDepthTarget].resourceHandle()
				print "\t - Depth target resource handle = " + str(depthStencilResourceHandle)

		if self.mOverrrideM3dViewDisplayMode:
			if len(self.mPanelName) > 0:
				view = omui.M3dView.getM3dViewFromModelPanel(self.mPanelName)
				view.setDisplayStyle( self.mPrevDisplayStyle )

	# Object type exclusions example.
	# In this example we want to hide cameras
	def objectTypeExclusions(self):
		# Example of hiding by type.
		return omr.MSceneRender.kExcludeCameras

	# Example scene override logic.
	# In this example, the scene to draw can be filtered by a returned
	# selection list. If an empty selection list is returned then we can
	# essentially disable scene drawing. The other option coded here
	# is to look at the current active selection list and return that.
	# This results in only rendering what has been selected by the user
	# If this filtering is required across more than one operation it
	# is better to precompute these values in the setup phase of
	# override and cache the information per operation as required.
	def objectSetOverride(self):
		self.mSelectionList.clear()

		# If you set this to True you can make the
		# scene draw draw no part of the scene, only the
		# additional UI elements
		#
		if self.mFilterDrawNothing:
			return self.mSelectionList

		# Turn this on to query the active list and only
		# use that for drawing
		#
		if self.mFilterDrawSelected:
			selList = om.MGlobal.getActiveSelectionList()
			if selList.length() > 0:
				iter = om.MItSelectionList(selList)
				while not iter.isDone():
					comp = iter.getComponent()
					self.mSelectionList.add( comp[0], comp[1] )
					iter.next()

			if self.mSelectionList.length() > 0:
				print "\t" + self.name() + " : Filtering render with active object list"
				return self.mSelectionList

		return None

	# Custom clear override.
	# Depending on whether we are drawing the "UI" or "non-UI"
	# parts of the scene we will clear different channels.
	# Color is never cleared since there is a separate operation
	# to clear the background.
	def clearOperation(self):
		clearOp = self.mClearOperation
		if (self.mSceneFilter & (omr.MSceneRender.kRenderOpaqueShadedItems | omr.MSceneRender.kRenderTransparentShadedItems | omr.MSceneRender.kRenderUIItems)) != 0:
			clearOp.setClearGradient(False)
		else:
			# Force a gradient clear with some sample colors.
			#
			val1 = [ 0.0, 0.2, 0.8, 1.0 ]
			val2 = [ 0.5, 0.4, 0.1, 1.0 ]
			clearOp.setClearColor(val1)
			clearOp.setClearColor2(val2)
			clearOp.setClearGradient(True)

		clearOp.setMask(self.mClearMask)
		return clearOp

	# Example of setting a shader override.
	# Some variations are presented based on some member flags:
	# - Use a stock shader or not
	# - Attach pre and post shader instance callbacks
	# - Use a shadow shader
	def shaderOverride(self):
		if self.mUseShaderOverride:
			if self.mShaderOverride is None:
				shaderManager = omr.MRenderer.getShaderManager()
				if not shaderManager is None:
					preCallBack = None
					postCallBack = None
					if not self.mUseStockShaderOverride:
						if self.mUseShadowShader:
							# This shader has parameters which can be updated
							# by the attached pre-callback.
							preCallBack = shaderOverrideCallbackBindLightingInfo
							self.mShaderOverride = shaderManager.getEffectsFileShader("MayaBlinnDirectionalLightShadow", "", None, True, preCallBack, postCallBack)
						else:
							# Use a sample Gooch shader
							if self.mAttachPrePostShaderCallback:
								preCallBack = shaderOverridePreDrawCallback
							if self.mAttachPrePostShaderCallback:
								postCallBack = shaderOverridePostDrawCallback
							self.mShaderOverride = shaderManager.getEffectsFileShader("Gooch", "", None, True, preCallBack, postCallBack)
					else:
						# Use a stock shader available from the shader manager
						# In this case the stock Blinn shader.
						if self.mAttachPrePostShaderCallback:
							preCallBack = shaderOverridePreDrawCallback
						if self.mAttachPrePostShaderCallback:
							postCallBack = shaderOverridePostDrawCallback
						self.mShaderOverride = shaderManager.getStockShader( omr.MShaderManager.k3dBlinnShader, preCallBack, postCallBack)

						if not self.mShaderOverride is None:
							print "\t" + self.name() + " : Set stock shader override " + str(omr.MShaderManager.k3dBlinnShader)
							diffColor = [0.0, 0.4, 1.0, 1.0]
							try:
								self.mShaderOverride.setParameter("diffuseColor", diffColor)
							except:
								print "Could not set diffuseColor on shader"

			return self.mShaderOverride

		# No override so return None
		return None

	def hasUIDrawables(self):
		return True

	# Pre UI draw
	def addPreUIDrawables(self, drawManager, frameContext):
		drawManager.beginDrawable()
		drawManager.setColor( om.MColor( (0.1, 0.5, 0.95) ) )
		drawManager.setFontSize( omr.MUIDrawManager.kSmallFontSize )
		drawManager.text( om.MPoint( -2, 2, -2 ), "Pre UI draw test in Scene operation", omr.MUIDrawManager.kRight )
		drawManager.line( om.MPoint( -2, 0, -2 ), om.MPoint( -2, 2, -2 ) )
		drawManager.setColor( om.MColor( (1.0, 1.0, 1.0) ) )
		drawManager.sphere( om.MPoint( -2, 2, -2 ), 0.8, False )
		drawManager.setColor( om.MColor( (0.1, 0.5, 0.95, 0.4) ) )
		drawManager.sphere( om.MPoint( -2, 2, -2 ), 0.8, True )
		drawManager.endDrawable()

	# Post UI draw
	def addPostUIDrawables(self, drawManager, frameContext):
		drawManager.beginDrawable()
		drawManager.setColor( om.MColor( (0.05, 0.95, 0.34) ) )
		drawManager.setFontSize( omr.MUIDrawManager.kSmallFontSize )
		drawManager.text( om.MPoint( 2, 2, 2 ), "Post UI draw test in Scene operation", omr.MUIDrawManager.kLeft )
		drawManager.line( om.MPoint( 2, 0, 2), om.MPoint( 2, 2, 2 ) )
		drawManager.setColor( om.MColor( (1.0, 1.0, 1.0) ) )
		drawManager.sphere( om.MPoint( 2, 2, 2 ), 0.8, False )
		drawManager.setColor( om.MColor( (0.05, 0.95, 0.34, 0.4) ) )
		drawManager.sphere( om.MPoint( 2, 2, 2 ), 0.8, True )
		drawManager.endDrawable()

	def panelName(self):
		return self.mPanelName

	def setPanelName(self, name):
		self.mPanelName = name

	def viewRectangle(self):
		return self.mViewRectangle

	def setViewRectangle(self, rect):
		self.mViewRectangle = rect

	def colorTarget(self):
		if not self.mTargets is None:
			return self.mTargets[kMyColorTarget]
		return None

	def depthTarget(self):
		if not self.mTargets is None:
			return self.mTargets[kMyDepthTarget]
		return None

	def setEnableSRGBWriteFlag(self, val):
		self.mEnableSRGBWrite = val

	def enableSRGBWriteFlag(self):
		return self.mEnableSRGBWrite

###################################################################
##
##	A very simplistic custom scene draw example which just draws
##	coloured bounding boxes for surface types.
##
##	Used by the custom user operation (viewRenderUserOperation)
##
###################################################################
class MCustomSceneDraw:

	def matrixAsArray(self, matrix):
		array = []
		for i in range(16):
			array.append(matrix[i])
		return array

	## Some simple code to draw a wireframe bounding box in OpenGL
	def drawBounds(self, dagPath, box):
		matrix = dagPath.inclusiveMatrix()
		minPt = box.min
		maxPt = box.max

		bottomLeftFront  = [ minPt.x, minPt.y, minPt.z ]
		topLeftFront     = [ minPt.x, maxPt.y, minPt.z ]
		bottomRightFront = [ maxPt.x, minPt.y, minPt.z ]
		topRightFront    = [ maxPt.x, maxPt.y, minPt.z ]
		bottomLeftBack   = [ minPt.x, minPt.y, maxPt.z ]
		topLeftBack      = [ minPt.x, maxPt.y, maxPt.z ]
		bottomRightBack  = [ maxPt.x, minPt.y, maxPt.z ]
		topRightBack     = [ maxPt.x, maxPt.y, maxPt.z ]

		glMatrixMode( GL_MODELVIEW )
		glPushMatrix()
		glMultMatrixd( self.matrixAsArray(matrix) )

		glBegin( GL_LINE_STRIP )
		glVertex3dv( bottomLeftFront )
		glVertex3dv( bottomLeftBack )
		glVertex3dv( topLeftBack )
		glVertex3dv( topLeftFront )
		glVertex3dv( bottomLeftFront )
		glVertex3dv( bottomRightFront )
		glVertex3dv( bottomRightBack)
		glVertex3dv( topRightBack )
		glVertex3dv( topRightFront )
		glVertex3dv( bottomRightFront )
		glEnd()

		glBegin( GL_LINES )
		glVertex3dv(bottomLeftBack)
		glVertex3dv(bottomRightBack)

		glVertex3dv(topLeftBack)
		glVertex3dv(topRightBack)

		glVertex3dv(topLeftFront)
		glVertex3dv(topRightFront)
		glEnd()

		glPopMatrix()

	## Draw a scene full of bounding boxes
	def draw(self, cameraPath, size):
		if not cameraPath.isValid:
			return False

		"""
		MDrawTraversal *trav = NULL;
		trav = new MSurfaceDrawTraversal;

		if (!trav)
			return false;

		trav->enableFiltering( true );
		trav->setFrustum( cameraPath, width, height );
		if (!trav->frustumValid())
		{
			delete trav; trav = NULL;
			return false;
		}
		trav->traverse();

		unsigned int numItems = trav->numberOfItems();
		unsigned int i;
		for (i=0; i<numItems; i++)
		{
			MDagPath path;
			trav->itemPath(i, path);

			if (path.isValid())
			{
				bool drawIt = false;

				//
				// Draw surfaces (polys, nurbs, subdivs)
				//
				if ( path.hasFn( MFn::kMesh) ||
					path.hasFn( MFn::kNurbsSurface) ||
					path.hasFn( MFn::kSubdiv) )
				{
					drawIt = true;
					if (trav->itemHasStatus( i, MDrawTraversal::kActiveItem ))
					{
						gGLFT->glColor3f( 1.0f, 1.0f, 1.0f );
					}
					else if (trav->itemHasStatus( i, MDrawTraversal::kTemplateItem ))
					{
						gGLFT->glColor3f( 0.2f, 0.2f, 0.2f );
					}
					else
					{
						if (path.hasFn( MFn::kMesh ))
							gGLFT->glColor3f( 0.286f, 0.706f, 1.0f );
						else if (path.hasFn( MFn::kNurbsSurface))
							gGLFT->glColor3f( 0.486f, 0.306f, 1.0f );
						else
							gGLFT->glColor3f( 0.886f, 0.206f, 1.0f );
					}
				}

				if (drawIt)
				{
					MFnDagNode dagNode(path);
					MBoundingBox box = dagNode.boundingBox();
					drawBounds( path, box );
				}
			}
		}

		if (trav)
		{
			delete trav;
			trav = NULL;
		}
		"""
		return True

###################################################################
#
#	Custom user operation. One approach to adding a pre and
#	post scene operations. In this approach only 1 operation
#	is reused twice with local state as to when it is being
#	used. Another approach which may be more suitable for when
#	global state is changed is to create 2 instances of this
#	operation and keep global state on the override instead of
#	locally here.
#
#	The cost of an override is very small so creating more instances
#	can provide a clearer and cleaner render loop logic.
#
class viewRenderUserOperation(omr.MUserRenderOperation):
	def __init__(self, name):
		omr.MUserRenderOperation.__init__(self, name)

		# 3D viewport panel name, if any
		self.mPanelName = ""
		# Camera override
		self.mCameraOverride = omr.MCameraOverride()
		# Viewport rectangle override
		self.mViewRectangle = om.MFloatPoint(0.0, 0.0, 1.0, 1.0) # 100 % of target size
		# Available targets
		self.mTargets = None
		# sRGB write flag
		self.fEnableSRGBWriteFlag = False
		# Draw an extra label
		self.fDrawLabel = False
		# Use camera override
		self.fUserCameraOverride = False
		# Draw colored bounding boxes
		self.fDrawBoundingBoxes = False
		# Debugging flags
		self.fDebugDrawContext = False
		self.fDebugLightingInfo = False

	def execute(self, drawContext):
		# Sample code to debug pass information
		debugPassInformation = False
		if debugPassInformation:
			passCtx = drawContext.getPassContext()
			passId = passCtx.passIdentifier()
			passSem = passCtx.passSemantics()
			print "viewRenderUserOperation: drawing in pass[" + str(passId) + "], semantic[" + str(passSem) + "]"

		# Example code to find the active override.
		# This is not necessary if the operations just keep a reference
		# to the override, but this demonstrates how this
		# contextual information can be extracted.
		#
		overrideName = omr.MRenderer.activeRenderOverride()
		overrideFunc = omr.MRenderer.findRenderOverride( overrideName )

		# Some sample code to debug lighting information in the MDrawContext
		#
		if self.fDebugLightingInfo:
			printDrawContextLightInfo( drawContext )

		# Some sample code to debug other MDrawContext information
		#
		if self.fDebugDrawContext:
			matrix = drawContext.getMatrix(omr.MFrameContext.kWorldMtx)
			print "World matrix is: " + str(matrix)

			viewDirection = drawContext.getTuple(omr.MFrameContext.kViewDirection)
			print "Viewdirection is: " + str(viewDirection)

			box = drawContext.getSceneBox()
			print "Screen box is: " + str(box)
			print "\tcenter=" + str(box.center)

			vpdim = drawContext.getViewportDimensions()
			print "Viewport dimension: " + str(vpdim)

		#  Draw some addition things for scene draw
		#
		if len(self.mPanelName) > 0:
			view = omui.M3dView.getM3dViewFromModelPanel(self.mPanelName)
			## Get the current viewport and scale it relative to that
			##
			targetSize = drawContext.getRenderTargetSize()

			if self.fDrawLabel:
				testString = "Drawing with override: "
				testString += overrideFunc.name()
				pos = om.MPoint(0.0, 0.0, 0.0)
				glColor3f( 1.0, 1.0, 1.0 )
				view.drawText( testString, pos )

			## Some user drawing of scene bounding boxes
			##
			if self.fDrawBoundingBoxes:
				cameraPath = view.getCamera()
				userDraw = MCustomSceneDraw()
				userDraw.draw( cameraPath, targetSize )

	def cameraOverride(self):
		if self.fUserCameraOverride:
			if len(self.mPanelName) > 0:
				view = omui.M3dView.getM3dViewFromModelPanel(self.mPanelName)
				self.mCameraOverride.mCameraPath = view.getCamera()
				return self.mCameraOverride

		return None

	def targetOverrideList(self):
		if not self.mTargets is None:
			return [ self.mTargets[kMyColorTarget], self.mTargets[kMyDepthTarget] ]
		return None

	def enableSRGBWrite(self):
		return self.fEnableSRGBWriteFlag

	def hasUIDrawables(self):
		return True

	def addUIDrawables(self, drawManager, frameContext):
		drawManager.beginDrawable()
		drawManager.setColor( om.MColor( (0.95, 0.5, 0.1) ) )
		drawManager.text( om.MPoint( 0, 2, 0 ), "UI draw test in user operation" )
		drawManager.line( om.MPoint( 0, 0, 0), om.MPoint( 0, 2, 0 ) )
		drawManager.setColor( om.MColor( (1.0, 1.0, 1.0) ) )
		drawManager.sphere( om.MPoint( 0, 2, 0 ), 0.8, False )
		drawManager.setColor( om.MColor( (0.95, 0.5, 0.1, 0.4) ) )
		drawManager.sphere( om.MPoint( 0, 2, 0 ), 0.8, True )
		drawManager.endDrawable()

	def setRenderTargets(self, targets):
		self.mTargets = targets

	def setEnableSRGBWriteFlag(self, val):
		self.fEnableSRGBWriteFlag = val

	def panelName(self):
		return self.mPanelName

	def setPanelName(self, name):
		self.mPanelName = name

	def viewRectangle(self):
		return self.mViewRectangle

	def setViewRectangle(self, rect):
		self.mViewRectangle = rect


###################################################################
#	Sample custom render override class.
#
#	Is responsible for setting up the render loop operations and
#	updating resources for each frame render as well as any
#	rendering options.
#
#	By default the plugin will perform a number of operations
#	in order to:
#
#	1) Draw a procedurally generated background
#	2) Draw the non-UI parts of the scene using internal logic.
#	3) Threshold the scene
#	4) Blur the thresholded output
#	5) Combine the thresholded output with the original scene (resulting
#	   in a "glow")
#	6a) Draw the UI parts of the scene using internal logic.
#	6b) Perform an option custom user operation for additional UI.
#	7) Draw the 2D HUD
#	8) 'Present' the final output
#
#	A number of intermediate render targets are created to hold contents
#	which are passed from operation to operation.
#
class viewRenderOverride(omr.MRenderOverride):
	def __init__(self, name):
		omr.MRenderOverride.__init__(self, name)

		# UI name which will show up in places
		# like the viewport 'Renderer' menu
		self.mUIName = "Sample VP2 Renderer Override"

		# Operation lists
		self.mRenderOperations = []
		self.mRenderOperationNames = []

		for i in range(kNumberOfOps):
			self.mRenderOperations.append(None)
			self.mRenderOperationNames.append("")
		self.mCurrentOperation = -1

		# Shared render target list
		self.mTargetOverrideNames = []
		self.mTargetDescriptions = []
		self.mTargets = []
		self.mTargetSupportsSRGBWrite = []

		for i in range(kTargetCount):
			self.mTargetOverrideNames.append("")
			self.mTargetDescriptions.append(None)
			self.mTargets.append(None)
			self.mTargetSupportsSRGBWrite.append(False)

		# Init target information for the override
		sampleCount = 1 # no multi-sampling
		colorFormat = omr.MRenderer.kR8G8B8A8_UNORM
		depthFormat = omr.MRenderer.kD24S8

		# There are 3 render targets used for the entire override:
		# 1. Color
		# 2. Depth
		# 3. Intermediate target to perform target blurs
		#
		self.mTargetOverrideNames	[kMyColorTarget] = "__viewRenderOverrideCustomColorTarget__"
		self.mTargetDescriptions	[kMyColorTarget] = omr.MRenderTargetDescription(self.mTargetOverrideNames[kMyColorTarget], 256, 256, sampleCount, colorFormat, 0, False)
		self.mTargets				[kMyColorTarget] = None
		self.mTargetSupportsSRGBWrite[kMyColorTarget] = False

		self.mTargetOverrideNames	[kMyDepthTarget] = "__viewRenderOverrideCustomDepthTarget__"
		self.mTargetDescriptions 	[kMyDepthTarget] = omr.MRenderTargetDescription(self.mTargetOverrideNames[kMyDepthTarget], 256, 256, sampleCount, depthFormat, 0, False)
		self.mTargets				[kMyDepthTarget] = None
		self.mTargetSupportsSRGBWrite[kMyDepthTarget] = False

		self.mTargetOverrideNames	[kMyBlurTarget] = "__viewRenderOverrideBlurTarget__"
		self.mTargetDescriptions 	[kMyBlurTarget]= omr.MRenderTargetDescription(self.mTargetOverrideNames[kMyBlurTarget], 256, 256, sampleCount, colorFormat, 0, False)
		self.mTargets				[kMyBlurTarget] = None
		self.mTargetSupportsSRGBWrite[kMyBlurTarget] = False

		# Set to True to split UI and non-UI draw
		self.mSplitUIDraw = False

		# For debugging
		self.mDebugOverride = False

		# Default do full effects
		self.mSimpleRendering = False

		# Override is for this panel
		self.mPanelName = ""

	def __del__(self):
		targetMgr = omr.MRenderer.getRenderTargetManager()

		# Delete any targets created
		for i in range(kTargetCount):
			self.mTargetDescriptions[i] = None
			
			if not self.mTargets[i] is None:
				if not targetMgr is None:
					targetMgr.releaseRenderTarget(self.mTargets[i])
				self.mTargets[i] = None

		self.cleanup()

		# Delete all the operations. This will release any
		# references to other resources used per operation
		#
		for i in range(kNumberOfOps):
			self.mRenderOperations[i] = None

		# Clean up callbacks
		#
		# PYAPI_TODO if (mRendererChangeCB)
			# PYAPI_TODO MMessage::removeCallback(mRendererChangeCB)
		# PYAPI_TODO if (mRenderOverrideChangeCB)
			# PYAPI_TODO MMessage::removeCallback(mRenderOverrideChangeCB)

	# Return that this plugin supports both GL and DX draw APIs
	def supportedDrawAPIs(self):
		return ( omr.MRenderer.kOpenGL | omr.MRenderer.kDirectX11 | omr.MRenderer.kOpenGLCoreProfile )

	# Initialize "iterator". We keep a list of operations indexed
	# by mCurrentOperation. Set to 0 to point to the first operation.
	def startOperationIterator(self):
		self.mCurrentOperation = 0
		return True

	# Return an operation indicated by mCurrentOperation
	def renderOperation(self):
		if self.mCurrentOperation >= 0 and self.mCurrentOperation < kNumberOfOps:
			while self.mRenderOperations[self.mCurrentOperation] is None:
				self.mCurrentOperation = self.mCurrentOperation+1
				if self.mCurrentOperation >= kNumberOfOps:
					return None

			if not self.mRenderOperations[self.mCurrentOperation] is None:
				if self.mDebugOverride:
					print "\t" + self.name() + "Queue render operation[" + str(self.mCurrentOperation) + "] = (" + self.mRenderOperations[self.mCurrentOperation].name() + ")"
				return self.mRenderOperations[self.mCurrentOperation]
		return None

	# Advance "iterator" to next operation
	def nextRenderOperation(self):
		self.mCurrentOperation = self.mCurrentOperation + 1
		if self.mCurrentOperation < kNumberOfOps:
			return True
		return False

	# Update the render targets that are required for the entire override.
	# References to these targets are set on the individual operations as
	# required so that they will send their output to the appropriate location.
	def updateRenderTargets(self):
		if self.mDebugOverride:
			print "\t" + self.name() + ": Set output render target overrides: color=" + self.mTargetDescriptions[kMyColorTarget].name() + ", depth=" + self.mTargetDescriptions[kMyDepthTarget].name()

		# Get the current output target size as specified by the
		# renderer. If it has changed then the targets need to be
		# resized to match.
		targetSize = omr.MRenderer.outputTargetSize()
		targetWidth = targetSize[0]
		targetHeight = targetSize[1]

		#if self.mTargetDescriptions[kMyColorTarget].width() != targetWidth or self.mTargetDescriptions[kMyColorTarget].height() != targetHeight:
			# A resize occured

		# Note that the render target sizes could be set to be
		# smaller than the size used by the renderer. In this case
		# a final present will generally stretch the output.

		# Update size value for all target descriptions kept
		for targetId in range(kTargetCount):
			self.mTargetDescriptions[targetId].setWidth( targetWidth )
			self.mTargetDescriptions[targetId].setHeight( targetHeight )

		# Keep track of whether the main color target can support sRGB write
		colorTargetSupportsSGRBWrite = False
		# Uncomment this to debug if targets support sRGB write.
		sDebugSRGBWrite = False
		# Enable to testing unordered write access
		testUnorderedWriteAccess = False

		# Either acquire a new target if it didn't exist before, resize
		# the current target.
		targetManager = omr.MRenderer.getRenderTargetManager()
		if not targetManager is None:
			if sDebugSRGBWrite:
				if omr.MRenderer.drawAPI() != omr.MRenderer.kOpenGL:
					# Sample code to scan all available targetgs for sRGB write support
					for i in range(omr.MRenderer.kNumberOfRasterFormats):
						if targetManager.formatSupportsSRGBWrite(i):
							print "Format " + str(i) + " supports SRGBwrite"

			for targetId in range(kTargetCount):
				# Check to see if the format supports sRGB write.
				# Set unordered write access flag if test enabled.
				supportsSRGBWrite = False
				if omr.MRenderer.drawAPI() != omr.MRenderer.kOpenGL:
					supportsSRGBWrite = targetManager.formatSupportsSRGBWrite( self.mTargetDescriptions[targetId].rasterFormat() )
					self.mTargetSupportsSRGBWrite[targetId] = supportsSRGBWrite
				self.mTargetDescriptions[targetId].setAllowsUnorderedAccess( testUnorderedWriteAccess )

				# Keep track of whether the main color target can support sRGB write
				if targetId == kMyColorTarget:
					colorTargetSupportsSGRBWrite = supportsSRGBWrite

				if sDebugSRGBWrite:
					if targetId == kMyColorTarget or targetId == kMyBlurTarget:
						print "Color target " + str(targetId) + " supports sRGB write = " + str(supportsSRGBWrite)
					# This would be expected to fail.
					if targetId == kMyDepthTarget:
						print "Depth target supports sRGB write = " + str(supportsSRGBWrite)

				# Create a new target
				if self.mTargets[targetId] is None:
					self.mTargets[targetId] = targetManager.acquireRenderTarget( self.mTargetDescriptions[targetId] )

				# "Update" using a description will resize as necessary
				else:
					self.mTargets[targetId].updateDescription( self.mTargetDescriptions[targetId] )

				if testUnorderedWriteAccess and not self.mTargets[targetId] is None:
					returnDesc = self.mTargets[targetId].targetDescription()
					self.mTargetDescriptions[targetId].setAllowsUnorderedAccess( returnDesc.allowsUnorderedAccess() )
					print "Acquire target[" + returnDesc.name() + "] with unordered access = " + str(returnDesc.allowsUnorderedAccess()) + ". Should fail if attempting with depth target = " + str(targetId == kMyDepthTarget)

		# Update the render targets on the individual operations
		#
		# Set the targets on the operations. For simplicity just
		# passing over the set of all targets used for the frame
		# to each operation.
		#
		quadOp = self.mRenderOperations[kBackgroundBlit]
		if not quadOp is None:
			quadOp.setRenderTargets(self.mTargets)

		sceneOp = self.mRenderOperations[kMaya3dSceneRender]
		if not sceneOp is None:
			sceneOp.setRenderTargets(self.mTargets)
			sceneOp.setEnableSRGBWriteFlag( colorTargetSupportsSGRBWrite )

		opaqueSceneOp = self.mRenderOperations[kMaya3dSceneRenderOpaque]
		if not opaqueSceneOp is None:
			opaqueSceneOp.setRenderTargets(self.mTargets)
			opaqueSceneOp.setEnableSRGBWriteFlag( colorTargetSupportsSGRBWrite )

		transparentSceneOp = self.mRenderOperations[kMaya3dSceneRenderTransparent]
		if not transparentSceneOp is None:
			transparentSceneOp.setRenderTargets(self.mTargets)
			transparentSceneOp.setEnableSRGBWriteFlag( colorTargetSupportsSGRBWrite )

		uiSceneOp = self.mRenderOperations[kMaya3dSceneRenderUI]
		if not uiSceneOp is None:
			uiSceneOp.setRenderTargets(self.mTargets)
			uiSceneOp.setEnableSRGBWriteFlag( False ) # Don't enable sRGB write for UI

		quadOp2 = self.mRenderOperations[kPostOperation1]
		if not quadOp2 is None:
			quadOp2.setRenderTargets(self.mTargets)

		quadOp3 = self.mRenderOperations[kPostOperation2]
		if not quadOp3 is None:
			quadOp3.setRenderTargets(self.mTargets)

		userOp = self.mRenderOperations[kUserOpNumber]
		if not userOp is None:
			userOp.setRenderTargets(self.mTargets)
			userOp.setEnableSRGBWriteFlag( colorTargetSupportsSGRBWrite ) # Enable sRGB write for user ops

		presentOp = self.mRenderOperations[kPresentOp]
		if not presentOp is None:
			presentOp.setRenderTargets(self.mTargets)

		thresholdOp = self.mRenderOperations[kThresholdOp]
		if not thresholdOp is None:
			thresholdOp.setRenderTargets(self.mTargets)

		horizBlur = self.mRenderOperations[kHorizBlurOp]
		if not horizBlur is None:
			horizBlur.setRenderTargets(self.mTargets)

		vertBlur = self.mRenderOperations[kVertBlurOp]
		if not vertBlur is None:
			vertBlur.setRenderTargets(self.mTargets)

		blendOp = self.mRenderOperations[kBlendOp]
		if not blendOp is None:
			blendOp.setRenderTargets(self.mTargets)

		hudOp = self.mRenderOperations[kHUDBlit]
		if not hudOp is None:
			hudOp.setRenderTargets(self.mTargets)

		return (not self.mTargets[kMyColorTarget] is None and not self.mTargets[kMyDepthTarget] is None and not self.mTargets[kMyBlurTarget] is None)

	# "setup" will be called for each frame update.
	# Here we set up the render loop logic and allocate any necessary resources.
	# The render loop logic setup is done by setting up a list of
	# render operations which will be returned by the "iterator" calls.
	def setup(self, destination ):
		if self.mDebugOverride:
			print self.name() + " : Perform setup with panel [" + destination + "]"

		# As an example, we keep track of the active 3d viewport panel
		# if any exists. This information is passed to the operations
		# in case they require accessing the current 3d view (M3dView).
		self.mPanelName = destination

		# Track changes to the renderer and override for this viewport (nothing
		# will be printed unless mDebugOverride is True)
		# PYAPI_TODO if (!mRendererChangeCB)
			# PYAPI_TODO mRendererChangeCB = MUiMessage::add3dViewRendererChangedCallback(destination, sRendererChangeFunc, (void*)mDebugOverride)
		# PYAPI_TODO if (!mRenderOverrideChangeCB)
			# PYAPI_TODO mRenderOverrideChangeCB = MUiMessage::add3dViewRenderOverrideChangedCallback(destination, sRenderOverrideChangeFunc, (void*)mDebugOverride)

		if self.mRenderOperations[kPresentOp] is None:
			# Sample of a "simple" render loop.
			# "Simple" means a scene draw + HUD + present to viewport
			if self.mSimpleRendering:
				self.mSplitUIDraw = False

				self.mRenderOperations[kBackgroundBlit] = None

				self.mRenderOperationNames[kMaya3dSceneRender] = "__MySimpleSceneRender"
				sceneOp = simpleViewRenderSceneRender( self.mRenderOperationNames[kMaya3dSceneRender] )
				self.mRenderOperations[kMaya3dSceneRender] = sceneOp

				# NULL out any additional opertions used for the "complex" render loop
				self.mRenderOperations[kMaya3dSceneRenderOpaque] = None
				self.mRenderOperations[kMaya3dSceneRenderTransparent] = None
				self.mRenderOperations[kThresholdOp] = None
				self.mRenderOperations[kHorizBlurOp] = None
				self.mRenderOperations[kVertBlurOp] = None
				self.mRenderOperations[kPostOperation1] = None
				self.mRenderOperations[kPostOperation2] = None
				self.mRenderOperations[kMaya3dSceneRenderUI] = None
				self.mRenderOperations[kUserOpNumber] = None

				self.mRenderOperations[kHUDBlit] = viewRenderHUDOperation()
				self.mRenderOperationNames[kHUDBlit] = self.mRenderOperations[kHUDBlit].name()

				self.mRenderOperationNames[kPresentOp] = "__MyPresentTarget"
				self.mRenderOperations[kPresentOp] = viewRenderPresentTarget( self.mRenderOperationNames[kPresentOp] )
				self.mRenderOperationNames[kPresentOp] = self.mRenderOperations[kPresentOp].name()

			# Sample which performs the full "complex" render loop
			#
			else:
				rect = om.MFloatPoint(0.0, 0.0, 1.0, 1.0)

				# Pre scene quad render to render a procedurally drawn background
				#
				self.mRenderOperationNames[kBackgroundBlit] = "__MyPreQuadRender"
				quadOp = viewRenderQuadRender( self.mRenderOperationNames[kBackgroundBlit] )
				quadOp.setShader( viewRenderQuadRender.kPre_MandelBrot ) # We use a shader override to render the background
				quadOp.setViewRectangle(rect)
				self.mRenderOperations[kBackgroundBlit] = quadOp

				# Set up scene draw operations
				#
				# This flag indicates that we wish to split up the scene draw into
				# opaque, transparent, and UI passes.
				#
				# If we don't split up the UI from the opaque and transparent,
				# the UI will have the "glow" effect applied to it. Instead
				# splitting up will allow the UI to draw after the "glow" effect
				# has been applied.
				#
				self.mSplitUIDraw = True
				self.mRenderOperations[kMaya3dSceneRender] = None
				self.mRenderOperations[kMaya3dSceneRenderOpaque] = None
				self.mRenderOperations[kMaya3dSceneRenderTransparent] = None
				self.mRenderOperations[kMaya3dSceneRenderUI] = None
				if self.mSplitUIDraw:
					# opaque
					sceneOp = None
					sDrawOpaque = True # can disable if desired
					if sDrawOpaque:
						self.mRenderOperationNames[kMaya3dSceneRenderOpaque] = "__MyStdSceneRenderOpaque"
						clearMask = omr.MClearOperation.kClearDepth | omr.MClearOperation.kClearStencil
						sceneOp = viewRenderSceneRender( self.mRenderOperationNames[kMaya3dSceneRenderOpaque], omr.MSceneRender.kRenderOpaqueShadedItems, clearMask )
						sceneOp.setViewRectangle(rect)
						self.mRenderOperations[kMaya3dSceneRenderOpaque] = sceneOp

					# transparent, clear nothing since needs to draw on top of opaque
					sDrawTransparent = True # can disable if desired
					if sDrawTransparent:
						self.mRenderOperationNames[kMaya3dSceneRenderTransparent] = "__MyStdSceneRenderTransparent"
						clearMask = omr.MClearOperation.kClearDepth | omr.MClearOperation.kClearStencil
						if sDrawOpaque:
							clearMask = omr.MClearOperation.kClearNone
						sceneOp = viewRenderSceneRender( self.mRenderOperationNames[kMaya3dSceneRenderTransparent], omr.MSceneRender.kRenderTransparentShadedItems, clearMask )
						sceneOp.setViewRectangle(rect)
						self.mRenderOperations[kMaya3dSceneRenderTransparent] = sceneOp

					# ui, don't clear depth since we need it for drawing ui correctly
					self.mRenderOperationNames[kMaya3dSceneRenderUI] = "__MyStdSceneRenderUI"
					clearMask = omr.MClearOperation.kClearDepth | omr.MClearOperation.kClearStencil
					if sDrawOpaque or sDrawTransparent:
						clearMask = omr.MClearOperation.kClearStencil
					sceneOp = viewRenderSceneRender( self.mRenderOperationNames[kMaya3dSceneRenderUI], omr.MSceneRender.kRenderUIItems, clearMask )
					sceneOp.setViewRectangle(rect)
					self.mRenderOperations[kMaya3dSceneRenderUI] = sceneOp
				else:
					# will draw all of opaque, transparent and ui at once
					self.mRenderOperationNames[kMaya3dSceneRender] = "__MyStdSceneRender"
					clearMask = omr.MClearOperation.kClearDepth | omr.MClearOperation.kClearStencil
					sceneOp = viewRenderSceneRender( self.mRenderOperationNames[kMaya3dSceneRender], omr.MSceneRender.kNoSceneFilterOverride, clearMask )
					sceneOp.setViewRectangle(rect)
					self.mRenderOperations[kMaya3dSceneRender] = sceneOp

				# Set up operations which will perform a threshold and a blur on the thresholded
				# render target. Also included is an operation to blend the non-UI scene
				# render target with the output of this set of operations (thresholded blurred scene)
				#
				self.mRenderOperationNames[kThresholdOp] = "__ThresholdColor"
				quadThreshold = viewRenderQuadRender( self.mRenderOperationNames[kThresholdOp] )
				quadThreshold.setShader( viewRenderQuadRender.kScene_Threshold ) # Use threshold shader
				quadThreshold.setViewRectangle(rect)
				self.mRenderOperations[kThresholdOp] = quadThreshold

				self.mRenderOperationNames[kHorizBlurOp] = "__HorizontalBlur"
				quadHBlur = viewRenderQuadRender( self.mRenderOperationNames[kHorizBlurOp] )
				quadHBlur.setShader( viewRenderQuadRender.kScene_BlurHoriz ) # Use horizontal blur shader
				quadHBlur.setViewRectangle(rect)
				self.mRenderOperations[kHorizBlurOp] = quadHBlur

				self.mRenderOperationNames[kVertBlurOp] = "__VerticalBlur"
				quadVBlur = viewRenderQuadRender( self.mRenderOperationNames[kVertBlurOp] )
				quadVBlur.setShader( viewRenderQuadRender.kScene_BlurVert ) # Use vertical blur shader
				quadVBlur.setViewRectangle(rect)
				self.mRenderOperations[kVertBlurOp] = quadVBlur

				self.mRenderOperationNames[kBlendOp] = "__SceneBlurBlend"
				quadBlend = viewRenderQuadRender( self.mRenderOperationNames[kBlendOp] )
				quadBlend.setShader( viewRenderQuadRender.kSceneBlur_Blend ) # Use color blend shader
				quadBlend.setViewRectangle(rect)
				self.mRenderOperations[kBlendOp] = quadBlend

				# Sample custom operation which will peform a custom "scene render"
				#
				self.mRenderOperationNames[kUserOpNumber] = "__MyCustomSceneRender"
				userOp = viewRenderUserOperation( self.mRenderOperationNames[kUserOpNumber] )
				userOp.setViewRectangle(rect)
				self.mRenderOperations[kUserOpNumber] = userOp

				wantPostQuadOps = False

				# Some sample post scene quad render operations
				# a. Monochrome quad render with custom shader
				self.mRenderOperationNames[kPostOperation1] = "__PostOperation1"
				quadOp2 = viewRenderQuadRender( self.mRenderOperationNames[kPostOperation1] )
				quadOp2.setShader( viewRenderQuadRender.kPost_EffectMonochrome )
				quadOp2.setViewRectangle(rect)
				if wantPostQuadOps:
					self.mRenderOperations[kPostOperation1] = quadOp2
				else:
					self.mRenderOperations[kPostOperation1] = None

				# b. Invert quad render with custom shader
				self.mRenderOperationNames[kPostOperation2] = "__PostOperation2"
				quadOp3 = viewRenderQuadRender( self.mRenderOperationNames[kPostOperation2] )
				quadOp3.setShader( viewRenderQuadRender.kPost_EffectInvert )
				quadOp3.setViewRectangle(rect)
				if wantPostQuadOps:
					self.mRenderOperations[kPostOperation2] = quadOp3
				else:
					self.mRenderOperations[kPostOperation2] = None

				# "Present" opertion which will display the target for viewports.
				# Operation is a no-op for batch rendering as there is no on-screen
				# buffer to send the result to.
				self.mRenderOperationNames[kPresentOp] = "__MyPresentTarget"
				self.mRenderOperations[kPresentOp] = viewRenderPresentTarget( self.mRenderOperationNames[kPresentOp] )
				self.mRenderOperationNames[kPresentOp] = self.mRenderOperations[kPresentOp].name()

				# A preset 2D HUD render operation
				self.mRenderOperations[kHUDBlit] = viewRenderHUDOperation()
				self.mRenderOperationNames[kHUDBlit] = self.mRenderOperations[kHUDBlit].name()

		gotTargets = True
		if not self.mSimpleRendering:
			# Update any of the render targets which will be required
			gotTargets = self.updateRenderTargets()

			# Set the name of the panel on operations which may use the panel
			# name to find out the associated M3dView.
			if not self.mRenderOperations[kMaya3dSceneRender] is None:
				self.mRenderOperations[kMaya3dSceneRender].setPanelName( self.mPanelName )
			if not self.mRenderOperations[kMaya3dSceneRenderOpaque] is None:
				self.mRenderOperations[kMaya3dSceneRenderOpaque].setPanelName( self.mPanelName )
			if not self.mRenderOperations[kMaya3dSceneRenderTransparent] is None:
				self.mRenderOperations[kMaya3dSceneRenderTransparent].setPanelName( self.mPanelName )
			if not self.mRenderOperations[kMaya3dSceneRenderUI] is None:
				self.mRenderOperations[kMaya3dSceneRenderUI].setPanelName( self.mPanelName )
			if not self.mRenderOperations[kUserOpNumber] is None:
				self.mRenderOperations[kUserOpNumber].setPanelName( self.mPanelName )

		self.mCurrentOperation = -1

		if not gotTargets:
			raise ValueError

	# End of frame cleanup. For now just clears out any data on operations which may
	# change from frame to frame (render target, output panel name etc)
	def cleanup(self):
		if self.mDebugOverride:
			print self.name() + " : Perform cleanup. panelname=" + self.mPanelName

		quadOp = self.mRenderOperations[kPostOperation1]
		if not quadOp is None:
			quadOp.setRenderTargets(None)

		quadOp = self.mRenderOperations[kPostOperation2]
		if not quadOp is None:
			quadOp.setRenderTargets(None)

		# Reset the active view
		self.mPanelName = ""
		# Reset current operation
		self.mCurrentOperation = -1

	def panelName(self):
		return self.mPanelName

	def setSimpleRendering(self, flag):
		self.mSimpleRendering = flag

	def uiName(self):
		return self.mUIName


viewRenderOverrideInstance = None

#
#	Register an override and associated command
#
def initializePlugin(obj):
	plugin = om.MFnPlugin(obj) 
	try:
		global viewRenderOverrideInstance
		viewRenderOverrideInstance = viewRenderOverride( "my_viewRenderOverride" )
		omr.MRenderer.registerOverride(viewRenderOverrideInstance)
	except:
		sys.stderr.write("registerOverride\n")
		raise

#
#	When uninitializing the plugin, make sure to deregister the
#	override and then delete the instance which is being kept here.
#
#	Also remove the command used to set options on the override
#
def uninitializePlugin(obj):
	plugin = om.MFnPlugin(obj) 
	try:
		global viewRenderOverrideInstance
		if not viewRenderOverrideInstance is None:
			omr.MRenderer.deregisterOverride(viewRenderOverrideInstance)
			viewRenderOverrideInstance = None
	except:
		sys.stderr.write("deregisterOverride\n")
		raise

