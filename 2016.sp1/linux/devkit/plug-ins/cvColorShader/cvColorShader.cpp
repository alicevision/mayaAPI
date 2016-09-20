//-
// ==========================================================================
// Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

#include <math.h>

#include <maya/MPxNode.h>
#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFloatVector.h>
#include <maya/MPointArray.h>
#include <maya/MFnPlugin.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MFnMesh.h>
#include <maya/MItMeshVertex.h>
#include <maya/MDagPath.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MMutexLock.h>
#include <maya/MRenderUtil.h>
#include <maya/MSelectionList.h>

class cvColorShader : public MPxNode
{
	public:
                    cvColorShader();
    virtual         ~cvColorShader();

    virtual MStatus compute( const MPlug&, MDataBlock& );
	virtual void    postConstructor();

    static  void *  creator();
    static  MStatus initialize();

	//  Id tag for use with binary file format
    static  MTypeId id;

	private:

	static inline float dotProd(const MFloatVector &, const MFloatVector &); 

	static MStatus		getTriangleInfo(
							const MDagPath&	meshPath,
							long			triangleId,
							MPointArray&	vertPositions,
							MColorArray&	vertColours
						);

	static MMutexLock	fCriticalSection;

	// Input attributes

	static MObject aReverseAlpha;

	static MObject aPointObj;				// Implicit attribute
	static MObject aPrimitiveId;			// Implicit attribute
	static MObject aObjectId;			// Implicit attribute

	// Output attributes
	static MObject aOutColor;
	static MObject aOutAlpha;
};

// Static data
MTypeId cvColorShader::id( 0x8000f );
MMutexLock cvColorShader::fCriticalSection;

// Attributes 
MObject cvColorShader::aReverseAlpha;
MObject cvColorShader::aPointObj;
MObject cvColorShader::aPrimitiveId;
MObject cvColorShader::aObjectId;
MObject cvColorShader::aOutColor;
MObject cvColorShader::aOutAlpha;

void cvColorShader::postConstructor( )
{
	setMPSafe(true);
}

cvColorShader::cvColorShader()
{
}

cvColorShader::~cvColorShader()
{
}

void * cvColorShader::creator()
{
    return new cvColorShader();
}

MStatus cvColorShader::initialize()
{
    MFnNumericAttribute nAttr;

	aReverseAlpha = nAttr.create( "reverseAlpha", "ra", 
								  MFnNumericData::kBoolean);
	CHECK_MSTATUS ( nAttr.setDefault( true ) );

    aPointObj  = nAttr.createPoint( "pointObj", "po" );
	CHECK_MSTATUS ( nAttr.setStorable(false) );
    CHECK_MSTATUS ( nAttr.setHidden(true) );

	aPrimitiveId = nAttr.create( "primitiveId", "pi", MFnNumericData::kLong);
    CHECK_MSTATUS ( nAttr.setHidden(true) );

	aObjectId = nAttr.createAddr("objectId", "oi");
	CHECK_MSTATUS ( nAttr.setHidden(true) );

    aOutColor = nAttr.createColor( "outColor", "oc" );
    CHECK_MSTATUS ( nAttr.setStorable(false) );
    CHECK_MSTATUS ( nAttr.setReadable(true) );
    CHECK_MSTATUS ( nAttr.setWritable(false) );

	aOutAlpha = nAttr.create( "outAlpha", "oa", MFnNumericData::kFloat);
	CHECK_MSTATUS (  nAttr.setDisconnectBehavior(MFnAttribute::kReset) );
    CHECK_MSTATUS ( nAttr.setStorable(false) );
    CHECK_MSTATUS ( nAttr.setReadable(true) );
    CHECK_MSTATUS ( nAttr.setWritable(false) );

    CHECK_MSTATUS ( addAttribute(aPointObj) );
    CHECK_MSTATUS ( addAttribute(aOutColor) );
    CHECK_MSTATUS ( addAttribute(aOutAlpha) );
    CHECK_MSTATUS ( addAttribute(aReverseAlpha) );
    CHECK_MSTATUS ( addAttribute(aPrimitiveId) );
    CHECK_MSTATUS ( addAttribute(aObjectId) );

    CHECK_MSTATUS ( attributeAffects(aPointObj,     aOutColor) );
    CHECK_MSTATUS ( attributeAffects(aPrimitiveId,  aOutColor) );
    CHECK_MSTATUS ( attributeAffects(aObjectId,  aOutColor) );

    CHECK_MSTATUS ( attributeAffects(aReverseAlpha, aOutAlpha) );
    CHECK_MSTATUS ( attributeAffects(aPointObj,     aOutAlpha) );
    CHECK_MSTATUS ( attributeAffects(aPrimitiveId,  aOutAlpha) );
    CHECK_MSTATUS ( attributeAffects(aObjectId,  aOutAlpha) );

	return MS::kSuccess;
}

// dot product on vectors
inline float cvColorShader::dotProd(
	const MFloatVector & v1,
	const MFloatVector & v2) 
{
	return  v1.x*v2.x +  v1.y*v2.y + v1.z*v2.z;
}

//
//
///////////////////////////////////////////////////////////////////////////////
MStatus cvColorShader::compute( const MPlug& plug, MDataBlock& block ) 
{
	if ((plug != aOutColor) && (plug.parent() != aOutColor) && 
		(plug != aOutAlpha))
		return MS::kUnknownParameter;

	MStatus status;
	MObject thisNode = thisMObject();

	bool rev_flag = block.inputValue(aReverseAlpha).asBool();
	long triangleId = block.inputValue(aPrimitiveId).asLong();
	void* objectId = block.inputValue(aObjectId).asAddr();

	// Location of the point we are shading
	MFloatVector& pointObj = block.inputValue( aPointObj ).asFloatVector();

	MColor resultColor;

	// It's only worth continuing if the renderer was able to supply us
	// with a surface.
	if (objectId != NULL) {
		// Get the mesh that we are shading.
		MDagPath		meshPath;
		MSelectionList	list;
		status = MRenderUtil::renderObjectItem(objectId, list);
		list.getDagPath(0, meshPath);

		// Get the positions and colours of the triangle's vertices.
		//
		// Note that we could get the triangle's vertices
		MPointArray	pos;
		MColorArray	colours;
		status = getTriangleInfo(meshPath, triangleId, pos, colours);

		MFloatVector pos1((float)pos[0].x, (float)pos[0].y, (float)pos[0].z);
		MFloatVector pos2((float)pos[1].x, (float)pos[1].y, (float)pos[1].z);
		MFloatVector pos3((float)pos[2].x, (float)pos[2].y, (float)pos[2].z);

		// Compute the barycentric coordinates of the sample.

		pointObj = pointObj - pos3;				// Translate pos3 to origin
		pos1 = pos1 - pos3;
		pos2 = pos2 - pos3;

		MFloatVector norm = pos1 ^ pos2;		// Triangle normal
		float len = dotProd(norm, norm);
		len = dotProd(norm, pointObj)/len;

		pointObj = pointObj - (len * norm);		// Make sure the point is
												// in the triangle

		float aa = dotProd(pos1, pos1);
		float bb = dotProd(pos2, pos2);
		float ab = dotProd(pos1, pos2);
		float am = dotProd(pos1, pointObj);
		float bm = dotProd(pos2, pointObj);
		float det = aa*bb - ab*ab;

		// a, b, c are the barycentric coordinates (assuming pnt
		// is in the triangle plane, best least square fit
		// otherwise.
		//
		float a = (am*bb - bm*ab) / det;
		float b = (bm*aa - am*ab) / det;
		float c = 1-a-b;

		resultColor = (a*colours[0]) + (b*colours[1]) + (c*colours[2]);

		if( rev_flag == true )
			resultColor.a = 1.0f - resultColor.a;
	}

	MDataHandle outColorHandle = block.outputValue( aOutColor );
	MFloatVector& outColor = outColorHandle.asFloatVector();
	outColor.x = resultColor.r;
	outColor.y = resultColor.g;
	outColor.z = resultColor.b;
	outColorHandle.setClean();

	MDataHandle outAlphaHandle = block.outputValue( aOutAlpha );
	float& outAlpha = outAlphaHandle.asFloat();
	outAlpha = resultColor.a;
	outAlphaHandle.setClean();

	return MS::kSuccess;
}


MStatus cvColorShader::getTriangleInfo(
	const MDagPath&	meshPath,
	long			triangleId,
	MPointArray&	vertPositions,
	MColorArray&	vertColours
) {
	MStatus	st;

	//	'triangleId' refers to the triangle currently being shaded. We need
	//	to find the positions and colours of the triangle's three vertices.
	//
	//	We could use the 'vertexCamera*' render attributes to determine the
	//	positions of the triangle's vertices, but to determine the color at
	//	a vertex we need to know the vertex's index within the mesh and
	//	there is no render attribute which will give us that. We will have
	//	to determine it ourselves by finding the face to which the triangle
	//	belongs and then using MItMeshPolygon::getTriangle() to get the
	//	indices of the triangle's vertices. The call to getTriangle() also
	//	happens to return the positions of the vertices, so we won't bother
	//	with the 'vertexCamera*' render attributes.
	//
	//	The way we find the face to which the triangle belongs is by
	//	running through all of the mesh's faces and counting the number of
	//	triangles in each one. When the count exceeds the value of
	//	'triangleId', the face which put us over is the face containing the
	//	triangle.
	//
	//	The renderer does not assign triangle ids to the mesh all at once,
	//	but separately for each shading group. For example, let's say that
	//	a mesh has 20 faces comprised of a total of 30 triangles, assigned
	//	to two different shaders, as follows:
	//
	//	- 12 faces, having a total of 19 triangles, are assigned to
	//	  the first shader
	//	- 5 faces, having a total of 7 triangles, are assigned to
	//	  the second shader
	//	- the remaining 3 faces, having a total of 4 triangles,
	//	  are not assigned to any shader.
	//
	//	The 19 triangles in the first shader will be given primitiveIds 0 to 18.
	//	The 7 trianges in the second shader will be given primitiveIds 19 to 25.
	//	The 4 triangles which are not assigned to any shader won't have any
	//	primitiveIds assigned to them.
	//
	//	So when we're counting triangles, we must do it in shader order.


	//	The first step is to get all of the shaders used by this mesh and the
	//	faces to which they are assigned.
	MFnMesh			meshFn(meshPath);
	MObjectArray	shaders;
	MObjectArray	components;

	meshFn.getConnectedSetsAndMembers(
		meshPath.instanceNumber(), shaders, components, true
	);

	int			polygonId = -1;
	int			triangleCount = 0;
	MIntArray	vertIndices;

	//	Step through each shader.
	for (unsigned int s = 0; (polygonId < 0) && (s < shaders.length()); ++s) {
		//	Iterate over the faces assigned to this shader.
		//
		//	The constructor for MItMeshPolygon is not threadsafe as it may
		//	initiate a recalculation of the mesh's normals. So we must lock
		//	the thread while making the call.
		//
		//	Similarly, MItMeshMeshPolygon::hasValidTriangulation() may
		//	trigger triangulation of the mesh, which is also not thread
		//	safe. So we want to keep our lock until after the first call to
		//	it.
		bool	isLocked = true;
		fCriticalSection.lock();
		MItMeshPolygon faceIter(meshPath, components[s]);


		for (; !faceIter.isDone(); faceIter.next()) {
			if (faceIter.hasValidTriangulation()) {
				//	Get the number of triangles in the current face.
				int nTri;
				faceIter.numTriangles(nTri);

				//	If this face will put the count over 'triangleId' then
				//	the triangle must belong to this face.
				if (triangleId < triangleCount + nTri) {
					//	Get the positions and indices of the triangle's
					//	vertices, then break out of the loop. We subtract
					//	'triangleCount' from 'triangleId' to get the index
					//	of the triangle within the face.
					polygonId = faceIter.index();
					st = faceIter.getTriangle(
							triangleId - triangleCount,
							vertPositions,
							vertIndices,
							MSpace::kObject
						);

					break;
				}

				//	We haven't found the right face yet. Add the number of
				//	triangles in this face to the count and keep going.
				triangleCount += nTri;
			}

			//	If hasValidTriangulation() was going to triangulate the
			//	mesh it will have done so by now. Subsequent calls will
			//	use the existing triangulation so it's safe to remove the
			//	lock now.
			if (isLocked) {
				fCriticalSection.unlock();
				isLocked = false;
			}
		}

		//	If the shader has no face components assigned to it then the
		//	'for' loop above will not have run and fCriticalSection will
		//	still be locked, in which case we must unlock it now.
		if (isLocked) {
			fCriticalSection.unlock();
		}
	}

	if ((polygonId == -1) || !st) {
		return MS::kFailure;
	}

	//	Now that we know the indices of the triangle's vertices, get their
	//	colours.
	MItMeshVertex vertIter(meshPath);
	int preIndex = 0;
	vertColours.setLength(3);

	CHECK_MSTATUS ( vertIter.setIndex( vertIndices[0], preIndex) );
	CHECK_MSTATUS ( vertIter.getColor( vertColours[0], polygonId ) );
	
	CHECK_MSTATUS ( vertIter.setIndex( vertIndices[1], preIndex) );
	CHECK_MSTATUS ( vertIter.getColor( vertColours[1], polygonId ) );

	CHECK_MSTATUS ( vertIter.setIndex( vertIndices[2], preIndex) );
	CHECK_MSTATUS ( vertIter.getColor( vertColours[2], polygonId ) );


	return MS::kSuccess;
}


/////////////////////////////////////////////////////////////////////////////////////////
MStatus initializePlugin( MObject obj )
{ 
	const MString UserClassify( "utility/color" );
	
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "5.0", "Any");
	CHECK_MSTATUS ( plugin.registerNode( "cvColorShader", cvColorShader::id, 
						 cvColorShader::creator, 
						 cvColorShader::initialize,
						 MPxNode::kDependNode, &UserClassify ) );

	return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj )
{
	MFnPlugin plugin( obj );
	CHECK_MSTATUS ( plugin.deregisterNode( cvColorShader::id ) );

	return MS::kSuccess;
}
