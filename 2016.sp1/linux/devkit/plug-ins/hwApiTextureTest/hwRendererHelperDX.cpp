#ifdef _WIN32

#include "hwRendererHelperDX.h"
#include "hwApiTextureTestStrings.h"
#include <maya/MTextureManager.h>
#include <maya/MRenderTargetManager.h>
#include <maya/MGlobal.h>

#include <d3d11.h>

#if _MSC_VER < 1700
#include <d3dx11.h>
#endif

// To build against the DX SDK header use the following commented line
//#include <../Samples/C++/Effects11/Inc/d3dx11effect.h>
#include <maya/d3dx11effect.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>


hwRendererHelperDX::hwRendererHelperDX(MHWRender::MRenderer* renderer, ID3D11Device* device)
: hwRendererHelper(renderer)
, fDXDevice(device)
, fDXContext(NULL)
, fDrawTextureEffect(NULL)
, fDrawTextureShaderVariable(NULL)
, fDrawTexturePass(NULL)
, fDrawTextureInputLayout(NULL)
, fDrawTextureVertexBuffersCount(0)
, fDrawTextureIndexBuffer(NULL)
, fDrawTextureIndexBufferCount(0)
{
	if(fDXDevice != NULL)
	{
		fDXDevice->GetImmediateContext(&fDXContext);
		//fDXDevice->CreateDeferredContext(0, &fDXContext);
	}
}

hwRendererHelperDX::~hwRendererHelperDX()
{
	if(fDrawTextureIndexBuffer) fDrawTextureIndexBuffer->Release();
	for(unsigned int i = 0; i < fDrawTextureVertexBuffersCount; ++i)
	{
		if(fDrawTextureVertexBuffers[i])
			fDrawTextureVertexBuffers[i]->Release();
	}
	if(fDrawTextureInputLayout) fDrawTextureInputLayout->Release();
	if(fDrawTextureEffect) fDrawTextureEffect->Release();
	
	if(fDXContext) fDXContext->Release();
}

bool hwRendererHelperDX::renderTextureToTarget(MHWRender::MTexture* texture, MHWRender::MRenderTarget *target)
{
	if( fDXContext == NULL )
		return false;

	ID3D11ShaderResourceView* textureResourceView = (ID3D11ShaderResourceView*)texture->resourceHandle();
	if( textureResourceView == NULL )
		return false;

	ID3D11RenderTargetView* renderTargetView = (ID3D11RenderTargetView*)target->resourceHandle();
	if( renderTargetView == NULL )
		return false;

	MHWRender::MRenderTargetDescription targetDesc;
	target->targetDescription(targetDesc);

	initializeDrawTextureEffect();

	if( fDrawTexturePass == NULL || fDrawTextureShaderVariable == NULL || fDrawTextureInputLayout == NULL || fDrawTextureVertexBuffersCount == 0 || fDrawTextureIndexBuffer == NULL )
		return false;

	// Save original targets
	ID3D11RenderTargetView* oldRenderTargetView = NULL;
	ID3D11DepthStencilView* oldDepthStencilView = NULL;
	fDXContext->OMGetRenderTargets( 1, &oldRenderTargetView, &oldDepthStencilView );

	ID3D11DepthStencilView* depthStencilView = NULL;
	fDXContext->OMSetRenderTargets( 1, &renderTargetView, depthStencilView );

	if(renderTargetView)
	{
		float clearColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
 		fDXContext->ClearRenderTargetView( renderTargetView, clearColor );
	}

	// Save original viewport
	unsigned int numViewport = 1;
	D3D11_VIEWPORT oldViewport;
	fDXContext->RSGetViewports(&numViewport, &oldViewport);

	const D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (float)(targetDesc.width()), (float)(targetDesc.height()), 0.0f, 1.0f };
	fDXContext->RSSetViewports( 1, &viewport );

	// Bind vertex buffers
	fDXContext->IASetVertexBuffers(0, fDrawTextureVertexBuffersCount, fDrawTextureVertexBuffers, fDrawTextureVertexBuffersStrides, fDrawTextureVertexBuffersOffsets);
	fDXContext->IASetInputLayout(fDrawTextureInputLayout);

	// Bind index buffer
	fDXContext->IASetIndexBuffer(fDrawTextureIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	fDXContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Bind texture with effect variable
	fDrawTextureShaderVariable->SetResource( textureResourceView );

	// Activate pass
	fDrawTexturePass->Apply(0, fDXContext);

	// Draw quad
	fDXContext->DrawIndexed(fDrawTextureIndexBufferCount, 0, 0);

	// Restore targets
	fDXContext->OMSetRenderTargets( 1, &renderTargetView, depthStencilView );

	// Restore viewport
	fDXContext->RSSetViewports(1, &oldViewport);

	return true;
}

void hwRendererHelperDX::initializeDrawTextureEffect()
{
	if( fDXContext == NULL )
		return;

	// Create the effect
	if( fDrawTextureEffect == NULL )
	{
		const char* simpleShaderCode =	"Texture2D myTexture; \r\n" \

										"SamplerState SamplerLinearWrap \r\n" \
										"{ \r\n" \
										"	Filter = MIN_MAG_MIP_LINEAR; \r\n" \
										"	AddressU = Wrap; \r\n" \
										"	AddressV = Wrap; \r\n" \
										"}; \r\n" \

										"struct APP_TO_VS \r\n" \
										"{ \r\n" \
										"	float3 Pos : POSITION; \r\n" \
										"	float2 TextCoord : TEXTCOORD; \r\n" \
										"}; \r\n" \

										"struct VS_TO_PS \r\n" \
										"{ \r\n" \
										"	float4 Pos : SV_Position; \r\n" \
										"	float2 TextCoord : TEXTCOORD; \r\n" \
										"}; \r\n" \

										"VS_TO_PS BasicVS(APP_TO_VS IN) \r\n" \
										"{ \r\n" \
										"	VS_TO_PS OUT; \r\n" \
										"	OUT.Pos = float4(IN.Pos, 1.0f); \r\n" \
										"	OUT.TextCoord = IN.TextCoord; \r\n" \
										"	return OUT; \r\n" \
										"} \r\n" \

										"float4 BasicPS(VS_TO_PS IN) : SV_Target \r\n" \
										"{ \r\n" \
										"	float4 color = myTexture.Sample(SamplerLinearWrap, IN.TextCoord); \r\n" \
										"	return color; \r\n" \
										"} \r\n" \

										"technique10 simple \r\n" \
										"{ \r\n" \
										"	pass p0 \r\n" \
										"	{ \r\n" \
										"		SetVertexShader( CompileShader( vs_4_0, BasicVS() ) ); \r\n" \
										"		SetGeometryShader( NULL ); \r\n" \
										"		SetPixelShader( CompileShader( ps_4_0, BasicPS() ) ); \r\n" \
										"	} \r\n" \
										"} \r\n";
		const unsigned int simpleShaderLength = (unsigned int)strlen(simpleShaderCode);

		const D3D10_SHADER_MACRO macros[] = { { "DIRECT3D_VERSION", "0xb00" }, { NULL, NULL } };

#ifdef _DEBUG
		const unsigned int flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		const unsigned int flags = 0;
#endif

		ID3DBlob *shader = NULL;
		ID3DBlob *error = NULL;
#if _MSC_VER >= 1700
		HRESULT hr = D3DCompile((char*)simpleShaderCode, simpleShaderLength, NULL, macros, NULL, "", "fx_5_0", flags, 0, &shader, &error);
#else
		HRESULT hr = D3DX11CompileFromMemory((char*)simpleShaderCode, simpleShaderLength, NULL, macros, NULL, "", "fx_5_0", flags, 0, NULL, &shader, &error, NULL);
#endif
		if( SUCCEEDED( hr ) && shader != NULL )
		{
			hr = D3DX11CreateEffectFromMemory(shader->GetBufferPointer(), shader->GetBufferSize(), 0, fDXDevice, &fDrawTextureEffect);
			if( SUCCEEDED( hr ) && fDrawTextureEffect != NULL )
			{
				ID3DX11EffectVariable* textureVariable = fDrawTextureEffect->GetVariableByName( "myTexture" );
				if( textureVariable != NULL )
				{
					fDrawTextureShaderVariable = textureVariable->AsShaderResource();
				}

				ID3DX11EffectTechnique* technique = fDrawTextureEffect->GetTechniqueByIndex(0);
				if( technique )
				{
					fDrawTexturePass = technique->GetPassByIndex(0);
				}
			}
		}
		else
		{
			MString errorStr( (error && error->GetBufferSize() > 0) ? (char*) error->GetBufferPointer() : (char*) NULL );

			MGlobal::displayWarning( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kDxErrorEffect, errorStr ) );
		}
		
		if( shader != NULL )
		{
			shader->Release();
		}
	}
	
	// Create the vertex buffers and the input layout
	if( fDrawTextureInputLayout == NULL && fDrawTexturePass != NULL )
	{
		fDrawTextureVertexBuffersCount = 0;

		D3D11_INPUT_ELEMENT_DESC inputDesc[MAX_VERTEX_BUFFERS];

		HRESULT hr;

		// Create the position stream
		{
			const float position[] = {	-1, -1, 0,		// bottom-left
										-1,  1, 0,		// top-left
										 1,  1, 0,		// top-right
										 1, -1, 0 };	// bottom-right

			const D3D11_BUFFER_DESC bufDesc = { sizeof(position), D3D11_USAGE_IMMUTABLE, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };
			const D3D11_SUBRESOURCE_DATA bufData = { position, 0, 0 };
			ID3D11Buffer *buffer = NULL;
			hr = fDXDevice->CreateBuffer(&bufDesc, &bufData, &buffer);
			if( SUCCEEDED( hr ) && buffer != NULL )
			{
				inputDesc[fDrawTextureVertexBuffersCount].SemanticName = "POSITION";
				inputDesc[fDrawTextureVertexBuffersCount].SemanticIndex = 0;
				inputDesc[fDrawTextureVertexBuffersCount].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				inputDesc[fDrawTextureVertexBuffersCount].InputSlot = fDrawTextureVertexBuffersCount;
				inputDesc[fDrawTextureVertexBuffersCount].AlignedByteOffset = 0;
				inputDesc[fDrawTextureVertexBuffersCount].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				inputDesc[fDrawTextureVertexBuffersCount].InstanceDataStepRate = 0;

				fDrawTextureVertexBuffers[fDrawTextureVertexBuffersCount] = buffer;
				fDrawTextureVertexBuffersStrides[fDrawTextureVertexBuffersCount] = 3 * sizeof(float);
				fDrawTextureVertexBuffersOffsets[fDrawTextureVertexBuffersCount] = 0;

				++fDrawTextureVertexBuffersCount;
			}
		}

		// Create the texture coord stream
		if( SUCCEEDED( hr ) )
		{
			const float textCoord[] = {	0, 1,		// bottom-left
										0, 0,		// top-left
										1, 0,		// top-right
										1, 1 };		// bottom-right

			const D3D11_BUFFER_DESC bufDesc = { sizeof(textCoord), D3D11_USAGE_IMMUTABLE, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };
			const D3D11_SUBRESOURCE_DATA bufData = { textCoord, 0, 0 };
			ID3D11Buffer *buffer = NULL;
			hr = fDXDevice->CreateBuffer(&bufDesc, &bufData, &buffer);
			if( SUCCEEDED( hr ) && buffer != NULL )
			{
				inputDesc[fDrawTextureVertexBuffersCount].SemanticName = "TEXTCOORD";
				inputDesc[fDrawTextureVertexBuffersCount].SemanticIndex = 0;
				inputDesc[fDrawTextureVertexBuffersCount].Format = DXGI_FORMAT_R32G32_FLOAT;
				inputDesc[fDrawTextureVertexBuffersCount].InputSlot = fDrawTextureVertexBuffersCount;
				inputDesc[fDrawTextureVertexBuffersCount].AlignedByteOffset = 0;
				inputDesc[fDrawTextureVertexBuffersCount].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				inputDesc[fDrawTextureVertexBuffersCount].InstanceDataStepRate = 0;

				fDrawTextureVertexBuffers[fDrawTextureVertexBuffersCount] = buffer;
				fDrawTextureVertexBuffersStrides[fDrawTextureVertexBuffersCount] = 2 * sizeof(float);
				fDrawTextureVertexBuffersOffsets[fDrawTextureVertexBuffersCount] = 0;

				++fDrawTextureVertexBuffersCount;
			}
		}

		if( SUCCEEDED( hr ) )
		{
			D3DX11_PASS_DESC descPass;
			fDrawTexturePass->GetDesc(&descPass);

			hr = fDXDevice->CreateInputLayout(inputDesc, fDrawTextureVertexBuffersCount, descPass.pIAInputSignature, descPass.IAInputSignatureSize, &fDrawTextureInputLayout);
			if( FAILED( hr ) )
			{
				MGlobal::displayWarning( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kDxErrorInputLayout ) );
			}
		}
	}
	
	// Create the index buffer
	if( fDrawTextureIndexBuffer == NULL && fDrawTextureVertexBuffersCount > 0  && fDrawTextureInputLayout != NULL )
	{
		const unsigned int indices[] = { 0, 1, 3, 3, 2, 1 };
		fDrawTextureIndexBufferCount = _countof(indices);

		const D3D11_BUFFER_DESC bufDesc = { sizeof(indices), D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0, 0, 0 };
		const D3D11_SUBRESOURCE_DATA bufData = { indices, 0, 0 };
		fDXDevice->CreateBuffer(&bufDesc, &bufData, &fDrawTextureIndexBuffer);
	}
}

#endif

//-
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+
