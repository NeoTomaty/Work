#include "SceneLight.h"
#include <SimpleMath.h>
#include "DirectXMath.h"
#include "iostream"
#include "Camera.h"

using namespace DirectX::SimpleMath;
using namespace DirectX;

// ライトの初期化
void SceneLight::InitLight()
{
	auto device = Renderer::GetDevice();
	auto context = Renderer::GetDeviceContext();


	// 平行光源の初期化
	m_DirectionalLight.Enable = true;								// ライトの可否
	m_DirectionalLight.Position = Vector3(-145.0f, 170.0f, -145.0f);// 座標
	m_DirectionalLight.Direction = Vector4(1.0f, 1.0f, 1.0f, 0.0f);	// ライト方向
	m_DirectionalLight.Direction.Normalize();
	m_DirectionalLight.Ambient = Color(0.5f, 0.5f, 0.5f, 1.0f);		// 環境光
	m_DirectionalLight.Diffuse = Color(0.5f, 0.5f, 0.5f, 1.0f);		// 拡散反射光

	m_target = Vector3(0, 0, 0);	// 注視点


	// ポイントライトの位置
	Vector3 pos[POINTLIGHT_NUM] = {
		{   3.5f,12.5f,  0.0f},
		{- 56.5f,12.5f,-80.0f},
		{   3.5f,12.5f,-80.0f},
		{  63.5f,12.5f,-80.0f},
		{  63.5f,12.5f,  0.0f},
	};

	// 点光源の初期化
	for (auto i = 0; i < POINTLIGHT_NUM; i++)
	{
		m_PointLights[i].Enable = true;								// ライトの可否
		m_PointLights[i].Position = pos[i];							// 座標
		m_PointLights[i].Intensity = 6.0f;							// 輝度
		m_PointLights[i].Ambient = Color(1.0f, 1.0f, 1.0f, 1.0f);	// 環境光
		m_PointLights[i].Diffuse = Color(1.0f, 1.0f, 1.0f, 1.0f);	// 拡散反射光
		m_PointLights[i].Attenuation = Vector3(1.0f, 0.1f, 0.01f);	// 減衰定数

	}


	// 行列の初期化
	m_lightView = m_lightProj = Matrix::Identity;


	// 定数バッファの生成
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(LIGHTPARAM);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = sizeof(float);

	// ライト定数バッファの作成
	device->CreateBuffer(&bufferDesc, NULL, &m_LightBuffer);
	context->VSSetConstantBuffers(6, 1, &m_LightBuffer);
	context->PSSetConstantBuffers(6, 1, &m_LightBuffer);

}

// ライト行列の更新
void SceneLight::UpdateLightMatrix(Camera* _camera)
{
	// ライト方向を単位ベクトルに
	Vector3 direction = Vector3(m_DirectionalLight.Direction.x, m_DirectionalLight.Direction.y,m_DirectionalLight.Direction.z);

	// ライトが見る位置
	Vector3 target = m_target;

	// ライトの位置
	Vector3 lightPos = m_DirectionalLight.Position;


	// ライトの上ベクトル
	Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
	if (fabsf(direction.x) < 0.01f && fabsf(direction.z) < 0.01f)
	{
		// ライトの方向が上ベクトルを平行にならないように
		up = Vector3(0.0f, 0.0f, -1.0f);
	}

	// ビュー行列（ライト視点のカメラ）
	m_lightView =
		DirectX::XMMatrixLookAtLH(
			lightPos,
			target,
			up);


	// 正射影のサイズ（ライト視点の視野範囲）
	float Width = 1500.0f;
	float Height = 1500.0f;
	float nearZ = 1.0f;
	float farZ = 500.0f;


	// プロジェクション行列の生成（正射影）
	m_lightProj =
		DirectX::XMMatrixOrthographicLH(
			Width,
			Height,
			nearZ,
			farZ);

}

// ライトの描画（バインド）
void SceneLight::DrawLight()
{
	auto context = Renderer::GetDeviceContext();

	// ライトのバインド
	LIGHTPARAM param;

	// 平行光源
	param.DirectionalLight = m_DirectionalLight;

	// 点光源
	for (auto i = 0; i < POINTLIGHT_NUM; i++)
	{
		param.PointLight[i] = m_PointLights[i];
	}

	// 転置した行列
	param.View = m_lightView.Transpose();
	param.Proj = m_lightProj.Transpose();


	context->UpdateSubresource(m_LightBuffer, 0, NULL, &param, 0, 0);
}

// ライトの解放処理
void SceneLight::DisposeLight()
{
	// リソースの解放
	if (m_LightBuffer) {
		m_LightBuffer->Release();
		m_LightBuffer = nullptr;
	}
}
