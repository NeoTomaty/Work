#include "StaticMeshRenderer.h"
#include "Application.h"
#include "DirectXMath.h"

using namespace DirectX::SimpleMath;
using namespace DirectX;

// 初期化
void StaticMeshRenderer::Init(StaticMesh& mesh)
{
	// 頂点バッファとインデックスバッファを生成
	MeshRenderer::Init(mesh);

	// サブセット情報取得
	m_Subsets = mesh.GetSubsets();

	// マテリアル情報取得	
	std::vector<MATERIAL> materials;
	materials = mesh.GetMaterials();

	// 行列の初期化
	m_WorldMatrix.Mat = Matrix::Identity;
	m_WorldMatrix.InvMat = Matrix::Identity;
	m_WorldMatrix.PrevMat = Matrix::Identity;


	// マテリアル数分ループしてマテリアルデータを生成
	for (int i = 0; i < materials.size(); i++)
	{

		// マテリアルオブジェクト生成
		std::unique_ptr<Material> m = std::make_unique<Material>();

		// マテリアル情報をセット
		m->Create(materials[i]);

		// マテリアルオブジェクトを配列に追加
		m_Materiales.push_back(std::move(m));
	}
}

// 更新
void StaticMeshRenderer::Update()
{

}

// 描画
void StaticMeshRenderer::Draw(bool shadowmap)
{
	// 行列情報を前フレームに保存
	m_WorldMatrix.PrevMat = m_WorldMatrix.Mat;

	float yaw = XMConvertToRadians(m_Rotation.y);
	float pitch = XMConvertToRadians(m_Rotation.x);
	float roll = XMConvertToRadians(m_Rotation.z);

	// ワールド変換行列を作成
	Matrix rmtx = Matrix::CreateFromYawPitchRoll(yaw, pitch, roll);
	Matrix smtx = Matrix::CreateScale(m_Scale.x, m_Scale.y, m_Scale.z);
	Matrix tmtx = Matrix::CreateTranslation(m_Position.x, m_Position.y, m_Position.z);

	Matrix wmtx = smtx * rmtx * tmtx;

	// ワールド行列をセット
	m_WorldMatrix.Mat = wmtx;

	// 逆行列を求める
	m_WorldMatrix.InvMat = m_WorldMatrix.Mat.Invert();

	// 定数バッファにセット
	Renderer::SetWorldMatrix(&m_WorldMatrix);

	// インデックスバッファ・頂点バッファをセット
	BeforeDraw();


	// マテリアル数分ループ 
	for (int i = 0; i < m_Subsets.size(); i++)
	{
		// マテリアルをセット(サブセット情報の中にあるマテリアルインデックを使用する)
		m_Materiales[m_Subsets[i].MaterialIdx]->SetGPU();

		// ポリゴン数
		auto num = m_Subsets[i].IndexNum / 3;
		// 描画情報をApplicaionに格納
		Application::CountDrawInfo(num,shadowmap);

		// サブセットの描画
		DrawSubset(
			m_Subsets[i].IndexNum,							// 描画するインデックス数
			m_Subsets[i].IndexBase,							// 最初のインデックスバッファの位置	
			m_Subsets[i].VertexBase);						// 頂点バッファの最初から使用
	}
}
