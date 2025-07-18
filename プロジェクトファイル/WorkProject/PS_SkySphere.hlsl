#include "Common.hlsli"


float4 main(PS_IN input) : SV_TARGET
{
	
    float4 color = g_Texture.Sample(g_SamplerState, input.uv);
    
    // 平行光源を適用
    if (DLight.Enable)
    {
        color *= DLight.Diffuse;
    }
    else
    {
        color *= NightBright; // 暗くする
    }
    
    
    return color;
}