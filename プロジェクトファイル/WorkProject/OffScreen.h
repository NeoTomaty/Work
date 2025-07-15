#pragma once
#include <SimpleMath.h>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Material.h"
#include "Texture.h"

// オフスクリーンレンダリング用のクアッドを描画するクラス
class OffScreen 
{
private:	// メンバ変数
	// 各種リソース
	IndexBuffer				m_IndexBuffer;
	VertexBuffer<VERTEX_3D>	m_VertexBuffer;
	Shader					m_Shader;
	Material				m_Material;
	Texture					m_Texture;

	std::vector<VERTEX_3D>	m_vertices;		// 頂点
	std::vector<unsigned int> m_indices;	// インデックス


public:		// メンバ関数
	void Init();
	void Update();
	void Draw();
	void Dispose();
};