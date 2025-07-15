#include "Common.hlsli"

float4 main(PS_IN input) : SV_TARGET
{
    float4 color = float4(0, 0, 0, 0);

    float shininess = 1.0f; // 輝度（大きいほど抑えた鏡面反射に）
    float3 specularColor = float3(0.01f, 0.01f, 0.01f); // 反射のしすぎを抑える
    
    // 法線
    float3 normal = input.normal;
    
    // 法線マップを使用しているか？
    if (UseNormalMap)
    {
        // 法線マップを参照
        float3 normal = g_Normal.Sample(g_SamplerState, input.uv).xyz;
        // 法線の範囲を０～１からー１～＋１にする（タンジェントスペース）
        normal = (normal - 0.5f) * 2.0f;
        
        // TBN行列を作る（ワールド空間に変換するため）
        float3x3 TBN = float3x3(normalize(input.tangent),
                            normalize(input.bitangent),
                            normalize(input.normal));

        // 法線をワールド空間に適用する
        normal = mul(normal, TBN);
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

        // 鏡面反射（スペキュラ）の計算
        float specular = pow(saturate(dot(R, V)), shininess);

        // 最終色の合成
        color.rgb += texColor.rgb * diffuse * DLight.Diffuse.rgb;
        color.rgb += specular * specularColor;
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
            float specular = pow(saturate(dot(R, V)), shininess);

            float3 lightColor = PLight[i].Diffuse * PLight[i].Intensity; // ライトの色
            
            // 点光源の影響を加算
            color.rgb += texColor.rgb * (diffuse * lightColor * attenuation);   // スペキュラはない方がいいかも

        }
        
    }

    
    return color;
}
