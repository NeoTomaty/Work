#include "Common.hlsli"

// 深度情報をリニア化
float LinearizeDepth(float z, float nearZ, float farZ)
{
    return (nearZ * farZ) / (farZ - z * (farZ - nearZ));
}

float4 main(PS_IN input) : SV_TARGET
{
    
    float4 color = (0.0f, 0.0f, 0.0f, 1.0f);
    
	// 各情報を取得
    float4 albedo = g_Texture.Sample(g_SamplerState, input.uv);
    float depth = g_DepthMap.Sample(g_SamplerState, input.uv).r;
    float4 blur = g_BlurTex.Sample(g_SamplerState, input.uv);

    // zをリニア化
    float linearDepth = LinearizeDepth(depth, Camera.nearClip, Camera.farClip);

    
    float focus = cameraFocus; // カメラからピントが合う位置までの距離
    float range = DoFRange;    // 被写界深度でぼかしの画像に変化する距離

    float centerDist = abs(linearDepth - focus);    // フォーカス位置からの距離
    float rate = saturate(centerDist / range);      // 正規化(0～1)

    color = lerp(albedo, blur, rate);


    return color;
}