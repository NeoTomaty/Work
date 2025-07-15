#pragma once
#include <DirectXMath.h>
#include <SimpleMath.h>
#include "Renderer.h"

// マウス操作用の構造体
struct Argument
{
	DirectX::XMFLOAT2 mouseMove;
	DirectX::XMVECTOR vCamFront;
	DirectX::XMVECTOR vCamSide;
	DirectX::XMVECTOR vCamUp;
	DirectX::XMVECTOR vCamPos;
	DirectX::XMVECTOR vCamLook;
	float focus;
};

// カメラクラス
class Camera {
private:

	// カメラの基本情報
	DirectX::SimpleMath::Vector3	m_Position = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	DirectX::SimpleMath::Vector3	m_Rotation = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	DirectX::SimpleMath::Vector3	m_Scale = DirectX::SimpleMath::Vector3(1.0f, 1.0f, 1.0f);

	// カメラが見ている位置
	DirectX::SimpleMath::Vector3	m_Target{};

	// カメラの方向
	DirectX::SimpleMath::Vector3	m_Direction;

	// クリップ空間値
	float m_nearClip = 1.0f;
	float m_farClip = 500.0f;

	// 行列
	MatrixInfo m_ViewMatrix;
	MatrixInfo m_ProjMatrix;

	// 前フレームの座標
	POINT  m_oldPos;

	DirectX::XMFLOAT3	m_up = { 0.0f,1.0f,0.0f };


	float m_RotationSpeed = 5.0f;  // 回転の感度



public:
	void Init();
	void Dispose();
	void Update();
	void Draw();

	float GetNearClip() {
		return m_nearClip;
	}

	float GetFarClip() {
		return m_farClip;
	}

	DirectX::SimpleMath::Vector3 GetPosition() {
		return m_Position;
	}

	DirectX::SimpleMath::Vector3 GetDirection() {
		return m_Direction;
	}
};