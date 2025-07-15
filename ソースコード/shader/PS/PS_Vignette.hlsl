#include "Common.hlsli"

float4 main(PS_IN input) : SV_TARGET
{
    float2 uv = input.uv;

    // ビネットの中心
    float2 screenCenter = float2(vignetteCenter[0], vignetteCenter[1]); 
    
    // 画面中心からの距離を計算
    float2 distVec = uv - screenCenter;
    float dist = length(distVec);

    // ビネットファクター計算
    float vignette = smoothstep(vignetteRadius, 1.0, dist);
    vignette = pow(vignette, vignetteStrength);

    // 元の色取得
    float4 color = g_Texture.Sample(g_SamplerState, uv);

    // ビネット効果を適用
    color.rgb *= (1.0 - vignette);

    return color;
}