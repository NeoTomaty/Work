// 定数
static const float NightBright = 0.1f;             // 夜の明るさ
static const float PLightNum = 5;      // 点光源の数

// ワールド変換行列
cbuffer WorldBuffer : register(b0)
{
    matrix World;
    matrix InvWorld;    // 逆行列
    matrix PrevWorld;   // 前フレームの行列
}

// ビュー変換行列
cbuffer ViewBuffer : register(b1)
{
    matrix View;
    matrix InvView;     // 逆行列
    matrix PrevView;    // 前フレームの行列
}

// プロジェクション変換行列
cbuffer ProjectionBuffer : register(b2)
{
    matrix Proj;
    matrix InvProj;     // 逆行列
    matrix PrevProj;    // 前フレームの行列
}

// マテリアル情報
struct MATERIAL
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emission;
    float Shininess;
    float Alpha;
    float2 Dummy;
};

// マテリアル定数
cbuffer MaterialBuffer : register(b3)
{
    MATERIAL Material;
}

// カメラ情報
struct CAMERA
{
    float3 eyePos;  // カメラの位置
    float padding;  // パディング
    
    float nearClip; // ニアクリップ
    float farClip;  // ファークリップ
    float2 padding4;// パディング
};

// カメラ定数
cbuffer CameraBuffer : register(b4)
{
    CAMERA Camera;
};


// その他パラメータ
cbuffer Param : register(b5)
{
    int UseNormalMap;   // 法線マップを使用するかどうか
    int TextureType;    // ディファードレンダリングで使用するテクスチャタイプ
    int isShadow;       // シャドウのON/OFF
    int padding2;       // パディング用
    
    float2 texSize;     // ブラーテクスチャサイズ
    float2 blurDir;     // ブラー方向
    
    float cameraFocus;  // カメラの焦点距離
    float DoFRange;     // 被写界深度範囲
    
    float vignetteStrength; // ビネットの強さ
    float vignetteRadius;   // ビネットの開始半径
    float2 vignetteCenter;  // ビネットの中心
    
    float bloomStrength;    // ブルーム発光の強さ
    float padding3;         // パディング
}

// 平行光源
struct DirectionalLight
{
    int Enable;         // ライトの可否
    float3 padding0;    // パディング

    float3 Position;    // ライトの位置
    float padding2;     // パディング
    
    float4 Direction;   // ライトの方向

    float4 Diffuse;     // 拡散反射光
    float4 Ambient;     // 環境光
    
};

// 点光源
struct PointLight
{
    int Enable;         // ライトの可否
    float3 padding1;    // パディング

    float3 Position;    // ライトの位置
    float Intensity;    // 明るさ

    float4 Diffuse;     // 拡散反射光
    float4 Ambient;     // 環境光

    float3 Attenuation; // 減衰係数
    float padding2;     // パディング
};

// ライト定数
cbuffer LightBuffer : register(b6)
{
    DirectionalLight DLight;        // 平行光源
    PointLight PLight[PLightNum];   // 点光源
    
    matrix LightView;               // ライトのビュー
    matrix LightProj;               // ライトのプロジェクション
};


// ディファードレンダリング
Texture2D g_Texture : register(t0);     // アルベド
Texture2D g_Normal : register(t1);      // 法線
Texture2D g_World : register(t2);       // ワールド座標
Texture2D g_Depth : register(t3);       // 深度
Texture2D g_MVector : register(t4);     // モーションベクトル

// シャドウマップ
Texture2D g_ShadowMap : register(t5);           // シャドウマップ

// 深度情報
Texture2D<float> g_DepthMap : register(t6);     // 深度書き込みで取得した深度情報

// ブラー
Texture2D g_BlurTex : register(t7);             // ブラーテクスチャ

// ブルーム（輝度）
Texture2D g_LuminanceTex : register(t8);        // ブルームテクスチャ


SamplerState g_SamplerState : register(s0);  // テクスチャサンプラー
SamplerState g_SamplerShadow : register(s1); // シャドウマップサンプラー

// 頂点入力
struct VS_IN
{
    float4 pos : POSITION0;
    float4 normal : NORMAL0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float3 tangent : TANGENT0;
};

// ピクセル入力
struct PS_IN
{
    float4 pos : SV_POSITION0;          // プロジェクション変換後座標
    float4 prevPos : POSITION0;         // 前フレームのプロジェクション変換後座標
    float4 worldPos : POSITION1;        // ワールド変換後座標
    float4 prevWorldPos : POSITION2;    // 前フレームのワールド変換後座標
    float4 viewPos : POSITION3;         // ビュー変換後座標
    float4 prevViewPos : POSITION4;     // 前フレームのビュー変換後座標

    float4 lightPos : POSITION5;        // ライトの座標
    
    float3 normal : NORMAL0;            // 法線
    float4 color : COLOR0;              // カラー
    float2 uv : TEXCOORD0;              // UV
    float3 tangent : TANGENT0;          // 接線
    float3 bitangent : BITANGENT0;      // 従法線
};

// シャドウマップ用
struct PS_IN_SHADOWMAP
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD0;
};


// 最終出力（深度情報を格納）
struct PS_OUT
{
    float4 BeseColor    : SV_TARGET0;  // 第1のレンダーターゲット（RTV 0）への出力
};


// シャドウマップ用Output
struct PS_OUT_SHADOWMAP
{
    float4 position : SV_TARGET0;
};


// Gバッファ用出力結果
struct PS_OUT_GBUFFER
{
    float4 Albedo : SV_TARGET0;     // 第1のレンダーターゲット（RTV 0）への出力
    float4 Normal : SV_TARGET1;     // 第2のレンダーターゲット（RTV 1）への出力
    float4 World : SV_TARGET2;      // 第3のレンダーターゲット（RTV 2）への出力
    float4 Depth : SV_TARGET3;      // 第4のレンダーターゲット（RTV 3）への出力
    float4 MVector : SV_TARGET4;    // 第5のレンダーターゲット（RTV 4）への出力
};


