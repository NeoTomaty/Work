#pragma once
#include <SimpleMath.h>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Material.h"
#include "Texture.h"


// �X�v���C�g�N���X
class Sprite
{
private:	// �����o�ϐ�
	// �e�탊�\�[�X
	IndexBuffer				m_IndexBuffer;
	VertexBuffer<VERTEX_3D>	m_VertexBuffer;
	Shader					m_Shader;
	Material				m_Material;
	Texture					m_Texture;
	Texture					m_Normal;

	std::vector<VERTEX_3D>	m_vertices;		// ���_
	std::vector<unsigned int> m_indices;	// �C���f�b�N�X

	DirectX::SimpleMath::Vector3 m_center;	// �|���S���̒��S���W
	DirectX::SimpleMath::Vector2 m_length;	// �|���S���̒���
	DirectX::SimpleMath::Vector3 m_rotate;	// �|���S���̊p�x
	DirectX::SimpleMath::Vector3 m_scale;	// �|���S���̑傫��
	DirectX::SimpleMath::Vector2 m_div;		// uv������
	bool m_isAnim;							// �A�j���[�V�����t���O
	int m_animcnt;							// �A�j���[�V�����J�E���g
	int m_animSpeed;						// �A�j���[�V�������x

	MatrixInfo m_WorldMatrix;	// ���[���h�ϊ��s��

public:		// �����o�֐�
	void Init(
		std::string _tex,
		std::string _nml,
		DirectX::SimpleMath::Vector3 _center = { 0, 0, 0 },
		DirectX::SimpleMath::Vector2 _length = { 100, 100 },
		DirectX::SimpleMath::Vector3 _rotate = { 0, 0, 0 },
		DirectX::SimpleMath::Vector3 _scale = { 1, 1, 1 },
		bool _isanim = false,
		DirectX::SimpleMath::Vector2 _div = { 1 ,1 },
		float _animspeed = 0);

	void Update();
	void Draw(bool shadowmap = false);
	void Dispose();

	// ���C�g�̈ʒu�Ɉˑ����������v�̍��W�̍X�V
	void UpdateLampPosition(DirectX::SimpleMath::Vector3 _lightPos);
};
