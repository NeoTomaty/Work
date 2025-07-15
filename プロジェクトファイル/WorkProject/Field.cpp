#include	"Field.h"
#include	"DebugUI.h"


using namespace DirectX::SimpleMath;
using namespace DirectX;


void Field::Init(XMUINT2 _div, XMUINT2 _size, Vector3 _pos, std::string _albedo, std::string _normal)
{
	// 床メッシュ生成
	m_PlaneMesh.Init(
		_div.x, _div.y,				// 分割数
		_size.x,_size.y,			// サイズ（XZ平面）
		Color(1, 1, 1, 1),			// 頂点カラー
		Vector3(0, 1, 0),			// 法線ベクトル
		Vector3(1, 0, 0),			// 接ベクトル
		true);						// XZ平面

	// レンダラー初期化
	m_MeshRenderer.Init(m_PlaneMesh);

	m_Position = _pos;

	// マテリアル生成
	MATERIAL	mtrl;
	mtrl.Ambient = Color(0, 0, 0, 0);
	mtrl.Diffuse = Color(1, 1, 1, 1);
	mtrl.Emission = Color(0, 0, 0, 0);
	mtrl.Specular = Color(0, 0, 0, 0);
	mtrl.Shiness = 0;

	m_Material.Create(mtrl);

	// 行列の初期化
	m_WorldMatrix.Mat = Matrix::Identity;
	m_WorldMatrix.InvMat = Matrix::Identity;
	m_WorldMatrix.PrevMat = Matrix::Identity;

	// テクスチャロード
	bool sts = m_Texture.Load(_albedo);
	assert(sts == true);

	sts = m_Normal.Load(_normal);
	assert(sts == true);
}


void Field::Draw(bool shadowmap)
{
	// 行列情報を前フレームに保存
	m_WorldMatrix.PrevMat = m_WorldMatrix.Mat;

	// SRT情報作成
	Matrix r = 
		Matrix::CreateFromYawPitchRoll(
			m_Rotation.y, 
			m_Rotation.x, 
			m_Rotation.z);

	Matrix t = Matrix::CreateTranslation(
		m_Position.x, 
		m_Position.y, 
		m_Position.z);

	Matrix s = Matrix::CreateScale(
		m_Scale.x, 
		m_Scale.y, 
		m_Scale.z);

	// ワールド行列をセット
	m_WorldMatrix.Mat = s * r * t;

	// 逆行列を求める
	m_WorldMatrix.InvMat = m_WorldMatrix.Mat.Invert();

	Renderer::SetWorldMatrix(&m_WorldMatrix);		// GPUにセット

	auto num = m_PlaneMesh.GetIndices().size() / 3;
	// 描画情報をApplicaionに格納
	Application::CountDrawInfo(num,shadowmap);

	m_Material.SetGPU();
	m_Texture.SetGPU(DIFFUSE);
	m_Normal.SetGPU(NORMAL);
	m_MeshRenderer.Draw();
}

void Field::Dispose()
{

}


