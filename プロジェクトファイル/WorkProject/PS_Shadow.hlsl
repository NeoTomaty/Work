#include "Common.hlsli"

// ライト視点での深度と比較して影の部分を判定する関数
float ShadowCalculation(float4 lightPos)
{
      // NDC → UV座標に変換
    float3 projCoords = lightPos.xyz / lightPos.w;
    projCoords = projCoords * 0.5f + 0.5f;

    // Y軸反転が必要ならここで
    projCoords.y = 1.0f - projCoords.y;

    // UVが範囲外なら影を落とさない
    if (projCoords.x < 0 || projCoords.x > 1 ||
        projCoords.y < 0 || projCoords.y > 1 ||
        projCoords.z < 0 || projCoords.z > 1)
        return 1.0f;

    // シャドウマップから比較用深度を取得
    float shadowMapDepth = g_ShadowMap.Sample(g_SamplerShadow, projCoords.xy).r;

    // シャドウバイアス（影のアーティファクトを防ぐための微調整）
    float bias = 0.005f;

    // 深度比較：ライト視点から見た奥行きと実際の奥行き
    float shadow = projCoords.z - bias > shadowMapDepth ? 0.0f : 1.0f;

    return shadow;
}

float4 main(PS_IN input) : SV_TARGET
{
    float4 color = float4(0, 0, 0, 0);
    
    // 法線
    float3 normal = input.normal;
    
    // 法線マップを使用しているか？
    if (UseNormalMap)
    {
        // 法線マップを参照
        float3 SampledNormal = g_Normal.Sample(g_SamplerState, input.uv).xyz;
        // 法線の範囲を０～１からー１～＋１にする（タンジェントスペース）
        SampledNormal = (SampledNormal - 0.5f) * 2.0f;
        
        // TBN行列を作る（ワールド空間に変換するため）
        float3x3 TBN = float3x3(normalize(input.tangent),
                            normalize(input.bitangent),
                            normalize(input.normal));

        // 法線をワールド空間に適用する
        normal = mul(SampledNormal, TBN);
    }
    
	// テクスチャから色を取得
    float4 texColor = g_Texture.Sample(g_SamplerState, input.uv);

    // 平行光源の有無で環境光が変化
    float3 ambient = DLight.Enable ? DLight.Ambient : NightBright;
    color.rgb += texColor.rgb * ambient;
    
    float3 N = normalize(normal); // 法線ベクトルを正規化
    float3 V = normalize(Camera.eyePos - input.worldPos.xyz); // 視線ベクトルの正規化
    
    // 平行光源があった場合（昼表現）
    if (DLight.Enable)
    {   
        float3 L = normalize(-DLight.Direction.xyz); // ライトの方向を正規化
        float3 R = reflect(-L, N); // 反射ベクトル

         // 拡散反射光の計算
        float diffuse = saturate(dot(N, L));

        // シャドウの計算
        float shadow;
        if(isShadow){
            shadow = ShadowCalculation(input.lightPos);
        } else {
            shadow = 1.0f;
        }

        
        // 最終色の合成
        color.rgb += texColor.rgb * diffuse * DLight.Diffuse.rgb * shadow;
    }
    
    // 点光源
    for (int i = 0; i < PLightNum; i++)
    {
        // 点光源があった場合
        if (PLight[i].Enable)
        {
            float3 lightPos = PLight[i].Position.xyz; // ライトの座標
            float3 L = normalize(lightPos - input.worldPos.xyz); // 光源ベクトル
            float3 R = reflect(-L, N); // 反射ベクトル

            float distance = length(lightPos - input.worldPos.xyz); // 光源との距離
            
            // 距離による減衰計算
            // 一般式「1 / (a + b*d + c*d^2)」による現実の減衰
            float a = PLight[i].Attenuation.x;
            float b = PLight[i].Attenuation.y;
            float c = PLight[i].Attenuation.z;
            float attenuation = 1.0f / (a + b * distance + c * distance * distance);

            // 拡散反射（ディフューズ）と鏡面反射（スペキュラ）の計算
            float diffuse = saturate(dot(N, L));
            //float specular = pow(saturate(dot(R, V)), shininess);

            float3 lightColor = PLight[i].Diffuse * PLight[i].Intensity; // ライトの色
            
            // 点光源の影響を加算
            color.rgb += texColor.rgb * (diffuse * lightColor * attenuation) /* + (specular * lightColor * attenuation)*/; // スペキュラはない方がいいかも

        }
        
    }

    return color;
}
