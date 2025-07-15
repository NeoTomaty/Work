#pragma once
#include <DirectXMath.h>
#include <SimpleMath.h>
#include <memory>
#include "Shader.h"
#include "DirectInput.h"
#include "debugui.h"
#include "camera.h"
#include "Field.h"
#include "SceneDemo.h"
#include "StaticMesh.h"
#include "StaticMeshRenderer.h"
#include "Renderer.h"
#include "Application.h"
#include "OffScreen.h"
#include "Sprite.h"
#include "SceneLight.h"

const UINT GBUFFER_NUM = 5;		// G�o�b�t�@�̐��iAlbedo, Normal, WorldPos, Depth, MVector�j
const UINT PINGPONG_NUM = 2;	// �|�X�g�v���Z�X�p���\�[�X�̐�
const UINT BLUR_NUM = 2;		// �u���[���o���s����

const UINT TREE_NUM = 3;		// �\������؂̐�
const UINT COTTAGE_NUM = 10;		// �\������Ƃ̐�
const UINT LAMP_NUM = 3;		// �\�����郉���v�̐�

// �V���h�E�}�b�v�̉𑜓x
const UINT SHADOW_MAP_WIDTH = 1024 * 2;
const UINT SHADOW_MAP_HEIGHT = 1024 * 2;

// �|�X�g�v���Z�X�̓K�p�؂�ւ��t���O
struct PPparam
{
	BOOL isDoF = true;		// ��ʊE�[�x
	BOOL isBloom = true;	// �u���[��
	BOOL isVignette = true;	// �r�l�b�g
	BOOL isPPAll = true;	// �S�Ẵ|�X�g�v���Z�X
};

// �V�[���f���N���X
class SceneDemo
{
public:		// �����o�֐�

	// �V�[���`��
	void SceneUpdate();		// �X�V
	void SceneInit();		// ������
	void SceneDraw();		// �`��
	void SceneDispose();	// �I��

private:

	// �e��X�V
	void UpdateDLSS();		// DLSS�̍X�V����
	void UpdateLight();		// ���C�g�̍X�V����

	// ���\�[�X����
	void CreateRenderResource();								// �t�H���[�h�����_�����O
	void CreateDeferredResource();								// �f�B�t�@�[�h�����_�����O
	void CreateDLSSResource(DirectX::XMUINT2 renderSize =		// DLSS
		{ Application::GetWidth(),Application::GetHeight() });
	void CreateShadowResource();								// �V���h�E�}�b�v
	void CreateHDRResource();									// HDR
	void CreatePostProcessResource();							// �|�X�g�v���Z�X

	// ���\�[�X�̃Z�b�g
	void SetDeferredGBufferRenderTarget();	// GBuffer�p�̃����_�[�^�[�Q�b�g
	void SetDeferredShaderResource();		// �f�B�t�@�[�h�����_�����O�p�̃V�F�[�_�[���\�[�X
	void SetDLSSRenderTarget();				// DLSS�p�̃����_�[�^�[�Q�b�g
	void SetDLSSShaderResource();			// DLSS�p�̃V�F�[�_�[���\�[�X
	void SetHDRRenderTarget(
	bool _isClearRTV = true,
	ID3D11DepthStencilView* _dsv = nullptr,
	bool _isClearDSV = false);				// HDR�̃����_�[�^�[�Q�b�g
	void SetHDRResource();					// HDR�̃V�F�[�_�[���\�[�X
	void SetDLSSOutputResource();			// �A�b�v�X�P�[���̏o�͌���
	void SetShadowResource();				// �V���h�E�C���O��̃V�F�[�_�[���\�[�X

	void SetShadowMap();					// �V���h�E�}�b�v

	void ChangePingPongResource();			// PingPong�V�F�[�_�[�؂�ւ�
	void ClearPingPongRenderTarget();		// �|�X�g�v���Z�X�̃����_�[�^�[�Q�b�g�����Z�b�g

	void ClearBlurRenderTarget();			// �u���[�p�����_�[�^�[�Q�b�g���N���A
	void SetBlurXResource(
		ID3D11ShaderResourceView* _srv,
		float _blurX);						// �������̃u���[�������̃��\�[�X���Z�b�g
	void SetBlurYResource(
		ID3D11ShaderResourceView* _srv,
		float _blurY);						// �c�����̃u���[�������̃��\�[�X���Z�b�g
	void SetDoFResource();					// ��ʊE�[�x�K�p�̃��\�[�X���Z�b�g
	void SetLuminanceResource();			// �P�x���o�p���\�[�X���Z�b�g
	void SetBloomResource();				// �u���[�����C�e�B���O�p���\�[�X���Z�b�g

private:	// �����o�ϐ�

	//-------------------------------------
	// �V�F�[�_�[
	//-------------------------------------
	Shader m_ShaderUnlit;				// Unlit���C�e�B���O
	Shader m_ShaderLit;					// Lit�V�F�[�f�B���O
	Shader m_ShaderDeferredRendering;	// �f�B�t�@�[�h�����_�����O
	Shader m_ShaderGBuffer;				// GBuffer�o��
	Shader m_ShaderDLSSInput;			// DLSS�ɓn�����́i�I�t�X�N���[���j
	Shader m_ShaderDLSSOutput;			// DLSS�A�b�v�X�P�[����̏o��
	Shader m_ShaderSkySphere;			// SkySphere�p
	Shader m_ShaderShadowMap;			// �V���h�E�}�b�v
	Shader m_ShaderShadowAlphaClip;		// �V���h�E�}�b�v�i���N���b�v�j
	Shader m_ShaderShadow;				// �V���h�E�C���O
	Shader m_ShaderOutline;				// �֊s��
	Shader m_ShaderDoF;					// ��ʊE�[�x
	Shader m_ShaderBlur;				// �u���[���o
	Shader m_ShaderLuminance;			// �P�x���o�i�u���[���p�j
	Shader m_ShaderBloom;				// �u���[��
	Shader m_ShaderVignette;			// �r�l�b�g
	Shader m_ShaderToneMapping;			// �g�[���}�b�s���O

	//-------------------------------------
	// �V�[���I�u�W�F�N�g
	//-------------------------------------
	Camera		m_Camera;	// �J����

	Field		m_Moss;		// �t�B�[���h�i���j
	Field		m_Dirt;		// �t�B�[���h�i�y�j
	Field		m_TileA;	// �t�B�[���h�i�΁j
	Field		m_TileB;	// �t�B�[���h�i�΁j

	OffScreen	m_Screen;				// �X�N���[���N�A�b�h
	Sprite		m_UnityChan;			// ���j�e�B�����
	Sprite		m_Lamp[POINTLIGHT_NUM];	// �����v
	Sprite		m_TreeA[TREE_NUM];		// ��A
	Sprite		m_TreeB[TREE_NUM];		// ��B

	StaticMesh			m_Skysphere;	// SkySphere
	StaticMeshRenderer	m_MRSkysphere;	// �����_���[

	StaticMesh			m_Cottage[COTTAGE_NUM];		// Cottage
	StaticMeshRenderer	m_MRCottage[COTTAGE_NUM];	// �����_���[

	StaticMesh			m_Mountain;		// �R
	StaticMeshRenderer	m_MRMountain;	// �����_���[

	StaticMesh			m_LightObj[POINTLIGHT_NUM];		// �_���������p�I�u�W�F�N�g
	StaticMeshRenderer	m_MRLightObj[POINTLIGHT_NUM];	// �����_���[

	//-------------------------------------
	// �����_�[�^�[�Q�b�g�iRTV�j
	//-------------------------------------
	ID3D11RenderTargetView*		m_DeferredGBufferRTVs[GBUFFER_NUM] = { nullptr };	// G�o�b�t�@�i�f�B�t�@�[�h�����_�����O�j
	ID3D11RenderTargetView*		m_DLSSGBufferRTVs[GBUFFER_NUM] = { nullptr };		// G�o�b�t�@�iDLSS�j
	ID3D11RenderTargetView*		m_DLSSInputRTV = nullptr;							// DLSS���͒l�i�I�t�X�N���[���j
	ID3D11RenderTargetView*		m_hdrRTV = nullptr;									// HDR�i�|�X�g�v���Z�X�p�j
	ID3D11RenderTargetView*		m_ShadowRTV = nullptr;								// �V���h�E

	ID3D11RenderTargetView*		m_PingPongRTV[PINGPONG_NUM] = { nullptr };			// �|�X�g�v���Z�X�i�s���|���V�F�[�_�[�j
	ID3D11RenderTargetView*		m_LuminanceRTV = nullptr;							// �P�x���o
	ID3D11RenderTargetView*		m_BlurRTV[BLUR_NUM] = { nullptr };					// �u���[���o

	//-------------------------------------
	// �V�F�[�_�[���\�[�X�iSRV�j
	//-------------------------------------
	ID3D11ShaderResourceView*	m_DeferredGBufferSRVs[GBUFFER_NUM] = { nullptr };	// G�o�b�t�@�i�f�B�t�@�[�h�����_�����O�j
	ID3D11ShaderResourceView*	m_DLSSGBufferSRVs[GBUFFER_NUM] = { nullptr };		// G�o�b�t�@�iDLSS�j
	ID3D11ShaderResourceView*	m_DLSSInputSRV = nullptr;							// DLSS���͒l�i�I�t�X�N���[���j
	ID3D11ShaderResourceView*	m_DLSSOutputSRV = nullptr;							// DLSS�o�͒l

	ID3D11ShaderResourceView*	m_DLSSDepthSRV = nullptr;							// DLSS�[�x���
	ID3D11ShaderResourceView*	m_ShadowMapSRV = nullptr;							// �V���h�E�}�b�v
	ID3D11ShaderResourceView*	m_hdrSRV = nullptr;									// HDR�i�|�X�g�v���Z�X�p�j
	ID3D11ShaderResourceView*	m_ShadowSRV = nullptr;								// �V���h�E

	ID3D11ShaderResourceView*	m_PingPongSRV[PINGPONG_NUM] = { nullptr };			// �|�X�g�v���Z�X�i�s���|���V�F�[�_�[�j
	ID3D11ShaderResourceView*	m_WriteDepthSRV = nullptr;							// �[�x�l�������ݗp
	ID3D11ShaderResourceView*	m_LuminanceSRV = nullptr;							// �P�x���o
	ID3D11ShaderResourceView*	m_BlurSRV[BLUR_NUM] = { nullptr };					// �u���[���o

	//-------------------------------------
	// �[�x�X�e���V���iDSV�j
	//-------------------------------------
	ID3D11DepthStencilView*		m_DLSSDepthDSV = nullptr;							// DLSS�[�x���
	ID3D11DepthStencilView*		m_ShadowMapDSV = nullptr;							// �V���h�E�}�b�v
	ID3D11DepthStencilView*		m_WriteDepthDSV = nullptr;							// �[�x�l�������ݗp


	//-------------------------------------
	// �������݃��\�[�X�iUAV�j
	//-------------------------------------
	ID3D11UnorderedAccessView*	m_DLSSOutputUAV = nullptr;							// DLSS�o�͒l

	//-------------------------------------
	// �o�b�t�@���\�[�X�iTexture�j
	//-------------------------------------
	ID3D11Texture2D*			m_DeferredGBufferTexs[GBUFFER_NUM]= { nullptr };	// G�o�b�t�@�i�f�B�t�@�[�h�����_�����O�j
	ID3D11Texture2D*			m_DLSSGBufferTexs[GBUFFER_NUM]	= { nullptr };		// G�o�b�t�@�iDLSS�j
	ID3D11Texture2D*			m_DLSSInputTex = nullptr;							// DLSS���͒l�i�I�t�X�N���[���j
	ID3D11Texture2D*			m_DLSSOutputTex = nullptr;							// DLSS�o�͒l

	ID3D11Texture2D*			m_DLSSDepthTex = nullptr;							// DLSS�[�x���
	ID3D11Texture2D*			m_ShadowMapTex = nullptr;							// �V���h�E�}�b�v

	ID3D11Texture2D*			m_hdrTex = nullptr;									// HDR�i�|�X�g�v���Z�X�p�j
	ID3D11Texture2D*			m_ShadowTex = nullptr;								// �V���h�E�C���O�p

	ID3D11Texture2D*			m_PingPongTex[PINGPONG_NUM] = { nullptr };			// �|�X�g�v���Z�X�i�s���|���V�F�[�_�[�j
	ID3D11Texture2D*			m_WriteDepthTex = nullptr;							// �[�x�l�������ݗp
	ID3D11Texture2D*			m_LuminanceTex = nullptr;							// �P�x���o
	ID3D11Texture2D*			m_BlurTex[BLUR_NUM] = { nullptr };					// �u���[���o
};


