#ifndef __hwRendererHelperDX_h__
#define __hwRendererHelperDX_h__

#ifdef _WIN32

#include "hwRendererHelper.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3DX11Effect;
struct ID3DX11EffectShaderResourceVariable;
struct ID3DX11EffectPass;
struct ID3D11InputLayout;
struct ID3D11Buffer;

#define MAX_VERTEX_BUFFERS 10


class hwRendererHelperDX : public hwRendererHelper
{
public:
	hwRendererHelperDX(MHWRender::MRenderer* renderer, ID3D11Device* device);
	virtual ~hwRendererHelperDX();
	
protected:
	virtual bool renderTextureToTarget(MHWRender::MTexture* texture, MHWRender::MRenderTarget *target);
	
private:
	void initializeDrawTextureEffect();
	
private:
	ID3D11Device*			fDXDevice;
	ID3D11DeviceContext*	fDXContext;
	
	ID3DX11Effect*			fDrawTextureEffect;
	ID3DX11EffectShaderResourceVariable* fDrawTextureShaderVariable;
	ID3DX11EffectPass*		fDrawTexturePass;
	ID3D11InputLayout*		fDrawTextureInputLayout;
	ID3D11Buffer*			fDrawTextureVertexBuffers[MAX_VERTEX_BUFFERS];
	unsigned int			fDrawTextureVertexBuffersStrides[MAX_VERTEX_BUFFERS];
	unsigned int			fDrawTextureVertexBuffersOffsets[MAX_VERTEX_BUFFERS];
	unsigned int			fDrawTextureVertexBuffersCount;
	ID3D11Buffer*			fDrawTextureIndexBuffer;
	unsigned int			fDrawTextureIndexBufferCount;
};

#endif

#endif __hwRendererHelperDX_h__

//-
// Copyright 2012 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+
