#include "Common.hlsli"


float4 main(PS_IN input) : SV_TARGET
{
	
    //カラーの出力
    float4 color = g_Texture.Sample(g_SamplerState, input.uv);
        
    // 平行光源の色を加算
    float3 light = DLight.Diffuse * 0.9f;

    // テクスチャカラーに加算
    float3 result = saturate(color.rgb * light);

    // 点光源による加算光
    for (int i = 0; i < PLightNum; ++i)
    {
        PointLight light = PLight[i];
        if (light.Enable == 0)
            continue;

        // ライトまでのベクトル
        float3 toLight = light.Position - input.worldPos.xyz;
        float dist = length(toLight);

        // 減衰係数
        float attenuation = 1.0 / (light.Attenuation.x + light.Attenuation.y * dist + light.Attenuation.z * dist * dist);

        // 拡散光 × 減衰 × 強さ
        float3 diffuse = light.Diffuse.rgb * light.Intensity * attenuation;

        // 色に加算
        result += color.rgb * diffuse * 0.1f;
    }
    
    return float4(result, color.a);
}