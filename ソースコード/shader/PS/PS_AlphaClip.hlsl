#include "Common.hlsli"

float4 main(PS_IN_SHADOWMAP input) : SV_TARGET
{
    
    // モデルのα値を取得
    float alpha = g_Texture.Sample(g_SamplerState, input.uv).a;

    // アルファが小さい場合は影を作らない
    clip(alpha - 0.0001f);

    return float4(0.0, 0.0, 0.0, 1.0); // 色は返さない
}