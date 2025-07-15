#include <d3d11.h>
#include "nvsdk_ngx/nvsdk_ngx.h"
#include "nvsdk_ngx/nvsdk_ngx_helpers.h"
#include "nvsdk_ngx/nvsdk_ngx_defs.h"
#include "nvsdk_ngx/nvsdk_ngx_params.h"
#include <iostream>
#include <DirectXMath.h>

#ifdef NDEBUG
#pragma comment(lib, "nvsdk_ngx/nvsdk_ngx_s.lib")
#else
#pragma comment(lib, "nvsdk_ngx/nvsdk_ngx_s_dbg.lib")
#endif

// NGXで提供されるパフォーマンスの設定・制御する構造体
struct PERF_QUALITY_ITEM
{
    NVSDK_NGX_PerfQuality_Value PerfQuality;
    const char*                 PerfQualityText;
    bool                        PerfQualityAllowed;
    bool                        PerfQualityDynamicAllowed;
};

// NGXが提供するDLSSのレンダリングプリセットを管理する構造体
struct RENDER_PRESET_ITEM
{
    NVSDK_NGX_DLSS_Hint_Render_Preset RenderPreset;
    const char* RenderPresetText;
};

// DLSSの推奨設定構造体
struct DlssRecommendedSettings  // ~(0u) : 無効の値（unsigned int型全ビット0の反転）
{
    float     m_ngxRecommendedSharpness = 0.0f;                             // ngx sdk 3.1では廃止されるステータス
    DirectX::XMUINT2 m_ngxRecommendedOptimalRenderSize = { ~(0u), ~(0u) };  // 推奨される最適レンダリング解像度
    DirectX::XMUINT2 m_ngxDynamicMaximumRenderSize = { ~(0u), ~(0u) };      // 動的に設定可能な最大レンダリング解像度
    DirectX::XMUINT2 m_ngxDynamicMinimumRenderSize = { ~(0u), ~(0u) };      // 動的に設定可能な最小レンダリング解像度
};

class DLSSManager
{
private:

    NVSDK_NGX_Parameter* m_ngxParameters = nullptr; // NGXのパラメータ管理
    NVSDK_NGX_Handle* m_dlssFeature = nullptr;      // DLSS機能オブジェクト
    bool m_ngxInitialized = false;                  // NGXの初期化状態
    bool m_dlssAvailable = false;                   // DLSSの使用可否状態
    NVSDK_NGX_FeatureDiscoveryInfo dis;             // 機能検出用データ
public:

    // NGXの初期化
    bool InitializeNGX(const wchar_t* _log);

    // NGXのシャットダウン
    void ShutdownNGX();

    // DLSS機能の初期化
    bool InitializeDLSSFeatures(
        DirectX::XMUINT2 iRenderSize,
        DirectX::XMUINT2 oRenderSize,
        int isHDR,
        bool depthInverted,
        bool enableSharpening,
        bool enableAutoExposure,
        NVSDK_NGX_PerfQuality_Value qualValue,
        unsigned int renderPreset);

    // DLSS機能の解放
    void ReleaseDLSSFeatures();

    // 最適な設定クエリ
    bool QueryOptimalSettings(
        DirectX::XMUINT2 renderSize,
        NVSDK_NGX_PerfQuality_Value inQualValue, 
        DlssRecommendedSettings* outRecommendedSettings);

    // 待機
    void Wait();

    // DLSS機能評価
    bool EvaluateSuperSampling(
        ID3D11Texture2D* unresolvedColor,
        ID3D11Texture2D* resolvedColor,
        ID3D11Texture2D* motionVectors,
        ID3D11Texture2D* depth,
        ID3D11Texture2D* exposure,
        bool bResetAccumulation,
        bool bUseNgxSdkExtApi,
        DirectX::XMFLOAT2 jitterOffset = { 0.0f, 0.0f },
        DirectX::XMFLOAT2 mVScale = { 1.0f, 1.0f });

    // シングルトンパターン
    static DLSSManager& GetInstance() {
        static DLSSManager Instance;
        return Instance;
    }
};
