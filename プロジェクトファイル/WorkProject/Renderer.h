#pragma once

#include	<d3d11.h>
#include	<DirectXMath.h>
#include	<SimpleMath.h>
#include	<io.h>
#include	<string>
#include	<vector>
#include	"NonCopyable.h"
#include	"AssimpPerse.h"
#include	"Application.h"

// 外部ライブラリ
#pragma comment(lib,"directxtk.lib")
#pragma comment(lib,"d3d11.lib")

//-------------------------------------
// 列挙型
//-------------------------------------
// カリング
enum CULLMODE
{
	BACK = 0,
	FRONT,
	NONE,
};
// 深度バッファタイプ
enum class DEPTHTYPE
{
	DEFAULT,	// デフォルト深度バッファ（フォワードレンダリング）
	DEFERRED,	// ディファードレンダリング用深度バッファ
	DLSS,		// DLSSスーパーサンプリング用深度バッファ
	NONE,		// なし（nullptr）
};
// 出力するテクスチャタイプ
enum class TEXTURETYPE
{
	COLOR,
	NORMAL,
	WORLD,
	DEPTH,
	MVECTOR,
	OUTPUT,
};
// 平行光源の種類
enum class DLIGHTTYPE
{
	FREE,		// 指定なし
	MORNING,	// 朝
	NOON,		// 昼
	EVENING,	// 夕
	NIGHT,		// 夜
};

//-------------------------------------
// 構造体（主にバッファバインド用）
//-------------------------------------
// 3D頂点データ
struct VERTEX_3D
{
	DirectX::SimpleMath::Vector3	Position;
	DirectX::SimpleMath::Vector3	Normal;
	DirectX::SimpleMath::Color		Diffuse;
	DirectX::SimpleMath::Vector2	TexCoord;
	DirectX::SimpleMath::Vector3	Tangent;
};
// マテリアル
struct MATERIAL
{
	DirectX::SimpleMath::Color	Ambient;
	DirectX::SimpleMath::Color	Diffuse;
	DirectX::SimpleMath::Color	Specular;
	DirectX::SimpleMath::Color	Emission;
	float Shiness;
	float Alpha;
	float Dummy[2]{};
};
// カメラ
struct CAMERAPARAM
{
	DirectX::SimpleMath::Vector3 Position;	// カメラの位置
	float Padding;							// 16バイトアラインメントを確保するためのパディング

	float nearClip;		// ニアクリップ
	float farClip;		// ファークリップ
	float Padding4[2];	// パディング
};
// その他パラメータ
struct ExtraParam
{
	int		UseNormalMap = 0;	// 法線マップを使用するかどうか
	int		TextureType = 0;	// Gバッファを出力する際のテクスチャタイプ
	int		isShadow = 1;		// シャドウのON/OFF
	int		Padding2;

	float	blurParam[4] = {
		0.0f,0.0f,				// ブラーを行う方向のテクスチャ
		1.0f,0.0f				// ブラー方向
	};

	float focus = 80.0f;			// カメラの焦点距離
	float DoFRange = 60.0f;			// 被写界深度範囲

	float vignetteStrength = 0.7f;			// ビネットの強さ
	float vignetteRadius = 0.2f;			// ビネットの開始半径
	float vignetteCenter[2] = { 0.5f,0.35f };// ビネットの中心

	float bloomStrength = 0.10f;	// ブルーム発光の強さ
	float padding3;
};
// メッシュ情報
struct SUBSET {
	std::string		MtrlName;						// マテリアル名
	unsigned int	IndexNum = 0;					// インデックス数
	unsigned int	VertexNum = 0;					// 頂点数
	unsigned int	IndexBase = 0;					// 開始インデックス
	unsigned int	VertexBase = 0;					// 頂点ベース
	unsigned int	MaterialIdx = 0;				// マテリアルインデックス
};
// 変換行列
struct MatrixInfo
{
	DirectX::SimpleMath::Matrix Mat;		// 現フレームの行列
	DirectX::SimpleMath::Matrix InvMat;		// 現フレームの逆行列
	DirectX::SimpleMath::Matrix PrevMat;	// 前フレームの行列
};


class Camera;

// レンダラークラス
class Renderer : NonCopyable
{
private:

	// パラメータ
	static D3D_FEATURE_LEVEL		m_FeatureLevel;
	static IDXGIFactory*			m_Factory;
	static IDXGIAdapter*			m_Adapter;

	// 基本機能
	static ID3D11Device*			m_Device;
	static ID3D11DeviceContext*		m_DeviceContext;
	static IDXGISwapChain*			m_SwapChain;

	// デフォルトのレンダリングリソース
	static ID3D11RenderTargetView*	m_DefaultRTV;

	// 定数バッファ
	static ID3D11Buffer*			m_WorldBuffer;
	static ID3D11Buffer*			m_ViewBuffer;
	static ID3D11Buffer*			m_ProjectionBuffer;
	static ID3D11Buffer*			m_MaterialBuffer;
	static ID3D11Buffer*			m_CameraBuffer;
	static ID3D11Buffer*			m_ParamBuffer;

	// 深度ステンシルステート
	static ID3D11DepthStencilState* m_DepthStateEnable;
	static ID3D11DepthStencilState* m_DepthStateDisable;

	// ラスタライザーステート
	static ID3D11RasterizerState* m_RsCullFront;	// カリング（表を描画しない）
	static ID3D11RasterizerState* m_RsCullBack;		// カリング（裏を描画しない）
	static ID3D11RasterizerState* m_RsCullNone;		// カリング（カリングなし）

	// ブレンドステート
	static ID3D11BlendState* m_BSAlphaOff;	// アルファブレンドなし
	static ID3D11BlendState* m_BSAlphaOn;	// アルファブレンドあり

	static bool				m_IsDisableNvidia;		// NVIDIA_GPUが使用可能か？
	static DirectX::XMUINT2	m_InputRenderSize;		// アップスケール前のレンダーターゲットサイズ（ウィンドウサイズ）
	static DirectX::XMUINT2	m_OutputRenderSize;		// アップスケール後のレンダーターゲットサイズ（3840x2160）（4K）


public:

	// 描画
	static void Init();
	static void Uninit();
	static void Begin();
	static void End();

	// 深度値の可否
	static void SetDepthEnable(bool Enable);

	// 各種リソースのセッタ
	static void SetWorldViewProjection2D();
	static void SetWorldMatrix(MatrixInfo* WorldMatrix);
	static void SetViewMatrix(MatrixInfo* ViewMatrix);
	static void SetProjectionMatrix(MatrixInfo* ProjectionMatrix);
	static void SetMaterial(MATERIAL Material);
	static void SetCamera(Camera camera);
	static void SetExtraParam(ExtraParam param);

	static void SetSamplerState(float LodBias = 1.0f);
	static void SetViewPort(DirectX::XMUINT2 renderSize = 
		{ Application::GetWidth(),Application::GetHeight() });

	// ゲッタ
	static ID3D11Device*			GetDevice()			{ return m_Device; }
	static ID3D11DeviceContext*		GetDeviceContext()	{ return m_DeviceContext; }
	static bool						GetIsAbleNVIDIA()	{ return m_IsDisableNvidia; }
	static IDXGIAdapter*			GetAdapter()		{ return m_Adapter; }
	static ID3D11RenderTargetView*	GetDefaultRTV()		{ return m_DefaultRTV; }

	// レンダリングサイズのセッタ・ゲッタ
	static DirectX::XMUINT2		GetInputRenderSize()	{ return m_InputRenderSize; }
	static DirectX::XMUINT2		GetOutputRenderSize()	{ return m_OutputRenderSize; }
	static void					SetInputRenderSize(DirectX::XMUINT2 inputSize)	
														{ m_InputRenderSize = inputSize; }
	static void					SetOutputRenderSize(DirectX::XMUINT2 outputSize)	
														{ m_OutputRenderSize = outputSize; }
	// シェーダー作成
	static void					CreateVertexShader(ID3D11VertexShader** VertexShader, ID3D11InputLayout** VertexLayout, const char* FileName);
	static void					CreatePixelShader(ID3D11PixelShader** PixelShader, const char* FileName);

	// カリングモードを変更
	static void					SetCullMode(CULLMODE type);

	// デフォルトのレンダーターゲットをクリア・セット
	static void					SetDefaultRenderTarget(bool _isClearRTV = true, 
													ID3D11DepthStencilView* _dsv = nullptr,
													bool _isClearDSV = false);
	
	// シャドウマップ用DSVのセット・クリア
	static void					SetShadowMapDSV(ID3D11DepthStencilView* _dsv = nullptr,
												bool _isClearDSV = false);

	// アルファブレンドの設定
	static void					SetAlphaBlend(bool type);
};