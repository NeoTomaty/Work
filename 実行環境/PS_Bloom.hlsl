#include "Common.hlsli"

float4 main(PS_IN input) : SV_TARGET
{
    float4 color = (0.0f, 0.0f, 0.0f, 1.0f);
    
    // ’Êí‚Ì•`‰æŒ‹‰Ê‚ğæ“¾
    float4 sceneColor = g_Texture.Sample(g_SamplerState, input.uv);

    // ‹P“xƒ}ƒbƒv‚ğæ“¾
    float4 bloomColor = g_LuminanceTex.Sample(g_SamplerState, input.uv);

    // ‰ÁZ‡¬
    color = sceneColor + bloomColor * bloomStrength;

    return color;
    
}
