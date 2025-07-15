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

const UINT GBUFFER_NUM = 5;		// Gバッファの数（Albedo, Normal, WorldPos, Depth, MVector）
const UINT PINGPONG_NUM = 2;	// ポストプロセス用リソースの数
const UINT BLUR_NUM = 2;		// ブラー抽出を行う回数

const UINT TREE_NUM = 3;		// 表示する木の数
const UINT COTTAGE_NUM = 10;		// 表示する家の数
const UINT LAMP_NUM = 3;		// 表示するランプの数

// シャドウマップの解像度
const UINT SHADOW_MAP_WIDTH = 1024 * 2;
const UINT SHADOW_MAP_HEIGHT = 1024 * 2;

// ポストプロセスの適用切り替えフラグ
struct PPparam
{
	BOOL isDoF = true;		// 被写界深度
	BOOL isBloom = true;	// ブルーム
	BOOL isVignette = true;	// ビネット
	BOOL isPPAll = true;	// 全てのポストプロセス
};

// シーンデモクラス
class SceneDemo
{
public:		// メンバ関数

	// シーン描画
	void SceneUpdate();		// 更新
	void SceneInit();		// 初期化
	void SceneDraw();		// 描画
	void SceneDispose();	// 終了

private:

	// 各種更新
	void UpdateDLSS();		// DLSSの更新処理
	void UpdateLight();		// ライトの更新処理

	// リソース生成
	void CreateRenderResource();								// フォワードレンダリング
	void CreateDeferredResource();								// ディファードレンダリング
	void CreateDLSSResource(DirectX::XMUINT2 renderSize =		// DLSS
		{ Application::GetWidth(),Application::GetHeight() });
	void CreateShadowResource();								// シャドウマップ
	void CreateHDRResource();									// HDR
	void CreatePostProcessResource();							// ポストプロセス

	// リソースのセット
	void SetDeferredGBufferRenderTarget();	// GBuffer用のレンダーターゲット
	void SetDeferredShaderResource();		// ディファードレンダリング用のシェーダーリソース
	void SetDLSSRenderTarget();				// DLSS用のレンダーターゲット
	void SetDLSSShaderResource();			// DLSS用のシェーダーリソース
	void SetHDRRenderTarget(
	bool _isClearRTV = true,
	ID3D11DepthStencilView* _dsv = nullptr,
	bool _isClearDSV = false);				// HDRのレンダーターゲット
	void SetHDRResource();					// HDRのシェーダーリソース
	void SetDLSSOutputResource();			// アップスケールの出力結果
	void SetShadowResource();				// シャドウイング後のシェーダーリソース

	void SetShadowMap();					// シャドウマップ

	void ChangePingPongResource();			// PingPongシェーダー切り替え
	void ClearPingPongRenderTarget();		// ポストプロセスのレンダーターゲットをリセット

	void ClearBlurRenderTarget();			// ブラー用レンダーターゲットをクリア
	void SetBlurXResource(
		ID3D11ShaderResourceView* _srv,
		float _blurX);						// 横方向のブラー処理時のリソースをセット
	void SetBlurYResource(
		ID3D11ShaderResourceView* _srv,
		float _blurY);						// 縦方向のブラー処理時のリソースをセット
	void SetDoFResource();					// 被写界深度適用のリソースをセット
	void SetLuminanceResource();			// 輝度抽出用リソースをセット
	void SetBloomResource();				// ブルームライティング用リソースをセット

private:	// メンバ変数

	//-------------------------------------
	// シェーダー
	//-------------------------------------
	Shader m_ShaderUnlit;				// Unlitライティング
	Shader m_ShaderLit;					// Litシェーディング
	Shader m_ShaderDeferredRendering;	// ディファードレンダリング
	Shader m_ShaderGBuffer;				// GBuffer出力
	Shader m_ShaderDLSSInput;			// DLSSに渡す入力（オフスクリーン）
	Shader m_ShaderDLSSOutput;			// DLSSアップスケール後の出力
	Shader m_ShaderSkySphere;			// SkySphere用
	Shader m_ShaderShadowMap;			// シャドウマップ
	Shader m_ShaderShadowAlphaClip;		// シャドウマップ（αクリップ）
	Shader m_ShaderShadow;				// シャドウイング
	Shader m_ShaderOutline;				// 輪郭線
	Shader m_ShaderDoF;					// 被写界深度
	Shader m_ShaderBlur;				// ブラー抽出
	Shader m_ShaderLuminance;			// 輝度抽出（ブルーム用）
	Shader m_ShaderBloom;				// ブルーム
	Shader m_ShaderVignette;			// ビネット
	Shader m_ShaderToneMapping;			// トーンマッピング

	//-------------------------------------
	// シーンオブジェクト
	//-------------------------------------
	Camera		m_Camera;	// カメラ

	Field		m_Moss;		// フィールド（草）
	Field		m_Dirt;		// フィールド（土）
	Field		m_TileA;	// フィールド（石）
	Field		m_TileB;	// フィールド（石）

	OffScreen	m_Screen;				// スクリーンクアッド
	Sprite		m_UnityChan;			// ユニティちゃん
	Sprite		m_Lamp[POINTLIGHT_NUM];	// ランプ
	Sprite		m_TreeA[TREE_NUM];		// 木A
	Sprite		m_TreeB[TREE_NUM];		// 木B

	StaticMesh			m_Skysphere;	// SkySphere
	StaticMeshRenderer	m_MRSkysphere;	// レンダラー

	StaticMesh			m_Cottage[COTTAGE_NUM];		// Cottage
	StaticMeshRenderer	m_MRCottage[COTTAGE_NUM];	// レンダラー

	StaticMesh			m_Mountain;		// 山
	StaticMeshRenderer	m_MRMountain;	// レンダラー

	StaticMesh			m_LightObj[POINTLIGHT_NUM];		// 点光源可視化用オブジェクト
	StaticMeshRenderer	m_MRLightObj[POINTLIGHT_NUM];	// レンダラー

	//-------------------------------------
	// レンダーターゲット（RTV）
	//-------------------------------------
	ID3D11RenderTargetView*		m_DeferredGBufferRTVs[GBUFFER_NUM] = { nullptr };	// Gバッファ（ディファードレンダリング）
	ID3D11RenderTargetView*		m_DLSSGBufferRTVs[GBUFFER_NUM] = { nullptr };		// Gバッファ（DLSS）
	ID3D11RenderTargetView*		m_DLSSInputRTV = nullptr;							// DLSS入力値（オフスクリーン）
	ID3D11RenderTargetView*		m_hdrRTV = nullptr;									// HDR（ポストプロセス用）
	ID3D11RenderTargetView*		m_ShadowRTV = nullptr;								// シャドウ

	ID3D11RenderTargetView*		m_PingPongRTV[PINGPONG_NUM] = { nullptr };			// ポストプロセス（ピンポンシェーダー）
	ID3D11RenderTargetView*		m_LuminanceRTV = nullptr;							// 輝度抽出
	ID3D11RenderTargetView*		m_BlurRTV[BLUR_NUM] = { nullptr };					// ブラー抽出

	//-------------------------------------
	// シェーダーリソース（SRV）
	//-------------------------------------
	ID3D11ShaderResourceView*	m_DeferredGBufferSRVs[GBUFFER_NUM] = { nullptr };	// Gバッファ（ディファードレンダリング）
	ID3D11ShaderResourceView*	m_DLSSGBufferSRVs[GBUFFER_NUM] = { nullptr };		// Gバッファ（DLSS）
	ID3D11ShaderResourceView*	m_DLSSInputSRV = nullptr;							// DLSS入力値（オフスクリーン）
	ID3D11ShaderResourceView*	m_DLSSOutputSRV = nullptr;							// DLSS出力値

	ID3D11ShaderResourceView*	m_DLSSDepthSRV = nullptr;							// DLSS深度情報
	ID3D11ShaderResourceView*	m_ShadowMapSRV = nullptr;							// シャドウマップ
	ID3D11ShaderResourceView*	m_hdrSRV = nullptr;									// HDR（ポストプロセス用）
	ID3D11ShaderResourceView*	m_ShadowSRV = nullptr;								// シャドウ

	ID3D11ShaderResourceView*	m_PingPongSRV[PINGPONG_NUM] = { nullptr };			// ポストプロセス（ピンポンシェーダー）
	ID3D11ShaderResourceView*	m_WriteDepthSRV = nullptr;							// 深度値書き込み用
	ID3D11ShaderResourceView*	m_LuminanceSRV = nullptr;							// 輝度抽出
	ID3D11ShaderResourceView*	m_BlurSRV[BLUR_NUM] = { nullptr };					// ブラー抽出

	//-------------------------------------
	// 深度ステンシル（DSV）
	//-------------------------------------
	ID3D11DepthStencilView*		m_DLSSDepthDSV = nullptr;							// DLSS深度情報
	ID3D11DepthStencilView*		m_ShadowMapDSV = nullptr;							// シャドウマップ
	ID3D11DepthStencilView*		m_WriteDepthDSV = nullptr;							// 深度値書き込み用


	//-------------------------------------
	// 書き込みリソース（UAV）
	//-------------------------------------
	ID3D11UnorderedAccessView*	m_DLSSOutputUAV = nullptr;							// DLSS出力値

	//-------------------------------------
	// バッファリソース（Texture）
	//-------------------------------------
	ID3D11Texture2D*			m_DeferredGBufferTexs[GBUFFER_NUM]= { nullptr };	// Gバッファ（ディファードレンダリング）
	ID3D11Texture2D*			m_DLSSGBufferTexs[GBUFFER_NUM]	= { nullptr };		// Gバッファ（DLSS）
	ID3D11Texture2D*			m_DLSSInputTex = nullptr;							// DLSS入力値（オフスクリーン）
	ID3D11Texture2D*			m_DLSSOutputTex = nullptr;							// DLSS出力値

	ID3D11Texture2D*			m_DLSSDepthTex = nullptr;							// DLSS深度情報
	ID3D11Texture2D*			m_ShadowMapTex = nullptr;							// シャドウマップ

	ID3D11Texture2D*			m_hdrTex = nullptr;									// HDR（ポストプロセス用）
	ID3D11Texture2D*			m_ShadowTex = nullptr;								// シャドウイング用

	ID3D11Texture2D*			m_PingPongTex[PINGPONG_NUM] = { nullptr };			// ポストプロセス（ピンポンシェーダー）
	ID3D11Texture2D*			m_WriteDepthTex = nullptr;							// 深度値書き込み用
	ID3D11Texture2D*			m_LuminanceTex = nullptr;							// 輝度抽出
	ID3D11Texture2D*			m_BlurTex[BLUR_NUM] = { nullptr };					// ブラー抽出
};


