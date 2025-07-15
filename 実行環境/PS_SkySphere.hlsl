#include "Common.hlsli"


float4 main(PS_IN input) : SV_TARGET
{
	
    float4 color = g_Texture.Sample(g_SamplerState, input.uv);
    
    // ���s������K�p
    if (DLight.Enable)
    {
        color *= DLight.Diffuse;
    }
    else
    {
        color *= NightBright; // �Â�����
    }
    
    
    return color;
}