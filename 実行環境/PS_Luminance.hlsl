#include "Common.hlsli"

float4 main(PS_IN input) : SV_TARGET
{
    // カラーサンプリング
    float4 color = g_Texture.Sample(g_SamplerState, input.uv);
	
	// サンプリングしたカラーから明るさを計算し、抽出する
    float4 t = dot(color.xyz, float3(0.215f, 0.7154f, 0.0721f));
	
	// 明るさ0.5f以下の場合は書き込みを行わない
    clip(t - 0.5f);
	
    return color;
    
}