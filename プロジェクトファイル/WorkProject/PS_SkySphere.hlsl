#include "Common.hlsli"


float4 main(PS_IN input) : SV_TARGET
{
	
    float4 color = g_Texture.Sample(g_SamplerState, input.uv);
    
    // •½sŒõŒ¹‚ğ“K—p
    if (DLight.Enable)
    {
        color *= DLight.Diffuse;
    }
    else
    {
        color *= NightBright; // ˆÃ‚­‚·‚é
    }
    
    
    return color;
}