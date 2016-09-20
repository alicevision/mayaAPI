//**************************************************************************/
// Copyright (c) 2012 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/

RasterizerState WireFrame
{
    CullMode = None;
    FillMode = 2; //Wireframe
};

// World-view-projection transformation.
float4x4 gWVPXf : WorldViewProjection < string UIWidget = "None"; >;

// Vertex shader input structure.
struct VS_INPUT
{
    float3 Pos : customPositionStream;
    float3 Norm : customNormalStream;
};

// Vertex shader output structure.
struct VS_TO_PS
{
    float4 HPos : SV_Position;
    float4 Color : COLOR0;
};

// Vertex shader.
VS_TO_PS VS_d3d11Example(VS_INPUT In)
{
    VS_TO_PS Out;

    // Transform the position from world space to clip space for output.
    Out.HPos = mul(float4(In.Pos, 1.0f), gWVPXf);

    Out.Color = float4(In.Norm, 1.0f);

    return Out;
}

float4 PS_d3d11Example(VS_TO_PS In) : SV_Target
{
    return In.Color;
}

// The main technique.
technique11 Main
<
	string index_buffer_type = "customPrimitiveTest";
>
{
    pass P0
    {
        SetRasterizerState(WireFrame);
        SetVertexShader(CompileShader(vs_5_0, VS_d3d11Example()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_d3d11Example()));
    }
}
