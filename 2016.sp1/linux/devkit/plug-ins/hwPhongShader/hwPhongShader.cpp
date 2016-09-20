//-
// ==========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

///////////////////////////////////////////////////////////////////
//
// NOTE: PLEASE READ THE README.TXT FILE FOR INSTRUCTIONS ON
// COMPILING AND USAGE REQUIREMENTS.
//
// DESCRIPTION:
//		This is an example of a using a cube-environment map
//		to perforce per pixel Phong shading.
//
//		The light direction is currently fixed at the eye
//		position. This could be changed to track an actual
//		light but has not been coded for this example.
//
//		If multiple lights are to be supported, than the environment
//		map would need to be looked up for each light either
//		using multitexturing or multipass.
//
///////////////////////////////////////////////////////////////////

#ifdef WIN32
#pragma warning( disable : 4786 )		// Disable STL warnings.
#endif

#include <maya/MIOStream.h>
#include <math.h>
#include <cstdlib>

#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFloatVector.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MSceneMessage.h>
#include <maya/MUserData.h>

#include <maya/MHWShaderSwatchGenerator.h>
#include <maya/MRenderUtilities.h>
#include <maya/MHardwareRenderer.h>
#include <maya/MGeometryData.h>
#include <maya/MImage.h>

#include <maya/MMatrix.h>

// Viewport 2.0 includes
#include <maya/MDrawRegistry.h>
#include <maya/MPxShaderOverride.h>
#include <maya/MDrawContext.h>
#include <maya/MStateManager.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MTextureManager.h>
#include <maya/MShaderManager.h>

#include <maya/MGLdefinitions.h>
#include <maya/MGLFunctionTable.h>

#undef ENABLE_TRACE_API_CALLS
#ifdef ENABLE_TRACE_API_CALLS
#define TRACE_API_CALLS(x) cerr << "hwPhongShader: "<<(x)<<"\n"
#else
#define TRACE_API_CALLS(x)
#endif

#ifndef GL_EXT_texture_cube_map
# define GL_NORMAL_MAP_EXT                   0x8511
# define GL_REFLECTION_MAP_EXT               0x8512
# define GL_TEXTURE_CUBE_MAP_EXT             0x8513
# define GL_TEXTURE_BINDING_CUBE_MAP_EXT     0x8514
# define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT  0x8515
# define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT  0x8516
# define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT  0x8517
# define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT  0x8518
# define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT  0x8519
# define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT  0x851A
# define GL_PROXY_TEXTURE_CUBE_MAP_EXT       0x851B
# define GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT    0x851C
#endif

#include "hwPhongShader.h"
#include "hwPhongShaderBehavior.h"

MTypeId hwPhongShader::id( 0x00105449 );

MObject  hwPhongShader::aColor;
MObject  hwPhongShader::aTransparency;
MObject  hwPhongShader::aDiffuseColor;
MObject  hwPhongShader::aSpecularColor;
MObject  hwPhongShader::aShininessX;
MObject  hwPhongShader::aShininessY;
MObject  hwPhongShader::aShininessZ;
MObject  hwPhongShader::aShininess;
MObject  hwPhongShader::aGeometryShape;

void hwPhongShader::postConstructor( )
{
	TRACE_API_CALLS("postConstructor");
	setMPSafe(false);
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////

void hwPhongShader::printGlError( const char *call )
{
    GLenum error;

	while( (error = glGetError()) != GL_NO_ERROR ) {
	    cerr << call << ":" << error << " is " << (const char *)gluErrorString( error ) << "\n";
	}
}


MFloatVector hwPhongShader::Phong ( double cos_a )
{
	MFloatVector p;

	if ( cos_a < 0.0 ) cos_a = 0.0;

	p[0] = (float)(mSpecularColor[0]*pow(cos_a,double(mShininess[0])) +
				   mDiffuseColor[0]*cos_a + mAmbientColor[0]);
	p[1] = (float)(mSpecularColor[1]*pow(cos_a,double(mShininess[1])) +
				   mDiffuseColor[1]*cos_a + mAmbientColor[1]);
	p[2] = (float)(mSpecularColor[2]*pow(cos_a,double(mShininess[2])) +
				   mDiffuseColor[2]*cos_a + mAmbientColor[2]);

	if ( p[0] > 1.0f ) p[0] = 1.0f;
	if ( p[1] > 1.0f ) p[1] = 1.0f;
	if ( p[2] > 1.0f ) p[2] = 1.0f;

	return p;
}

#define PHONG_TEXTURE_RES 256

static const GLenum faceTarget[6] =
{
	GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT
};

// Small reusable utility to bind and unbind a cube map texture by id.
class CubeMapTextureDrawUtility
{
public:
	static void bind( unsigned int phong_map_id )
	{
		// Setup up phong texture
		{
			glPushAttrib ( GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_TEXTURE_BIT | GL_TRANSFORM_BIT );

			glDisable ( GL_LIGHTING );
			glDisable ( GL_TEXTURE_1D );
			glDisable ( GL_TEXTURE_2D );

			// Setup cube map generation
			glEnable ( GL_TEXTURE_CUBE_MAP_EXT );
			glBindTexture ( GL_TEXTURE_CUBE_MAP_EXT, phong_map_id );
			glEnable ( GL_TEXTURE_GEN_S );
			glEnable ( GL_TEXTURE_GEN_T );
			glEnable ( GL_TEXTURE_GEN_R );
			glTexGeni ( GL_S, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_EXT );
			glTexGeni ( GL_T, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_EXT );
			glTexGeni ( GL_R, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_EXT );

			glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexEnvi ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

			// Setup texture matrix
			glMatrixMode ( GL_TEXTURE );
			glPushMatrix ();
			glLoadIdentity ();
			glMatrixMode ( GL_MODELVIEW );
		}
	}

	static void unbind()
	{
		// Restore texture matrix
		glMatrixMode ( GL_TEXTURE );
		glPopMatrix ();
		glMatrixMode ( GL_MODELVIEW );

		// Disable cube map texture (bind 0)
		glBindTexture( GL_TEXTURE_CUBE_MAP_EXT, 0 );
		glDisable ( GL_TEXTURE_CUBE_MAP_EXT );

		glPopAttrib();
	}
};

static void cubeToDir ( int face, double s, double t,
					  double &x, double &y, double &z )
//
// Description:
//		Map uv to cube direction
{
	switch ( face )
	{
		case 0:
			x = 1;
			y = -t;
			z = -s;
			break;
		case 1:
			x = -1;
			y = -t;
			z = s;
			break;
		case 2:
			x = s;
			y = 1;
			z = t;
			break;
		case 3:
			x = s;
			y = -1;
			z = -t;
			break;
		case 4:
			x = s;
			y = -t;
			z = 1;
			break;
		case 5:
			x = -s;
			y = -t;
			z = -1;
			break;
	}

	double invLen = 1.0 / sqrt ( x*x + y*y + z*z );
	x *= invLen;
	y *= invLen;
	z *= invLen;
}


void hwPhongShader::init_Phong_texture ( void )
//
// Description:
//		Set up a cube map for Phong lookup
//
{
	// There is nothing dirty, so don't rebuild the texture.
	if ( !mAttributesChanged && (phong_map_id != 0))
	{
		return;
	}

	GLubyte * texture_data;

	// Always release the old texture id before getting a
	// new one.
	if (phong_map_id != 0)
		glDeleteTextures( 1, &phong_map_id );
	glGenTextures ( 1, &phong_map_id );

	glEnable ( GL_TEXTURE_CUBE_MAP_EXT );
	glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );
	glPixelStorei ( GL_UNPACK_ROW_LENGTH, 0 );
	glBindTexture ( GL_TEXTURE_CUBE_MAP_EXT, phong_map_id );

	texture_data = new GLubyte[3*PHONG_TEXTURE_RES*PHONG_TEXTURE_RES];

	for ( int face=0 ; face<6 ; face++ )
	{
		int index = 0;
		for ( int j=0 ; j<PHONG_TEXTURE_RES ; j++ )
		{
			double t = 2*double(j)/(PHONG_TEXTURE_RES - 1) - 1;	// -1 to 1
			for ( int i=0 ; i<PHONG_TEXTURE_RES ; i++ )
			{
				double s = 2*double(i)/(PHONG_TEXTURE_RES - 1) - 1;	// -1 to 1
				double x = 0.0, y = 0.0, z = 0.0;
				cubeToDir ( face, s, t, x, y, z );

				MFloatVector intensity = Phong ( z );

				texture_data[index++] = (GLubyte)(255*intensity[0]);
				texture_data[index++] = (GLubyte)(255*intensity[1]);
				texture_data[index++] = (GLubyte)(255*intensity[2]);
			}
		}

		glTexImage2D ( faceTarget[face], 0, GL_RGB, PHONG_TEXTURE_RES, PHONG_TEXTURE_RES,
			0, GL_RGB, GL_UNSIGNED_BYTE, texture_data );

	}

	glDisable ( GL_TEXTURE_CUBE_MAP_EXT );

	delete [] texture_data;

	// Make sure to mark attributes "clean".
	mAttributesChanged = false;
}

void hwPhongShader::setTransparency(const float fTransparency)
{
	mTransparency = fTransparency; 
	mAttributesChanged = true;
	mAttributesChangedVP2 = true;
}

void hwPhongShader::setAmbient(const float3 &fambient) 
{ 
	mAmbientColor[0] = fambient[0];
	mAmbientColor[1] = fambient[1];
	mAmbientColor[2] = fambient[2];
	mAttributesChanged = true;
	mAttributesChangedVP2 = true;
}

void hwPhongShader::setDiffuse(const float3 &fDiffuse)
{
	mDiffuseColor[0] = fDiffuse[0];
	mDiffuseColor[1] = fDiffuse[1];
	mDiffuseColor[2] = fDiffuse[2];
	mAttributesChanged = true;
	mAttributesChangedVP2 = true;
}

void hwPhongShader::setSpecular(const float3 &fSpecular)
{
	mSpecularColor[0] = fSpecular[0];
	mSpecularColor[1] = fSpecular[1];
	mSpecularColor[2] = fSpecular[2];
	mAttributesChanged = true;
	mAttributesChangedVP2 = true;
}

void hwPhongShader::setShininess(const float3 &fShininess)
{
	mShininess[0] = fShininess[0];
	mShininess[1] = fShininess[1];
	mShininess[2] = fShininess[2];
	mAttributesChanged = true;
	mAttributesChangedVP2 = true;
}

hwPhongShader::hwPhongShader()
{
	TRACE_API_CALLS("hwPhongShader");
	attachSceneCallbacks();

	mAmbientColor[0] = mAmbientColor[1] = mAmbientColor[2] = 0.1f;
	mDiffuseColor[0] = mDiffuseColor[1] = mDiffuseColor[2] = 0.5f;
	mSpecularColor[0] = mSpecularColor[1] = mSpecularColor[2] = 0.5f;
	mShininess[0] = mShininess[1] = mShininess[2] = 100.0f;
	mAttributesChanged = false;
	markAttributesChangedVP2();

	phong_map_id = 0;
	mGeometryShape = 0;
	mTransparency = 0.0f;
}

hwPhongShader::~hwPhongShader()
{
	TRACE_API_CALLS("~hwPhongShader");
	detachSceneCallbacks();
}

void hwPhongShader::releaseEverything()
{
    if (phong_map_id != 0) {
		M3dView view = M3dView::active3dView();

        // The M3dView class doesn't return the correct status if there isn't
        // an active 3D view, so we rely on the success of beginGL() which
        // will make the context current.
        //
        if (view.beginGL()) {
		    glDeleteTextures( 1, &phong_map_id );

            phong_map_id = 0;
        }

        view.endGL();
    }
}

void hwPhongShader::attachSceneCallbacks()
{
	fBeforeNewCB  = MSceneMessage::addCallback(MSceneMessage::kBeforeNew,  releaseCallback, this);
	fBeforeOpenCB = MSceneMessage::addCallback(MSceneMessage::kBeforeOpen, releaseCallback, this);
	fBeforeRemoveReferenceCB = MSceneMessage::addCallback(MSceneMessage::kBeforeRemoveReference,
														  releaseCallback, this);
	fMayaExitingCB = MSceneMessage::addCallback(MSceneMessage::kMayaExiting, releaseCallback, this);
}

/*static*/
void hwPhongShader::releaseCallback(void* clientData)
{
	hwPhongShader *pThis = (hwPhongShader*) clientData;
	pThis->releaseEverything();
}

void hwPhongShader::detachSceneCallbacks()
{
	if (fBeforeNewCB)
		MMessage::removeCallback(fBeforeNewCB);
	if (fBeforeOpenCB)
		MMessage::removeCallback(fBeforeOpenCB);
	if (fBeforeRemoveReferenceCB)
		MMessage::removeCallback(fBeforeRemoveReferenceCB);
	if (fMayaExitingCB)
		MMessage::removeCallback(fMayaExitingCB);

	fBeforeNewCB = 0;
	fBeforeOpenCB = 0;
	fBeforeRemoveReferenceCB = 0;
	fMayaExitingCB = 0;
}


static const MString sHWPhongShaderRegistrantId("HWPhongShaderRegistrantId");

// Custom data for using Viewport 2.0 implementation of shader
class hwPhongShaderData : public MUserData
{
public:
	hwPhongShaderData() : MUserData(true) {}
	virtual ~hwPhongShaderData() {}

	MString fPath;
};

// Viewport 2.0 implementation for the shader
class hwPhongShaderOverride : public MHWRender::MPxShaderOverride
{
public:
	static MHWRender::MPxShaderOverride* Creator(const MObject& obj)
	{
		return new hwPhongShaderOverride(obj);
	}

	virtual ~hwPhongShaderOverride()
	{
		delete[] fTextureData;
		fTextureData = NULL;
		// It is possible for the MPxShaderOverride object to be deleted when
		// the fShaderNode is not.  If this happens we need to be sure that the
		// next hwPhongShaderOverride created sets the parameters on the shader
		// instance at least once, or we'll render with unititialized parameters.
		if (fShaderNode) fShaderNode->markAttributesChangedVP2();
		fShaderNode = NULL;

		MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
		if (theRenderer)
		{
			MHWRender::MTextureManager *theTextureManager = theRenderer->getTextureManager();
			if (theTextureManager)
			{
				if (fTexture)
				{
					theTextureManager->releaseTexture(fTexture);
				}
			}

			const MHWRender::MShaderManager* shaderMgr = theRenderer->getShaderManager();
			if (shaderMgr)
			{
				if (fColorShaderInstance)
				{
					shaderMgr->releaseShader(fColorShaderInstance);
				}
				if (fShadowShaderInstance)
				{
					shaderMgr->releaseShader(fShadowShaderInstance);
				}
				if (fNTColorShaderInstance)
				{
					shaderMgr->releaseShader(fNTColorShaderInstance);
				}
			}
		}
		fTexture = NULL;
		fColorShaderInstance = NULL;
		fShadowShaderInstance = NULL;
		fNTColorShaderInstance = NULL;
	}

	// Initialize phase
	virtual MString initialize(
		const MInitContext& initContext,
		MInitFeedback& initFeedback)
	{
		TRACE_API_CALLS("hwPhongShaderOverride::initialize");

		bool setRequirementsFromShader = false;
		if (fColorShaderInstance)
		{
			// Use the requirements for the color shader which is a superset
			// of the requirements for the color and shadow shadowers.
			//
			// - Generally this is the easiest way to set geometry requirements since they
			// are taken directly from the shader. To use the custom primitive types
			// branch this has been disabled so that explicit requirments can be set.
			// - The type and format should be made to exactly match the shader.
			if (setRequirementsFromShader)
				setGeometryRequirements( *fColorShaderInstance );

			// This plugin is using the utility method
			// MPxShaderOverride::drawGeometry(). For DX11 drawing,
			// a shader signature is required. We use
			// the signature from the same MShaderInstance used to
			// set the geometry requirements so that the signature
			// will match the requirements.
			//
			addShaderSignature( *fColorShaderInstance );
		}

		if (!setRequirementsFromShader)
		{
			// Set the geometry requirements for drawing. Only need
			// position and normals.
			MString empty;

			// custom primitive types can be used by shader overrides.
			// This code is a simple example to show the mechanics of how that works.
			// Here we declare a custom custom indexing requirements.
			// The name "customPrimitiveTest" will be used to look up a registered
			// MPxPrimitiveGenerator that will handle the generation of the index buffer.
			// The example primitive generator is registered at startup by this plugin.
			//
			// As part of this example the plugin customPrimitiveGenerator should
			// be loaded. Note that this plugin will only handle polygonal meshes
			// where "Smooth Mesh Preview" has not be turned on. That is it will only
			// rebuild indexing apropriately for the original non-smoothed mesh.
			//
			static bool useCustomPrimitiveGenerator = (getenv("MAYA_USE_CUSTOMPRIMITIVEGENERATOR") != NULL);
			if(useCustomPrimitiveGenerator)
			{
				MString customPrimitiveName("customPrimitiveTest");
				MHWRender::MIndexBufferDescriptor indexingRequirement(
					MHWRender::MIndexBufferDescriptor::kCustom,
					customPrimitiveName,
					MHWRender::MGeometry::kTriangles);

				addIndexingRequirement(indexingRequirement);

				MHWRender::MVertexBufferDescriptor positionDesc(
					empty,
					MHWRender::MGeometry::kPosition,
					MHWRender::MGeometry::kFloat,
					3);
				positionDesc.setSemanticName("customPositionStream");

				MHWRender::MVertexBufferDescriptor normalDesc(
					empty,
					MHWRender::MGeometry::kNormal,
					MHWRender::MGeometry::kFloat,
					3);
				normalDesc.setSemanticName("customNormalStream");

				addGeometryRequirement(positionDesc);
				addGeometryRequirement(normalDesc);
			}
			else
			{
				MHWRender::MVertexBufferDescriptor positionDesc(
					empty,
					MHWRender::MGeometry::kPosition,
					MHWRender::MGeometry::kFloat,
					3);
				// Use the custom semantic name "swizzlePosition"
				// When the vertexBufferMutator plugin is loaded,
				// this will swap the x,y and z values of the vertex buffer.
				positionDesc.setSemanticName("swizzlePosition");

				MHWRender::MVertexBufferDescriptor normalDesc(
					empty,
					MHWRender::MGeometry::kNormal,
					MHWRender::MGeometry::kFloat,
					3);

				addGeometryRequirement(positionDesc);
				addGeometryRequirement(normalDesc);
			}
		}

		// Store path name as string to show Maya source on draw
		// NOTE 1: We cannot use the path to access the DAG object during the
		//         draw callback since that could trigger DG evaluation, we
		//         just use the string for debugging info.
		// NOTE 2: Adding custom data here makes consolidation of objects
		//         sharing the same hwPhongShader impossible. Performance will
		//         suffer in that case.
		hwPhongShaderData* data = new hwPhongShaderData();
		data->fPath = initContext.dagPath.fullPathName();
		initFeedback.customData = data;

        return MString("Autodesk Maya hwPhongShaderOverride");
    }

	// Update phase
	virtual void updateDG(MObject object)
	{
		TRACE_API_CALLS("hwPhongShaderOverride::updateDG");

		if (object != MObject::kNullObj)
		{
			// Get the hardware shader node from the MObject.
			fShaderNode = (hwPhongShader *) MPxHwShaderNode::getHwShaderNodePtr( object );
			
			if (fShaderNode)
			{
				MStatus status;
			    MFnDependencyNode node(object, &status);

				if (status)
				{
					node.findPlug("transparency").getValue(fTransparency);
					fShaderNode->setTransparency(fTransparency);

					node.findPlug("colorR").getValue(fAmbient[0]);
					node.findPlug("colorG").getValue(fAmbient[1]);
					node.findPlug("colorB").getValue(fAmbient[2]);
					fShaderNode->setAmbient(fAmbient);

					node.findPlug("diffuseColorR").getValue(fDiffuse[0]);
					node.findPlug("diffuseColorG").getValue(fDiffuse[1]);
					node.findPlug("diffuseColorB").getValue(fDiffuse[2]);
					float3 fDiffuse_RGB = {fDiffuse[0], fDiffuse[1], fDiffuse[2]};
					fShaderNode->setDiffuse(fDiffuse_RGB);
					fDiffuse[3] = 1.0f - fTransparency;
					
					node.findPlug("specularColorR").getValue(fSpecular[0]);
					node.findPlug("specularColorG").getValue(fSpecular[1]);
					node.findPlug("specularColorB").getValue(fSpecular[2]);
					fShaderNode->setSpecular(fSpecular);

					node.findPlug("shininessX").getValue(fShininess[0]);
					node.findPlug("shininessY").getValue(fShininess[1]);
					node.findPlug("shininessZ").getValue(fShininess[2]);
					fShaderNode->setShininess(fShininess);
				}
			}
			else
			{
				fTransparency = 0.0f;
			}	
		}
	}
	virtual void updateDevice()
	{
		TRACE_API_CALLS("hwPhongShaderOverride::updateDevice");

		if (fDrawUsingShader)
			updateShaderInstance();

		if (!fColorShaderInstance)
			rebuildTexture();
	}
	virtual void endUpdate()
	{
		TRACE_API_CALLS("hwPhongShaderOverride::endUpdate");
	}

	// Draw phase
	virtual bool handlesDraw(MHWRender::MDrawContext& context)
	{
		const MHWRender::MPassContext & passCtx = context.getPassContext();
		const MString & passId = passCtx.passIdentifier();
		const MStringArray & passSem = passCtx.passSemantics();

		fInShadowPass = false;
		fInColorPass = false;

		// Enable to debug what is occuring in handlesDraw()
		//
		bool debugHandlesDraw = false;
		if (debugHandlesDraw)
			printf("In hwPhong shader handlesDraw(). Pass Identifier = %s\n", passId.asChar());
		bool handlePass = false;
		for (unsigned int i=0; i<passSem.length(); i++)
		{
			if (passSem[i] == MHWRender::MPassContext::kColorPassSemantic)
			{
				bool hasOverrideShader = passCtx.hasShaderOverride();
				if (!hasOverrideShader)
				{
					if (debugHandlesDraw)
						printf("-> handle semantic[%d][%s]\n", i, passSem[i].asChar());
					handlePass = true;
					fInColorPass = true;
				}
			}
			else if (passSem[i] == MHWRender::MPassContext::kShadowPassSemantic)
			{
				// Only if we can load in a shadow shader will we handle
				// the shadow pass.
				//
				if (fShadowShaderInstance)
					handlePass = true;
				if (debugHandlesDraw)
					printf("-> handle semantic[%d][%s] = %d\n", i, passSem[i].asChar(), handlePass);

				// Remember that we are currently drawing in a shadow pass
				fInShadowPass = true;
			}
			else if (passSem[i] == MHWRender::MPassContext::kDepthPassSemantic)
			{
				if (debugHandlesDraw)
					printf("-> don't handle semantic[%d][%s]\n", i, passSem[i].asChar());
				handlePass = false;
			}
			else if (passSem[i] == MHWRender::MPassContext::kNormalDepthPassSemantic)
			{
				if (debugHandlesDraw)
					printf("-> don't handle semantic[%d][%s]\n", i, passSem[i].asChar());
				handlePass = false;
			}
			else
			{
				if (debugHandlesDraw)
					printf("-> additional semantic[%d][%s]\n", i, passSem[i].asChar());
			}
		}

		// Any other passes, don't override drawing
		//
		return handlePass;
	}

	virtual MHWRender::MShaderInstance* shaderInstance() const
	{
		if (fDrawUsingShader)
		{
			if (fInColorPass)
			{
				// If color pass
				if (fColorShaderInstance)
					return fColorShaderInstance;
			}
			else if (fInShadowPass)
			{
				// If shadow pass
				if (fShadowShaderInstance)
					return fShadowShaderInstance;
			}
		}
		return NULL;
	}

	// Example of using an MShaderInstance for draw.
	// We make the activation occur in activateKey() and
	// the termination occur in terminateKey().
    virtual void activateKey(MHWRender::MDrawContext& context, const MString& key)
	{
		TRACE_API_CALLS("hwPhongShaderOverride::activateKey");

		fShaderBound = false;
		if (fDrawUsingShader)
		{
			if (fInColorPass && fColorShaderInstance)
			{
				fColorShaderInstance->bind( context );
				fShaderBound = true;
			}
			else if (fInShadowPass && fShadowShaderInstance)
			{
				// Update the parameters on the shadow shader
				MMatrix viewProj = context.getMatrix(MHWRender::MFrameContext::kViewProjMtx);
				fShadowShaderInstance->setParameter("shadowViewProj", viewProj );
				fShadowShaderInstance->bind( context );
				fShaderBound = true;
			}
		}
	}

	// Example of using MShaderInstace to draw. Terminate
	// the shader instance here.
    virtual void terminateKey(MHWRender::MDrawContext& context, const MString& key)
	{
		TRACE_API_CALLS("hwPhongShaderOverride::terminateKey");

		if (fShaderBound)
		{
			if (fInColorPass && fColorShaderInstance)
			{
				fColorShaderInstance->unbind( context );
			}
			else if (fInShadowPass && fShadowShaderInstance)
			{
				fShadowShaderInstance->unbind( context );
			}
		}
		fShaderBound = false;
	}

	virtual bool draw(
		MHWRender::MDrawContext& context,
		const MHWRender::MRenderItemList& renderItemList) const
	{
		TRACE_API_CALLS("hwPhongShaderOverride::draw");

		MHWRender::MStateManager* stateMgr = context.getStateManager();

		//initialize hwPhongShader blend state once
		if(sBlendState == NULL)
		{
			//initilize blend state
			MHWRender::MBlendStateDesc blendStateDesc;

			for(int i = 0; i < (blendStateDesc.independentBlendEnable ? MHWRender::MBlendState::kMaxTargets : 1); ++i)
			{
				blendStateDesc.targetBlends[i].blendEnable = true;
	   			blendStateDesc.targetBlends[i].sourceBlend = MHWRender::MBlendState::kSourceAlpha;
				blendStateDesc.targetBlends[i].destinationBlend = MHWRender::MBlendState::kInvSourceAlpha;
				blendStateDesc.targetBlends[i].blendOperation = MHWRender::MBlendState::kAdd;
	   			blendStateDesc.targetBlends[i].alphaSourceBlend = MHWRender::MBlendState::kOne;
				blendStateDesc.targetBlends[i].alphaDestinationBlend = MHWRender::MBlendState::kInvSourceAlpha;
				blendStateDesc.targetBlends[i].alphaBlendOperation = MHWRender::MBlendState::kAdd;
			}

			blendStateDesc.blendFactor[0] = 1.0f;
			blendStateDesc.blendFactor[1] = 1.0f;
			blendStateDesc.blendFactor[2] = 1.0f;
			blendStateDesc.blendFactor[3] = 1.0f;
			sBlendState = stateMgr->acquireBlendState(blendStateDesc);
		}

		int val = 0;
		bool debugDrawContext = false;
		if (MGlobal::getOptionVarValue("hwPhong_debugDrawContext", val))
		{
			debugDrawContext = (val > 0);
		}
		if (debugDrawContext)
			printContextInformation(context);

		const MHWRender::MPassContext & passCtx = context.getPassContext();
		const MStringArray & passSem = passCtx.passSemantics();
		bool debugPassInformation = false;
		if (debugPassInformation)
		{
			const MString & passId = passCtx.passIdentifier();
			printf("hwPhong node drawing in pass[%s], semantic[", passId.asChar());
			for (unsigned int i=0; i<passSem.length(); i++)
				printf(" %s", passSem[i].asChar());
			printf(" ]\n");
		}

		// save old render state
		const MHWRender::MBlendState* pOldBlendState = stateMgr->getBlendState();

		// Have a MShaderInstance bound then just need to draw the geometry
		//
		if (fShaderBound)
		{
			// Draw for color pass
			if (fInColorPass)
			{
				bool needBlending = false;
				if (fTransparency > 0.0f)
				{
					needBlending = true;
					stateMgr->setBlendState(sBlendState);
				}
				unsigned int passCount = fColorShaderInstance->getPassCount( context );
				if (passCount)
				{
					for (unsigned int i=0; i<passCount; i++)
					{
						fColorShaderInstance->activatePass( context, i );
						MHWRender::MPxShaderOverride::drawGeometry(context);
					}
				}
				if (needBlending)
				{
					stateMgr->setBlendState(pOldBlendState);
				}
			}

			// Draw for shadow pass
			else if (fInShadowPass)
			{
				unsigned int passCount = fShadowShaderInstance->getPassCount( context );
				if (passCount)
				{
					for (unsigned int i=0; i<passCount; i++)
					{
						fShadowShaderInstance->activatePass( context, i );
						MHWRender::MPxShaderOverride::drawGeometry(context);
					}
				}
			}

		}

		// Use old method of a cube-map texture to draw
		//
		else
		{
			int phongTextureId = 0;
			MHWRender::MRenderer *theRenderer = MHWRender::MRenderer::theRenderer();
			if (theRenderer)
			{
				MHWRender::MTextureManager *theTextureManager = theRenderer->getTextureManager();
				if(theTextureManager)
				{
					if(fTexture)
					{
						void *idPtr = fTexture->resourceHandle();
						if (idPtr)
						{
							phongTextureId = *((int *)idPtr);
						}
					}
				}
			}
			if (phongTextureId == 0)
			{
				return false;
			}

			// Setup the matrix to draw the object in world space
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			MStatus status;
			MMatrix transform =
				context.getMatrix(MHWRender::MFrameContext::kWorldViewMtx, &status);
			if (status)
			{
				glLoadMatrixd(transform.matrix[0]);
			}

			// set projection matrix
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			MMatrix projection =
				context.getMatrix(MHWRender::MFrameContext::kProjectionMtx, &status);
			if (status)
			{
				glLoadMatrixd(projection.matrix[0]);
			}

			bool needBlending = false;
			if (fTransparency > 0.0f)
			{
				needBlending = true;
				stateMgr->setBlendState(sBlendState);
				glColor4f(1.0, 1.0, 1.0, 1.0f-fTransparency );
			}
			else
				glColor4f(1.0, 1.0, 1.0, 1.0f);

			// Setup up phong texture
			CubeMapTextureDrawUtility::bind( phongTextureId );

			// Trigger geometric draw
			bool debugGeometricDraw = false; // Set to true to debug
			if (debugGeometricDraw)
			{
				// Debugging code to see what is being sent down to draw
				// by MDrawContext::drawGeometry().
				//
				// The sample draw code does basically what is done internally.
				//
				customDraw(context, renderItemList);
			}
			else
			{
				// Draw the geometry through the internal interface instead
				// of drawing via the plugin.
				MHWRender::MPxShaderOverride::drawGeometry(context);
			}

			// Disable texture state
			CubeMapTextureDrawUtility::unbind();

			if (needBlending)
			{
				glColor4f(1.0, 1.0, 1.0, 1.0 );
				//restore blend state
				stateMgr->setBlendState(pOldBlendState);
			}
			glPopMatrix();

			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
		}
		return true;
	}

	// Custom draw. OpenGL version only
	//
	void customDraw(
		MHWRender::MDrawContext& context,
		const MHWRender::MRenderItemList& renderItemList) const
	{
		static MGLFunctionTable *gGLFT = 0;
		if ( 0 == gGLFT )
			gGLFT = MHardwareRenderer::theRenderer()->glFunctionTable();

		MGLenum currentError = 0;

		glPushClientAttrib ( GL_CLIENT_ALL_ATTRIB_BITS );
		{
			int numRenderItems = renderItemList.length();
			for (int renderItemIdx=0; renderItemIdx<numRenderItems; renderItemIdx++)
			{
				const MHWRender::MRenderItem* renderItem = renderItemList.itemAt(renderItemIdx);
				if (!renderItem) continue;
				const MHWRender::MGeometry* geometry = renderItem->geometry();
				if (!geometry) continue;

				hwPhongShaderData* phongData = dynamic_cast<hwPhongShaderData*>(renderItem->customData());
				if (phongData)
				{
					fprintf(stderr, "Source object path=%s\n", phongData->fPath.asChar());
				}

				// Dump out vertex field information for each field
				//
				int bufferCount = geometry->vertexBufferCount();

				#define GLOBJECT_BUFFER_OFFSET(i) ((char *)NULL + (i)) // For GLObject offsets

				bool boundData = true;
				for (int i=0; i<bufferCount && boundData; i++)
				{
					const MHWRender::MVertexBuffer* buffer = geometry->vertexBuffer(i);
					if (!buffer)
					{
						boundData = false;
						continue;
					}
					const MHWRender::MVertexBufferDescriptor& desc = buffer->descriptor();
					GLuint * dataBufferId = NULL;
					void *dataHandle = buffer->resourceHandle();
					if (!dataHandle)
					{
						boundData = false;
						continue;
					}
					dataBufferId = (GLuint *)(dataHandle);

					unsigned int fieldOffset = desc.offset();
					unsigned int fieldStride = desc.stride();
					{
						fprintf(stderr, "Buffer(%d), Name(%s), BufferType(%s), BufferDimension(%d), BufferSemantic(%s), Offset(%d), Stride(%d), Handle(%d)\n",
							i,
							desc.name().asChar(),
							MHWRender::MGeometry::dataTypeString(desc.dataType()).asChar(),
							desc.dimension(),
							MHWRender::MGeometry::semanticString(desc.semantic()).asChar(),
							fieldOffset,
							fieldStride,
							*dataBufferId);
					}

					// Bind each data buffer
					if (*dataBufferId > 0)
					{
						gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, *dataBufferId);
						currentError = gGLFT->glGetError();
						if (currentError != MGL_NO_ERROR)
							boundData = false;
					}
					else
						boundData = false;

					if (boundData)
					{
						// Set the data pointers
						if (desc.semantic() == MHWRender::MGeometry::kPosition)
						{
							glEnableClientState(GL_VERTEX_ARRAY);
							glVertexPointer(3, GL_FLOAT, fieldStride*4, GLOBJECT_BUFFER_OFFSET(fieldOffset));
							currentError = gGLFT->glGetError();
							if (currentError != MGL_NO_ERROR)
								boundData = false;
						}
						else if (desc.semantic() == MHWRender::MGeometry::kNormal)
						{
							glEnableClientState(GL_NORMAL_ARRAY);
							glNormalPointer(GL_FLOAT, fieldStride*4, GLOBJECT_BUFFER_OFFSET(fieldOffset));
							currentError = gGLFT->glGetError();
							if (currentError != MGL_NO_ERROR)
								boundData = false;
						}
					}
				}

				if (boundData && geometry->indexBufferCount() > 0)
				{
					// Dump out indexing information
					//
					const MHWRender::MIndexBuffer* buffer = geometry->indexBuffer(0);
					void *indexHandle = buffer->resourceHandle();
					unsigned int indexBufferCount = 0;
					GLuint *indexBufferId = NULL;
					MHWRender::MGeometry::Primitive indexPrimType = renderItem->primitive();
					if (indexHandle)
					{
						indexBufferId = (GLuint *)(indexHandle);
						indexBufferCount = buffer->size();
						{
							fprintf(stderr, "IndexingPrimType(%s), IndexType(%s), IndexCount(%d), Handle(%d)\n",
								MHWRender::MGeometry::primitiveString(indexPrimType).asChar(),
								MHWRender::MGeometry::dataTypeString(buffer->dataType()).asChar(),
								indexBufferCount,
								*indexBufferId);
						}
					}

					// Bind the index buffer
					if (indexBufferId  && (*indexBufferId > 0))
					{
						gGLFT->glBindBufferARB(MGL_ELEMENT_ARRAY_BUFFER_ARB, *indexBufferId);
						currentError = gGLFT->glGetError();
						if (currentError == MGL_NO_ERROR)
						{
							GLenum indexPrimTypeGL = GL_TRIANGLES;
							switch (indexPrimType) {
							case MHWRender::MGeometry::kPoints:
								indexPrimTypeGL = GL_POINTS; break;
							case MHWRender::MGeometry::kLines:
								indexPrimTypeGL = GL_LINES; break;
							case MHWRender::MGeometry::kLineStrip:
								indexPrimTypeGL = GL_LINE_STRIP; break;
							case MHWRender::MGeometry::kTriangles:
								indexPrimTypeGL = GL_TRIANGLES; break;
							case MHWRender::MGeometry::kTriangleStrip:
								indexPrimTypeGL = GL_TRIANGLE_STRIP; break;
							default:
								boundData = false;
								break;
							};
							if (boundData)
							{
								// Draw the geometry
								GLenum indexType =
									( buffer->dataType() == MHWRender::MGeometry::kUnsignedInt32  ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT );
								glDrawElements(indexPrimTypeGL, indexBufferCount, indexType, GLOBJECT_BUFFER_OFFSET(0));
							}
						}
					}
				}
			}
		}
		glPopClientAttrib();
	}

	virtual MHWRender::DrawAPI supportedDrawAPIs() const
	{
		// Using a custom internal shader means we can draw in OpenGL
		// and DirectX.
		if (fDrawUsingShader)
			return (MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile);
		else
			return MHWRender::kOpenGL;
	}

	virtual bool isTransparent()
	{
		TRACE_API_CALLS("hwPhongShaderOverride::isTransparent");
		if (fShaderNode)
		{
			return (fTransparency > 0.0f);
		}
		return false;
	}

	// Set a non-texture mode shader which is fixed.
	virtual MHWRender::MShaderInstance* nonTexturedShaderInstance(bool &monitor) const
	{
		if (fNTColorShaderInstance)
		{
			monitor = false;
			return fNTColorShaderInstance;
		}
		return NULL;
	}

	virtual bool overridesDrawState()
	{
		return true;
	}

	void debugShaderParameters(const MHWRender::MShaderInstance *shaderInstance)
	{
		MStringArray params;
		shaderInstance->parameterList(params);

		unsigned int numParams = params.length();
		printf("DEBUGGING SHADER, BEGIN PARAM LIST OF LENGTH %d\n", numParams);
		for (unsigned int i=0; i<numParams; i++)
		{
			printf("ParamName='%s', ParamType=", params[i].asChar());
			switch (shaderInstance->parameterType(params[i]))
			{
			case MHWRender::MShaderInstance::kInvalid:
				printf("'Invalid', ");
				break;
			case MHWRender::MShaderInstance::kBoolean:
				printf("'Boolean', ");
				break;
			case MHWRender::MShaderInstance::kInteger:
				printf("'Integer', ");
				break;
			case MHWRender::MShaderInstance::kFloat:
				printf("'Float', ");
				break;
			case MHWRender::MShaderInstance::kFloat2:
				printf("'Float2', ");
				break;
			case MHWRender::MShaderInstance::kFloat3:
				printf("'Float3', ");
				break;
			case MHWRender::MShaderInstance::kFloat4:
				printf("'Float4', ");
				break;
			case MHWRender::MShaderInstance::kFloat4x4Row:
				printf("'Float4x4Row', ");
				break;
			case MHWRender::MShaderInstance::kFloat4x4Col:
				printf("'Float4x4Col', ");
				break;
			case MHWRender::MShaderInstance::kTexture1:
				printf("'1D Texture', ");
				break;
			case MHWRender::MShaderInstance::kTexture2:
				printf("'2D Texture', ");
				break;
			case MHWRender::MShaderInstance::kTexture3:
				printf("'3D Texture', ");
				break;
			case MHWRender::MShaderInstance::kTextureCube:
				printf("'Cube Texture', ");
				break;
			case MHWRender::MShaderInstance::kSampler:
				printf("'Sampler', ");
				break;
			default:
				printf("'Unknown', ");
				break;
			}
			printf("IsArrayParameter='%s'\n", shaderInstance->isArrayParameter(params[i]) ? "YES" : "NO");
		}
		printf("END PARAM LIST\n");
	}

	// Code to create a cached MShaderInstance
	void createShaderInstance()
	{
		TRACE_API_CALLS("hwPhongShaderOverride::createShaderInstance");

		MHWRender::MRenderer *renderer = MHWRender::MRenderer::theRenderer();
		if (!renderer)
			return;

		const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
		if (!shaderMgr)
			return;

		bool debugShader = false;

		// If no shadow shader instance created yet acquire one. Use
		// the stock shadow shader provided.
		//
		if (!fShadowShaderInstance)
		{
			fShadowShaderInstance = shaderMgr->getStockShader( MHWRender::MShaderManager::k3dShadowerShader );

			// Some sample debugging code to see what parameters are available.
			if (fShadowShaderInstance)
			{
				if (debugShader)
					debugShaderParameters( fShadowShaderInstance );
			}
			else
			{
				fprintf(stderr, "Failed to load shadower shader for hwPhong\n");
			}
		}

		// If no color shader instance created yet acquire one. For
		// now it's just using an internal shader for convenience but
		// a custom shader could be written here as well.
		if (!fColorShaderInstance)
		{
			fColorShaderInstance = shaderMgr->getStockShader( MHWRender::MShaderManager::k3dBlinnShader );

			// Some sample debugging code to see what parameters are available.
			if (fColorShaderInstance && debugShader)
			{
				debugShaderParameters( fColorShaderInstance );
			}
		}
		if (!fNTColorShaderInstance)
		{
			fNTColorShaderInstance = shaderMgr->getStockShader( MHWRender::MShaderManager::k3dBlinnShader );
			if (fNTColorShaderInstance)
			{
				float val[4] = { 0.3f, 0.5f, 1.0f, 1.0f };
				fNTColorShaderInstance->setParameter("diffuseColor", &val[0]);
			}
		}
	}

	// Code to update a cached MShaderInstance
	//
	void updateShaderInstance()
	{
		TRACE_API_CALLS("hwPhongShaderOverride::updateShaderInstance");

		MHWRender::MRenderer *renderer = MHWRender::MRenderer::theRenderer();
		if (!renderer)
			return;

		// Update the parameters on the color shader
		//
		if (fColorShaderInstance)
		{
			// Update shader to mark it as drawing with transparency or not.
			fColorShaderInstance->setIsTransparent( isTransparent() );

			if (fShaderNode && fShaderNode->attributesChangedVP2())
			{
				fColorShaderInstance->setParameter("emissionColor", &fAmbient[0] );
				fColorShaderInstance->setParameter("diffuseColor", &fDiffuse[0] );
				fColorShaderInstance->setParameter("specularColor", &fSpecular[0] );
				// "specularPower" is set using single-float argument version of setParameter()
				float specPower = fShininess[0];
				fColorShaderInstance->setParameter("specularPower", specPower );

				fShaderNode->markAttributesCleanVP2();
			}
		}
	}

	// Code to recreate a new texture on parameter change.
	// Not used when using a shader to draw with.
	void rebuildTexture()
	{
		TRACE_API_CALLS("hwPhongShaderOverride::rebuildTexture");

		// Rebuild the hardware Phong texture if needed
		if (fShaderNode && fShaderNode->attributesChangedVP2())
		{
			MHWRender::MRenderer *theRenderer = MHWRender::MRenderer::theRenderer();
			if (theRenderer)
			{
				MHWRender::MTextureManager *theTextureManager = theRenderer ? theRenderer->getTextureManager() : NULL;
				// Set a unique identifier for this texture
				// based on the shading parameters used to create
				// the texture
				//
				MString newTextureName = MString("MyPhongCubeMap__");
				const float3 * amb = fShaderNode->Ambient();
				const float3 * diff = fShaderNode->Diffuse();
				const float3 * spec= fShaderNode->Specular();
				const float3 * shininess = fShaderNode->Shininess();

				newTextureName += (*amb)[0]; newTextureName += MString("_");
				newTextureName += (*amb)[1]; newTextureName += MString("_");
				newTextureName += (*amb)[2]; newTextureName += MString("_");
				newTextureName += (*diff)[0]; newTextureName += MString("_");
				newTextureName += (*diff)[1]; newTextureName += MString("_");
				newTextureName += (*diff)[2]; newTextureName += MString("_");
				newTextureName += (*spec)[0]; newTextureName += MString("_");
				newTextureName += (*spec)[1]; newTextureName += MString("_");
				newTextureName += (*spec)[2]; newTextureName += MString("_");
				newTextureName += (*shininess)[0]; newTextureName += MString("_");
				newTextureName += (*shininess)[1]; newTextureName += MString("_");
				newTextureName += (*shininess)[2];

				// Release the old one, and set the new name
				//
				if(fTexture)
				{
					if (theTextureManager)
					{
						theTextureManager->releaseTexture(fTexture);
					}
					fTexture = NULL;
				}

				// We create 1 contiguous block of data for the texture.
				//
				if (!fTextureData)
				{
					fTextureData =
						new unsigned char[4*PHONG_TEXTURE_RES*PHONG_TEXTURE_RES*6];
				}
				if (fTextureData)
				{
					int index = 0;
					for ( int face=0 ; face<6 ; face++ )
					{
						for ( int j=0 ; j<PHONG_TEXTURE_RES ; j++ )
						{
							double t = 2*double(j)/(PHONG_TEXTURE_RES - 1) - 1;	// -1 to 1
							for ( int i=0 ; i<PHONG_TEXTURE_RES ; i++ )
							{
								double s = 2*double(i)/(PHONG_TEXTURE_RES - 1) - 1;	// -1 to 1
								double x = 0.0, y = 0.0, z = 0.0;
								cubeToDir ( face, s, t, x, y, z );

								MFloatVector intensity = fShaderNode->Phong ( z );

								fTextureData[index++] = (unsigned char)(255*intensity[0]);
								fTextureData[index++] = (unsigned char)(255*intensity[1]);
								fTextureData[index++] = (unsigned char)(255*intensity[2]);
								fTextureData[index++] = 255;
							}
						}
					}
					MHWRender::MTextureDescription desc;
					{
						desc.setToDefault2DTexture();
						desc.fWidth = PHONG_TEXTURE_RES;
						desc.fHeight = PHONG_TEXTURE_RES;
						desc.fDepth = 1;
						desc.fBytesPerRow = 4*PHONG_TEXTURE_RES;
						desc.fBytesPerSlice = 4*PHONG_TEXTURE_RES*PHONG_TEXTURE_RES;
						desc.fMipmaps = 1;
						desc.fArraySlices = 6;
						desc.fFormat = MHWRender::kR8G8B8A8_UNORM;
						desc.fTextureType = MHWRender::kCubeMap;
					}

					if (theTextureManager)
					{
						fTexture = theTextureManager->acquireTexture( newTextureName, desc, fTextureData );
					}
				}

				// Mark the texture clean
				fShaderNode->markAttributesCleanVP2();
			}
		}
	}

	// Utility code to print the current context information
	//
	static void printContextInformation( const MHWRender::MDrawContext & context )
	{
		TRACE_API_CALLS("hwPhongShaderOverride::printContextInformation");

		// Sample code to print out the information found in MDrawContext
		//
		MDoubleArray dtuple;
		printf("Draw Context Diagnostics {\n");
		{
			dtuple = context.getTuple( MHWRender::MFrameContext::kViewPosition );
			printf("\tView position: %f, %f, %f\n", dtuple[0], dtuple[1], dtuple[2]);
			dtuple = context.getTuple( MHWRender::MFrameContext::kViewPosition );
			printf("\tView dir : %f, %f, %f\n", dtuple[0], dtuple[1], dtuple[2]);
			dtuple = context.getTuple( MHWRender::MFrameContext::kViewUp );
			printf("\tView up : %f, %f, %f\n", dtuple[0], dtuple[1], dtuple[2]);
			dtuple = context.getTuple( MHWRender::MFrameContext::kViewRight );
			printf("\tView right : %f, %f, %f\n", dtuple[0], dtuple[1], dtuple[2]);
			printf("\n");

			MBoundingBox bbox = context.getSceneBox();
			MPoint bmin = bbox.min();
			MPoint bmax = bbox.max();
			printf("\tScene bounding box = %g,%g,%g -> %g,%g,%g\n",
				bmin[0],bmin[1],bmin[2],
				bmax[0],bmax[1],bmax[2]);

			int originX; int originY; int width; int height;
			context.getRenderTargetSize( width, height );
			printf("\tRender target size: %d x %d\n", width, height);
			context.getViewportDimensions( originX, originY, width, height);
			printf("\tViewport dimensions: %d, %d, -> %d, %d\n", originX, originY, width, height);
			MStatus xStatus;
			printf("\tView direction along neg z = %d\n", context.viewDirectionAlongNegZ( &xStatus ));

			// Flag to test getting all scene lights or the subset used for lighting
			//
			static MHWRender::MDrawContext::LightFilter considerAllSceneLights = MHWRender::MDrawContext::kFilteredToLightLimit;
			if (considerAllSceneLights == MHWRender::MDrawContext::kFilteredToLightLimit)
				considerAllSceneLights = MHWRender::MDrawContext::kFilteredIgnoreLightLimit;
			else
				considerAllSceneLights = MHWRender::MDrawContext::kFilteredToLightLimit;

			printf("\tLight Information for %s\n",
				considerAllSceneLights == MHWRender::MDrawContext::kFilteredToLightLimit
				? "only lights clamped to light limit." : "lights not clamped to light limit.");

			unsigned int lightCount = context.numberOfActiveLights(considerAllSceneLights);
			MFloatPointArray positions;
			MFloatPoint position;
			MFloatVector direction;
			float intensity = 1.0f;
			MColor color;
			bool hasDirection = false;
			bool hasPosition = false;

			bool visualizeLighting = false;
			// Reuse for each light.
			MMatrix identity;
			if (visualizeLighting )
			{
				for (unsigned int i=0; i<8; i++)
				{
					GLenum light = GL_LIGHT0+i;
					glDisable(light);
				}
				if (!lightCount)
					glDisable(GL_LIGHTING);
				else
					glEnable(GL_LIGHTING);
			}

			for (unsigned int i=0; i<lightCount; i++)
			{
				//
				// Shown here are 2 avenues for accessing light data:
				//
				// If only some basic information which is common to all
				// light types is required then the getLightInformation()
				// interface can be used. It is a simpler interface and
				// in general will be faster to execute.
				//
				// The alternative is to access the lights by parameter
				// via the getLightParameterInformation() interface
				// which allows us to inspect all the per light type specific
				// information (e.g. the drop off on spot lights).
				// As shown it is a bit more complex
				// to use but provides more information.
				//
				bool getCommonParametersOnly = false;

				// Look at common information only
				//
				if (getCommonParametersOnly)
				{
					context.getLightInformation( i,
									 positions, direction,
									 intensity, color, hasDirection,
									 hasPosition,
									 considerAllSceneLights);
					printf("\tLight %d {\n", i);
					printf("\t\tDirectional %d, Positional %d\n", hasDirection, hasPosition);
					printf("\t\tDirection = %g, %g, %g\n", direction[0],direction[1],direction[2]);
					unsigned int positionCount = positions.length();
					if (hasPosition && positionCount)
					{
						for (unsigned int p=0; p<positions.length(); p++)
						{
							printf("\t\tPosition[%d] = %g, %g, %g\n", p, positions[p][0], positions[p][1], positions[p][2]);
							position += positions[p];
						}
						position[0] /= (float)positionCount;
						position[1] /= (float)positionCount;
						position[2] /= (float)positionCount;
					}
					printf("\t\tColor = %g, %g, %g\n", color[0], color[1], color[2]);
					printf("\t\tIntensity = %g\n", intensity);
					printf("\t}\n");
				}

				// Look at all information which may differ per light type
				//
				else
				{
					unsigned int positionCount = 0;
					position[0] = position[1] = position[2] = 0.0f;
					MHWRender::MLightParameterInformation *lightParam =
						context.getLightParameterInformation( i, considerAllSceneLights );
					if (lightParam)
					{
						printf("\tLight %d {\n", i);

						MStringArray params;
						lightParam->parameterList(params);
						for (unsigned int p=0; p<params.length(); p++)
						{
							MString pname = params[p];
							MHWRender::MLightParameterInformation::ParameterType ptype = lightParam->parameterType( pname );
							MFloatArray floatVals;
							MIntArray intVals;
							MMatrix matrixVal;
							MHWRender::MSamplerStateDesc samplerDesc;
							switch (ptype)
							{
							case MHWRender::MLightParameterInformation::kBoolean:
								lightParam->getParameter( pname, intVals );
								printf("\t\tLight parameter %s. Bool[%d]\n", pname.asChar(),
									intVals[0]);
								break;
							case MHWRender::MLightParameterInformation::kInteger:
								lightParam->getParameter( pname, intVals );
								printf("\t\tLight parameter %s. Integer[%d]\n", pname.asChar(),
									intVals[0]);
								break;
							case MHWRender::MLightParameterInformation::kFloat:
								lightParam->getParameter( pname, floatVals );
								printf("\t\tLight parameter %s. Float[%g]\n", pname.asChar(),
									floatVals[0]);
								break;
							case MHWRender::MLightParameterInformation::kFloat2:
								lightParam->getParameter( pname, floatVals );
								printf("\t\tLight parameter %s. Float[%g,%g]\n", pname.asChar(),
									floatVals[0], floatVals[1]);
								break;
							case MHWRender::MLightParameterInformation::kFloat3:
								lightParam->getParameter( pname, floatVals );
								printf("\t\tLight parameter %s. Float3[%g,%g,%g]\n", pname.asChar(),
									floatVals[0], floatVals[1], floatVals[2]);
								break;
							case MHWRender::MLightParameterInformation::kFloat4:
								lightParam->getParameter( pname, floatVals );
								printf("\t\tLight parameter %s. Float4[%g,%g,%g,%g]\n", pname.asChar(),
									floatVals[0], floatVals[1], floatVals[2], floatVals[3]);
								break;
							case MHWRender::MLightParameterInformation::kFloat4x4Row:
								lightParam->getParameter( pname, matrixVal );
								printf("\t\tLight parameter %s. Float4x4Row [%g,%g,%g,%g]\n\t\t[%g,%g,%g,%g]\n\t\t[%g,%g,%g,%g]\n\t\t[%g,%g,%g,%g]\n",
									pname.asChar(),
									matrixVal[0][0], matrixVal[0][1], matrixVal[0][2], matrixVal[0][3],
									matrixVal[1][0], matrixVal[1][1], matrixVal[1][2], matrixVal[1][3],
									matrixVal[2][0], matrixVal[2][1], matrixVal[2][2], matrixVal[2][3],
									matrixVal[3][0], matrixVal[3][1], matrixVal[3][2], matrixVal[3][3]
									);
								break;
							case MHWRender::MLightParameterInformation::kFloat4x4Col:
								lightParam->getParameter( pname, matrixVal );
								printf("\t\tLight parameter %s. Float4x4Row\n", pname.asChar() );
								break;
							case MHWRender::MLightParameterInformation::kTexture2:
							{
								void *handle = lightParam->getParameterTextureHandle( pname );
								printf("\t\tLight texture parameter %s. OpenGL texture id = %d\n", pname.asChar(),
									*((int *)handle));
								break;
							}
							case MHWRender::MLightParameterInformation::kSampler:
								lightParam->getParameter( pname, samplerDesc );
								printf("\t\tLight sampler parameter %s. filter = %d\n", pname.asChar(),
									samplerDesc.filter );
								break;
							default:
								break;
							}

							// Do some discovery to map stock parameters to usable values
							// based on the semantic
							//
							MHWRender::MLightParameterInformation::StockParameterSemantic semantic = lightParam->parameterSemantic( pname );
							switch (semantic)
							{
							case MHWRender::MLightParameterInformation::kLightEnabled:
								printf("\t\t- Parameter semantic : light enabled\n");
								break;
							case MHWRender::MLightParameterInformation::kWorldPosition:
								printf("\t\t- Parameter semantic : world position\n");
								position += MFloatPoint( floatVals[0], floatVals[1], floatVals[2] );
								positionCount++;
								hasPosition = true;
								break;
							case MHWRender::MLightParameterInformation::kWorldDirection:
								printf("\t\t- Parameter semantic : world direction\n");
								direction = MFloatVector( floatVals[0], floatVals[1], floatVals[2] );
								hasDirection = true;
								break;
							case MHWRender::MLightParameterInformation::kIntensity:
								printf("\t\t- Parameter semantic : intensity\n");
								intensity = floatVals[0];
								break;
							case MHWRender::MLightParameterInformation::kColor:
								printf("\t\t- Parameter semantic : color\n");
								color = MColor( floatVals[0], floatVals[1], floatVals[2] );
								break;
							case MHWRender::MLightParameterInformation::kEmitsDiffuse:
								printf("\t\t- Parameter semantic : emits-diffuse\n");
								break;
							case MHWRender::MLightParameterInformation::kEmitsSpecular:
								printf("\t\t- Parameter semantic : emits-specular\n");
								break;
							case MHWRender::MLightParameterInformation::kDecayRate:
								printf("\t\t- Parameter semantic : decay rate\n");
								break;
							case MHWRender::MLightParameterInformation::kDropoff:
								printf("\t\t- Parameter semantic : drop-off\n");
								break;
							case MHWRender::MLightParameterInformation::kCosConeAngle:
								printf("\t\t- Parameter semantic : cosine cone angle\n");
								break;
							case MHWRender::MLightParameterInformation::kShadowMap:
								printf("\t\t- Parameter semantic : shadow map\n");
								break;
							case MHWRender::MLightParameterInformation::kShadowSamp:
								printf("\t\t- Parameter semantic : shadow map sampler\n");
								break;
							case MHWRender::MLightParameterInformation::kShadowBias:
								printf("\t\t- Parameter semantic : shadow map bias\n");
								break;
							case MHWRender::MLightParameterInformation::kShadowMapSize:
								printf("\t\t- Parameter semantic : shadow map size\n");
								break;
							case MHWRender::MLightParameterInformation::kShadowViewProj:
								printf("\t\t- Parameter semantic : shadow map view projection matrix\n");
								break;
							case MHWRender::MLightParameterInformation::kShadowColor:
								printf("\t\t- Parameter semantic : shadow color\n");
								break;
							case MHWRender::MLightParameterInformation::kGlobalShadowOn:
								printf("\t\t- Parameter semantic : global shadows on \n");
								break;
							case MHWRender::MLightParameterInformation::kShadowOn:
								printf("\t\t- Parameter semantic : local shadows on\n");
								break;
							default:
								break;
							}

						}

						// Compute an average position
						if (positionCount > 1)
						{
							position[0] /= (float)positionCount;
							position[1] /= (float)positionCount;
							position[2] /= (float)positionCount;
							printf("\t\tCompute average position [%g,%g,%g]\n", position[0],
								position[1], position[2]);
						}
						//delete lightParam;
						printf("\t}\n");
					}
				}

				MHWRender::MRenderer *theRenderer = MHWRender::MRenderer::theRenderer();
				if (theRenderer && theRenderer->drawAPIIsOpenGL() && visualizeLighting
					&& lightCount < 8)
				{
					GLenum light = GL_LIGHT0+i;

					// Set light parameters
					float ambient[3] = { 0.0f, 0.0f, 0.0f };
					float specular[3] = { 1.0f, 1.0f, 1.0f };
					glLightfv(light, GL_AMBIENT, &ambient[0]);
					color[0] *= intensity;
					color[1] *= intensity;
					color[2] *= intensity;
					glLightfv(light, GL_DIFFUSE, &color[0]);
					glLightfv(light, GL_SPECULAR, &specular[0]);

					glLightf(light,  GL_CONSTANT_ATTENUATION, 1.0f);
					glLightf(light,  GL_LINEAR_ATTENUATION, 0.0f);
					glLightf(light,  GL_QUADRATIC_ATTENUATION, 0.0f);

					glPushMatrix();
					glLoadMatrixd( identity.matrix[0] );

					// Set light position
					if (hasPosition)
						glLightfv(light, GL_POSITION, &position[0]);
					else {
						position[0] = position[1] = position[2] = 0.0f;
						glLightfv(light, GL_POSITION, &position[0]);
					}

					// Set rest of parameters.
					if (hasDirection)
					{
						glLightf(light,  GL_SPOT_CUTOFF, 90.0f);
						glLightf(light,  GL_SPOT_EXPONENT, 64.0f);
						glLightfv(light, GL_SPOT_DIRECTION, &direction[0]);
					}
					else
					{
						glLightf(light, GL_SPOT_CUTOFF, 180.0f);
						glLightf(light, GL_SPOT_EXPONENT, 0.0f);
					}

					glEnable(light);
					glPopMatrix();
				}
			}

		}
		printf("}\n");
	}



protected:
	hwPhongShaderOverride(const MObject& obj)
	: MHWRender::MPxShaderOverride(obj)
	, fShaderNode(NULL)
	, fTextureData(NULL)
	, fDrawUsingShader(true) // Disabling this will use fixed-function which only has an OpenGL implementation
	, fShaderBound(false)
	, fTexture(NULL)
	, fInColorPass(false)
	, fColorShaderInstance(NULL)
	, fNTColorShaderInstance(NULL)
	, fInShadowPass(false)
	, fShadowShaderInstance(NULL)
	, fTransparency(0.0f)
	{
		// Create a shader instance to use for drawing
		//
		if (fDrawUsingShader)
		{
			createShaderInstance();
		}
		fAmbient[0] = fAmbient[1] = fAmbient[2] = 0.0f;
		fDiffuse[0] = fDiffuse[1] = fDiffuse[2] = fDiffuse[3] = 0.0f;
		fSpecular[0] = fSpecular[1] = fSpecular[2] = 0.0f;
		fShininess[0] = fShininess[1] = fShininess[2] = 500.0f;
	}

	// override blend state when there is blending
    static const MHWRender::MBlendState *sBlendState;

	// Current hwPhongShader node associated with the shader override.
	// Updated during doDG() time.
	hwPhongShader *fShaderNode;
	// Shader inputs values including transparency
	float fTransparency;
	float fAmbient[3];
	float fDiffuse[4];
	float fSpecular[3];
	float fShininess[3];

	// Temporary system buffer for creating textures
	unsigned char* fTextureData;

	// Pass tracking
	bool fInColorPass;
	bool fInShadowPass;

	// Draw with texture or shader flag
	bool fDrawUsingShader;
	// VP2 texture
	MHWRender::MTexture *fTexture;
	// VP2 color shader
	MHWRender::MShaderInstance *fColorShaderInstance;
	// VP2 shadow shader
	MHWRender::MShaderInstance *fShadowShaderInstance;
	// VP2 non-textured shader
	MHWRender::MShaderInstance *fNTColorShaderInstance;
	mutable bool fShaderBound;
};

const MHWRender::MBlendState* hwPhongShaderOverride::sBlendState = NULL;



MStatus initializePlugin( MObject obj )
{
	TRACE_API_CALLS("initializePlugin");
	MStatus   status;

	const MString& swatchName =	MHWShaderSwatchGenerator::initialize();
	const MString UserClassify( "shader/surface/utility/:drawdb/shader/surface/hwPhongShader:swatch/"+swatchName );

	MFnPlugin plugin( obj, PLUGIN_COMPANY, "4.5", "Any");
	status = plugin.registerNode( "hwPhongShader", hwPhongShader::id,
			                      hwPhongShader::creator, hwPhongShader::initialize,
								  MPxNode::kHwShaderNode, &UserClassify );
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	plugin.registerDragAndDropBehavior("hwPhongShaderBehavior",
									   hwPhongShaderBehavior::creator);

	// Register a shader override for this node
	MHWRender::MDrawRegistry::registerShaderOverrideCreator(
		"drawdb/shader/surface/hwPhongShader",
		sHWPhongShaderRegistrantId,
		hwPhongShaderOverride::Creator);
	if (status != MS::kSuccess) return status;

	return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj )
{
	TRACE_API_CALLS("uninitializePlugin");
	MStatus   status;

	MFnPlugin plugin( obj );

	// Unregister all chamelion shader nodes
	plugin.deregisterNode( hwPhongShader::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	plugin.deregisterDragAndDropBehavior("hwPhongShaderBehavior");

	// Deregister the shader override
	status = MHWRender::MDrawRegistry::deregisterShaderOverrideCreator(
		"drawdb/shader/surface/hwPhongShader", sHWPhongShaderRegistrantId);
	if (status != MS::kSuccess) return status;

	return MS::kSuccess;
}

void * hwPhongShader::creator()
{
	TRACE_API_CALLS("creator");
    return new hwPhongShader();
}

MStatus hwPhongShader::initialize()
{
	// Make sure that all attributes are cached internal for
	// optimal performance !

	TRACE_API_CALLS("initialize");
    MFnNumericAttribute nAttr;
	MFnCompoundAttribute cAttr;

    // Create input attributes
    aColor = nAttr.createColor( "color", "c");
    nAttr.setStorable(true);
	nAttr.setKeyable(true);
	nAttr.setDefault(0.1f, 0.1f, 0.1f);
	nAttr.setCached( true );
	nAttr.setInternal( true );
	nAttr.setAffectsAppearance( true );
	
	aTransparency = nAttr.create( "transparency", "tr", MFnNumericData::kFloat );
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	nAttr.setDefault(0.0f);
	nAttr.setMax(1.0f);
	nAttr.setMin(0.0f);
	nAttr.setCached( true );
	nAttr.setInternal( true );
	nAttr.setAffectsAppearance( true );

    aDiffuseColor = nAttr.createColor( "diffuseColor", "dc" );
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	nAttr.setDefault(1.f, 0.5f, 0.5f);
	nAttr.setCached( true );
	nAttr.setInternal( true );
	nAttr.setAffectsAppearance( true );

    aSpecularColor = nAttr.createColor( "specularColor", "sc" );
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	nAttr.setDefault(0.5f, 0.5f, 0.5f);
	nAttr.setCached( true );
	nAttr.setInternal( true );
	nAttr.setAffectsAppearance( true );

	// This is defined as a compound attribute, users can easily enter
	// values beyond 1.
	aShininessX = nAttr.create( "shininessX", "shx", MFnNumericData::kFloat, 100.0 );
	aShininessY = nAttr.create( "shininessY", "shy", MFnNumericData::kFloat, 100.0 );
	aShininessZ = nAttr.create( "shininessZ", "shz", MFnNumericData::kFloat, 100.0 );
    aShininess = cAttr.create( "shininess", "sh" );
	cAttr.addChild(aShininessX);
	cAttr.addChild(aShininessY);
	cAttr.addChild(aShininessZ) ;
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	nAttr.setCached( true );
	nAttr.setInternal( true );
	nAttr.setAffectsAppearance(true);	
    cAttr.setHidden(false);
	
	aGeometryShape = nAttr.create( "geometryShape", "gs", MFnNumericData::kInt );
    nAttr.setStorable(true);
	nAttr.setKeyable(true);
	nAttr.setDefault(0);
	nAttr.setCached( true );
	nAttr.setInternal( true );

	// create output attributes here
	// outColor is the only output attribute and it is inherited
	// so we do not need to create or add it.
	//

	// Add the attributes here

    addAttribute(aColor);
	addAttribute(aTransparency);
	addAttribute(aDiffuseColor);
	addAttribute(aSpecularColor);
	addAttribute(aShininess);
	addAttribute(aGeometryShape);

    attributeAffects (aColor,			outColor);
    attributeAffects (aTransparency,	outColor);
    attributeAffects (aDiffuseColor,	outColor);
	attributeAffects (aSpecularColor,	outColor);
	attributeAffects (aShininessX,		outColor);
	attributeAffects (aShininessY,		outColor);
	attributeAffects (aShininessZ,		outColor);
	attributeAffects (aShininess,		outColor);

    return MS::kSuccess;
}


// DESCRIPTION:
//
MStatus hwPhongShader::compute(
const MPlug&      plug,
      MDataBlock& block )
{
	TRACE_API_CALLS("compute");

    if ((plug != outColor) && (plug.parent() != outColor))
		return MS::kUnknownParameter;

	MFloatVector & color  = block.inputValue( aDiffuseColor ).asFloatVector();

    // set output color attribute
    MDataHandle outColorHandle = block.outputValue( outColor );
    MFloatVector& outColor = outColorHandle.asFloatVector();
	outColor = color;

    outColorHandle.setClean();
    return MS::kSuccess;
}

/* virtual */
bool hwPhongShader::setInternalValueInContext( const MPlug &plug,
											   const MDataHandle &handle,
											   MDGContext & )
{
	if (plug == aColor)
	{
		float3 & val = handle.asFloat3();
		if (val[0] != mAmbientColor[0] ||
			val[1] != mAmbientColor[1] ||
			val[2] != mAmbientColor[2])
		{
			mAmbientColor[0] = val[0];
			mAmbientColor[1] = val[1];
			mAmbientColor[2] = val[2];
			mAttributesChanged = true;
			mAttributesChangedVP2 = true;
		}
	}
	else if (plug == aTransparency)
	{
		float val = handle.asFloat();
		if (val != mTransparency)
		{
			mTransparency = val;
			mAttributesChanged = true;
			mAttributesChangedVP2 = true;
		}
	}
	else if (plug == aDiffuseColor)
	{
		float3 & val = handle.asFloat3();
		if (val[0] != mDiffuseColor[0] ||
			val[1] != mDiffuseColor[1] ||
			val[2] != mDiffuseColor[2])
		{
			mDiffuseColor[0] = val[0];
			mDiffuseColor[1] = val[1];
			mDiffuseColor[2] = val[2];
			mAttributesChanged = true;
			mAttributesChangedVP2 = true;
		}
	}
	else if (plug == aSpecularColor)
	{
		float3 & val = handle.asFloat3();
		if (val[0] != mSpecularColor[0] ||
			val[1] != mSpecularColor[1] ||
			val[2] != mSpecularColor[2])
		{
			mSpecularColor[0] = val[0];
			mSpecularColor[1] = val[1];
			mSpecularColor[2] = val[2];
			mAttributesChanged = true;
			mAttributesChangedVP2 = true;
		}
	}
	else if (plug == aShininessX)
	{
		float val = handle.asFloat();
		if (val != mShininess[0])
		{
			mShininess[0] = val; 
			mAttributesChanged = true;
			mAttributesChangedVP2 = true;
		}
	}
	else if (plug == aShininessY)
	{
		float val = handle.asFloat();
		if (val != mShininess[1])
		{
			mShininess[1] = val; 
			mAttributesChanged = true;
			mAttributesChangedVP2 = true;
		}
	}
	else if (plug == aShininessZ)
	{
		float val = handle.asFloat();
		if (val != mShininess[2])
		{
			mShininess[2] = val; 
			mAttributesChanged = true;
			mAttributesChangedVP2 = true;
		}
	}
	else if (plug == aGeometryShape)
	{
		mGeometryShape = handle.asInt();
	}

	return false;
}

/* virtual */
bool
hwPhongShader::getInternalValueInContext( const MPlug& plug,
										 MDataHandle& handle,
										 MDGContext&)
{
	if (plug == aColor)
	{
		handle.set( mAmbientColor[0], mAmbientColor[1], mAmbientColor[2] );
	}
	if (plug == aTransparency)
	{
		handle.set( mTransparency );
	}
	else if (plug == aDiffuseColor)
	{
		handle.set( mDiffuseColor[0], mDiffuseColor[1], mDiffuseColor[2] );
	}
	else if (plug == aSpecularColor)
	{
		handle.set( mSpecularColor[0], mSpecularColor[1], mSpecularColor[2] );
	}
	else if (plug == aShininessX)
	{
		handle.set( mShininess[0] );
	}
	else if (plug == aShininessY)
	{
		handle.set( mShininess[1] );
	}
	else if (plug == aShininessZ)
	{
		handle.set( mShininess[2] );
	}
	else if (plug == aGeometryShape)
	{
		handle.set( (int) mGeometryShape );
	}
	return false;
}

/* virtual */
MStatus	hwPhongShader::bind(const MDrawRequest& request, M3dView& view)

{
	TRACE_API_CALLS("bind");

	init_Phong_texture ();

	return MS::kSuccess;
}


/* virtual */
MStatus	hwPhongShader::glBind(const MDagPath&)
{
	TRACE_API_CALLS("glBind");

	init_Phong_texture ();

	return MS::kSuccess;
}


/* virtual */
MStatus	hwPhongShader::unbind(const MDrawRequest& request, M3dView& view)
{
	TRACE_API_CALLS("unbind");

    // The texture may have been allocated by the draw; it's kept
    // around for use again. When scene new or open is performed this
    // texture will be released in releaseEverything().

	return MS::kSuccess;
}

/* virtual */
MStatus	hwPhongShader::glUnbind(const MDagPath&)
{
	TRACE_API_CALLS("glUnbind");

    // The texture may have been allocated by the draw; it's kept
    // around for use again. When scene new or open is performed this
    // texture will be released in releaseEverything().

	return MS::kSuccess;
}

/* virtual */
MStatus	hwPhongShader::geometry( const MDrawRequest& request,
							    M3dView& view,
                                int prim,
								unsigned int writable,
								int indexCount,
								const unsigned int * indexArray,
								int vertexCount,
								const int * vertexIDs,
								const float * vertexArray,
								int normalCount,
								const float ** normalArrays,
								int colorCount,
								const float ** colorArrays,
								int texCoordCount,
								const float ** texCoordArrays)
{
	TRACE_API_CALLS("geometry");
	MStatus stat = MStatus::kSuccess;

	if (mGeometryShape != 0)
		drawDefaultGeometry();
	else
		stat = draw( prim, writable, indexCount, indexArray, vertexCount,
				vertexIDs, vertexArray, normalCount, normalArrays, colorCount,
				colorArrays, texCoordCount, texCoordArrays);
	return stat;
}

/* virtual */
MStatus	hwPhongShader::glGeometry(const MDagPath & path,
                                int prim,
								unsigned int writable,
								int indexCount,
								const unsigned int * indexArray,
								int vertexCount,
								const int * vertexIDs,
								const float * vertexArray,
								int normalCount,
								const float ** normalArrays,
								int colorCount,
								const float ** colorArrays,
								int texCoordCount,
								const float ** texCoordArrays)
{
	TRACE_API_CALLS("glGeometry");
	MStatus stat = MStatus::kSuccess;

	if (mGeometryShape != 0)
		drawDefaultGeometry();
	else
		stat = draw( prim, writable, indexCount, indexArray, vertexCount,
					vertexIDs, vertexArray, normalCount, normalArrays, colorCount,
					colorArrays, texCoordCount, texCoordArrays);
	return stat;
}

/* virtual */
MStatus
hwPhongShader::renderSwatchImage( MImage & outImage )
{
	MStatus status = MStatus::kFailure;

	// Use VP2 swatch drawing (especially useful for Dx11 and Core Profile OpenGL)
	if (MHWRender::MRenderer::theRenderer())
	{
		MString meshSphere("meshSphere");
		MString meshShaderball("meshShaderball");

		unsigned int targetW, targetH;
		outImage.getSize(targetW, targetH);

		return MHWRender::MRenderUtilities::renderMaterialViewerGeometry(targetW > 128 ? meshShaderball : meshSphere, 
																		thisMObject(), 
																		outImage, 
																		MHWRender::MRenderUtilities::kPerspectiveCamera, 
																		MHWRender::MRenderUtilities::kSwatchLight);
	}

	// Get the hardware renderer utility class
	MHardwareRenderer *pRenderer = MHardwareRenderer::theRenderer();
	if (pRenderer)
	{
		const MString& backEndStr = pRenderer->backEndString();

		// Get geometry
		// ============
		unsigned int* pIndexing = 0;
		unsigned int  numberOfData = 0;
		unsigned int  indexCount = 0;

		MHardwareRenderer::GeometricShape gshape = MHardwareRenderer::kDefaultSphere;
		if (mGeometryShape == 2)
		{
			gshape = MHardwareRenderer::kDefaultCube;
		}
		else if (mGeometryShape == 3)
		{
			gshape = MHardwareRenderer::kDefaultPlane;
		}

		MGeometryData* pGeomData =
			pRenderer->referenceDefaultGeometry( gshape, numberOfData, pIndexing, indexCount );
		if( !pGeomData )
		{
			return MStatus::kFailure;
		}

		// Make the swatch context current
		// ===============================
		//
		unsigned int width, height;
		outImage.getSize( width, height );
		unsigned int origWidth = width;
		unsigned int origHeight = height;

		MStatus status2 = pRenderer->makeSwatchContextCurrent( backEndStr, width, height );

		if( status2 == MS::kSuccess )
		{
			glPushAttrib ( GL_ALL_ATTRIB_BITS );

			// Get camera
			// ==========
			{
				// Get the camera frustum from the API
				double l, r, b, t, n, f;
				pRenderer->getSwatchOrthoCameraSetting( l, r, b, t, n, f );

				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glOrtho( l, r, b, t, n, f );

				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				// Rotate the cube a bit so we don't see it head on
				if (gshape ==  MHardwareRenderer::kDefaultCube)
					glRotatef( 45, 1.0, 1.0, 1.0 );
				else if (gshape == MHardwareRenderer::kDefaultPlane)
					glScalef( 1.5, 1.5, 1.5 );
				else
					glScalef( 1.0, 1.0, 1.0 );
			}

			// Draw The Swatch
			// ===============
			drawTheSwatch( pGeomData, pIndexing, numberOfData, indexCount );

			// Read pixels back from swatch context to MImage
			// ==============================================
			pRenderer->readSwatchContextPixels( backEndStr, outImage );

			// Double check the outing going image size as image resizing
			// was required to properly read from the swatch context
			outImage.getSize( width, height );
			if (width != origWidth || height != origHeight)
			{
				status = MStatus::kFailure;
			}
			else
			{
				status = MStatus::kSuccess;
			}

			glPopAttrib();
		}
		else
		{
			pRenderer->dereferenceGeometry( pGeomData, numberOfData );
		}
	}
	return status;
}

void
hwPhongShader::drawTheSwatch( MGeometryData* pGeomData,
								   unsigned int* pIndexing,
								   unsigned int  numberOfData,
								   unsigned int  indexCount )
{
	TRACE_API_CALLS("drwaTheSwatch");

	MHardwareRenderer *pRenderer = MHardwareRenderer::theRenderer();
	if( !pRenderer )	return;

	if ( mAttributesChanged || (phong_map_id == 0))
	{
		init_Phong_texture ();
	}


	// Get the default background color
	float r, g, b, a;
	MHWShaderSwatchGenerator::getSwatchBackgroundColor( r, g, b, a );
	glClearColor( r, g, b, a );

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	CubeMapTextureDrawUtility::bind( phong_map_id );

	// Draw default geometry
	{
		if (pGeomData)
		{
			glPushClientAttrib ( GL_CLIENT_VERTEX_ARRAY_BIT );

			float *vertexData = (float *)( pGeomData[0].data() );
			if (vertexData)
			{
				glEnableClientState( GL_VERTEX_ARRAY );
				glVertexPointer ( 3, GL_FLOAT, 0, vertexData );
			}

			float *normalData = (float *)( pGeomData[1].data() );
			if (normalData)
			{
				glEnableClientState( GL_NORMAL_ARRAY );
				glNormalPointer (    GL_FLOAT, 0, normalData );
			}

			if (vertexData && normalData && pIndexing )
				glDrawElements ( GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, pIndexing );

			glPopClientAttrib();

			// Release data references
			pRenderer->dereferenceGeometry( pGeomData, numberOfData );
		}
	}

	CubeMapTextureDrawUtility::unbind();
}

void hwPhongShader::drawDefaultGeometry()
{
	TRACE_API_CALLS("drawDefaultGeometry");

	MHardwareRenderer *pRenderer = MHardwareRenderer::theRenderer();
	if (!pRenderer)
		return;

	CubeMapTextureDrawUtility::bind( phong_map_id );

	// Get default geometry
	{
		unsigned int numberOfData = 0;
		unsigned int *pIndexing = 0;
		unsigned int indexCount = 0;

		MHardwareRenderer::GeometricShape gshape = MHardwareRenderer::kDefaultSphere;
		if (mGeometryShape == 2)
		{
			gshape = MHardwareRenderer::kDefaultCube;
		}
		else if (mGeometryShape == 3)
		{
			gshape = MHardwareRenderer::kDefaultPlane;
		}

		// Get data references
		MGeometryData * pGeomData =
			pRenderer->referenceDefaultGeometry( gshape, numberOfData, pIndexing, indexCount);

		if (pGeomData)
		{
			glPushClientAttrib ( GL_CLIENT_VERTEX_ARRAY_BIT );

			float *vertexData = (float *)( pGeomData[0].data() );
			if (vertexData)
			{
				glEnableClientState( GL_VERTEX_ARRAY );
				glVertexPointer ( 3, GL_FLOAT, 0, vertexData );
			}

			float *normalData = (float *)( pGeomData[1].data() );
			if (normalData)
			{
				glEnableClientState( GL_NORMAL_ARRAY );
				glNormalPointer (    GL_FLOAT, 0, normalData );
			}

			if (vertexData && normalData && pIndexing )
				glDrawElements ( GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, pIndexing );

			glPopClientAttrib();

			// Release data references
			pRenderer->dereferenceGeometry( pGeomData, numberOfData );
		}
	}

	CubeMapTextureDrawUtility::unbind();
}

MStatus	hwPhongShader::draw(int prim,
							unsigned int writable,
							int indexCount,
							const unsigned int * indexArray,
							int vertexCount,
							const int * vertexIDs,
							const float * vertexArray,
							int normalCount,
							const float ** normalArrays,
							int colorCount,
							const float ** colorArrays,
							int texCoordCount,
							const float ** texCoordArrays)
{
	TRACE_API_CALLS("draw");

	if ( prim != GL_TRIANGLES && prim != GL_TRIANGLE_STRIP)	{
        return MS::kFailure;
    }

	CubeMapTextureDrawUtility::bind( phong_map_id );

	// Draw the surface.
	//
    {
		bool needBlending = false;
		if (mTransparency > 0.0f)
		{
			needBlending = true;
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f-mTransparency );
		}
		else
			glColor4f(1.0, 1.0, 1.0, 1.0f);

		glPushClientAttrib ( GL_CLIENT_VERTEX_ARRAY_BIT );
		// GL_VERTEX_ARRAY does not necessarily need to be
		// enabled, as it should be enabled before this routine
		// is valled.
		glEnableClientState( GL_VERTEX_ARRAY );
		glEnableClientState( GL_NORMAL_ARRAY );

		glVertexPointer ( 3, GL_FLOAT, 0, &vertexArray[0] );
		glNormalPointer (    GL_FLOAT, 0, &normalArrays[0][0] );

		glDrawElements ( prim, indexCount, GL_UNSIGNED_INT, indexArray );

		// The client attribute is already being popped. You
		// don't need to reset state here.
		//glDisableClientState( GL_NORMAL_ARRAY );
		//glDisableClientState( GL_VERTEX_ARRAY );
		glPopClientAttrib();

		if (needBlending)
		{
			glColor4f(1.0, 1.0, 1.0, 1.0f);
			glDisable(GL_BLEND);
		}
    }

	CubeMapTextureDrawUtility::unbind();

	return MS::kSuccess;
}

/* virtual */
int	hwPhongShader::normalsPerVertex()
{
	TRACE_API_CALLS("normalsPerVertex");
	return 1;
}

/* virtual */
int	hwPhongShader::texCoordsPerVertex()
{
	TRACE_API_CALLS("texCoordsPerVertex");
	return 0;
}

/* virtual */
int	hwPhongShader::getTexCoordSetNames(MStringArray& names)
{
	return 0;
}

/* virtual */
bool hwPhongShader::hasTransparency()
{
	return (mTransparency > 0.0f);
}


