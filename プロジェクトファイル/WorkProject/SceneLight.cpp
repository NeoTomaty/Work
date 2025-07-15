#include "SceneLight.h"
#include <SimpleMath.h>
#include "DirectXMath.h"
#include "iostream"
#include "Camera.h"

using namespace DirectX::SimpleMath;
using namespace DirectX;

// ���C�g�̏�����
void SceneLight::InitLight()
{
	auto device = Renderer::GetDevice();
	auto context = Renderer::GetDeviceContext();


	// ���s�����̏�����
	m_DirectionalLight.Enable = true;								// ���C�g�̉�
	m_DirectionalLight.Position = Vector3(-145.0f, 170.0f, -145.0f);// ���W
	m_DirectionalLight.Direction = Vector4(1.0f, 1.0f, 1.0f, 0.0f);	// ���C�g����
	m_DirectionalLight.Direction.Normalize();
	m_DirectionalLight.Ambient = Color(0.5f, 0.5f, 0.5f, 1.0f);		// ����
	m_DirectionalLight.Diffuse = Color(0.5f, 0.5f, 0.5f, 1.0f);		// �g�U���ˌ�

	m_target = Vector3(0, 0, 0);	// �����_


	// �|�C���g���C�g�̈ʒu
	Vector3 pos[POINTLIGHT_NUM] = {
		{   3.5f,12.5f,  0.0f},
		{- 56.5f,12.5f,-80.0f},
		{   3.5f,12.5f,-80.0f},
		{  63.5f,12.5f,-80.0f},
		{  63.5f,12.5f,  0.0f},
	};

	// �_�����̏�����
	for (auto i = 0; i < POINTLIGHT_NUM; i++)
	{
		m_PointLights[i].Enable = true;								// ���C�g�̉�
		m_PointLights[i].Position = pos[i];							// ���W
		m_PointLights[i].Intensity = 6.0f;							// �P�x
		m_PointLights[i].Ambient = Color(1.0f, 1.0f, 1.0f, 1.0f);	// ����
		m_PointLights[i].Diffuse = Color(1.0f, 1.0f, 1.0f, 1.0f);	// �g�U���ˌ�
		m_PointLights[i].Attenuation = Vector3(1.0f, 0.1f, 0.01f);	// �����萔

	}


	// �s��̏�����
	m_lightView = m_lightProj = Matrix::Identity;


	// �萔�o�b�t�@�̐���
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(LIGHTPARAM);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = sizeof(float);

	// ���C�g�萔�o�b�t�@�̍쐬
	device->CreateBuffer(&bufferDesc, NULL, &m_LightBuffer);
	context->VSSetConstantBuffers(6, 1, &m_LightBuffer);
	context->PSSetConstantBuffers(6, 1, &m_LightBuffer);

}

// ���C�g�s��̍X�V
void SceneLight::UpdateLightMatrix(Camera* _camera)
{
	// ���C�g������P�ʃx�N�g����
	Vector3 direction = Vector3(m_DirectionalLight.Direction.x, m_DirectionalLight.Direction.y,m_DirectionalLight.Direction.z);

	// ���C�g������ʒu
	Vector3 target = m_target;

	// ���C�g�̈ʒu
	Vector3 lightPos = m_DirectionalLight.Position;


	// ���C�g�̏�x�N�g��
	Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
	if (fabsf(direction.x) < 0.01f && fabsf(direction.z) < 0.01f)
	{
		// ���C�g�̕�������x�N�g���𕽍s�ɂȂ�Ȃ��悤��
		up = Vector3(0.0f, 0.0f, -1.0f);
	}

	// �r���[�s��i���C�g���_�̃J�����j
	m_lightView =
		DirectX::XMMatrixLookAtLH(
			lightPos,
			target,
			up);


	// ���ˉe�̃T�C�Y�i���C�g���_�̎���͈́j
	float Width = 1500.0f;
	float Height = 1500.0f;
	float nearZ = 1.0f;
	float farZ = 500.0f;


	// �v���W�F�N�V�����s��̐����i���ˉe�j
	m_lightProj =
		DirectX::XMMatrixOrthographicLH(
			Width,
			Height,
			nearZ,
			farZ);

}

// ���C�g�̕`��i�o�C���h�j
void SceneLight::DrawLight()
{
	auto context = Renderer::GetDeviceContext();

	// ���C�g�̃o�C���h
	LIGHTPARAM param;

	// ���s����
	param.DirectionalLight = m_DirectionalLight;

	// �_����
	for (auto i = 0; i < POINTLIGHT_NUM; i++)
	{
		param.PointLight[i] = m_PointLights[i];
	}

	// �]�u�����s��
	param.View = m_lightView.Transpose();
	param.Proj = m_lightProj.Transpose();


	context->UpdateSubresource(m_LightBuffer, 0, NULL, &param, 0, 0);
}

// ���C�g�̉������
void SceneLight::DisposeLight()
{
	// ���\�[�X�̉��
	if (m_LightBuffer) {
		m_LightBuffer->Release();
		m_LightBuffer = nullptr;
	}
}
