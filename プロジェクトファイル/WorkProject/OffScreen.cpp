#include	"OffScreen.h"
#include	"Application.h"

using namespace DirectX::SimpleMath;

// ������
void OffScreen::Init()
{

	m_vertices.resize(4);

	float width = Application::GetWidth();
	float height = Application::GetHeight();

	m_vertices[0].Position = Vector3(0, 0, 0);			// ����
	m_vertices[1].Position = Vector3(width, 0, 0);		// �E��
	m_vertices[2].Position = Vector3(0, height, 0);		// ����
	m_vertices[3].Position = Vector3(width, height, 0);	// �E��

	m_vertices[0].TexCoord = Vector2(0, 0);
	m_vertices[1].TexCoord = Vector2(1, 0);
	m_vertices[2].TexCoord = Vector2(0, 1);
	m_vertices[3].TexCoord = Vector2(1, 1);

	// ���_�o�b�t�@����
	m_VertexBuffer.Create(m_vertices);


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


	m_Material.Create(mtrl);

}

void OffScreen::Update()
{

}

void OffScreen::Draw()
{
	// 2D�`��p
	Renderer::SetWorldViewProjection2D();

	ID3D11DeviceContext* devicecontext;

	devicecontext = Renderer::GetDeviceContext();

	// �g�|���W�[���Z�b�g�i���v���~�e�B�u�^�C�v�j
	devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	m_VertexBuffer.SetGPU();
	m_IndexBuffer.SetGPU();
	m_Material.SetGPU();

	devicecontext->DrawIndexed(
		4,				// �`�悷��C���f�b�N�X���i�l�p�`�Ȃ̂łS�j
		0,				// �ŏ��̃C���f�b�N�X�o�b�t�@�̈ʒu
		0);
}

void OffScreen::Dispose()
{

}