#include "Common.hlsli"

float3 ReinhardToneMap(float3 color)
{
    return color / (1.0 + color);
}

float4 main(PS_IN input) : SV_Target
{
    float3 hdrColor = g_Texture.Sample(g_SamplerState, input.uv).rgb;

    float exposure = 1.0;
    hdrColor *= exposure;

    float3 mapped = ReinhardToneMap(hdrColor);

    //mapped = pow(mapped, 1.0 / 2.2);

    return float4(mapped, 1.0);
}