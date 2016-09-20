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
#include <maya/MIOStream.h>
#include <math.h>
#include <cstdlib>

#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MPlug.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFloatVector.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MUserData.h>

#include <maya/MHWShaderSwatchGenerator.h>
#include <maya/MHardwareRenderer.h>

#include <maya/MImage.h>

#include <maya/MMatrix.h>

// Viewport 2.0 includes
#include <maya/MDrawRegistry.h>
#include <maya/MPxShaderOverride.h>
#include <maya/MDrawContext.h>
#include <maya/MStateManager.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MShaderManager.h>

#undef ENABLE_TRACE_API_CALLS
#ifdef ENABLE_TRACE_API_CALLS
#define TRACE_API_CALLS(x) cerr << "vp2BlinnShader: "<<(x)<<"\n"
#else
#define TRACE_API_CALLS(x)
#endif

#include "vp2BlinnShader.h"

MTypeId vp2BlinnShader::id( 0x00081102 );

MObject  vp2BlinnShader::aColor;
MObject  vp2BlinnShader::aTransparency;
MObject  vp2BlinnShader::aSpecularColor;
MObject  vp2BlinnShader::aNonTexturedColor;

///////////////////////////////////////////////////////////////////////////////////////////
// Node methods
///////////////////////////////////////////////////////////////////////////////////////////
void * vp2BlinnShader::creator()
{
	TRACE_API_CALLS("creator");
    return new vp2BlinnShader();
}

vp2BlinnShader::vp2BlinnShader()
{
	TRACE_API_CALLS("vp2BlinnShader");
}

vp2BlinnShader::~vp2BlinnShader()
{
	TRACE_API_CALLS("~vp2BlinnShader");
}

MStatus vp2BlinnShader::initialize()
{
	// Make sure that all attributes are cached internal for
	// optimal performance !

	TRACE_API_CALLS("initialize");
    MFnNumericAttribute nAttr;

    // Create input attributes
    aColor = nAttr.createColor( "color", "c");
    nAttr.setStorable(true);
	nAttr.setKeyable(true);
	nAttr.setDefault(0.6f, 0.6f, 0.6f);
	nAttr.setAffectsAppearance( true );
	
	aTransparency = nAttr.create( "transparency", "tr", MFnNumericData::kFloat );
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	nAttr.setDefault(0.0f);
	nAttr.setMax(1.0f);
	nAttr.setMin(0.0f);
	nAttr.setAffectsAppearance( true );

    aSpecularColor = nAttr.createColor( "specularColor", "sc" );
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	nAttr.setDefault(1.0f, 1.0f, 1.0f);
	nAttr.setAffectsAppearance( true );

    // Create input attributes
    aNonTexturedColor = nAttr.createColor( "nonTexturedColor", "nc");
    nAttr.setStorable(true);
	nAttr.setKeyable(true);
	nAttr.setDefault(1.0f, 0.0f, 0.0f);
	nAttr.setAffectsAppearance( true );

	// create output attributes here
	// outColor is the only output attribute and it is inherited
	// so we do not need to create or add it.
	//

	// Add the attributes here
    addAttribute(aColor);
	addAttribute(aTransparency);
	addAttribute(aSpecularColor);
    addAttribute(aNonTexturedColor);

    attributeAffects (aColor,			outColor);
    attributeAffects (aTransparency,	outColor);
	attributeAffects (aSpecularColor,	outColor);
	attributeAffects (aNonTexturedColor,outColor);
    return MS::kSuccess;
}

// DESCRIPTION:
//
MStatus vp2BlinnShader::compute(
const MPlug&      plug,
      MDataBlock& block )
{
	TRACE_API_CALLS("compute");

    if ((plug != outColor) && (plug.parent() != outColor))
		return MS::kUnknownParameter;

	MFloatVector & color  = block.inputValue( aColor ).asFloatVector();

    // set output color attribute
    MDataHandle outColorHandle = block.outputValue( outColor );
    MFloatVector& outColor = outColorHandle.asFloatVector();
	outColor = color;

    outColorHandle.setClean();
    return MS::kSuccess;
}

////////////////////////////////////////////////////////////////////////////////////
// VP1 interface
////////////////////////////////////////////////////////////////////////////////////
/* virtual */
MStatus
vp2BlinnShader::renderSwatchImage( MImage & outImage )
{	
	return MS::kSuccess;
}

////////////////////////////////////////////////////////////////////////////////////
// Viewport 2.0 implementation for the shader
////////////////////////////////////////////////////////////////////////////////////
class vp2BlinnShaderOverride : public MHWRender::MPxShaderOverride
{
public:
	static MHWRender::MPxShaderOverride* Creator(const MObject& obj)
	{
		return new vp2BlinnShaderOverride(obj);
	}

	virtual ~vp2BlinnShaderOverride()
	{
		MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
		if (theRenderer)
		{
			const MHWRender::MShaderManager* shaderMgr = theRenderer->getShaderManager();
			if (shaderMgr)
			{
				if (fColorShaderInstance)
				{
					shaderMgr->releaseShader(fColorShaderInstance);
				}
				fColorShaderInstance = NULL;

				if (fNonTexturedColorShaderInstance)
				{
					shaderMgr->releaseShader(fNonTexturedColorShaderInstance);
				}
				fColorShaderInstance = NULL;
			}
		}
		
	}

	// Initialize phase
	virtual MString initialize(const MInitContext& initContext,
									 MInitFeedback& initFeedback)
	{
		TRACE_API_CALLS("vp2BlinnShaderOverride::initialize");

		if (fColorShaderInstance)
		{
			// This plugin is using the utility method
			// MPxShaderOverride::drawGeometry(). For DX11 drawing,
			// a shader signature is required. We use
			// the signature from the same MShaderInstance used to
			// set the geometry requirements so that the signature
			// will match the requirements.
			//
			addShaderSignature( *fColorShaderInstance );
		}

		// Set the geometry requirements for drawing. Only need
		// position and normals.
		MString empty;

		MHWRender::MVertexBufferDescriptor positionDesc(
			empty,
			MHWRender::MGeometry::kPosition,
			MHWRender::MGeometry::kFloat,
			3);

		MHWRender::MVertexBufferDescriptor normalDesc(
			empty,
			MHWRender::MGeometry::kNormal,
			MHWRender::MGeometry::kFloat,
			3);

		addGeometryRequirement(positionDesc);
		addGeometryRequirement(normalDesc);

        return MString("Autodesk Maya vp2 Blinn Shader Override");
    }

	// Access the node attributes and cache the values to update
	// during updateDevice()
	//
	virtual void updateDG(MObject object)
	{
		TRACE_API_CALLS("vp2BlinnShaderOverride::updateDG");

		if (object == MObject::kNullObj)
			return;

		// Get the hardware shader node from the MObject.
		vp2BlinnShader *shaderNode = (vp2BlinnShader *) MPxHwShaderNode::getHwShaderNodePtr( object );		
		if (!shaderNode)
			return;

		MStatus status;
		MFnDependencyNode node(object, &status);
		if (status)
		{
			node.findPlug("colorR").getValue(fDiffuse[0]);
			node.findPlug("colorG").getValue(fDiffuse[1]);
			node.findPlug("colorB").getValue(fDiffuse[2]);
			node.findPlug("transparency").getValue(fTransparency);
			fDiffuse[3] = 1.0f - fTransparency;
					
			node.findPlug("specularColorR").getValue(fSpecular[0]);
			node.findPlug("specularColorG").getValue(fSpecular[1]);
			node.findPlug("specularColorB").getValue(fSpecular[2]);

			node.findPlug("nonTexturedColorR").getValue(fNonTextured[0]);
			node.findPlug("nonTexturedColorG").getValue(fNonTextured[1]);
			node.findPlug("nonTexturedColorB").getValue(fNonTextured[2]);
		}
	}

	virtual void updateDevice()
	{
		updateShaderInstance();

	}
	virtual void endUpdate()
	{
		TRACE_API_CALLS("vp2BlinnShaderOverride::endUpdate");
	}

	virtual MHWRender::MShaderInstance* shaderInstance() const
	{
		return fColorShaderInstance;
	}

	// Bind the shader on activateKey() and
	// the termination occur in terminateKey().
    virtual void activateKey(MHWRender::MDrawContext& context, const MString& key)
	{
		TRACE_API_CALLS("vp2BlinnShaderOverride::activateKey");
		fColorShaderInstance->bind( context );
	}

	// Unbind / terminate the shader instance here.
    virtual void terminateKey(MHWRender::MDrawContext& context, const MString& key)
	{
		TRACE_API_CALLS("vp2BlinnShaderOverride::terminateKey");
		fColorShaderInstance->unbind( context );
	}
	
	// Use custom shader with custom blend state if required for transparency
	// handling.
	//
	virtual bool draw(MHWRender::MDrawContext& context,
				 const MHWRender::MRenderItemList& renderItemList) const
	{
		TRACE_API_CALLS("vp2BlinnShaderOverride::draw");

		MHWRender::MStateManager* stateMgr = context.getStateManager();

		// initialize vp2BlinnShader blend state once
		if(sBlendState == NULL)
		{
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

		// Save old blend state
		const MHWRender::MBlendState* pOldBlendState = stateMgr->getBlendState();

		bool needBlending = false;
		if (fTransparency > 0.0f)
		{
			needBlending = true;
			stateMgr->setBlendState(sBlendState);
		}

		// Activate all the shader passes and draw using internal draw methods.
		unsigned int passCount = fColorShaderInstance->getPassCount( context );
		for (unsigned int i=0; i<passCount; i++)
		{
			fColorShaderInstance->activatePass( context, i );
			MHWRender::MPxShaderOverride::drawGeometry(context);
		}

		// Restore blend state
		if (needBlending)
		{
			stateMgr->setBlendState(pOldBlendState);
		}
		return true;
	}
	
	// We are using an internal resources so we support all draw APIs
	// automatically.
	virtual MHWRender::DrawAPI supportedDrawAPIs() const
	{
		return (MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile);
	}

	// Transparency indicator
	virtual bool isTransparent()
	{
		TRACE_API_CALLS("vp2BlinnShaderOverride::isTransparent");
		return (fTransparency > 0.0f);
	}

	virtual MHWRender::MShaderInstance* nonTexturedShaderInstance(bool &monitor) const
	{
		if (fNonTexturedColorShaderInstance)
		{
			monitor = true;
			return fNonTexturedColorShaderInstance;
		}
		return NULL;
	}

	virtual bool overridesDrawState()
	{
		return true;
	}	

	// Code to create a cached MShaderInstance using a stock internal Blinn shader
	void createShaderInstance()
	{
		TRACE_API_CALLS("vp2BlinnShaderOverride::createShaderInstance");
		MHWRender::MRenderer *renderer = MHWRender::MRenderer::theRenderer();
		const MHWRender::MShaderManager* shaderMgr = renderer ? renderer->getShaderManager() : NULL;
		if (!shaderMgr)
			return;

		if (!fColorShaderInstance)
		{
			fColorShaderInstance = shaderMgr->getStockShader( MHWRender::MShaderManager::k3dBlinnShader );
		}
		if (!fNonTexturedColorShaderInstance)
		{
			fNonTexturedColorShaderInstance = shaderMgr->getStockShader( MHWRender::MShaderManager::k3dBlinnShader );
			if (fNonTexturedColorShaderInstance)
			{
				fNonTexturedColorShaderInstance->setParameter("diffuseColor", &fNonTextured[0]);
			}
		}
	}

	// Update the shader using the values cached during DG evaluation
	//
	void updateShaderInstance()
	{
		TRACE_API_CALLS("vp2BlinnShaderOverride::updateShaderInstance");
		if (fColorShaderInstance)
		{
			// Update shader to mark it as drawing with transparency or not.
			fColorShaderInstance->setIsTransparent( isTransparent() );
			fColorShaderInstance->setParameter("diffuseColor", &fDiffuse[0] );
			fColorShaderInstance->setParameter("specularColor", &fSpecular[0] );
		}
		if (fNonTexturedColorShaderInstance)
		{
			fNonTexturedColorShaderInstance->setParameter("diffuseColor", &fNonTextured[0]);
		}
	}

protected:
	vp2BlinnShaderOverride(const MObject& obj)
	: MHWRender::MPxShaderOverride(obj)
	, fColorShaderInstance(NULL)
	, fNonTexturedColorShaderInstance(NULL)
	, fTransparency(0.0f)
	{		
		fDiffuse[0] = fDiffuse[1] = fDiffuse[2] = fDiffuse[3] = 0.0f;
		fSpecular[0] = fSpecular[1] = fSpecular[2] = 0.0f;
		fNonTextured[0] = 1.0; fNonTextured[1] = fNonTextured[2] = 0.0f;

		// Create a shader instance to use for drawing
		//
		createShaderInstance();
	}

	// override blend state when there is blending
    static const MHWRender::MBlendState *sBlendState;

	// Cacled shader inputs values
	float fTransparency;
	float fDiffuse[4];
	float fSpecular[3];
	float fShininess[3];
	float fNonTextured[3];

	// Shader to use to draw with
	MHWRender::MShaderInstance *fColorShaderInstance;
	// Shader to use to draw non-textured with
	MHWRender::MShaderInstance *fNonTexturedColorShaderInstance;
};

const MHWRender::MBlendState* vp2BlinnShaderOverride::sBlendState = NULL;

/////////////////////////////////////////////////////////////////////////////////////////
// Plug-in handling
/////////////////////////////////////////////////////////////////////////////////////////
static const MString svp2BlinnShaderRegistrantId("vp2BlinnShaderRegistrantId");

MStatus initializePlugin( MObject obj )
{
	TRACE_API_CALLS("initializePlugin");
	MStatus   status;

	const MString& swatchName =	MHWShaderSwatchGenerator::initialize();
	const MString UserClassify( "shader/surface/utility/:drawdb/shader/surface/vp2BlinnShader:swatch/"+swatchName );

	MFnPlugin plugin( obj, PLUGIN_COMPANY, "4.5", "Any");
	status = plugin.registerNode( "vp2BlinnShader", vp2BlinnShader::id,
			                      vp2BlinnShader::creator, vp2BlinnShader::initialize,
								  MPxNode::kHwShaderNode, &UserClassify );
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	// Register a shader override for this node
	MHWRender::MDrawRegistry::registerShaderOverrideCreator(
		"drawdb/shader/surface/vp2BlinnShader",
		svp2BlinnShaderRegistrantId,
		vp2BlinnShaderOverride::Creator);
	if (status != MS::kSuccess) return status;

	return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj )
{
	TRACE_API_CALLS("uninitializePlugin");
	MStatus   status;

	MFnPlugin plugin( obj );

	// Unregister all chamelion shader nodes
	plugin.deregisterNode( vp2BlinnShader::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	// Deregister the shader override
	status = MHWRender::MDrawRegistry::deregisterShaderOverrideCreator(
		"drawdb/shader/surface/vp2BlinnShader", svp2BlinnShaderRegistrantId);
	if (status != MS::kSuccess) return status;

	return MS::kSuccess;
}



