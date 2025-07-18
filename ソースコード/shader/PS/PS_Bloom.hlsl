#include "Common.hlsli"

float4 main(PS_IN input) : SV_TARGET
{
    float4 color = (0.0f, 0.0f, 0.0f, 1.0f);
    
    // 通常の描画結果を取得
    float4 sceneColor = g_Texture.Sample(g_SamplerState, input.uv);

    // 輝度マップを取得
    float4 bloomColor = g_LuminanceTex.Sample(g_SamplerState, input.uv);

    // 加算合成
    color = sceneColor + bloomColor * bloomStrength;

    return color;
    
}
