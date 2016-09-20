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
import ctypes
import maya.api.OpenMaya as om
import maya.api.OpenMayaUI as omui
import maya.api.OpenMayaAnim as oma
import maya.api.OpenMayaRender as omr

def maya_useNewAPI():
	"""
	The presence of this function tells Maya that the plugin produces, and
	expects to be passed, objects created using the Maya Python API 2.0.
	"""
	pass

def matrixAsArray(matrix):
	array = []
	for i in range(16):
		array.append(matrix[i])
	return array

## Foot Data
##
sole = [ [  0.00, 0.0, -0.70 ],
				 [  0.04, 0.0, -0.69 ],
				 [  0.09, 0.0, -0.65 ],
				 [  0.13, 0.0, -0.61 ],
				 [  0.16, 0.0, -0.54 ],
				 [  0.17, 0.0, -0.46 ],
				 [  0.17, 0.0, -0.35 ],
				 [  0.16, 0.0, -0.25 ],
				 [  0.15, 0.0, -0.14 ],
				 [  0.13, 0.0,  0.00 ],
				 [  0.00, 0.0,  0.00 ],
				 [ -0.13, 0.0,  0.00 ],
				 [ -0.15, 0.0, -0.14 ],
				 [ -0.16, 0.0, -0.25 ],
				 [ -0.17, 0.0, -0.35 ],
				 [ -0.17, 0.0, -0.46 ],
				 [ -0.16, 0.0, -0.54 ],
				 [ -0.13, 0.0, -0.61 ],
				 [ -0.09, 0.0, -0.65 ],
				 [ -0.04, 0.0, -0.69 ],
				 [ -0.00, 0.0, -0.70 ] ]
heel = [ [  0.00, 0.0,  0.06 ],
				 [  0.13, 0.0,  0.06 ],
				 [  0.14, 0.0,  0.15 ],
				 [  0.14, 0.0,  0.21 ],
				 [  0.13, 0.0,  0.25 ],
				 [  0.11, 0.0,  0.28 ],
				 [  0.09, 0.0,  0.29 ],
				 [  0.04, 0.0,  0.30 ],
				 [  0.00, 0.0,  0.30 ],
				 [ -0.04, 0.0,  0.30 ],
				 [ -0.09, 0.0,  0.29 ],
				 [ -0.11, 0.0,  0.28 ],
				 [ -0.13, 0.0,  0.25 ],
				 [ -0.14, 0.0,  0.21 ],
				 [ -0.14, 0.0,  0.15 ],
				 [ -0.13, 0.0,  0.06 ],
				 [ -0.00, 0.0,  0.06 ] ]
soleCount = 21
heelCount = 17


#############################################################################
##
## Node implementation with standard viewport draw
##
#############################################################################
class footPrint(omui.MPxLocatorNode):
	id = om.MTypeId( 0x80007 )
	drawDbClassification = "drawdb/geometry/footPrint"
	drawRegistrantId = "FootprintNodePlugin"

	size = None	## The size of the foot

	@staticmethod
	def creator():
		return footPrint()

	@staticmethod
	def initialize():
		unitFn = om.MFnUnitAttribute()

		footPrint.size = unitFn.create( "size", "sz", om.MFnUnitAttribute.kDistance )
		unitFn.default = om.MDistance(1.0)

		om.MPxNode.addAttribute( footPrint.size )

	def __init__(self):
		omui.MPxLocatorNode.__init__(self)

	def compute(self, plug, data):
		return None

	def draw(self, view, path, style, status):
		## Get the size
		##
		thisNode = self.thisMObject()
		plug = om.MPlug( thisNode, footPrint.size )
		sizeVal = plug.asMDistance()
		multiplier = sizeVal.asCentimeters()

		global sole, soleCount
		global heel, heelCount

		view.beginGL()

		## drawing in VP1 views will be done using V1 Python APIs:
		import maya.OpenMayaRender as v1omr
		glRenderer = v1omr.MHardwareRenderer.theRenderer()
		glFT = glRenderer.glFunctionTable()

		if ( style == omui.M3dView.kFlatShaded ) or ( style == omui.M3dView.kGouraudShaded ):
			## Push the color settings
			##
			glFT.glPushAttrib( v1omr.MGL_CURRENT_BIT )
			
			# Show both faces
			glFT.glDisable( v1omr.MGL_CULL_FACE )

			if status == omui.M3dView.kActive:
				view.setDrawColor( 13, omui.M3dView.kActiveColors )
			else:
				view.setDrawColor( 13, omui.M3dView.kDormantColors )

			glFT.glBegin( v1omr.MGL_TRIANGLE_FAN )
			for i in range(soleCount-1):
				glFT.glVertex3f( sole[i][0] * multiplier, sole[i][1] * multiplier, sole[i][2] * multiplier )
			glFT.glEnd()

			glFT.glBegin( v1omr.MGL_TRIANGLE_FAN )
			for i in range(heelCount-1):
				glFT.glVertex3f( heel[i][0] * multiplier, heel[i][1] * multiplier, heel[i][2] * multiplier )
			glFT.glEnd()

			glFT.glPopAttrib()

		## Draw the outline of the foot
		##
		glFT.glBegin( v1omr.MGL_LINES )
		for i in range(soleCount-1):
			glFT.glVertex3f( sole[i][0] * multiplier, sole[i][1] * multiplier, sole[i][2] * multiplier )
			glFT.glVertex3f( sole[i+1][0] * multiplier, sole[i+1][1] * multiplier, sole[i+1][2] * multiplier )

		for i in range(heelCount-1):
			glFT.glVertex3f( heel[i][0] * multiplier, heel[i][1] * multiplier, heel[i][2] * multiplier )
			glFT.glVertex3f( heel[i+1][0] * multiplier, heel[i+1][1] * multiplier, heel[i+1][2] * multiplier )
		glFT.glEnd()

		view.endGL()

		## Draw the name of the footPrint
		view.setDrawColor( om.MColor( (0.1, 0.8, 0.8, 1.0) ) )
		view.drawText( "Footprint", om.MPoint( 0.0, 0.0, 0.0 ), omui.M3dView.kCenter )

	def isBounded(self):
		return True

	def boundingBox(self):
		## Get the size
		##
		thisNode = self.thisMObject()
		plug = om.MPlug( thisNode, footPrint.size )
		sizeVal = plug.asMDistance()
		multiplier = sizeVal.asCentimeters()

		corner1 = om.MPoint( -0.17, 0.0, -0.7 )
		corner2 = om.MPoint( 0.17, 0.0, 0.3 )

		corner1 *= multiplier
		corner2 *= multiplier

		return om.MBoundingBox( corner1, corner2 )

#############################################################################
##
## Viewport 2.0 override implementation
##
#############################################################################
class footPrintData(om.MUserData):
	def __init__(self):
		om.MUserData.__init__(self, False) ## don't delete after draw

		self.fMultiplier = 0.0
		self.fColor = [0.0, 0.0, 0.0]
		self.fCustomBoxDraw = False
		self.fDrawOV = om.MDAGDrawOverrideInfo()

## Helper class declaration for the object drawing
class footPrintDrawAgent:
	def __init__(self):
		self.mShader = None

		self.mBoundingboxVertexBuffer = None
		self.mBoundingboxIndexBuffer = None
		self.mSoleVertexBuffer = None
		self.mHeelVertexBuffer = None
		self.mSoleWireIndexBuffer = None
		self.mHeelWireIndexBuffer = None
		self.mSoleShadedIndexBuffer = None
		self.mHeelShadedIndexBuffer = None

	def __del__(self):
		if self.mShader is not None:
			shaderMgr = omr.MRenderer.getShaderManager()
			if shaderMgr is not None:
				shaderMgr.releaseShader(self.mShader)
			self.mShader = None
		
		self.mBoundingboxVertexBuffer = None
		self.mBoundingboxIndexBuffer = None
		self.mSoleVertexBuffer = None
		self.mHeelVertexBuffer = None
		self.mSoleWireIndexBuffer = None
		self.mHeelWireIndexBuffer = None
		self.mSoleShadedIndexBuffer = None
		self.mHeelShadedIndexBuffer = None

	def beginDraw(self, context, color, scale):
		self.initShader()
		self.initBuffers()

		if self.mShader is not None:
			self.mShader.setParameter("matColor", color)
			self.mShader.setParameter("scale", scale)
			self.mShader.bind(context)
			self.mShader.activatePass(context, 0)

	def drawShaded(self, context):
		global soleCount, heelCount

		## Draw the sole
		if self.mSoleVertexBuffer is not None and self.mSoleShadedIndexBuffer is not None:
			omr.MRenderUtilities.drawSimpleMesh(context, self.mSoleVertexBuffer, self.mSoleShadedIndexBuffer, omr.MGeometry.kTriangles, 0, 3 * (soleCount-2))
		
		## Draw the heel
		if self.mHeelVertexBuffer is not None and self.mHeelShadedIndexBuffer is not None:
			omr.MRenderUtilities.drawSimpleMesh(context, self.mHeelVertexBuffer, self.mHeelShadedIndexBuffer, omr.MGeometry.kTriangles, 0, 3 * (heelCount-2))
		
	def drawBoundingBox(self, context):
		if self.mBoundingboxVertexBuffer is not None and self.mBoundingboxIndexBuffer is not None:
			omr.MRenderUtilities.drawSimpleMesh(context, self.mBoundingboxVertexBuffer, self.mBoundingboxIndexBuffer, omr.MGeometry.kLines, 0, 24)

	def drawWireframe(self, context):
		global soleCount, heelCount

		## Draw the sole
		if self.mSoleVertexBuffer is not None and self.mSoleWireIndexBuffer is not None:
			omr.MRenderUtilities.drawSimpleMesh(context, self.mSoleVertexBuffer, self.mSoleWireIndexBuffer, omr.MGeometry.kLines, 0, 2 * (soleCount-1))
		
		## Draw the heel
		if self.mHeelVertexBuffer is not None and self.mHeelWireIndexBuffer is not None:
			omr.MRenderUtilities.drawSimpleMesh(context, self.mHeelVertexBuffer, self.mHeelWireIndexBuffer, omr.MGeometry.kLines, 0, 2 * (heelCount-1))

	def endDraw(self, context):
		if self.mShader is not None:
			self.mShader.unbind(context)

	def initShader(self):
		if self.mShader is None:
			shaderMgr = omr.MRenderer.getShaderManager()
			if shaderMgr is not None:
				shaderCode = self.getShaderCode()
				self.mShader = shaderMgr.getEffectsBufferShader(shaderCode, len(shaderCode), "")

		return self.mShader is not None
	
	def shaderCode(self):
		return ""

	def initBuffers(self):
		global soleCount, sole
		global heelCount, heel

		if self.mBoundingboxVertexBuffer is None:
			count = 8
			rawData = [ [ -0.5, -0.5, -0.5 ],
						[  0.5, -0.5, -0.5 ],
						[  0.5, -0.5,  0.5 ],
						[ -0.5, -0.5,  0.5 ],
						[ -0.5,  0.5, -0.5 ],
						[  0.5,  0.5, -0.5 ],
						[  0.5,  0.5,  0.5 ],
						[ -0.5,  0.5,  0.5 ] ]
									
			desc = omr.MVertexBufferDescriptor("", omr.MGeometry.kPosition, omr.MGeometry.kFloat, 3)
			self.mBoundingboxVertexBuffer = omr.MVertexBuffer(desc)
			
			dataAddress = self.mBoundingboxVertexBuffer.acquire(count, True)
			data = ((ctypes.c_float * 3)*count).from_address(dataAddress)
			
			for i in range(count):
				data[i][0] = rawData[i][0]
				data[i][1] = rawData[i][1]
				data[i][2] = rawData[i][2]
									
			self.mBoundingboxVertexBuffer.commit(dataAddress)
			dataAddress = None
			data = None

		if self.mBoundingboxIndexBuffer is None:
			count = 24
			rawData = [ 0,1,
						1,2,
						2,3,
						3,0,
						4,5,
						5,6,
						6,7,
						7,4,
						0,4,
						1,5,
						2,6,
						3,7 ]
		
			self.mBoundingboxIndexBuffer = omr.MIndexBuffer(omr.MGeometry.kUnsignedInt32)

			dataAddress = self.mBoundingboxIndexBuffer.acquire(count, True)
			data = (ctypes.c_uint * count).from_address(dataAddress)
			
			for i in range(count):
				data[i] = rawData[i]

			self.mBoundingboxIndexBuffer.commit(dataAddress)
			dataAddress = None
			data = None

		if self.mSoleVertexBuffer is None:
			desc = omr.MVertexBufferDescriptor("", omr.MGeometry.kPosition, omr.MGeometry.kFloat, 3)
			self.mSoleVertexBuffer = omr.MVertexBuffer(desc)

			dataAddress = self.mSoleVertexBuffer.acquire(soleCount, True)
			data = ((ctypes.c_float * 3)*soleCount).from_address(dataAddress)

			for i in range(soleCount):
				data[i][0] = sole[i][0]
				data[i][1] = sole[i][1]
				data[i][2] = sole[i][2]
									
			self.mSoleVertexBuffer.commit(dataAddress)
			dataAddress = None
			data = None

		if self.mHeelVertexBuffer is None:
			desc = omr.MVertexBufferDescriptor("", omr.MGeometry.kPosition, omr.MGeometry.kFloat, 3)
			self.mHeelVertexBuffer = omr.MVertexBuffer(desc)

			dataAddress = self.mHeelVertexBuffer.acquire(heelCount, True)
			data = ((ctypes.c_float * 3)*heelCount).from_address(dataAddress)

			for i in range(heelCount):
				data[i][0] = heel[i][0]
				data[i][1] = heel[i][1]
				data[i][2] = heel[i][2]
									
			self.mHeelVertexBuffer.commit(dataAddress)
			dataAddress = None
			data = None

		if self.mSoleWireIndexBuffer is None:
			count = 2 * (soleCount-1)
			rawData = [ 0, 1,
						1, 2,
						2, 3,
						3, 4,
						4, 5,
						5, 6,
						6, 7,
						7, 8,
						8, 9,
						9, 10,
						10, 11,
						11, 12,
						12, 13,
						13, 14,
						14, 15,
						15, 16,
						16, 17,
						17, 18,
						18, 19,
						19, 20 ]
									
			self.mSoleWireIndexBuffer = omr.MIndexBuffer(omr.MGeometry.kUnsignedInt32)

			dataAddress = self.mSoleWireIndexBuffer.acquire(count, True)
			data = (ctypes.c_uint * count).from_address(dataAddress)

			for i in range(count):
				data[i] = rawData[i]

			self.mSoleWireIndexBuffer.commit(dataAddress)
			dataAddress = None
			data = None

		if self.mHeelWireIndexBuffer is None:
			count = 2 * (heelCount-1)
			rawData = [ 0, 1,
						1, 2,
						2, 3,
						3, 4,
						4, 5,
						5, 6,
						6, 7,
						7, 8,
						8, 9,
						9, 10,
						10, 11,
						11, 12,
						12, 13,
						13, 14,
						14, 15,
						15, 16 ]
									
			self.mHeelWireIndexBuffer = omr.MIndexBuffer(omr.MGeometry.kUnsignedInt32)

			dataAddress = self.mHeelWireIndexBuffer.acquire(count, True)
			data = (ctypes.c_uint * count).from_address(dataAddress)

			for i in range(count):
				data[i] = rawData[i]

			self.mHeelWireIndexBuffer.commit(dataAddress)
			dataAddress = None
			data = None

		if self.mSoleShadedIndexBuffer is None:
			count = 3 * (soleCount-2)
			rawData = [ 0, 1, 2,
						0, 2, 3,
						0, 3, 4,
						0, 4, 5,
						0, 5, 6,
						0, 6, 7,
						0, 7, 8,
						0, 8, 9,
						0, 9, 10,
						0, 10, 11,
						0, 11, 12,
						0, 12, 13,
						0, 13, 14,
						0, 14, 15,
						0, 15, 16,
						0, 16, 17,
						0, 17, 18,
						0, 18, 19,
						0, 19, 20 ]

			self.mSoleShadedIndexBuffer = omr.MIndexBuffer(omr.MGeometry.kUnsignedInt32)

			dataAddress = self.mSoleShadedIndexBuffer.acquire(count, True)
			data = (ctypes.c_uint * count).from_address(dataAddress)

			for i in range(count):
				data[i] = rawData[i]

			self.mSoleShadedIndexBuffer.commit(dataAddress)
			dataAddress = None
			data = None

		if self.mHeelShadedIndexBuffer is None:
			count = 3 * (heelCount-2)
			rawData = [ 0, 1, 2,
						0, 2, 3,
						0, 3, 4,
						0, 4, 5,
						0, 5, 6,
						0, 6, 7,
						0, 7, 8,
						0, 8, 9,
						0, 9, 10,
						0, 10, 11,
						0, 11, 12,
						0, 12, 13,
						0, 13, 14,
						0, 14, 15,
						0, 15, 16 ]
									
			self.mHeelShadedIndexBuffer = omr.MIndexBuffer(omr.MGeometry.kUnsignedInt32)

			dataAddress = self.mHeelShadedIndexBuffer.acquire(count, True)
			data = (ctypes.c_uint * count).from_address(dataAddress)

			for i in range(count):
				data[i] = rawData[i]

			self.mHeelShadedIndexBuffer.commit(dataAddress)
			dataAddress = None
			data = None

		return True

## GL draw agent declaration
class footPrintDrawAgentGL(footPrintDrawAgent):
	def __init__(self):
		footPrintDrawAgent.__init__(self)

	def getShaderCode(self):
		shaderCode = """
float4x4 gWVXf : WorldView;
float4x4 gPXf : Projection;
float4 matColor = float4(0.8, 0.2, 0.0, 1.0);
float scale = 1.0;

struct appdata {
	float3 position : POSITION;
};

struct vsOutput {
	float4 position : POSITION;
};

vsOutput footPrintVS(appdata IN)
{
	float4x4 scaleMat = float4x4(scale, 0, 0, 0,
								 0, scale, 0, 0,
								 0, 0, scale, 0,
								 0, 0, 0, 1);
	float4x4 transform = mul(gPXf, mul(gWVXf, scaleMat));

	vsOutput OUT;
	OUT.position = mul(transform, float4(IN.position, 1));
	return OUT;
}

float4 footPrintPS(vsOutput IN) : COLOR
{
	return matColor;
}

technique Main
{
	pass p0
	{
		VertexShader = compile glslv footPrintVS();
		PixelShader = compile glslf footPrintPS();
	}
}
"""
		return shaderCode

## DX draw agent declaration
class footPrintDrawAgentDX(footPrintDrawAgent):
	def __init__(self):
		footPrintDrawAgent.__init__(self)

	def getShaderCode(self):
		shaderCode = """
extern float4x4 gWVXf : WorldView;
extern float4x4 gPXf : Projection;
extern float4 matColor = float4(0.8, 0.2, 0.0, 1.0);
extern float scale = 1.0;

struct appdata {
	float3 position : POSITION;
};

struct vsOutput {
	float4 position : SV_Position;
};

vsOutput footPrintVS(appdata IN)
{
	float4x4 scaleMat = float4x4(scale, 0, 0, 0,
								 0, scale, 0, 0,
								 0, 0, scale, 0,
								 0, 0, 0, 1);
	float4x4 transform = mul(mul(scaleMat, gWVXf), gPXf);

	vsOutput OUT;
	OUT.position = mul(float4(IN.position, 1), transform);
	return OUT;
}

float4 footPrintPS(vsOutput IN) : SV_Target
{
	return matColor;
}

technique10 Main
{
	pass p0
	{
		SetVertexShader( CompileShader( vs_5_0, footPrintVS() ));
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, footPrintPS() ));
	}
}
"""
		return shaderCode

blendState = None
rasterState = None
drawAgent = None

class footPrintDrawOverride(omr.MPxDrawOverride):
	@staticmethod
	def creator(obj):
		return footPrintDrawOverride(obj)

	@staticmethod
	def draw(context, data):
		## Get user draw data
		footData = data
		if not isinstance(footData, footPrintData):
			return

		## Get DAG object draw override
		objectOverrideInfo = footData.fDrawOV

		## Sample code to determine the rendering destination
		debugDestination = False
		if debugDestination:
			destination = context.renderingDestination()
			destinationType = "3d viewport"
			if destination[0] == omr.MFrameContext.k2dViewport:
				destinationType = "2d viewport"
			elif destination[0] == omr.MFrameContext.kImage:
				destinationType = "image"

			print "footprint node render destination is " + destinationType + ". Destination name=" + str(destination[1])

		## Just return and draw nothing, if it is overridden invisible
		if objectOverrideInfo.overrideEnabled and not objectOverrideInfo.enableVisible:
			return

		## Get display status
		displayStyle = context.getDisplayStyle()
		drawAsBoundingbox = (displayStyle & omr.MFrameContext.kBoundingBox) or (objectOverrideInfo.lod == om.MDAGDrawOverrideInfo.kLODBoundingBox)
		## If we don't want to draw the bounds within this plugin
		## manually, then skip drawing altogether in bounding box mode
		## since the bounds draw is handled by the renderer and
		## doesn't need to be drawn here.
		##
		if drawAsBoundingbox and not footData.fCustomBoxDraw:
			return

		animPlay = oma.MAnimControl.isPlaying()
		animScrub = oma.MAnimControl.isScrubbing()
		## If in playback but hidden in playback, skip drawing
		if (animPlay or animScrub) and not objectOverrideInfo.playbackVisible:
			return

		## For any viewport interactions switch to bounding box mode,
		## except when we are in playback.
		if context.inUserInteraction() or context.userChangingViewContext():
			if not animPlay and not animScrub:
				drawAsBoundingbox = True

		## Now, something gonna draw...

		## Check to see if we are drawing in a shadow pass.
		## If so then we keep the shading simple which in this
		## example means to disable any extra blending state changes
		##
		passCtx = context.getPassContext()
		passSemantics = passCtx.passSemantics()
		castingShadows = False
		for semantic in passSemantics:
			if semantic == omr.MPassContext.kShadowPassSemantic:
				castingShadows = True

		debugPassInformation = False
		if debugPassInformation:
			passId = passCtx.passIdentifier()
			print "footprint node drawing in pass[" + str(passId) + "], semantic[" + str(passSemantics) + "]"

		## get cached data
		multiplier = footData.fMultiplier
		color = [ footData.fColor[0], footData.fColor[1], footData.fColor[2], 1.0 ]

		requireBlending = False

		## If we're not casting shadows then do extra work
		## for display styles
		if not castingShadows:
			## Use some monotone version of color to show "default material mode"
			##
			if displayStyle & omr.MFrameContext.kDefaultMaterial:
				color[0] = color[1] = color[2] = (color[0] + color[1] + color[2]) / 3.0

			## Do some alpha blending if in x-ray mode
			##
			elif displayStyle & omr.MFrameContext.kXray:
				requireBlending = True
				color[3] = 0.3

		## Set blend and raster state
		##
		stateMgr = context.getStateManager()
		oldBlendState = None
		oldRasterState = None
		rasterStateModified = False

		if stateMgr is not None and (displayStyle & omr.MFrameContext.kGouraudShaded):
			## draw filled, and with blending if required
			if requireBlending:
				global blendState
				if blendState is None:
					desc = omr.MBlendStateDesc()
					desc.targetBlends[0].blendEnable = True
					desc.targetBlends[0].destinationBlend = omr.MBlendStatekInvSourceAlpha
					desc.targetBlends[0].alphaDestinationBlend = omr.MBlendStatekInvSourceAlpha
					blendState = stateMgr.acquireBlendState(desc)

				if blendState is not None:
					oldBlendState = stateMgr.getBlendState()
					stateMgr.setBlendState(blendState)

			## Override culling mode since we always want double-sided
			##
			oldRasterState = stateMgr.getRasterizerState()
			if oldRasterState:
				desc = oldRasterState.desc()
				## It's also possible to change this to kCullFront or kCullBack if we
				## wanted to set it to that.
				cullMode = omr.MRasterizerState.kCullNone
				if desc.cullMode != cullMode:
					global rasterState
					if rasterState is None:
						## Just override the cullmode
						desc.cullMode = cullMode
						rasterState = stateMgr.acquireRasterizerState(desc)

					if rasterState is not None:
						rasterStateModified = True
						stateMgr.setRasterizerState(rasterState)

		##========================
		## Start the draw work
		##========================

		## Prepare draw agent, default using OpenGL
		global drawAgent
		if drawAgent is None:
			if omr.MRenderer.drawAPIIsOpenGL():
				drawAgent = footPrintDrawAgentGL()
			else:
			 	drawAgent = footPrintDrawAgentDX()

		if not drawAgent is None:

			drawAgent.beginDraw( context, color, multiplier )

			if drawAsBoundingbox:
				drawAgent.drawBoundingBox( context )

			else:
				## Templated, only draw wirefame and it is not selectale
				overideTemplated = objectOverrideInfo.overrideEnabled and objectOverrideInfo.displayType == om.MDAGDrawOverrideInfo.kDisplayTypeTemplate
				## Override no shaded, only show wireframe
				overrideNoShaded = objectOverrideInfo.overrideEnabled and not objectOverrideInfo.enableShading

				if overideTemplated or overrideNoShaded:
					drawAgent.drawWireframe( context )

				else:
					if (displayStyle & omr.MFrameContext.kGouraudShaded) or (displayStyle & omr.MFrameContext.kTextured):
						drawAgent.drawShaded( context )

					if (displayStyle & omr.MFrameContext.kWireFrame):
						drawAgent.drawWireframe( context )

			drawAgent.endDraw( context )

		##========================
		## End the draw work
		##========================

		## Restore old blend state and old raster state
		if stateMgr is not None and (displayStyle & omr.MFrameContext.kGouraudShaded):
			if oldBlendState is not None:
				stateMgr.setBlendState(oldBlendState)

			if rasterStateModified and oldRasterState is not None:
				stateMgr.setRasterizerState(oldRasterState)

	def __init__(self, obj):
		omr.MPxDrawOverride.__init__(self, obj, footPrintDrawOverride.draw)

		## We want to perform custom bounding box drawing
		## so return True so that the internal rendering code
		## will not draw it for us.
		self.mCustomBoxDraw = True
		self.mCurrentBoundingBox = om.MBoundingBox()

	def supportedDrawAPIs(self):
		## this plugin supports both GL and DX
		return omr.MRenderer.kOpenGL | omr.MRenderer.kDirectX11

	def isBounded(self, objPath, cameraPath):
		return True

	def boundingBox(self, objPath, cameraPath):
		corner1 = om.MPoint( -0.17, 0.0, -0.7 )
		corner2 = om.MPoint( 0.17, 0.0, 0.3 )

		multiplier = self.getMultiplier(objPath)
		corner1 *= multiplier
		corner2 *= multiplier

		self.mCurrentBoundingBox.clear()
		self.mCurrentBoundingBox.expand( corner1 )
		self.mCurrentBoundingBox.expand( corner2 )

		return self.mCurrentBoundingBox

	def disableInternalBoundingBoxDraw(self):
		return self.mCustomBoxDraw

	def prepareForDraw(self, objPath, cameraPath, frameContext, oldData):
		## Retrieve data cache (create if does not exist)
		data = oldData
		if not isinstance(data, footPrintData):
			data = footPrintData()

		## compute data and cache it
		data.fMultiplier = self.getMultiplier(objPath)
		color = omr.MGeometryUtilities.wireframeColor(objPath)
		data.fColor = [ color.r, color.g, color.b ]
		data.fCustomBoxDraw = self.mCustomBoxDraw

		## Get the draw override information
		data.fDrawOV = objPath.getDrawOverrideInfo()

		return data

	def hasUIDrawables(self):
		return True

	def addUIDrawables(self, objPath, drawManager, frameContext, data):
		## Draw a text "Foot"
		pos = om.MPoint( 0.0, 0.0, 0.0 )  ## Position of the text
		textColor = om.MColor( (0.1, 0.8, 0.8, 1.0) )  ## Text color

		drawManager.beginDrawable()

		drawManager.setColor( textColor )
		drawManager.setFontSize( omr.MUIDrawManager.kSmallFontSize )
		drawManager.text(pos, "Footprint", omr.MUIDrawManager.kCenter )

		drawManager.endDrawable()

	def getMultiplier(self, objPath):
		## Retrieve value of the size attribute from the node
		footprintNode = objPath.node()
		plug = om.MPlug(footprintNode, footPrint.size)
		if not plug.isNull:
			sizeVal = plug.asMDistance()
			return sizeVal.asCentimeters()

		return 1.0

def initializePlugin(obj):
	plugin = om.MFnPlugin(obj, "Autodesk", "3.0", "Any")

	try:
		plugin.registerNode("footPrint", footPrint.id, footPrint.creator, footPrint.initialize, om.MPxNode.kLocatorNode, footPrint.drawDbClassification)
	except:
		sys.stderr.write("Failed to register node\n")
		raise

	try:
		omr.MDrawRegistry.registerDrawOverrideCreator(footPrint.drawDbClassification, footPrint.drawRegistrantId, footPrintDrawOverride.creator)
	except:
		sys.stderr.write("Failed to register override\n")
		raise

def uninitializePlugin(obj):
	plugin = om.MFnPlugin(obj)

	try:
		plugin.deregisterNode(footPrint.id)
	except:
		sys.stderr.write("Failed to deregister node\n")
		pass

	try:
		omr.MDrawRegistry.deregisterDrawOverrideCreator(footPrint.drawDbClassification, footPrint.drawRegistrantId)
	except:
		sys.stderr.write("Failed to deregister override\n")
		pass

