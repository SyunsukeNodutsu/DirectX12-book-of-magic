//-----------------------------------------------------------------------------
// File: BasicVertexShader.hlsl
//
// ��{�I�Ȓ��_�V�F�[�_
//-----------------------------------------------------------------------------
#include "BasicShaderHeader.hlsli"

//���_�V�F�[�_
Output BasicVS(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    Output output;
    output.svpos = mul(mat, pos);
    output.uv = uv;
	
    return output;
}