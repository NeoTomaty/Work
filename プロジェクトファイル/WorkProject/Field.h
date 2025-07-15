#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Material.h"
#include "Texture.h"
#include "MeshRenderer.h"
#include "PlaneMesh.h"

class Field {
	// SRT情報（姿勢情報）
	DirectX::SimpleMath::Vector3	m_Position = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	DirectX::SimpleMath::Vector3	m_Rotation = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	DirectX::SimpleMath::Vector3	m_Scale = DirectX::SimpleMath::Vector3(1.0f, 1.0f, 1.0f);

	// メッシュ
	PlaneMesh		m_PlaneMesh;				// メッシュ
	MeshRenderer	m_MeshRenderer;				// メッシュレンダラー

	// 行列
	MatrixInfo						m_WorldMatrix;

	// 描画の為の情報（見た目に関わる部分）
	Material					m_Material;					// マテリアル
	Texture						m_Texture;					// テクスチャ
	Texture						m_Normal;					// 法線マップ


public:
	void Init(DirectX::XMUINT2 _div, DirectX::XMUINT2 _size, DirectX::SimpleMath::Vector3 _pos, std::string _albedo, std::string _normal = nullptr);
	void Draw(bool shaowmap = false);
	void Dispose();

};