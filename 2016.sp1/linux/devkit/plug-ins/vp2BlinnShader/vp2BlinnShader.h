#ifndef _vp2BlinnShader
#define _vp2BlinnShader
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
#include <maya/MPxHwShaderNode.h>

class vp2BlinnShader : public MPxHwShaderNode 
{
public:
                    vp2BlinnShader();
    virtual         ~vp2BlinnShader();

    virtual MStatus compute( const MPlug&, MDataBlock& );

	// VP1 shader methods
	virtual MStatus		bind( const MDrawRequest& request,
							  M3dView& view )
	{
		return MStatus::kSuccess;
	}

	virtual MStatus		unbind( const MDrawRequest& request,
								M3dView& view )
	{
		return MStatus::kSuccess;
	}

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
								const float ** texCoordArrays)
	{
		return MStatus::kSuccess;
	}
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
							  const float ** texCoordArrays)
	{
		return MStatus::kSuccess;
	}

	// Swatch rendering
	//
	virtual MStatus renderSwatchImage( MImage & image );

    static  void *  creator();
    static  MStatus initialize();

    static  MTypeId id;

protected:

private:
	// Attributes
	static MObject  aColor;
	static MObject	aTransparency;
	static MObject  aDiffuseColor;
	static MObject  aSpecularColor;
	static MObject  aNonTexturedColor;
};

#endif /* _vp2BlinnShader */
