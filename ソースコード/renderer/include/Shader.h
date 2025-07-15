#pragma once
#include	"ComPtr.h"
#include	<string>
#include	<d3d11.h>
#include	"NonCopyable.h"


class Shader : NonCopyable{
public:
	void Release();
	void Create(std::string vs, std::string ps);
	void SetGPU();
private:
	ComPtr<ID3D11VertexShader> m_pVertexShader;		// 頂点シェーダー
	ComPtr<ID3D11PixelShader>  m_pPixelShader;		// ピクセルシェーダー
	ComPtr<ID3D11InputLayout>  m_pVertexLayout;		// 頂点レイアウト
};

