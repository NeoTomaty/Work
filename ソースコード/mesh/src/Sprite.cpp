#include "Sprite.h"
#include "Application.h"
#include <iostream>

using namespace DirectX::SimpleMath;

// 初期化
void Sprite::Init(
	std::string _tex,	// テクスチャ
	std::string _nml,	// 法線
	Vector3 _center,	// 中心座標
	Vector2 _length,	// 長さ
	Vector3 _rotate,	// 回転
	Vector3 _scale,		// 大きさ
	bool _isanim,		// アニメーションフラグ
	Vector2 _div,		// 分割数
	float _animspeed)	// アニメーション速度
{

	// 初期情報格納
	m_center = _center;
	m_length = _length;
	m_rotate = _rotate;
	m_scale = _scale;
	m_isAnim = _isanim;
	m_div = _div;
	m_animSpeed = _animspeed;

	// リサイズ
	m_vertices.resize(4);

	m_vertices[0].Position = Vector3(m_center.x - m_length.x / 2, m_center.y + m_length.y / 2, m_center.z);	 // 左上
	m_vertices[1].Position = Vector3(m_center.x + m_length.x / 2, m_center.y + m_length.y / 2, m_center.z);	 // 右上
	m_vertices[2].Position = Vector3(m_center.x - m_length.x / 2, m_center.y - m_length.y / 2, m_center.z);	 // 左下
	m_vertices[3].Position = Vector3(m_center.x + m_length.x / 2, m_center.y - m_length.y / 2, m_center.z);	 // 右下

	m_vertices[0].TexCoord = Vector2(0, 0);	
	m_vertices[1].TexCoord = Vector2(1, 0);
	m_vertices[2].TexCoord = Vector2(0, 1);
	m_vertices[3].TexCoord = Vector2(1, 1);

	// 法線は前を向いている（Z方向）
	m_vertices[0].Normal = Vector3(0, 0, 1);
	m_vertices[1].Normal = Vector3(0, 0, 1);
	m_vertices[2].Normal = Vector3(0, 0, 1);
	m_vertices[3].Normal = Vector3(0, 0, 1);

	// 接線
	m_vertices[0].Tangent = Vector3(1, 0, 0);
	m_vertices[1].Tangent = Vector3(1, 0, 0);
	m_vertices[2].Tangent = Vector3(1, 0, 0);
	m_vertices[3].Tangent = Vector3(1, 0, 0);

	// 頂点バッファ生成
	m_VertexBuffer.Create(m_vertices);

	// リサイズ
	m_indices.resize(4);

	m_indices[0] = 0;
	m_indices[1] = 1;
	m_indices[2] = 2;
	m_indices[3] = 3;

	// インデックスバッファ生成
	m_IndexBuffer.Create(m_indices);

	// マテリアル生成
	MATERIAL	mtrl;
	mtrl.Ambient = Color(0, 0, 0, 0);
	mtrl.Diffuse = Color(1, 1, 1, 1);
	mtrl.Emission = Color(0, 0, 0, 0);
	mtrl.Specular = Color(0, 0, 0, 0);
	mtrl.Shiness = 0;

	// マテリアル生成
	m_Material.Create(mtrl);

	// テクスチャロード
	bool sts = m_Texture.Load(_tex);
	assert(sts == true);

	// 法線ロード
	if (_nml != "")
	{
		bool sts = m_Normal.Load(_nml);
		assert(sts == true);
	}
}

// 更新
void Sprite::Update()
{

	// 頂点座標の更新
	m_vertices[0].Position = Vector3(m_center.x - m_length.x / 2, m_center.y + m_length.y / 2, m_center.z);	 // 左上
	m_vertices[1].Position = Vector3(m_center.x + m_length.x / 2, m_center.y + m_length.y / 2, m_center.z);	 // 右上
	m_vertices[2].Position = Vector3(m_center.x - m_length.x / 2, m_center.y - m_length.y / 2, m_center.z);	 // 左下
	m_vertices[3].Position = Vector3(m_center.x + m_length.x / 2, m_center.y - m_length.y / 2, m_center.z);	 // 右下


	// アニメーションがONの時
	if (m_isAnim)
	{
		float divx = 1.0f / m_div.x;
		float divy = 1.0f / m_div.y;

		// 現在のフレーム番号からUV座標の位置を計算
		int frameX = m_animcnt % static_cast<int>(m_div.x); // 横の位置
		int frameY = m_animcnt / static_cast<int>(m_div.y); // 縦の位置

		float u = frameX * divx;
		float v = frameY * divy;

		// 頂点データ書き換え (v 座標にも対応)
		m_vertices[0].TexCoord.x = u;
		m_vertices[0].TexCoord.y = v;
		m_vertices[1].TexCoord.x = u + divx;
		m_vertices[1].TexCoord.y = v;
		m_vertices[2].TexCoord.x = u;
		m_vertices[2].TexCoord.y = v + divy;
		m_vertices[3].TexCoord.x = u + divx;
		m_vertices[3].TexCoord.y = v + divy;
		m_VertexBuffer.Modify(m_vertices);


		// アニメーション切り替え時間調整
		static auto loopcnt = 0;
		loopcnt++;

		if (loopcnt % m_animSpeed == 0) {
			m_animcnt++;
			m_animcnt = m_animcnt % static_cast<int>(m_div.x);
		}

	}


	// 頂点更新
	m_VertexBuffer.Modify(m_vertices);
}



// 描画
void Sprite::Draw(bool shadowmap)
{

	ID3D11DeviceContext* devicecontext = Renderer::GetDeviceContext();

	// 行列情報を前フレームに保存
	m_WorldMatrix.PrevMat = m_WorldMatrix.Mat;

	// ワールド変換行列を作成
	Matrix rmtx = Matrix::CreateFromYawPitchRoll(m_rotate.y, m_rotate.x, m_rotate.z);
	Matrix smtx = Matrix::CreateScale(m_scale.x, m_scale.y, m_scale.z);
	Matrix tmtx = Matrix::CreateTranslation(m_center.x, m_center.y, m_center.z);

	Matrix wmtx = smtx * rmtx * tmtx;

	// ワールド行列をセット
	m_WorldMatrix.Mat = wmtx;

	// 逆行列を求める
	m_WorldMatrix.InvMat = m_WorldMatrix.Mat.Invert();

	// 定数バッファにセット
	Renderer::SetWorldMatrix(&m_WorldMatrix);

	// トポロジーをセット（旧プリミティブタイプ）
	devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	m_VertexBuffer.SetGPU();
	m_IndexBuffer.SetGPU();
	m_Material.SetGPU();
	m_Texture.SetGPU(DIFFUSE);
	m_Normal.SetGPU(NORMAL);

	// 描画カウントを加算
	auto num = 2;
	Application::CountDrawInfo(num,shadowmap);

	devicecontext->DrawIndexed(
		4,				// 描画するインデックス数（四角形なので４）
		0,				// 最初のインデックスバッファの位置
		0);
}

// 解放
void Sprite::Dispose()
{

}

// 光源に依存したランプスプライトの更新
void Sprite::UpdateLampPosition(Vector3 _lightPos)
{
	// 中心座標に加算
	m_center.x = _lightPos.x + 20.0f;
	m_center.y = _lightPos.y - 10.0f;
	m_center.z = _lightPos.z + 40.0f;
}
