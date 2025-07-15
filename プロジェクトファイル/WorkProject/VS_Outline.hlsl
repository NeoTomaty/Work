#include "Common.hlsli"

PS_IN main( VS_IN input)
{
    PS_IN output;
    
    // ローカル座標をスクリーン座標へ変換
    output.pos = float4(input.pos.xyz, 1.0f);
    
    // 輪郭線として表示するために、法線方向へ頂点を移動
    // 拡大が大きくなりすぎるのを防ぐため補正を入れる
    output.pos.xyz += normalize(input.normal) * 0.01f;
    output.pos = mul(output.pos, World);    // ワールド座標
    output.pos = mul(output.pos, View);     // ビュー座標
    output.pos = mul(output.pos, Proj);     // プロジェクション座標
    
    // uv座標
    output.uv = input.uv;
    
    // ピクセルシェーダーで利用する法線を渡す
    output.normal = mul(input.normal.xyz, (float3x3) World);
    
    return output;
}
