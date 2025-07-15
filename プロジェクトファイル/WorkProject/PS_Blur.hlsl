#include "Common.hlsli"

float4 main(PS_IN input) : SV_TARGET
{
    float value = 1.0f / 5.0f; // 5x5のフィルター値
    float filter[5] =
    {
        value, value, value, value, value,
    };

	// フィルターに基づいて色を取得
    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);  // 出力色の初期化
    float2 uvOffset = 1.0f / texSize;               // 1ピクセルのUV上での移動量
    uvOffset *= blurDir;                            // 指定された方向のみ移動させる
    float2 uv = input.uv - uvOffset * 2;            // 初期位置からずれたUV座標を計算
	
	// フィルターを適用して色を計算
    for (int i = 0; i < 5; ++i)
    {
		// テクスチャから色をサンプリングしてフィルターをかける
        color += g_Texture.Sample(g_SamplerState, uv) * filter[i];
        uv += uvOffset;
    }
    
    
    return color;
}