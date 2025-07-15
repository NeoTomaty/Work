#include "Renderer.h"
#include "Camera.h"
#include <iostream>

#pragma comment(lib, "dxgi.lib")

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

// �A�b�v�X�P�[���𑜓x
const XMUINT2 OutputRenderSize = { 1920,1080 };

// �A�_�v�^�[�E�����_�����O�֘A
D3D_FEATURE_LEVEL		Renderer::m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;
IDXGIFactory*			Renderer::m_Factory;
IDXGIAdapter*			Renderer::m_Adapter;
bool					Renderer::m_IsDisableNvidia = false;

// �����_�����O�T�C�Y
// �A�b�v�X�P�[���O�̃����_�[�^�[�Q�b�g�T�C�Y
XMUINT2					Renderer::m_InputRenderSize;
// �A�b�v�X�P�[����̃����_�[�^�[�Q�b�g�T�C�Y�i3840x2160�j�i4K�j
XMUINT2					Renderer::m_OutputRenderSize;

// ��{�@�\
ID3D11Device*			Renderer::m_Device{};
ID3D11DeviceContext*	Renderer::m_DeviceContext{};
IDXGISwapChain*			Renderer::m_SwapChain{};
ID3D11RenderTargetView*	Renderer::m_DefaultRTV{};

// �萔�o�b�t�@���\�[�X
ID3D11Buffer*			Renderer::m_WorldBuffer{};
ID3D11Buffer*			Renderer::m_ViewBuffer{};
ID3D11Buffer*			Renderer::m_ProjectionBuffer{};
ID3D11Buffer*			Renderer::m_MaterialBuffer{};
ID3D11Buffer*			Renderer::m_CameraBuffer{};
ID3D11Buffer*			Renderer::m_ParamBuffer{};

// �[�x�X�e���V��
ID3D11DepthStencilState* Renderer::m_DepthStateEnable{};
ID3D11DepthStencilState* Renderer::m_DepthStateDisable{};

// ���X�^���C�U�[�X�e�[�g
ID3D11RasterizerState* Renderer::m_RsCullFront = nullptr;	// �J�����O�i�\��`�悵�Ȃ��j
ID3D11RasterizerState* Renderer::m_RsCullBack = nullptr;	// �J�����O�i����`�悵�Ȃ��j
ID3D11RasterizerState* Renderer::m_RsCullNone = nullptr;	// �J�����O�i�J�����O�Ȃ��j

// �u�����h�X�e�[�g
ID3D11BlendState* Renderer::m_BSAlphaOff = nullptr;	// �f�t�H���g
ID3D11BlendState* Renderer::m_BSAlphaOn = nullptr;	// �A���t�@�u�����h

void Renderer::Init()
{
	HRESULT hr = S_OK;

	// �����_�����O�T�C�Y
	// �A�b�v�X�P�[���O�̃����_�[�^�[�Q�b�g�T�C�Y�i�E�B���h�E�T�C�Y�j
	m_InputRenderSize = { Application::GetWidth(),Application::GetHeight() };
	// �A�b�v�X�P�[����̃����_�[�^�[�Q�b�g�T�C�Y
	m_OutputRenderSize = OutputRenderSize;


	//=====================================================
	// GPU���ōł����\�̍������̂��g�p����
	//=====================================================
	hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&m_Factory);
	if (FAILED(hr)) {
		std::cerr << "DXGIFactory�̍쐬�Ɏ��s���܂���" << std::endl;
		return;
	}

	IDXGIAdapter* bestAdapter = nullptr;
	SIZE_T maxScore = 0;

	for (UINT i = 0; ; ++i) 
	{
		IDXGIAdapter* adapter = nullptr;
		if (m_Factory->EnumAdapters(i, &adapter) == DXGI_ERROR_NOT_FOUND)
			break;

		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);

		std::wcout << L"���o���ꂽGPU: " << desc.Description << std::endl;

		// GPU���\�ɉ������X�R�A���v�Z
		SIZE_T score = desc.DedicatedVideoMemory;

		// NVIDIA��D��I�ɑI�ԁi�X�R�A���Z�j
		if (wcsstr(desc.Description, L"NVIDIA")) {
			score += 1024 * 1024 * 1024; // 1GB�Ԃ�̃X�R�A���Z
		}

		// =============================
		// Intel�����������瑦�̗p
		// =============================
		//if (wcsstr(desc.Description, L"Intel")) {
		//	std::wcout << L"Intel GPU�����o�����̂ŗD��I�Ɏg�p���܂�: " << desc.Description << std::endl;
		//	if (bestAdapter) bestAdapter->Release(); // �O�̃A�_�v�^�����
		//	bestAdapter = adapter;
		//	break; // ���[�v�I��
		//}

		// ��荂�X�R�A��GPU��I��
		if (score > maxScore) {
			if (bestAdapter) bestAdapter->Release(); // �O�̃A�_�v�^�����
			bestAdapter = adapter;
			maxScore = score;
		}
		else {
			adapter->Release(); // �g�p���Ȃ��A�_�v�^�͉��
		}
	}

	// ���ʂ̔��f
	if (bestAdapter) {
		DXGI_ADAPTER_DESC desc;
		bestAdapter->GetDesc(&desc);
		std::wcout << L"�g�p����GPU: " << desc.Description << std::endl;
		m_Adapter = bestAdapter; // �����Fm_Adapter �ɃZ�b�g
	}
	else {
		std::cerr << "�g�p�\��GPU��������܂���ł���" << std::endl;
		return;
	}

	//=====================================================
	// �f�o�C�X�E�X���b�v�`�F�[���쐬
	//=====================================================
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = Application::GetWidth();
	swapChainDesc.BufferDesc.Height = Application::GetHeight();
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = Application::GetWindow();
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;

	// ����
	hr = D3D11CreateDeviceAndSwapChain(m_Adapter,D3D_DRIVER_TYPE_UNKNOWN,NULL,0,NULL,0,
		D3D11_SDK_VERSION,&swapChainDesc,&m_SwapChain,&m_Device,&m_FeatureLevel,&m_DeviceContext);


	//=====================================================
	// �����_�[�^�[�Q�b�g
	//=====================================================
	
	ID3D11Texture2D* tempTexture{};

	// �f�t�H���g�����_�[�^�[�Q�b�g
	// �f�t�H���g�̃����_�[�^�[�Q�b�g���o�b�N�o�b�t�@�Ɏw��
	m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&tempTexture);
	// �f�t�H���g�����_�[�^�[�Q�b�g�r���[�쐬

	m_Device->CreateRenderTargetView(tempTexture, nullptr, &m_DefaultRTV);
	tempTexture->Release();


	//=====================================================
	// ���X�^���C�U�X�e�[�g�ݒ�
	//=====================================================
	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.SlopeScaledDepthBias = 4.0f; // �X���[�v�X�P�[���h�[�x�o�C�A�X
	rasterizerDesc.DepthBias = 100;             // �[�x�o�C�A�X
	// ����
	m_Device->CreateRasterizerState(&rasterizerDesc, &m_RsCullFront);
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	m_Device->CreateRasterizerState(&rasterizerDesc, &m_RsCullBack);
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	m_Device->CreateRasterizerState(&rasterizerDesc, &m_RsCullNone);

	m_DeviceContext->RSSetState(m_RsCullBack);


	//=====================================================
	// �u�����h�X�e�[�g�ݒ�
	//=====================================================
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	//����
	hr = m_Device->CreateBlendState(&blendDesc, &m_BSAlphaOn);
	if (FAILED(hr)) {
		cerr << "�u�����h�X�e�[�g�������s" << endl;
	}

	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	// ����
	hr = m_Device->CreateBlendState(&blendDesc, &m_BSAlphaOff);
	if (FAILED(hr)) {
		cerr << "�u�����h�X�e�[�g�������s" << endl;
	}


	//=====================================================
	// �f�v�X�X�e���V���X�e�[�g�ݒ�
	//=====================================================
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;

	m_Device->CreateDepthStencilState( &depthStencilDesc, &m_DepthStateEnable );//�[�x�L���X�e�[�g


	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ZERO;
	m_Device->CreateDepthStencilState( &depthStencilDesc, &m_DepthStateDisable );//�[�x�����X�e�[�g

	m_DeviceContext->OMSetDepthStencilState( m_DepthStateEnable, NULL );


	//=====================================================
	// �T���v���[�X�e�[�g�ݒ�
	//=====================================================

	// �T���v���[�X�e�[�g�̍쐬�E�Z�b�g
	SetSamplerState();

	//=====================================================
	// �r���[�|�[�g�ݒ�
	//=====================================================

	// �r���[�|�[�g�̃Z�b�g
	SetViewPort();

	//=====================================================
	// �萔�o�b�t�@�̐���
	//=====================================================

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(MatrixInfo);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = sizeof(float);

	// �J����
	m_Device->CreateBuffer( &bufferDesc, NULL, &m_WorldBuffer );
	m_DeviceContext->VSSetConstantBuffers( 0, 1, &m_WorldBuffer);

	m_Device->CreateBuffer( &bufferDesc, NULL, &m_ViewBuffer );
	m_DeviceContext->VSSetConstantBuffers( 1, 1, &m_ViewBuffer );

	m_Device->CreateBuffer( &bufferDesc, NULL, &m_ProjectionBuffer );
	m_DeviceContext->VSSetConstantBuffers( 2, 1, &m_ProjectionBuffer );


	// �}�e���A��
	bufferDesc.ByteWidth = sizeof(MATERIAL);

	m_Device->CreateBuffer( &bufferDesc, NULL, &m_MaterialBuffer );
	m_DeviceContext->VSSetConstantBuffers( 3, 1, &m_MaterialBuffer );
	m_DeviceContext->PSSetConstantBuffers( 3, 1, &m_MaterialBuffer );


	// �J�����i���W�j
	bufferDesc.ByteWidth = sizeof(CAMERAPARAM);

	m_Device->CreateBuffer(&bufferDesc, NULL, &m_CameraBuffer);
	m_DeviceContext->VSSetConstantBuffers(4, 1, &m_CameraBuffer);
	m_DeviceContext->PSSetConstantBuffers(4, 1, &m_CameraBuffer);


	// �e�N�X�`���p�����[�^
	bufferDesc.ByteWidth = sizeof(ExtraParam);

	m_Device->CreateBuffer(&bufferDesc, NULL, &m_ParamBuffer);
	m_DeviceContext->VSSetConstantBuffers(5, 1, &m_ParamBuffer);
	m_DeviceContext->PSSetConstantBuffers(5, 1, &m_ParamBuffer);


	// �}�e���A��������
	MATERIAL material{};
	material.Diffuse = Color(1.0f, 1.0f, 1.0f, 1.0f);
	material.Ambient = Color(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

}


// ���\�[�X�̉������
void Renderer::Uninit()
{

	// �萔�o�b�t�@
	m_WorldBuffer->Release();
	m_WorldBuffer = nullptr;
	m_ViewBuffer->Release();
	m_ViewBuffer = nullptr;
	m_ProjectionBuffer->Release();
	m_ProjectionBuffer = nullptr;
	m_MaterialBuffer->Release();
	m_MaterialBuffer = nullptr;
	m_CameraBuffer->Release();
	m_CameraBuffer = nullptr;
	m_ParamBuffer->Release();
	m_ParamBuffer = nullptr;

	// ��b�@�\
	m_DeviceContext->ClearState();
	m_DefaultRTV->Release();
	m_DefaultRTV = nullptr;
	m_SwapChain->Release();
	m_SwapChain = nullptr;
	m_Adapter->Release();
	m_Adapter = nullptr;
	m_Factory->Release();
	m_Factory = nullptr;

	// ���X�^���C�U�X�e�[�g
	m_DepthStateEnable->Release();
	m_DepthStateEnable = nullptr;
	m_DepthStateDisable->Release();
	m_DepthStateDisable = nullptr;
	m_RsCullFront->Release();
	m_RsCullFront = nullptr;
	m_RsCullBack->Release();
	m_RsCullBack = nullptr;
	m_RsCullNone->Release();
	m_RsCullNone = nullptr;

	// �u�����h�X�e�[�g
	m_BSAlphaOn->Release();
	m_BSAlphaOn = nullptr;
	m_BSAlphaOff->Release();
	m_BSAlphaOff = nullptr;

	// �f�o�C�X�֘A
	m_DeviceContext->Release();
	m_DeviceContext = nullptr;
	m_Device->Release();
	m_Device = nullptr;
}



// �`��O�̏���
void Renderer::Begin()
{
	// �����_�[�^�[�Q�b�g�̃N���A
	SetDefaultRenderTarget();
}
// �X���b�v�����i�`��I���O�̏����j
void Renderer::End()
{
	m_SwapChain->Present( 1, 0 );
}



// �[�x�l��ON/OFF
void Renderer::SetDepthEnable( bool Enable )
{
	if( Enable )
		m_DeviceContext->OMSetDepthStencilState( m_DepthStateEnable, NULL );
	else
		m_DeviceContext->OMSetDepthStencilState( m_DepthStateDisable, NULL );

}



// ���s���e�p�̃J����
void Renderer::SetWorldViewProjection2D()
{
	Matrix world;
	world = Matrix::Identity;			// �P�ʍs��ɂ���
	world = world.Transpose();			// �]�u
	m_DeviceContext->UpdateSubresource(m_WorldBuffer, 0, NULL, &world, 0, 0);

	Matrix view;
	view = Matrix::Identity;			// �P�ʍs��ɂ���
	view = view.Transpose();			// �]�u
	m_DeviceContext->UpdateSubresource(m_ViewBuffer, 0, NULL, &view, 0, 0);

	Matrix projection;

	// 2D�`������㌴�_�ɂ���
	projection = DirectX::XMMatrixOrthographicOffCenterLH(
		0.0f,
		static_cast<float>(Application::GetWidth()),		// �r���[�{�����[���̍ŏ��w
		static_cast<float>(Application::GetHeight()),		// �r���[�{�����[���̍ŏ��x
		0.0f,												// �r���[�{�����[���̍ő�x
		0.0f,
		1.0f);

	projection = projection.Transpose();


	m_DeviceContext->UpdateSubresource( m_ProjectionBuffer, 0, NULL, &projection, 0, 0 );
}



// ���[���h�ϊ��s��̃Z�b�g
void Renderer::SetWorldMatrix( MatrixInfo* WorldMatrix )
{
	MatrixInfo world;

	// �]�u
	world.Mat = WorldMatrix->Mat.Transpose();
	world.InvMat = WorldMatrix->InvMat.Transpose();
	world.PrevMat = WorldMatrix->PrevMat.Transpose();

	m_DeviceContext->UpdateSubresource(m_WorldBuffer, 0, NULL, &world, 0, 0);

}
// �r���[�ϊ��s��̃Z�b�g
void Renderer::SetViewMatrix( MatrixInfo* ViewMatrix )
{
	MatrixInfo view;

	// �]�u
	view.Mat = ViewMatrix->Mat.Transpose();
	view.InvMat = ViewMatrix->InvMat.Transpose();
	view.PrevMat = ViewMatrix->PrevMat.Transpose();

	m_DeviceContext->UpdateSubresource(m_ViewBuffer, 0, NULL, &view, 0, 0);
}
// �v���W�F�N�V�����ϊ��s��̃Z�b�g
void Renderer::SetProjectionMatrix( MatrixInfo* ProjectionMatrix )
{
	MatrixInfo projection;

	// �]�u
	projection.Mat = ProjectionMatrix->Mat.Transpose();
	projection.InvMat = ProjectionMatrix->InvMat.Transpose();
	projection.PrevMat = ProjectionMatrix->PrevMat.Transpose();


	m_DeviceContext->UpdateSubresource(m_ProjectionBuffer, 0, NULL, &projection, 0, 0);
}
// �}�e���A���̃Z�b�g
void Renderer::SetMaterial( MATERIAL Material )
{
	m_DeviceContext->UpdateSubresource( m_MaterialBuffer, 0, NULL, &Material, 0, 0 );
}
// �J�����̃Z�b�g
void Renderer::SetCamera(Camera camera)
{
	CAMERAPARAM param;

	param.Position = camera.GetPosition();
	param.nearClip = camera.GetNearClip();
	param.farClip = camera.GetFarClip();

	m_DeviceContext->UpdateSubresource(m_CameraBuffer, 0, NULL, &param, 0, 0);
}
// �p�����[�^�̃Z�b�g
void Renderer::SetExtraParam(ExtraParam param)
{

	m_DeviceContext->UpdateSubresource(m_ParamBuffer, 0, NULL, &param, 0, 0);
}



// �T���v���[�X�e�[�g�̃Z�b�g
void Renderer::SetSamplerState(float LodBias)
{
	//---------------------------------------
	// �e�N�X�`���T���v���[�is0�j
	//---------------------------------------	
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 16;  // �ٕ����t�B���^�̋��x
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.MipLODBias = LodBias;

	// �T���v���[�X�e�[�g�̍쐬�E�Z�b�g
	ID3D11SamplerState* samplerState{};
	m_Device->CreateSamplerState(&samplerDesc, &samplerState);
	m_DeviceContext->PSSetSamplers(0, 1, &samplerState);

	//---------------------------------------
	// �V���h�E�p�T���v���[�is1�j
	//---------------------------------------

	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT; // ��r�t���t�B���^
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL; // �[�x��r�̊
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.MaxAnisotropy = 1;

	m_Device->CreateSamplerState(&samplerDesc, &samplerState);
	m_DeviceContext->PSSetSamplers(1, 1, &samplerState);

	// ���
	samplerState->Release();
	samplerState = nullptr;
}
// �r���[�|�[�g�̃Z�b�g
void Renderer::SetViewPort(DirectX::XMUINT2 renderSize)
{
	D3D11_VIEWPORT viewport;
	viewport.Width = (FLOAT)renderSize.x;
	viewport.Height = (FLOAT)renderSize.y;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	m_DeviceContext->RSSetViewports(1, &viewport);

}



// ���_�V�F�[�_�[�̍쐬
void Renderer::CreateVertexShader( ID3D11VertexShader** VertexShader, ID3D11InputLayout** VertexLayout, const char* FileName )
{

	FILE* file;
	long int fsize;

	fopen_s(&file,FileName, "rb");
	assert(file);

	fsize = _filelength(_fileno(file));
	unsigned char* buffer = new unsigned char[fsize];
	fread(buffer, fsize, 1, file);
	fclose(file);

	m_Device->CreateVertexShader(buffer, fsize, NULL, VertexShader);


	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0,	0,		D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,		0,	4 * 3,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	4 * 6,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0,	4 * 10, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);

	m_Device->CreateInputLayout(layout,
		numElements,
		buffer,
		fsize,
		VertexLayout);

	delete[] buffer;
}
// �s�N�Z���V�F�[�_�[�̍쐬
void Renderer::CreatePixelShader( ID3D11PixelShader** PixelShader, const char* FileName )
{
	FILE* file;
	long int fsize;

	fopen_s(&file, FileName, "rb");
	assert(file);

	fsize = _filelength(_fileno(file));
	unsigned char* buffer = new unsigned char[fsize];
	fread(buffer, fsize, 1, file);
	fclose(file);

	m_Device->CreatePixelShader(buffer, fsize, NULL, PixelShader);

	delete[] buffer;
}



// �J�����O���[�h��ݒ�
void Renderer::SetCullMode(CULLMODE type)
{
	switch (type)
	{
	case CULLMODE::BACK:
		m_DeviceContext->RSSetState(m_RsCullBack);
		break;
	case CULLMODE::FRONT:
		m_DeviceContext->RSSetState(m_RsCullFront);
		break;
	case CULLMODE::NONE:
		m_DeviceContext->RSSetState(m_RsCullNone);
		break;
	}

}

// �f�t�H���g�̃����_�[�^�[�Q�b�g���N���A�E�Z�b�g
void Renderer::SetDefaultRenderTarget(
	bool _isClearRTV,
	ID3D11DepthStencilView* _dsv,
	bool _isClearDSV)
{
	// �����_�[�^�[�Q�b�g�̃Z�b�g
	m_DeviceContext->OMSetRenderTargets(1, &m_DefaultRTV, _dsv);

	// �����_�[�^�[�Q�b�g�̃N���A
	if (_isClearRTV)
	{
		// �F�ŃN���A
		float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
		// RTV���N���A
		m_DeviceContext->ClearRenderTargetView(m_DefaultRTV, clearColor);
	}
	// DSV�̃N���A
	if (_isClearDSV)
	{
		m_DeviceContext->ClearDepthStencilView(_dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

}

// �V���h�E�}�b�v�pDSV�̃Z�b�g�E�N���A
void Renderer::SetShadowMapDSV(ID3D11DepthStencilView* _dsv, bool _isClearDSV)
{
	if (_dsv == nullptr)
	{
		cout << "DSV��nullptr�ł�" << endl;
		return;
	}

	// �����_�[�^�[�Q�b�g�̃Z�b�g
	m_DeviceContext->OMSetRenderTargets(0, nullptr, _dsv);

	// DSV�̃N���A
	if (_isClearDSV)
	{
		m_DeviceContext->ClearDepthStencilView(_dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

}



// �A���t�@�u�����h�̐ݒ�
void Renderer::SetAlphaBlend(bool type)
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	if (type)
		m_DeviceContext->OMSetBlendState(m_BSAlphaOn, blendFactor, 0xffffffff);
	else
		m_DeviceContext->OMSetBlendState(m_BSAlphaOff, blendFactor, 0xffffffff);
}



