//
// Copyright (C) 2002-2004 NVIDIA
//
// File: cgfxShaderNode.cpp
//
// Dependency Graph Node: cgfxShader
//
// Author: Jim Atkinson
//
// Changes:
//  10/2003  Kurt Harriman - www.octopusgraphics.com +1-415-893-1023
//           - Multiple UV sets; user-specified texcoord assignment.
//           - "tcs/texCoordSource", a new static attribute, is a
//             string array of up to 32 elements.  Set it to specify
//             the source of each TEXCOORD vertex parameter as one of:
//             a UV set name; "tangent"; "binormal"; "normal"; an empty
//             string; or up to 4 float values "x y z w".  Default is
//             {"map1","tangent","binormal"}.
//           - "-mtc/maxTexCoords" flag of cgfxShader command returns an
//             upper bound on the number of texcoord inputs per vertex
//             (GL_MAX_TEXTURE_UNITS) that can be passed from Maya thru
//             OpenGL to vertex shaders on the current workstation.
//           - The MEL command `pluginInfo -q -version cgfxShader`
//             returns the plug-in version and cgfxShaderNode.cpp
//             compile date.
//           - Improved error handling.
//  12/2003  Kurt Harriman - www.octopusgraphics.com +1-415-893-1023
//           - To load or reload an effect, use the cgfxShader command
//             "-fx/fxFile <filename>" flag.  Setting the cgfxShader
//             node's "s/shader" attribute no longer loads the effect.
//           - To choose a technique, set the "t/technique"
//             attribute of the cgfxShader node.  The effect is not
//             reloaded.  There is no longer a message box requiring
//             the user to choose a technique when loading an effect.
//           - The techniques defined by the current effect are returned
//             by the cgfxShader command "-lt/-listTechniques" flag.
//           - Fixed incorrect transformation of direction/position
//             parameters to spaces other than world space.
//           - Dangling references to deleted dynamic attributes
//             caused exceptions in MObject destructor, terminating
//             the Maya process.  This has been fixed.
//           - Improved error handling.
//
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
#ifndef CGFXSHADER_VERSION
#define CGFXSHADER_VERSION  "4.5"
#endif

#include "cgfxShaderNode.h"
#include "cgfxProfile.h"
#include "cgfxFindImage.h"
#include "cgfxPassStateSetter.h"
#include "cgfxTextureCache.h"

#include <maya/MDagPath.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MEventMessage.h>
#include <maya/MFloatVector.h>
#include <maya/MFnMesh.h>
#include <maya/MFnStringArrayData.h>
#include <maya/MFnStringData.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MGlobal.h>
#include <maya/MHwTextureManager.h>
#include <maya/MPlug.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MVector.h>
#include <maya/MDGModifier.h>
#include <maya/MFileIO.h>
#include <maya/MNodeMessage.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MTextureManager.h>
#include <maya/MUIDrawManager.h>
#include <maya/MGLFunctionTable.h>

// Viewport 2.0 includes
#include <maya/MDrawContext.h>
#include <maya/MMatrix.h>
#include <maya/MHWGeometry.h>

#undef ENABLE_TRACE_API_CALLS
#ifdef ENABLE_TRACE_API_CALLS
#define TRACE_API_CALLS(x) cerr << "cgfxShader: "<<(x)<<"\n"
#else
#define TRACE_API_CALLS(x)
#endif


#if defined(_SWATCH_RENDERING_SUPPORTED_)
	// For swatch rendering
	#include <maya/MHardwareRenderer.h>
	#include <maya/MGeometryData.h>
	#include <maya/MHWShaderSwatchGenerator.h>
#endif

#include <maya/MImage.h>
#include "nv_dds.h"

#ifdef _WIN32
	#include <Mmsystem.h>	// for timeGetTime
#else
	#include <sys/timeb.h>
	#include <string.h>
	#include <limits.h>		// for UINT_MAX

	#define stricmp strcasecmp
	#define strnicmp strncasecmp
#endif

#define GLOBJECT_BUFFER_OFFSET(i) ((char *)NULL + (i)) // For GLObject offsets

//
// Statics and globals...
//

PFNGLCLIENTACTIVETEXTUREARBPROC glStateCache::glClientActiveTexture = 0;
PFNGLVERTEXATTRIBPOINTERARBPROC glStateCache::glVertexAttribPointer = 0;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC glStateCache::glEnableVertexAttribArray = 0;
PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glStateCache::glDisableVertexAttribArray = 0;
PFNGLVERTEXATTRIB4FARBPROC glStateCache::glVertexAttrib4f = 0;
PFNGLSECONDARYCOLORPOINTEREXTPROC glStateCache::glSecondaryColorPointer = 0;
PFNGLSECONDARYCOLOR3FEXTPROC glStateCache::glSecondaryColor3f = 0;
PFNGLMULTITEXCOORD4FARBPROC glStateCache::glMultiTexCoord4fARB = 0;

int glStateCache::sMaxTextureUnits = 0;

glStateCache::glStateCache()
{
	reset();
}


glStateCache glStateCache::gInstance;

void glStateCache::activeTexture( int i)
{
	if( i != fActiveTextureUnit)
	{
		fActiveTextureUnit = i;
		if( glStateCache::glClientActiveTexture)
			glStateCache::glClientActiveTexture( GL_TEXTURE0_ARB + i );
	}
}

void glStateCache::enableVertexAttrib( int i)
{
	if( !(fEnabledRegisters & (1 << (glRegister::kVertexAttrib + i))))
	{
		if( glStateCache::glEnableVertexAttribArray)
			glStateCache::glEnableVertexAttribArray( i);
		fEnabledRegisters |= (1 << (glRegister::kVertexAttrib + i));
	}
	fRequiredRegisters |= (1 << (glRegister::kVertexAttrib + i));
}


void glStateCache::flushState()
{
	// Work out which registers are enabled, but no longer required
	long redundantRegisters = fEnabledRegisters & ~fRequiredRegisters;
	//printf( "State requires %d, enabled %d, redundant %d\n", fRequiredRegisters, fEnabledRegisters, redundantRegisters);

	// Disable them
	if( redundantRegisters & (1 << glRegister::kPosition))
		glDisableClientState(GL_VERTEX_ARRAY);
	if( redundantRegisters & (1 << glRegister::kNormal))
		glDisableClientState(GL_NORMAL_ARRAY);
	if( redundantRegisters & (1 << glRegister::kColor))
		glDisableClientState(GL_COLOR_ARRAY);
	if( redundantRegisters & (1 << glRegister::kSecondaryColor))
		glDisableClientState(GL_SECONDARY_COLOR_ARRAY_EXT);
	for( int i = glRegister::kTexCoord; i <= glRegister::kLastTexCoord; i++)
	{
		if( redundantRegisters & (1 << i))
		{
			activeTexture( i - glRegister::kTexCoord);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
	}
	for( int i = glRegister::kVertexAttrib; i <= glRegister::kLastVertexAttrib; i++)
	{
		if( redundantRegisters & (1 << i))
		{
			if( glStateCache::glDisableVertexAttribArray)
				glStateCache::glDisableVertexAttribArray( i - glRegister::kVertexAttrib);
		}
	}
	fEnabledRegisters = fRequiredRegisters;
	fRequiredRegisters = 0;
}


// This typeid must be unique across the universe of Maya plug-ins.
//
// TODO: Get a unique ID from NVIDIA if they have them or from A|W
// if they do not.
//

#ifdef _WIN32
MTypeId     cgfxShaderNode::sId( 4084862000 );
#else
MTypeId     cgfxShaderNode::sId( 0xF37A0C30 );
#endif

CGcontext		cgfxShaderNode::sCgContext;

cgfxShaderNode::Effect2NodesMap cgfxShaderNode::sEffect2NodesMap;

// Attribute declarations
//
MObject     cgfxShaderNode::sShader;
MObject     cgfxShaderNode::sTechnique;
MObject     cgfxShaderNode::sProfile;
MObject		cgfxShaderNode::sAttributeList;
MObject		cgfxShaderNode::sVertexAttributeList;
MObject     cgfxShaderNode::sVertexAttributeSource;
MObject     cgfxShaderNode::sTexCoordSource;
MObject     cgfxShaderNode::sColorSource;
MObject     cgfxShaderNode::sTexturesByName;


// Codes used in ftexCoordList array
enum ETexCoord
{
	etcNull      = -1,
	etcConstant  = -2,
	etcNormal    = -3,
	etcTangent   = -4,
	etcBinormal  = -5,
	etcDataSet	 = -6,
};



//--------------------------------------------------------------------//
// Constructor:
//
cgfxShaderNode::cgfxShaderNode()
:   fCurrentTechnique( NULL)
,   fVertexAttributes( NULL)
#ifdef TEXTURES_BY_NAME
,	fTexturesByName( true )
#else
,	fTexturesByName( false )
#endif
,   fNormalsPerVertex( 3 )
,   fPassStateSetters( NULL )
,   fConstructed(false)
,   fErrorCount( 0 )
,   fErrorLimit( 8 )
,   fProfileName( "" )
,	fLastShaderFxFileAtVASSet( "" )
,	fShaderFxFile()
,	fShaderFxFileChanged( false )
,   fGeomReqDataVersionId( 0 )
,   fUVEditorTexture( NULL )
{
	// Set texCoordSource attribute to its default value.
	MStringArray sa;
	sa.append( "map1" );
	sa.append( "tangent" );
	sa.append( "binormal" );
	MStringArray sa2;
	sa2.append( "colorSet1" );
	setDataSources( &sa, &sa2 );
}


// Post-constructor
void
cgfxShaderNode::postConstructor()
{
	fConstructed = true;

	// Watch for attribute removals, see comments in the callback for details
	MObject thisObj = thisMObject();
	fCallbackIds.append(
		MNodeMessage::addAttributeAddedOrRemovedCallback(
			thisObj,
			attributeAddedOrRemovedCB,
			(void*)MPxHwShaderNode::getHwShaderNodePtr(thisObj)));
}


// Destructor:
//
cgfxShaderNode::~cgfxShaderNode()
{
	// Remove effect - node association
	cgfxShaderNode::removeAssociation(this, fEffect);

#ifdef KH_DEBUG
	MString ss = "  .. ~node ";
	if ( fConstructed )
	{
		MFnDependencyNode fnNode( thisMObject() );
		ss += fnNode.name();
	}
	ss += "\n";
	::OutputDebugString( ss.asChar() );
#endif

    // Free up any the textures referenced by the attributes. We have
	// to perform this manually becauce the attribute list might be
	// kept alive by the undo queue.
    if (!fAttrDefList.isNull()) {
        fAttrDefList->releaseTextures();
    }

	// Remove all the callbacks that we registered.
	MMessage::removeCallbacks( fCallbackIds );
	fCallbackIds.clear();

    if (fUVEditorTexture) {
	    MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
	    if (theRenderer) {
        	MHWRender::MTextureManager*	txtManager = theRenderer->getTextureManager();
        	if (txtManager) {
    	        txtManager->releaseTexture(fUVEditorTexture);
            }
        }
    }

    delete [] fPassStateSetters;
}

//
//	Description:
//		This method computes the value of the given output plug based
//		on the values of the input attributes.
//
//	Arguments:
//		plug - the plug to compute
//		data - object that provides access to the attributes for this node
//
MStatus cgfxShaderNode::compute( const MPlug& plug, MDataBlock& data )
{
	MStatus returnStatus;

	// Compute a color, so that Hypershade swatches do not render black.
	if ((plug == outColor) || (plug.parent() == outColor))
	{
		MFloatVector color(.07f, .8f, .07f);
		MDataHandle outputHandle = data.outputValue( outColor );
		outputHandle.asFloatVector() = color;
		outputHandle.setClean();
		return MS::kSuccess;
	}

	return MS::kUnknownParameter;
}

// ========== cgfxShaderNode::creator ==========
//
//	Description:
//		this method exists to give Maya a way to create new objects
//      of this type.
//
//	Return Value:
//		a new object of this type.
//
/* static */
void* cgfxShaderNode::creator()
{
	return new cgfxShaderNode();
}


// ========== cgfxShaderNode::initialize ==========
//
//	Description:
//		This method is called to create and initialize all of the attributes
//      and attribute dependencies for this node type.  This is only called
//		once when the node type is registered with Maya.
//
//	Return Values:
//		MS::kSuccess
//		MS::kFailure
//
/* static */
MStatus
cgfxShaderNode::initialize()
{
	MStatus ms;

	try
	{
		initializeNodeAttrs();
	}
	catch ( cgfxShaderCommon::InternalError* e )   // internal error
	{
		size_t ee = (size_t)e;
		MString es = "cgfxShaderNode internal error ";
		es += (int)ee;
		MGlobal::displayError( es );
		ms = MS::kFailure;
	}
	catch ( ... )
	{
		MString es = "cgfxShaderNode internal error: Unhandled exception in initialize";
		MGlobal::displayError( es );
		ms = MS::kFailure;
	}

	return ms;
}


// Create all the attributes.
/* static */
void
cgfxShaderNode::initializeNodeAttrs()
{
	MFnTypedAttribute	typedAttr;
	MFnNumericAttribute	numericAttr;
	MFnStringData		stringData;
	MFnStringArrayData	stringArrayData;
	MStatus				stat, stat2;

	// The shader attribute holds the name of the .fx file that defines
	// the shader
	//
	sShader = typedAttr.create("shader", "s", MFnData::kString, stringData.create(&stat2), &stat);
	M_CHECK( stat2 );
	M_CHECK( stat );

	// Attribute is keyable and will show up in the channel box
	//
	stat = typedAttr.setKeyable(true);
	M_CHECK( stat );

	// Mark it as internal so we can track changes to it and know when to
	// reload the .fx file
	//
	stat = typedAttr.setInternal(true);
	M_CHECK( stat );

	// Attribute affects VP2.0 viewport appearance
	//
	stat = typedAttr.setAffectsAppearance(true);
	M_CHECK( stat );

	// Add the attribute we have created to the node
	//
	stat = addAttribute(sShader);
	M_CHECK( stat );

	//
	// technique
	//

	sTechnique = typedAttr.create("technique", "t", MFnData::kString,
		stringData.create(&stat2), &stat);
	M_CHECK( stat2 );
	M_CHECK( stat );

	// Attribute is keyable and will show up in the channel box
	//
	stat = typedAttr.setKeyable(true);
	M_CHECK( stat );

	// Mark it as internal so we can track changes to it and know when to
	// set the technique.
	//
	stat = typedAttr.setInternal(true);
	M_CHECK( stat );

	// Attribute affects VP2.0 viewport appearance
	//
	stat = typedAttr.setAffectsAppearance(true);
	M_CHECK( stat );

	// Add the attribute we have created to the node
	//
	stat = addAttribute(sTechnique);
	M_CHECK( stat );

	//
    // Profile
	//
	sProfile = typedAttr.create("profile", "p", MFnData::kString,
                                stringData.create(&stat2), &stat);
	M_CHECK( stat2 );
	M_CHECK( stat );

	// Attribute is keyable and will show up in the channel box
	//
	stat = typedAttr.setKeyable(true);
	M_CHECK( stat );

	// Mark it as internal so we can track changes to it and know when to
	// set the profile and recompile the Cg programs.
	//
	stat = typedAttr.setInternal(true);
	M_CHECK( stat );

	// Attribute affects VP2.0 viewport appearance
	//
	stat = typedAttr.setAffectsAppearance(true);
	M_CHECK( stat );

	// Add the attribute we have created to the node
	//
	stat = addAttribute(sProfile);
	M_CHECK( stat );

	//
	// attributeList (uniform parameters)
	//

	sAttributeList = typedAttr.create("attributeList", "al", MFnData::kStringArray,
		stringArrayData.create(&stat2), &stat);
	M_CHECK( stat2 );
	M_CHECK( stat );

	// Attribute is NOT keyable and will NOT show up in the channel box
	//
	stat = typedAttr.setKeyable(false);
	M_CHECK( stat );

	// Attribute is NOT connectable
	//
	stat = typedAttr.setConnectable(false);
	M_CHECK( stat );

	// This attribute is an NOT an array.  (It is a single valued attribute
	// whose value is a single MStringArray object.)
	//
	stat = typedAttr.setArray(false);
	M_CHECK( stat );

	// Mark it as internal so we can track changes to it and know when to
	// reload the .fx file
	//
	stat = typedAttr.setInternal(true);
	M_CHECK( stat );

	// This attribute is a hidden.
	//
	stat = typedAttr.setHidden(true);
	M_CHECK( stat );

	// Attribute affects VP2.0 viewport appearance
	//
	stat = typedAttr.setAffectsAppearance(true);
	M_CHECK( stat );

	// Add the attribute we have created to the node
	//
	stat = addAttribute(sAttributeList);
	M_CHECK( stat );

	//
	// vertexAttributeList (varying parameters)
	//

	sVertexAttributeList = typedAttr.create("vertexAttributeList", "val", MFnData::kStringArray,
		stringArrayData.create(&stat2), &stat);
	M_CHECK( stat2 );
	M_CHECK( stat );

	// Attribute is NOT keyable and will NOT show up in the channel box
	//
	stat = typedAttr.setKeyable(false);
	M_CHECK( stat );

	// Attribute is NOT connectable
	//
	stat = typedAttr.setConnectable(false);
	M_CHECK( stat );

	// This attribute is an NOT an array.  (It is a single valued attribute
	// whose value is a single MStringArray object.)
	//
	stat = typedAttr.setArray(false);
	M_CHECK( stat );

	// Mark it as internal so we can track changes to it and know when to
	// reload the .fx file
	//
	stat = typedAttr.setInternal(true);
	M_CHECK( stat );

	// This attribute is a hidden.
	//
	stat = typedAttr.setHidden(true);
	M_CHECK( stat );

	// Attribute affects VP2.0 viewport appearance
	//
	stat = typedAttr.setAffectsAppearance(true);
	M_CHECK( stat );

	// Add the attribute we have created to the node
	//
	stat = addAttribute(sVertexAttributeList);
	M_CHECK( stat );

	//
	// vertexAttributeSource
	//
	sVertexAttributeSource = typedAttr.create( "vertexAttributeSource", "vas", MFnData::kStringArray,
		MObject::kNullObj, &stat );
	M_CHECK( stat );
	stat = typedAttr.setInternal( true );
	M_CHECK( stat );
	stat = typedAttr.setAffectsAppearance(true);
	M_CHECK( stat );
	stat = addAttribute( sVertexAttributeSource );
	M_CHECK( stat );


	//
	// texCoordSource
	//
	sTexCoordSource = typedAttr.create( "texCoordSource", "tcs", MFnData::kStringArray,
		MObject::kNullObj, &stat );
	M_CHECK( stat );
	stat = typedAttr.setInternal( true );
	M_CHECK( stat );
	stat = typedAttr.setAffectsAppearance(true);
	M_CHECK( stat );
	stat = addAttribute( sTexCoordSource );
	M_CHECK( stat );


	//
	// colorSource
	//
	sColorSource = typedAttr.create( "colorSource", "cs", MFnData::kStringArray,
		MObject::kNullObj, &stat );
	M_CHECK( stat );
	stat = typedAttr.setInternal( true );
	M_CHECK( stat );
	stat = typedAttr.setAffectsAppearance(true);
	M_CHECK( stat );
	stat = addAttribute( sColorSource );
	M_CHECK( stat );


	//
	// texturesByName
	//
	sTexturesByName = numericAttr.create( "texturesByName", "tbn", MFnNumericData::kBoolean,
		0, &stat );
	M_CHECK( stat );
	stat = numericAttr.setInternal(true);
	M_CHECK( stat );
	stat = typedAttr.setAffectsAppearance(true);
	M_CHECK( stat );
	// Hide this switch - TDs can recompile this to default to
	// different options, but we don't want to encourage users
	// to switch some shading nodes to use node textures, and
	// others named textures (and we definitely don't want to
	// try and handle converting configured shaders from one to
	// the other)
	//
	stat = numericAttr.setHidden(true);
	M_CHECK( stat );
	numericAttr.setKeyable(false);
	stat = addAttribute( sTexturesByName );
	M_CHECK( stat );

}                                      // cgfxShaderNode::initializeNodeAttrs

/* virtual */
void
cgfxShaderNode::copyInternalData( MPxNode* pSrc )
{
	const cgfxShaderNode& src = *(cgfxShaderNode*)pSrc;
	setTexturesByName( src.getTexturesByName() );
	setShaderFxFile( src.shaderFxFile() );
	setShaderFxFileChanged( true );
	setDataSources( &src.getTexCoordSource(), &src.getColorSource() );

    // Flush the effect, since we are going to reload the Fx from the
    // file.
    fEffect = cgfxRCPtr<const cgfxEffect>();
    fCurrentTechnique = NULL;

	// Rebuild the shader from the fx file.
	//
	MString fileName = cgfxFindFile(shaderFxFile());
	bool hasFile = (fileName.asChar() != NULL) && strcmp(fileName.asChar(), "");
	if ( hasFile )
	{
		// Create the effect for this node.
		//
		const cgfxRCPtr<const cgfxEffect> effect = cgfxEffect::loadEffect(fileName, cgfxProfile::getProfile(src.getProfile()));

		if (effect->isValid())
		{
            cgfxRCPtr<cgfxAttrDefList> effectList;
			MStringArray attributeList;
			MDGModifier dagMod;

			// Update the node.
			//
			cgfxAttrDef::updateNode(effect, this, &dagMod, effectList, attributeList);
#ifndef NDEBUG
			MStatus status =
#endif
				dagMod.doIt();

			assert(status == MS::kSuccess);

			setAttrDefList(effectList);
			setAttributeList(attributeList);
			setEffect(effect);
		}
	}

    setTechnique( src.getTechnique() );
	setProfile( src.getProfile() );
}

// cgfxShaderNode::copyInternalData

bool cgfxShaderNode::setInternalValueInContext( const MPlug& plug,
											  const MDataHandle& handle,
											  MDGContext&)
{
	bool retVal = true;

	try
	{
#ifdef KH_DEBUG
		MString ss = "  .. seti ";
		ss += plug.partialName( true, true, true, false, false, true );
		if (plug == sShader ||
			plug == sTechnique)
		{
			ss += " \"";
			ss += handle.asString();
			ss += "\"";
		}
		ss += "\n";
		::OutputDebugString( ss.asChar() );
#endif
		if (plug == sShader)
		{
			setShaderFxFile(handle.asString());
		}
		else if (plug == sTechnique)
		{
			setTechnique(handle.asString());
		}
		else if (plug == sProfile)
		{
			setProfile(handle.asString());
		}
		else if (plug == sAttributeList)
		{
			MDataHandle nonConstHandle(handle);
			MObject saData = nonConstHandle.data();
			MFnStringArrayData fnSaData(saData);
			setAttributeList(fnSaData.array());
		}
		else if (plug == sVertexAttributeList)
		{
			MDataHandle nonConstHandle(handle);
			MObject saData = nonConstHandle.data();
			MFnStringArrayData fnSaData(saData);
			const MStringArray& attributeList = fnSaData.array();

			cgfxRCPtr<cgfxVertexAttribute> attributes;
			cgfxRCPtr<cgfxVertexAttribute>* nextAttribute = &attributes;
			int numAttributes = attributeList.length() / 4;
			for( int i = 0; i < numAttributes; i++)
			{
				cgfxRCPtr<cgfxVertexAttribute> attribute = cgfxRCPtr<cgfxVertexAttribute>(new cgfxVertexAttribute());
				attribute->fName = attributeList[ i * 4 + 0];
				attribute->fType = attributeList[ i * 4 + 1];
				attribute->fUIName = attributeList[ i * 4 + 2];
				attribute->fSemantic = attributeList[ i * 4 + 3];
				*nextAttribute = attribute;
				nextAttribute = &attribute->fNext;
			}
			setVertexAttributes( attributes );
		}
		else if ( plug == sVertexAttributeSource )
		{
			MDataHandle nonConstHandle( handle );
			MObject     saData = nonConstHandle.data();
			MFnStringArrayData fnSaData( saData );
			MStringArray values = fnSaData.array();
			setVertexAttributeSource( values);
		}
		else if ( plug == sTexCoordSource )
		{
			MDataHandle nonConstHandle( handle );
			MObject     saData = nonConstHandle.data();
			MFnStringArrayData fnSaData( saData );
			MStringArray values = fnSaData.array();
			setDataSources( &values, NULL );
		}
		else if ( plug == sColorSource )
		{
			MDataHandle nonConstHandle( handle );
			MObject     saData = nonConstHandle.data();
			MFnStringArrayData fnSaData( saData );
			MStringArray values = fnSaData.array();
			setDataSources( NULL, &values );
		}
		else if ( plug == sTexturesByName )
		{
			setTexturesByName( handle.asBool(), !MFileIO::isReadingFile());
		}
		else
		{
			retVal = MPxHwShaderNode::setInternalValue(plug, handle);
		}
	}
	catch ( cgfxShaderCommon::InternalError* e )
	{
		reportInternalError( __FILE__, (size_t)e );
		retVal = false;
	}
	catch ( ... )
	{
		reportInternalError( __FILE__, __LINE__ );
		retVal = false;
	}

	return retVal;
}


/* virtual */
bool cgfxShaderNode::getInternalValueInContext( const MPlug& plug,
											  MDataHandle& handle,
											  MDGContext&)
{
	bool retVal = true;

	try
	{
#ifdef KH_DEBUG
		MString ss = "  .. geti ";
		ss += plug.partialName( true, true, true, false, false, true );
		if ( plug == sShader )
			ss += " \"" + fShaderFxFile + "\"";
		else if (plug == sTechnique)
			ss += " \"" + fTechnique + "\"";
		ss += "\n";
		::OutputDebugString( ss.asChar() );
#endif
		if (plug == sShader)
		{
			handle.set(fShaderFxFile);
		}
		else if (plug == sTechnique)
		{
			handle.set(fTechnique);
		}
		else if (plug == sProfile)
		{
			handle.set(fProfileName);
		}
		else if (plug == sAttributeList)
		{
			MFnStringArrayData saData;
			handle.set(saData.create(fAttributeListArray));
		}
		else if (plug == sVertexAttributeList)
		{

			MStringArray attributeList;
			cgfxRCPtr<cgfxVertexAttribute> attribute = fVertexAttributes;
			while( attribute.isNull() == false)
			{
				attributeList.append( attribute->fName);
				attributeList.append( attribute->fType);
				attributeList.append( attribute->fUIName);
				attributeList.append( attribute->fSemantic);
				attribute = attribute->fNext;
			}

			MFnStringArrayData saData;
			handle.set(saData.create( attributeList));
		}
		else if ( plug == sVertexAttributeSource )
		{
			MStringArray attributeSources;
			cgfxRCPtr<cgfxVertexAttribute> attribute = fVertexAttributes;
			while( attribute.isNull() == false)
			{
				attributeSources.append( attribute->fSourceName);
				attribute = attribute->fNext;
			}

			MFnStringArrayData saData;
			handle.set( saData.create( attributeSources ) );
		}
		else if ( plug == sTexCoordSource )
		{
			MFnStringArrayData saData;
			handle.set( saData.create( fTexCoordSource ) );
		}
		else if ( plug == sColorSource )
		{
			MFnStringArrayData saData;
			handle.set( saData.create( fColorSource ) );
		}
		else if (plug == sTexturesByName)
		{
			handle.set(fTexturesByName);
		}
		else
		{
			retVal = MPxHwShaderNode::getInternalValue(plug, handle);
		}
	}
	catch ( cgfxShaderCommon::InternalError* e )
	{
		reportInternalError( __FILE__, (size_t)e );
		retVal = false;
	}
	catch ( ... )
	{
		reportInternalError( __FILE__, __LINE__ );
		retVal = false;
	}

	return retVal;
}


static void checkGlErrors(const char* msg)
{
#if defined(CGFX_DEBUG)
#define MYERR(n)	case n: OutputDebugStrings("    ", #n); break

	GLenum err;
	bool errors = false;

	while ((err = glGetError()) != GL_NO_ERROR)
	{
		if (!errors)
		{
			// Print this the first time through the loop
			//
			OutputDebugStrings("OpenGl errors: ", msg);
		}

		errors = true;

		switch (err)
		{
			MYERR(GL_INVALID_ENUM);
			MYERR(GL_INVALID_VALUE);
			MYERR(GL_INVALID_OPERATION);
			MYERR(GL_STACK_OVERFLOW);
			MYERR(GL_STACK_UNDERFLOW);
			MYERR(GL_OUT_OF_MEMORY);
		default:
			{
				char tmp[32];
				sprintf(tmp, "%d", err);
				OutputDebugStrings("    GL Error #", tmp);
			}
		}
	}
#undef MYERR
#endif /* CGFX_DEBUG */
}


// Handle a change in a connected texture
//
void textureChangedCallback( MNodeMessage::AttributeMessage msg, MPlug & plug, MPlug & otherPlug, void* aDefVoid)
{
    cgfxAttrDef* aDef = (cgfxAttrDef*)aDefVoid;

    MStatus status;
    MFnAttribute plugAttr(plug.attribute(&status));
    assert(status == MS::kSuccess);
    if (!status) {
        return;
    }

    if (plugAttr.name() == "fileTextureName") {

        MFnDependencyNode textureNode(plug.node());
        MPlug outPlug(textureNode.findPlug("outColor", true));

        for( MItDependencyGraph iter(outPlug); !iter.isDone(); iter.next()) {
            MPlug oplug(iter.thisPlug());
            if (oplug.attribute() == aDef->fAttr) {
                // This callback invocation is not for a texture attribute.
                // Whenever there is a change in our texture's attributes (which
                // could also be texture node deleted or disconnected), remove
                // our callback to signify that this texture needs to be refreshed.
                // We don't release the GL texture here because there may not be
                // a valid GL context around when the DG is being updated. The
                // texture will get flushed at the next draw time when the bind
                // code determines there is a node but no callback.
                aDef->releaseCallback();

                // We mark the texture as staled in the texture cache. If we don't
                // do that, it won't be read back again from disk.
                aDef->fTexture->markAsStaled();
            }
        }
    }
}


#if defined(_SWATCH_RENDERING_SUPPORTED_)
/* virtual */
MStatus cgfxShaderNode::renderSwatchImage( MImage & outImage )
{

	MStatus status = MStatus::kFailure;
	if( sCgContext == 0 )	return status;

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

		if( status2 != MS::kSuccess )
		{
			pRenderer->dereferenceGeometry( pGeomData, numberOfData );
			return status2;
		}

		glPushAttrib ( GL_ALL_ATTRIB_BITS );
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		// Get the light direction from the API, and use it
		// =============================================
		{
			float light_pos[4];
			pRenderer->getSwatchLightDirection( light_pos[0], light_pos[1], light_pos[2], light_pos[3] );
		}

		// Get camera
		// ==========
		{
			// Get the camera frustum from the API
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			double l, r, b, t, n, f;
			pRenderer->getSwatchPerspectiveCameraSetting( l, r, b, t, n, f );
			glFrustum( l, r, b, t, n, f );

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			float x, y, z, w;
			pRenderer->getSwatchPerspectiveCameraTranslation( x, y, z, w );
			glTranslatef( x, y, z );
		}

		// Get the default background color and clear the background
		//
		float r, g, b, a;
		MHWShaderSwatchGenerator::getSwatchBackgroundColor( r, g, b, a );
		glClearColor( r, g, b, a );
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glShadeModel(GL_SMOOTH);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		// Draw The Swatch
		// ===============
		//drawTheSwatch( pGeomData, pIndexing, numberOfData, indexCount );
		MDagPath dummyPath;
		glBind( dummyPath );

		float *vertexData = (float *)( pGeomData[0].data() );
		float *normalData = (float *)( pGeomData[1].data() );
		float *uvData = (float *)( pGeomData[2].data() );
		float *tangentData = (float *)( pGeomData[3].data() );
		float *binormalData = (float *)( pGeomData[4].data() );

		// Stick uvs into ptr array
		int uvCount = fUVSets.length();
		float ** texCoordArrays = uvCount ? new float * [ uvCount] : NULL;
		for( int uv = 0; uv < uvCount; uv++)
		{
			texCoordArrays[ uv] = uvData;
		}

		// Stick normal, tangent, binormals into ptr array
		int normalCount = uvCount > 0 ? uvCount : 1;
		float ** normalArrays = new float * [ fNormalsPerVertex * normalCount];
		for( int n = 0; n < normalCount; n++)
		{
			if( fNormalsPerVertex > 0)
			{
				normalArrays[ n * fNormalsPerVertex + 0] = normalData;
				if( fNormalsPerVertex > 1)
				{
					normalArrays[ n * fNormalsPerVertex + 1] = tangentData;
					if( fNormalsPerVertex > 2)
					{
						normalArrays[ n * fNormalsPerVertex + 2] = binormalData;
					}
				}
			}
		}

		glGeometry( dummyPath,
					GL_TRIANGLES,
					false,
					indexCount,
					pIndexing,
					pGeomData[0].elementCount(),
					NULL, /* no vertex ids */
					vertexData,
					fNormalsPerVertex,
					(const float **) normalArrays,
					0,
					NULL, /* no colours */
					uvCount,
					(const float **) texCoordArrays);


		glUnbind( dummyPath );

		if( normalArrays) delete[] normalArrays;
		if( texCoordArrays) delete[] texCoordArrays;

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

		//restore matrix and gl state
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glPopAttrib();

		//dereference geometry after rendering
		pRenderer->dereferenceGeometry( pGeomData, numberOfData );
	}
	return status;
}
#endif

// Tell Maya that Cg effects can be batched
//
bool cgfxShaderNode::supportsBatching() const
{
	return true;
}


// Tell Maya to invert texture coordinates for this shader
// This function is only called in the old interface: glBind/glGeometry/glUnbind
bool cgfxShaderNode::invertTexCoords() const
{
    if (cgfxProfile::getTexCoordOrientation() == cgfxProfile::TEXCOORD_OPENGL)
        return false;
    else
        return true;
}


// Try and create a missing effect (e.g. once a GL context is available)
//
bool cgfxShaderNode::createEffect()
{
	// Attempt to read the effect from the file. But only when it has
	// changed file name. In the case where the file cannot be found
	// we will not continuously search for the same file while refreshing.
	// The user will need to manually "refresh" the file name, or change
	// it to force a new attempt to load the file here.
	//
	bool rc = false;
	if (shaderFxFileChanged())
	{
		MString fileName = cgfxFindFile(shaderFxFile());

		if(fileName.asChar() != NULL && strcmp(fileName.asChar(), ""))
		{
			// Compile and create the effect.
			const cgfxRCPtr<const cgfxEffect> effect = cgfxEffect::loadEffect(fileName, cgfxProfile::getProfile(fProfileName));

			if (effect->isValid())
			{
				cgfxRCPtr<cgfxAttrDefList> effectList;
				MStringArray attributeList;
				MDGModifier dagMod;
				// updateNode does a fair amount of work.  It determines which
				// attributes need to be added and which need to be deleted and
				// fills in all the changes in the MDagModifier.  Then it builds
				// a new value for the attributeList attribute.  Finally, it
				// builds a new value for the attrDefList internal value.  All
				// these values are returned here where we can set them into the
				// node.
				cgfxAttrDef::updateNode(effect, this, &dagMod, effectList, attributeList);
#ifndef NDEBUG
				MStatus status =
#endif
					dagMod.doIt();
				assert(status == MS::kSuccess);

				// Actually update the node.
				setAttrDefList(effectList);
				setAttributeList(attributeList);
				setEffect(effect);
				setTechnique(fTechnique);
				rc = true;
			}
		}
		setShaderFxFileChanged( false );
	}
	return rc;
}


/* virtual */
MStatus cgfxShaderNode::glBind(const MDagPath& shapePath)
{
	// This is the routine where you would do all the expensive,
	// one-time kind of work.  Create vertex programs, load
	// textures, etc.
	//
	glStateCache::instance().reset();

	// Since we have no idea what the effect may set, we have
	// to push everything.

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	// In this case, we will evaluate all the attributes and store the
	// parameter values.  In theory, there could be multiple calls to
	// geometry in between single calls to bind and unbind.  Since we
	// only need to get the attribute values once per frame, get them
	// in bind.

	MStatus stat;

#ifdef KH_DEBUG
	MString ss = "  .. bind ";
	if ( this && fConstructed )
		ss += name();
	ss += " ";
	ss += request.multiPath().fullPathName();
	ss += "\n";
	::OutputDebugString( ss.asChar() );
#endif

	try
	{
		// One-time OpenGL initialization...
		if ( glStateCache::sMaxTextureUnits <= 0 )
		{
			// Before this point, we never had a good OpenGL context.  Now
			// we can check for extensions and set up pointers to the
			// extension procs.
#ifdef _WIN32
#define RESOLVE_GL_EXTENSION( fn, ext) wglGetProcAddress( #fn #ext)
#elif defined LINUX
#define RESOLVE_GL_EXTENSION( fn, ext) &fn ## ext
#else
#define RESOLVE_GL_EXTENSION( fn, ext) &fn
#endif

			glStateCache::glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREARBPROC) RESOLVE_GL_EXTENSION( glClientActiveTexture, ARB);
			glStateCache::glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERARBPROC) RESOLVE_GL_EXTENSION( glVertexAttribPointer, ARB);
			glStateCache::glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYARBPROC) RESOLVE_GL_EXTENSION( glEnableVertexAttribArray, ARB);
			glStateCache::glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC) RESOLVE_GL_EXTENSION( glDisableVertexAttribArray, ARB);
			glStateCache::glVertexAttrib4f = (PFNGLVERTEXATTRIB4FARBPROC) RESOLVE_GL_EXTENSION( glVertexAttrib4f, ARB);
			glStateCache::glSecondaryColorPointer = (PFNGLSECONDARYCOLORPOINTEREXTPROC) RESOLVE_GL_EXTENSION( glSecondaryColorPointer, EXT);
			glStateCache::glSecondaryColor3f = (PFNGLSECONDARYCOLOR3FEXTPROC) RESOLVE_GL_EXTENSION( glSecondaryColor3f, EXT);
			glStateCache::glMultiTexCoord4fARB = (PFNGLMULTITEXCOORD4FARBPROC) RESOLVE_GL_EXTENSION( glMultiTexCoord4f, ARB);

			// Don't use GL_MAX_TEXTURE_UNITS as this does not provide a proper
			// count when the # of image or texcoord inputs differs
			// from the conventional (older) notion of texture unit.
			//
			// Instead take the minimum of GL_MAX_TEXTURE_COORDS_ARB and
			// GL_MAX_TEXUTRE_IMAGE_UNITS_ARB according to the
			// ARB_FRAGMENT_PROGRAM specification.
			//
			GLint tval;
			glGetIntegerv( GL_MAX_TEXTURE_COORDS_ARB, &tval );
			GLint mic = 0;
			glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &mic );
			if (mic < tval)
				tval = mic;

			// Don't use this...
			//glGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &tval );
			glStateCache::sMaxTextureUnits = tval;
			if (!glStateCache::glClientActiveTexture || glStateCache::sMaxTextureUnits < 1)
				glStateCache::sMaxTextureUnits = 1;
			else if (glStateCache::sMaxTextureUnits > CGFXSHADERNODE_GL_TEXTURE_MAX)
				glStateCache::sMaxTextureUnits = CGFXSHADERNODE_GL_TEXTURE_MAX;
		}

		// Try and grab the first pass of our effect
		if(fCurrentTechnique && fCurrentTechnique->isValid())
		{
			// Set up the uniform attribute values for the effect.
			bindAttrValues();

			// Set depth function properly in case we have multi-pass
			if (fCurrentTechnique->hasBlending())
				glPushAttrib( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
			glGetBooleanv( GL_DEPTH_TEST, &fDepthEnableState);
			glGetIntegerv( GL_DEPTH_FUNC, &fDepthFunc);
			glGetIntegerv( GL_BLEND_SRC, &fBlendSourceFactor);
			glGetIntegerv( GL_BLEND_DST, &fBlendDestFactor);
			glDepthFunc(GL_LEQUAL);
		}
		else
		{
			// There is no effect.  Either they never set one or the one provided
			// failed to compile.  Just use this default material which is sort
			// of a shiny salmon-pink color.  It looks like nothing that Maya
			// creates by default but still lets you see your geometry.
			//
			glPushAttrib( GL_LIGHTING);
			static float diffuse_color[4]  = {1.0, 0.5, 0.5, 1.0};
			static float specular_color[4] = {1.0, 1.0, 1.0, 1.0};

			glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
			glEnable(GL_COLOR_MATERIAL);
			glColor4fv(diffuse_color);

			// Set up the specular color
			//
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular_color);

			// Set up a default shininess
			//
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 100.0);
		}

		checkGlErrors("cgfxShaderNode::glBind");
	}
	catch ( cgfxShaderCommon::InternalError* e )
	{
		reportInternalError( __FILE__, (size_t)e );
		stat = MS::kFailure;
	}
	catch ( ... )
	{
		reportInternalError( __FILE__, __LINE__ );
		stat = MS::kFailure;
	}

	return stat;
}                                      // cgfxShaderNode::bind


void cgfxShaderNode::bindAttrValues()
{
	if (fEffect.isNull() || !fEffect->isValid() || !fTechnique.length())
		return;

	MStatus  status;
	MObject  oNode = thisMObject();

	// This method should NEVER access the shape. If you find yourself tempted to access
	// any data from the shape here (like the matrices), be strong and resist! Any shape
	// dependent data should be set in bindAttrViewValues instead!
	//

	for ( cgfxAttrDefList::iterator it( fAttrDefList ); it; ++it )
	{                                  // loop over fAttrDefList
		cgfxAttrDef* aDef = *it;

		try
		{

			switch (aDef->fType)
			{
			case cgfxAttrDef::kAttrTypeBool:
				{
					bool tmp;
					aDef->getValue(oNode, tmp);
					cgSetParameter1i(aDef->fParameterHandle, tmp);
					break;
				}

			case cgfxAttrDef::kAttrTypeInt:
				{
					int tmp;
					aDef->getValue(oNode, tmp);
					cgSetParameter1i(aDef->fParameterHandle, tmp);
					break;
				}

			case cgfxAttrDef::kAttrTypeFloat:
				{
					float tmp;
					aDef->getValue(oNode, tmp);
					cgSetParameter1f(aDef->fParameterHandle, tmp);
					aDef->setUnitsToInternal(aDef->fParameterHandle);
					break;
				}

			case cgfxAttrDef::kAttrTypeString:
				{
					MString tmp;
					aDef->getValue(oNode, tmp);
					cgSetStringParameterValue(aDef->fParameterHandle, tmp.asChar());
					break;
				}

			case cgfxAttrDef::kAttrTypeVector2:
				{
					float tmp[2];
					aDef->getValue(oNode, tmp[0], tmp[1]);
					cgSetParameter2fv(aDef->fParameterHandle, tmp);
					aDef->setUnitsToInternal(aDef->fParameterHandle);
					break;
				}

			case cgfxAttrDef::kAttrTypeVector3:
			case cgfxAttrDef::kAttrTypeColor3:
				{
					float tmp[3];
					aDef->getValue(oNode, tmp[0], tmp[1], tmp[2]);
					cgSetParameter3fv(aDef->fParameterHandle, tmp);
					aDef->setUnitsToInternal(aDef->fParameterHandle);
					break;
				}

			case cgfxAttrDef::kAttrTypeVector4:
			case cgfxAttrDef::kAttrTypeColor4:
				{
					float tmp[4];
					aDef->getValue(oNode, tmp[0], tmp[1], tmp[2], tmp[3]);
					cgSetParameter4fv(aDef->fParameterHandle, tmp);
					aDef->setUnitsToInternal(aDef->fParameterHandle);
					break;
				}

			case cgfxAttrDef::kAttrTypeWorldDir:
			case cgfxAttrDef::kAttrTypeWorldPos:
				{
					// since it is in world space, we don't need to do extra mat computation. set the value directly.
					// Read the value
					float tmp[4];
					if (aDef->fSize == 3)
					{
						aDef->getValue(oNode, tmp[0], tmp[1], tmp[2]);
						tmp[3] = 1.0;
					}
					else
					{
						aDef->getValue(oNode, tmp[0], tmp[1], tmp[2], tmp[3]);
					}

					cgSetParameterValuefr(aDef->fParameterHandle, aDef->fSize, tmp);
					aDef->setUnitsToInternal(aDef->fParameterHandle);
					break;
				}

			case cgfxAttrDef::kAttrTypeMatrix:
				{
					MMatrix tmp;
					float tmp2[4][4];
					aDef->getValue(oNode, tmp);

					if (aDef->fInvertMatrix)
					{
						tmp = tmp.inverse();
					}

					if (!aDef->fTransposeMatrix)
					{
						tmp = tmp.transpose();
					}

					tmp.get(tmp2);
					cgSetMatrixParameterfr(aDef->fParameterHandle, &tmp2[0][0]);
					break;
				}

			case cgfxAttrDef::kAttrTypeColor1DTexture:
			case cgfxAttrDef::kAttrTypeColor2DTexture:
			case cgfxAttrDef::kAttrTypeColor3DTexture:
			case cgfxAttrDef::kAttrTypeColor2DRectTexture:
			case cgfxAttrDef::kAttrTypeNormalTexture:
			case cgfxAttrDef::kAttrTypeBumpTexture:
			case cgfxAttrDef::kAttrTypeCubeTexture:
			case cgfxAttrDef::kAttrTypeEnvTexture:
			case cgfxAttrDef::kAttrTypeNormalizationTexture:

				{
					MString texFileName;
					MObject textureNode = MObject::kNullObj;

					if( fTexturesByName)
					{
						aDef->getValue(oNode, texFileName);
					}
					else
					{
						// If we have a fileTexture node connect, get the
						// filename it is using
						MPlug srcPlug;
						aDef->getSource(oNode, srcPlug);
						MObject srcNode = srcPlug.node();
						if( srcNode != MObject::kNullObj)
						{
							MStatus rc;
							MFnDependencyNode dgFn(srcNode);
							MPlug filenamePlug = dgFn.findPlug( "fileTextureName", &rc);
							if( rc == MStatus::kSuccess)
							{
								filenamePlug.getValue( texFileName);
								textureNode = filenamePlug.node(&rc);
							}

							// attach a monitor to this texture if we don't already have one
							// Note that we don't need to worry about handling node destroyed
							// or disconnected, as both of these will trigger attribute changed
							// messages before going away, and we will deregister our callback
							// in the handler!
							if( aDef->fTextureMonitor == kNullCallback && textureNode != MObject::kNullObj)
							{
								// If we don't have a callback, this may mean our texture is dirty
								// and needs to be re-loaded (because we can't actually delete the
								// texture itself in the DG callback we need to wait until we
								// know we have a GL context - like right here)
								aDef->releaseTexture();
								aDef->fTextureMonitor =
                                    MNodeMessage::addAttributeChangedCallback(textureNode, textureChangedCallback, aDef);
							}
						}
					}

					if (aDef->fTexture.isNull() || texFileName != aDef->fStringDef)
					{
                        aDef->fStringDef = texFileName;
                        aDef->fTexture = cgfxTextureCache::instance().getTexture(
                            texFileName, textureNode, fShaderFxFile,
                            aDef->fName, aDef->fType);

                        if (!aDef->fTexture->isValid() && texFileName.length() > 0) {
                            MFnDependencyNode fnNode( oNode );
                            MString sMsg = "cgfxShader ";
                            sMsg += fnNode.name();
                            sMsg += " : failed to load texture \"";
                            sMsg += texFileName;
                            sMsg += "\".";
                            MGlobal::displayWarning( sMsg );
                        }
					}

					checkGlErrors("After loading texture");
					cgGLSetupSampler(aDef->fParameterHandle, aDef->fTexture->getTextureId());

					break;
				}
#ifdef _WIN32
			case cgfxAttrDef::kAttrTypeTime:
				{
					int ival = timeGetTime() & 0xffffff;
					float val = (float)ival * 0.001f;
					cgSetParameter1f(aDef->fParameterHandle, val);
					break;
				}
#endif
			case cgfxAttrDef::kAttrTypeOther:
			case cgfxAttrDef::kAttrTypeUnknown:
				break;

			case cgfxAttrDef::kAttrTypeObjectDir:
			case cgfxAttrDef::kAttrTypeViewDir:
			case cgfxAttrDef::kAttrTypeProjectionDir:
			case cgfxAttrDef::kAttrTypeScreenDir:

			case cgfxAttrDef::kAttrTypeObjectPos:
			case cgfxAttrDef::kAttrTypeViewPos:
			case cgfxAttrDef::kAttrTypeProjectionPos:
			case cgfxAttrDef::kAttrTypeScreenPos:

			case cgfxAttrDef::kAttrTypeWorldMatrix:
			case cgfxAttrDef::kAttrTypeViewMatrix:
			case cgfxAttrDef::kAttrTypeProjectionMatrix:
			case cgfxAttrDef::kAttrTypeWorldViewMatrix:
			case cgfxAttrDef::kAttrTypeWorldViewProjectionMatrix:
				// View dependent parameter
				break;
			default:
				M_CHECK( false );
			}                          // switch (aDef->fType)
		}
		catch ( cgfxShaderCommon::InternalError* e )
		{
			if ( ++fErrorCount <= fErrorLimit )
			{
				size_t ee = (size_t)e;
				MFnDependencyNode fnNode( oNode );
				MString sMsg = "cgfxShader warning ";
				sMsg += (int)ee;
				sMsg += ": ";
				sMsg += fnNode.name();
				sMsg += " internal error while setting parameter \"";
				sMsg += aDef->fName;
				sMsg += "\" of effect \"";
				sMsg += fShaderFxFile;
				sMsg += "\" for shape ";
				sMsg += currentPath().partialPathName();
				MGlobal::displayWarning( sMsg );
			}
		}
	}                                  // loop over fAttrDefList
}                                      // cgfxShaderNode::bindAttrValues

void
cgfxShaderNode::bindViewAttrValues(const MDagPath& shapePath)
{
	if (fEffect.isNull() || !fEffect->isValid() || !fTechnique.length())
		return;

	MStatus  status;
	MObject  oNode = thisMObject();

	MMatrix wMatrix, vMatrix, pMatrix, sMatrix;
	MMatrix wvMatrix, wvpMatrix, wvpsMatrix;
	{
		float tmp[4][4];

		if (shapePath.isValid())
			wMatrix = shapePath.inclusiveMatrix();
		else
			wMatrix.setToIdentity();

		glGetFloatv(GL_MODELVIEW_MATRIX, &tmp[0][0]);
		wvMatrix = MMatrix(tmp);

		vMatrix = wMatrix.inverse() * wvMatrix;
//
		glGetFloatv(GL_PROJECTION_MATRIX, &tmp[0][0]);
		pMatrix = MMatrix(tmp);

		wvpMatrix = wvMatrix * pMatrix;

		float vpt[4];
		float depth[2];

		glGetFloatv(GL_VIEWPORT, vpt);
		glGetFloatv(GL_DEPTH_RANGE, depth);

		// Construct the NDC -> screen space matrix
		//
		float x0, y0, z0, w, h, d;

		x0 = vpt[0];
		y0 = vpt[1];
		z0 = depth[0];
		w  = vpt[2];
		h  = vpt[3];
		d  = depth[1] - z0;

		// Make a reference to ease the typing
		//
		double* s = &sMatrix.matrix[0][0];

		s[ 0] = w/2;	s[ 1] = 0.0;	s[ 2] = 0.0;	s[ 3] = 0.0;
		s[ 4] = 0.0;	s[ 5] = h/2;	s[ 6] = 0.0;	s[ 7] = 0.0;
		s[ 8] = 0.0;	s[ 9] = 0.0;	s[10] = d/2;	s[11] = 0.0;
		s[12] = x0+w/2;	s[13] = y0+h/2;	s[14] = z0+d/2;	s[15] = 1.0;

		wvpsMatrix = wvpMatrix * sMatrix;
	}

	for ( cgfxAttrDefList::iterator it( fAttrDefList ); it; ++it )
	{                                  // loop over fAttrDefList
		cgfxAttrDef* aDef = *it;

		try
		{

			switch (aDef->fType)
			{
			case cgfxAttrDef::kAttrTypeObjectDir:
			case cgfxAttrDef::kAttrTypeViewDir:
			case cgfxAttrDef::kAttrTypeProjectionDir:
			case cgfxAttrDef::kAttrTypeScreenDir:

			case cgfxAttrDef::kAttrTypeObjectPos:
			case cgfxAttrDef::kAttrTypeViewPos:
			case cgfxAttrDef::kAttrTypeProjectionPos:
			case cgfxAttrDef::kAttrTypeScreenPos:
				{
					float tmp[4];
					if (aDef->fSize == 3)
					{
						aDef->getValue(oNode, tmp[0], tmp[1], tmp[2]);
						tmp[3] = 1.0;
					}
					else
					{
						aDef->getValue(oNode, tmp[0], tmp[1], tmp[2], tmp[3]);
					}

					// Maya's API only provides for vectors of size 3.
					// When we do the matrix multiply, it will only
					// work correctly if the 4th coordinate is 1.0
					//

					MVector vec(tmp[0], tmp[1], tmp[2]);

					int space = aDef->fType - cgfxAttrDef::kAttrTypeFirstPos;
					if (space < 0)
					{
						space = aDef->fType - cgfxAttrDef::kAttrTypeFirstDir;
					}

					MMatrix mat;  // initially the identity matrix.

					switch (space)
					{
					case 0:	/* mat = identity */	break;
					case 1:	mat = wMatrix;			break;
					case 2:	mat = wvMatrix;			break;
					case 3:	mat = wvpMatrix;		break;
					case 4:	mat = wvpsMatrix;		break;
					}

					// Maya's transformation matrices are set up with
					// the translation in row 3 (like OpenGL) rather
					// than column 3. To transform a point or vector,
					// use V*M, not M*V.   kh 11/2003
					int base = cgfxAttrDef::kAttrTypeFirstPos;
					if (aDef->fType <= cgfxAttrDef::kAttrTypeLastDir)
						base = cgfxAttrDef::kAttrTypeFirstDir;
					if (base == cgfxAttrDef::kAttrTypeFirstPos)
					{
						MPoint point(tmp[0], tmp[1], tmp[2], tmp[3]);
						point *= wMatrix.inverse() * mat;
						tmp[0] = (float)point.x;
						tmp[1] = (float)point.y;
						tmp[2] = (float)point.z;
						tmp[3] = (float)point.w;
					}
					else
					{
						MVector vec(tmp[0], tmp[1], tmp[2]);
						vec *= wMatrix.inverse() * mat;
						tmp[0] = (float)vec.x;
						tmp[1] = (float)vec.y;
						tmp[2] = (float)vec.z;
						tmp[3] = 1.F;
					}

					cgSetParameterValuefc(aDef->fParameterHandle, aDef->fSize, tmp);
					aDef->setUnitsToInternal(aDef->fParameterHandle);
					break;
				}

			case cgfxAttrDef::kAttrTypeWorldMatrix:
			case cgfxAttrDef::kAttrTypeViewMatrix:
			case cgfxAttrDef::kAttrTypeProjectionMatrix:
			case cgfxAttrDef::kAttrTypeWorldViewMatrix:
			case cgfxAttrDef::kAttrTypeWorldViewProjectionMatrix:
				{
					MMatrix mat;

					switch (aDef->fType)
					{
					case cgfxAttrDef::kAttrTypeWorldMatrix:
						mat = wMatrix; break;
					case cgfxAttrDef::kAttrTypeViewMatrix:
						mat = vMatrix; break;
					case cgfxAttrDef::kAttrTypeProjectionMatrix:
						mat = pMatrix; break;
					case cgfxAttrDef::kAttrTypeWorldViewMatrix:
						mat = wvMatrix; break;
					case cgfxAttrDef::kAttrTypeWorldViewProjectionMatrix:
						mat = wvpMatrix; break;
					default:
						break;
					}

					if (aDef->fInvertMatrix)
					{
						mat = mat.inverse();
					}

					if (!aDef->fTransposeMatrix)
					{
						mat = mat.transpose();
					}

					float tmp[4][4];
					mat.get(tmp);
					cgSetMatrixParameterfr(aDef->fParameterHandle, &tmp[0][0]);
					break;
				}
			case cgfxAttrDef::kAttrTypeHardwareFogEnabled:
			case cgfxAttrDef::kAttrTypeHardwareFogMode:
			case cgfxAttrDef::kAttrTypeHardwareFogStart:
			case cgfxAttrDef::kAttrTypeHardwareFogEnd:
			case cgfxAttrDef::kAttrTypeHardwareFogDensity:
			case cgfxAttrDef::kAttrTypeHardwareFogColor:
					break;
				default:
					break;
			}                          // switch (aDef->fType)
		}
		catch ( cgfxShaderCommon::InternalError* e )
		{
			if ( ++fErrorCount <= fErrorLimit )
			{
				size_t ee = (size_t)e;
				MFnDependencyNode fnNode( oNode );
				MString sMsg = "cgfxShader warning ";
				sMsg += (int)ee;
				sMsg += ": ";
				sMsg += fnNode.name();
				sMsg += " internal error while setting parameter \"";
				sMsg += aDef->fName;
				sMsg += "\" of effect \"";
				sMsg += fShaderFxFile;
				sMsg += "\" for shape ";
				if (shapePath.isValid())
					sMsg += shapePath.partialPathName();
				else
					sMsg += "SWATCH GEOMETRY";
				MGlobal::displayWarning( sMsg );
			}
		}
	}                                  // loop over fAttrDefList
}

/* virtual */
MStatus cgfxShaderNode::glUnbind(const MDagPath& shapePath)
{
	if (fCurrentTechnique && fCurrentTechnique->isValid())
	{
		// Shaders have an uncanny ability to corrupt depth state
		if( fDepthEnableState)
			glEnable( GL_DEPTH_TEST);
		else
			glDisable( GL_DEPTH_TEST);
		glDepthFunc( fDepthFunc);
		glBlendFunc( fBlendSourceFactor, fBlendDestFactor);

		if (fCurrentTechnique->hasBlending())
			glPopAttrib();
	}
	else
	{
		// Restore material attributes
		glPopAttrib();
	}

	glPopClientAttrib();
	glPopAttrib();

	glStateCache::instance().disableAll();
	glStateCache::instance().activeTexture( 0);


#ifdef KH_DEBUG
	MString ss = "  .. unbd ";
	if ( this && fConstructed )
		ss += name();
	ss += "\n";
	::OutputDebugString( ss.asChar() );
#endif
	return MS::kSuccess;
}


/* virtual */
MStatus cgfxShaderNode::glGeometry(const MDagPath& shapePath,
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
	MStatus stat;

#ifdef KH_DEBUG
	MString ss = "  .. geom ";
	if ( this && fConstructed )
		ss += name();
	ss += " ";
	ss += indexCount;
	ss += "i ";
	ss += vertexCount;
	ss += "v ";
	ss += normalCount;
	ss += "n ";
	ss += colorCount;
	ss += "c ";
	ss += texCoordCount;
	ss += "t ";
	ss += "\n";
	::OutputDebugString( ss.asChar() );
#endif

	try
	{
		if (fCurrentTechnique && fCurrentTechnique->isValid())
		{
			//register cg default state callbacks
			cgfxPassStateSetter::registerCgStateCallBacks(
                cgfxPassStateSetter::kDefaultViewport);

			// Set up the uniform attribute values for the effect.
			bindViewAttrValues(shapePath);

			// If our input shape is dirty, clear any cached data
			if( dirtyMask() != kDirtyNone)
				fBoundDataCache.flush(shapePath);

			// Now render the passes for this effect
			const cgfxPass* pass = fCurrentTechnique->getFirstPass();
			while( pass)
			{
				pass->bind(shapePath, &fBoundDataCache,
                           vertexCount, vertexArray,
                           fNormalsPerVertex, normalCount, normalArrays,
                           colorCount, colorArrays,
                           texCoordCount, texCoordArrays);
				glStateCache::instance().flushState();
				pass->setCgState();
				glDrawElements(prim, indexCount, GL_UNSIGNED_INT, indexArray);
				pass->resetCgState();
				pass = pass->getNext();
			}
		}
		else // fEffect must be NULL
		{
			// Now call glDrawElements to put all the primitives on the
			// screen.  See the comment above re: glDrawRangeElements.
			//
			glStateCache::instance().enablePosition();
			glVertexPointer(3, GL_FLOAT, 0, vertexArray);

			if ( normalCount > 0 && normalArrays[ 0 ] )
			{
				glStateCache::instance().enableNormal();
				glNormalPointer(GL_FLOAT, 0, normalArrays[0]);
			}
			else
			{
				glStateCache::instance().disableNormal();
				glNormal3f(0.0, 0.0, 1.0);
			}
			glStateCache::instance().flushState();
			glDrawElements(prim, indexCount, GL_UNSIGNED_INT, indexArray);
		}

		checkGlErrors("After effects End");
	}
	catch ( cgfxShaderCommon::InternalError* e )
	{
		reportInternalError( __FILE__, (size_t)e );
		stat = MS::kFailure;
	}
	catch ( ... )
	{
		reportInternalError( __FILE__, __LINE__ );
		stat = MS::kFailure;
	}

	return stat;
}                                      // cgfxShaderNode::geometry


/* virtual */
int
cgfxShaderNode::getTexCoordSetNames( MStringArray& names )
{
	names = fUVSets;
	return names.length();
}                                      // cgfxShaderNode::getTexCoordSetNames


#if MAYA_API_VERSION >= 700

/* virtual */
int
cgfxShaderNode::getColorSetNames( MStringArray& names )
{
	names = fColorSets;
	return names.length();
}

#else

/* virtual */
int cgfxShaderNode::colorsPerVertex()
{
	fColorType.setLength(1);
	fColorIndex.setLength(1);
	fColorType[0] = 0;
	fColorIndex[0] = 0;
	return 1;
} // cgfxShaderNode::texCoordsPerVertex

#endif

/* virtual */
int cgfxShaderNode::normalsPerVertex()
{
#ifdef KH_DEBUG
	MString ss = "  .. npv  ";
	if ( this && fConstructed )
		ss += name();
	ss += " ";
	ss += fNormalsPerVertex;
	ss += "\n";
	::OutputDebugString( ss.asChar() );
#endif

	// Now, when using MPxHwShaderNode, this is the first call Maya makes when
	// trying to render a plugin shader. So, in the cases where we were unable
	// to create our effect, try and do it here
	if (fEffect.isNull() || !fEffect->isValid()) {
#ifdef _WIN32
		::OutputDebugString( "CGFX: fEffect was NULL\n");
#endif

		// When batch off-screen rendering through "mayabatch -command hwRender ...",
		// the effect will be uninitialized because there was no active OpenGL
		// context at the time "cgfxShader -e -fx ..." was executed. This setup
		// is delayed until now when hardware renderer guarantees a valid context
		// and requests the plug-in to bind its resources to it. -cdt
		//
		createEffect();
	}

	return fNormalsPerVertex;

	// NB: Maya calls normalsPerVertex() both before and after bind().
	// It appears that the normalCount passed to geometry() is
	// obtained *before* the call to bind().  Therefore we set
	// fNormalsPerVertex as early as possible.  kh 9/03
}                                      // cgfxShaderNode::normalsPerVertex

MStatus
cgfxShaderNode::getAvailableImages( const MString& uvSetName,
							        MStringArray& imageNames)
{
	// Find all vertex attributes assigned to this uvSetName
	// and record the variable name.
	//
	MStringArray varNames;
	cgfxRCPtr<cgfxVertexAttribute> attr = fVertexAttributes;
	while( attr.isNull() == false )
	{
		MString source = attr->fSourceName;
		MStringArray sourceArray;
		source.split( ':', sourceArray );
		if( sourceArray.length() == 2				&&
			sourceArray[0].toLowerCase() == "uv"	&&
			sourceArray[1] == uvSetName				)
		{
			varNames.append( attr->fName );
		}
		attr = attr->fNext;
	}

	// For each input assigned to this UV set, determine
	// associated textures from the UVLink annotation.
	//
	const cgfxRCPtr<cgfxAttrDefList>& nodeList = attrDefList();
	if( nodeList.isNull() )
	{
		// Can occur when shader has not been rendered yet, but
		// the object is selected with the UV texture editor open.
		//
		return MS::kNotImplemented;
	}
	unsigned int nVars = varNames.length();
	for( unsigned int i = 0; i < nVars; i++ )
	{
		cgfxAttrDefList::iterator nmIt;
		for (nmIt = nodeList->begin(); nmIt; ++nmIt)
		{
			cgfxAttrDef* adef = (*nmIt);
			if( adef->fType == cgfxAttrDef::kAttrTypeColor2DTexture &&
				adef->fTextureUVLink == varNames[i]	)
			{
				imageNames.append( adef->fName );
			}
		}
	}

	// If no UVLinks found for this UV set, display all 2D textures.
	//
	if( imageNames.length() == 0 )
	{
		cgfxAttrDefList::iterator nmIt;
		for (nmIt = nodeList->begin(); nmIt; ++nmIt)
		{
			cgfxAttrDef* adef = (*nmIt);
			if( adef->fType == cgfxAttrDef::kAttrTypeColor2DTexture	)
			{
				imageNames.append( adef->fName );
			}
		}
	}

	return (imageNames.length() > 0) ? MStatus::kSuccess : MStatus::kNotImplemented;
}

// Render selected texture for UV editor in legacy mode (openGL)
MStatus
cgfxShaderNode::renderImage( const MString& imageName,
							floatRegion region,
							const MPxHwShaderNode::RenderParameters& /*parameters*/,
							int& imageWidth,
							int& imageHeight)
{
	// Locate the shader
	const cgfxRCPtr<cgfxAttrDefList>& nodeList = attrDefList();
	cgfxAttrDef* texDef = NULL;
	cgfxAttrDefList::iterator nmIt;
	for (nmIt = nodeList->begin(); nmIt; ++nmIt)
	{
		cgfxAttrDef* adef = (*nmIt);
		if( adef->fType >= cgfxAttrDef::kAttrTypeFirstTexture	&&
			adef->fType <= cgfxAttrDef::kAttrTypeLastTexture	&&
			adef->fName == imageName )
		{
			texDef = adef;
			break;
		}
	}

	if( !texDef )
	{
		return MStatus::kNotImplemented;
	}

	// Only supports 2D textures.
	//
	if( texDef->fType != cgfxAttrDef::kAttrTypeColor2DTexture )
	{
		return MStatus::kNotImplemented;
	}

	// Draw the texture
	//
	glPushAttrib( GL_ALL_ATTRIB_BITS );
    glPushClientAttrib( GL_CLIENT_VERTEX_ARRAY_BIT);

    // Do not use the texture cache as that depends on the shader rendering
	// first to initialize the cache.
	//
	MObject thisNode( thisMObject() );
	MPlug texPlug;
	texDef->getSource( thisNode, texPlug );
	MImageFileInfo::MHwTextureType hwType;
	if( MS::kSuccess != MHwTextureManager::glBind(texPlug, hwType) )
	{
		glPopAttrib();
		glPopClientAttrib();

		MFnDependencyNode fnNode( thisMObject() );
        MString sMsg = "cgfxShader ";
        sMsg += fnNode.name();
        sMsg += " : failed to load texture \"";
		sMsg += imageName;
		sMsg += "\".";
        MGlobal::displayWarning( sMsg );

		return MStatus::kNotImplemented;
	}

	GLint width = 0;
	GLint height = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBegin( GL_QUADS );
    glTexCoord2f(region[0][0], region[0][1]);
    glVertex2f(region[0][0], region[0][1]);
    glTexCoord2f(region[0][0], region[1][1]);
    glVertex2f(region[0][0], region[1][1]);
    glTexCoord2f(region[1][0], region[1][1]);
    glVertex2f(region[1][0], region[1][1]);
    glTexCoord2f(region[1][0], region[0][1]);
    glVertex2f(region[1][0], region[0][1]);
    glEnd();

    glPopAttrib();
    glPopClientAttrib();

	imageWidth = (int) width;
	imageHeight = (int) height;

	return MStatus::kSuccess;
}

// Render selected texture for UV editor in viewport 2.0 mode
MStatus
cgfxShaderNode::renderImage( MHWRender::MUIDrawManager& uiDrawManager,
							const MString& imageName,
							floatRegion region,
							const MPxHwShaderNode::RenderParameters& parameters,
							int& imageWidth,
							int& imageHeight)
{
	// Locate the shader
	const cgfxRCPtr<cgfxAttrDefList>& nodeList = attrDefList();
	cgfxAttrDef* texDef = NULL;
	cgfxAttrDefList::iterator nmIt;
	for (nmIt = nodeList->begin(); nmIt; ++nmIt)
	{
		cgfxAttrDef* adef = (*nmIt);
		if( adef->fType >= cgfxAttrDef::kAttrTypeFirstTexture	&&
			adef->fType <= cgfxAttrDef::kAttrTypeLastTexture	&&
			adef->fName == imageName )
		{
			texDef = adef;
			break;
		}
	}

	if( !texDef )
	{
		return MStatus::kNotImplemented;
	}

	// Only supports 2D textures.
	//
	if( texDef->fType != cgfxAttrDef::kAttrTypeColor2DTexture )
	{
		return MStatus::kNotImplemented;
	}

    // We could have used MTextureManager::acquireTexture that takes the plug in parameter,
    // but this is way too slow: the file data gets loaded every time before checking the cache.
    // Load using file name instead.
    // Retrieve texture file name from attribute def and linked plug
    MString textureFileName;
    {
        MObject thisNode( thisMObject() );
        MPlug texPlug;
        texDef->getSource( thisNode, texPlug );

        MFnDependencyNode dgFn( texPlug.node() );
        MStatus rc;
        MPlug filenamePlug = dgFn.findPlug( "fileTextureName", &rc);
        if(rc == MStatus::kSuccess) {
            filenamePlug.getValue(textureFileName);
        }
    }
    if(textureFileName.length() == 0)
		return MStatus::kFailure;

	MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
	if(theRenderer == NULL)
		return MStatus::kFailure;

	MHWRender::MTextureManager*	txtManager = theRenderer->getTextureManager();
	if(txtManager == NULL)
		return MStatus::kFailure;

	int mipmapLevels = 1;
    MHWRender::MTexture* texture = txtManager->acquireTexture(textureFileName, mipmapLevels); 
	if(texture == NULL)
		return MStatus::kFailure;

	// Release texture used for previous uv editor render and store the new one.
	// This is helpful if the scene does not render the texture.
	// This prevent having to load the same texture again and again on each draw
    if(fUVEditorTexture) {
    	txtManager->releaseTexture(fUVEditorTexture);
    }
	fUVEditorTexture = texture;

    MHWRender::MTextureDescription desc;
    texture->textureDescription(desc);

	imageWidth = desc.fWidth;
	imageHeight = desc.fHeight;

	// Early return, this is just a call to get the size of the texture ("Use image ratio" is on)
	if(region[0][0] == 0 && region[0][1] == 0 && region[1][0] == 0 && region[1][1] == 0)
		return MStatus::kSuccess;

    // Render texture on quad
    MPointArray positions;
    MPointArray& texcoords = positions;

    // Tri #0
    positions.append(region[0][0], region[0][1]);
    positions.append(region[1][0], region[0][1]);
    positions.append(region[1][0], region[1][1]);

    // Tri #1
    positions.append(region[0][0], region[0][1]);
    positions.append(region[1][0], region[1][1]);
    positions.append(region[0][0], region[1][1]);

    uiDrawManager.setColor( parameters.baseColor );
    uiDrawManager.setTexture( texture );
    uiDrawManager.setTextureSampler( parameters.unfiltered ? MHWRender::MSamplerState::kMinMagMipLinear : MHWRender::MSamplerState::kMinMagMipPoint, MHWRender::MSamplerState::kTexWrap );
    uiDrawManager.setTextureMask( parameters.showAlphaMask ? MHWRender::MBlendState::kAlphaChannel : MHWRender::MBlendState::kRGBAChannels );
    uiDrawManager.mesh( MHWRender::MUIDrawManager::kTriangles, positions, NULL, NULL, NULL, &texcoords );
    uiDrawManager.setTexture( NULL );

	return MStatus::kSuccess;
}


void
cgfxShaderNode::setAttrDefList( const cgfxRCPtr<cgfxAttrDefList>& list )
{
	if (!fAttrDefList.isNull()) {
		cgfxAttrDef::purgeMObjectCache( fAttrDefList );
    }

	if (!list.isNull()) {
		cgfxAttrDef::validateMObjectCache( thisMObject(), list );
	}

	fAttrDefList = list;
}                                      // cgfxShaderNode::setAttrDefList


void cgfxShaderNode::getAttributeList(MStringArray& attrList) const
{
	MString tmp;
	int len = fAttributeListArray.length();

	attrList.clear();

	for (int i = 0; i < len; ++i)
	{
		tmp = fAttributeListArray[i];
		attrList.append(tmp);
	}
}

void cgfxShaderNode::setAttributeList(const MStringArray& attrList)
{
	MString tmp;
	int len = attrList.length();

	fAttributeListArray.clear();

	for (int i = 0; i < len; ++i)
	{
		tmp = attrList[i];
		fAttributeListArray.append(tmp);
	}
}


//
// Set the current per-vertex attributes the shader needs (replacing any existing set)
//
void
cgfxShaderNode::setVertexAttributes( cgfxRCPtr<cgfxVertexAttribute> attributeList)
{
	// Backward compatibility: if we have values set in the old texCoordSources
	// or colorSources, find any varying attributes that use that register
	// and inherit the maya source
	if( fTexCoordSource.length())
	{
		int length = fTexCoordSource.length();
		for( int i = 0; i < length; i++)
		{
			MString semantic( "TEXCOORD");
			if( i)
				semantic += i;
			else
				semantic += "0";
			MString source( fTexCoordSource[ i]);
			if( source.index( ':') < 0)
				source = "uv:" + source;
			cgfxRCPtr<cgfxVertexAttribute> newAttribute = attributeList;
			while( newAttribute.isNull() == false)
			{
				if( newAttribute->fSemantic == semantic ||
					(i == 6 && (newAttribute->fSemantic == "TANGENT" || newAttribute->fSemantic == "TANGENT0")) ||
					(i == 7 && (newAttribute->fSemantic == "BINORMAL" || newAttribute->fSemantic == "BINORMAL0")))
					newAttribute->fSourceName = source;
				newAttribute = newAttribute->fNext;
			}
		}
		fTexCoordSource.clear();
	}
	if( fColorSource.length())
	{
		int length = fColorSource.length();
		for( int i = 0; i < length; i++)
		{
			MString semantic( "COLOR");
			if( i)
				semantic += i;
			else
				semantic += "0";
			MString source( fColorSource[ i]);
			if( source.index( ':') < 0)
				source = "color:" + source;
			cgfxRCPtr<cgfxVertexAttribute> newAttribute = attributeList;
			while( newAttribute.isNull() == false)
			{
				if( newAttribute->fSemantic == semantic)
					newAttribute->fSourceName = source;
				newAttribute = newAttribute->fNext;
			}
		}
		fColorSource.clear();
	}

	// Copy sourceName data to new list if fx file remains the same
	// Does best-effort matching, changing techniques may result in
	// changing streams.
	if (fLastShaderFxFileAtVASSet == fShaderFxFile)
	{
		cgfxRCPtr<cgfxVertexAttribute> oldAttribute = fVertexAttributes;
		while (oldAttribute.isNull() == false)
		{
			cgfxRCPtr<cgfxVertexAttribute> newAttribute = attributeList;
			while (newAttribute.isNull() == false)
			{
				if (newAttribute->fSourceName.length() == 0 &&
					newAttribute->fName == oldAttribute->fName &&
					newAttribute->fSemantic == oldAttribute->fSemantic &&
					newAttribute->fType == oldAttribute->fType)
				{
					newAttribute->fSourceName = oldAttribute->fSourceName;
					break;
				}
				newAttribute = newAttribute->fNext;
			}
			oldAttribute = oldAttribute->fNext;
		}
	}

	// Now set our new attribute list
	fVertexAttributes = attributeList;

	// And determine the minimum set of data we need to request from Maya to
	// populate these values
	analyseVertexAttributes();
}


//
// Set the data set names that will be populating our vertex attributes
//
void
cgfxShaderNode::setVertexAttributeSource( const MStringArray& sources)
{
	// Flush any cached data stream - the inputs have changed
	fBoundDataCache.flush();

	// Set the attributes sources as specified
	int i = 0;
	int numSources = sources.length();
	cgfxRCPtr<cgfxVertexAttribute> attribute = fVertexAttributes;
	while( attribute.isNull() == false)
	{
		attribute->fSourceName = ( i <	numSources) ? sources[ i++] : "";
		attribute = attribute->fNext;
	}

	// Cache shader fx file name used when setting attribute source
	fLastShaderFxFileAtVASSet = fShaderFxFile;

	// And determine the minimum set of data we need to request from Maya to
	// populate these values
	analyseVertexAttributes();
}

inline int findOrInsert( const MString& value, MStringArray& list)
{
	int length = list.length();
	for( int i = 0; i < length; i++)
		if( list[ i] == value)
			return i;
	list.append( value);
	return length;
}

//
// Analyse the per-vertex attributes to work out the minimum set of data we require
//
void
cgfxShaderNode::analyseVertexAttributes()
{
    ++fGeomReqDataVersionId;

	fUVSets.clear();
	fColorSets.clear();
	fNormalsPerVertex = 0;

	cgfxRCPtr<cgfxVertexAttribute> attribute = fVertexAttributes;
	while( attribute.isNull() == false)
	{
		// Work out where this attribute should come from
		MString source( attribute->fSourceName);
		source.toLowerCase();
		if( attribute->fSourceName.length() == 0)
		{
			attribute->fSourceType = cgfxVertexAttribute::kNone;
			// revert the source to default position source stream if it is empty position stream.
			if(attribute->fSemantic == "POSITION")
			{
				const MString warnMsg = "position can't be empty! Will use default position data!";
				MGlobal::displayWarning(warnMsg);
				attribute->fSourceName = "position";
				attribute->fSourceType = cgfxVertexAttribute::kPosition;
			}
		}
		else if( source == "position")
		{
			attribute->fSourceType = cgfxVertexAttribute::kPosition;
		}
		else if( source == "normal")
		{
			attribute->fSourceType = cgfxVertexAttribute::kNormal;
			if( fNormalsPerVertex < 1)
				fNormalsPerVertex = 1;
		}
		else
		{
			// Try and pull off the type
			MString set = attribute->fSourceName;
			int colon = set.index( ':');
			MString type;
			if( colon >= 0)
			{
				if( colon > 0) type = source.substring( 0, colon - 1);
				set = set.substring( colon + 1, set.length() - 1);
			}

			// Now, work out what kind of set we have here
			if( type == "uv")
			{
				attribute->fSourceType = cgfxVertexAttribute::kUV;
				attribute->fSourceIndex = findOrInsert( set, fUVSets);
			}
			else if( type == "tangent")
			{
				attribute->fSourceType = cgfxVertexAttribute::kTangent;
				if( fNormalsPerVertex < 2)
					fNormalsPerVertex = 2;
				attribute->fSourceIndex = findOrInsert( set, fUVSets);
			}
			else if( type == "binormal")
			{
				attribute->fSourceType = cgfxVertexAttribute::kBinormal;
				if( fNormalsPerVertex < 3)
					fNormalsPerVertex = 3;
				attribute->fSourceIndex = findOrInsert( set, fUVSets);
			}
			else if( type == "color")
			{
				attribute->fSourceType = cgfxVertexAttribute::kColor;
				attribute->fSourceIndex = findOrInsert( set, fColorSets);
			}
			else
			{
				attribute->fSourceType = cgfxVertexAttribute::kBlindData;
			}
		}

		attribute = attribute->fNext;
	}

	//for( unsigned int i = 0; i < fUVSets.length(); i++) printf( "Requesting UVset[%d] = %s\n", i, fUVSets[i]);
}


// Data accessors for the texCoordSource attribute.
const MStringArray&
cgfxShaderNode::getTexCoordSource() const
{
#ifdef KH_DEBUG
	MString ss = "  .. gtcs ";
	if ( this && fConstructed )
		ss += name();
	ss += " ";
	for ( int ii = 0; ii < fTexCoordSource.length(); ++ii )
	{
		ss += "\"";
		ss += fTexCoordSource[ii];
		ss += "\" ";
	}
	ss += "\n";
	::OutputDebugString( ss.asChar() );
#endif
	return fTexCoordSource;
}                                      // cgfxShaderNode::getTexCoordSource


// Data accessors for the colorSource attribute.
const MStringArray&
cgfxShaderNode::getColorSource() const
{
#ifdef KH_DEBUG
	MString ss = "  .. gtcs ";
	if ( this && fConstructed )
		ss += name();
	ss += " ";
	for ( int ii = 0; ii < fColorSource.length(); ++ii )
	{
		ss += "\"";
		ss += fColorSource[ii];
		ss += "\" ";
	}
	ss += "\n";
	::OutputDebugString( ss.asChar() );
#endif
	return fColorSource;
}                                      // cgfxShaderNode::getColorSource


void
cgfxShaderNode::setDataSources( const MStringArray* texCoordSources,
							    const MStringArray* colorSources)
{
	if( texCoordSources )
	{
		int length_TC = texCoordSources->length();
		if ( length_TC > CGFXSHADERNODE_GL_TEXTURE_MAX )
			length_TC = CGFXSHADERNODE_GL_TEXTURE_MAX;
		fTexCoordSource.clear();
		for ( int i = 0; i < length_TC; ++i )
		{
			fTexCoordSource.append( (*texCoordSources)[ i ] );
		}
		// This method is unstable and may causes crashes in the API
		// Don't use for now.
		//fTexCoordSource.setLength( length_TC );
		//for ( int i = 0; i < length_TC; ++i )
		//	fTexCoordSource[ i ] = texCoordSources[ i ];
	}

	if( colorSources )
	{
		int length_CS = colorSources->length();
		if ( length_CS > CGFXSHADERNODE_GL_COLOR_MAX )
			length_CS = CGFXSHADERNODE_GL_COLOR_MAX;
		fColorSource.setLength( length_CS );
		for ( int i = 0; i < length_CS; ++i )
			fColorSource[ i ] = (*colorSources)[ i ];
	}

	fDataSetNames.clear();
	fNormalsPerVertex = 1;
	updateDataSource( fTexCoordSource, fTexCoordType, fTexCoordIndex);
	updateDataSource( fColorSource, fColorType, fColorIndex);
}


void
cgfxShaderNode::updateDataSource( MStringArray& v, MIntArray& typeList, MIntArray& indexList)
{
#ifdef KH_DEBUG
	MString ss = "  .. stcs ";
	if ( this && fConstructed )
		ss += name();
	ss += " ";
	for ( int ii = 0; ii < v.length(); ++ii )
	{
		ss += "\"";
		ss += v[ii];
		ss += "\" ";
	}
	ss += "\n";
	::OutputDebugString( ss.asChar() );
#endif

	int nDataSets = v.length();
	typeList.setLength( nDataSets );
	indexList.setLength( nDataSets );
	for ( int iDataSet = 0; iDataSet < nDataSets; ++iDataSet )
	{                                  // iDataSet loop
		MString s;
		int iType = etcNull;
		int iBuf = 0;

		// Strip leading and trailing spaces and control chars.
		const char* bp = v[ iDataSet ].asChar();
		const char* ep = v[ iDataSet ].length() + bp;
#ifdef _WIN32
		while ( bp < ep && *bp <= ' ' && *bp >= '\0') ++bp;
#else
		while ( bp < ep && *bp <= ' ') ++bp;
#endif

#ifdef _WIN32
		while ( bp < ep && ep[-1] <= ' ' && ep[-1] >= '\0' ) --ep;
#else
		while ( bp < ep && ep[-1] <= ' ' ) --ep;
#endif

		// Empty?
		if ( bp == ep )
			iType = etcNull;

		// Constant?  (1, 2, 3 or 4 float values)
		else if ( (*bp >= '0' && *bp <= '9') ||
			*bp == '-' ||
			*bp == '+' ||
			*bp == '.' )
		{
			const char* cp = bp;
			int nValues = 0;
			while ( cp < ep &&
				nValues < 4 )
			{
				float x;
				int   nc = 0;
				int   nv = sscanf( cp, " %f%n", &x, &nc );
				if ( nv != 1 )
					break;
				++nValues;
				cp += nc;
			}
			if ( nValues > 0 )
			{
				s.set( bp, (int)(cp - bp) );      // drop trailing junk
				for ( ; nValues < 4; ++nValues )
					s += " 0";
				iType = etcConstant;
			}
		}

		// UV set name or reserved word.
		else
		{
			s.set( bp, (int)(ep - bp) );

			// Pull out any qualifiers (e.g. tangent:uvSet1) and register
			// the data set they require
			//
			MStringArray splitStrings;
			#define kDefaultUVSet "map1"
			if ((MStatus::kSuccess == s.split( ':', splitStrings)) && splitStrings.length() > 1)
			{
				s = splitStrings[0];
				iBuf = findOrAppend( fDataSetNames, splitStrings[1]);
			}

			// Force reserved words to lower case.
			bp = s.asChar();
			if ( 0 == stricmp( "normal", bp ) )
			{
				s = "normal";
				iType = etcNormal;
			}
			else if ( 0 == stricmp( "tangent", bp ) )
			{
				s = "tangent";
				if( splitStrings.length() < 2)
				{
					splitStrings.setLength( 2);
					splitStrings[ 1] = kDefaultUVSet;
					iBuf = findOrAppend( fDataSetNames, kDefaultUVSet);
				}
				s += ":" + splitStrings[1];
				iType = etcTangent;
				if( fNormalsPerVertex < 2)
					fNormalsPerVertex = 2;
			}
			else if ( 0 == stricmp( "binormal", bp ) )
			{
				s = "binormal";
				if( splitStrings.length() < 2)
				{
					splitStrings.setLength( 2);
					splitStrings[ 1] = kDefaultUVSet;
					iBuf = findOrAppend( fDataSetNames, kDefaultUVSet);
				}
				s += ":" + splitStrings[1];
				iType = etcBinormal;
				fNormalsPerVertex = 3;
			}

			// Data set name... tell Maya that we want to retrieve this data set.
			else
			{
				iType = etcDataSet;
				iBuf = findOrAppend( fDataSetNames, s );
			}
		}

		// Tell our geometry() method where to get data.
		typeList[ iDataSet ] = iType;
		indexList[ iDataSet ] = iBuf;

		// Store cleaned-up string.
		v[ iDataSet ] = s;
	}                                  // iDataSet loop
}                                      // cgfxShaderNode::updateDataSource


// Data accessor for list of empty UV sets.
const MStringArray&
cgfxShaderNode::getEmptyUVSets() const
{
	static const MStringArray saNull;
	return saNull;
}                                      // cgfxShaderNode::getEmptyUVSets

const MObjectArray&
cgfxShaderNode::getEmptyUVSetShapes() const
{
	static const MObjectArray oaNull;
	return oaNull;
}                                      // cgfxShaderNode::getEmptyUVSetShapes


void
cgfxShaderNode::setEffect(const cgfxRCPtr<cgfxEffect const>& pNewEffect)
{
	// Remove old effect - node association
	cgfxShaderNode::removeAssociation(this, fEffect);

	fEffect = pNewEffect;

	// Add new effect - node association
	cgfxShaderNode::addAssociation(this, fEffect);

    updateTechniqueList();
	setTechnique( getTechnique() );
}

void
cgfxShaderNode::updateTechniqueList()
{
	// Build string array containing technique names and descriptions.
	//     Each item in the technique list has the form
	//         "techniqueName<TAB>numPasses"
	//     where
	//         numPasses is the number of passes defined by the
	//             technique, or 0 if the technique is not valid.
	//     (Future versions of the cgfxShader plug-in may append
	//      additional tab-separated fields.)
	fTechniqueList.clear();
	if (!fEffect.isNull() && fEffect->isValid())
	{
        const cgfxTechnique* technique = fEffect->getFirstTechnique();
		while (technique)
		{
			MString s;
            s += technique->getName();

			if (technique->isValid())
			{
				s += "\t";
				s += technique->getNumPasses();
			}
			else
			{
				s += "\t0";
			}

			fTechniqueList.append(s);
			technique = technique->getNext();
		}
	}
}                                      // cgfxShaderNode::setEffect

/* virtual */ bool cgfxShaderNode::hasTransparency()
{
	// Always return false, so that transparencyOptions() will be
	// called to give finer grain control.
	return false;
}


/* virtual */
unsigned int cgfxShaderNode::transparencyOptions()
{
	if (fCurrentTechnique && fCurrentTechnique->isValid() && fCurrentTechnique->hasBlending())
	{
		// Set as transparent, but we don't want any internal transparency algorithms
		// being used.
		return ( kIsTransparent | kNoTransparencyFrontBackCull | kNoTransparencyPolygonSort );
	}
	return 0;
}

void
cgfxShaderNode::setTechnique( const MString& techn )
{
	// If effect not loaded, just store the technique name.
	if (fEffect.isNull() || !fEffect->isValid())
	{
		fTechnique = techn;
		return;
	}

	// Search for requested technique.
    if (techn.length() != 0) {
        const cgfxTechnique* technique = fEffect->getTechnique(techn);
        if (technique) {
            if (technique->isValid())
            {
                fTechnique = techn;
                fCurrentTechnique = technique;

                // Setup the vertex parameters for this technique
                setVertexAttributes(fCurrentTechnique->getVertexAttributes());

                // Flush any cached data streams has when the
                // technique changes.
                fBoundDataCache.flush();

                ++fGeomReqDataVersionId;
                return;
            }
            else {
                MString s;
                s += typeName();
                s += " \"";
                s += name();
                s += "\" : unable to validate technique \"";
                s += techn.asChar();
                s += "\"";
                MGlobal::displayError(s);
                MGlobal::displayError(technique->getCompilationErrors());
            }
        }
        else if (!shaderFxFileChanged()) {
            MString s;
            s += typeName();
            s += " \"";
            s += name();
            s += "\" : unable to find technique \"";
            s += techn.asChar();
            s += "\"";
            MGlobal::displayError(s);
        }
    }

	// Requested technique was not found or not valid.  Revert to the old one.
    if (fTechnique.length() != 0 && fTechnique != techn) {
        const cgfxTechnique* technique = fEffect->getTechnique(techn);
        if (technique) {
            if (technique->isValid())
            {
                fCurrentTechnique = technique;

                // Setup the vertex parameters for this technique
                setVertexAttributes(fCurrentTechnique->getVertexAttributes());

                // Flush any cached data streams has when the
                // technique changes.
                fBoundDataCache.flush();

                return;
            }
            else {
                MString s;
                s += typeName();
                s += " \"";
                s += name();
                s += "\" : unable to validate technique \"";
                s += fTechnique.asChar();
                s += "\"";
                MGlobal::displayError(s);
                MGlobal::displayError(technique->getCompilationErrors());
            }
        }
        else if (!shaderFxFileChanged()) {
            MString s;
            s += typeName();
            s += " \"";
            s += name();
            s += "\" : unable to find technique \"";
            s += fTechnique.asChar();
            s += "\"";
            MGlobal::displayError(s);
        }
    }

	// Old technique is no good.  Activate the first valid technique.
    const cgfxTechnique* technique = fEffect->getFirstTechnique();
	while (technique)
	{
        if (technique->isValid())
		{
			fTechnique = technique->getName();
            fCurrentTechnique = technique;

            // Setup the vertex parameters for this technique
            setVertexAttributes(fCurrentTechnique->getVertexAttributes());

            // Flush any cached data streams has when the
            // technique changes.
            fBoundDataCache.flush();

			// Setup the vertex parameters for this technique
            ++fGeomReqDataVersionId;
			return;
		}
		technique = technique->getNext();
    }

	// No valid technique exists for the current effect.
	//   Save requested technique name.  We'll try to use it as the
	//   initial technique the next time a valid effect is loaded.
	fTechnique = techn;

    MString s;
    s += typeName();
    s += " \"";
    s += name();
    s += "\" : unable to find a valid technique.";
    MGlobal::displayError(s);
}                                      // cgfxShaderNode::setTechnique


void
cgfxShaderNode::setProfile( const MString& profileName )
{
    const cgfxProfile* profile = cgfxProfile::getProfile(profileName);

    if (profile) {
        fProfileName = profileName;
        setProfile(profile);
    }
    else {
        fProfileName = "";
        setProfile(NULL);

        if (profileName.length() > 0) {
            MString sMsg = "cgfxShader : ";
            sMsg += "The profile \"";
            sMsg += profileName;
            sMsg += "\" is not a supported profile on your platform. Reverting to use the default profile.";
            MGlobal::displayWarning( sMsg );
        }
    }
}


void
cgfxShaderNode::setProfile( const cgfxProfile* profile )
{
	if (fEffect.isNull() || !fEffect->isValid())
		return;

    // Search for requested profile.
    fEffect->setProfile(profile);

    // The list of valid techniques depends on the selected profile.
    updateTechniqueList();

	// We must set the technique again to verify if the technique is
	// still valid under the new profile.
    setTechnique(fTechnique);
}


MStatus cgfxShaderNode::shouldSave ( const MPlug & plug, bool & ret )
{
	if (plug == sAttributeList)
	{
		ret = true;
		return MS::kSuccess;
	}
	else if (plug == sVertexAttributeList)
	{
		ret = true;
		return MS::kSuccess;
	}
	return MPxNode::shouldSave(plug, ret);
}


void cgfxShaderNode::setTexturesByName(bool texturesByName, bool updateAttributes)
{
	if( updateAttributes && fTexturesByName != texturesByName)
	{
		// We've been explicitly changed to a different
		// texture mode.

		// If we have any current texture attributes, destroy them
		//
		MDGModifier dgMod;
		const cgfxRCPtr<cgfxAttrDefList>& nodeList = attrDefList();
		cgfxAttrDefList::iterator nmIt;
		bool foundTextures = false;
		for (nmIt = nodeList->begin(); nmIt; ++nmIt)
		{
			cgfxAttrDef* adef = (*nmIt);
			if(adef->fType >= cgfxAttrDef::kAttrTypeFirstTexture && adef->fType <= cgfxAttrDef::kAttrTypeLastTexture)
			{
				MObject theMObject = thisMObject();
				adef->destroyAttribute( theMObject, &dgMod);
				foundTextures = true;
			}
		}

		// Switch across to the new texture mode (before creating the
		// new attributes)
		//
		fTexturesByName = texturesByName;

		// Now re-create our texture attributes
		//
		if( foundTextures)
		{
			dgMod.doIt();
			for (nmIt = nodeList->begin(); nmIt; ++nmIt)
			{
				cgfxAttrDef* adef = (*nmIt);
				if( adef->fType >= cgfxAttrDef::kAttrTypeFirstTexture && adef->fType <= cgfxAttrDef::kAttrTypeLastTexture)
				{
					adef->createAttribute(thisMObject(), &dgMod, this);
				}
			}
			dgMod.doIt();

			// Finally, if we just created new string attributes, we need to
			// set them to a sensible value or they won't show up
			//
			if( fTexturesByName)
			{
				for (nmIt = nodeList->begin(); nmIt; ++nmIt)
				{
					cgfxAttrDef* adef = (*nmIt);
					if( adef->fType >= cgfxAttrDef::kAttrTypeFirstTexture &&
						adef->fType <= cgfxAttrDef::kAttrTypeLastTexture)
					{
		                MObject theMObject = thisMObject();
						adef->setTexture( theMObject, adef->fStringDef, &dgMod);
					}
				}
			}
		}
	}
	else
	{
		fTexturesByName = texturesByName;
	}
}


// Get cgfxShader version string.
MString
cgfxShaderNode::getPluginVersion()
{
	MString sVer = "cgfxShader ";
	sVer += CGFXSHADER_VERSION;
	sVer += " for Maya ";
	sVer += (int)(MAYA_API_VERSION / 100);
	sVer += ".";
	sVer += (int)(MAYA_API_VERSION % 100 / 10);
	sVer += " (";
	sVer += __DATE__;
	sVer += ")";
	return sVer;
}                                      // cgfxShaderNode::getPluginVersion


// Error reporting
void
cgfxShaderNode::reportInternalError( const char* function, size_t errcode )
{
	MString es = "cgfxShader";

	try
	{
		if ( this &&
			fConstructed )
		{
			if ( ++fErrorCount > fErrorLimit )
				return;
			MString s;
			s += "\"";
			s += name();
			s += "\": ";
			s += typeName();
			es = s;
		}
	}
	catch ( ... )
	{}
	es += " internal error ";
	es += (int)errcode;
	es += " in ";
	es += function;
#ifdef KH_DEBUG
	::OutputDebugString( es.asChar() );
	::OutputDebugString( "\n" );
#endif
	MGlobal::displayError( es );
}                                      // cgfxShaderNode::reportInternalError

void
cgfxShaderNode::cgErrorCallBack()
{
	MGlobal::displayInfo(__FUNCTION__);
	CGerror cgLastError = cgGetError();
	if(cgLastError)
	{
		MGlobal::displayError(cgGetErrorString(cgLastError));
		MGlobal::displayError(cgGetLastListing(sCgContext));
	}
}																			 // cgfxShaderNode::cgErrorCallBack

void
cgfxShaderNode::cgErrorHandler(CGcontext cgContext, CGerror cgError, void* userData)
{
	MGlobal::displayError(cgGetErrorString(cgError));
	MGlobal::displayError(cgGetLastListing(sCgContext));
}


/*static*/
void cgfxShaderNode::getNodesUsingEffect(const cgfxRCPtr<const cgfxEffect>& effect, NodeList &nodes)
{
	Effect2NodesMap::const_iterator it = sEffect2NodesMap.find( effect.operator->() );
	if(it != sEffect2NodesMap.end())
	{
		const NodeList &nodeList = it->second;
		nodes.insert(nodeList.begin(), nodeList.end());
	}
}

/*static*/
void cgfxShaderNode::addAssociation(cgfxShaderNode* node, const cgfxRCPtr<const cgfxEffect>& effect)
{
	if(effect.isNull() == false)
	{
		NodeList &nodes = sEffect2NodesMap[ effect.operator->() ];
		nodes.insert(node);
	}
}

/*static*/
void cgfxShaderNode::removeAssociation(cgfxShaderNode* node, const cgfxRCPtr<const cgfxEffect>& effect)
{
	if(effect.isNull() == false)
	{
		Effect2NodesMap::iterator it = sEffect2NodesMap.find( effect.operator->() );
		if(it != sEffect2NodesMap.end())
		{
			NodeList &nodes = it->second;
			nodes.erase(node);
			if(nodes.empty())
				sEffect2NodesMap.erase(it);
		}
	}
}

/*static*/
void cgfxShaderNode::attributeAddedOrRemovedCB(
	MNodeMessage::AttributeMessage msg,
	MPlug& plug,
	void* clientData)
{
	// The CgFX shader node does not respond well to having its fx file
	// attribute altered via a reference edit. This is not a supported workflow
	// and should be avoided (change the fx file attribute in the original file
	// instead). Recent changes have tried to accomodate this workflow so that
	// the saved file does not get into a bad state. However, there are still
	// legacy files that have been saved in the bad state and this code is to
	// prevent crashes when loading them. It's a bit heavy-handed but is
	// limited to the case that crashes. If while opening a scene, an attribute
	// is remvoed from the node, we clear the effect data structure so that it
	// is forced to rebuild from scratch when it is next needed. This will
	// prevent the plugin from accidentally accessing attributes that have been
	// deleted.
	if (msg == MNodeMessage::kAttributeRemoved &&
		clientData &&
		MFileIO::isOpeningFile())
	{
		cgfxShaderNode* shaderNode = (cgfxShaderNode*)clientData;
		if (!shaderNode->effect().isNull())
		{
			// set shader file changed and effect NULL to force rebuild
			shaderNode->setShaderFxFileChanged(true);
			shaderNode->setEffect(cgfxRCPtr<const cgfxEffect>());
		}
	}
}

// ===================================================================================
// viewport 2.0 implementation
// ===================================================================================

const MString cgfxShaderOverride::drawDbClassification("drawdb/shader/surface/cgfxShader");
const MString cgfxShaderOverride::drawRegistrantId("cgfxShaderRegistrantId");

cgfxShaderNode* cgfxShaderOverride::sActiveShaderNode = NULL;
cgfxShaderNode* cgfxShaderOverride::sLastDrawShaderNode = NULL;

MHWRender::MPxShaderOverride* cgfxShaderOverride::Creator(const MObject& obj)
{
	return new cgfxShaderOverride(obj);
}

cgfxShaderOverride::cgfxShaderOverride(const MObject& obj)
	: MPxShaderOverride(obj)
    , fShaderNode(NULL)
    , fGeomReqDataVersionId(0)
    , fNeedPassSetterInit(false)
    , fOldBlendState(NULL)
    , fOldDepthStencilState(NULL)
    , fOldRasterizerState(NULL)
{
}

cgfxShaderOverride::~cgfxShaderOverride()
{
	fShaderNode = NULL;
}

// Initialize phase
MString cgfxShaderOverride::initialize(MObject shader)
{
	TRACE_API_CALLS("cgfxShaderOverride::initialize");

	// This is the routine where you would do all the expensive,
	// one-time kind of work.  Create vertex programs, load
	// textures, etc.
	//
	glStateCache::instance().reset();

	// One-time OpenGL initialization...
	if ( glStateCache::sMaxTextureUnits <= 0 )
	{
		// Before this point, we never had a good OpenGL context.  Now
		// we can check for extensions and set up pointers to the
		// extension procs.
#ifdef _WIN32
#define RESOLVE_GL_EXTENSION( fn, ext) wglGetProcAddress( #fn #ext)
#elif defined LINUX
#define RESOLVE_GL_EXTENSION( fn, ext) &fn ## ext
#else
#define RESOLVE_GL_EXTENSION( fn, ext) &fn
#endif

		glStateCache::glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREARBPROC) RESOLVE_GL_EXTENSION( glClientActiveTexture, ARB);
		glStateCache::glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERARBPROC) RESOLVE_GL_EXTENSION( glVertexAttribPointer, ARB);
		glStateCache::glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYARBPROC) RESOLVE_GL_EXTENSION( glEnableVertexAttribArray, ARB);
		glStateCache::glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC) RESOLVE_GL_EXTENSION( glDisableVertexAttribArray, ARB);
		glStateCache::glVertexAttrib4f = (PFNGLVERTEXATTRIB4FARBPROC) RESOLVE_GL_EXTENSION( glVertexAttrib4f, ARB);
		glStateCache::glSecondaryColorPointer = (PFNGLSECONDARYCOLORPOINTEREXTPROC) RESOLVE_GL_EXTENSION( glSecondaryColorPointer, EXT);
		glStateCache::glSecondaryColor3f = (PFNGLSECONDARYCOLOR3FEXTPROC) RESOLVE_GL_EXTENSION( glSecondaryColor3f, EXT);
		glStateCache::glMultiTexCoord4fARB = (PFNGLMULTITEXCOORD4FARBPROC) RESOLVE_GL_EXTENSION( glMultiTexCoord4f, ARB);

		// Don't use GL_MAX_TEXTURE_UNITS as this does not provide a proper
		// count when the # of image or texcoord inputs differs
		// from the conventional (older) notion of texture unit.
		//
		// Instead take the minimum of GL_MAX_TEXTURE_COORDS_ARB and
		// GL_MAX_TEXUTRE_IMAGE_UNITS_ARB according to the
		// ARB_FRAGMENT_PROGRAM specification.
		//
		GLint tval;
		glGetIntegerv( GL_MAX_TEXTURE_COORDS_ARB, &tval );
		GLint mic = 0;
		glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &mic );
		if (mic < tval)
			tval = mic;

		// Don't use this...
		//glGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &tval );
		glStateCache::sMaxTextureUnits = tval;
		if (!glStateCache::glClientActiveTexture || glStateCache::sMaxTextureUnits < 1)
			glStateCache::sMaxTextureUnits = 1;
		else if (glStateCache::sMaxTextureUnits > CGFXSHADERNODE_GL_TEXTURE_MAX)
			glStateCache::sMaxTextureUnits = CGFXSHADERNODE_GL_TEXTURE_MAX;
	}

	// Get the effect parameters updated
	//
	if (shader != MObject::kNullObj)
	{
		// Get the hardware shader node from the MObject.
		fShaderNode = (cgfxShaderNode *) MPxHwShaderNode::getHwShaderNodePtr( shader );
	}
	else
		fShaderNode = NULL;

	bool useCustomPrimitiveGenerator = false;

	if (fShaderNode)
	{
	    static bool enableCustomPrimitiveGenerator = (getenv("MAYA_USE_CUSTOMPRIMITIVEGENERATOR") != NULL);

		fShaderNode->createEffect();
        const cgfxTechnique* technique = fShaderNode->fCurrentTechnique;
        if (technique && technique->isValid())
		{
			// Add in geometry requirements based on the attributes
			// being asked for.
            //
            // Note that we can ask for streams on initialize since we
			// have set rebuildAlways() to return true when any of the
			// attributes affecting geometry requirements have
			// changed.

			MString sourceName;
			cgfxRCPtr<cgfxVertexAttribute> pVertexAttribute = fShaderNode->fVertexAttributes;
			while(pVertexAttribute.isNull() == false)
			{
				// Convert UI name into a real geometry name
				//
				sourceName = pVertexAttribute->fSourceName;
				if ( sourceName  == "position")
				{
					// Positions have no name
					sourceName = "";
				}
				else if( sourceName  == "normal")
				{
					// Normals have no name
					sourceName = "";
				}
				else
				{
					// Try and pull off the set name
					MString set = pVertexAttribute->fSourceName;
					int colon = set.index( ':');
					if( colon >= 0)
					{
						sourceName = set.substring( colon + 1, set.length() - 1);
						//printf("Parsed : out of [%s] to get [%s]\n", set.asChar(), sourceName.asChar() );
					}
				}


				MHWRender::MGeometry::DataType dataType = MHWRender::MGeometry::kInvalidType;
				int dimension = 1;

				unsigned int dimensionIndex = UINT_MAX;
				if(pVertexAttribute->fType.indexW( MString("float") ) == 0)
				{
					dataType = MHWRender::MGeometry::kFloat;
					dimensionIndex = 5;
				}
				else if(pVertexAttribute->fType.indexW( MString("half") ) == 0)
				{
					dataType = MHWRender::MGeometry::kInt16;
					dimensionIndex = 4;
				}
				else if(pVertexAttribute->fType.indexW( MString("int") ) == 0)
				{
					dataType = MHWRender::MGeometry::kInt32;
					dimensionIndex = 3;
				}

				if(dimensionIndex < pVertexAttribute->fType.length())
				{
					char dim = pVertexAttribute->fType.asChar()[dimensionIndex];
					dimension = dim - '0';
				}

				cgfxVertexAttribute::SourceType sourceType = pVertexAttribute->fSourceType;
				MString semanticName;

				if(enableCustomPrimitiveGenerator &&
					(pVertexAttribute->fSourceType == cgfxVertexAttribute::kBlindData || pVertexAttribute->fSourceType == cgfxVertexAttribute::kPosition) &&
					pVertexAttribute->fSemantic == "ATTR7") {
					useCustomPrimitiveGenerator = true;
					sourceType = cgfxVertexAttribute::kPosition;
					pVertexAttribute->fSourceName = "position";
					semanticName = "customPositionStream";
				}
				else if(enableCustomPrimitiveGenerator &&
					(pVertexAttribute->fSourceType == cgfxVertexAttribute::kBlindData || pVertexAttribute->fSourceType == cgfxVertexAttribute::kNormal) &&
					pVertexAttribute->fSemantic == "ATTR8") {
					useCustomPrimitiveGenerator = true;
					sourceType = cgfxVertexAttribute::kNormal;
					pVertexAttribute->fSourceName = "normal";
					semanticName = "customNormalStream";
				}
				else if(pVertexAttribute->fSourceType == cgfxVertexAttribute::kBlindData) {
					// we treat blind data as a named texture channel.
					// create the texture channel and set the semantic name.
					sourceType = cgfxVertexAttribute::kUV;
					semanticName = pVertexAttribute->fSourceName;
				}

                MStatus geomReqStatus = MS::kFailure;

				switch(sourceType)
				{
                    case cgfxVertexAttribute::kPosition:
                        {
                            //printf("Ask for position name = [%s]\n", sourceName.asChar());
							MHWRender::MVertexBufferDescriptor desc(
                                sourceName,
                                MHWRender::MGeometry::kPosition,
                                dataType,
                                dimension);
                            desc.setSemanticName(semanticName);

                            geomReqStatus = addGeometryRequirement(desc);
                        }
                        break;
                    case cgfxVertexAttribute::kNormal:
                        {
                            //printf("Ask for normals name = [%s]\n", sourceName.asChar());
							MHWRender::MVertexBufferDescriptor desc(
                                sourceName,
                                MHWRender::MGeometry::kNormal,
                                dataType,
                                dimension);
                            desc.setSemanticName(semanticName);

                            geomReqStatus = addGeometryRequirement(desc);
                        }
                        break;
                    case cgfxVertexAttribute::kUV:
                        {
                            //printf("Ask for uvset name = [%s]\n", sourceName.asChar());
                            if (semanticName.length() == 0)
                            {
								// if no semantic name, force UVs to be 2float
								// to work well with Maya
								dimension = 2;
								dataType = MHWRender::MGeometry::kFloat;
							}
							MHWRender::MVertexBufferDescriptor desc(
                                sourceName,
                                MHWRender::MGeometry::kTexture,
                                dataType,
                                dimension);
                            desc.setSemanticName(semanticName);

                            geomReqStatus = addGeometryRequirement(desc);
						}
                        break;
                    case cgfxVertexAttribute::kTangent:
                        {
                            //printf("Ask for tangent name = [%s]\n", sourceName.asChar());
							MHWRender::MVertexBufferDescriptor desc(
                                sourceName,
                                MHWRender::MGeometry::kTangent,
                                dataType,
                                dimension);
                            desc.setSemanticName(semanticName);

                            geomReqStatus = addGeometryRequirement(desc);
                        }
                        break;
                    case cgfxVertexAttribute::kBinormal:
                        {
                            //printf("Ask for bitangent name = [%s]\n", sourceName.asChar());
							MHWRender::MVertexBufferDescriptor desc(
                                sourceName,
                                MHWRender::MGeometry::kBitangent,
                                dataType,
                                dimension);
                            desc.setSemanticName(semanticName);

                            geomReqStatus = addGeometryRequirement(desc);
                        }
                        break;
                    case cgfxVertexAttribute::kColor:
                        {
                            //printf("Ask for color name = [%s]\n", sourceName.asChar());
							MHWRender::MVertexBufferDescriptor desc(
                                sourceName,
                                MHWRender::MGeometry::kColor,
                                dataType,
                                dimension);
                            desc.setSemanticName(semanticName);

                            geomReqStatus = addGeometryRequirement(desc);
                        }
                        break;
                    default:;
				}

                if (geomReqStatus != MS::kSuccess) {
                    MString s = "cgfxShader : Can't find the source named \"";
                    s += pVertexAttribute->fSourceName;;
                    s += "\" for vertex attribute \"";
                    s += pVertexAttribute->fName;;
                    s +="\".";
                    MGlobal::displayWarning(s);
                }

                pVertexAttribute = pVertexAttribute->fNext;
			}
		}
	}

    fGeomReqDataVersionId = fShaderNode->fGeomReqDataVersionId;
    fNeedPassSetterInit = true;

    // custom primitive types can be used by shader overrides.
    // This code is a simple example to show the mechanics of how that works.
    // Here we declare a custom custom indexing requirements.
    // The name "customPrimitiveTest" will be used to look up a registered
    // MPxPrimitiveGenerator that will handle the generation of the index buffer.
    // The example primitive generator is registered at startup by this plugin.
    if (useCustomPrimitiveGenerator)
    {
        MString customPrimitiveName("customPrimitiveTest");
        MHWRender::MIndexBufferDescriptor indexingRequirement
            (MHWRender::MIndexBufferDescriptor::kCustom, customPrimitiveName, MHWRender::MGeometry::kTriangles);
        addIndexingRequirement(indexingRequirement);
    }


    // FIXME: We probably want to include the timestamp and size of
    // the FX file at time that it was read to uniquely identify the
    // FX.
    //
    // Logged as bug #375613
    MString result =
        (MString("Autodesk Maya cgfxShaderOverride, shader file = ") +
         fShaderNode->shaderFxFile() +
         MString(" technique = ") +
         fShaderNode->getTechnique() +
         MString(" profile = ") +
         fShaderNode->getProfile());

    return result;
}

// Update phase
void cgfxShaderOverride::updateDG(MObject object)
{
	TRACE_API_CALLS("cgfxShaderOverride::updateDG");

	if (object != MObject::kNullObj)
	{
		// Get the hardware shader node from the MObject.
		fShaderNode = (cgfxShaderNode *) MPxHwShaderNode::getHwShaderNodePtr( object );
	}
	else
		fShaderNode = NULL;
}

void cgfxShaderOverride::updateDevice()
{
}

void cgfxShaderOverride::endUpdate()
{
}

// Draw phase
void cgfxShaderOverride::activateKey(MHWRender::MDrawContext& context)
{
	TRACE_API_CALLS("cgfxShaderOverride::activateKey");

	if (!fShaderNode)
	{
		//printf("Failed cgfxShaderOverride::activateKey() - no shader node\n");
		return;
	}

    // We use the Cg technique, pass and parameters from the shader
    // node at activation time. These Cg data structure can be used
    // until termination because all the fShaderNode's involved will
    // share the ame key.
    sActiveShaderNode = fShaderNode;
    sLastDrawShaderNode = NULL;

	const cgfxTechnique* technique = sActiveShaderNode->fCurrentTechnique;
	if(technique && technique->isValid())
	{
		// Register VP20 state callbacks for cg pass state
		cgfxPassStateSetter::registerCgStateCallBacks(
            cgfxPassStateSetter::kVP20Viewport);

        MHWRender::MStateManager* stateMgr = context.getStateManager();

        // Now initialize the passes for this effect
        {
            if (fNeedPassSetterInit) {
                delete [] sActiveShaderNode->fPassStateSetters;

                sActiveShaderNode->fPassStateSetters =
                    new cgfxPassStateSetter[technique->getNumPasses()];

                const cgfxPass* pass = technique->getFirstPass();
                for (int i=0; pass; ++i, pass = pass->getNext()) {
                    sActiveShaderNode->fPassStateSetters[i].init(
                        stateMgr, pass->getCgPass());
                }

                fNeedPassSetterInit = false;
            }
        }

		//save render state before rendering
        fOldBlendState = stateMgr->getBlendState();
        fOldDepthStencilState = stateMgr->getDepthStencilState();
        fOldRasterizerState = stateMgr->getRasterizerState();

		glPushClientAttrib ( GL_CLIENT_ALL_ATTRIB_BITS );

		glStateCache::instance().reset();   // the state cache should be reset before draw

        if (technique->getNumPasses() == 1) {
            // For single pass effects, we set the pass state at
            // activation time.
            sActiveShaderNode->fPassStateSetters[0].setPassState(stateMgr);
        }
	}
}

bool cgfxShaderOverride::draw(
    MHWRender::MDrawContext& context,
    const MHWRender::MRenderItemList& renderItemList) const
{
	TRACE_API_CALLS("cgfxShaderOverride::draw");

	if (!fShaderNode || !sActiveShaderNode)
	{
		//printf("Failed cgfxShaderOverride::draw() - no shader node\n");
		return false;
	}

	// Sample code to debug pass information
	static const bool debugPassInformation = false;
	if (debugPassInformation)
	{
		const MHWRender::MPassContext & passCtx = context.getPassContext();
		const MString & passId = passCtx.passIdentifier();
		const MStringArray & passSem = passCtx.passSemantics();
		printf("CgFx shader drawing in pass[%s], semantic[", passId.asChar());
		for (unsigned int i=0; i<passSem.length(); i++)
			printf(" %s", passSem[i].asChar());
		printf( "]\n");
	}

	static MGLFunctionTable *gGLFT = 0;
    if ( 0 == gGLFT )
        gGLFT = MHardwareRenderer::theRenderer()->glFunctionTable();

	bool result = true;

	const cgfxTechnique* technique = sActiveShaderNode->fCurrentTechnique;
	if (technique && technique->isValid())
	{
        MHWRender::MStateManager* stateMgr = context.getStateManager();

        bool needFullCgSetPassState = false;

        // Bind non-varying attributes if necessary.
        if (sLastDrawShaderNode != fShaderNode) {
            try
            {
                needFullCgSetPassState = bindAttrValues();
                checkGlErrors("cgfxShaderOverride::bindAttrValues");
            }
            catch ( cgfxShaderCommon::InternalError* e )
            {
                if (fShaderNode)
                    fShaderNode->reportInternalError( __FILE__, (size_t)e );
            }
            catch ( ... )
            {
                if (fShaderNode)
                    fShaderNode->reportInternalError( __FILE__, __LINE__ );
            }
        }

		// bind varying attributes
        bindViewAttrValues(context);

		const int numRenderItems = renderItemList.length();
		for (int renderItemIdx=0; renderItemIdx<numRenderItems; renderItemIdx++)
		{
			const MHWRender::MRenderItem* renderItem = renderItemList.itemAt(renderItemIdx);
			if (!renderItem) continue;
			const MHWRender::MGeometry* geometry = renderItem->geometry();
			if (!geometry) continue;

			bool boundData = true;
			const int bufferCount = geometry->vertexBufferCount();
			sourceStreamInfo *pBindSource = new sourceStreamInfo[bufferCount];
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

				switch(desc.semantic())
				{
					case MHWRender::MGeometry::kPosition:
						{
							pBindSource[i].fSourceType = cgfxVertexAttribute::kPosition;
							pBindSource[i].fSourceName = "position";
						}
						break;
					case MHWRender::MGeometry::kNormal:
						{
							pBindSource[i].fSourceType = cgfxVertexAttribute::kNormal;
							pBindSource[i].fSourceName = "normal";
						}
						break;
					case MHWRender::MGeometry::kTexture:
						{
                            if (desc.semanticName().length() == 0)
                            {
                                pBindSource[i].fSourceName = "uv:" + desc.name();
                                pBindSource[i].fSourceType = cgfxVertexAttribute::kUV;
                            }
                            else
                            {
                                // if the descriptor has a custom semantic name then use it as the source name
                                pBindSource[i].fSourceName = desc.semanticName();
                                pBindSource[i].fSourceType = cgfxVertexAttribute::kBlindData;
                            }
                            //printf("uv description name is [%s]\n", desc.name().asChar());
                            //printf("Build uv source name %s\n", pBindSource[i].fSourceName.asChar());
						}
						break;
					case MHWRender::MGeometry::kColor:
						{
							pBindSource[i].fSourceType =  cgfxVertexAttribute::kColor;
							pBindSource[i].fSourceName = "color:" + desc.name();
						}
						break;
					case MHWRender::MGeometry::kTangent:
						{
							pBindSource[i].fSourceType = cgfxVertexAttribute::kTangent;
							pBindSource[i].fSourceName = "tangent:" + desc.name();
						}
						break;
					case MHWRender::MGeometry::kBitangent:
						{
							pBindSource[i].fSourceType = cgfxVertexAttribute::kBinormal;
							pBindSource[i].fSourceName = "binormal:" + desc.name();
						}
						break;
					default:
						{
							pBindSource[i].fSourceType = cgfxVertexAttribute::kBlindData;
							pBindSource[i].fSourceName = desc.semanticName();
						}
						break;
				}

				pBindSource[i].fDimension = desc.dimension();
				pBindSource[i].fOffset = desc.offset();
				pBindSource[i].fStride = desc.stride();
				pBindSource[i].fElementSize = desc.dataTypeSize();
				pBindSource[i].fDataBufferId = *dataBufferId;
			}

            //draw
            // Dump out indexing information
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
                    /*	if (debugGeometricDrawPrintData)
                        {
                        fprintf(stderr, "IndexingPrimType(%s), IndexType(%s), IndexCount(%d), Handle(%d)\n",
                        MHWRender::MGeometry::primitiveString(indexPrimType).asChar(),
                        MHWRender::MGeometry::dataTypeString(buffer->dataType()).asChar(),
                        indexBufferCount,
                        *indexBufferId);
                        }						*/
                }

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
                        result = false;
                        break;
                };

                GLenum indexType =
                    ( buffer->dataType() == MHWRender::MGeometry::kUnsignedInt32  ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT );
                if(!result)
                    break;

                if (indexBufferId  && (*indexBufferId > 0))
                {
                    // Now render the passes for this effect
                    const cgfxPass* pass = technique->getFirstPass();

                    if (technique->getNumPasses() == 1) {
                        // For single pass effect, the pass state has
                        // is set only once.
                        if (sLastDrawShaderNode == NULL) {
                            if (sActiveShaderNode->fPassStateSetters[0].isPushPopAttribsRequired()) {
                                gGLFT->glPushAttrib(GL_ALL_ATTRIB_BITS);
                            }
                            pass->setCgState();
                        }
                        else {
                            if (needFullCgSetPassState) {
                                pass->setCgState();
                            }
                            else {
                                pass->updateCgParameters();
                            }
                        }

                        pass->bind(pBindSource, bufferCount);
                        gGLFT->glBindBufferARB(MGL_ELEMENT_ARRAY_BUFFER_ARB, *indexBufferId);

                        // Now render the passes for this effect
                        gGLFT->glDrawElements(indexPrimTypeGL, indexBufferCount, indexType, GLOBJECT_BUFFER_OFFSET(0));
                    }
                    else {
                        for (int i=0; pass; ++i, pass = pass->getNext()) {

                            sActiveShaderNode->fPassStateSetters[i].setPassState(stateMgr);

                            // Update render state for each pass
                            if (sActiveShaderNode->fPassStateSetters[i].isPushPopAttribsRequired()) {
                                gGLFT->glPushAttrib(GL_ALL_ATTRIB_BITS);
                            }
                            pass->setCgState();

                            pass->bind(pBindSource, bufferCount);
                            gGLFT->glBindBufferARB(MGL_ELEMENT_ARRAY_BUFFER_ARB, *indexBufferId);

                            // Now render the passes for this effect
                            gGLFT->glDrawElements(indexPrimTypeGL, indexBufferCount, indexType, GLOBJECT_BUFFER_OFFSET(0));

                            glStateCache::instance().flushState();
                            pass->resetCgState();
                            if (sActiveShaderNode->fPassStateSetters[i].isPushPopAttribsRequired()) {
                                gGLFT->glPopAttrib();
                            }
                        }
                        stateMgr->setBlendState(fOldBlendState);
                        stateMgr->setDepthStencilState(fOldDepthStencilState);
                        stateMgr->setRasterizerState(fOldRasterizerState);
                    }
				}
			}

			delete[] pBindSource;
		}

		checkGlErrors("cgfxShaderOverride::draw");

        sLastDrawShaderNode = fShaderNode;
	}
	else // fEffect must be NULL
	{
		// Setting the result to false means that the plugin
		// cannot perform the render properly.
		result = false;
	}

	return result;
}

void cgfxShaderOverride::terminateKey(MHWRender::MDrawContext& context)
{
	TRACE_API_CALLS("cgfxShaderOverride::terminateKey");

	if (!fShaderNode || !sActiveShaderNode)
	{
		//printf("Failed cgfxShaderOverride::terminateKey() - no shader node\n");
		return;
	}

	const cgfxTechnique* technique = sActiveShaderNode->fCurrentTechnique;
	if (technique && technique->isValid())
	{
        MHWRender::MStateManager* stateMgr = context.getStateManager();

        const cgfxPass* pass = technique->getFirstPass();
        if (technique->getNumPasses() == 1) {
            // For single pass effects, we reset the pass state at
            // termination time.
            glStateCache::instance().flushState();
            pass->resetCgState();
            if (sActiveShaderNode->fPassStateSetters[0].isPushPopAttribsRequired()) {
                MGLFunctionTable *gGLFT =
                    MHardwareRenderer::theRenderer()->glFunctionTable();
                gGLFT->glPopAttrib();
            }

		    //restore render state after rendering
		    stateMgr->setBlendState(fOldBlendState);
		    stateMgr->setDepthStencilState(fOldDepthStencilState);
		    stateMgr->setRasterizerState(fOldRasterizerState);
        }

		glPopClientAttrib();

        MHWRender::MStateManager::releaseBlendState(fOldBlendState);
        MHWRender::MStateManager::releaseDepthStencilState(fOldDepthStencilState);
        MHWRender::MStateManager::releaseRasterizerState(fOldRasterizerState);
        fOldBlendState = 0;
        fOldDepthStencilState = 0;
        fOldRasterizerState = 0;
	}

    sActiveShaderNode = NULL;
    sLastDrawShaderNode = NULL;
}

// Override properties
MHWRender::DrawAPI cgfxShaderOverride::supportedDrawAPIs() const
{
	return MHWRender::kOpenGL;
}

bool cgfxShaderOverride::isTransparent()
{
	if(fShaderNode && fShaderNode->fCurrentTechnique)
		return fShaderNode->fCurrentTechnique->hasBlending();

	return false;
}

bool cgfxShaderOverride::overridesDrawState()
{
	return true;
}

bool cgfxShaderOverride::bindAttrValues() const
{
	if (!fShaderNode || !sActiveShaderNode ||
        sActiveShaderNode->fEffect.isNull() ||
        !sActiveShaderNode->fEffect->isValid() ||
        !sActiveShaderNode->fTechnique.length())
		return false;

	MStatus  status;
	MObject  oNode = fShaderNode->thisMObject();

    MGLFunctionTable *gGLFT =
      MHardwareRenderer::theRenderer()->glFunctionTable();

    bool needFullCgSetPassState = false;

	// This method should NEVER access the shape. If you find yourself tempted to access
	// any data from the shape here (like the matrices), be strong and resist! Any shape
	// dependent data should be set in bindAttrViewValues instead!
	//

    // The cgfxAttrDef class contains data members (such as fAttr and
    // fAttr2) that are relative to the current node (fShaderNode). It
    // also contains data members (such as fParameterHandle) that are
    // relative the to current CGeffect (sActiveShaderNode). It is
    // important that we use the correct cgfxAttrDef when accessing
    // these data members. Note that we assume here that the
    // attributes of two lists are listed in the same order. This
    // should be the case because they have been created from the same
    // CgFX file.
    //
    // Note that this is a temporary situation. It should go away once
    // we create a unique tupple of CGeffect, cgfxEffectDef and
    // cgfxAttrDefList for each matching shader key.
	for (cgfxAttrDefList::iterator it(fShaderNode->fAttrDefList),
             activeIt(sActiveShaderNode->fAttrDefList);
         it; ++it, ++activeIt )
	{                                  // loop over fAttrDefList
        cgfxAttrDef* aDef = *it;
		cgfxAttrDef* activeDef = *activeIt;

        if (aDef->fName != activeDef->fName) {
          fShaderNode->reportInternalError( __FILE__, __LINE__ );
        }

		try
		{

			switch (aDef->fType)
			{
			case cgfxAttrDef::kAttrTypeBool:
				{
					bool tmp;
					aDef->getValue(oNode, tmp);
					cgSetParameter1i(activeDef->fParameterHandle, tmp);
					break;
				}

			case cgfxAttrDef::kAttrTypeInt:
				{
					int tmp;
					aDef->getValue(oNode, tmp);
					cgSetParameter1i(activeDef->fParameterHandle, tmp);
					break;
				}

			case cgfxAttrDef::kAttrTypeFloat:
				{
					float tmp;
					aDef->getValue(oNode, tmp);
					cgSetParameter1f(activeDef->fParameterHandle, tmp);
					aDef->setUnitsToInternal(aDef->fParameterHandle);
					break;
				}

			case cgfxAttrDef::kAttrTypeString:
				{
					MString tmp;
					aDef->getValue(oNode, tmp);
					cgSetStringParameterValue(activeDef->fParameterHandle, tmp.asChar());
					break;
				}

			case cgfxAttrDef::kAttrTypeVector2:
				{
					float tmp[2];
					aDef->getValue(oNode, tmp[0], tmp[1]);
					cgSetParameter2fv(activeDef->fParameterHandle, tmp);
					aDef->setUnitsToInternal(aDef->fParameterHandle);
					break;
				}

			case cgfxAttrDef::kAttrTypeVector3:
			case cgfxAttrDef::kAttrTypeColor3:
				{
					float tmp[3];
					aDef->getValue(oNode, tmp[0], tmp[1], tmp[2]);
					cgSetParameter3fv(activeDef->fParameterHandle, tmp);
					aDef->setUnitsToInternal(aDef->fParameterHandle);
					break;
				}

			case cgfxAttrDef::kAttrTypeVector4:
			case cgfxAttrDef::kAttrTypeColor4:
				{
					float tmp[4];
					aDef->getValue(oNode, tmp[0], tmp[1], tmp[2], tmp[3]);
					cgSetParameter4fv(activeDef->fParameterHandle, tmp);
					aDef->setUnitsToInternal(aDef->fParameterHandle);
					break;
				}

			case cgfxAttrDef::kAttrTypeWorldDir:
			case cgfxAttrDef::kAttrTypeWorldPos:
				{
					// since it is in world space, we don't need to do extra mat computation. set the value directly.
					// Read the value
					float tmp[4];
					if (aDef->fSize == 3)
					{
						aDef->getValue(oNode, tmp[0], tmp[1], tmp[2]);
						tmp[3] = 1.0;
					}
					else
					{
						aDef->getValue(oNode, tmp[0], tmp[1], tmp[2], tmp[3]);
					}

					cgSetParameterValuefr(activeDef->fParameterHandle, aDef->fSize, tmp);
					aDef->setUnitsToInternal(aDef->fParameterHandle);
					break;
				}

			case cgfxAttrDef::kAttrTypeMatrix:
				{
					MMatrix tmp;
					float tmp2[4][4];
					aDef->getValue(oNode, tmp);

					if (aDef->fInvertMatrix)
					{
						tmp = tmp.inverse();
					}

					if (!aDef->fTransposeMatrix)
					{
						tmp = tmp.transpose();
					}

					tmp.get(tmp2);
					cgSetMatrixParameterfr(activeDef->fParameterHandle, &tmp2[0][0]);
					break;
				}

			case cgfxAttrDef::kAttrTypeColor1DTexture:
			case cgfxAttrDef::kAttrTypeColor2DTexture:
			case cgfxAttrDef::kAttrTypeColor3DTexture:
			case cgfxAttrDef::kAttrTypeColor2DRectTexture:
			case cgfxAttrDef::kAttrTypeNormalTexture:
			case cgfxAttrDef::kAttrTypeBumpTexture:
			case cgfxAttrDef::kAttrTypeCubeTexture:
			case cgfxAttrDef::kAttrTypeEnvTexture:
			case cgfxAttrDef::kAttrTypeNormalizationTexture:


				{
					MString texFileName;
					MObject textureNode = MObject::kNullObj;

					if( fShaderNode->fTexturesByName)
					{
						aDef->getValue(oNode, texFileName);
					}
					else
					{
						// If we have a fileTexture node connect, get the
						// filename it is using
						MPlug srcPlug;
						aDef->getSource(oNode, srcPlug);
						MObject srcNode = srcPlug.node();
						if( srcNode != MObject::kNullObj)
						{
							MStatus rc;
							MFnDependencyNode dgFn( srcNode);
							MPlug filenamePlug = dgFn.findPlug( "fileTextureName", &rc);
							if( rc == MStatus::kSuccess)
							{
								filenamePlug.getValue( texFileName);
								textureNode = filenamePlug.node(&rc);
							}

							// attach a monitor to this texture if we don't already have one
							// Note that we don't need to worry about handling node destroyed
							// or disconnected, as both of these will trigger attribute changed
							// messages before going away, and we will deregister our callback
							// in the handler!
							if( aDef->fTextureMonitor == kNullCallback && textureNode != MObject::kNullObj)
							{
								// If we don't have a callback, this may mean our texture is dirty
								// and needs to be re-loaded (because we can't actually delete the
								// texture itself in the DG callback we need to wait until we
								// know we have a GL context - like right here)
								aDef->releaseTexture();
								aDef->fTextureMonitor = MNodeMessage::addAttributeChangedCallback(
                                    textureNode, textureChangedCallback, aDef);
							}
						}
					}

					if (aDef->fTexture.isNull() || texFileName != aDef->fStringDef)
					{
                        aDef->fStringDef = texFileName;
                        aDef->fTexture = cgfxTextureCache::instance().getTexture(
                            texFileName, textureNode, fShaderNode->fShaderFxFile,
                            aDef->fName, aDef->fType);

                        cgGLSetupSampler(activeDef->fParameterHandle, aDef->fTexture->getTextureId());

                        if (!aDef->fTexture->isValid() && texFileName.length() > 0) {
                            MFnDependencyNode fnNode( oNode );
                            MString sMsg = "cgfxShader ";
                            sMsg += fnNode.name();
                            sMsg += " : failed to load texture \"";
                            sMsg += texFileName;
                            sMsg += "\".";
                            MGlobal::displayWarning( sMsg );
                        }

                        // We need to call cgSetPassState() after
                        // having called cgGLSetupSampler(). Only
                        // calling cgUpdateProgramParameters() is
                        // insufficient...
                        needFullCgSetPassState = true;
					}
                    else if (sLastDrawShaderNode == NULL) {
                        // cgSetPassState() will be called in this case
                        // and cgGLSetTextureParameter() will
                        // therefore work correctly.
                        cgGLSetTextureParameter(activeDef->fParameterHandle, aDef->fTexture->getTextureId());
                    }
                    else {
                        GLuint textureId = aDef->fTexture->getTextureId();

                        // cgUpdateProgramParameters() will be called in this case
                        // and cgGLSetTextureParameter() does not work
                        // for some reason in this case. The
                        // currrently bound active texture never gets
                        // updated. We therefore have to manually
                        // update the currently bound OpenGL texture.
                        cgGLSetTextureParameter(activeDef->fParameterHandle, textureId);

                        GLenum texEnum = cgGLGetTextureEnum(activeDef->fParameterHandle);
                        gGLFT->glActiveTexture(texEnum);
						switch (aDef->fType)
						{
						case cgfxAttrDef::kAttrTypeColor1DTexture:
							gGLFT->glBindTexture(GL_TEXTURE_1D, textureId);
							break;
						case cgfxAttrDef::kAttrTypeColor2DTexture:
						case cgfxAttrDef::kAttrTypeNormalTexture:
						case cgfxAttrDef::kAttrTypeBumpTexture:
#if !defined(WIN32) && !defined(LINUX)
						case cgfxAttrDef::kAttrTypeColor2DRectTexture:
#endif
							gGLFT->glBindTexture(GL_TEXTURE_2D, textureId);
							break;
						case cgfxAttrDef::kAttrTypeEnvTexture:
						case cgfxAttrDef::kAttrTypeCubeTexture:
						case cgfxAttrDef::kAttrTypeNormalizationTexture:
							{
								gGLFT->glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, textureId);
								break;
							}
						case cgfxAttrDef::kAttrTypeColor3DTexture:
							gGLFT->glBindTexture(GL_TEXTURE_3D, textureId);
#if defined(WIN32) || defined(LINUX)
							// No such thing as NV texture rectangle
							// on Mac.
						case cgfxAttrDef::kAttrTypeColor2DRectTexture:
							gGLFT->glBindTexture(GL_TEXTURE_RECTANGLE_NV, textureId);
							break;
#endif
						default:
							assert(false);
						}
                    }

					checkGlErrors("After loading texture");

					break;
				}
#ifdef _WIN32
			case cgfxAttrDef::kAttrTypeTime:
				{
					int ival = timeGetTime() & 0xffffff;
					float val = (float)ival * 0.001f;
					cgSetParameter1f(activeDef->fParameterHandle, val);
					break;
				}
#endif
			case cgfxAttrDef::kAttrTypeOther:
			case cgfxAttrDef::kAttrTypeUnknown:
				break;

			case cgfxAttrDef::kAttrTypeObjectDir:
			case cgfxAttrDef::kAttrTypeViewDir:
			case cgfxAttrDef::kAttrTypeProjectionDir:
			case cgfxAttrDef::kAttrTypeScreenDir:

			case cgfxAttrDef::kAttrTypeObjectPos:
			case cgfxAttrDef::kAttrTypeViewPos:
			case cgfxAttrDef::kAttrTypeProjectionPos:
			case cgfxAttrDef::kAttrTypeScreenPos:

			case cgfxAttrDef::kAttrTypeWorldMatrix:
			case cgfxAttrDef::kAttrTypeViewMatrix:
			case cgfxAttrDef::kAttrTypeProjectionMatrix:
			case cgfxAttrDef::kAttrTypeWorldViewMatrix:
			case cgfxAttrDef::kAttrTypeWorldViewProjectionMatrix:
				// View dependent parameter
				break;
			default:
				M_CHECK( false );
			}                          // switch (aDef->fType)
		}
		catch ( cgfxShaderCommon::InternalError* e )
		{
			if ( ++fShaderNode->fErrorCount <= fShaderNode->fErrorLimit )
			{
				size_t ee = (size_t)e;
				MFnDependencyNode fnNode( oNode );
				MString sMsg = "cgfxShader warning ";
				sMsg += (int)ee;
				sMsg += ": ";
				sMsg += fnNode.name();
				sMsg += " internal error while setting parameter \"";
				sMsg += aDef->fName;
				sMsg += "\" of effect \"";
				sMsg += fShaderNode->fShaderFxFile;
				sMsg += "\" for shape ";
				sMsg += fShaderNode->currentPath().partialPathName();
				MGlobal::displayWarning( sMsg );
			}
		}
	}                                  // loop over fAttrDefList
    return needFullCgSetPassState;
}

void cgfxShaderOverride::bindViewAttrValues(const MHWRender::MDrawContext& context) const
{
	if (!fShaderNode || !sActiveShaderNode ||
        sActiveShaderNode->fEffect.isNull() ||
        !sActiveShaderNode->fEffect->isValid() ||
        !sActiveShaderNode->fTechnique.length())
		return;

	MStatus  status;
	MObject  oNode = fShaderNode->thisMObject();

	MMatrix wMatrix, vMatrix, pMatrix, sMatrix;
	MMatrix wvMatrix, wvpMatrix, wvpsMatrix;
	MMatrix vpMatrix, vpsMatrix;
	{
		wvpMatrix = context.getMatrix(MHWRender::MFrameContext::kWorldViewProjMtx);
		wvMatrix = context.getMatrix(MHWRender::MFrameContext::kWorldViewMtx);
		wMatrix = context.getMatrix(MHWRender::MFrameContext::kWorldMtx);
		vMatrix = context.getMatrix(MHWRender::MFrameContext::kViewMtx);
		pMatrix = context.getMatrix(MHWRender::MFrameContext::kProjectionMtx);
		vpMatrix = context.getMatrix(MHWRender::MFrameContext::kViewProjMtx);

		int vpt[4];
		float depth[2];

		context.getViewportDimensions(vpt[0], vpt[1], vpt[2], vpt[3]);
		context.getDepthRange(depth[0], depth[1]);

		// Construct the NDC -> screen space matrix
		//
		double x0, y0, z0, w, h, d;

		x0 = (double)vpt[0];
		y0 = (double)vpt[1];
		z0 = depth[0];
		w  = (double)vpt[2];
		h  = (double)vpt[3];
		d  = depth[1] - z0;

		// Make a reference to ease the typing
		//
		double* s = &sMatrix.matrix[0][0];

		s[ 0] = w/2;	s[ 1] = 0.0;	s[ 2] = 0.0;	s[ 3] = 0.0;
		s[ 4] = 0.0;	s[ 5] = h/2;	s[ 6] = 0.0;	s[ 7] = 0.0;
		s[ 8] = 0.0;	s[ 9] = 0.0;	s[10] = d/2;	s[11] = 0.0;
		s[12] = x0+w/2;	s[13] = y0+h/2;	s[14] = z0+d/2;	s[15] = 1.0;

		vpsMatrix = vpMatrix * sMatrix;
		wvpsMatrix = wvpMatrix * sMatrix;
	}

	// Get Hardware Fog params.
	MHWRender::MFrameContext::HwFogParams hwFogParams = context.getHwFogParameters();

    // The cgfxAttrDef class contains data members (such as fAttr and
    // fAttr2) that are relative to the current node (fShaderNode). It
    // also contains data members (such as fParameterHandle) that are
    // relative the to current CGeffect (sActiveShaderNode). It is
    // important that we use the correct cgfxAttrDef when accessing
    // these data members. Note that we assume here that the
    // attributes of two lists are listed in the same order. This
    // should be the case because they have been created from the same
    // CgFX file.
    //
    // Note that this is a temporary situation. It should go away once
    // we create a unique tupple of CGeffect, cgfxEffectDef and
    // cgfxAttrDefList for each matching shader key.
	for (cgfxAttrDefList::iterator it(fShaderNode->fAttrDefList),
             activeIt(sActiveShaderNode->fAttrDefList);
         it; ++it, ++activeIt )
	{                                  // loop over fAttrDefList
		cgfxAttrDef* aDef = *it;
		cgfxAttrDef* activeDef = *activeIt;

        if (aDef->fName != activeDef->fName) {
          fShaderNode->reportInternalError( __FILE__, __LINE__ );
        }

		try
		{

			switch (aDef->fType)
			{
			case cgfxAttrDef::kAttrTypeObjectDir:
			case cgfxAttrDef::kAttrTypeViewDir:
			case cgfxAttrDef::kAttrTypeProjectionDir:
			case cgfxAttrDef::kAttrTypeScreenDir:

			case cgfxAttrDef::kAttrTypeObjectPos:
			case cgfxAttrDef::kAttrTypeViewPos:
			case cgfxAttrDef::kAttrTypeProjectionPos:
			case cgfxAttrDef::kAttrTypeScreenPos:
				{
					float tmp[4];
					if (aDef->fSize == 3)
					{
						aDef->getValue(oNode, tmp[0], tmp[1], tmp[2]);
						tmp[3] = 1.0;
					}
					else
					{
						aDef->getValue(oNode, tmp[0], tmp[1], tmp[2], tmp[3]);
					}

					int space = aDef->fType - cgfxAttrDef::kAttrTypeFirstPos;
					if (space < 0)
					{
						space = aDef->fType - cgfxAttrDef::kAttrTypeFirstDir;
					}

					MMatrix mat;  // initially the identity matrix.

					switch (space)
					{
					case 0:	mat = wMatrix.inverse();	break;
					case 1:	/*mat = identity;*/			break;
					case 2:	mat = vMatrix;			break;
					case 3:	mat = vpMatrix;		break;
					case 4:	mat = vpsMatrix;		break;
					}

					// Maya's transformation matrices are set up with
					// the translation in row 3 (like OpenGL) rather
					// than column 3. To transform a point or vector,
					// use V*M, not M*V.   kh 11/2003
					int base = cgfxAttrDef::kAttrTypeFirstPos;
					if (aDef->fType <= cgfxAttrDef::kAttrTypeLastDir)
						base = cgfxAttrDef::kAttrTypeFirstDir;
					if (base == cgfxAttrDef::kAttrTypeFirstPos)
					{
						MPoint point(tmp[0], tmp[1], tmp[2], tmp[3]);
						point *= mat;
						tmp[0] = (float)point.x;
						tmp[1] = (float)point.y;
						tmp[2] = (float)point.z;
						tmp[3] = (float)point.w;
					}
					else
					{
						MVector vec(tmp[0], tmp[1], tmp[2]);
						vec *= mat;
						tmp[0] = (float)vec.x;
						tmp[1] = (float)vec.y;
						tmp[2] = (float)vec.z;
						tmp[3] = 1.F;
					}

					cgSetParameterValuefc(activeDef->fParameterHandle, aDef->fSize, tmp);
					aDef->setUnitsToInternal(aDef->fParameterHandle);
					break;
				}

			case cgfxAttrDef::kAttrTypeWorldMatrix:
			case cgfxAttrDef::kAttrTypeViewMatrix:
			case cgfxAttrDef::kAttrTypeProjectionMatrix:
			case cgfxAttrDef::kAttrTypeWorldViewMatrix:
			case cgfxAttrDef::kAttrTypeWorldViewProjectionMatrix:
				{
					MHWRender::MFrameContext::MatrixType matrixType;
					switch (aDef->fType)
					{
					case cgfxAttrDef::kAttrTypeWorldMatrix:
						if(aDef->fInvertMatrix && !aDef->fTransposeMatrix)  matrixType = MHWRender::MFrameContext::kWorldTranspInverseMtx;
						else if(aDef->fInvertMatrix) matrixType = MHWRender::MFrameContext::kWorldInverseMtx;
						else if(!aDef->fTransposeMatrix) matrixType = MHWRender::MFrameContext::kWorldTransposeMtx;
						else matrixType = MHWRender::MFrameContext::kWorldMtx;
						break;
					case cgfxAttrDef::kAttrTypeViewMatrix:
						if(aDef->fInvertMatrix && !aDef->fTransposeMatrix)  matrixType = MHWRender::MFrameContext::kViewTranspInverseMtx;
						else if(aDef->fInvertMatrix) matrixType = MHWRender::MFrameContext::kViewInverseMtx;
						else if(!aDef->fTransposeMatrix) matrixType = MHWRender::MFrameContext::kViewTransposeMtx;
						else matrixType = MHWRender::MFrameContext::kViewMtx;
						break;
					case cgfxAttrDef::kAttrTypeProjectionMatrix:
						if(aDef->fInvertMatrix && !aDef->fTransposeMatrix)  matrixType = MHWRender::MFrameContext::kProjectionTranspInverseMtx;
						else if(aDef->fInvertMatrix) matrixType = MHWRender::MFrameContext::kProjectionInverseMtx;
						else if(!aDef->fTransposeMatrix) matrixType = MHWRender::MFrameContext::kProjectionTranposeMtx;
						else matrixType = MHWRender::MFrameContext::kProjectionMtx;
						break;
					case cgfxAttrDef::kAttrTypeWorldViewMatrix:
						if(aDef->fInvertMatrix && !aDef->fTransposeMatrix)  matrixType = MHWRender::MFrameContext::kWorldViewTranspInverseMtx;
						else if(aDef->fInvertMatrix) matrixType = MHWRender::MFrameContext::kWorldViewInverseMtx;
						else if(!aDef->fTransposeMatrix) matrixType = MHWRender::MFrameContext::kWorldViewTransposeMtx;
						else matrixType = MHWRender::MFrameContext::kWorldViewMtx;
						break;
					case cgfxAttrDef::kAttrTypeWorldViewProjectionMatrix:
						if(aDef->fInvertMatrix && !aDef->fTransposeMatrix)  matrixType = MHWRender::MFrameContext::kWorldViewProjTranspInverseMtx;
						else if(aDef->fInvertMatrix) matrixType = MHWRender::MFrameContext::kWorldViewProjInverseMtx;
						else if(!aDef->fTransposeMatrix) matrixType = MHWRender::MFrameContext::kWorldViewProjTransposeMtx;
						else matrixType = MHWRender::MFrameContext::kWorldViewProjMtx;
						break;
					default:
						matrixType = MHWRender::MFrameContext::kWorldMtx;
						break;
					}

					MMatrix mat = context.getMatrix(matrixType);

					double tmp[4][4];
					mat.get(tmp);
					cgSetMatrixParameterdr(activeDef->fParameterHandle, &tmp[0][0]);
					break;
				}

			case cgfxAttrDef::kAttrTypeHardwareFogEnabled:
				{
					bool fogEnabled = hwFogParams.HwFogEnabled;
					cgSetParameter1i(activeDef->fParameterHandle, fogEnabled);
					break;
				}
			case cgfxAttrDef::kAttrTypeHardwareFogMode:
				{
					unsigned int fogMode = hwFogParams.HwFogMode;
					cgSetParameter1i(activeDef->fParameterHandle, fogMode);
					break;
				}
			case cgfxAttrDef::kAttrTypeHardwareFogStart:
				{
					float fogStart = hwFogParams.HwFogStart;
					cgSetParameter1f(activeDef->fParameterHandle, fogStart);
					break;
				}
			case cgfxAttrDef::kAttrTypeHardwareFogEnd:
				{
					float fogEnd = hwFogParams.HwFogEnd;
					cgSetParameter1f(activeDef->fParameterHandle, fogEnd);
					break;
				}
			case cgfxAttrDef::kAttrTypeHardwareFogDensity:
				{
					float fogDensity = hwFogParams.HwFogDensity;
					cgSetParameter1f(activeDef->fParameterHandle, fogDensity);
					break;
				}
			case cgfxAttrDef::kAttrTypeHardwareFogColor:
				{
					cgSetParameter4fv(activeDef->fParameterHandle, &hwFogParams.HwFogColor[0]);
					break;
				}
				default:
					break;
			}                          // switch (aDef->fType)
		}
		catch ( cgfxShaderCommon::InternalError* e )
		{
			if ( ++fShaderNode->fErrorCount <= fShaderNode->fErrorLimit )
			{
				size_t ee = (size_t)e;
				MFnDependencyNode fnNode( oNode );
				MString sMsg = "cgfxShader warning ";
				sMsg += (int)ee;
				sMsg += ": ";
				sMsg += fnNode.name();
				sMsg += " internal error while setting parameter \"";
				sMsg += aDef->fName;
				sMsg += "\" of effect \"";
				sMsg += fShaderNode->fShaderFxFile;
				sMsg += "\" for shape ";
				/*if (shapePath.isValid())
					sMsg += shapePath.partialPathName();
				else
					sMsg += "SWATCH GEOMETRY";*/
				MGlobal::displayWarning( sMsg );
			}
		}
	}                                  // loop over fAttrDefList

}
