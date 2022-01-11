//-----------------------------------------------------------------------------
// File: BasicPixelShader.hlsl
//
// 基本的なピクセルシェーダ
//-----------------------------------------------------------------------------
#include "BasicShaderHeader.hlsli"

//ピクセルシェーダ
float4 BasicPS(Output input) : SV_TARGET
{
    return float4(input.uv, 1.0f, 1.0f);
}