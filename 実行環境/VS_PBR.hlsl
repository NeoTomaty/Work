#include "Common.hlsli"

PS_IN main(in VS_IN input)
{
    PS_IN output;

    // モデル空間 → ワールド空間変換
    float4 pos = float4(input.pos.xyz, 1.0f); // 頂点座標（x,y,z）を取得し、wを1.0fにする
    output.pos = mul(pos, World);                   // ワールド行列と乗算
    output.worldPos = float4(output.pos.xyz, 1.0f); // ワールド空間での頂点位置を保存
    
    // ワールド空間 → ビュー空間 → クリップ空間変換
    output.pos = mul(output.pos, View);             // ビュー行列と乗算
    output.viewPos = output.pos;                    // ビュー空間での頂点位置を保存
    output.pos = mul(output.pos, Proj);             // プロジェクション行列と乗算（クリップ空間）
    
    // モデル空間の法線をワールド空間に変換
    output.normal = normalize(mul(input.normal.xyz, (float3x3) World));

    // 接線とバイノーマルをワールド空間に変換
    output.tangent = normalize(mul(input.tangent, (float3x3) World));
    output.bitangent = normalize(cross(output.normal, output.tangent));
    
    // その他のデータをそのまま渡す
    output.uv = input.uv;
    output.color = input.color;

    return output;
   
}


