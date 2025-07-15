#include "SceneDemo.h"
#include "DLSSManager.h"
#include <map>

using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace std;


//-------------------------------------
// 列挙型
//-------------------------------------
// ON/OFF列挙
enum Enable
{
	DISABLE,
	ENABLE,
};
// レンダリングモード列挙
enum class RenderMode {
	NONE,		// なし（フォワードレンダリング）
	DEFERRED,	// ディファードレンダリング
	DLSS,		// DLSSスーパーサンプリング
	DDLSS,		// ディファード＋DLSS
};
// バッファモード列挙
enum class BufferMode {
	DEFAULT = 0,	//0：通常
	COLOR,			//1：アルベド
	NORMAL,			//2：法線
	WORLD,			//3：ワールド座標
	DEPTH,			//4：深度
	MVECTOR,		//5：モーションベクトル
};
// アンチエイリアシングモード列挙
enum class  AntiAliasingMode {
	NONE,
	DLSS,
};


//-------------------------------------
// ImGuiパラメータ
//-------------------------------------
ExtraParam	Param{};					// テクスチャ情報
static int renderPresetIndex = 0;		// 現在選択されているプリセットのインデックス
static int perfQualityIndex = 0;		// 現在選択されているパフォーマンス品質のインデックス
static int bufferMode = 0;				// 現在選択されているバッファモード
static bool isResetResource = false;	// 蓄積されたパラメータを破棄するか
static bool isInitDLSS = false;			// DLSS機能の初期化が出来ているか？
static bool isUseDLSSFeature = false;	// DLSSを使用するかどうか

// レンダリングモード
RenderMode g_RenderMode = RenderMode::NONE;
// アンチエイリアシングモード
AntiAliasingMode g_AntiAliasingMode
= AntiAliasingMode::NONE;

//-------------------------------------
// DLSS関連パラメータ
//-------------------------------------
// クオリティリスト
std::vector<PERF_QUALITY_ITEM> PERF_QUALITY_LIST =
{
	{NVSDK_NGX_PerfQuality_Value_MaxPerf,          "Performance", false, false},	// レンダリング倍率2.0倍
	{NVSDK_NGX_PerfQuality_Value_Balanced,         "Balanced"   , false, false},	// レンダリング倍率1.7倍
	{NVSDK_NGX_PerfQuality_Value_MaxQuality,       "Quality"    , false, false},	// レンダリング倍率1.5倍
	{NVSDK_NGX_PerfQuality_Value_UltraPerformance, "UltraPerf"  , false, false},	// レンダリング倍率3.0倍
	{NVSDK_NGX_PerfQuality_Value_DLAA,             "DLAA"       , false, false},	// アンチエイリアシング
};
// DLSSの推奨設定（画質モード別）
map<NVSDK_NGX_PerfQuality_Value, DlssRecommendedSettings> g_RecommendedSettingsMap;
// 前フレームのクォリティモード
NVSDK_NGX_PerfQuality_Value PrevQuality = 
PERF_QUALITY_LIST[perfQualityIndex].PerfQuality;
// 現フレームのクォリティモード
NVSDK_NGX_PerfQuality_Value PerfQualityMode =
PERF_QUALITY_LIST[perfQualityIndex].PerfQuality;


//-------------------------------------
// ライトパラメータ
//-------------------------------------
static SceneLight	Lights;	// ライト
static DLIGHTTYPE LightType = DLIGHTTYPE::NOON;	// 平行光源の種類

//-------------------------------------
// その他パラメータ
//-------------------------------------
static int currentIndex = 0;	// ポストプロセスリソースの要素番号
static PPparam isPP;			// ポストプロセスの適用フラグ
static int isOutline = 1;		// 輪郭線の表示

//-------------------------------------
// DebugUI
//-------------------------------------
// DLSSデバッグ
void DebugDLSS() {

	if (g_RenderMode == RenderMode::DLSS || g_RenderMode == RenderMode::DDLSS)
	{
		ImGui::Begin("DLSS Setting");

		auto input = Renderer::GetInputRenderSize();
		auto output = Renderer::GetOutputRenderSize();

		// 現在のレンダリングサイズを表示
		ImGui::Text("Input Render Size: %d x %d", input.x, input.y);
		ImGui::Text("Output Render Size: %d x %d", output.x, output.y);

		// レンダリングサイズの変更
		// 解像度の選択肢を用意
		const char* resolutionOptions[] = { "540p", "720p", "1080p", "4K" };
		static int selectedResolutionIndex = 2;  // 初期値を1080pに設定（インデックス2）

		// 解像度を選択
		if (ImGui::Combo("Output Resolution", &selectedResolutionIndex, resolutionOptions, IM_ARRAYSIZE(resolutionOptions))) {
			// レンダリングサイズを更新
			switch (selectedResolutionIndex) {
			case 0: // 540p
				output.x = 960;
				output.y = 540;
				break;
			case 1: // 720p
				output.x = 1280;
				output.y = 720;
				break;
			case 2: // 1080p
				output.x = 1920;
				output.y = 1080;
				break;
			case 3: // 4K
				output.x = 3840;
				output.y = 2160;
				break;
			}
			// 出力サイズを更新
			Renderer::SetOutputRenderSize(output);
		}

		// DLSSで蓄積したリソースを初期化のON/OFF
		if (ImGui::Checkbox("Use DLSS", &isUseDLSSFeature)) {
			// チェック状態が変更されたときに呼び出される
			if (isUseDLSSFeature) {
				printf("Use DLSS Enabled\n");
			}
			else {
				printf("Use DLSS Disabled\n");
			}
		}

		// DLSSで蓄積したリソースを初期化のON/OFF
		if (ImGui::Checkbox("Reset Resource", &isResetResource)) {
			// チェック状態が変更されたときに呼び出される
			if (isResetResource) {
				printf("Reset Enabled\n");
			}
			else {
				printf("Reset Disabled\n");
			}
		}

		// アンチエイリアシングモードの選択
		const char* aaModes[] = { "NONE", "DLSS" };
		int currentAAMode = static_cast<int>(g_AntiAliasingMode); // 現在のモードを整数値に変換
		if (ImGui::Combo("Anti-Aliasing Mode", &currentAAMode, aaModes, IM_ARRAYSIZE(aaModes))) {
			g_AntiAliasingMode = static_cast<AntiAliasingMode>(currentAAMode); // 選択されたモードを直接適用
			printf("Selected Anti-Aliasing Mode: %s\n", aaModes[currentAAMode]);
		}

		// パフォーマンス品質の選択
		std::vector<const char*> perfQualityItems;

		for (const auto& quality : PERF_QUALITY_LIST) {
			perfQualityItems.push_back(quality.PerfQualityText);
		}

		// アンチエイリアシングモードがDLSSの時
		if (g_AntiAliasingMode == AntiAliasingMode::DLSS)
		{
			if (ImGui::Combo("DLSS Performance Quality", &perfQualityIndex, perfQualityItems.data(), static_cast<int>(perfQualityItems.size()))) {
				// パフォーマンス品質が変更された場合に呼び出される
				printf("Selected Performance Quality: %s\n", perfQualityItems[perfQualityIndex]);
			}
		}

		ImGui::End();
	}
}
// 平行光源ライティングデバッグ
void DebugDLight()
{
	ImGui::Begin("DirectionalLight Parameter");

	Directional DLight = Lights.GetDirectionalLight();

	Vector3 target = Lights.SetTarget();

	ImGui::Text("Position: (%.2f, %.2f, %.2f)", DLight.Position.x, DLight.Position.y, DLight.Position.z);
	ImGui::Text("Direction: (%.2f, %.2f, %.2f)", DLight.Direction.x, DLight.Direction.y, DLight.Direction.z);
	ImGui::Text("Target: (%.2f, %.2f, %.2f)", target.x, target.y, target.z);

	// 平行光源の有無
	bool enable = (DLight.Enable == TRUE); // BOOL -> bool に変換
	// チェックボックスの値を更新
	if (ImGui::Checkbox("Light Enabled", &enable))
	{
		DLight.Enable = (enable ? TRUE : FALSE); // bool -> BOOL に変換
	}

	// ライトの座標
	static float position[3] = { DLight.Position.x, DLight.Position.y, DLight.Position.z };
	if (ImGui::SliderFloat3("Position", position, -200.0f, 200.0f))
	{
		// 変更を反映させる
		DLight.Position = Vector3(position[0], position[1], position[2]);
	}

	// ライトの方向
	Vector3 direction = target - DLight.Position;
	direction.Normalize();
	DLight.Direction = Vector4(direction.x, direction.y, direction.z, 0.0f);

	const char* lightTypeNames[] = { "FREE", "MORNING", "NOON", "EVENING", "NIGHT" };

	// 平行光源の種類を設定
	int currentItem = static_cast<int>(LightType);
	if (ImGui::Combo("DirectionalLight Type", &currentItem, lightTypeNames, IM_ARRAYSIZE(lightTypeNames)))
	{
		// 変更を反映
		LightType = static_cast<DLIGHTTYPE>(currentItem);
	}

	// ライトタイプが自由なら
	if (LightType == DLIGHTTYPE::FREE)
	{
		// 平行光源の拡散反射光
		static float diffuse[3] = { DLight.Diffuse.x, DLight.Diffuse.y, DLight.Diffuse.z };
		if (ImGui::ColorEdit3("Diffuse", diffuse))
		{
			DLight.Diffuse = Color(diffuse[0], diffuse[1], diffuse[2], 1.0f);
		}
		// 平行光源の環境光
		static float ambient[3] = { DLight.Ambient.x, DLight.Ambient.y, DLight.Ambient.z };
		if (ImGui::ColorEdit3("Ambient", ambient))
		{
			DLight.Ambient = Color(ambient[0], ambient[1], ambient[2], 1.0f);
		}

	}

	Lights.SetDirectionalLight(DLight);

	ImGui::End();
}
// 点光源ライティングデバッグ
void DebugPLight()
{

	ImGui::Begin("PointLight Parameter");

	// 全ての点光源
	if (ImGui::TreeNode("All PointLights"))
	{
		static bool allEnabled = true;
		if (ImGui::Checkbox("All Enabled", &allEnabled)) {
			for (int i = 0; i < POINTLIGHT_NUM; ++i) {
				Point light = Lights.GetPointLight(i);
				light.Enable = (allEnabled ? TRUE : FALSE);
				Lights.SetPointLight(i, light);
			}
		}

		static float allIntensity = 10.0f;
		if (ImGui::SliderFloat("All Intensity", &allIntensity, 0.0f, 100.0f)) {
			for (int i = 0; i < POINTLIGHT_NUM; ++i) {
				Point light = Lights.GetPointLight(i);
				light.Intensity = allIntensity;
				Lights.SetPointLight(i, light);
			}
		}

		static float allDiffuse[3] = { 1.0f, 1.0f, 1.0f };
		if (ImGui::ColorEdit3("All Diffuse", allDiffuse)) {
			for (int i = 0; i < POINTLIGHT_NUM; ++i) {
				Point light = Lights.GetPointLight(i);
				light.Diffuse = Color(allDiffuse[0], allDiffuse[1], allDiffuse[2], 1.0f);
				Lights.SetPointLight(i, light);
			}
		}

		static float allAmbient[3] = { 0.1f, 0.1f, 0.1f };
		if (ImGui::ColorEdit3("All Ambient", allAmbient)) {
			for (int i = 0; i < POINTLIGHT_NUM; ++i) {
				Point light = Lights.GetPointLight(i);
				light.Ambient = Color(allAmbient[0], allAmbient[1], allAmbient[2], 1.0f);
				Lights.SetPointLight(i, light);
			}
		}

		static float allConstant = 1.0f;
		static float allLinear = 0.1f;
		static float allQuadratic = 0.01f;
		if (ImGui::SliderFloat("All Constant", &allConstant, 0.0f, 10.0f) |
			ImGui::SliderFloat("All Linear", &allLinear, 0.0f, 1.0f) |
			ImGui::SliderFloat("All Quadratic", &allQuadratic, 0.0f, 1.0f)) {
			for (int i = 0; i < POINTLIGHT_NUM; ++i) {
				Point light = Lights.GetPointLight(i);
				light.Attenuation = Vector3(allConstant, allLinear, allQuadratic);
				Lights.SetPointLight(i, light);
			}
		}

		ImGui::TreePop();
	}

	// 個別の点光源
	for (int i = 0; i < POINTLIGHT_NUM; ++i)
	{

		Point PLight = Lights.GetPointLight(i);

		ImGui::PushID(i); // 各ライト用の識別子

		// ライト名（PointLight1など）
		std::string nodeName = "PointLight " + std::to_string(i + 1);

		// ツリー表示で折りたたみ式UI
		if (ImGui::TreeNode(nodeName.c_str()))
		{
			// 点光源の可否
			bool pEnable = (PLight.Enable == TRUE);
			if (ImGui::Checkbox("Enabled", &pEnable)) {
				PLight.Enable = (pEnable ? TRUE : FALSE);
			}

			// 座標
			ImGui::Text("Position");
			ImGui::SliderFloat3("Pos", &PLight.Position.x, -200.0f, 200.0f);

			// 明るさ（Intensity）
			ImGui::SliderFloat("Intensity", &PLight.Intensity, 0.0f, 100.0f);

			// 拡散反射光
			static float pDiffuse[POINTLIGHT_NUM][3];
			pDiffuse[i][0] = PLight.Diffuse.x;
			pDiffuse[i][1] = PLight.Diffuse.y;
			pDiffuse[i][2] = PLight.Diffuse.z;
			if (ImGui::ColorEdit3("Diffuse", pDiffuse[i])) {
				PLight.Diffuse = Color(pDiffuse[i][0], pDiffuse[i][1], pDiffuse[i][2], 1.0f);
			}

			// 環境光
			static float pAmbient[POINTLIGHT_NUM][3];
			pAmbient[i][0] = PLight.Ambient.x;
			pAmbient[i][1] = PLight.Ambient.y;
			pAmbient[i][2] = PLight.Ambient.z;
			if (ImGui::ColorEdit3("Ambient", pAmbient[i])) {
				PLight.Ambient = Color(pAmbient[i][0], pAmbient[i][1], pAmbient[i][2], 1.0f);
			}

			// 減衰係数（Attenuation）
			ImGui::Text("Attenuation");
			ImGui::SliderFloat("Constant", &PLight.Attenuation.x, 0.0f, 10.0f);
			ImGui::SliderFloat("Linear", &PLight.Attenuation.y, 0.0f, 1.0f);
			ImGui::SliderFloat("Quadratic", &PLight.Attenuation.z, 0.0f, 1.0f);

			ImGui::TreePop(); // TreeNodeの終了

			Lights.SetPointLight(i, PLight);
		}

		ImGui::PopID(); // IDのリセット

	}

	ImGui::End();
}
// レンダリングデバッグ
void DebugRender()
{
	ImGui::Begin("Rendering Setting");

	// レンダリングモード選択用の静的変数
	static int renderMode = static_cast<int>(g_RenderMode);

	// レンダリングモードの選択肢（文字列配列）
	const char* renderItems[] = { "NONE", "DEFERRED", "DLSS", "DEFERRED+DLSS"};

	// Comboボックスでレンダリングモードを選択
	if (ImGui::Combo("Select RenderMode", &renderMode, renderItems, IM_ARRAYSIZE(renderItems))) {
		// ユーザーが選択を変更した場合に呼び出される
		printf("Selected RenderMode: %s\n", renderItems[renderMode]);

		// グローバル変数を更新
		g_RenderMode = static_cast<RenderMode>(renderMode);
	}

	// ディファードレンダリングを行っている場合に追加の選択項目を表示
	if (g_RenderMode == RenderMode::DEFERRED || g_RenderMode == RenderMode::DDLSS)
	{
		// バッファ選択肢（文字列配列）
		const char* bufferItems[] = { "DEFAULT", "COLOR", "NORMAL", "WORLD", "DEPTH", "MVECTOR" };

		// Comboボックスでバッファを選択
		if (ImGui::Combo("Select Buffer", &bufferMode, bufferItems, IM_ARRAYSIZE(bufferItems))) {
			// 選択されたバッファに応じて処理
			printf("Selected Buffer: %s\n", bufferItems[bufferMode]);
		}
	}

	ImGui::End();
}
// ポストプロセスデバッグ
void DebugPostProcess()
{
	ImGui::Begin("Visual Effects");

	// 各ポストプロセスの切り替え
	bool enableDoF = (isPP.isDoF == TRUE);
	bool enableBloom = (isPP.isBloom == TRUE);
	bool enableVignette = (isPP.isVignette == TRUE);


	// シャドウのON/OFF
	bool enableShadow = (Param.isShadow == 1);
	if (ImGui::Checkbox("Shadow", &enableShadow))
	{
		Param.isShadow = (enableShadow ? 1 : 0);
	}

	ImGui::Separator();

	// 輪郭線のON/OFF
	bool enableOutline = (isOutline == 1);
	if (ImGui::Checkbox("OutLine", &enableOutline))
	{
		isOutline = (enableOutline ? 1 : 0);
	}


	ImGui::Separator();

	// チェックボックスを表示して更新
	if (ImGui::Checkbox("Depth of Field", &enableDoF))
	{
		isPP.isDoF = (enableDoF ? TRUE : FALSE);
	}
	// スライダーでFocusを調整
	ImGui::Text("Depth of Field Parameters");
	ImGui::SliderFloat("Focus", &Param.focus, 0.1f, 500.0f);
	ImGui::SliderFloat("Range", &Param.DoFRange, 0.1f, 500.0f);
	
	ImGui::Separator();
	
	// ブルーム表現
	if (ImGui::Checkbox("Bloom", &enableBloom))
	{
		isPP.isBloom = (enableBloom ? TRUE : FALSE);
	}

	ImGui::SliderFloat("Bloom Strength", &Param.bloomStrength, 0.1f, 1.0f);

	ImGui::Separator();

	// ビネット
	if (ImGui::Checkbox("Vignette", &enableVignette))
	{
		isPP.isVignette = (enableVignette ? TRUE : FALSE);
	}
	// スライダーでビネット強度を調整
	ImGui::Text("Vignette Parameters");
	ImGui::SliderFloat("Strength", &Param.vignetteStrength, 0.1f, 1.0f);
	ImGui::SliderFloat("Radius", &Param.vignetteRadius, 0.1f, 1.0f);
	ImGui::SliderFloat2("Center", Param.vignetteCenter, 0.0f, 1.0f);

	// isPPAll の状態を更新
	if (isPP.isDoF || isPP.isBloom || isPP.isVignette)
	{
		isPP.isPPAll = TRUE;
	}
	else
	{
		isPP.isPPAll = FALSE;
	}


	Renderer::SetExtraParam(Param);

	ImGui::End();

}

//-------------------------------------
// シーンメソッド
//-------------------------------------
// シーンの初期化
void SceneDemo::SceneInit()
{

	// モデルファイルパス
	std::vector<std::string> filename =
	{
		"assets/model/Cottage/cottage.fbx",	// 0
		"assets/model/SkySphere.fbx",		// 1
		"assets/model/Cube.fbx",			// 2
		"assets/model/Mount.fbx",			// 3
		"assets/model/Sphere.fbx",			// 4
	};
	// テクスチャファイルパス
	std::vector<std::string> texfilename =
	{
		"assets/model/Cottage/cottage_diffuse.png",		// 0
		"assets/model/Cottage/cottage_normal.png",		// 1
		"assets/texture/bg.jpg",						// 2
		"assets/texture/White.png",						// 3

		"assets/texture/concrete_moss_diff_1k.jpg",		// 4
		"assets/texture/concrete_moss_nor_1k.jpg",		// 5

		"assets/texture/sandy_gravel_02_diff_1k.jpg",	// 6
		"assets/texture/sandy_gravel_02_nor_1k.jpg",	// 7

		"assets/texture/aerial_rocks_04_diff_1k.jpg",	// 8
		"assets/texture/aerial_rocks_04_nor_1k.jpg",	// 9

		"assets/texture/aerial_rocks_02_diff_1k.jpg",	// 10
		"assets/texture/aerial_rocks_02_nor_1k.jpg",	// 11

		"assets/texture/coast_sand_rocks_02_diff_1k.jpg",	// 12
		"assets/texture/coast_sand_rocks_02_nor_1k.jpg",	// 13

		"assets/texture/stone_tiles_03_diff_1k.jpg",	// 14
		"assets/texture/stone_tiles_03_nor_1k.jpg",		// 15

	};
	// スプライトファイルパス
	std::vector<std::string> spritefilename =
	{
		"assets/texture/UnityChan.png",		// 0
		"assets/texture/Lamp.png",			// 1
		"assets/texture/TreeA.png",			// 2
		"assets/texture/TreeB.png",			// 3

		"assets/texture/UnityChan_normal.png",	// 4
	};
	// シェーダーファイルパス
	std::vector<std::string> shaderfile =
	{
		"VS_Default.hlsl",				// 0		基本的な頂点情報を渡す
		"PS_UnlitShader.hlsl",			// 1		ライティングなし
		"PS_LitShader.hlsl",			// 2		ライティングあり

		"PS_GBuffer.hlsl",				// 3		Gバッファ出力
		"PS_DeferredRendering.hlsl",	// 4		Gバッファを用いたライティング
		"PS_DLSSOffScreenColor.hlsl",	// 5		DLSS用オフスクリーン
		"PS_DLSSOutput.hlsl",			// 6		DLSS用レンダリング

		"PS_SkySphere.hlsl",			// 7		SkySphere用

		"VS_ShadowMap.hlsl",			// 8		シャドウマップ用頂点情報
		"PS_AlphaClip.hlsl",			// 9		シャドウマップ出力
		"PS_Shadow.hlsl",				// 10		シャドウイング

		"VS_Outline.hlsl",				// 11		輪郭線
		"PS_Outline.hlsl",				// 12		輪郭線

		"PS_DoF.hlsl",					// 13		被写界深度
		"PS_Blur.hlsl",					// 14		ブラー抽出

		"PS_Luminance.hlsl",			// 15		輝度抽出
		"PS_Bloom.hlsl",				// 16		ブルームライティング
		"PS_Vignette.hlsl",				// 17		ビネット

		"PS_ToneMapping.hlsl",			// 18		トーンマッピング
	};


	// カメラ初期化
	m_Camera.Init();

	// スクリーンクアッド初期化
	m_Screen.Init();

	// フィールド初期化
	{
		XMUINT2 div = { 20,20 };			// 分割数
		XMUINT2 size = { 450,450 };			// サイズ
		Vector3 pos = { 0.0f,0.0f,0.0f };	// 座標

		m_Moss.Init(div, size, pos, texfilename[4], texfilename[5]);

		div = { 40,3 };					// 分割数
		size = { 450,30 };				// サイズ
		pos = { 0.0f,0.001f,-100.0f };	// 座標

		m_TileA.Init(div, size, pos, texfilename[14], texfilename[15]);

		div = { 3,4 };					// 分割数
		size = { 30,50 };				// サイズ
		pos = { 30.0f,0.001f, -60.0f };	// 座標
	
		m_TileB.Init(div, size, pos, texfilename[14], texfilename[15]);
	}

	// 光源の初期化
	Lights.InitLight();


	// レンダリングリソースの生成
	CreateRenderResource();

	// Imgui用デバッグ関数呼び出し
	DebugUI::RedistDebugFunction(DebugDLight);
	DebugUI::RedistDebugFunction(DebugPLight);
	DebugUI::RedistDebugFunction(DebugDLSS);
	DebugUI::RedistDebugFunction(DebugRender);
	DebugUI::RedistDebugFunction(DebugPostProcess);

	// シェーダー
	{
		m_ShaderUnlit.Create(shaderfile[0], shaderfile[1]);				// ライティングなし
		m_ShaderLit.Create(shaderfile[0], shaderfile[2]);				// ライティングあり
		m_ShaderGBuffer.Create(shaderfile[0], shaderfile[3]);			// Gバッファ出力
		m_ShaderDeferredRendering.Create(shaderfile[0], shaderfile[4]);	// ディファードレンダリング
		m_ShaderDLSSInput.Create(shaderfile[0], shaderfile[5]);			// DLSSの入力値（オフスクリーン）
		m_ShaderDLSSOutput.Create(shaderfile[0], shaderfile[6]);		// DLSSの出力値
		m_ShaderSkySphere.Create(shaderfile[0], shaderfile[7]);			// SkySphere用
		m_ShaderShadowMap.Create(shaderfile[8], shaderfile[1]);			// シャドウマップ
		m_ShaderShadowAlphaClip.Create(shaderfile[8], shaderfile[9]);	// アルファクリップ
		m_ShaderShadow.Create(shaderfile[0], shaderfile[10]);			// シャドウイング
		m_ShaderOutline.Create(shaderfile[11], shaderfile[12]);			// 輪郭線
		m_ShaderDoF.Create(shaderfile[0], shaderfile[13]);				// 被写界深度
		m_ShaderBlur.Create(shaderfile[0], shaderfile[14]);				// ブラー抽出
		m_ShaderLuminance.Create(shaderfile[0], shaderfile[15]);		// 輝度抽出
		m_ShaderBloom.Create(shaderfile[0], shaderfile[16]);			// フルーム
		m_ShaderVignette.Create(shaderfile[0], shaderfile[17]);			// ビネット
		m_ShaderToneMapping.Create(shaderfile[0], shaderfile[18]);		// トーンマッピング
	}


	// ユニティちゃん初期化
	m_UnityChan.Init(
		spritefilename[0],
		spritefilename[4],
		{ 0.0f,2.0f,-50.0f },	// ポリゴンの中心座標
		{ 10,10 },				// ポリゴンの長さ
		{ 0,0,0 },				// 回転
		{ 1,1,1 },				// 大きさ
		true,					// アニメーションの有無
		{ 4,1 },				// uv分割数
		10.0f);					// アニメーション速度

	// 木の読み込み
	{
		// 木AのXZ座標
		float treeAPosX[TREE_NUM] = { -25.0f,-5.0f,30.0f };
		float treeAPosZ[TREE_NUM] = { -10.0f,20.0f,0.0f };
		// 木AのXZ座標
		float treeBPosX[TREE_NUM] = { -40.0f,60.0f,15.0f };
		float treeBPosZ[TREE_NUM] = { -30.0f,-25.0f,-65.0f };

		for (auto i = 0; i < TREE_NUM; i++)
		{

			// 木Aの初期化
			m_TreeA[i].Init(
				spritefilename[2],
				"",
				{ treeAPosX[i],7.5f,treeAPosZ[i] },	// ポリゴンの中心座標
				{ 30,30 },							// ポリゴンの長さ
				{ 0,0,0 },							// 回転
				{ 1,1,1 },							// 大きさ
				true,								// アニメーションの有無
				{ 4,1 },							// uv分割数
				10.0f);								// アニメーション速度


			// 木Bの初期化
			m_TreeB[i].Init(
				spritefilename[3],
				"",
				{ treeBPosX[i],7.5f,treeBPosZ[i] },	// ポリゴンの中心座標
				{ 30,30 },							// ポリゴンの長さ
				{ 0,0,0 },							// 回転
				{ 1,1,1 },							// 大きさ
				true,								// アニメーションの有無
				{ 4,1 },							// uv分割数
				10.0f);								// アニメーション速度

		}
	}

	// SkyBox読み込み
	{
		std::string f = filename[1];

		// モデル読み込み
		m_Skysphere.Load(f);

		// テクスチャ読み込み
		f = texfilename[2];
		m_Skysphere.LoadTex(f, DIFFUSE);

		// レンダラーにモデルを渡す
		m_MRSkysphere.Init(m_Skysphere);

		Vector3 pos = Vector3(0.0f, 0.0f, 0.0f);
		Vector3 rotate = Vector3(0.0f, 0.0f, 0.0f);
		Vector3 scale = Vector3(0.4f, 0.4f, 0.4f);
		m_MRSkysphere.SetPosition(pos);
		m_MRSkysphere.SetRotation(rotate);
		m_MRSkysphere.SetScale(scale);
	}
	// Cottage読み込み
	{
		std::string f;

		// 座標、回転、スケールを個別に定義
		Vector3 pos[COTTAGE_NUM] = {
			Vector3(-20.0f, 0.0f, -60.0f),
			Vector3(-45.0f, 0.0f, -60.0f),
			Vector3(65.0f, 0.0f, -65.0f),
			Vector3(65.0f, 0.0f, -35.0f),
			Vector3(-20.0f, 0.0f, -150.0f),
			Vector3(-45.0f, 0.0f, -150.0f),
			Vector3(55.0f, 0.0f, -150.0f),
			Vector3(80.0f, 0.0f, -150.0f),
			Vector3(110.0f, 0.0f, -150.0f),
			Vector3(30.0f, 0.0f, 40.0f)
		};

		float rotate[COTTAGE_NUM] = {
			-95.0f,
			-95.0f,
			-5.0f,
			-5.0f,
			85.0f,
			85.0f,
			85.0f,
			85.0f,
			85.0f,
			-95.0f,
		};

		Vector3 scale[COTTAGE_NUM] = {
			Vector3(1.0f, 1.3f, 1.1f),
			Vector3(1.0f, 1.3f, 1.1f),
			Vector3(1.0f, 1.5f, 1.2f),
			Vector3(1.0f, 1.3f, 1.0f),
			Vector3(1.0f, 1.3f, 1.1f),
			Vector3(1.0f, 1.3f, 1.1f),
			Vector3(1.2f, 1.4f, 1.2f),
			Vector3(1.0f, 1.3f, 1.3f),
			Vector3(1.1f, 1.5f, 1.4f),
			Vector3(1.5f, 2.0f, 2.0f)
		};

		for (auto i = 0; i < COTTAGE_NUM; i++)
		{
			f = filename[0];

			// モデル読み込み
			m_Cottage[i].Load(f);

			// テクスチャ読み込み
			f = texfilename[0];
			m_Cottage[i].LoadTex(f, TexType::DIFFUSE);
			f = texfilename[1];
			m_Cottage[i].LoadTex(f, TexType::NORMAL);


			// レンダラーにモデルを渡す
			m_MRCottage[i].Init(m_Cottage[i]);

			m_MRCottage[i].SetPosition(pos[i]);
			m_MRCottage[i].SetRotation(Vector3(0.0f, rotate[i], 0.0f));
			m_MRCottage[i].SetScale(scale[i]);
		}

	}
	// 山読み込み
	{
		std::string f = filename[3];

		// モデル読み込み
		m_Mountain.Load(f);

		// テクスチャ読み込み
		f = texfilename[4];
		m_Mountain.LoadTex(f, TexType::DIFFUSE);

		// レンダラーにモデルを渡す
		m_MRMountain.Init(m_Mountain);

		Vector3 pos = Vector3(0.0f, 0.25f, 150.0f);
		Vector3 rotate = Vector3(0.0f, 0.0f, 0.0f);
		Vector3 scale = Vector3(0.6f, 0.3f, 0.2f);
		m_MRMountain.SetPosition(pos);
		m_MRMountain.SetRotation(rotate);
		m_MRMountain.SetScale(scale);
	}

	// ランプ及びライトオブジェクト読み込み
	{

		// ランプの位置
		Vector3 lampPos[LAMP_NUM] = {
			{-30.0f,3.6f,-40.0f},
			{  0.0f,3.6f,-40.0f},
			{ 30.0f,3.6f,-40.0f},
		};

		for (auto i = 0; i < LAMP_NUM; i++)
		{
			m_Lamp[i].Init(
				spritefilename[1],
				"",
				lampPos[i],				// ポリゴンの中心座標
				{ 10,12 },				// ポリゴンの長さ
				{ 0,0,0 },				// 回転
				{ 1.0f,1.5f,1.0f },		// 大きさ
				false);					// アニメーションの有無

		}

		// ライトオブジェクト（デバッグ用）
		for (auto i = 0; i < POINTLIGHT_NUM; i++)
		{
			Point PLight = Lights.GetPointLight(i);

			std::string f = filename[4];

			// モデル読み込み
			m_LightObj[i].Load(f);

			// テクスチャ読み込み
			f = texfilename[3];
			m_LightObj[i].LoadTex(f, TexType::DIFFUSE);

			// レンダラーにモデルを渡す
			m_MRLightObj[i].Init(m_LightObj[i]);

			Vector3 pos = PLight.Position;
			Vector3 rotate = Vector3(0.0f, 0.0f, 0.0f);
			Vector3 scale = Vector3(0.5f, 0.5f, 0.5f);
			m_MRLightObj[i].SetPosition(pos);
			m_MRLightObj[i].SetRotation(rotate);
			m_MRLightObj[i].SetScale(scale);

		}
	}
}
// シーンの更新
void SceneDemo::SceneUpdate()
{
	// 各種オブジェクト更新処理
	m_Camera.Update();
	for (auto i = 0; i < COTTAGE_NUM; i++)
	{
		m_MRCottage[i].Update();
	}
	m_Screen.Update();
	m_UnityChan.Update();

	// ライトの更新処理
	UpdateLight();

	// DLSSモードで使用していたら
	if (g_RenderMode == RenderMode::DLSS &&
		g_AntiAliasingMode == AntiAliasingMode::DLSS)
		UpdateDLSS();	// DLSSアップスケーリングの更新処理


	// サンプラーステートの設定
	Renderer::SetSamplerState();
}
// シーンの描画
void SceneDemo::SceneDraw()
{
	// Applicationに格納されている描画カウントをリセット
	Application::ResetDrawInfo();

	// シャドウマップのレンダリングサイズ
	XMUINT2 shadowRenderSize = { SHADOW_MAP_WIDTH ,SHADOW_MAP_HEIGHT };

	//==============================================
	// シャドウイング
	//==============================================

	Renderer::SetDepthEnable(true);					// 深度ステンシルステートをON
	Renderer::SetCullMode(CULLMODE::NONE);			// カリングモード
	Renderer::SetViewPort(shadowRenderSize);		// シャドウマップ用のビューポートを指定
	Renderer::SetShadowMapDSV(m_ShadowMapDSV, true);// シャドウマップ用のDSVをセット
	Lights.DrawLight();								// ライトのバインド

	bool isShadowMap = true;

	m_ShaderShadowMap.SetGPU();						// シャドウマップシェーダー
	// モデルの影描画
	for (auto i = 0; i < COTTAGE_NUM; i++)
	{
		m_MRCottage[i].Draw(isShadowMap);			// Cottageの描画
	}

	// αクリップを行うモデルの影描画
	m_ShaderShadowAlphaClip.SetGPU();				// シャドウマップシェーダー（αクリップ）
	m_UnityChan.Draw(isShadowMap);					// ユニティちゃんの描画

	for (auto i = 0; i < LAMP_NUM; i++)				// ランプの描画
	{
		m_Lamp[i].Draw(isShadowMap);
	}

	for (auto i = 0; i < TREE_NUM; i++)				// 木の描画
	{
		m_TreeA[i].Draw();
		m_TreeB[i].Draw();
	}

	//==============================================
	// レンダリング
	//==============================================

	Param.TextureType = bufferMode;							// 表示するテクスチャタイプを設定
	Renderer::SetCamera(m_Camera);							// 3D（透視投影）カメラセット
	m_Camera.Draw();										// バインド

	// ウィンドウサイズのビューポートをセット
	Renderer::SetViewPort();
	// DLSSモードの場合はレンダリングサイズに依存したビューポートをセット
	if (g_RenderMode == RenderMode::DLSS || g_RenderMode == RenderMode::DDLSS)
	Renderer::SetViewPort(Renderer::GetInputRenderSize());	// ビューポートを指定


	// レンダーターゲットをHDRオフスクリーンにセット
	SetHDRRenderTarget(true, m_WriteDepthDSV, true);
	// ディファードレンダリングモードの場合は専用のRTVに
	if (g_RenderMode == RenderMode::DEFERRED || g_RenderMode == RenderMode::DDLSS)
		SetDeferredGBufferRenderTarget();

	SetShadowMap();								// シャドウマップをバインド

	//----------------------------------------------
	// 輪郭線
	//----------------------------------------------

	if (isOutline)
	{
		Renderer::SetCullMode(CULLMODE::FRONT);		// カリングモードをフロントに
		m_ShaderOutline.SetGPU();					// 輪郭線用のシェーダー

		for (auto i = 0; i < COTTAGE_NUM; i++)
		{
			m_MRCottage[i].Draw();						// Cottageの描画
		}

		Renderer::SetCullMode(CULLMODE::BACK);		// カリングモードをバックに（2Dスプライトはカリングバックで）
		m_UnityChan.Draw();							// ユニティちゃんの描画

		for (auto i = 0; i < LAMP_NUM; i++)			// 光源オブジェクトの描画（デバッグ用）
		{
			m_Lamp[i].Draw();						// ランプの描画
		}

		for (auto i = 0; i < TREE_NUM; i++)			// 木の描画
		{
			m_TreeA[i].Draw();
			m_TreeB[i].Draw();
		}
	}


	//----------------------------------------------
	// フォワードレンダリング
	//----------------------------------------------
	if (g_RenderMode == RenderMode::NONE)
	{
		m_ShaderLit.SetGPU();						// ライティング（シャドウなし）用のシェーダー

		Param.UseNormalMap = Enable::DISABLE;		// 法線マップ未使用
		Renderer::SetExtraParam(Param);				// バインド
		m_Mountain.SetTexture();					// テクスチャをセット
		m_MRMountain.Draw();						// 山の描画

		m_ShaderShadow.SetGPU();					// ライティング（シャドウあり）用のシェーダー

		Param.UseNormalMap = Enable::ENABLE;		// 法線マップ使用
		Renderer::SetExtraParam(Param);				// バインド
		m_Moss.Draw();								// フィールドの描画
		//m_TileA.Draw();								// フィールドの描画
		//m_TileB.Draw();								// フィールドの描画

		for (auto i = 0; i < COTTAGE_NUM; i++)
		{
			m_Cottage[i].SetTexture();				// テクスチャをセット
			m_MRCottage[i].Draw();					// Cottageの描画
		}

		Param.UseNormalMap = Enable::DISABLE;		// 法線マップ未使用
		Renderer::SetExtraParam(Param);				// バインド

		m_ShaderSkySphere.SetGPU();					// シェーダーをセット
		m_Skysphere.SetTexture();					// テクスチャをセット
		m_MRSkysphere.Draw();						// skysphereの描画

		Renderer::SetAlphaBlend(true);				// アルファブレンドON
		m_ShaderUnlit.SetGPU();						// シェーダーをセット

		for (auto i = 0; i < TREE_NUM; i++)
		{
			m_TreeA[i].Draw();						// 木の描画
		}

		m_TreeB[0].Draw();							// 木の描画
		m_TreeB[1].Draw();							// 木の描画

		for (auto i = 0; i < LAMP_NUM; i++)
		{
			m_Lamp[i].Draw();						// ランプの描画
		}


		//for (auto i = 0; i < POINTLIGHT_NUM; i++)	// 光源オブジェクトの描画（デバッグ用）
		//{
		//	Point PLight = Lights.GetPointLight(i);
		//	// 点光源があった場合
		//	if (PLight.Enable)
		//	{
		//		m_LightObj[i].SetTexture();			// テクスチャをセット	
		//		m_MRLightObj[i].Draw();				// 描画
		//	}
		//}

		//Param.UseNormalMap = Enable::ENABLE;		// 法線マップ未使用
		//Renderer::SetExtraParam(Param);				// バインド
		//m_ShaderLit.SetGPU();						// ライティング（シャドウなし）用のシェーダー
		m_UnityChan.Draw();							// ユニティちゃんの描画

		Param.UseNormalMap = Enable::DISABLE;		// 法線マップ未使用
		Renderer::SetExtraParam(Param);				// バインド
		m_ShaderUnlit.SetGPU();						// シェーダーをセット
		m_TreeB[2].Draw();							// 木の描画

		Renderer::SetAlphaBlend(false);				// アルファブレンドOFF

	}
	//----------------------------------------------
	// ディファードレンダリング（制作中）
	//----------------------------------------------
	if (g_RenderMode == RenderMode::DEFERRED)
	{
		//----------------------------------------------
		// ディファードレンダリング
		//----------------------------------------------
		m_ShaderGBuffer.SetGPU();					// シェーダーをセット（GBuffer）

		Param.UseNormalMap = Enable::ENABLE;		// 法線マップ使用
		Renderer::SetExtraParam(Param);				// バインド
		m_Moss.Draw();								// フィールドの描画
		//m_TileA.Draw();								// フィールドの描画
		//m_TileB.Draw();								// フィールドの描画

		for (auto i = 0; i < COTTAGE_NUM; i++)
		{
			m_Cottage[i].SetTexture();				// テクスチャをセット
			m_MRCottage[i].Draw();					// Cottageの描画
		}

		Param.UseNormalMap = Enable::DISABLE;		// 法線マップ未使用
		Renderer::SetExtraParam(Param);				// バインド
		m_Mountain.SetTexture();					// テクスチャをセット
		m_MRMountain.Draw();						// 山の描画
		m_Skysphere.SetTexture();					// テクスチャをセット
		m_MRSkysphere.Draw();						// skysphereの描画

		Renderer::SetAlphaBlend(true);				// アルファブレンドON

		for (auto i = 0; i < POINTLIGHT_NUM; i++)	// 光源オブジェクトの描画（デバッグ用）
		{
			//Point PLight = Lights.GetPointLight(i);
			//// 点光源があった場合
			//if (PLight.Enable)
			//{
			//	m_LightObj[i].SetTexture();			// ライトテクスチャをセット	
			//	m_MRLightObj[i].Draw();				// ライトオブジェクトの描画
			//}

			m_Lamp[i].Draw();				// ランプの描画
		}

		m_UnityChan.Draw();							// ユニティちゃんの描画
		Renderer::SetAlphaBlend(false);				// アルファブレンドOFF


		//---------------------------------------
		// ディファードシェーディング
		//---------------------------------------
		SetHDRRenderTarget(true, nullptr);			//	HDR用レンダーターゲットをセット
		SetDeferredShaderResource();				// GBufferの情報をテクスチャとしてバインド
		m_ShaderDeferredRendering.SetGPU();			// シェーダーのセット（ディファードレンダリング）
		m_Screen.Draw();							// 描画


		//↓半透明オブジェクトや異なるライティングを行うモデルのため↓
		//---------------------------------------
		// フォワードレンダリング
		//---------------------------------------

		// //フォワードレンダリングは深度バッファを有効化
		//context->OMSetRenderTargets(1, &defaultRTV, m_hdrDSV);
		// //深度バッファはそのまま使用する (必要ならクリア)
		// context->ClearDepthStencilView(m_hdrDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	}


	//----------------------------------------------
	// DLSSアップスケール（制作中）
	//----------------------------------------------

	// レンダリングモードに応じて描画を変える
	//switch (g_RenderMode)
	//{
	//	//case RenderMode::DLSS:
	//	//	// DLSSアップスケール
	//	//	{
	//	//	Renderer::SetViewPort(Renderer::GetInputRenderSize());	// ビューポートを指定
	//	//	//----------------------------------------------
	//	//	// ディファードレンダリング
	//	//	//----------------------------------------------
	//	//	SetDLSSRenderTarget();						// DLSS用レンダーターゲットを指定
	//	//	// GBuffer用シェーダーをセット
	//	//	m_ShaderGBuffer.SetGPU();
	//	//	// オブジェクトの描画
	//	//	ExtraParam.UseNormalMap = Enable::DISABLE;	// 法線マップ未使用
	//	//	Renderer::SetExtraParam(ExtraParam);		// バインド
	//	//	m_Skysphere.SetTexture();					// テクスチャをセット
	//	//	m_MRSkysphere.Draw();						// skysphereの描画
	//	//	ExtraParam.UseNormalMap = Enable::ENABLE;		// 法線マップ使用
	//	//	Renderer::SetExtraParam(ExtraParam);		// バインド
	//	//	m_Moss.Draw();								// フィールドの描画
	//	//	m_Cottage.SetTexture();						// テクスチャをセット
	//	//	m_MRCottage.Draw();							// モデルの描画
	//	//	//-----------------------------------------------------------------------------
	//	//	// オフスクリーンレンダリング（DLSSに渡す入力値を取得）
	//	//	// ※オフスクリーンのレンダーターゲットはSetDLSSRenderTarget()で指定しておく
	//	//	//-----------------------------------------------------------------------------
	//	//	SetHDRRenderTarget();		// オフスクリーン用のレンダーターゲットをセット
	//	//	SetDLSSShaderResource();		// Gバッファをバインド
	//	//	m_ShaderDLSSInput.SetGPU();		// オフスクリーンレンダリング用シェーダーをセット
	//	//	m_Screen.Draw();				// オフスクリーンレンダリング
	//	//	//------------------------------------------------
	//	//	// レンダリング結果をアップスケーリング（DLSS）
	//	//	//------------------------------------------------
	//	//	// Gバッファから得られたリソースを取得
	//	//	ID3D11Texture2D* PreResolveColor = m_DLSSInputTex;
	//	//	ID3D11Texture2D* ResolvedColor = m_DLSSOutputTex;
	//	//	ID3D11Texture2D* MotionVectors = m_DLSSGBufferTexs[4];
	//	//	ID3D11Texture2D* Depth = m_DLSSDepthTex;
	//	//	ID3D11Texture2D* Exposure = nullptr;
	//	//	// アップスケールの成功フラグ
	//	//	bool isEvaluate = false;
	//	//	// DLSSスーパーサンプリング
	//	//	// ココでDLSSサンプリングを行う
	//	//	if (g_AntiAliasingMode == AntiAliasingMode::DLSS)
	//	//	{
	//	//		// DLSS機能が初期化されていて
	//	//		// DLSSを使用するフラグが経っている場合
	//	//		if (isInitDLSS && isUseDLSSFeature)
	//	//		{
	//	//			bool ResetScene = isResetResource;
	//	//			//==============================================================================
	//	//			// 問題点２
	//	//			// DLAA以外のアップスケールの処理が行われない
	//	//			// →DLAAが成功していることから渡しているリソースに以上はないことがわかる
	//	//			// →レンダリングサイズが原因？あるいは足りないリソースがある？
	//	//			//==============================================================================
	//	//			// DLSSスーパーサンプリング
	//	//			if (DLSSManager::GetInstance().EvaluateSuperSampling(
	//	//				PreResolveColor,		// 入力
	//	//				ResolvedColor,			// 出力
	//	//				MotionVectors,			// モーションベクトル
	//	//				Depth,					// 深度
	//	//				Exposure,				// 露出
	//	//				ResetScene,				// リセットフラグ
	//	//				true,					// 拡張APIフラグ
	//	//				{ 0.0f, 0.0f },			// ジッターオフセット
	//	//				{ 1.0f, 1.0f }))		// モーションベクトルスケール
	//	//			{
	//	//				std::cout << "アップスケールに成功しました" << std::endl;
	//	//				isEvaluate = true;
	//	//			}
	//	//			else
	//	//			{
	//	//				// スーパーサンプリングに失敗した場合は
	//	//				// 入力前のリソースを使用する
	//	//				std::cerr << "アップスケールに失敗しました" << std::endl;
	//	//				std::cout << "アップスケール前の画像を使用します" << std::endl;
	//	//				isEvaluate = false;
	//	//			}
	//	//		}
	//	//		else
	//	//		{
	//	//			// DLSS機能の初期化がされていなかったら
	//	//			if (!isInitDLSS)
	//	//			{
	//	//				std::cerr << "DLSS機能が初期化されていません" << std::endl;
	//	//				std::cerr << "アップスケール前の画像を使用します" << std::endl;
	//	//				isEvaluate = false;
	//	//			}
	//	//		}
	//	//	}
	//	//	//---------------------------------
	//	//	// アップスケール結果を描画
	//	//	//---------------------------------
	//	//	Renderer::SetDefaultRenderTarget();		// レンダーターゲットを元に戻す
	//	//	Renderer::SetViewPort();					// ビューポートをウィンドウサイズに戻す
	//	//
	//	//	// スーパーサンプリングの可否に応じてバインドするリソースを変更
	//	//	if (isEvaluate) {
	//	//		SetDLSSOutputResource();				// アウトプットリソース
	//	//	}
	//	//	else {
	//	//		SetHDRResource();					// オフスクリーンリソース
	//	//	}
	//	//	m_ShaderDLSSOutput.SetGPU();				// 出力用シェーダーをセット
	//	//	m_Screen.Draw();							// 結果をフルスクリーンクアッドに描画
	//	//	}
	//	//	break;
	//}


	//----------------------------------------------
	// ポストプロセス
	//----------------------------------------------
	ClearPingPongRenderTarget();				// ポストプロセスレンダーターゲットをクリア

	// ポストプロセスが一つもなかった場合のケア
	ChangePingPongResource();					// ピンポンシェーダー切り替え
	SetHDRResource();							// SRVセット（SRV->1更新）
	m_Screen.Draw();

	// 被写界深度 //
	if (isPP.isDoF)
	{
		ClearBlurRenderTarget();				// ブラー用レンダーターゲットをクリア

		// 横方向のブラー
		SetBlurXResource(m_hdrSRV, 0.7f);		// 横方向のブラー抽出の準備
		m_ShaderBlur.SetGPU();					// ブラー抽出シェーダー
		m_Screen.Draw();						// 横方向のブラー抽出

		// 縦方向のブラー
		SetBlurYResource(m_BlurSRV[0], 0.7f);	// 横方向のブラー抽出の準備
		m_Screen.Draw();						// 横方向のブラー抽出

		// 被写界深度の適用
		SetDoFResource();						// 被写界深度の準備
		m_ShaderDoF.SetGPU();					// 被写界深度シェーダー
		m_Screen.Draw();						// 描画

	}

	// ブルーム //
	if (isPP.isBloom)
	{
		SetLuminanceResource();					// 輝度抽出の準備
		m_ShaderLuminance.SetGPU();				// 輝度抽出のシェーダー
		m_Screen.Draw();						// 描画

		ClearBlurRenderTarget();				// ブラー用レンダーターゲットをクリア

		// 横方向のブラー
		SetBlurXResource(m_LuminanceSRV, 3.0f);	// 横方向のブラー抽出の準備
		m_ShaderBlur.SetGPU();					// ブラー抽出シェーダー
		m_Screen.Draw();						// 横方向のブラー抽出

		// 縦方向のブラー
		SetBlurYResource(m_BlurSRV[0], 3.0f);	// 横方向のブラー抽出の準備
		m_Screen.Draw();						// 横方向のブラー抽出


		SetBloomResource();						// ブルームライティングの準備
		m_ShaderBloom.SetGPU();					// ブルームシェーダー
		m_Screen.Draw();						// 描画
	}

	// ビネット //
	if (isPP.isVignette)
	{
		ChangePingPongResource();				// ピンポンシェーダー切り替え
		m_ShaderVignette.SetGPU();				// ビネットシェーダー
		m_Screen.Draw();						// 描画
	}


	// 最終出力 //
	ChangePingPongResource();					// ピンポンシェーダー切り替え
	Renderer::SetDefaultRenderTarget();			// バックバッファのレンダーターゲットをセット
	m_ShaderUnlit.SetGPU();						// Unlitシェーダー（エフェクトなし）
	m_Screen.Draw();							// 描画

}
// シーンの終了
void SceneDemo::SceneDispose()
{
	// 解放 
	
	// G-Buffer
	for (int i = 0; i < GBUFFER_NUM; ++i) {
		if (m_DeferredGBufferTexs[i]) {
			m_DeferredGBufferTexs[i]->Release();
			m_DeferredGBufferTexs[i] = nullptr;
		}
		if (m_DeferredGBufferRTVs[i]) {
			m_DeferredGBufferRTVs[i]->Release();
			m_DeferredGBufferRTVs[i] = nullptr;
		}
		if (m_DeferredGBufferSRVs[i]) {
			m_DeferredGBufferSRVs[i]->Release();
			m_DeferredGBufferSRVs[i] = nullptr;
		}

		if (m_DLSSGBufferTexs[i]) {
			m_DLSSGBufferTexs[i]->Release();
			m_DLSSGBufferTexs[i] = nullptr;
		}
		if (m_DLSSGBufferRTVs[i]) {
			m_DLSSGBufferRTVs[i]->Release();
			m_DLSSGBufferRTVs[i] = nullptr;
		}
		if (m_DLSSGBufferSRVs[i]) {
			m_DLSSGBufferSRVs[i]->Release();
			m_DLSSGBufferSRVs[i] = nullptr;
		}
	}

	// DLSS深度
	m_DLSSDepthTex->Release();
	m_DLSSDepthTex = nullptr;
	m_DLSSDepthDSV->Release();
	m_DLSSDepthDSV = nullptr;
	m_DLSSDepthSRV->Release();
	m_DLSSDepthSRV = nullptr;

	// DLSS出力
	m_DLSSOutputTex->Release();
	m_DLSSOutputTex = nullptr;
	m_DLSSOutputUAV->Release();
	m_DLSSOutputUAV = nullptr;
	m_DLSSOutputSRV->Release();
	m_DLSSOutputSRV = nullptr;

	// オフスクリーン
	m_DLSSInputTex->Release();
	m_DLSSInputTex = nullptr;
	m_DLSSInputRTV->Release();
	m_DLSSInputRTV = nullptr;
	m_DLSSInputSRV->Release();
	m_DLSSInputSRV = nullptr;


	// シャドウマップ
	m_ShadowMapTex->Release();
	m_ShadowMapTex = nullptr;
	m_ShadowMapDSV->Release();
	m_ShadowMapDSV = nullptr;
	m_ShadowMapSRV->Release();
	m_ShadowMapSRV = nullptr;

	// シャドウ
	m_ShadowRTV->Release();
	m_ShadowRTV = nullptr;
	m_ShadowTex->Release();
	m_ShadowTex = nullptr;
	m_ShadowSRV->Release();
	m_ShadowSRV = nullptr;
	

	// HDR（ポストプロセス）
	m_hdrTex->Release();
	m_hdrTex = nullptr;
	m_hdrRTV->Release();
	m_hdrRTV = nullptr;
	m_hdrSRV->Release();
	m_hdrSRV = nullptr;


	// ポストプロセス
	for (int i = 0; i < PINGPONG_NUM; ++i)
	{
		m_PingPongTex[i]->Release();
		m_PingPongTex[i] = nullptr;
		m_PingPongRTV[i]->Release();
		m_PingPongRTV[i] = nullptr;
		m_PingPongSRV[i]->Release();
		m_PingPongSRV[i] = nullptr;
	}

	// 深度書き込み（DoF）
	m_WriteDepthTex->Release();
	m_WriteDepthTex = nullptr;
	m_WriteDepthDSV->Release();
	m_WriteDepthDSV = nullptr;
	m_WriteDepthSRV->Release();
	m_WriteDepthSRV = nullptr;

	// 輝度抽出
	m_LuminanceTex->Release();
	m_LuminanceTex = nullptr;
	m_LuminanceRTV->Release();
	m_LuminanceRTV = nullptr;
	m_LuminanceSRV->Release();
	m_LuminanceSRV = nullptr;

	// ブラー抽出
	for (int i = 0; i < BLUR_NUM; ++i)
	{
		m_BlurTex[i]->Release();
		m_BlurTex[i] = nullptr;
		m_BlurRTV[i]->Release();
		m_BlurRTV[i] = nullptr;
		m_BlurSRV[i]->Release();
		m_BlurSRV[i] = nullptr;
	}

	// ライト
	Lights.DisposeLight();
}


//-------------------------------------
// Updateメソッド
//-------------------------------------
// DLSSの更新処理
void SceneDemo::UpdateDLSS()
{
	//-------------------------------
	// レンダリングサイズ関連
	//-------------------------------

	// レンダリングサイズ（アップスケール前）
	XMUINT2 inputRenderTargetSize = Renderer::GetInputRenderSize();
	// 最終出力レンダリングサイズ（アップスケール後）
	XMUINT2 outputRenderTargetSize = Renderer::GetOutputRenderSize();
	// 直前（前フレーム）の推奨レンダリングサイズ
	static XMUINT2 inputLastSize = { 0, 0 };	// 入力
	static XMUINT2 outputLastSize = { 0, 0 };	// 出力 
	// サンプラーステートに渡すLodバイアス
	float lodBias = 0.f;
	// DLSS作成時に設定したレンダリングサイズ
	XMUINT2 dlssCreationTimeRenderSize = inputRenderTargetSize;

	//-------------------------------
	// クオリティモードの設定
	//-------------------------------

	// 現在のクオリティモード（UIから設定可能にしている）
	PrevQuality = PerfQualityMode;
	PerfQualityMode = PERF_QUALITY_LIST[perfQualityIndex].PerfQuality;

	//------------------------------------------------
	// DLSSの推奨サイズ設定（レンダリングサイズ）のクエリ
	// 最終レンダリングサイズ（output）から
	// 推奨レンダリングサイズ（input）を求める
	//------------------------------------------------

	// アンチエイリアシングモードがDLSSの場合
	if (g_AntiAliasingMode == AntiAliasingMode::DLSS)
	{
		// 最終出力レンダリングサイズが前フレームから更新されていない場合は
		// 最適レンダリングサイズの取得クエリは省略出来る
		if (outputLastSize.x != outputRenderTargetSize.x ||
			outputLastSize.y != outputRenderTargetSize.y)
		{
			// 各パフォーマンスモードについて最適設定のクエリ
			for (PERF_QUALITY_ITEM& item : PERF_QUALITY_LIST)
			{
				// 推奨レンダリングサイズのクエリ
				DLSSManager::GetInstance().QueryOptimalSettings(
					outputRenderTargetSize,
					item.PerfQuality,
					&g_RecommendedSettingsMap[item.PerfQuality]);

				// 推奨設定が有効か確認
				bool isRecommendedSettingValid = g_RecommendedSettingsMap[item.PerfQuality].m_ngxRecommendedOptimalRenderSize.x > 0;
				item.PerfQualityAllowed = isRecommendedSettingValid;

				if (isRecommendedSettingValid)
				{
					std::cout << "true isvalid" << endl;
				}
				else
				{
					std::cout << "false isvalid" << endl;
				}

				// 推奨サイズのデバッグ表示
				std::cout << "Recommended Optimal Render Size: ("
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxRecommendedOptimalRenderSize.x << ", "
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxRecommendedOptimalRenderSize.y << ")" << std::endl;
				std::cout << "Dynamic Maximum Render Size: ("
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMaximumRenderSize.x << ", "
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMaximumRenderSize.y << ")" << std::endl;
				std::cout << "Dynamic Minimum Render Size: ("
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMinimumRenderSize.x << ", "
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMinimumRenderSize.y << ")" << std::endl;

				// 動的設定が可能か確認
				bool isDynamicSettingAllowed =
					(g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMaximumRenderSize.x != g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMinimumRenderSize.x) ||
					(g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMaximumRenderSize.y != g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMinimumRenderSize.y);
				item.PerfQualityDynamicAllowed = isDynamicSettingAllowed;

				if (isDynamicSettingAllowed)
				{
					std::cout << "true dynamic allowed" << endl;
				}
				else
				{
					std::cout << "false dynamic allowd" << endl;
				}
			}

			// 前フレームのレンダリングサイズに
			// 現在のDLSS最終レンダリングサイズを保存しておく
			outputLastSize = outputRenderTargetSize;

			std::cout << "DLSS最適設定完了" << endl;
		}
	}


	//--------------------------------------------------------------------
	// レンダリングサイズが前フレームと比較して変更されていた場合の処理
	// （主にパフォーマンスモードに依存してサイズが変わる）
	//--------------------------------------------------------------------

	// テクスチャのLOD（Level of Detail）のX軸方向のサイズを初期化
	float texLodXDimension = { 0 };

	// 推奨されている最適なレンダリングサイズをDLSS初期化時のレンダリングサイズに設定
	dlssCreationTimeRenderSize = g_RecommendedSettingsMap[PerfQualityMode].m_ngxRecommendedOptimalRenderSize;

	// DLSS作成時のレンダリングサイズをアップスケール前のレンダリングサイズに設定
	inputRenderTargetSize = dlssCreationTimeRenderSize;

	// Lodバイアスをレンダリングサイズから計算
	texLodXDimension = inputRenderTargetSize.x;
	float ratio = (float)inputRenderTargetSize.x / (float)outputRenderTargetSize.x;
	lodBias = (std::log2f(ratio)) - 1.0f;

	// 求めたLodバイアスでサンプラーをセット
	Renderer::SetSamplerState(lodBias);

	//-------------------------------
	// DLSSの初期化
	//-------------------------------
	// レンダリングサイズが前フレームと異なる場合
	if (inputRenderTargetSize.x != inputLastSize.x ||
		inputRenderTargetSize.y != inputLastSize.y)
	{

		std::cout << "Lod Bias: " << lodBias << std::endl;

		// HDRや深度反転の設定（必要に応じて変更）
		bool isHDR = false;
		bool depthInverted = false;

		// DLSSオプション（例: Quality設定）
		NVSDK_NGX_PerfQuality_Value qualitySetting = PerfQualityMode;

		// レンダープリセット（0はデフォルト）
		unsigned int renderPreset = 0;

		// レンダリングサイズに応じてリソースを再生成
		Renderer::SetInputRenderSize(inputRenderTargetSize);
		CreateDLSSResource(inputRenderTargetSize);


		// NVIDIA GPUが使用可能な場合
		if (Renderer::GetIsAbleNVIDIA())
		{
			isInitDLSS = true;

			// DLSS初期化
			if (!DLSSManager::GetInstance().InitializeDLSSFeatures(
				inputRenderTargetSize,
				outputRenderTargetSize,
				isHDR,
				depthInverted,
				true, // enableSharpening
				true, // enableAutoExposure
				qualitySetting,
				renderPreset))
			{
				std::cerr << "DLSSの初期化に失敗しました" << std::endl;
				isInitDLSS = false;
			}

		}
		else
		{
			std::cerr << "NVIDIA_GPUを使用していません" << std::endl;
		}

	}

	// 前フレームに現在のレンダリングサイズを保存
	inputLastSize = inputRenderTargetSize;
}
// ライトの更新処理
void SceneDemo::UpdateLight()
{

	Directional DLight = Lights.GetDirectionalLight();

	//-------------------------------------
	// 平行光源
	//-------------------------------------
	// ライトパラメータによって変更
	switch (LightType)
	{
	case DLIGHTTYPE::MORNING:	// 朝
		DLight.Diffuse = { 0.8f, 0.7f, 0.9f, 1.0f };
		DLight.Ambient = { 0.5f, 0.5f, 0.8f, 1.0f };
		break;
	case DLIGHTTYPE::NOON:		// 昼
		DLight.Diffuse = { 1.0f, 1.0f, 0.95f, 1.0f };
		DLight.Ambient = { 0.8f, 0.8f, 0.8f, 1.0f };
		break;
	case DLIGHTTYPE::EVENING:	// 夕
		DLight.Diffuse = { 1.0f, 0.5f, 0.3f, 1.0f };
		DLight.Ambient = { 0.6f, 0.4f, 0.3f, 1.0f };
		break;
	case DLIGHTTYPE::NIGHT:		// 夜
		DLight.Diffuse = { 0.3f, 0.3f, 0.5f, 1.0f };
		DLight.Ambient = { 0.3f, 0.3f, 0.4f, 1.0f };
		break;
	default:
		break;
	}

	Lights.SetDirectionalLight(DLight);

	//-------------------------------------
	// 点光源
	//-------------------------------------
	for (auto i = 0; i < POINTLIGHT_NUM; i++)
	{
		Point PLight = Lights.GetPointLight(i);
		Directional DLight = Lights.GetDirectionalLight();

		// 点光源の座標を更新
		//Vector3 pos = PLight.Position;
		Vector3 pos = PLight.Position;
		m_MRLightObj[i].SetPosition(pos);

		Lights.SetPointLight(i, PLight);

	}


	// シャドウイングのためにカメラの情報をカメラに渡す
	Lights.UpdateLightMatrix(&m_Camera);

}


//-------------------------------------
// Resource生成メソッド
//-------------------------------------
// レンダリングリソースの生成
void SceneDemo::CreateRenderResource()
{
	CreateDeferredResource();	// ディファードレンダリング
	CreateDLSSResource();		// DLSS
	CreateShadowResource();		// シャドウ
	CreateHDRResource();		// オフスクリーン
	CreatePostProcessResource();// ポストプロセス
}

// ディファードレンダリングリソースの生成
void SceneDemo::CreateDeferredResource()
{
	auto device = Renderer::GetDevice();
	auto iRenderSize = Renderer::GetInputRenderSize();
	HRESULT hr = S_OK;

	// 各Gバッファのフォーマット
	DXGI_FORMAT formats[GBUFFER_NUM] = {
		DXGI_FORMAT_R8G8B8A8_UNORM,		// Albedo
		DXGI_FORMAT_R16G16B16A16_FLOAT, // Normal
		DXGI_FORMAT_R32G32B32A32_FLOAT, // WorldPos
		DXGI_FORMAT_R32G32B32A32_FLOAT,	// Depth（ダミー）
		DXGI_FORMAT_R32G32_FLOAT,		// MVector
	};

	// Gバッファの生成
	for (int i = 0; i < GBUFFER_NUM; ++i) {
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = iRenderSize.x;
		textureDesc.Height = iRenderSize.y;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.SampleDesc.Count = 1;	// マルチサンプルは無し
		textureDesc.Format = formats[i];
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		// テクスチャ作成
		hr = device->CreateTexture2D(&textureDesc, nullptr, &m_DeferredGBufferTexs[i]);
		if (FAILED(hr)) { cout << "Gバッファテクスチャの作成に失敗しました" << endl; }

		// RTV作成
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = formats[i];
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		hr = device->CreateRenderTargetView(m_DeferredGBufferTexs[i], &rtvDesc, &m_DeferredGBufferRTVs[i]);
		if (FAILED(hr)) { cout << "GバッファRTVの作成に失敗しました" << endl; }

		// SRV作成
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = formats[i];
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		hr = device->CreateShaderResourceView(m_DeferredGBufferTexs[i], &srvDesc, &m_DeferredGBufferSRVs[i]);
		if (FAILED(hr)) { cout << "GバッファSRVの作成に失敗しました" << endl; }

	}

}
// DLSSリソースの生成（及び再生成）
void SceneDemo::CreateDLSSResource(XMUINT2 renderSize)
{
	auto device = Renderer::GetDevice();
	HRESULT hr = S_OK;

	// 既存リソースの解放
	// Gバッファの解放
	for (int i = 0; i < GBUFFER_NUM; ++i) {
		if (m_DLSSGBufferRTVs[i]) m_DLSSGBufferRTVs[i]->Release();
		if (m_DLSSGBufferSRVs[i]) m_DLSSGBufferSRVs[i]->Release();
		if (m_DLSSGBufferTexs[i]) m_DLSSGBufferTexs[i]->Release();
	}
	// DLSS入力バッファの解放
	if (m_DLSSInputRTV) m_DLSSInputRTV->Release();
	if (m_DLSSInputSRV) m_DLSSInputSRV->Release();
	if (m_DLSSInputTex) m_DLSSInputTex->Release();
	// DLSS出力バッファの解放
	if (m_DLSSOutputUAV) m_DLSSOutputUAV->Release();
	if (m_DLSSOutputSRV) m_DLSSOutputSRV->Release();
	if (m_DLSSOutputTex) m_DLSSOutputTex->Release();
	// 深度バッファの解放
	if (m_DLSSDepthDSV) m_DLSSDepthDSV->Release();
	if (m_DLSSDepthSRV) m_DLSSDepthSRV->Release();
	if (m_DLSSDepthTex) m_DLSSDepthTex->Release();


	D3D11_TEXTURE2D_DESC textureDesc = {};
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

	// 各Gバッファのフォーマット
	DXGI_FORMAT formats[GBUFFER_NUM] = {
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,// Albedo（非線形空間）
		DXGI_FORMAT_R16G16B16A16_FLOAT, // Normal
		DXGI_FORMAT_R32G32B32A32_FLOAT, // WorldPos
		DXGI_FORMAT_R32_FLOAT,			// Depth
		DXGI_FORMAT_R32G32_FLOAT,		// MVector
	};

	//------------------------------------------------
	// Gバッファの生成
	//------------------------------------------------
	for (int i = 0; i < GBUFFER_NUM; ++i) {

		textureDesc.Width = renderSize.x;
		textureDesc.Height = renderSize.y;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.SampleDesc.Count = 1;	// マルチサンプルは無し
		textureDesc.Format = formats[i];
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		// テクスチャ
		hr = device->CreateTexture2D(&textureDesc, nullptr, &m_DLSSGBufferTexs[i]);
		if (FAILED(hr)) std::cout << "GBufferのテクスチャ生成に失敗しました：要素番号[" << i << "]" << endl;
		// RTV
		hr = device->CreateRenderTargetView(m_DLSSGBufferTexs[i], nullptr, &m_DLSSGBufferRTVs[i]);
		if (FAILED(hr)) std::cout << "GBufferのRTV生成に失敗しました：要素番号[" << i << "]" << endl;
		// SRV
		hr = device->CreateShaderResourceView(m_DLSSGBufferTexs[i], nullptr, &m_DLSSGBufferSRVs[i]);
		if (FAILED(hr)) std::cout << "GBufferのSRV生成に失敗しました：要素番号[" << i << "]" << endl;

	}

	//------------------------------------------------
	// 深度バッファの生成
	//------------------------------------------------
	textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	textureDesc.Width = renderSize.x;
	textureDesc.Height = renderSize.y;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; // SRVをバインド可能に
	// テクスチャ
	hr = device->CreateTexture2D(&textureDesc, nullptr, &m_DLSSDepthTex);
	if (FAILED(hr)) std::cout << "深度テクスチャの生成に失敗しました" << endl;
	// DSV
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	hr = device->CreateDepthStencilView(m_DLSSDepthTex, &dsvDesc, &m_DLSSDepthDSV);
	if (FAILED(hr)) std::cout << "深度用のDSVの生成に失敗しました" << endl;
	// SRV
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; // シェーダーで深度のみを使用
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(m_DLSSDepthTex, &srvDesc, &m_DLSSDepthSRV);
	if (FAILED(hr)) std::cout << "深度用のSRVの生成に失敗しました" << endl;

	//------------------------------------------------
	// 入力先バッファの生成
	//------------------------------------------------
	textureDesc.Width = renderSize.x;
	textureDesc.Height = renderSize.y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;	// マルチサンプルは無し
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	// テクスチャ
	device->CreateTexture2D(&textureDesc, nullptr, &m_DLSSInputTex);
	if (FAILED(hr)) std::cout << "オフスクリーンテクスチャの生成に失敗しました" << endl;
	// RTV
	device->CreateRenderTargetView(m_DLSSInputTex, nullptr, &m_DLSSInputRTV);
	if (FAILED(hr)) std::cout << "オフスクリーン用RTVの生成に失敗しました" << endl;
	// SRV
	device->CreateShaderResourceView(m_DLSSInputTex, nullptr, &m_DLSSInputSRV);
	if (FAILED(hr)) std::cout << "オフスクリーン用SRVの生成に失敗しました" << endl;


	//------------------------------------------------
	// 出力先バッファの生成
	//------------------------------------------------
	textureDesc.Width = renderSize.x;
	textureDesc.Height = renderSize.y;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT,	 // 出力フォーマット
	textureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	// テクスチャ
	device->CreateTexture2D(&textureDesc, nullptr, &m_DLSSOutputTex);
	if (FAILED(hr)) std::cout << "書き込み先テクスチャの生成に失敗しました" << endl;
	// UAV
	uavDesc.Format = textureDesc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(m_DLSSOutputTex, &uavDesc, &m_DLSSOutputUAV);
	if (FAILED(hr)) std::cout << "書き込み用のUAVの生成に失敗しました" << endl;
	// SRV
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(m_DLSSOutputTex, &srvDesc, &m_DLSSOutputSRV);
	if (FAILED(hr)) std::cout << "書き込み用のSRVの生成に失敗しました" << endl;

}
// シャドウリソースの生成
void SceneDemo::CreateShadowResource()
{
	auto device = Renderer::GetDevice();
	HRESULT hr = S_OK;

	//-------------------------------------
	// シャドウマップ
	//-------------------------------------
	// テクスチャ
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = SHADOW_MAP_WIDTH;
	texDesc.Height = SHADOW_MAP_HEIGHT;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	hr = device->CreateTexture2D(&texDesc, nullptr, &m_ShadowMapTex);
	if (FAILED(hr)) std::cout << "シャドウマップテクスチャの生成に失敗しました" << endl;

	// DSV
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Typeless に対応するフォーマット
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = device->CreateDepthStencilView(m_ShadowMapTex, &dsvDesc, &m_ShadowMapDSV);
	if (FAILED(hr)) std::cout << "シャドウマップ用DSVの生成に失敗しました" << endl;

	// SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;  // SRV用に変換
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(m_ShadowMapTex, &srvDesc, &m_ShadowMapSRV);
	if (FAILED(hr)) std::cout << "シャドウマップ用のSRVの生成に失敗しました" << endl;

	//-------------------------------------
	// シャドウイングを行うためのRTV
	//-------------------------------------
	auto iRenderSize = Renderer::GetInputRenderSize();

	// テクスチャ
	D3D11_TEXTURE2D_DESC hdrDesc = {};
	hdrDesc.Width = iRenderSize.x;
	hdrDesc.Height = iRenderSize.y;
	hdrDesc.MipLevels = 1;
	hdrDesc.ArraySize = 1;
	hdrDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	hdrDesc.SampleDesc.Count = 1;
	hdrDesc.Usage = D3D11_USAGE_DEFAULT;
	hdrDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	hr = device->CreateTexture2D(&hdrDesc, nullptr, &m_ShadowTex);
	if (FAILED(hr)) std::cout << "シャドウテクスチャの作成に失敗しました。" << std::endl;

	// レンダーターゲットビュー
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	hr = device->CreateRenderTargetView(m_ShadowTex, &rtvDesc, &m_ShadowRTV);
	if (FAILED(hr)) std::cout << "シャドウRTVの作成に失敗しました。" << std::endl;

	// シェーダーリソースビュー
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	hr = device->CreateShaderResourceView(m_ShadowTex, &srvDesc, &m_ShadowSRV);
	if (FAILED(hr)) std::cout << "シャドウSRVの作成に失敗しました。" << std::endl;

}
// HDRリソースの作成（基本リソース）
void SceneDemo::CreateHDRResource()
{
	auto device = Renderer::GetDevice();
	auto iRenderSize = Renderer::GetInputRenderSize();
	HRESULT hr = S_OK;

	//-------------------------------------
	// HDRリソース
	//-------------------------------------
	D3D11_TEXTURE2D_DESC hdrDesc = {};
	hdrDesc.Width = iRenderSize.x;
	hdrDesc.Height = iRenderSize.y;
	hdrDesc.MipLevels = 1;
	hdrDesc.ArraySize = 1;
	hdrDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	hdrDesc.SampleDesc.Count = 1;
	hdrDesc.Usage = D3D11_USAGE_DEFAULT;
	hdrDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	// テクスチャの作成
	hr = device->CreateTexture2D(&hdrDesc, nullptr, &m_hdrTex);
	if (FAILED(hr)) std::cout << "HDR テクスチャの作成に失敗しました。" << std::endl;

	// HDR用のRTV作成
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	hr = device->CreateRenderTargetView(m_hdrTex, &rtvDesc, &m_hdrRTV);
	if (FAILED(hr)) std::cout << "HDR RTVの作成に失敗しました。" << std::endl;

	// HDR用のSRV作成
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	hr = device->CreateShaderResourceView(m_hdrTex, &srvDesc, &m_hdrSRV);
	if (FAILED(hr)) std::cout << "HDR SRV の作成に失敗しました。" << std::endl;


	//-------------------------------------
	// 深度書き込み情報
	//-------------------------------------
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = iRenderSize.x;
	depthDesc.Height = iRenderSize.y;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; // typelessでDSV/SRV両対応
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;
	// テクスチャ
	hr = device->CreateTexture2D(&depthDesc, nullptr, &m_WriteDepthTex);
	if (FAILED(hr)) { cout << "深度書き込みテクスチャの作成に失敗しました" << endl; }

	// DSV
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = device->CreateDepthStencilView(m_WriteDepthTex, &dsvDesc, &m_WriteDepthDSV);
	if (FAILED(hr)) { cout << "深度書き込みDSVの作成に失敗しました" << endl; }

	// SRV
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(m_WriteDepthTex, &srvDesc, &m_WriteDepthSRV);
	if (FAILED(hr)) { cout << "深度書き込みSRVの作成に失敗しました" << endl; }


}
// ポストプロセスリソースの作成
void SceneDemo::CreatePostProcessResource()
{
	auto device = Renderer::GetDevice();
	auto iRenderSize = Renderer::GetInputRenderSize();
	HRESULT hr = S_OK;

	// テクスチャ情報
	D3D11_TEXTURE2D_DESC hdrDesc = {};
	hdrDesc.Width = iRenderSize.x;
	hdrDesc.Height = iRenderSize.y;
	hdrDesc.MipLevels = 1;
	hdrDesc.ArraySize = 1;
	hdrDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	hdrDesc.SampleDesc.Count = 1;
	hdrDesc.Usage = D3D11_USAGE_DEFAULT;
	hdrDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	// RTV情報
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	// SRV情報
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	
	//-------------------------------------
	// PingPong
	//-------------------------------------
	for (int i = 0; i < PINGPONG_NUM; ++i)
	{
		hr = device->CreateTexture2D(&hdrDesc, nullptr, &m_PingPongTex[i]);
		if (FAILED(hr)) std::cout << "ポストプロセステクスチャの作成に失敗しました。" << std::endl;

		hr = device->CreateRenderTargetView(m_PingPongTex[i], &rtvDesc, &m_PingPongRTV[i]);
		if (FAILED(hr)) std::cout << "ポストプロセスRTVの作成に失敗しました。" << std::endl;

		hr = device->CreateShaderResourceView(m_PingPongTex[i], &srvDesc, &m_PingPongSRV[i]);
		if (FAILED(hr)) std::cout << "ポストプロセスSRVの作成に失敗しました。" << std::endl;
	}

	//-------------------------------------
	// ブラー
	//-------------------------------------

	//// ブラーレンダリングサイズ
	//XMUINT2 blurSize[BLUR_NUM] = {
	//	{iRenderSize.x / 2,iRenderSize.y},		// 横方向ブラー（Horizontal）
	//	{iRenderSize.x / 2,iRenderSize.y / 2}	// 縦方向ブラー（Vertical）
	//};

	// ブラー用テクスチャ
	D3D11_TEXTURE2D_DESC blurDesc = {};
	blurDesc.Width = iRenderSize.x;
	blurDesc.Height = iRenderSize.y;
	blurDesc.MipLevels = 1;
	blurDesc.ArraySize = 1;
	blurDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;  // HDRフォーマット
	blurDesc.SampleDesc.Count = 1;
	blurDesc.Usage = D3D11_USAGE_DEFAULT;
	blurDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	// ブラー用RTV
	D3D11_RENDER_TARGET_VIEW_DESC blurRTVDesc = {};
	blurRTVDesc.Format = blurDesc.Format;
	blurRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	// ブラー用SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC blurSRVDesc = {};
	blurSRVDesc.Format = blurDesc.Format;
	blurSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	blurSRVDesc.Texture2D.MipLevels = 1;


	for (int i = 0; i < BLUR_NUM; ++i)
	{
		//// ブラーレンダリングサイズをセット
		//blurDesc.Width = blurSize[i].x;
		//blurDesc.Height = blurSize[i].y;

		hr = device->CreateTexture2D(&blurDesc, nullptr, &m_BlurTex[i]);
		if (FAILED(hr)) {
			std::cout << "ブラー用テクスチャの作成に失敗しました。" << std::endl;
		}

		hr = device->CreateRenderTargetView(m_BlurTex[i], &blurRTVDesc, &m_BlurRTV[i]);
		if (FAILED(hr)) {
			std::cout << "ブラー用RTVの作成に失敗しました。" << std::endl;
		}

		hr = device->CreateShaderResourceView(m_BlurTex[i], &blurSRVDesc, &m_BlurSRV[i]);
		if (FAILED(hr)) {
			std::cout << "ブラー用SRVの作成に失敗しました。" << std::endl;
		}
	}

	//-------------------------------------
	// 輝度（ブルーム表現用）
	//-------------------------------------

	hr = device->CreateTexture2D(&hdrDesc, nullptr, &m_LuminanceTex);
	if (FAILED(hr)) std::cout << "輝度書き込みテクスチャの作成に失敗しました。" << std::endl;

	hr = device->CreateRenderTargetView(m_LuminanceTex, &rtvDesc, &m_LuminanceRTV);
	if (FAILED(hr)) std::cout << "輝度書き込みRTVの作成に失敗しました。" << std::endl;

	hr = device->CreateShaderResourceView(m_LuminanceTex, &srvDesc, &m_LuminanceSRV);
	if (FAILED(hr)) std::cout << "輝度書き込みSRVの作成に失敗しました。" << std::endl;


	//-------------------------------------
	// 深度書き込み（DoF）
	//-------------------------------------
	// 深度バッファ作成
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = iRenderSize.x;
	depthDesc.Height = iRenderSize.y;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; // 深度 + ステンシル
	depthDesc.SampleDesc.Count = 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	// テクスチャ
	hr = device->CreateTexture2D(&depthDesc, nullptr, &m_WriteDepthTex);
	if (FAILED(hr)) { cout << "深度書き込みテクスチャの作成に失敗しました" << endl; }
	// DSV
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	hr = device->CreateDepthStencilView(m_WriteDepthTex, &dsvDesc, &m_WriteDepthDSV);
	if (FAILED(hr)) { cout << "深度書き込みDSVの作成に失敗しました" << endl; }
	// SRV
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(m_WriteDepthTex, &srvDesc, &m_WriteDepthSRV);
	if (FAILED(hr)) { cout << "深度書き込みSRVの作成に失敗しました" << endl; }

}


//-------------------------------------
// Setメソッド
//-------------------------------------
// レンダーターゲットをセット（ディファードレンダリング）
void SceneDemo::SetDeferredGBufferRenderTarget()
{
	auto context = Renderer::GetDeviceContext();

	// Gバッファをレンダーターゲットとして設定
	ID3D11RenderTargetView* rtv[] = {
		m_DeferredGBufferRTVs[0], // Albedo
		m_DeferredGBufferRTVs[1], // Normal
		m_DeferredGBufferRTVs[2], // WorldPos
		m_DeferredGBufferRTVs[3], // Depth
		m_DeferredGBufferRTVs[4], // MVector
	};
	
	context->OMSetRenderTargets(GBUFFER_NUM, rtv, m_WriteDepthDSV);

	// Gバッファをクリア
	float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	for (int i = 0; i < GBUFFER_NUM; ++i) {	
		context->ClearRenderTargetView(m_DeferredGBufferRTVs[i], clearColor);
	}
	context->ClearDepthStencilView(m_WriteDepthDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

}
// ディファードレンダリングリソースをセット
void SceneDemo::SetDeferredShaderResource()
{
	auto context = Renderer::GetDeviceContext();

	// SRVをピクセルシェーダーにバインド
	context->PSSetShaderResources(0, GBUFFER_NUM, m_DeferredGBufferSRVs);

	context->PSSetShaderResources(6, 1, &m_WriteDepthSRV);	// 深度情報

}

// DLSS用のレンダーターゲットを指定
void SceneDemo::SetDLSSRenderTarget()
{
	auto context = Renderer::GetDeviceContext();

	// Gバッファをレンダーターゲットとして設定
	ID3D11RenderTargetView* rtv[] = {
		m_DLSSGBufferRTVs[0],	// Albedo
		m_DLSSGBufferRTVs[1],	// Normal
		m_DLSSGBufferRTVs[2],	// WorldPos
		m_DLSSGBufferRTVs[3],	// Depth（ダミー）
		m_DLSSGBufferRTVs[4],	// MVector
	};;

	// ビューの数
	unsigned int viewNum = GBUFFER_NUM;

	context->OMSetRenderTargets(viewNum, rtv, m_DLSSDepthDSV);

	// Gバッファをクリア
	float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	for (int i = 0; i < viewNum; ++i) {
		context->ClearRenderTargetView(rtv[i], clearColor);
	}
	context->ClearDepthStencilView(m_DLSSDepthDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}
// DLSS用のシェーダーリソースを指定
void SceneDemo::SetDLSSShaderResource()
{
	auto context = Renderer::GetDeviceContext();

	// SRVをピクセルシェーダーにバインド
	context->PSSetShaderResources(0, GBUFFER_NUM, m_DLSSGBufferSRVs);
	// DSVで書き込んだ深度値をバインド
	context->PSSetShaderResources(3, 1, &m_DLSSDepthSRV);
}
// アップスケール後のリソースをバインド
void SceneDemo::SetDLSSOutputResource()
{
	auto context = Renderer::GetDeviceContext();

	// SRVをピクセルシェーダーにバインド
	context->PSSetShaderResources(0, 1, &m_DLSSOutputSRV);
}

// シャドウマップのセット
void SceneDemo::SetShadowMap()
{
	auto context = Renderer::GetDeviceContext();

	// バインド
	context->PSSetShaderResources(5, 1, &m_ShadowMapSRV);
}
// ポストプロセス用リソースをセット
void SceneDemo::ChangePingPongResource()
{
	auto context = Renderer::GetDeviceContext();

	// RTVを設定（描画先）
	context->OMSetRenderTargets(1, &m_PingPongRTV[currentIndex], nullptr);

	// インデックスを切り替え（0 <-> 1）
	currentIndex = 1 - currentIndex;

	// 前回の描画結果をバインド
	context->PSSetShaderResources(0, 1, &m_PingPongSRV[currentIndex]);


}
// ポストプロセス用レンダーターゲットをクリア
void SceneDemo::ClearPingPongRenderTarget()
{
	auto context = Renderer::GetDeviceContext();

	// 青色でクリア
	float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };

	for (int i = 0; i < PINGPONG_NUM; ++i) {
		// RTVをクリア
		context->ClearRenderTargetView(m_PingPongRTV[i], clearColor);
	}

	// 参照する要素番号をリセット
	currentIndex = 0;
}

// ブラー出力用レンダーターゲットをクリア
void SceneDemo::ClearBlurRenderTarget()
{
	auto context = Renderer::GetDeviceContext();

	// 青色でクリア
	float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };

	for (int i = 0; i < BLUR_NUM; ++i) {
		// RTVをクリア
		context->ClearRenderTargetView(m_BlurRTV[i], clearColor);
	}


}
// 横方向のブラー抽出の準備
void SceneDemo::SetBlurXResource(ID3D11ShaderResourceView* _srv, float _blurX)
{
	auto context = Renderer::GetDeviceContext();
	auto iRenderSize = Renderer::GetInputRenderSize();

	// RTVを設定
	context->OMSetRenderTargets(1, &m_BlurRTV[0], nullptr);

	// 前回の描画結果をバインド
	context->PSSetShaderResources(0, 1, &_srv);

	// 横方向のブラー
	Param.blurParam[0] = (float)iRenderSize.x * 0.5f;	// ブラーを行うテクスチャの横幅
	Param.blurParam[1] = (float)iRenderSize.y;			// ブラーを行うテクスチャの縦幅
	Param.blurParam[2] = _blurX;						// x方向のブラー
	Param.blurParam[3] = 0.0f;							// y方向のブラー
	Renderer::SetExtraParam(Param);						// バインド

}
// 縦方向のブラー抽出の準備
void SceneDemo::SetBlurYResource(ID3D11ShaderResourceView* _srv, float _blurY)
{
	auto context = Renderer::GetDeviceContext();

	// RTVを設定
	context->OMSetRenderTargets(1, &m_BlurRTV[1], nullptr);

	// 横方向のブラー結果をバインド
	context->PSSetShaderResources(0, 1, &_srv);

	// 縦方向のブラー
	Param.blurParam[1] *= 0.5f;		// ブラーを行うテクスチャの縦幅
	Param.blurParam[2]  = 0.0f;		// x方向のブラー
	Param.blurParam[3]  = _blurY;	// y方向のブラー
	Renderer::SetExtraParam(Param);	// バインド

}
// 被写界深度の準備
void SceneDemo::SetDoFResource()
{
	auto context = Renderer::GetDeviceContext();

	// RTVを設定
	context->OMSetRenderTargets(1, &m_PingPongRTV[currentIndex], nullptr);

	// インデックスを切り替え（0 <-> 1）
	currentIndex = 1 - currentIndex;


	// 被写界深度に必要なリソースを全てバインド
	context->PSSetShaderResources(0, 1, &m_hdrSRV);			// 描画結果
	context->PSSetShaderResources(6, 1, &m_WriteDepthSRV);	// 深度情報
	context->PSSetShaderResources(7, 1, &m_BlurSRV[1]);		// ブラー結果
}
// 輝度抽出の準備
void SceneDemo::SetLuminanceResource()
{
	auto context = Renderer::GetDeviceContext();

	// RTVを設定
	context->OMSetRenderTargets(1, &m_LuminanceRTV, nullptr);

	// 黒色でクリア
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	// RTVをクリア
	context->ClearRenderTargetView(m_LuminanceRTV, clearColor);


	// インデックスを切り替え（0 <-> 1）
	currentIndex = 1 - currentIndex;

	// 描画結果をセット
	context->PSSetShaderResources(0, 1, &m_PingPongSRV[currentIndex]);

	// インデックスを切り替え（0 <-> 1）
	currentIndex = 1 - currentIndex;
}
// ブルームライティングの準備
void SceneDemo::SetBloomResource()
{
	auto context = Renderer::GetDeviceContext();

	// RTVを設定
	context->OMSetRenderTargets(1, &m_PingPongRTV[currentIndex], nullptr);

	// インデックスを切り替え（0 <-> 1）
	currentIndex = 1 - currentIndex;

	// 描画結果と輝度抽出結果をセット
	context->PSSetShaderResources(0, 1, &m_PingPongSRV[currentIndex]);
	context->PSSetShaderResources(8, 1, &m_BlurSRV[1]);

}


// シャドウイング後のシェーダーリソースをバインド
void SceneDemo::SetShadowResource()
{
	auto context = Renderer::GetDeviceContext();

	// SRVをピクセルシェーダーにバインド
	context->PSSetShaderResources(0, 1, &m_ShadowSRV);

}
// オフスクリーン先のレンダーターゲットを指定
void SceneDemo::SetHDRRenderTarget(bool _isClearRTV,ID3D11DepthStencilView* _dsv,bool _isClearDSV)
{
	auto context = Renderer::GetDeviceContext();


	// オフスクリーンレンダーターゲットをセット
	context->OMSetRenderTargets(1, &m_hdrRTV, _dsv);


	// レンダーターゲットのクリア
	if (_isClearRTV)
	{
		// 青色でクリア
		float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
		// RTVをクリア
		context->ClearRenderTargetView(m_hdrRTV, clearColor);
	}
	// DSVのクリア
	if (_isClearDSV)
	{
		context->ClearDepthStencilView(_dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
}
// HDR出力のシェーダーリソースをバインド
void SceneDemo::SetHDRResource()
{
	auto context = Renderer::GetDeviceContext();

	// SRVをピクセルシェーダーにバインド
	context->PSSetShaderResources(0, 1, &m_hdrSRV);			// 描画結果

	context->PSSetShaderResources(6, 1, &m_WriteDepthSRV);	// 深度情報

}


