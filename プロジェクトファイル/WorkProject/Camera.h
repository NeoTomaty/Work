#pragma once
#include <DirectXMath.h>
#include <SimpleMath.h>
#include "Renderer.h"

// �}�E�X����p�̍\����
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

// �J�����N���X
class Camera {
private:

	// �J�����̊�{���
	DirectX::SimpleMath::Vector3	m_Position = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	DirectX::SimpleMath::Vector3	m_Rotation = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	DirectX::SimpleMath::Vector3	m_Scale = DirectX::SimpleMath::Vector3(1.0f, 1.0f, 1.0f);

	// �J���������Ă���ʒu
	DirectX::SimpleMath::Vector3	m_Target{};

	// �J�����̕���
	DirectX::SimpleMath::Vector3	m_Direction;

	// �N���b�v��Ԓl
	float m_nearClip = 1.0f;
	float m_farClip = 500.0f;

	// �s��
	MatrixInfo m_ViewMatrix;
	MatrixInfo m_ProjMatrix;

	// �O�t���[���̍��W
	POINT  m_oldPos;

	DirectX::XMFLOAT3	m_up = { 0.0f,1.0f,0.0f };


	float m_RotationSpeed = 5.0f;  // ��]�̊��x



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