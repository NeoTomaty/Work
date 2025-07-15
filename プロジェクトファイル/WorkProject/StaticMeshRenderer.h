#pragma once
#include	<SimpleMath.h>
#include	"StaticMesh.h"
#include	"MeshRenderer.h"
#include	"Texture.h"
#include    "Material.h"

class StaticMeshRenderer : public MeshRenderer 
{
	std::vector<SUBSET> m_Subsets;
	std::vector<std::unique_ptr<Material>> m_Materiales;

	// SRTèÓïÒ
	DirectX::SimpleMath::Vector3 m_Position = { 0,0,0 };
	DirectX::SimpleMath::Vector3 m_Rotation = { 0,0,0 };
	DirectX::SimpleMath::Vector3 m_Scale = { 0.1f,0.1f,0.1f };

	MatrixInfo m_WorldMatrix;

public:	
	void Init(StaticMesh& mesh);
	void Update();
	void Draw(bool shadowmap = false);

	void SetPosition(DirectX::SimpleMath::Vector3 pos) {
		m_Position = pos;
	}

	void SetRotation(DirectX::SimpleMath::Vector3 rotate) {
		m_Rotation = rotate;
	}

	void SetScale(DirectX::SimpleMath::Vector3 scale) {
		m_Scale = scale;
	}
};
