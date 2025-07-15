#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Material.h"
#include "Texture.h"
#include "MeshRenderer.h"
#include "PlaneMesh.h"

class Field {
	// SRT���i�p�����j
	DirectX::SimpleMath::Vector3	m_Position = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	DirectX::SimpleMath::Vector3	m_Rotation = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	DirectX::SimpleMath::Vector3	m_Scale = DirectX::SimpleMath::Vector3(1.0f, 1.0f, 1.0f);

	// ���b�V��
	PlaneMesh		m_PlaneMesh;				// ���b�V��
	MeshRenderer	m_MeshRenderer;				// ���b�V�������_���[

	// �s��
	MatrixInfo						m_WorldMatrix;

	// �`��ׂ̈̏��i�����ڂɊւ�镔���j
	Material					m_Material;					// �}�e���A��
	Texture						m_Texture;					// �e�N�X�`��
	Texture						m_Normal;					// �@���}�b�v


public:
	void Init(DirectX::XMUINT2 _div, DirectX::XMUINT2 _size, DirectX::SimpleMath::Vector3 _pos, std::string _albedo, std::string _normal = nullptr);
	void Draw(bool shaowmap = false);
	void Dispose();

};