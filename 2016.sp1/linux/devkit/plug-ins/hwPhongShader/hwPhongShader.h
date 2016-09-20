#ifndef _hwPhongShader
#define _hwPhongShader

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
//
// Simple node that implements Phong shading for the special
// case when the light is at the eye. We use a simple
// spherical reflection environment map to compute the Phong
// highlight
//
///////////////////////////////////////////////////////////////////

#include <maya/MPxHwShaderNode.h>
#include <maya/MPoint.h>

// For swatch rendering
#include <maya/MHardwareRenderer.h>
#include <maya/MGeometryData.h>

class hwPhongShader : public MPxHwShaderNode
{
public:
                    hwPhongShader();
    virtual         ~hwPhongShader();
	void			releaseEverything();

    virtual MStatus compute( const MPlug&, MDataBlock& );
	virtual void    postConstructor();

	// Internally cached attribute handling routines
	virtual bool getInternalValueInContext( const MPlug&,
											  MDataHandle&,
											  MDGContext&);
    virtual bool setInternalValueInContext( const MPlug&,
											  const MDataHandle&,
											  MDGContext&);

	// Interactive overrides
	virtual MStatus		bind( const MDrawRequest& request,
							  M3dView& view );
	virtual MStatus		unbind( const MDrawRequest& request,
								M3dView& view );
	virtual MStatus		geometry( const MDrawRequest& request,
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
								const float ** texCoordArrays);

	// Batch overrides
	virtual MStatus	glBind(const MDagPath& shapePath);
	virtual MStatus	glUnbind(const MDagPath& shapePath);
	virtual MStatus	glGeometry( const MDagPath& shapePath,
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
							  const float ** texCoordArrays);

	MStatus			draw(
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
							  const float ** texCoordArrays);

	void			drawDefaultGeometry();

	// Overridden to draw an image for swatch rendering.
	///
	virtual MStatus renderSwatchImage( MImage & image );
	void			drawTheSwatch( MGeometryData* pGeomData,
								   unsigned int* pIndexing,
								   unsigned int  numberOfData,
								   unsigned int  indexCount );

	virtual int		normalsPerVertex();
	virtual int		texCoordsPerVertex();
	virtual int		getTexCoordSetNames(MStringArray& names);

	virtual bool	hasTransparency();

    static  void *  creator();
    static  MStatus initialize();

	MFloatVector	Phong ( double cos_a );
	void			init_Phong_texture ( void );
	unsigned int	PhongTextureId() const { return phong_map_id; }
	float			Transparency() const { return mTransparency; }
	const float3 *	Ambient() const { return &mAmbientColor; }
	const float3 *	Diffuse() const { return &mDiffuseColor; }
	const float3 *	Specular() const { return &mSpecularColor; }
	const float3 * 	Shininess() const { return &mShininess; }
	void setTransparency(const float fTransparency);
	void setAmbient(const float3 &fAmbient);
	void setDiffuse(const float3 &fDiffuse);
	void setSpecular(const float3 &fSpecular);
	void setShininess(const float3 &fShininess);

	void			printGlError( const char *call );

	bool			attributesChangedVP2() { return mAttributesChangedVP2; }
	void			markAttributesChangedVP2() { mAttributesChangedVP2 = true; }
	void			markAttributesCleanVP2() { mAttributesChangedVP2 = false; }

    static  MTypeId id;

	protected:

	void attachSceneCallbacks();
	void detachSceneCallbacks();

	static void releaseCallback(void* clientData);

	private:

	// Attributes
	static MObject  aColor;
	static MObject  aDiffuseColor;
	static MObject  aSpecularColor;
	static MObject  aShininessX;
	static MObject  aShininessY;
	static MObject  aShininessZ;
	static MObject  aShininess;
	static MObject	aTransparency;
	static MObject	aGeometryShape;

	// Internal data
	GLuint			phong_map_id;

	MPoint			cameraPosWS;

	float3			mAmbientColor;
	float			mTransparency;
	float3			mDiffuseColor;
	float3			mSpecularColor;
	float3			mShininess;
	unsigned int	mGeometryShape;
	bool			mAttributesChanged; // Keep track if any attributes changed
	bool			mAttributesChangedVP2; // Keep track if any attributes changed for VP2

	// Callbacks that we monitor so we can release OpenGL-dependant
	// resources before their context gets destroyed.
	MCallbackId fBeforeNewCB;
	MCallbackId fBeforeOpenCB;
	MCallbackId fBeforeRemoveReferenceCB;
	MCallbackId fMayaExitingCB;


};

#endif /* _hwPhongShader */
