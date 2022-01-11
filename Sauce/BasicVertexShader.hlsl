//-----------------------------------------------------------------------------
// File: BasicVertexShader.hlsl
//
// 基本的な頂点シェーダ
//-----------------------------------------------------------------------------
#include "BasicShaderHeader.hlsli"

//頂点シェーダ
Output BasicVS(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    Output output;
    output.svpos = pos;
    output.uv = uv;
	
    return output;
}