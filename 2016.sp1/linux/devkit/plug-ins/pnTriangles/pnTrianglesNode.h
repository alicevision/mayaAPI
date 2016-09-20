#ifndef _pnTrianglesNode
#define _pnTrianglesNode

//-
// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//+

// File: pnTrianglesNode.h
//
// Dependency Graph Node: pnTriangles
//
// Description:
//
//		Hardware shader plugin to perform ATI PN triangle
//		tessellation on geometry. If the ATI extension
//		is not available, the extension will be emulated
//		in software.
//
#include <maya/MPxHwShaderNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MTypeId.h> 

// ATI extension definitions
#include "ATIext.h"
 
class pnTriangles : public MPxHwShaderNode
{
	// Class node methods
public:
						pnTriangles();
	virtual				~pnTriangles(); 

	virtual bool		getInternalValue( const MPlug&, MDataHandle&);
    virtual bool		setInternalValue( const MPlug&, const MDataHandle&);

	// Shader node overrides
	virtual MStatus	bind(const MDrawRequest& request,
						 M3dView& view);

	virtual MStatus	unbind(const MDrawRequest& request,
						   M3dView& view);

	virtual MStatus	geometry( const MDrawRequest& request,
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

	virtual int		normalsPerVertex();
	virtual int		texCoordsPerVertex();
	virtual int		colorsPerVertex();

	static  void*		creator();
	static  MStatus		initialize();

public:
	typedef enum {
		kPointLinear,
		kPointCubic
	} ePointMode;

	typedef enum {
		kNormalLinear,
		kNormalQuadratic 
	} eNormalMode;
	typedef enum
	{
		kPNTriangesEXT = 0,
		kVertexShaderEXT = 1,
		kFragmentShaderEXT = 2
	} eExtension;

	
	// Data access methods
	int					maxTessellationLevel() const { return fMaxTessellationLevel; }
	ePointMode			pointMode() const { return fPointMode; }
	eNormalMode			normalMode() const { return fNormalMode; }
	unsigned int		subdivisions() const { return fSubdivisions; }
	GLboolean			extensionSupported( eExtension e ) const 
							{ return fExtensionSupported[e] ; }

	// Node attributes for PN-triangles
	static  MObject		attrSubdivisions;	// Number of subdivisions
	static	MObject		attrNormalMode;		// Normal evaluation mode
	static	MObject		attrPointMode;		// Point evaluation mode

	// Node attributes for programmable shading
	static	MObject		attrEnableVertexProgram; // Enable vertex program
	static	MObject		attrEnablePixelProgram; // Enable pixel program

	// General node attributes for display
	static	MObject		attrColored;		// Display colored
	static	MObject		attrWireframe;		// Display wireframe
	static	MObject		attrTextured;		// Display textured


	// Node ID
	static	MTypeId		id;					

protected:
	MStatus		 getFloat3(MObject attr, float value[3]);
	unsigned int computePNTriangles(const float * vertexArray,
									const float * normalArray,
									const float * texCoordArray,
									float * pnVertexArray,
									float * pnNormalArray,
									float * pnTexCoordArray,
									float * pnColorArray);
	void bindVertexProgram(const MColor diffuse, 
							const MColor specular, 
							const MColor emission, 
							const MColor ambient);
	void bindFragmentProgram();

	unsigned int		fSubdivisions;		// Cached subdivision value
	GLboolean			fExtensionSupported[3]; // Is PN-triangles extension supported ?

	ePointMode			fPointMode;				// Point mode
	eNormalMode			fNormalMode;			// Normal mode
	unsigned int		fMaxTessellationLevel; // Maximum tessellation level

	// Attribute requirements for geometry. Set
	// depending on display mode
	int					fNumTextureCoords;	// textured
	int					fNumNormals;		// shaded 
	int					fNumColors;			// coloured
	bool				fWireframe;			// wireframe			

	bool				fInTexturedMode;

	bool				fTestVertexProgram;
	bool				fTestFragmentProgram;
	int					fVertexShaderId;
	int					fFragmentShaderId;
};

#endif
