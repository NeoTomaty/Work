#include "Common.hlsli"


PS_IN main( VS_IN input )
{
    PS_IN output;
	
    float4 pos = float4(input.pos.xyz, 1.0f); // wを1.0fにする
    
    // 現在フレームの頂点座標
    output.pos = mul(pos, World);                   // ワールド行列と乗算
    output.worldPos = float4(output.pos.xyz, 1.0f); // ワールド空間での頂点位置を保存
    output.pos = mul(output.pos, View);             // ビュー行列と乗算
    output.viewPos = output.pos;                    // ビュー空間での頂点位置を保存
    output.pos = mul(output.pos, Proj);             // プロジェクション行列と乗算（クリップ空間）
    
    // 前フレームの頂点座標
    output.prevPos = mul(pos, PrevWorld);                   // 前フレームのワールド行列と乗算
    output.prevWorldPos = float4(output.prevPos.xyz, 1.0f); // 前フレームのワールド空間での頂点位置を保存
    output.prevPos = mul(output.prevPos, PrevView);         // 前フレームのビュー行列と乗算
    output.prevViewPos = output.prevPos;                    // 前フレームのビュー空間での頂点位置を保存
    output.prevPos = mul(output.prevPos, PrevProj);         // 前フレームのプロジェクション行列と乗算（クリップ空間）

    // ライトのカメラ座標を計算
    //float4 Light = mul(pos, World);
    float4 Light = pos;
    Light = mul(Light, LightView);
    Light = mul(Light, LightProj);
    output.lightPos = Light;
    
	// 法線と接線をワールド行列で変換
    output.normal = normalize(mul(input.normal.xyz, (float3x3) World));
    output.tangent = normalize(mul(input.tangent.xyz, (float3x3) World));

    // 従法線を求める
    output.bitangent = normalize(cross(output.normal, output.tangent)); 
    
	// その他のデータはそのまま渡す
    output.uv = input.uv;       // UV座標
    output.color = input.color; // カラー
    
    return output;
}

    
