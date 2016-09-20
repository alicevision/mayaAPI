//**************************************************************************/
// Copyright 2011 Autodesk, Inc. 
// All rights reserved.
// Use of this software is subject to the terms of the Autodesk license 
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form. 
//**************************************************************************/

// World-view-projection transformation.
float4x4 gWVPXf : WorldViewProjection < string UIWidget = "None"; >;

// Vertex shader input structure.
struct VS_INPUT
{
    float4 Pos : POSITION;
	float4 Norm : NORMAL;

    // this is the custom stream
    // the vertex buffer provider will fill in the stream
    // when a matching custom semantic is found
	float4 CustomStream : myCustomStreamB;
	//int4 CustomStream : myCustomStreamB;
};

// Vertex shader output structure.
struct VS_TO_PS
{
    // The vertex position in clip space.
    float4 HPos : SV_Position;

    // Color.
    float4 Color : COLOR0;
};

// Vertex shader.
VS_TO_PS VS_d3d11Example(VS_INPUT In)
{
    VS_TO_PS Out;

    float4 HPm = In.Pos;

    // Transform the position from world space to clip space for output.
    Out.HPos = mul(HPm, gWVPXf);

	Out.Color = In.Norm;

	// here we use our custom stream data to affect the output. 
	Out.Color = In.CustomStream;
	//Out.Color = float4(In.CustomStream.x / 255.0f, In.CustomStream.y / 255.0f, In.CustomStream.z / 255.0f, In.CustomStream.w / 255.0f);

    return Out;
}


float4 PS_d3d11Example(VS_TO_PS In) : SV_Target
{
    return In.Color;
}

// The main technique.
technique11 Main
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS_d3d11Example()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_d3d11Example()));
    }
}
