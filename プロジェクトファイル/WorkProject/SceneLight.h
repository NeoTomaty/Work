#pragma once
#include "Renderer.h"

const UINT POINTLIGHT_NUM = 5;	// �_�����̐�

// ���s����
struct Directional
{
    BOOL							Enable;		// ���C�g�̉�
    BOOL							Dummy[3];

    DirectX::SimpleMath::Vector3	Position;	// ���C�g�̍��W
    float							Dummy2;

    DirectX::SimpleMath::Vector4	Direction;	// ���C�g�̕���
    DirectX::SimpleMath::Color		Diffuse;	// �g�U���ˌ�
    DirectX::SimpleMath::Color		Ambient;	// ����

};
// �_����
struct Point
{
    BOOL							Enable;		// ���C�g�̉�
    BOOL							Dummy[3];

    DirectX::SimpleMath::Vector3	Position;	// ���C�g�̍��W
    float							Intensity;	// ���C�g�̖��邳

    DirectX::SimpleMath::Color		Diffuse;	// �g�U���ˌ�
    DirectX::SimpleMath::Color		Ambient;	// ����


    //Attenuation = float3(Constant, Linear, Quadratic)
    // x : Constant�i��茸���W���j
    // y : Linear�i���`�����W���j
    // z : Quadratic�i�Q�������W���j
    DirectX::SimpleMath::Vector3	Attenuation;// �����萔
    float							Padding;
};

// �o�C���h�p���C�g���
struct LIGHTPARAM
{
    Directional	DirectionalLight;
    Point		PointLight[POINTLIGHT_NUM];

    DirectX::SimpleMath::Matrix View;
    DirectX::SimpleMath::Matrix Proj;
};

// �O���錾
class Camera;

// �����N���X
class SceneLight
{
public:     // �����o�֐�

    void InitLight();                           // ���C�g�̏�����
    void UpdateLightMatrix(Camera* _camera);    // ���C�g�s��̍X�V
    void DrawLight();                           // ���C�g�̕`��i�o�C���h�j
    void DisposeLight();                        // ���C�g�̉������

    // �Z�b�^
    void SetDirectionalLight(const Directional& dir) { m_DirectionalLight = dir; }
    void SetPointLight(int index, const Point& point) { m_PointLights[index] = point; }
    void SetTarget(DirectX::SimpleMath::Vector3 _target) { m_target = _target; }

    // �Q�b�^
    const Directional& GetDirectionalLight() const { return m_DirectionalLight; }
    const Point& GetPointLight(int index) const { return m_PointLights[index]; }
    const DirectX::SimpleMath::Vector3 SetTarget() { return m_target; }

private:    // �����o�ϐ�

    Directional m_DirectionalLight;      // �_����
    Point m_PointLights[POINTLIGHT_NUM]; // �ʌ���

    // ���C�g�̍s��
    DirectX::SimpleMath::Matrix m_lightView;
    DirectX::SimpleMath::Matrix m_lightProj;
    
    // ���s�����̒����_
    DirectX::SimpleMath::Vector3 m_target;

    ID3D11Buffer* m_LightBuffer;            // ���C�g�p�萔�o�b�t�@
};


