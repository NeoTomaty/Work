#include "Common.hlsli"

float4 main(PS_IN input) : SV_TARGET
{
	
    float alpha = g_Texture.Sample(g_SamplerState, input.uv).a;
	
	// アルファが小さい場合は影を作らない
    clip(alpha - 0.0001f);
	
	return float4(0.0f, 0.0f, 0.0f, 1.0f);
}