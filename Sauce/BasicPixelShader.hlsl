//-----------------------------------------------------------------------------
// File: BasicPixelShader.hlsl
//
// ��{�I�ȃs�N�Z���V�F�[�_
//-----------------------------------------------------------------------------
#include "BasicShaderHeader.hlsli"

//�s�N�Z���V�F�[�_
float4 BasicPS(Output input) : SV_TARGET
{
    return float4(input.uv, 1.0f, 1.0f);
}