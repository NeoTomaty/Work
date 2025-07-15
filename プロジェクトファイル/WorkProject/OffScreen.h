#pragma once
#include <SimpleMath.h>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Material.h"
#include "Texture.h"

// �I�t�X�N���[�������_�����O�p�̃N�A�b�h��`�悷��N���X
class OffScreen 
{
private:	// �����o�ϐ�
	// �e�탊�\�[�X
	IndexBuffer				m_IndexBuffer;
	VertexBuffer<VERTEX_3D>	m_VertexBuffer;
	Shader					m_Shader;
	Material				m_Material;
	Texture					m_Texture;

	std::vector<VERTEX_3D>	m_vertices;		// ���_
	std::vector<unsigned int> m_indices;	// �C���f�b�N�X


public:		// �����o�֐�
	void Init();
	void Update();
	void Draw();
	void Dispose();
};