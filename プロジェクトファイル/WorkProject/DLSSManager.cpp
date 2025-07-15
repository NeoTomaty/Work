#include "DLSSManager.h"
#include <assert.h>
#include <iostream>  // デバッグ用
#include <memory>
#include "Renderer.h"
#include "Application.h"


// アプリケーションID
#define APP_ID (231313132)

using namespace DirectX;

// NGXの初期化
// --------------------------------------------------
// 引数
// _log:ログ表示を行うファイルのパス（必須ではない）
// --------------------------------------------------
bool DLSSManager::InitializeNGX(const wchar_t* _log)
{

    // デバイスを取得
    ID3D11Device* device = Renderer::GetDevice();

    // デバイスがなかったら
    if (device == nullptr)
    {
        std::cerr << "デバイスがありません" << std::endl;
        return false;
    }

    // NGXを初期化
    auto result = NVSDK_NGX_D3D11_Init(APP_ID, _log, device);

    // NGX初期化の結果を格納
    m_ngxInitialized = !NVSDK_NGX_FAILED(result);

    // 成功が返されなかったら
    if (!m_ngxInitialized){
        // 初期化の失敗
        if (result == NVSDK_NGX_Result_FAIL_FeatureNotSupported || result == NVSDK_NGX_Result_FAIL_PlatformError)
            std::wcerr << L"NVIDIA NGXが使用できません code = 0x"
            << std::hex << result
            << L", info: " << GetNGXResultAsString(result)
            << std::endl;
        else
            std::wcerr << L"NVIDIA NGXの初期化に失敗しました error code = 0x"
            << std::hex << result
            << L", info: " << GetNGXResultAsString(result)
            << std::endl;

        return false;
    }

    // NGX機能のパラメータを取得
    result = NVSDK_NGX_D3D11_GetCapabilityParameters(&m_ngxParameters);
    // 成功が返されなかったら
    if (result != NVSDK_NGX_Result_Success)
    {
        // パラメータの取得失敗
        std::cerr << "機能のパラメータ取得に失敗しました" << std::endl;
        // シャットダウン
        ShutdownNGX();
        return false;
    }

    // NVIDIAドライバのバーション確認
#if defined(NVSDK_NGX_Parameter_SuperSampling_NeedsUpdatedDriver)        \
    && defined (NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMajor) \
    && defined (NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMinor)

    int needsUpdatedDriver = 0;
    unsigned int minDriverVersionMajor = 0;
    unsigned int minDriverVersionMinor = 0;
    NVSDK_NGX_Result ResultUpdatedDriver = m_ngxParameters->Get(NVSDK_NGX_Parameter_SuperSampling_NeedsUpdatedDriver, &needsUpdatedDriver);
    NVSDK_NGX_Result ResultMinDriverVersionMajor = m_ngxParameters->Get(NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMajor, &minDriverVersionMajor);
    NVSDK_NGX_Result ResultMinDriverVersionMinor = m_ngxParameters->Get(NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMinor, &minDriverVersionMinor);
    if (ResultUpdatedDriver == NVSDK_NGX_Result_Success &&
        ResultMinDriverVersionMajor == NVSDK_NGX_Result_Success &&
        ResultMinDriverVersionMinor == NVSDK_NGX_Result_Success)
    {
        if (needsUpdatedDriver)
        {
            std::cerr << "エラー: NVIDIA DLSSをロードできません。ドライバが古いためです。必要な最低ドライババージョン: "
                << minDriverVersionMajor << "." << minDriverVersionMinor << std::endl;
            std::cout << "機能のパラメータ取得に失敗しました" << std::endl;
            ShutdownNGX();
            return false;
        }
        else
        {
            std::cout << "NVIDIA DLSSの最低ドライババージョン: "
                << minDriverVersionMajor << "." << minDriverVersionMinor << std::endl;
        }
    }
    else
    {
        std::cout << "NVIDIA DLSSの最低ドライババージョンは報告されませんでした。" << std::endl;
    }
#endif

    // DLSSが使用可能なのかを確認する
    int dlssAvailable = 0;
    result = m_ngxParameters->Get(NVSDK_NGX_Parameter_SuperSampling_Available, &dlssAvailable);
    // パラメータの取得に失敗または利用できないハードウェアだった場合
    if (result != NVSDK_NGX_Result_Success || !dlssAvailable)
    {
        std::wcout << L"NVIDIA DLSS が使用できません result = 0x"
            << std::hex << result
            << L", info: " << GetNGXResultAsString(result)
            << std::endl;
        // シャットダウン
        ShutdownNGX();
        return false;
    }

    // DLSSが使用可能なのでフラグを変更
    m_dlssAvailable = true;

    // 初期化成功
    std::cout << "NGX SDKの初期化に成功しました" << std::endl;


    return true;
}

// NGX及びDLSSの機能の解放
void DLSSManager::ShutdownNGX()
{
    // デバイスを取得
    ID3D11Device* device = Renderer::GetDevice();

    if (device == nullptr)
    {
        std::cerr << "デバイスがありません" << std::endl;
        return;
    }

    // NGXが初期化されていたら
    if (m_ngxInitialized)
    {
        // 全てのGPU処理か完了するまで待機
        Wait();

        // DLSS機能が空でなかった場合
        if (m_dlssFeature != nullptr)
        {
            std::cerr << "機能がリリースされる前にNGXライブラリを解放しようとしています 現在解放していますがコードを確認する必要があります" << std::endl;
            ReleaseDLSSFeatures();
        }


        NVSDK_NGX_D3D11_DestroyParameters(m_ngxParameters);
        NVSDK_NGX_D3D11_Shutdown1(nullptr);

        m_ngxInitialized = false;
    }
}


// DLSS機能の初期化
// ---------------------------------------------------------------------
// 引数
// OptimalRenderSize:入力画像の解像度
// OutputRenderSize:出力画像の解像度
// isHDR:HDR出力がどうか
// depthInverted:深度値の反転の有無
// depthScale:深度値（基本的には1.0fをセット）
// enableSharpening:シャープニング（画像の鮮鋭化）を有効にするかどうか
// enableAutoExposure:自動露出補正を有効にするかどうか
// qualValue:DLSSのパフォーマンス品質を示すパラメータ
// renderPreset:レンダリングのプリセット設定
// ---------------------------------------------------------------------
bool DLSSManager::InitializeDLSSFeatures(
    XMUINT2 OptimalRenderSize,
    XMUINT2 OutputRenderSize,
    int isHDR,
    bool depthInverted,
    bool enableSharpening,
    bool enableAutoExposure,
    NVSDK_NGX_PerfQuality_Value qualValue,
    unsigned int renderPreset)
{
    // デバイスを取得
    ID3D11Device* device = Renderer::GetDevice();
    // コンテキストを取得
    ID3D11DeviceContext* context = Renderer::GetDeviceContext();


    // NGXが初期化されていなかったら
    if (!m_ngxInitialized)
    {
        std::cerr << "NGXが初期化されていない状態でのDLSSの初期化になります" << std::endl;
        return false;
    }

    NVSDK_NGX_Result ResultDLSS = NVSDK_NGX_Result_Fail;

    int MotionVectorResolutionLow = 1; // スニペット（コード片）にモーションベクトルのアップサンプリングを任せます
    // 機能の作成	
    int DlssCreateFeatureFlags = NVSDK_NGX_DLSS_Feature_Flags_None;
    DlssCreateFeatureFlags |= MotionVectorResolutionLow ? NVSDK_NGX_DLSS_Feature_Flags_MVLowRes : 0;
    DlssCreateFeatureFlags |= isHDR ? NVSDK_NGX_DLSS_Feature_Flags_IsHDR : 0;
    DlssCreateFeatureFlags |= depthInverted ? NVSDK_NGX_DLSS_Feature_Flags_DepthInverted : 0;
    DlssCreateFeatureFlags |= enableSharpening ? NVSDK_NGX_DLSS_Feature_Flags_DoSharpening : 0;
    DlssCreateFeatureFlags |= enableAutoExposure ? NVSDK_NGX_DLSS_Feature_Flags_AutoExposure : 0;

    NVSDK_NGX_DLSS_Create_Params DlssCreateParams;
    memset(&DlssCreateParams, 0, sizeof(DlssCreateParams));

    DlssCreateParams.Feature.InWidth = OptimalRenderSize.x;
    DlssCreateParams.Feature.InHeight = OptimalRenderSize.y;
    DlssCreateParams.Feature.InTargetWidth = OutputRenderSize.x;
    DlssCreateParams.Feature.InTargetHeight = OutputRenderSize.y;
    DlssCreateParams.Feature.InPerfQualityValue = qualValue;
    DlssCreateParams.InFeatureCreateFlags = DlssCreateFeatureFlags;

    //DlssCreateParams.Feature.InWidth = 1920;
    //DlssCreateParams.Feature.InHeight = 1080;
    //DlssCreateParams.Feature.InTargetWidth = 3840;
    //DlssCreateParams.Feature.InTargetHeight = 2160;
    //DlssCreateParams.Feature.InPerfQualityValue = NVSDK_NGX_PerfQuality_Value_MaxPerf;


    // レンダープリセットを選択
    NVSDK_NGX_Parameter_SetUI(m_ngxParameters, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_DLAA, renderPreset);
    NVSDK_NGX_Parameter_SetUI(m_ngxParameters, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Quality, renderPreset);
    NVSDK_NGX_Parameter_SetUI(m_ngxParameters, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Balanced, renderPreset);
    NVSDK_NGX_Parameter_SetUI(m_ngxParameters, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Performance, renderPreset);
    NVSDK_NGX_Parameter_SetUI(m_ngxParameters, NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraPerformance, renderPreset);

    // 機能の作成
    ResultDLSS = NGX_D3D11_CREATE_DLSS_EXT(
        context, 
        &m_dlssFeature, 
        m_ngxParameters, 
        &DlssCreateParams);

    // 作成に失敗したとき
    if (NVSDK_NGX_FAILED(ResultDLSS))
    {
        std::wcerr << L"Failed to create DLSS Features = 0x"
            << std::hex << ResultDLSS
            << L", info: " << GetNGXResultAsString(ResultDLSS)
            << std::endl;
        return false;
    }

    // 初期化成功フラグを立てる
    m_dlssAvailable = true;

    // 初期化成功ログ
    std::cout << "DLSSの初期化に成功しました" << std::endl;

    return m_dlssAvailable;
}

// DLSS機能の解放
void DLSSManager::ReleaseDLSSFeatures()
{
    ID3D11Device* device = Renderer::GetDevice();

    if (device == nullptr)
    {
        std::cerr << "デバイスがありません" << std::endl;
        return;
    }

    if (!m_ngxInitialized)
    {
        std::cerr << "NGXの初期化がされていない状態での解放になります" << std::endl;
        return;
    }

    // GPUの全てのリソースが完了するまで待機
    Wait();

    // DLSS機能の解放
    auto result = (m_dlssFeature != nullptr) ? NVSDK_NGX_D3D11_ReleaseFeature(m_dlssFeature) : NVSDK_NGX_Result_Success;
    if (NVSDK_NGX_FAILED(result))
    {
        std::cerr << "DLSS機能の解放に失敗しました" << std::endl;
    }

    // 解放処理
    m_dlssFeature = nullptr;

}

// DLSS機能の最適なレンダリング設定
// --------------------------------------------------------
// 引数
// renderSize:レンダリングサイズ
// inQualValue:DLSSのパフォーマンス品質を示すパラメータ
// outRecommendedSettings:推奨レンダリング設定パラメータ
// --------------------------------------------------------
bool DLSSManager::QueryOptimalSettings(
    XMUINT2 inDisplaySize,
    NVSDK_NGX_PerfQuality_Value inQualValue,
    DlssRecommendedSettings* outRecommendedSettings)
{
    
    // NGXが初期化されていなかったら
    if (!m_ngxInitialized)
    {
        outRecommendedSettings->m_ngxRecommendedOptimalRenderSize = inDisplaySize;
        outRecommendedSettings->m_ngxDynamicMaximumRenderSize = inDisplaySize;
        outRecommendedSettings->m_ngxDynamicMinimumRenderSize = inDisplaySize;

        std::cerr << "NGXが初期化されていない状態での呼び出しです" << std::endl;
        return false;
    }

    // 最適設定を適用
    NVSDK_NGX_Result result = NGX_DLSS_GET_OPTIMAL_SETTINGS(m_ngxParameters,
        inDisplaySize.x, inDisplaySize.y, 
        inQualValue,
        &outRecommendedSettings->m_ngxRecommendedOptimalRenderSize.x, &outRecommendedSettings->m_ngxRecommendedOptimalRenderSize.y,
        &outRecommendedSettings->m_ngxDynamicMaximumRenderSize.x, &outRecommendedSettings->m_ngxDynamicMaximumRenderSize.y,
        &outRecommendedSettings->m_ngxDynamicMinimumRenderSize.x, &outRecommendedSettings->m_ngxDynamicMinimumRenderSize.y,
        &outRecommendedSettings->m_ngxRecommendedSharpness);

    // 設定に失敗したとき
    if (NVSDK_NGX_FAILED(result))
    {
        outRecommendedSettings->m_ngxRecommendedOptimalRenderSize = inDisplaySize;
        outRecommendedSettings->m_ngxDynamicMaximumRenderSize = inDisplaySize;
        outRecommendedSettings->m_ngxDynamicMinimumRenderSize = inDisplaySize;
        outRecommendedSettings->m_ngxRecommendedSharpness = 0.0f;

        std::wcerr << L"Optimal Settingに失敗しました code = 0x"
            << std::hex << result
            << L", info: " << GetNGXResultAsString(result)
            << std::endl;
        return false;
    }

    return true;
}

// 待機
void DLSSManager::Wait()
{
    ID3D11Device* device = Renderer::GetDevice();
    ID3D11DeviceContext* context = Renderer::GetDeviceContext();

    // 必要ならば GPU 処理が完了するまでのクエリを設定
    D3D11_QUERY_DESC queryDesc = { D3D11_QUERY_EVENT, 0 };
    ID3D11Query* query = nullptr;
    device->CreateQuery(&queryDesc, &query);

    context->End(query);
    while (context->GetData(query, nullptr, 0, 0) == S_FALSE) {
        // 処理が完了するまで待機
    }

    query->Release();
}


// DLSSスーパーサンプリング評価
// --------------------------------------------------------------------------------
// 引数
// unresolvedColor:解決前のレンダリングターゲット（通常のカラー出力）
// resolvedColor:DLSSでアップスケールされた最終的な解像度のレンダリングターゲット
// motionVectors:モーションベクターを格納するテクスチャ
// depth:深度情報を格納するテクスチャ
// bResetAccumulation:累積バッファをリセットするフラグ（TAAやDLSSの再初期化）
// bUseNgxSdkExtApi:拡張APIを使用するかのフラグ
// jitterOffset:ジッターオフセット（TAAやDLSSで使用されるサブピクセル位置）
// mVScale:モーションベクターのスケール値
// --------------------------------------------------------------------------------
bool DLSSManager::EvaluateSuperSampling(
    ID3D11Texture2D* unresolvedColor,
    ID3D11Texture2D* resolvedColor,
    ID3D11Texture2D* motionVectors,
    ID3D11Texture2D* depth,
    ID3D11Texture2D* exposure,
    bool bResetAccumulation,
    bool bUseNgxSdkExtApi,
    XMFLOAT2 jitterOffset,
    XMFLOAT2 mVScale)
{

    // DLSSが使用できない状態ならば
    if (!m_dlssAvailable)
    {
        std::cerr << ("DLSSが使用できません　スーパーサンプリング機能が無効です") << std::endl;
        return false;
    }

    // 各リソースの状態を確認
    if (m_dlssFeature == nullptr)
        std::cerr << "m_dlssFeature is nullptr" << std::endl;
    if (m_ngxParameters == nullptr)
        std::cerr << "m_ngxParameters is nullptr" << std::endl;
    if (unresolvedColor == nullptr)
    std::cerr << "unresolvedColor is nullptr" << std::endl;
    if (resolvedColor == nullptr)
    std::cerr << "resolvedColor is nullptr" << std::endl;
    if (motionVectors == nullptr) 
    std::cerr << "motionVectors is nullptr" << std::endl;
    if (depth == nullptr) 
    std::cerr << "depth is nullptr" << std::endl;


    // 累積リセットフラグを整数として設定（1:リセット、0:リセットなし）
    int Reset = bResetAccumulation ? 1 : 0;

    // レンダリングサイズを取得
    auto renderSize = Renderer::GetInputRenderSize();

    // 現在のビューポートのレンダリングオフセットを設定（ビューポートの左上）
    // 現在は[0,0]で固定のため定数で
    NVSDK_NGX_Coordinates renderingOffset = { 0,0 };

    // 現在のビューポートのレンダリングサイズを設定（ビューポートの幅と高さ）
    NVSDK_NGX_Dimensions  renderingSize = 
        { renderSize.x - renderingOffset.X,
          renderSize.y - renderingOffset.Y };

    // Direct3D 11 のデバイスコンテキストとリソースを取得
    NVSDK_NGX_Result Result;
    auto deviceContext = Renderer::GetDeviceContext();

    ID3D11Resource* unresolvedColorResource = static_cast<ID3D11Resource*>(unresolvedColor);
    ID3D11Resource* motionVectorsResource = static_cast<ID3D11Resource*>(motionVectors);
    ID3D11Resource* resolvedColorResource = static_cast<ID3D11Resource*>(resolvedColor);
    ID3D11Resource* depthResource = static_cast<ID3D11Resource*>(depth);


    D3D11_TEXTURE2D_DESC desc;
    unresolvedColor->GetDesc(&desc);
    std::cout << "unresolvedColor Format: " << desc.Format << std::endl;
    std::cout << "unresolvedColor BindFlags: " << desc.BindFlags << std::endl;
    std::cout << "unresolvedColor Size: " << desc.Width << " x " << desc.Height << std::endl;

    resolvedColor->GetDesc(&desc);
    std::cout << "resolvedColor Format: " << desc.Format << std::endl;
    std::cout << "resolvedColor BindFlags: " << desc.BindFlags << std::endl;
    std::cout << "resolvedColor Size: " << desc.Width << " x " << desc.Height << std::endl;

    motionVectors->GetDesc(&desc);
    std::cout << "motionVectors Format: " << desc.Format << std::endl;
    std::cout << "motionVectors BindFlags: " << desc.BindFlags << std::endl;
    std::cout << "motionVectors Size: " << desc.Width << " x " << desc.Height << std::endl;

    depth->GetDesc(&desc);
    std::cout << "depth Format: " << desc.Format << std::endl;
    std::cout << "depth BindFlags: " << desc.BindFlags << std::endl;
    std::cout << "depth Size: " << desc.Width << " x " << desc.Height << std::endl;

    std::cout << "Motion Vector Scale: (" << mVScale.x << ", " << mVScale.y << ")" << std::endl;

    // DLSS評価パラメータ構造体を初期化
    NVSDK_NGX_D3D11_DLSS_Eval_Params D3D11DlssEvalParams;
    memset(&D3D11DlssEvalParams, 0, sizeof(D3D11DlssEvalParams));

    // パラメータにリソースを設定
    D3D11DlssEvalParams.Feature.pInColor            = unresolvedColorResource;
    D3D11DlssEvalParams.Feature.pInOutput           = resolvedColorResource;
    D3D11DlssEvalParams.pInDepth                    = depthResource;
    D3D11DlssEvalParams.pInMotionVectors            = motionVectorsResource;
    //D3D11DlssEvalParams.pInExposureTexture          = exposure;

    D3D11DlssEvalParams.InJitterOffsetX             = jitterOffset.x;
    D3D11DlssEvalParams.InJitterOffsetY             = jitterOffset.y;
    D3D11DlssEvalParams.InReset                     = Reset;
    D3D11DlssEvalParams.InMVScaleX                  = mVScale.x;
    D3D11DlssEvalParams.InMVScaleY                  = mVScale.y;
    D3D11DlssEvalParams.InColorSubrectBase          = renderingOffset;
    D3D11DlssEvalParams.InDepthSubrectBase          = renderingOffset;
    D3D11DlssEvalParams.InTranslucencySubrectBase   = renderingOffset;
    D3D11DlssEvalParams.InMVSubrectBase             = renderingOffset;
    D3D11DlssEvalParams.InRenderSubrectDimensions   = renderingSize;

    // スーパーサンプリング実行
    Result = NGX_D3D11_EVALUATE_DLSS_EXT(
        deviceContext,
        m_dlssFeature, 
        m_ngxParameters, 
        &D3D11DlssEvalParams);

    // 失敗した場合
    if (NVSDK_NGX_FAILED(Result))
    {
        std::wcerr << L"DLSSスーパーサンプリングに失敗しました code = 0x"
            << std::hex << Result << L", info: "
            << GetNGXResultAsString(Result) << std::endl;
        return false;
    }

    return true;
}