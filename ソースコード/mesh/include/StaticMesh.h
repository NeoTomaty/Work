#pragma once

#include	<simplemath.h>
#include	<string>
#include	<vector>
#include	<memory>
#include	"Texture.h"
#include	"Mesh.h"
#include	"Renderer.h"

class StaticMesh : public Mesh {
public:
	void Load(std::string filename);

	void LoadTex(std::string filename, TexType type)
	{
		switch (type)
		{
		case DIFFUSE:
			m_albedomap.Load(filename);
			break;
		case NORMAL:
			m_normalmap.Load(filename);
			break;
		}
	}

	void SetTexture()
	{
		m_albedomap.SetGPU(DIFFUSE);
		m_normalmap.SetGPU(NORMAL);
	}

	const std::vector<MATERIAL>& GetMaterials() {
		return m_materials;
	}

	const std::vector<SUBSET>& GetSubsets() {
		return m_subsets;
	}

	const std::vector<std::string>& GetDiffuseTextureNames() {
		return m_diffusetexturenames;
	}


private:

	std::vector<MATERIAL> m_materials;					// マテリアル情報
	std::vector<std::string> m_diffusetexturenames;		// ディフューズテクスチャ名
	std::vector<SUBSET> m_subsets;						// サブセット情報

	Texture	m_albedomap;	// ディフューズマップ
	Texture	m_normalmap;	// 法線マップ

};