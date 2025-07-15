#pragma once
#include <SimpleMath.h>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Material.h"
#include "Texture.h"


// スプライトクラス
class Sprite
{
private:	// メンバ変数
	// 各種リソース
	IndexBuffer				m_IndexBuffer;
	VertexBuffer<VERTEX_3D>	m_VertexBuffer;
	Shader					m_Shader;
	Material				m_Material;
	Texture					m_Texture;
	Texture					m_Normal;

	std::vector<VERTEX_3D>	m_vertices;		// 頂点
	std::vector<unsigned int> m_indices;	// インデックス

	DirectX::SimpleMath::Vector3 m_center;	// ポリゴンの中心座標
	DirectX::SimpleMath::Vector2 m_length;	// ポリゴンの長さ
	DirectX::SimpleMath::Vector3 m_rotate;	// ポリゴンの角度
	DirectX::SimpleMath::Vector3 m_scale;	// ポリゴンの大きさ
	DirectX::SimpleMath::Vector2 m_div;		// uv分割数
	bool m_isAnim;							// アニメーションフラグ
	int m_animcnt;							// アニメーションカウント
	int m_animSpeed;						// アニメーション速度

	MatrixInfo m_WorldMatrix;	// ワールド変換行列

public:		// メンバ関数
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

	// ライトの位置に依存したランプの座標の更新
	void UpdateLampPosition(DirectX::SimpleMath::Vector3 _lightPos);
};
