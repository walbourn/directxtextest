//--------------------------------------------------------------------------------------
// VertexShader.hlsl
//
// Simple vertex shader for rendering a textured quad
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//--------------------------------------------------------------------------------------

struct Vertex
{
    float4 position     : SV_Position;
    float2 texcoord     : TEXCOORD0;
};

struct Interpolants
{
    float4 position     : SV_Position;
    float2 texcoord     : TEXCOORD0;
};

Interpolants main( Vertex In )
{
    return In;
}