#pragma once
#include "Renderer.h"

const UINT POINTLIGHT_NUM = 5;	// 点光源の数

// 平行光源
struct Directional
{
    BOOL							Enable;		// ライトの可否
    BOOL							Dummy[3];

    DirectX::SimpleMath::Vector3	Position;	// ライトの座標
    float							Dummy2;

    DirectX::SimpleMath::Vector4	Direction;	// ライトの方向
    DirectX::SimpleMath::Color		Diffuse;	// 拡散反射光
    DirectX::SimpleMath::Color		Ambient;	// 環境光

};
// 点光源
struct Point
{
    BOOL							Enable;		// ライトの可否
    BOOL							Dummy[3];

    DirectX::SimpleMath::Vector3	Position;	// ライトの座標
    float							Intensity;	// ライトの明るさ

    DirectX::SimpleMath::Color		Diffuse;	// 拡散反射光
    DirectX::SimpleMath::Color		Ambient;	// 環境光


    //Attenuation = float3(Constant, Linear, Quadratic)
    // x : Constant（一定減衰係数）
    // y : Linear（線形減衰係数）
    // z : Quadratic（２次減衰係数）
    DirectX::SimpleMath::Vector3	Attenuation;// 減衰定数
    float							Padding;
};

// バインド用ライト情報
struct LIGHTPARAM
{
    Directional	DirectionalLight;
    Point		PointLight[POINTLIGHT_NUM];

    DirectX::SimpleMath::Matrix View;
    DirectX::SimpleMath::Matrix Proj;
};

// 前方宣言
class Camera;

// 光源クラス
class SceneLight
{
public:     // メンバ関数

    void InitLight();                           // ライトの初期化
    void UpdateLightMatrix(Camera* _camera);    // ライト行列の更新
    void DrawLight();                           // ライトの描画（バインド）
    void DisposeLight();                        // ライトの解放処理

    // セッタ
    void SetDirectionalLight(const Directional& dir) { m_DirectionalLight = dir; }
    void SetPointLight(int index, const Point& point) { m_PointLights[index] = point; }
    void SetTarget(DirectX::SimpleMath::Vector3 _target) { m_target = _target; }

    // ゲッタ
    const Directional& GetDirectionalLight() const { return m_DirectionalLight; }
    const Point& GetPointLight(int index) const { return m_PointLights[index]; }
    const DirectX::SimpleMath::Vector3 SetTarget() { return m_target; }

private:    // メンバ変数

    Directional m_DirectionalLight;      // 点光源
    Point m_PointLights[POINTLIGHT_NUM]; // 面光源

    // ライトの行列
    DirectX::SimpleMath::Matrix m_lightView;
    DirectX::SimpleMath::Matrix m_lightProj;
    
    // 平行光源の注視点
    DirectX::SimpleMath::Vector3 m_target;

    ID3D11Buffer* m_LightBuffer;            // ライト用定数バッファ
};


