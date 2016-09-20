//-
// ==========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
// ==========================================================================
//+
#if _MSC_VER >= 1700
#pragma warning( disable: 4005 )
#endif

#include <maya/MPxLocatorNode.h>
#include <maya/MString.h>
#include <maya/MDagPath.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MVector.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MColor.h>
#include <maya/M3dView.h>
#include <maya/MFnPlugin.h>
#include <maya/MDistance.h>
#include <maya/MMatrix.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFileObject.h>

// Viewport 2.0 includes
#include <maya/MDrawRegistry.h>
#include <maya/MPxDrawOverride.h>
#include <maya/MUserData.h>
#include <maya/MDrawContext.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MHWGeometryUtilities.h>
#include <maya/MStateManager.h>
#include <maya/MShaderManager.h>
#include <maya/MRenderTargetManager.h>
#include <maya/MAnimControl.h>

#include <assert.h>

// DX stuff
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <d3d11.h>

#if _MSC_VER < 1700
	#include <d3dx11.h>
	#include <xnamath.h>
#else
	#include <dxgi.h>
	#include <DirectXMath.h>
	
	using namespace DirectX;
#endif

#include <d3dcompiler.h>


#ifndef D3DCOMPILE_ENABLE_STRICTNESS
	#define D3DCOMPILE_ENABLE_STRICTNESS D3D10_SHADER_ENABLE_STRICTNESS
	#define D3DCOMPILE_DEBUG D3D10_SHADER_DEBUG
#endif

#endif //_WIN32

// Foot Data
//
static float sole[][3] = { {  0.00f, 0.0f, -0.70f },
						   {  0.04f, 0.0f, -0.69f },
						   {  0.09f, 0.0f, -0.65f },
						   {  0.13f, 0.0f, -0.61f },
						   {  0.16f, 0.0f, -0.54f },
						   {  0.17f, 0.0f, -0.46f },
						   {  0.17f, 0.0f, -0.35f },
						   {  0.16f, 0.0f, -0.25f },
						   {  0.15f, 0.0f, -0.14f },
						   {  0.13f, 0.0f,  0.00f },
						   {  0.00f, 0.0f,  0.00f },
						   { -0.13f, 0.0f,  0.00f },
						   { -0.15f, 0.0f, -0.14f },
						   { -0.16f, 0.0f, -0.25f },
						   { -0.17f, 0.0f, -0.35f },
						   { -0.17f, 0.0f, -0.46f },
						   { -0.16f, 0.0f, -0.54f },
						   { -0.13f, 0.0f, -0.61f },
						   { -0.09f, 0.0f, -0.65f },
						   { -0.04f, 0.0f, -0.69f },
						   { -0.00f, 0.0f, -0.70f } };
static float heel[][3] = { {  0.00f, 0.0f,  0.06f },
						   {  0.13f, 0.0f,  0.06f },
						   {  0.14f, 0.0f,  0.15f },
						   {  0.14f, 0.0f,  0.21f },
						   {  0.13f, 0.0f,  0.25f },
						   {  0.11f, 0.0f,  0.28f },
						   {  0.09f, 0.0f,  0.29f },
						   {  0.04f, 0.0f,  0.30f },
						   {  0.00f, 0.0f,  0.30f },
						   { -0.04f, 0.0f,  0.30f },
						   { -0.09f, 0.0f,  0.29f },
						   { -0.11f, 0.0f,  0.28f },
						   { -0.13f, 0.0f,  0.25f },
						   { -0.14f, 0.0f,  0.21f },
						   { -0.14f, 0.0f,  0.15f },
						   { -0.13f, 0.0f,  0.06f },
						   { -0.00f, 0.0f,  0.06f } };
static int soleCount = 21;
static int heelCount = 17;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Node implementation with standard viewport draw
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

class rawfootPrint : public MPxLocatorNode
{
public:
	rawfootPrint();
	virtual ~rawfootPrint();

    virtual MStatus   		compute( const MPlug& plug, MDataBlock& data );

	virtual void            draw( M3dView & view, const MDagPath & path,
								  M3dView::DisplayStyle style,
								  M3dView::DisplayStatus status );

	virtual bool            isBounded() const;
	virtual MBoundingBox    boundingBox() const;

	static  void *          creator();
	static  MStatus         initialize();

	static  MObject         size;         // The size of the foot

public:
	static	MTypeId		id;
	static	MString		drawDbClassification;
	static	MString		drawRegistrantId;
};

MObject rawfootPrint::size;
MTypeId rawfootPrint::id( 0x0008002D );
MString	rawfootPrint::drawDbClassification("drawdb/geometry/rawfootPrint");
MString	rawfootPrint::drawRegistrantId("RawFootprintNodePlugin");

rawfootPrint::rawfootPrint() {}
rawfootPrint::~rawfootPrint() {}

MStatus rawfootPrint::compute( const MPlug& /*plug*/, MDataBlock& /*data*/ )
{
	return MS::kUnknownParameter;
}

void rawfootPrint::draw( M3dView & view, const MDagPath & /*path*/,
							 M3dView::DisplayStyle style,
							 M3dView::DisplayStatus status )
{
	// Get the size
	//
	MObject thisNode = thisMObject();
	MPlug plug( thisNode, size );
	MDistance sizeVal;
	plug.getValue( sizeVal );

	float multiplier = (float) sizeVal.asCentimeters();

	view.beginGL();


	if ( ( style == M3dView::kFlatShaded ) ||
		 ( style == M3dView::kGouraudShaded ) )
	{
		// Push the color settings
		//
		glPushAttrib( GL_CURRENT_BIT );

		if ( status == M3dView::kActive ) {
			view.setDrawColor( 13, M3dView::kActiveColors );
		} else {
			view.setDrawColor( 13, M3dView::kDormantColors );
		}

		glBegin( GL_TRIANGLE_FAN );
	        int i;
			int last = soleCount - 1;
			for ( i = 0; i < last; ++i ) {
				glVertex3f( sole[i][0] * multiplier,
							sole[i][1] * multiplier,
							sole[i][2] * multiplier );
			}
		glEnd();
		glBegin( GL_TRIANGLE_FAN );
			last = heelCount - 1;
			for ( i = 0; i < last; ++i ) {
				glVertex3f( heel[i][0] * multiplier,
							heel[i][1] * multiplier,
							heel[i][2] * multiplier );
			}
		glEnd();

		glPopAttrib();
	}

	// Draw the outline of the foot
	//
	glBegin( GL_LINES );
	    int i;
	    int last = soleCount - 1;
	    for ( i = 0; i < last; ++i ) {
			glVertex3f( sole[i][0] * multiplier,
						sole[i][1] * multiplier,
						sole[i][2] * multiplier );
			glVertex3f( sole[i+1][0] * multiplier,
						sole[i+1][1] * multiplier,
						sole[i+1][2] * multiplier );
		}
		last = heelCount - 1;
	    for ( i = 0; i < last; ++i ) {
			glVertex3f( heel[i][0] * multiplier,
						heel[i][1] * multiplier,
						heel[i][2] * multiplier );
			glVertex3f( heel[i+1][0] * multiplier,
						heel[i+1][1] * multiplier,
						heel[i+1][2] * multiplier );
		}
	glEnd();


	view.endGL();

	// Draw the name of the rawfootPrint
	view.setDrawColor( MColor( 0.1f, 0.8f, 0.8f, 1.0f ) );
	view.drawText( MString("rawFootprint"), MPoint( 0.0, 0.0, 0.0 ), M3dView::kCenter );
}

bool rawfootPrint::isBounded() const
{
	return true;
}

MBoundingBox rawfootPrint::boundingBox() const
{
	// Get the size
	//
	MObject thisNode = thisMObject();
	MPlug plug( thisNode, size );
	MDistance sizeVal;
	plug.getValue( sizeVal );

	double multiplier = sizeVal.asCentimeters();

	MPoint corner1( -0.17, 0.0, -0.7 );
	MPoint corner2( 0.17, 0.0, 0.3 );

	corner1 = corner1 * multiplier;
	corner2 = corner2 * multiplier;

	return MBoundingBox( corner1, corner2 );
}

void* rawfootPrint::creator()
{
	return new rawfootPrint();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Viewport 2.0 override implementation
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

class RawFootPrintData : public MUserData
{
public:
	RawFootPrintData() : MUserData(false) {} // don't delete after draw
	virtual ~RawFootPrintData() {}

	float fMultiplier;
	float fColor[3];
	bool  fCustomBoxDraw;
	MBoundingBox fCurrentBoundingBox;
	MDAGDrawOverrideInfo fDrawOV;
};

// Helper class declaration for the object drawing
class RawFootPrintDrawAgent
{
public:
	RawFootPrintDrawAgent(){}
	virtual ~RawFootPrintDrawAgent(){}

	virtual void drawShaded( float multiplier ) = 0;
	virtual void drawBoundingBox( const MPoint&, const MPoint&) = 0;
	virtual void drawWireframe( float multiplier ) = 0;
	virtual void beginDraw() = 0;
	virtual void endDraw() = 0;

	void setMatrix( const MMatrix& wvMatrix, const MMatrix& projMatrix ){
		mWorldViewMatrix = wvMatrix;
		mProjectionMatrix = projMatrix;
	}

	void setColor( const MColor& color){
		mColor = color;
	}

protected:
	MMatrix mWorldViewMatrix;
	MMatrix mProjectionMatrix;
	MColor  mColor;
};

// GL draw agent declaration
class RawFootPrintDrawAgentGL : public RawFootPrintDrawAgent
{
public:
	static RawFootPrintDrawAgentGL& getDrawAgent(){
		static RawFootPrintDrawAgentGL obj;
		return obj;
	}

	virtual void drawShaded( float multiplier );
	virtual void drawBoundingBox( const MPoint& min, const MPoint& max );
	virtual void drawWireframe( float multiplier );
	virtual void beginDraw();
	virtual void endDraw();
private:
	RawFootPrintDrawAgentGL(){}
	~RawFootPrintDrawAgentGL(){}
	RawFootPrintDrawAgentGL( const RawFootPrintDrawAgentGL& v ){}
	RawFootPrintDrawAgentGL operator = (const RawFootPrintDrawAgentGL& v){ return *this; }
};

// DX stuff
#ifdef _WIN32

// DX draw agent declaration
class RawFootPrintDrawAgentDX : public RawFootPrintDrawAgent
{
public:
	static RawFootPrintDrawAgentDX& getDrawAgent(){
		static RawFootPrintDrawAgentDX obj;
		return obj;
	}

	virtual void drawShaded( float multiplier );
	virtual void drawBoundingBox( const MPoint& min, const MPoint& max );
	virtual void drawWireframe( float multiplier );
	virtual void beginDraw();
	virtual void endDraw();

	bool releaseDXResources();
private:
	ID3D11Device* mDevicePtr;
	ID3D11DeviceContext* mDeviceContextPtr;

	ID3D11Buffer* mBoundingboxVertexBufferPtr;
	ID3D11Buffer* mBoundingboxIndexBufferPtr;
	ID3D11Buffer* mSoleVertexBufferPtr;
	ID3D11Buffer* mHeelVertexBufferPtr;
	ID3D11Buffer* mSoleWireIndexBufferPtr;
	ID3D11Buffer* mSoleShadedIndexBufferPtr;
	ID3D11Buffer* mHeelWireIndexBufferPtr;
	ID3D11Buffer* mHeelShadedIndexBufferPtr;
	ID3D11Buffer* mConstantBufferPtr;
	ID3D11VertexShader* mVertexShaderPtr;
	ID3D11PixelShader* mPixelShaderPtr;
	ID3D11InputLayout* mVertexLayoutPtr;

	MString mEffectLocation;
	bool mEffectLoad;
	unsigned int mStride;
	unsigned int mOffset;
	struct ConstantBufferDef
	{
		XMMATRIX fWVP;
		XMFLOAT4 fMatColor;
	};
	bool initShadersDX();
	bool initBuffersDX();
	void setupConstantBuffer( const XMMATRIX& scale );
private:
	RawFootPrintDrawAgentDX():
		mDevicePtr(NULL), mDeviceContextPtr(NULL), mEffectLocation(""),
		mEffectLoad(false),
		mStride(sizeof(float)*3), mOffset(0),
		mBoundingboxVertexBufferPtr(NULL),
		mBoundingboxIndexBufferPtr(NULL),
		mSoleVertexBufferPtr(NULL),
		mHeelVertexBufferPtr(NULL),
		mSoleWireIndexBufferPtr(NULL),
		mSoleShadedIndexBufferPtr(NULL),
		mHeelWireIndexBufferPtr(NULL),
		mHeelShadedIndexBufferPtr(NULL),
		mConstantBufferPtr(NULL),
		mVertexShaderPtr(NULL),
		mPixelShaderPtr(NULL),
		mVertexLayoutPtr(NULL)
		{}
	~RawFootPrintDrawAgentDX(){}
	RawFootPrintDrawAgentDX( const RawFootPrintDrawAgentDX& v ){}
	RawFootPrintDrawAgentDX operator = (const RawFootPrintDrawAgentDX& v){ return *this; }
};

#endif // _WIN32

// GL draw agent definition
//
void RawFootPrintDrawAgentGL::beginDraw()
{
	// set world view matrix
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixd(mWorldViewMatrix.matrix[0]);
	// set projection matrix
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixd(mProjectionMatrix.matrix[0]);
	glPushAttrib( GL_CURRENT_BIT );
}

void RawFootPrintDrawAgentGL::endDraw()
{
	glPopAttrib();
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void RawFootPrintDrawAgentGL::drawShaded( float multiplier )
{
	// set color
	glColor4fv( &(mColor.r) );

	glBegin( GL_TRIANGLE_FAN );
	int i;
	int last = soleCount - 1;
	for ( i = 0; i < last; ++i ) {
		glVertex3f( sole[i][0] * multiplier,
			sole[i][1] * multiplier,
			sole[i][2] * multiplier );
	}
	glEnd();
	glBegin( GL_TRIANGLE_FAN );
	last = heelCount - 1;
	for ( i = 0; i < last; ++i ) {
		glVertex3f( heel[i][0] * multiplier,
			heel[i][1] * multiplier,
			heel[i][2] * multiplier );
	}
	glEnd();
}

void RawFootPrintDrawAgentGL::drawBoundingBox( const MPoint& min, const MPoint& max )
{
	// set color
	glColor4fv( &(mColor.r) );

	float bottomLeftFront[3] = { (float)min[0], (float)min[1], (float)min[2] };
	float topLeftFront[3] = { (float)min[0], (float)max[1], (float)min[2] }; //1
	float bottomRightFront[3] = { (float)max[0], (float)min[1], (float)min[2] }; //2
	float topRightFront[3] = { (float)max[0], (float)max[1], (float)min[2] }; //3
	float bottomLeftBack[3] = { (float)min[0], (float)min[1], (float)max[2] }; //4
	float topLeftBack[3] = { (float)min[0], (float)max[1], (float)max[2] }; //5
	float bottomRightBack[3] = { (float)max[0], (float)min[1], (float)max[2] }; //6
	float topRightBack[3] = { (float)max[0], (float)max[1], (float)max[2] }; //7

	glBegin( GL_LINES );

	// 4 Bottom lines
	//
	glVertex3fv( bottomLeftFront );
	glVertex3fv( bottomRightFront );
	glVertex3fv( bottomRightFront );
	glVertex3fv( bottomRightBack );
	glVertex3fv( bottomRightBack );
	glVertex3fv( bottomLeftBack );
	glVertex3fv( bottomLeftBack );
	glVertex3fv( bottomLeftFront );

	// 4 Top lines
	//
	glVertex3fv( topLeftFront );
	glVertex3fv( topRightFront );
	glVertex3fv( topRightFront );
	glVertex3fv( topRightBack );
	glVertex3fv( topRightBack );
	glVertex3fv( topLeftBack );
	glVertex3fv( topLeftBack );
	glVertex3fv( topLeftFront );

	// 4 Side lines
	//
	glVertex3fv( bottomLeftFront );
	glVertex3fv( topLeftFront );
	glVertex3fv( bottomRightFront );
	glVertex3fv( topRightFront );
	glVertex3fv( bottomRightBack );
	glVertex3fv( topRightBack );
	glVertex3fv( bottomLeftBack );
	glVertex3fv( topLeftBack );

	glEnd();
}

void RawFootPrintDrawAgentGL::drawWireframe( float multiplier )
{
	// set color
	glColor4fv( &(mColor.r) );

	// draw wire
	glBegin( GL_LINES );
	int i;
	int last = soleCount - 1;
	for ( i = 0; i < last; ++i ) {
		glVertex3f( sole[i][0] * multiplier,
			sole[i][1] * multiplier,
			sole[i][2] * multiplier );
		glVertex3f( sole[i+1][0] * multiplier,
			sole[i+1][1] * multiplier,
			sole[i+1][2] * multiplier );
	}
	last = heelCount - 1;
	for ( i = 0; i < last; ++i ) {
		glVertex3f( heel[i][0] * multiplier,
			heel[i][1] * multiplier,
			heel[i][2] * multiplier );
		glVertex3f( heel[i+1][0] * multiplier,
			heel[i+1][1] * multiplier,
			heel[i+1][2] * multiplier );
	}
	glEnd();
}

// DX stuff
#ifdef _WIN32
// DX draw agent definition
//
void RawFootPrintDrawAgentDX::beginDraw()
{
	// Please move file "rawfootprint.hlsl" to following location, or 
	// change following location as your local path.
	mEffectLocation = 
		MString(getenv("MAYA_LOCATION")) +
		MString("\\devkit\\plug-ins") +
		MString("\\rawfootprint.hlsl");

	MFileObject fileObj;
	fileObj.setRawFullName(mEffectLocation);
	assert(fileObj.exists());
	if (!fileObj.exists())
	{
		MGlobal::displayWarning("Can not find file:" +  mEffectLocation);
		mEffectLoad = false;
		return;
	}
	mEffectLoad = true;

	// Initial device
	if( !mDevicePtr || !mDeviceContextPtr ){
		// get renderer
		MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
		if ( theRenderer ){
			mDevicePtr = (ID3D11Device*)theRenderer->GPUDeviceHandle();
			if ( mDevicePtr ){
				mDevicePtr->GetImmediateContext( &mDeviceContextPtr );
			}
		}
	}
	assert( mDevicePtr );
	assert( mDeviceContextPtr );

	if ( mDevicePtr && mDeviceContextPtr ){

		// Init shaders
		if ( initShadersDX() ){
			// Set up shader
			mDeviceContextPtr->VSSetShader(mVertexShaderPtr, NULL, 0);
			mDeviceContextPtr->IASetInputLayout(mVertexLayoutPtr);
			mDeviceContextPtr->PSSetShader(mPixelShaderPtr, NULL, 0);
		}

		// Init buffers
		initBuffersDX();
	}
}

void RawFootPrintDrawAgentDX::endDraw()
{}

void RawFootPrintDrawAgentDX::setupConstantBuffer( const XMMATRIX& scale )
{
	assert( mDeviceContextPtr );
	if ( !mDeviceContextPtr )
		return;

	// Compute matrix
	XMMATRIX dxTransform = XMMATRIX(
		(float)mWorldViewMatrix.matrix[0][0], (float)mWorldViewMatrix.matrix[0][1], (float)mWorldViewMatrix.matrix[0][2], (float)mWorldViewMatrix.matrix[0][3],
		(float)mWorldViewMatrix.matrix[1][0], (float)mWorldViewMatrix.matrix[1][1], (float)mWorldViewMatrix.matrix[1][2], (float)mWorldViewMatrix.matrix[1][3],
		(float)mWorldViewMatrix.matrix[2][0], (float)mWorldViewMatrix.matrix[2][1], (float)mWorldViewMatrix.matrix[2][2], (float)mWorldViewMatrix.matrix[2][3],
		(float)mWorldViewMatrix.matrix[3][0], (float)mWorldViewMatrix.matrix[3][1], (float)mWorldViewMatrix.matrix[3][2], (float)mWorldViewMatrix.matrix[3][3]);
	XMMATRIX dxProjection = XMMATRIX(
		(float)mProjectionMatrix.matrix[0][0], (float)mProjectionMatrix.matrix[0][1], (float)mProjectionMatrix.matrix[0][2], (float)mProjectionMatrix.matrix[0][3],
		(float)mProjectionMatrix.matrix[1][0], (float)mProjectionMatrix.matrix[1][1], (float)mProjectionMatrix.matrix[1][2], (float)mProjectionMatrix.matrix[1][3],
		(float)mProjectionMatrix.matrix[2][0], (float)mProjectionMatrix.matrix[2][1], (float)mProjectionMatrix.matrix[2][2], (float)mProjectionMatrix.matrix[2][3],
		(float)mProjectionMatrix.matrix[3][0], (float)mProjectionMatrix.matrix[3][1], (float)mProjectionMatrix.matrix[3][2], (float)mProjectionMatrix.matrix[3][3]);

	// Set constant buffer
	ConstantBufferDef cb;
	cb.fWVP = XMMatrixTranspose(scale * dxTransform * dxProjection);
	cb.fMatColor = XMFLOAT4( mColor.r, mColor.g, mColor.b, mColor.a);
	mDeviceContextPtr->UpdateSubresource(mConstantBufferPtr, 0, NULL, &cb, 0, 0);
	mDeviceContextPtr->VSSetConstantBuffers(0, 1, &mConstantBufferPtr);
	mDeviceContextPtr->PSSetConstantBuffers(0, 1, &mConstantBufferPtr);
}

void RawFootPrintDrawAgentDX::drawShaded( float multiplier )
{
	assert( mDeviceContextPtr );
	if ( !(mDeviceContextPtr && mEffectLoad) )
		return;

	// Set constant buffer
	XMMATRIX scale(
		multiplier, 0.0f, 0.0f, 0.0f,
		0.0f, multiplier, 0.0f, 0.0f,
		0.0f, 0.0f,	multiplier, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f );
	setupConstantBuffer( scale );

	// Draw the sole
	mDeviceContextPtr->IASetVertexBuffers(0, 1, &mSoleVertexBufferPtr, &mStride, &mOffset);
	mDeviceContextPtr->IASetIndexBuffer(mSoleShadedIndexBufferPtr, DXGI_FORMAT_R16_UINT, 0);
	mDeviceContextPtr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mDeviceContextPtr->DrawIndexed(3 * (soleCount-2), 0, 0);
	// Draw the heel
	mDeviceContextPtr->IASetVertexBuffers(0, 1, &mHeelVertexBufferPtr, &mStride, &mOffset);
	mDeviceContextPtr->IASetIndexBuffer(mHeelShadedIndexBufferPtr, DXGI_FORMAT_R16_UINT, 0);
	mDeviceContextPtr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mDeviceContextPtr->DrawIndexed(3 * (heelCount-2), 0, 0);
}

void RawFootPrintDrawAgentDX::drawBoundingBox( const MPoint& min, const MPoint& max )
{
	assert( mDeviceContextPtr );
	if ( !(mDeviceContextPtr && mEffectLoad) )
		return;

	// Set constant buffer
	XMMATRIX scale(
		static_cast<float>(max[0]- min[0]), 0.0f, 0.0f, 0.0f,
		0.0f, static_cast<float>(max[1]-min[1]), 0.0f, 0.0f,
		0.0f, 0.0f,	static_cast<float>(max[2]-min[2]), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f );
	setupConstantBuffer( scale );

	mDeviceContextPtr->IASetVertexBuffers(0, 1, &mBoundingboxVertexBufferPtr, &mStride, &mOffset);
	mDeviceContextPtr->IASetIndexBuffer(mBoundingboxIndexBufferPtr, DXGI_FORMAT_R16_UINT, 0);
	mDeviceContextPtr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	mDeviceContextPtr->DrawIndexed(2 * 12, 0, 0);
}

void RawFootPrintDrawAgentDX::drawWireframe( float multiplier )
{
	assert( mDeviceContextPtr );
	if ( !(mDeviceContextPtr && mEffectLoad) )
		return;

	// Set constant buffer
	XMMATRIX scale(
		multiplier, 0.0f, 0.0f, 0.0f,
		0.0f, multiplier, 0.0f, 0.0f,
		0.0f, 0.0f,	multiplier, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f );
	setupConstantBuffer( scale );

	// Draw the sole
	mDeviceContextPtr->IASetVertexBuffers(0, 1, &mSoleVertexBufferPtr, &mStride, &mOffset);
	mDeviceContextPtr->IASetIndexBuffer(mSoleWireIndexBufferPtr, DXGI_FORMAT_R16_UINT, 0);
	mDeviceContextPtr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	mDeviceContextPtr->DrawIndexed(2 * (soleCount-1), 0, 0);
	// Draw the heel
	mDeviceContextPtr->IASetVertexBuffers(0, 1, &mHeelVertexBufferPtr, &mStride, &mOffset);
	mDeviceContextPtr->IASetIndexBuffer(mHeelWireIndexBufferPtr, DXGI_FORMAT_R16_UINT, 0);
	mDeviceContextPtr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	mDeviceContextPtr->DrawIndexed(2 * (heelCount-1), 0, 0);
}

#endif // _WIN32


class RawFootPrintDrawOverride : public MHWRender::MPxDrawOverride
{
public:
	static MHWRender::MPxDrawOverride* Creator(const MObject& obj)
	{
		return new RawFootPrintDrawOverride(obj);
	}

	virtual ~RawFootPrintDrawOverride();

	virtual MHWRender::DrawAPI supportedDrawAPIs() const;

	virtual bool isBounded(
		const MDagPath& objPath,
		const MDagPath& cameraPath) const;

	virtual MBoundingBox boundingBox(
		const MDagPath& objPath,
		const MDagPath& cameraPath) const;

	virtual bool disableInternalBoundingBoxDraw() const;

	virtual MUserData* prepareForDraw(
		const MDagPath& objPath,
		const MDagPath& cameraPath,
		const MHWRender::MFrameContext& frameContext,
		MUserData* oldData);

	virtual bool hasUIDrawables() const { return true; }

	virtual void addUIDrawables(
		const MDagPath& objPath,
		MHWRender::MUIDrawManager& drawManager,
		const MHWRender::MFrameContext& frameContext,
		const MUserData* data);

	static void draw(const MHWRender::MDrawContext& context, const MUserData* data);

protected:
	MBoundingBox mCurrentBoundingBox;
	bool		 mCustomBoxDraw;
private:
	RawFootPrintDrawOverride(const MObject& obj);
	float getMultiplier(const MDagPath& objPath) const;
};

RawFootPrintDrawOverride::RawFootPrintDrawOverride(const MObject& obj)
: MHWRender::MPxDrawOverride(obj, RawFootPrintDrawOverride::draw)
// We want to perform custom bounding box drawing
// so return true so that the internal rendering code
// will not draw it for us.
, mCustomBoxDraw(true)
{
}

RawFootPrintDrawOverride::~RawFootPrintDrawOverride()
{
}

MHWRender::DrawAPI RawFootPrintDrawOverride::supportedDrawAPIs() const
{
	// this plugin supports both GL and DX
	return (MHWRender::kOpenGL | MHWRender::kDirectX11);
}

float RawFootPrintDrawOverride::getMultiplier(const MDagPath& objPath) const
{
	// Retrieve value of the size attribute from the node
	MStatus status;
	MObject rawfootprintNode = objPath.node(&status);
	if (status)
	{
		MPlug plug(rawfootprintNode, rawfootPrint::size);
		if (!plug.isNull())
		{
			MDistance sizeVal;
			if (plug.getValue(sizeVal))
			{
				return (float)sizeVal.asCentimeters();
			}
		}
	}

	return 1.0f;
}

bool RawFootPrintDrawOverride::isBounded(const MDagPath& /*objPath*/,
                                      const MDagPath& /*cameraPath*/) const
{
	return true;
}

MBoundingBox RawFootPrintDrawOverride::boundingBox(
	const MDagPath& objPath,
	const MDagPath& cameraPath) const
{
	MPoint corner1( -0.17, 0.0, -0.7 );
	MPoint corner2( 0.17, 0.0, 0.3 );

	float multiplier = getMultiplier(objPath);
	corner1 = corner1 * multiplier;
	corner2 = corner2 * multiplier;

	RawFootPrintDrawOverride *nonConstThis = (RawFootPrintDrawOverride *)this;
	nonConstThis->mCurrentBoundingBox.clear();
	nonConstThis->mCurrentBoundingBox.expand( corner1 );
	nonConstThis->mCurrentBoundingBox.expand( corner2 );

	return mCurrentBoundingBox;
}

bool RawFootPrintDrawOverride::disableInternalBoundingBoxDraw() const
{
	return mCustomBoxDraw;
}

MUserData* RawFootPrintDrawOverride::prepareForDraw(
	const MDagPath& objPath,
	const MDagPath& cameraPath,
	const MHWRender::MFrameContext& frameContext,
	MUserData* oldData)
{
	// Retrieve data cache (create if does not exist)
	RawFootPrintData* data = dynamic_cast<RawFootPrintData*>(oldData);
	if (!data)
	{
		data = new RawFootPrintData();
	}

	// compute data and cache it
	data->fMultiplier = getMultiplier(objPath);
	MColor color = MHWRender::MGeometryUtilities::wireframeColor(objPath);
	data->fColor[0] = color.r;
	data->fColor[1] = color.g;
	data->fColor[2] = color.b;
	data->fCustomBoxDraw = mCustomBoxDraw;
	data->fCurrentBoundingBox = mCurrentBoundingBox;

	// Get the draw override information
	data->fDrawOV = objPath.getDrawOverrideInfo();

	return data;
}

void RawFootPrintDrawOverride::addUIDrawables(
		const MDagPath& objPath,
		MHWRender::MUIDrawManager& drawManager,
		const MHWRender::MFrameContext& frameContext,
		const MUserData* data)
{
	// Draw a text "Foot"
	MPoint pos( 0.0, 0.0, 0.0 ); // Position of the text
	MColor textColor( 0.1f, 0.8f, 0.8f, 1.0f ); // Text color

	drawManager.beginDrawable();

	drawManager.setColor( textColor );
	drawManager.setFontSize( MHWRender::MUIDrawManager::kSmallFontSize );
	drawManager.text( pos,  MString("rawFootprint"), MHWRender::MUIDrawManager::kCenter );

	drawManager.endDrawable();
}

void RawFootPrintDrawOverride::draw(const MHWRender::MDrawContext& context, const MUserData* data)
{
	// Get user draw data
	const RawFootPrintData* footData = dynamic_cast<const RawFootPrintData*>(data);
	if (!footData)
		return;

	// Get DAG object draw override
	const MDAGDrawOverrideInfo& objectOverrideInfo = footData->fDrawOV;

	// Sample code to determine the rendering destination
	bool debugDestination = false;
	if (debugDestination)
	{
		MString destinationIdentifier;
		MHWRender::MFrameContext::RenderingDestination dest = context.renderingDestination(destinationIdentifier);
		MString destinationType = " 3d viewport";
		bool found3dView = false;
		if (dest == MHWRender::MFrameContext::k3dViewport)
		{
			M3dView view;
			if (M3dView::getM3dViewFromModelPanel(destinationIdentifier, view) == MStatus::kSuccess)
			{
				found3dView = true;
			}
		}
		else if (dest == MHWRender::MFrameContext::k2dViewport)
			destinationType = " 2d viewport";
		else if (dest == MHWRender::MFrameContext::kImage)
			destinationType = "n image";
		printf("rawfootprint node render destination is a%s. Destination name=%s. Found M3dView=%d\n", 
			destinationType.asChar(), destinationIdentifier.asChar(), found3dView);
	}

	// Just return and draw nothing, if it is overridden invisible
	if ( objectOverrideInfo.fOverrideEnabled && !objectOverrideInfo.fEnableVisible )
		return;

	// Get display status
	const unsigned int displayStyle = context.getDisplayStyle();
	bool drawAsBoundingbox =
		(displayStyle & MHWRender::MFrameContext::kBoundingBox) ||
		(footData->fDrawOV.fLOD == MDAGDrawOverrideInfo::kLODBoundingBox);
	// If we don't want to draw the bounds within this plugin
	// manually, then skip drawing altogether in bounding box mode
	// since the bounds draw is handled by the renderer and
	// doesn't need to be drawn here.
	//
	if ( drawAsBoundingbox && !footData->fCustomBoxDraw )
	{
		return;
	}

	bool animPlay = MAnimControl::isPlaying();
	bool animScrub = MAnimControl::isScrubbing();
	// If in playback but hidden in playback, skip drawing
	if (!objectOverrideInfo.fPlaybackVisible &&
		(animPlay || animScrub))
	{
		return;
	}	
	// For any viewport interactions switch to bounding box mode,
	// except when we are in playback.
	if (context.inUserInteraction() || context.userChangingViewContext())
	{
		if (!animPlay && !animScrub)
			drawAsBoundingbox = true;
	}

	// Now, something gonna draw...

	// Get renderer
	MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
	if (!theRenderer)
		return;

	// Get  world view matrix
	MStatus status;
	const MMatrix transform =
		context.getMatrix(MHWRender::MFrameContext::kWorldViewMtx, &status);
	if (status != MStatus::kSuccess) return;
	// Get projection matrix
	const MMatrix projection =
		context.getMatrix(MHWRender::MFrameContext::kProjectionMtx, &status);
	if (status != MStatus::kSuccess) return;

	// Check to see if we are drawing in a shadow pass.
	// If so then we keep the shading simple which in this
	// example means to disable any extra blending state changes
	//
	const MHWRender::MPassContext & passCtx = context.getPassContext();
	const MStringArray & passSem = passCtx.passSemantics();
	bool castingShadows = false;
	for (unsigned int i=0; i<passSem.length(); i++)
	{
		if (passSem[i] == MHWRender::MPassContext::kShadowPassSemantic)
			castingShadows = true;
	}
	bool debugPassInformation = false;
	if (debugPassInformation)
	{
		const MString & passId = passCtx.passIdentifier();
		printf("rawfootprint node drawing in pass[%s], semantic[", passId.asChar());
		for (unsigned int i=0; i<passSem.length(); i++)
			printf(" %s", passSem[i].asChar());
		printf("\n");
	}

	// get cached data
	float multiplier = footData->fMultiplier;
	float color[4] = {
			footData->fColor[0],
			footData->fColor[1],
			footData->fColor[2],
			1.0f
	};

	bool requireBlending = false;

	// If we're not casting shadows then do extra work
	// for display styles
	if (!castingShadows)
	{

		// Use some monotone version of color to show "default material mode"
		//
		if (displayStyle & MHWRender::MFrameContext::kDefaultMaterial)
		{
			color[0] = color[1] = color[2] = (color[0] + color[1] + color[2]) / 3.0f;
		}
		// Do some alpha blending if in x-ray mode
		//
		else if (displayStyle & MHWRender::MFrameContext::kXray)
		{
			requireBlending = true;
			color[3] = 0.3f;
		}
	}

	// Set blend and raster state
	//
	MHWRender::MStateManager* stateMgr = context.getStateManager();
	const MHWRender::MBlendState* pOldBlendState = NULL;
	const MHWRender::MRasterizerState* pOldRasterState = NULL;
	bool rasterStateModified = false;

	if(stateMgr && (displayStyle & MHWRender::MFrameContext::kGouraudShaded))
	{
		// draw filled, and with blending if required
		if (stateMgr && requireBlending)
		{
			static const MHWRender::MBlendState* blendState = NULL;
			if (!blendState)
			{
				MHWRender::MBlendStateDesc desc;
				desc.targetBlends[0].blendEnable = true;
				desc.targetBlends[0].destinationBlend = MHWRender::MBlendState::kInvSourceAlpha;
				desc.targetBlends[0].alphaDestinationBlend = MHWRender::MBlendState::kInvSourceAlpha;
				blendState = stateMgr->acquireBlendState(desc);
			}

			if (blendState)
			{
				pOldBlendState = stateMgr->getBlendState();
				stateMgr->setBlendState(blendState);
				stateMgr->releaseBlendState(blendState);
				blendState = NULL;
			}
		}

		// Override culling mode since we always want double-sided
		//
		pOldRasterState = stateMgr ? stateMgr->getRasterizerState() : NULL;
		if (pOldRasterState)
		{
			MHWRender::MRasterizerStateDesc desc( pOldRasterState->desc() );
			// It's also possible to change this to kCullFront or kCullBack if we
			// wanted to set it to that.
			MHWRender::MRasterizerState::CullMode cullMode = MHWRender::MRasterizerState::kCullNone;
			if (desc.cullMode != cullMode)
			{
				static const MHWRender::MRasterizerState *rasterState = NULL;
				if (!rasterState)
				{
					// Just override the cullmode
					desc.cullMode = cullMode;
					rasterState = stateMgr->acquireRasterizerState(desc);
				}
				if (rasterState)
				{
					rasterStateModified = true;
					stateMgr->setRasterizerState(rasterState);
					stateMgr->releaseRasterizerState(rasterState);
					rasterState = NULL;
				}
			}
		}
	}

	//========================
	// Start the draw work
	//========================

	// Prepare draw agent, default using OpenGL
	RawFootPrintDrawAgentGL& drawAgentRef = RawFootPrintDrawAgentGL::getDrawAgent();
	RawFootPrintDrawAgent* drawAgentPtr = &drawAgentRef;
#ifdef _WIN32
	// DX Draw
	if ( !theRenderer->drawAPIIsOpenGL() )
	{
		RawFootPrintDrawAgentDX& drawAgentRef = RawFootPrintDrawAgentDX::getDrawAgent();
		drawAgentPtr = &drawAgentRef;
	}
#endif
	assert( drawAgentPtr );

	if ( drawAgentPtr ){

		// Set color
		drawAgentPtr->setColor( MColor(color[0],color[1],color[2],color[3]) );
		// Set matrix
		drawAgentPtr->setMatrix( transform, projection );

		drawAgentPtr->beginDraw();

		if ( drawAsBoundingbox )
		{
			// If it is in bounding bode, draw only bounding box wireframe, nothing else
			MPoint min = footData->fCurrentBoundingBox.min();
			MPoint max = footData->fCurrentBoundingBox.max();

			drawAgentPtr->drawBoundingBox( min, max );
		} else {
			// Templated, only draw wirefame and it is not selectale
			const bool overideTemplated = objectOverrideInfo.fOverrideEnabled &&
				(objectOverrideInfo.fDisplayType == MDAGDrawOverrideInfo::kDisplayTypeTemplate);
			// Override no shaded, only show wireframe
			const bool overrideNoShaded = objectOverrideInfo.fOverrideEnabled && !objectOverrideInfo.fEnableShading;

			if ( overideTemplated || overrideNoShaded ){
				drawAgentPtr->drawWireframe( multiplier );
			} else {
				if( (displayStyle & MHWRender::MFrameContext::kGouraudShaded) ||
					(displayStyle & MHWRender::MFrameContext::kTextured))
				{
					drawAgentPtr->drawShaded( multiplier );
				}

				if(displayStyle & MHWRender::MFrameContext::kWireFrame)
				{
					drawAgentPtr->drawWireframe( multiplier );
				}
			}
		}

		drawAgentPtr->endDraw();
	}

	//========================
	// End the draw work
	//========================

	// Restore old blend state and old raster state
	if(stateMgr && (displayStyle & MHWRender::MFrameContext::kGouraudShaded))
	{
		if (stateMgr && pOldBlendState)
		{
			stateMgr->setBlendState(pOldBlendState);
            stateMgr->releaseBlendState(pOldBlendState);
		}
		if (rasterStateModified && pOldRasterState)
		{
			stateMgr->setRasterizerState(pOldRasterState);
            stateMgr->releaseRasterizerState(pOldRasterState);
		}
	}
}

// DX stuff
#ifdef _WIN32

bool RawFootPrintDrawAgentDX::initShadersDX()
{
	assert( mDevicePtr );
	if ( !mDevicePtr )
		return false;

	HRESULT hr;
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
	ID3DBlob* vsBlob = NULL;
	ID3DBlob* psBlob = NULL;
	ID3DBlob* pErrorBlob;


	// VS
	if (!mVertexShaderPtr)
	{
#if _MSC_VER < 1700
		hr = D3DX11CompileFromFile(
			mEffectLocation.asChar(),
			NULL,
			NULL,
			"mainVS",
			"vs_5_0",
			dwShaderFlags,
			0,
			NULL,
			&vsBlob,
			&pErrorBlob,
			NULL);
#else
		hr = D3DCompileFromFile(
			mEffectLocation.asWChar(),
			NULL,
			NULL,
			"mainVS",
			"vs_5_0",
			dwShaderFlags,
			0,
			&vsBlob,
			&pErrorBlob);
#endif
		if (FAILED(hr))
		{
			printf("Failed to compile vertex shader\n");
			if (pErrorBlob) pErrorBlob->Release();
			return false;
		}
		if (pErrorBlob) pErrorBlob->Release();
		hr = mDevicePtr->CreateVertexShader(
			vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &mVertexShaderPtr);
		if (FAILED(hr))
		{
			printf("Failed to create vertex shader\n");
			vsBlob->Release();
			return false;
		}
	}

	// Layout
	if (!mVertexLayoutPtr)
	{
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		int numLayoutElements = sizeof layout/sizeof layout[0];
		hr = mDevicePtr->CreateInputLayout(
			layout, numLayoutElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &mVertexLayoutPtr);
		vsBlob->Release();
		if (FAILED(hr))
		{
			printf("Failed to create input layout\n");
			return false;
		}
	}

	// PS
	if (!mPixelShaderPtr)
	{
#if _MSC_VER < 1700
		hr = D3DX11CompileFromFile(
			mEffectLocation.asChar(),
			NULL,
			NULL,
			"mainPS",
			"ps_5_0",
			dwShaderFlags,
			0,
			NULL,
			&psBlob,
			&pErrorBlob,
			NULL);
#else
		hr = D3DCompileFromFile(
			mEffectLocation.asWChar(),
			NULL,
			NULL,
			"mainPS",
			"ps_5_0",
			dwShaderFlags,
			0,
			&psBlob,
			&pErrorBlob);
#endif
		if (FAILED(hr))
		{
			printf("Failed to compile vertex shader\n");
			mVertexShaderPtr->Release();
			mVertexLayoutPtr->Release();
			if (pErrorBlob) pErrorBlob->Release();
			return false;
		}
		if (pErrorBlob) pErrorBlob->Release();
		hr = mDevicePtr->CreatePixelShader(
			psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &mPixelShaderPtr);
		psBlob->Release();
		if (FAILED(hr))
		{
			printf("Failed to create pixel shader\n");
			mVertexShaderPtr->Release();
			mVertexLayoutPtr->Release();
			return false;
		}
	}

	return true;
}

bool RawFootPrintDrawAgentDX::initBuffersDX()
{
	assert( mDevicePtr );
	if ( !mDevicePtr )
		return false;

	HRESULT hr;
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));


	if (!mBoundingboxVertexBufferPtr)
	{
		// 8 Vertices for drawing the bounding box in DX mode
		float bbData[][3] = {
			{ -0.5, -0.5f, -0.5f},
			{  0.5, -0.5f, -0.5f},
			{  0.5, -0.5f,  0.5f},
			{ -0.5, -0.5f,  0.5f},
			{ -0.5,  0.5f, -0.5f},
			{  0.5,  0.5f, -0.5f},
			{  0.5,  0.5f,  0.5f},
			{ -0.5,  0.5f,  0.5f}};

		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(float) * 3 * 8;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = bbData;
		hr = mDevicePtr->CreateBuffer(&bd, &InitData, &mBoundingboxVertexBufferPtr);
		if (FAILED(hr)) return false;
	}
	if( !mBoundingboxIndexBufferPtr ){
		unsigned short bbWireIndices[] =
		{
			0,1,
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
			3,7
		};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(unsigned short) * 2 * 12;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = bbWireIndices;
		hr = mDevicePtr->CreateBuffer(&bd, &InitData, &mBoundingboxIndexBufferPtr);
		if (FAILED(hr)) return false;
	}


	if (!mSoleVertexBufferPtr)
	{
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(float) * 3 * soleCount;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = sole;
		hr = mDevicePtr->CreateBuffer(&bd, &InitData, &mSoleVertexBufferPtr);
		if (FAILED(hr)) return false;
	}
	if (!mHeelVertexBufferPtr)
	{
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(float) * 3 * heelCount;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = heel;
		hr = mDevicePtr->CreateBuffer(&bd, &InitData, &mHeelVertexBufferPtr);
		if (FAILED(hr)) return false;
	}
	if (!mSoleWireIndexBufferPtr)
	{
		unsigned short soleWireIndices[] =
		{
			0, 1,
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
			19, 20
		};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(unsigned short) * 2 * (soleCount-1);
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = soleWireIndices;
		hr = mDevicePtr->CreateBuffer(&bd, &InitData, &mSoleWireIndexBufferPtr);
		if (FAILED(hr)) return false;
	}
	if (!mHeelWireIndexBufferPtr)
	{
		unsigned short heelWireIndices[] =
		{
			0, 1,
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
			15, 16
		};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(unsigned short) * 2 * (heelCount-1);
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = heelWireIndices;
		hr = mDevicePtr->CreateBuffer(&bd, &InitData, &mHeelWireIndexBufferPtr);
		if (FAILED(hr)) return false;
	}
	if (!mSoleShadedIndexBufferPtr)
	{
		unsigned short soleShadedIndices[] =
		{
			0, 1, 2,
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
			0, 19, 20
		};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(unsigned short) * 3 * (soleCount-2);
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = soleShadedIndices;
		hr = mDevicePtr->CreateBuffer(&bd, &InitData, &mSoleShadedIndexBufferPtr);
		if (FAILED(hr)) return false;
	}
	if (!mHeelShadedIndexBufferPtr)
	{
		unsigned short heelShadedIndices[] =
		{
			0, 1, 2,
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
			0, 15, 16
		};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(unsigned short) * 3 * (heelCount-2);
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = heelShadedIndices;
		hr = mDevicePtr->CreateBuffer(&bd, &InitData, &mHeelShadedIndexBufferPtr);
		if (FAILED(hr)) return false;
	}
	if (!mConstantBufferPtr)
	{
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(ConstantBufferDef);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		hr = mDevicePtr->CreateBuffer(&bd, NULL, &mConstantBufferPtr);
		if (FAILED(hr)) return false;
	}

	return true;
}

bool RawFootPrintDrawAgentDX::releaseDXResources()
{
	if (mBoundingboxVertexBufferPtr)
	{
		mBoundingboxVertexBufferPtr->Release();
		mBoundingboxVertexBufferPtr = NULL;
	}
	if (mBoundingboxIndexBufferPtr)
	{
		mBoundingboxIndexBufferPtr->Release();
		mBoundingboxIndexBufferPtr = NULL;
	}

	if (mSoleVertexBufferPtr)
	{
		mSoleVertexBufferPtr->Release();
		mSoleVertexBufferPtr = NULL;
	}
	if (mHeelVertexBufferPtr)
	{
		mHeelVertexBufferPtr->Release();
		mHeelVertexBufferPtr = NULL;
	}
	if (mSoleWireIndexBufferPtr)
	{
		mSoleWireIndexBufferPtr->Release();
		mSoleWireIndexBufferPtr = NULL;
	}
	if (mSoleShadedIndexBufferPtr)
	{
		mSoleShadedIndexBufferPtr->Release();
		mSoleShadedIndexBufferPtr = NULL;
	}
	if (mHeelWireIndexBufferPtr)
	{
		mHeelWireIndexBufferPtr->Release();
		mHeelWireIndexBufferPtr = NULL;
	}
	if (mHeelShadedIndexBufferPtr)
	{
		mHeelShadedIndexBufferPtr->Release();
		mHeelShadedIndexBufferPtr = NULL;
	}
	if (mVertexShaderPtr)
	{
		mVertexShaderPtr->Release();
		mVertexShaderPtr = NULL;
	}
	if (mPixelShaderPtr)
	{
		mPixelShaderPtr->Release();
		mPixelShaderPtr = NULL;
	}
	if (mVertexLayoutPtr)
	{
		mVertexLayoutPtr->Release();
		mVertexLayoutPtr = NULL;
	}
	if (mConstantBufferPtr)
	{
		mConstantBufferPtr->Release();
		mConstantBufferPtr = NULL;
	}

	return true;
}

#endif // _WIN32

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Plugin Registration
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

MStatus rawfootPrint::initialize()
{
	MFnUnitAttribute unitFn;
	MStatus			 stat;

	size = unitFn.create( "size", "sz", MFnUnitAttribute::kDistance );
	unitFn.setDefault( 1.0 );

	stat = addAttribute( size );
	if (!stat) {
		stat.perror("addAttribute");
		return stat;
	}

	return MS::kSuccess;
}

MStatus initializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");

	status = plugin.registerNode(
				"rawfootPrint",
				rawfootPrint::id,
				&rawfootPrint::creator,
				&rawfootPrint::initialize,
				MPxNode::kLocatorNode,
				&rawfootPrint::drawDbClassification);
	if (!status) {
		status.perror("registerNode");
		return status;
	}

    status = MHWRender::MDrawRegistry::registerDrawOverrideCreator(
		rawfootPrint::drawDbClassification,
		rawfootPrint::drawRegistrantId,
		RawFootPrintDrawOverride::Creator);
	if (!status) {
		status.perror("registerDrawOverrideCreator");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = MHWRender::MDrawRegistry::deregisterDrawOverrideCreator(
		rawfootPrint::drawDbClassification,
		rawfootPrint::drawRegistrantId);
	if (!status) {
		status.perror("deregisterDrawOverrideCreator");
		return status;
	}

	status = plugin.deregisterNode( rawfootPrint::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	// Release DX resource
#ifdef _WIN32
	RawFootPrintDrawAgentDX& drawAgentRef = RawFootPrintDrawAgentDX::getDrawAgent();
	drawAgentRef.releaseDXResources();
#endif

	return status;
}
