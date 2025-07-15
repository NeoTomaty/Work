#include "Sprite.h"
#include "Application.h"
#include <iostream>

using namespace DirectX::SimpleMath;

// ������
void Sprite::Init(
	std::string _tex,	// �e�N�X�`��
	std::string _nml,	// �@��
	Vector3 _center,	// ���S���W
	Vector2 _length,	// ����
	Vector3 _rotate,	// ��]
	Vector3 _scale,		// �傫��
	bool _isanim,		// �A�j���[�V�����t���O
	Vector2 _div,		// ������
	float _animspeed)	// �A�j���[�V�������x
{

	// �������i�[
	m_center = _center;
	m_length = _length;
	m_rotate = _rotate;
	m_scale = _scale;
	m_isAnim = _isanim;
	m_div = _div;
	m_animSpeed = _animspeed;

	// ���T�C�Y
	m_vertices.resize(4);

	m_vertices[0].Position = Vector3(m_center.x - m_length.x / 2, m_center.y + m_length.y / 2, m_center.z);	 // ����
	m_vertices[1].Position = Vector3(m_center.x + m_length.x / 2, m_center.y + m_length.y / 2, m_center.z);	 // �E��
	m_vertices[2].Position = Vector3(m_center.x - m_length.x / 2, m_center.y - m_length.y / 2, m_center.z);	 // ����
	m_vertices[3].Position = Vector3(m_center.x + m_length.x / 2, m_center.y - m_length.y / 2, m_center.z);	 // �E��

	m_vertices[0].TexCoord = Vector2(0, 0);	
	m_vertices[1].TexCoord = Vector2(1, 0);
	m_vertices[2].TexCoord = Vector2(0, 1);
	m_vertices[3].TexCoord = Vector2(1, 1);

	// �@���͑O�������Ă���iZ�����j
	m_vertices[0].Normal = Vector3(0, 0, 1);
	m_vertices[1].Normal = Vector3(0, 0, 1);
	m_vertices[2].Normal = Vector3(0, 0, 1);
	m_vertices[3].Normal = Vector3(0, 0, 1);

	// �ڐ�
	m_vertices[0].Tangent = Vector3(1, 0, 0);
	m_vertices[1].Tangent = Vector3(1, 0, 0);
	m_vertices[2].Tangent = Vector3(1, 0, 0);
	m_vertices[3].Tangent = Vector3(1, 0, 0);

	// ���_�o�b�t�@����
	m_VertexBuffer.Create(m_vertices);

	// ���T�C�Y
	m_indices.resize(4);

	m_indices[0] = 0;
	m_indices[1] = 1;
	m_indices[2] = 2;
	m_indices[3] = 3;

	// �C���f�b�N�X�o�b�t�@����
	m_IndexBuffer.Create(m_indices);

	// �}�e���A������
	MATERIAL	mtrl;
	mtrl.Ambient = Color(0, 0, 0, 0);
	mtrl.Diffuse = Color(1, 1, 1, 1);
	mtrl.Emission = Color(0, 0, 0, 0);
	mtrl.Specular = Color(0, 0, 0, 0);
	mtrl.Shiness = 0;

	// �}�e���A������
	m_Material.Create(mtrl);

	// �e�N�X�`�����[�h
	bool sts = m_Texture.Load(_tex);
	assert(sts == true);

	// �@�����[�h
	if (_nml != "")
	{
		bool sts = m_Normal.Load(_nml);
		assert(sts == true);
	}
}

// �X�V
void Sprite::Update()
{

	// ���_���W�̍X�V
	m_vertices[0].Position = Vector3(m_center.x - m_length.x / 2, m_center.y + m_length.y / 2, m_center.z);	 // ����
	m_vertices[1].Position = Vector3(m_center.x + m_length.x / 2, m_center.y + m_length.y / 2, m_center.z);	 // �E��
	m_vertices[2].Position = Vector3(m_center.x - m_length.x / 2, m_center.y - m_length.y / 2, m_center.z);	 // ����
	m_vertices[3].Position = Vector3(m_center.x + m_length.x / 2, m_center.y - m_length.y / 2, m_center.z);	 // �E��


	// �A�j���[�V������ON�̎�
	if (m_isAnim)
	{
		float divx = 1.0f / m_div.x;
		float divy = 1.0f / m_div.y;

		// ���݂̃t���[���ԍ�����UV���W�̈ʒu���v�Z
		int frameX = m_animcnt % static_cast<int>(m_div.x); // ���̈ʒu
		int frameY = m_animcnt / static_cast<int>(m_div.y); // �c�̈ʒu

		float u = frameX * divx;
		float v = frameY * divy;

		// ���_�f�[�^�������� (v ���W�ɂ��Ή�)
		m_vertices[0].TexCoord.x = u;
		m_vertices[0].TexCoord.y = v;
		m_vertices[1].TexCoord.x = u + divx;
		m_vertices[1].TexCoord.y = v;
		m_vertices[2].TexCoord.x = u;
		m_vertices[2].TexCoord.y = v + divy;
		m_vertices[3].TexCoord.x = u + divx;
		m_vertices[3].TexCoord.y = v + divy;
		m_VertexBuffer.Modify(m_vertices);


		// �A�j���[�V�����؂�ւ����Ԓ���
		static auto loopcnt = 0;
		loopcnt++;

		if (loopcnt % m_animSpeed == 0) {
			m_animcnt++;
			m_animcnt = m_animcnt % static_cast<int>(m_div.x);
		}

	}


	// ���_�X�V
	m_VertexBuffer.Modify(m_vertices);
}



// �`��
void Sprite::Draw(bool shadowmap)
{

	ID3D11DeviceContext* devicecontext = Renderer::GetDeviceContext();

	// �s�����O�t���[���ɕۑ�
	m_WorldMatrix.PrevMat = m_WorldMatrix.Mat;

	// ���[���h�ϊ��s����쐬
	Matrix rmtx = Matrix::CreateFromYawPitchRoll(m_rotate.y, m_rotate.x, m_rotate.z);
	Matrix smtx = Matrix::CreateScale(m_scale.x, m_scale.y, m_scale.z);
	Matrix tmtx = Matrix::CreateTranslation(m_center.x, m_center.y, m_center.z);

	Matrix wmtx = smtx * rmtx * tmtx;

	// ���[���h�s����Z�b�g
	m_WorldMatrix.Mat = wmtx;

	// �t�s������߂�
	m_WorldMatrix.InvMat = m_WorldMatrix.Mat.Invert();

	// �萔�o�b�t�@�ɃZ�b�g
	Renderer::SetWorldMatrix(&m_WorldMatrix);

	// �g�|���W�[���Z�b�g�i���v���~�e�B�u�^�C�v�j
	devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	m_VertexBuffer.SetGPU();
	m_IndexBuffer.SetGPU();
	m_Material.SetGPU();
	m_Texture.SetGPU(DIFFUSE);
	m_Normal.SetGPU(NORMAL);

	// �`��J�E���g�����Z
	auto num = 2;
	Application::CountDrawInfo(num,shadowmap);

	devicecontext->DrawIndexed(
		4,				// �`�悷��C���f�b�N�X���i�l�p�`�Ȃ̂łS�j
		0,				// �ŏ��̃C���f�b�N�X�o�b�t�@�̈ʒu
		0);
}

// ���
void Sprite::Dispose()
{

}

// �����Ɉˑ����������v�X�v���C�g�̍X�V
void Sprite::UpdateLampPosition(Vector3 _lightPos)
{
	// ���S���W�ɉ��Z
	m_center.x = _lightPos.x + 20.0f;
	m_center.y = _lightPos.y - 10.0f;
	m_center.z = _lightPos.z + 40.0f;
}
