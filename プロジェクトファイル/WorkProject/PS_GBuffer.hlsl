#include "Common.hlsli"

// Pos:クリップ空間座標
// prevPos:前フレームのクリップ空間
float2 GetMotionVector(float4 Pos, float4 prevPos)
{
    //前フレームのクリップ空間
    float4 clipPos = prevPos;
    // 遠すぎる（クリップ外の）オブジェクトは動きを無視
    if (clipPos.w <= 0)
        return 0;

    // 透視除算（Clip 空間 → 正規化デバイス座標（NDC）[-1～1]）
    clipPos.xyz /= clipPos.w;
    
    // 前フレームのウィンドウ座標へ変換
    // （ビューポート座標は固定なので今回は直接宣言）
    float viewportWidth = 1280.0f;
    float viewportHeight = 720.0f;
    //float2 prevWindowPos = (clipPos.xy + 1) * 0.5f * float2(viewportWidth, viewportHeight);
    float2 clipToWindowScale = float2(viewportWidth * 0.5, viewportHeight * -0.5);  // （640.0f, -360.0f）
    float2 clipToWindowBias = float2(viewportWidth * 0.5, viewportHeight * 0.5);    // （640.0f, 360.0f）
    float2 prevWindowPos = clipPos.xy * clipToWindowScale + clipToWindowBias;

    // モーションベクトルを計算
    return saturate(prevWindowPos - Pos.xy);
}


PS_OUT_GBUFFER main(PS_IN input)
{
    PS_OUT_GBUFFER output;

    
    //// アルファが小さい場合は無視
    //clip(alpha - 0.0001f);
    
    // カラー
    output.Albedo = g_Texture.Sample(g_SamplerState, input.uv);
    
    // 法線
    if (UseNormalMap == 0){
        // 法線を正規化してw成分を1に
        float4 normal = float4(normalize(input.normal), 1.0);
        // skybox用に作成した法線
        output.Normal = normal;
    }
    else if (UseNormalMap == 1)
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
        float3 normal = mul(SampledNormal, TBN);
        
        // 正規化して出力
        output.Normal = float4(normal.xyz, 1.0);
        
    }

    // ワールド座標
    output.World = input.worldPos;
    
    // 深度
    float zValue = input.pos.z / input.pos.w;
    output.Depth = saturate(zValue.r);
    
    // モーションベクトル
    float2 mVector = GetMotionVector(input.pos, input.prevPos);

    // UV空間でのモーションベクトル
    output.MVector = float4(mVector, 0.0f, 1.0f);
    
    return output;
}