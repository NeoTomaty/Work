#include "SceneDemo.h"
#include "DLSSManager.h"
#include <map>

using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace std;


//-------------------------------------
// �񋓌^
//-------------------------------------
// ON/OFF��
enum Enable
{
	DISABLE,
	ENABLE,
};
// �����_�����O���[�h��
enum class RenderMode {
	NONE,		// �Ȃ��i�t�H���[�h�����_�����O�j
	DEFERRED,	// �f�B�t�@�[�h�����_�����O
	DLSS,		// DLSS�X�[�p�[�T���v�����O
	DDLSS,		// �f�B�t�@�[�h�{DLSS
};
// �o�b�t�@���[�h��
enum class BufferMode {
	DEFAULT = 0,	//0�F�ʏ�
	COLOR,			//1�F�A���x�h
	NORMAL,			//2�F�@��
	WORLD,			//3�F���[���h���W
	DEPTH,			//4�F�[�x
	MVECTOR,		//5�F���[�V�����x�N�g��
};
// �A���`�G�C���A�V���O���[�h��
enum class  AntiAliasingMode {
	NONE,
	DLSS,
};


//-------------------------------------
// ImGui�p�����[�^
//-------------------------------------
ExtraParam	Param{};					// �e�N�X�`�����
static int renderPresetIndex = 0;		// ���ݑI������Ă���v���Z�b�g�̃C���f�b�N�X
static int perfQualityIndex = 0;		// ���ݑI������Ă���p�t�H�[�}���X�i���̃C���f�b�N�X
static int bufferMode = 0;				// ���ݑI������Ă���o�b�t�@���[�h
static bool isResetResource = false;	// �~�ς��ꂽ�p�����[�^��j�����邩
static bool isInitDLSS = false;			// DLSS�@�\�̏��������o���Ă��邩�H
static bool isUseDLSSFeature = false;	// DLSS���g�p���邩�ǂ���

// �����_�����O���[�h
RenderMode g_RenderMode = RenderMode::NONE;
// �A���`�G�C���A�V���O���[�h
AntiAliasingMode g_AntiAliasingMode
= AntiAliasingMode::NONE;

//-------------------------------------
// DLSS�֘A�p�����[�^
//-------------------------------------
// �N�I���e�B���X�g
std::vector<PERF_QUALITY_ITEM> PERF_QUALITY_LIST =
{
	{NVSDK_NGX_PerfQuality_Value_MaxPerf,          "Performance", false, false},	// �����_�����O�{��2.0�{
	{NVSDK_NGX_PerfQuality_Value_Balanced,         "Balanced"   , false, false},	// �����_�����O�{��1.7�{
	{NVSDK_NGX_PerfQuality_Value_MaxQuality,       "Quality"    , false, false},	// �����_�����O�{��1.5�{
	{NVSDK_NGX_PerfQuality_Value_UltraPerformance, "UltraPerf"  , false, false},	// �����_�����O�{��3.0�{
	{NVSDK_NGX_PerfQuality_Value_DLAA,             "DLAA"       , false, false},	// �A���`�G�C���A�V���O
};
// DLSS�̐����ݒ�i�掿���[�h�ʁj
map<NVSDK_NGX_PerfQuality_Value, DlssRecommendedSettings> g_RecommendedSettingsMap;
// �O�t���[���̃N�H���e�B���[�h
NVSDK_NGX_PerfQuality_Value PrevQuality = 
PERF_QUALITY_LIST[perfQualityIndex].PerfQuality;
// ���t���[���̃N�H���e�B���[�h
NVSDK_NGX_PerfQuality_Value PerfQualityMode =
PERF_QUALITY_LIST[perfQualityIndex].PerfQuality;


//-------------------------------------
// ���C�g�p�����[�^
//-------------------------------------
static SceneLight	Lights;	// ���C�g
static DLIGHTTYPE LightType = DLIGHTTYPE::NOON;	// ���s�����̎��

//-------------------------------------
// ���̑��p�����[�^
//-------------------------------------
static int currentIndex = 0;	// �|�X�g�v���Z�X���\�[�X�̗v�f�ԍ�
static PPparam isPP;			// �|�X�g�v���Z�X�̓K�p�t���O
static int isOutline = 1;		// �֊s���̕\��

//-------------------------------------
// DebugUI
//-------------------------------------
// DLSS�f�o�b�O
void DebugDLSS() {

	if (g_RenderMode == RenderMode::DLSS || g_RenderMode == RenderMode::DDLSS)
	{
		ImGui::Begin("DLSS Setting");

		auto input = Renderer::GetInputRenderSize();
		auto output = Renderer::GetOutputRenderSize();

		// ���݂̃����_�����O�T�C�Y��\��
		ImGui::Text("Input Render Size: %d x %d", input.x, input.y);
		ImGui::Text("Output Render Size: %d x %d", output.x, output.y);

		// �����_�����O�T�C�Y�̕ύX
		// �𑜓x�̑I������p��
		const char* resolutionOptions[] = { "540p", "720p", "1080p", "4K" };
		static int selectedResolutionIndex = 2;  // �����l��1080p�ɐݒ�i�C���f�b�N�X2�j

		// �𑜓x��I��
		if (ImGui::Combo("Output Resolution", &selectedResolutionIndex, resolutionOptions, IM_ARRAYSIZE(resolutionOptions))) {
			// �����_�����O�T�C�Y���X�V
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
			// �o�̓T�C�Y���X�V
			Renderer::SetOutputRenderSize(output);
		}

		// DLSS�Œ~�ς������\�[�X����������ON/OFF
		if (ImGui::Checkbox("Use DLSS", &isUseDLSSFeature)) {
			// �`�F�b�N��Ԃ��ύX���ꂽ�Ƃ��ɌĂяo�����
			if (isUseDLSSFeature) {
				printf("Use DLSS Enabled\n");
			}
			else {
				printf("Use DLSS Disabled\n");
			}
		}

		// DLSS�Œ~�ς������\�[�X����������ON/OFF
		if (ImGui::Checkbox("Reset Resource", &isResetResource)) {
			// �`�F�b�N��Ԃ��ύX���ꂽ�Ƃ��ɌĂяo�����
			if (isResetResource) {
				printf("Reset Enabled\n");
			}
			else {
				printf("Reset Disabled\n");
			}
		}

		// �A���`�G�C���A�V���O���[�h�̑I��
		const char* aaModes[] = { "NONE", "DLSS" };
		int currentAAMode = static_cast<int>(g_AntiAliasingMode); // ���݂̃��[�h�𐮐��l�ɕϊ�
		if (ImGui::Combo("Anti-Aliasing Mode", &currentAAMode, aaModes, IM_ARRAYSIZE(aaModes))) {
			g_AntiAliasingMode = static_cast<AntiAliasingMode>(currentAAMode); // �I�����ꂽ���[�h�𒼐ړK�p
			printf("Selected Anti-Aliasing Mode: %s\n", aaModes[currentAAMode]);
		}

		// �p�t�H�[�}���X�i���̑I��
		std::vector<const char*> perfQualityItems;

		for (const auto& quality : PERF_QUALITY_LIST) {
			perfQualityItems.push_back(quality.PerfQualityText);
		}

		// �A���`�G�C���A�V���O���[�h��DLSS�̎�
		if (g_AntiAliasingMode == AntiAliasingMode::DLSS)
		{
			if (ImGui::Combo("DLSS Performance Quality", &perfQualityIndex, perfQualityItems.data(), static_cast<int>(perfQualityItems.size()))) {
				// �p�t�H�[�}���X�i�����ύX���ꂽ�ꍇ�ɌĂяo�����
				printf("Selected Performance Quality: %s\n", perfQualityItems[perfQualityIndex]);
			}
		}

		ImGui::End();
	}
}
// ���s�������C�e�B���O�f�o�b�O
void DebugDLight()
{
	ImGui::Begin("DirectionalLight Parameter");

	Directional DLight = Lights.GetDirectionalLight();

	Vector3 target = Lights.SetTarget();

	ImGui::Text("Position: (%.2f, %.2f, %.2f)", DLight.Position.x, DLight.Position.y, DLight.Position.z);
	ImGui::Text("Direction: (%.2f, %.2f, %.2f)", DLight.Direction.x, DLight.Direction.y, DLight.Direction.z);
	ImGui::Text("Target: (%.2f, %.2f, %.2f)", target.x, target.y, target.z);

	// ���s�����̗L��
	bool enable = (DLight.Enable == TRUE); // BOOL -> bool �ɕϊ�
	// �`�F�b�N�{�b�N�X�̒l���X�V
	if (ImGui::Checkbox("Light Enabled", &enable))
	{
		DLight.Enable = (enable ? TRUE : FALSE); // bool -> BOOL �ɕϊ�
	}

	// ���C�g�̍��W
	static float position[3] = { DLight.Position.x, DLight.Position.y, DLight.Position.z };
	if (ImGui::SliderFloat3("Position", position, -200.0f, 200.0f))
	{
		// �ύX�𔽉f������
		DLight.Position = Vector3(position[0], position[1], position[2]);
	}

	// ���C�g�̕���
	Vector3 direction = target - DLight.Position;
	direction.Normalize();
	DLight.Direction = Vector4(direction.x, direction.y, direction.z, 0.0f);

	const char* lightTypeNames[] = { "FREE", "MORNING", "NOON", "EVENING", "NIGHT" };

	// ���s�����̎�ނ�ݒ�
	int currentItem = static_cast<int>(LightType);
	if (ImGui::Combo("DirectionalLight Type", &currentItem, lightTypeNames, IM_ARRAYSIZE(lightTypeNames)))
	{
		// �ύX�𔽉f
		LightType = static_cast<DLIGHTTYPE>(currentItem);
	}

	// ���C�g�^�C�v�����R�Ȃ�
	if (LightType == DLIGHTTYPE::FREE)
	{
		// ���s�����̊g�U���ˌ�
		static float diffuse[3] = { DLight.Diffuse.x, DLight.Diffuse.y, DLight.Diffuse.z };
		if (ImGui::ColorEdit3("Diffuse", diffuse))
		{
			DLight.Diffuse = Color(diffuse[0], diffuse[1], diffuse[2], 1.0f);
		}
		// ���s�����̊���
		static float ambient[3] = { DLight.Ambient.x, DLight.Ambient.y, DLight.Ambient.z };
		if (ImGui::ColorEdit3("Ambient", ambient))
		{
			DLight.Ambient = Color(ambient[0], ambient[1], ambient[2], 1.0f);
		}

	}

	Lights.SetDirectionalLight(DLight);

	ImGui::End();
}
// �_�������C�e�B���O�f�o�b�O
void DebugPLight()
{

	ImGui::Begin("PointLight Parameter");

	// �S�Ă̓_����
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

	// �ʂ̓_����
	for (int i = 0; i < POINTLIGHT_NUM; ++i)
	{

		Point PLight = Lights.GetPointLight(i);

		ImGui::PushID(i); // �e���C�g�p�̎��ʎq

		// ���C�g���iPointLight1�Ȃǁj
		std::string nodeName = "PointLight " + std::to_string(i + 1);

		// �c���[�\���Ő܂肽���ݎ�UI
		if (ImGui::TreeNode(nodeName.c_str()))
		{
			// �_�����̉�
			bool pEnable = (PLight.Enable == TRUE);
			if (ImGui::Checkbox("Enabled", &pEnable)) {
				PLight.Enable = (pEnable ? TRUE : FALSE);
			}

			// ���W
			ImGui::Text("Position");
			ImGui::SliderFloat3("Pos", &PLight.Position.x, -200.0f, 200.0f);

			// ���邳�iIntensity�j
			ImGui::SliderFloat("Intensity", &PLight.Intensity, 0.0f, 100.0f);

			// �g�U���ˌ�
			static float pDiffuse[POINTLIGHT_NUM][3];
			pDiffuse[i][0] = PLight.Diffuse.x;
			pDiffuse[i][1] = PLight.Diffuse.y;
			pDiffuse[i][2] = PLight.Diffuse.z;
			if (ImGui::ColorEdit3("Diffuse", pDiffuse[i])) {
				PLight.Diffuse = Color(pDiffuse[i][0], pDiffuse[i][1], pDiffuse[i][2], 1.0f);
			}

			// ����
			static float pAmbient[POINTLIGHT_NUM][3];
			pAmbient[i][0] = PLight.Ambient.x;
			pAmbient[i][1] = PLight.Ambient.y;
			pAmbient[i][2] = PLight.Ambient.z;
			if (ImGui::ColorEdit3("Ambient", pAmbient[i])) {
				PLight.Ambient = Color(pAmbient[i][0], pAmbient[i][1], pAmbient[i][2], 1.0f);
			}

			// �����W���iAttenuation�j
			ImGui::Text("Attenuation");
			ImGui::SliderFloat("Constant", &PLight.Attenuation.x, 0.0f, 10.0f);
			ImGui::SliderFloat("Linear", &PLight.Attenuation.y, 0.0f, 1.0f);
			ImGui::SliderFloat("Quadratic", &PLight.Attenuation.z, 0.0f, 1.0f);

			ImGui::TreePop(); // TreeNode�̏I��

			Lights.SetPointLight(i, PLight);
		}

		ImGui::PopID(); // ID�̃��Z�b�g

	}

	ImGui::End();
}
// �����_�����O�f�o�b�O
void DebugRender()
{
	ImGui::Begin("Rendering Setting");

	// �����_�����O���[�h�I��p�̐ÓI�ϐ�
	static int renderMode = static_cast<int>(g_RenderMode);

	// �����_�����O���[�h�̑I�����i������z��j
	const char* renderItems[] = { "NONE", "DEFERRED", "DLSS", "DEFERRED+DLSS"};

	// Combo�{�b�N�X�Ń����_�����O���[�h��I��
	if (ImGui::Combo("Select RenderMode", &renderMode, renderItems, IM_ARRAYSIZE(renderItems))) {
		// ���[�U�[���I����ύX�����ꍇ�ɌĂяo�����
		printf("Selected RenderMode: %s\n", renderItems[renderMode]);

		// �O���[�o���ϐ����X�V
		g_RenderMode = static_cast<RenderMode>(renderMode);
	}

	// �f�B�t�@�[�h�����_�����O���s���Ă���ꍇ�ɒǉ��̑I�����ڂ�\��
	if (g_RenderMode == RenderMode::DEFERRED || g_RenderMode == RenderMode::DDLSS)
	{
		// �o�b�t�@�I�����i������z��j
		const char* bufferItems[] = { "DEFAULT", "COLOR", "NORMAL", "WORLD", "DEPTH", "MVECTOR" };

		// Combo�{�b�N�X�Ńo�b�t�@��I��
		if (ImGui::Combo("Select Buffer", &bufferMode, bufferItems, IM_ARRAYSIZE(bufferItems))) {
			// �I�����ꂽ�o�b�t�@�ɉ����ď���
			printf("Selected Buffer: %s\n", bufferItems[bufferMode]);
		}
	}

	ImGui::End();
}
// �|�X�g�v���Z�X�f�o�b�O
void DebugPostProcess()
{
	ImGui::Begin("Visual Effects");

	// �e�|�X�g�v���Z�X�̐؂�ւ�
	bool enableDoF = (isPP.isDoF == TRUE);
	bool enableBloom = (isPP.isBloom == TRUE);
	bool enableVignette = (isPP.isVignette == TRUE);


	// �V���h�E��ON/OFF
	bool enableShadow = (Param.isShadow == 1);
	if (ImGui::Checkbox("Shadow", &enableShadow))
	{
		Param.isShadow = (enableShadow ? 1 : 0);
	}

	ImGui::Separator();

	// �֊s����ON/OFF
	bool enableOutline = (isOutline == 1);
	if (ImGui::Checkbox("OutLine", &enableOutline))
	{
		isOutline = (enableOutline ? 1 : 0);
	}


	ImGui::Separator();

	// �`�F�b�N�{�b�N�X��\�����čX�V
	if (ImGui::Checkbox("Depth of Field", &enableDoF))
	{
		isPP.isDoF = (enableDoF ? TRUE : FALSE);
	}
	// �X���C�_�[��Focus�𒲐�
	ImGui::Text("Depth of Field Parameters");
	ImGui::SliderFloat("Focus", &Param.focus, 0.1f, 500.0f);
	ImGui::SliderFloat("Range", &Param.DoFRange, 0.1f, 500.0f);
	
	ImGui::Separator();
	
	// �u���[���\��
	if (ImGui::Checkbox("Bloom", &enableBloom))
	{
		isPP.isBloom = (enableBloom ? TRUE : FALSE);
	}

	ImGui::SliderFloat("Bloom Strength", &Param.bloomStrength, 0.1f, 1.0f);

	ImGui::Separator();

	// �r�l�b�g
	if (ImGui::Checkbox("Vignette", &enableVignette))
	{
		isPP.isVignette = (enableVignette ? TRUE : FALSE);
	}
	// �X���C�_�[�Ńr�l�b�g���x�𒲐�
	ImGui::Text("Vignette Parameters");
	ImGui::SliderFloat("Strength", &Param.vignetteStrength, 0.1f, 1.0f);
	ImGui::SliderFloat("Radius", &Param.vignetteRadius, 0.1f, 1.0f);
	ImGui::SliderFloat2("Center", Param.vignetteCenter, 0.0f, 1.0f);

	// isPPAll �̏�Ԃ��X�V
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
// �V�[�����\�b�h
//-------------------------------------
// �V�[���̏�����
void SceneDemo::SceneInit()
{

	// ���f���t�@�C���p�X
	std::vector<std::string> filename =
	{
		"assets/model/Cottage/cottage.fbx",	// 0
		"assets/model/SkySphere.fbx",		// 1
		"assets/model/Cube.fbx",			// 2
		"assets/model/Mount.fbx",			// 3
		"assets/model/Sphere.fbx",			// 4
	};
	// �e�N�X�`���t�@�C���p�X
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
	// �X�v���C�g�t�@�C���p�X
	std::vector<std::string> spritefilename =
	{
		"assets/texture/UnityChan.png",		// 0
		"assets/texture/Lamp.png",			// 1
		"assets/texture/TreeA.png",			// 2
		"assets/texture/TreeB.png",			// 3

		"assets/texture/UnityChan_normal.png",	// 4
	};
	// �V�F�[�_�[�t�@�C���p�X
	std::vector<std::string> shaderfile =
	{
		"VS_Default.hlsl",				// 0		��{�I�Ȓ��_����n��
		"PS_UnlitShader.hlsl",			// 1		���C�e�B���O�Ȃ�
		"PS_LitShader.hlsl",			// 2		���C�e�B���O����

		"PS_GBuffer.hlsl",				// 3		G�o�b�t�@�o��
		"PS_DeferredRendering.hlsl",	// 4		G�o�b�t�@��p�������C�e�B���O
		"PS_DLSSOffScreenColor.hlsl",	// 5		DLSS�p�I�t�X�N���[��
		"PS_DLSSOutput.hlsl",			// 6		DLSS�p�����_�����O

		"PS_SkySphere.hlsl",			// 7		SkySphere�p

		"VS_ShadowMap.hlsl",			// 8		�V���h�E�}�b�v�p���_���
		"PS_AlphaClip.hlsl",			// 9		�V���h�E�}�b�v�o��
		"PS_Shadow.hlsl",				// 10		�V���h�E�C���O

		"VS_Outline.hlsl",				// 11		�֊s��
		"PS_Outline.hlsl",				// 12		�֊s��

		"PS_DoF.hlsl",					// 13		��ʊE�[�x
		"PS_Blur.hlsl",					// 14		�u���[���o

		"PS_Luminance.hlsl",			// 15		�P�x���o
		"PS_Bloom.hlsl",				// 16		�u���[�����C�e�B���O
		"PS_Vignette.hlsl",				// 17		�r�l�b�g

		"PS_ToneMapping.hlsl",			// 18		�g�[���}�b�s���O
	};


	// �J����������
	m_Camera.Init();

	// �X�N���[���N�A�b�h������
	m_Screen.Init();

	// �t�B�[���h������
	{
		XMUINT2 div = { 20,20 };			// ������
		XMUINT2 size = { 450,450 };			// �T�C�Y
		Vector3 pos = { 0.0f,0.0f,0.0f };	// ���W

		m_Moss.Init(div, size, pos, texfilename[4], texfilename[5]);

		div = { 40,3 };					// ������
		size = { 450,30 };				// �T�C�Y
		pos = { 0.0f,0.001f,-100.0f };	// ���W

		m_TileA.Init(div, size, pos, texfilename[14], texfilename[15]);

		div = { 3,4 };					// ������
		size = { 30,50 };				// �T�C�Y
		pos = { 30.0f,0.001f, -60.0f };	// ���W
	
		m_TileB.Init(div, size, pos, texfilename[14], texfilename[15]);
	}

	// �����̏�����
	Lights.InitLight();


	// �����_�����O���\�[�X�̐���
	CreateRenderResource();

	// Imgui�p�f�o�b�O�֐��Ăяo��
	DebugUI::RedistDebugFunction(DebugDLight);
	DebugUI::RedistDebugFunction(DebugPLight);
	DebugUI::RedistDebugFunction(DebugDLSS);
	DebugUI::RedistDebugFunction(DebugRender);
	DebugUI::RedistDebugFunction(DebugPostProcess);

	// �V�F�[�_�[
	{
		m_ShaderUnlit.Create(shaderfile[0], shaderfile[1]);				// ���C�e�B���O�Ȃ�
		m_ShaderLit.Create(shaderfile[0], shaderfile[2]);				// ���C�e�B���O����
		m_ShaderGBuffer.Create(shaderfile[0], shaderfile[3]);			// G�o�b�t�@�o��
		m_ShaderDeferredRendering.Create(shaderfile[0], shaderfile[4]);	// �f�B�t�@�[�h�����_�����O
		m_ShaderDLSSInput.Create(shaderfile[0], shaderfile[5]);			// DLSS�̓��͒l�i�I�t�X�N���[���j
		m_ShaderDLSSOutput.Create(shaderfile[0], shaderfile[6]);		// DLSS�̏o�͒l
		m_ShaderSkySphere.Create(shaderfile[0], shaderfile[7]);			// SkySphere�p
		m_ShaderShadowMap.Create(shaderfile[8], shaderfile[1]);			// �V���h�E�}�b�v
		m_ShaderShadowAlphaClip.Create(shaderfile[8], shaderfile[9]);	// �A���t�@�N���b�v
		m_ShaderShadow.Create(shaderfile[0], shaderfile[10]);			// �V���h�E�C���O
		m_ShaderOutline.Create(shaderfile[11], shaderfile[12]);			// �֊s��
		m_ShaderDoF.Create(shaderfile[0], shaderfile[13]);				// ��ʊE�[�x
		m_ShaderBlur.Create(shaderfile[0], shaderfile[14]);				// �u���[���o
		m_ShaderLuminance.Create(shaderfile[0], shaderfile[15]);		// �P�x���o
		m_ShaderBloom.Create(shaderfile[0], shaderfile[16]);			// �t���[��
		m_ShaderVignette.Create(shaderfile[0], shaderfile[17]);			// �r�l�b�g
		m_ShaderToneMapping.Create(shaderfile[0], shaderfile[18]);		// �g�[���}�b�s���O
	}


	// ���j�e�B����񏉊���
	m_UnityChan.Init(
		spritefilename[0],
		spritefilename[4],
		{ 0.0f,2.0f,-50.0f },	// �|���S���̒��S���W
		{ 10,10 },				// �|���S���̒���
		{ 0,0,0 },				// ��]
		{ 1,1,1 },				// �傫��
		true,					// �A�j���[�V�����̗L��
		{ 4,1 },				// uv������
		10.0f);					// �A�j���[�V�������x

	// �؂̓ǂݍ���
	{
		// ��A��XZ���W
		float treeAPosX[TREE_NUM] = { -25.0f,-5.0f,30.0f };
		float treeAPosZ[TREE_NUM] = { -10.0f,20.0f,0.0f };
		// ��A��XZ���W
		float treeBPosX[TREE_NUM] = { -40.0f,60.0f,15.0f };
		float treeBPosZ[TREE_NUM] = { -30.0f,-25.0f,-65.0f };

		for (auto i = 0; i < TREE_NUM; i++)
		{

			// ��A�̏�����
			m_TreeA[i].Init(
				spritefilename[2],
				"",
				{ treeAPosX[i],7.5f,treeAPosZ[i] },	// �|���S���̒��S���W
				{ 30,30 },							// �|���S���̒���
				{ 0,0,0 },							// ��]
				{ 1,1,1 },							// �傫��
				true,								// �A�j���[�V�����̗L��
				{ 4,1 },							// uv������
				10.0f);								// �A�j���[�V�������x


			// ��B�̏�����
			m_TreeB[i].Init(
				spritefilename[3],
				"",
				{ treeBPosX[i],7.5f,treeBPosZ[i] },	// �|���S���̒��S���W
				{ 30,30 },							// �|���S���̒���
				{ 0,0,0 },							// ��]
				{ 1,1,1 },							// �傫��
				true,								// �A�j���[�V�����̗L��
				{ 4,1 },							// uv������
				10.0f);								// �A�j���[�V�������x

		}
	}

	// SkyBox�ǂݍ���
	{
		std::string f = filename[1];

		// ���f���ǂݍ���
		m_Skysphere.Load(f);

		// �e�N�X�`���ǂݍ���
		f = texfilename[2];
		m_Skysphere.LoadTex(f, DIFFUSE);

		// �����_���[�Ƀ��f����n��
		m_MRSkysphere.Init(m_Skysphere);

		Vector3 pos = Vector3(0.0f, 0.0f, 0.0f);
		Vector3 rotate = Vector3(0.0f, 0.0f, 0.0f);
		Vector3 scale = Vector3(0.4f, 0.4f, 0.4f);
		m_MRSkysphere.SetPosition(pos);
		m_MRSkysphere.SetRotation(rotate);
		m_MRSkysphere.SetScale(scale);
	}
	// Cottage�ǂݍ���
	{
		std::string f;

		// ���W�A��]�A�X�P�[�����ʂɒ�`
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

			// ���f���ǂݍ���
			m_Cottage[i].Load(f);

			// �e�N�X�`���ǂݍ���
			f = texfilename[0];
			m_Cottage[i].LoadTex(f, TexType::DIFFUSE);
			f = texfilename[1];
			m_Cottage[i].LoadTex(f, TexType::NORMAL);


			// �����_���[�Ƀ��f����n��
			m_MRCottage[i].Init(m_Cottage[i]);

			m_MRCottage[i].SetPosition(pos[i]);
			m_MRCottage[i].SetRotation(Vector3(0.0f, rotate[i], 0.0f));
			m_MRCottage[i].SetScale(scale[i]);
		}

	}
	// �R�ǂݍ���
	{
		std::string f = filename[3];

		// ���f���ǂݍ���
		m_Mountain.Load(f);

		// �e�N�X�`���ǂݍ���
		f = texfilename[4];
		m_Mountain.LoadTex(f, TexType::DIFFUSE);

		// �����_���[�Ƀ��f����n��
		m_MRMountain.Init(m_Mountain);

		Vector3 pos = Vector3(0.0f, 0.25f, 150.0f);
		Vector3 rotate = Vector3(0.0f, 0.0f, 0.0f);
		Vector3 scale = Vector3(0.6f, 0.3f, 0.2f);
		m_MRMountain.SetPosition(pos);
		m_MRMountain.SetRotation(rotate);
		m_MRMountain.SetScale(scale);
	}

	// �����v�y�у��C�g�I�u�W�F�N�g�ǂݍ���
	{

		// �����v�̈ʒu
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
				lampPos[i],				// �|���S���̒��S���W
				{ 10,12 },				// �|���S���̒���
				{ 0,0,0 },				// ��]
				{ 1.0f,1.5f,1.0f },		// �傫��
				false);					// �A�j���[�V�����̗L��

		}

		// ���C�g�I�u�W�F�N�g�i�f�o�b�O�p�j
		for (auto i = 0; i < POINTLIGHT_NUM; i++)
		{
			Point PLight = Lights.GetPointLight(i);

			std::string f = filename[4];

			// ���f���ǂݍ���
			m_LightObj[i].Load(f);

			// �e�N�X�`���ǂݍ���
			f = texfilename[3];
			m_LightObj[i].LoadTex(f, TexType::DIFFUSE);

			// �����_���[�Ƀ��f����n��
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
// �V�[���̍X�V
void SceneDemo::SceneUpdate()
{
	// �e��I�u�W�F�N�g�X�V����
	m_Camera.Update();
	for (auto i = 0; i < COTTAGE_NUM; i++)
	{
		m_MRCottage[i].Update();
	}
	m_Screen.Update();
	m_UnityChan.Update();

	// ���C�g�̍X�V����
	UpdateLight();

	// DLSS���[�h�Ŏg�p���Ă�����
	if (g_RenderMode == RenderMode::DLSS &&
		g_AntiAliasingMode == AntiAliasingMode::DLSS)
		UpdateDLSS();	// DLSS�A�b�v�X�P�[�����O�̍X�V����


	// �T���v���[�X�e�[�g�̐ݒ�
	Renderer::SetSamplerState();
}
// �V�[���̕`��
void SceneDemo::SceneDraw()
{
	// Application�Ɋi�[����Ă���`��J�E���g�����Z�b�g
	Application::ResetDrawInfo();

	// �V���h�E�}�b�v�̃����_�����O�T�C�Y
	XMUINT2 shadowRenderSize = { SHADOW_MAP_WIDTH ,SHADOW_MAP_HEIGHT };

	//==============================================
	// �V���h�E�C���O
	//==============================================

	Renderer::SetDepthEnable(true);					// �[�x�X�e���V���X�e�[�g��ON
	Renderer::SetCullMode(CULLMODE::NONE);			// �J�����O���[�h
	Renderer::SetViewPort(shadowRenderSize);		// �V���h�E�}�b�v�p�̃r���[�|�[�g���w��
	Renderer::SetShadowMapDSV(m_ShadowMapDSV, true);// �V���h�E�}�b�v�p��DSV���Z�b�g
	Lights.DrawLight();								// ���C�g�̃o�C���h

	bool isShadowMap = true;

	m_ShaderShadowMap.SetGPU();						// �V���h�E�}�b�v�V�F�[�_�[
	// ���f���̉e�`��
	for (auto i = 0; i < COTTAGE_NUM; i++)
	{
		m_MRCottage[i].Draw(isShadowMap);			// Cottage�̕`��
	}

	// ���N���b�v���s�����f���̉e�`��
	m_ShaderShadowAlphaClip.SetGPU();				// �V���h�E�}�b�v�V�F�[�_�[�i���N���b�v�j
	m_UnityChan.Draw(isShadowMap);					// ���j�e�B�����̕`��

	for (auto i = 0; i < LAMP_NUM; i++)				// �����v�̕`��
	{
		m_Lamp[i].Draw(isShadowMap);
	}

	for (auto i = 0; i < TREE_NUM; i++)				// �؂̕`��
	{
		m_TreeA[i].Draw();
		m_TreeB[i].Draw();
	}

	//==============================================
	// �����_�����O
	//==============================================

	Param.TextureType = bufferMode;							// �\������e�N�X�`���^�C�v��ݒ�
	Renderer::SetCamera(m_Camera);							// 3D�i�������e�j�J�����Z�b�g
	m_Camera.Draw();										// �o�C���h

	// �E�B���h�E�T�C�Y�̃r���[�|�[�g���Z�b�g
	Renderer::SetViewPort();
	// DLSS���[�h�̏ꍇ�̓����_�����O�T�C�Y�Ɉˑ������r���[�|�[�g���Z�b�g
	if (g_RenderMode == RenderMode::DLSS || g_RenderMode == RenderMode::DDLSS)
	Renderer::SetViewPort(Renderer::GetInputRenderSize());	// �r���[�|�[�g���w��


	// �����_�[�^�[�Q�b�g��HDR�I�t�X�N���[���ɃZ�b�g
	SetHDRRenderTarget(true, m_WriteDepthDSV, true);
	// �f�B�t�@�[�h�����_�����O���[�h�̏ꍇ�͐�p��RTV��
	if (g_RenderMode == RenderMode::DEFERRED || g_RenderMode == RenderMode::DDLSS)
		SetDeferredGBufferRenderTarget();

	SetShadowMap();								// �V���h�E�}�b�v���o�C���h

	//----------------------------------------------
	// �֊s��
	//----------------------------------------------

	if (isOutline)
	{
		Renderer::SetCullMode(CULLMODE::FRONT);		// �J�����O���[�h���t�����g��
		m_ShaderOutline.SetGPU();					// �֊s���p�̃V�F�[�_�[

		for (auto i = 0; i < COTTAGE_NUM; i++)
		{
			m_MRCottage[i].Draw();						// Cottage�̕`��
		}

		Renderer::SetCullMode(CULLMODE::BACK);		// �J�����O���[�h���o�b�N�Ɂi2D�X�v���C�g�̓J�����O�o�b�N�Łj
		m_UnityChan.Draw();							// ���j�e�B�����̕`��

		for (auto i = 0; i < LAMP_NUM; i++)			// �����I�u�W�F�N�g�̕`��i�f�o�b�O�p�j
		{
			m_Lamp[i].Draw();						// �����v�̕`��
		}

		for (auto i = 0; i < TREE_NUM; i++)			// �؂̕`��
		{
			m_TreeA[i].Draw();
			m_TreeB[i].Draw();
		}
	}


	//----------------------------------------------
	// �t�H���[�h�����_�����O
	//----------------------------------------------
	if (g_RenderMode == RenderMode::NONE)
	{
		m_ShaderLit.SetGPU();						// ���C�e�B���O�i�V���h�E�Ȃ��j�p�̃V�F�[�_�[

		Param.UseNormalMap = Enable::DISABLE;		// �@���}�b�v���g�p
		Renderer::SetExtraParam(Param);				// �o�C���h
		m_Mountain.SetTexture();					// �e�N�X�`�����Z�b�g
		m_MRMountain.Draw();						// �R�̕`��

		m_ShaderShadow.SetGPU();					// ���C�e�B���O�i�V���h�E����j�p�̃V�F�[�_�[

		Param.UseNormalMap = Enable::ENABLE;		// �@���}�b�v�g�p
		Renderer::SetExtraParam(Param);				// �o�C���h
		m_Moss.Draw();								// �t�B�[���h�̕`��
		//m_TileA.Draw();								// �t�B�[���h�̕`��
		//m_TileB.Draw();								// �t�B�[���h�̕`��

		for (auto i = 0; i < COTTAGE_NUM; i++)
		{
			m_Cottage[i].SetTexture();				// �e�N�X�`�����Z�b�g
			m_MRCottage[i].Draw();					// Cottage�̕`��
		}

		Param.UseNormalMap = Enable::DISABLE;		// �@���}�b�v���g�p
		Renderer::SetExtraParam(Param);				// �o�C���h

		m_ShaderSkySphere.SetGPU();					// �V�F�[�_�[���Z�b�g
		m_Skysphere.SetTexture();					// �e�N�X�`�����Z�b�g
		m_MRSkysphere.Draw();						// skysphere�̕`��

		Renderer::SetAlphaBlend(true);				// �A���t�@�u�����hON
		m_ShaderUnlit.SetGPU();						// �V�F�[�_�[���Z�b�g

		for (auto i = 0; i < TREE_NUM; i++)
		{
			m_TreeA[i].Draw();						// �؂̕`��
		}

		m_TreeB[0].Draw();							// �؂̕`��
		m_TreeB[1].Draw();							// �؂̕`��

		for (auto i = 0; i < LAMP_NUM; i++)
		{
			m_Lamp[i].Draw();						// �����v�̕`��
		}


		//for (auto i = 0; i < POINTLIGHT_NUM; i++)	// �����I�u�W�F�N�g�̕`��i�f�o�b�O�p�j
		//{
		//	Point PLight = Lights.GetPointLight(i);
		//	// �_�������������ꍇ
		//	if (PLight.Enable)
		//	{
		//		m_LightObj[i].SetTexture();			// �e�N�X�`�����Z�b�g	
		//		m_MRLightObj[i].Draw();				// �`��
		//	}
		//}

		//Param.UseNormalMap = Enable::ENABLE;		// �@���}�b�v���g�p
		//Renderer::SetExtraParam(Param);				// �o�C���h
		//m_ShaderLit.SetGPU();						// ���C�e�B���O�i�V���h�E�Ȃ��j�p�̃V�F�[�_�[
		m_UnityChan.Draw();							// ���j�e�B�����̕`��

		Param.UseNormalMap = Enable::DISABLE;		// �@���}�b�v���g�p
		Renderer::SetExtraParam(Param);				// �o�C���h
		m_ShaderUnlit.SetGPU();						// �V�F�[�_�[���Z�b�g
		m_TreeB[2].Draw();							// �؂̕`��

		Renderer::SetAlphaBlend(false);				// �A���t�@�u�����hOFF

	}
	//----------------------------------------------
	// �f�B�t�@�[�h�����_�����O�i���쒆�j
	//----------------------------------------------
	if (g_RenderMode == RenderMode::DEFERRED)
	{
		//----------------------------------------------
		// �f�B�t�@�[�h�����_�����O
		//----------------------------------------------
		m_ShaderGBuffer.SetGPU();					// �V�F�[�_�[���Z�b�g�iGBuffer�j

		Param.UseNormalMap = Enable::ENABLE;		// �@���}�b�v�g�p
		Renderer::SetExtraParam(Param);				// �o�C���h
		m_Moss.Draw();								// �t�B�[���h�̕`��
		//m_TileA.Draw();								// �t�B�[���h�̕`��
		//m_TileB.Draw();								// �t�B�[���h�̕`��

		for (auto i = 0; i < COTTAGE_NUM; i++)
		{
			m_Cottage[i].SetTexture();				// �e�N�X�`�����Z�b�g
			m_MRCottage[i].Draw();					// Cottage�̕`��
		}

		Param.UseNormalMap = Enable::DISABLE;		// �@���}�b�v���g�p
		Renderer::SetExtraParam(Param);				// �o�C���h
		m_Mountain.SetTexture();					// �e�N�X�`�����Z�b�g
		m_MRMountain.Draw();						// �R�̕`��
		m_Skysphere.SetTexture();					// �e�N�X�`�����Z�b�g
		m_MRSkysphere.Draw();						// skysphere�̕`��

		Renderer::SetAlphaBlend(true);				// �A���t�@�u�����hON

		for (auto i = 0; i < POINTLIGHT_NUM; i++)	// �����I�u�W�F�N�g�̕`��i�f�o�b�O�p�j
		{
			//Point PLight = Lights.GetPointLight(i);
			//// �_�������������ꍇ
			//if (PLight.Enable)
			//{
			//	m_LightObj[i].SetTexture();			// ���C�g�e�N�X�`�����Z�b�g	
			//	m_MRLightObj[i].Draw();				// ���C�g�I�u�W�F�N�g�̕`��
			//}

			m_Lamp[i].Draw();				// �����v�̕`��
		}

		m_UnityChan.Draw();							// ���j�e�B�����̕`��
		Renderer::SetAlphaBlend(false);				// �A���t�@�u�����hOFF


		//---------------------------------------
		// �f�B�t�@�[�h�V�F�[�f�B���O
		//---------------------------------------
		SetHDRRenderTarget(true, nullptr);			//	HDR�p�����_�[�^�[�Q�b�g���Z�b�g
		SetDeferredShaderResource();				// GBuffer�̏����e�N�X�`���Ƃ��ăo�C���h
		m_ShaderDeferredRendering.SetGPU();			// �V�F�[�_�[�̃Z�b�g�i�f�B�t�@�[�h�����_�����O�j
		m_Screen.Draw();							// �`��


		//���������I�u�W�F�N�g��قȂ郉�C�e�B���O���s�����f���̂��߁�
		//---------------------------------------
		// �t�H���[�h�����_�����O
		//---------------------------------------

		// //�t�H���[�h�����_�����O�͐[�x�o�b�t�@��L����
		//context->OMSetRenderTargets(1, &defaultRTV, m_hdrDSV);
		// //�[�x�o�b�t�@�͂��̂܂܎g�p���� (�K�v�Ȃ�N���A)
		// context->ClearDepthStencilView(m_hdrDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	}


	//----------------------------------------------
	// DLSS�A�b�v�X�P�[���i���쒆�j
	//----------------------------------------------

	// �����_�����O���[�h�ɉ����ĕ`���ς���
	//switch (g_RenderMode)
	//{
	//	//case RenderMode::DLSS:
	//	//	// DLSS�A�b�v�X�P�[��
	//	//	{
	//	//	Renderer::SetViewPort(Renderer::GetInputRenderSize());	// �r���[�|�[�g���w��
	//	//	//----------------------------------------------
	//	//	// �f�B�t�@�[�h�����_�����O
	//	//	//----------------------------------------------
	//	//	SetDLSSRenderTarget();						// DLSS�p�����_�[�^�[�Q�b�g���w��
	//	//	// GBuffer�p�V�F�[�_�[���Z�b�g
	//	//	m_ShaderGBuffer.SetGPU();
	//	//	// �I�u�W�F�N�g�̕`��
	//	//	ExtraParam.UseNormalMap = Enable::DISABLE;	// �@���}�b�v���g�p
	//	//	Renderer::SetExtraParam(ExtraParam);		// �o�C���h
	//	//	m_Skysphere.SetTexture();					// �e�N�X�`�����Z�b�g
	//	//	m_MRSkysphere.Draw();						// skysphere�̕`��
	//	//	ExtraParam.UseNormalMap = Enable::ENABLE;		// �@���}�b�v�g�p
	//	//	Renderer::SetExtraParam(ExtraParam);		// �o�C���h
	//	//	m_Moss.Draw();								// �t�B�[���h�̕`��
	//	//	m_Cottage.SetTexture();						// �e�N�X�`�����Z�b�g
	//	//	m_MRCottage.Draw();							// ���f���̕`��
	//	//	//-----------------------------------------------------------------------------
	//	//	// �I�t�X�N���[�������_�����O�iDLSS�ɓn�����͒l���擾�j
	//	//	// ���I�t�X�N���[���̃����_�[�^�[�Q�b�g��SetDLSSRenderTarget()�Ŏw�肵�Ă���
	//	//	//-----------------------------------------------------------------------------
	//	//	SetHDRRenderTarget();		// �I�t�X�N���[���p�̃����_�[�^�[�Q�b�g���Z�b�g
	//	//	SetDLSSShaderResource();		// G�o�b�t�@���o�C���h
	//	//	m_ShaderDLSSInput.SetGPU();		// �I�t�X�N���[�������_�����O�p�V�F�[�_�[���Z�b�g
	//	//	m_Screen.Draw();				// �I�t�X�N���[�������_�����O
	//	//	//------------------------------------------------
	//	//	// �����_�����O���ʂ��A�b�v�X�P�[�����O�iDLSS�j
	//	//	//------------------------------------------------
	//	//	// G�o�b�t�@���瓾��ꂽ���\�[�X���擾
	//	//	ID3D11Texture2D* PreResolveColor = m_DLSSInputTex;
	//	//	ID3D11Texture2D* ResolvedColor = m_DLSSOutputTex;
	//	//	ID3D11Texture2D* MotionVectors = m_DLSSGBufferTexs[4];
	//	//	ID3D11Texture2D* Depth = m_DLSSDepthTex;
	//	//	ID3D11Texture2D* Exposure = nullptr;
	//	//	// �A�b�v�X�P�[���̐����t���O
	//	//	bool isEvaluate = false;
	//	//	// DLSS�X�[�p�[�T���v�����O
	//	//	// �R�R��DLSS�T���v�����O���s��
	//	//	if (g_AntiAliasingMode == AntiAliasingMode::DLSS)
	//	//	{
	//	//		// DLSS�@�\������������Ă���
	//	//		// DLSS���g�p����t���O���o���Ă���ꍇ
	//	//		if (isInitDLSS && isUseDLSSFeature)
	//	//		{
	//	//			bool ResetScene = isResetResource;
	//	//			//==============================================================================
	//	//			// ���_�Q
	//	//			// DLAA�ȊO�̃A�b�v�X�P�[���̏������s���Ȃ�
	//	//			// ��DLAA���������Ă��邱�Ƃ���n���Ă��郊�\�[�X�Ɉȏ�͂Ȃ����Ƃ��킩��
	//	//			// �������_�����O�T�C�Y�������H���邢�͑���Ȃ����\�[�X������H
	//	//			//==============================================================================
	//	//			// DLSS�X�[�p�[�T���v�����O
	//	//			if (DLSSManager::GetInstance().EvaluateSuperSampling(
	//	//				PreResolveColor,		// ����
	//	//				ResolvedColor,			// �o��
	//	//				MotionVectors,			// ���[�V�����x�N�g��
	//	//				Depth,					// �[�x
	//	//				Exposure,				// �I�o
	//	//				ResetScene,				// ���Z�b�g�t���O
	//	//				true,					// �g��API�t���O
	//	//				{ 0.0f, 0.0f },			// �W�b�^�[�I�t�Z�b�g
	//	//				{ 1.0f, 1.0f }))		// ���[�V�����x�N�g���X�P�[��
	//	//			{
	//	//				std::cout << "�A�b�v�X�P�[���ɐ������܂���" << std::endl;
	//	//				isEvaluate = true;
	//	//			}
	//	//			else
	//	//			{
	//	//				// �X�[�p�[�T���v�����O�Ɏ��s�����ꍇ��
	//	//				// ���͑O�̃��\�[�X���g�p����
	//	//				std::cerr << "�A�b�v�X�P�[���Ɏ��s���܂���" << std::endl;
	//	//				std::cout << "�A�b�v�X�P�[���O�̉摜���g�p���܂�" << std::endl;
	//	//				isEvaluate = false;
	//	//			}
	//	//		}
	//	//		else
	//	//		{
	//	//			// DLSS�@�\�̏�����������Ă��Ȃ�������
	//	//			if (!isInitDLSS)
	//	//			{
	//	//				std::cerr << "DLSS�@�\������������Ă��܂���" << std::endl;
	//	//				std::cerr << "�A�b�v�X�P�[���O�̉摜���g�p���܂�" << std::endl;
	//	//				isEvaluate = false;
	//	//			}
	//	//		}
	//	//	}
	//	//	//---------------------------------
	//	//	// �A�b�v�X�P�[�����ʂ�`��
	//	//	//---------------------------------
	//	//	Renderer::SetDefaultRenderTarget();		// �����_�[�^�[�Q�b�g�����ɖ߂�
	//	//	Renderer::SetViewPort();					// �r���[�|�[�g���E�B���h�E�T�C�Y�ɖ߂�
	//	//
	//	//	// �X�[�p�[�T���v�����O�̉ۂɉ����ăo�C���h���郊�\�[�X��ύX
	//	//	if (isEvaluate) {
	//	//		SetDLSSOutputResource();				// �A�E�g�v�b�g���\�[�X
	//	//	}
	//	//	else {
	//	//		SetHDRResource();					// �I�t�X�N���[�����\�[�X
	//	//	}
	//	//	m_ShaderDLSSOutput.SetGPU();				// �o�͗p�V�F�[�_�[���Z�b�g
	//	//	m_Screen.Draw();							// ���ʂ��t���X�N���[���N�A�b�h�ɕ`��
	//	//	}
	//	//	break;
	//}


	//----------------------------------------------
	// �|�X�g�v���Z�X
	//----------------------------------------------
	ClearPingPongRenderTarget();				// �|�X�g�v���Z�X�����_�[�^�[�Q�b�g���N���A

	// �|�X�g�v���Z�X������Ȃ������ꍇ�̃P�A
	ChangePingPongResource();					// �s���|���V�F�[�_�[�؂�ւ�
	SetHDRResource();							// SRV�Z�b�g�iSRV->1�X�V�j
	m_Screen.Draw();

	// ��ʊE�[�x //
	if (isPP.isDoF)
	{
		ClearBlurRenderTarget();				// �u���[�p�����_�[�^�[�Q�b�g���N���A

		// �������̃u���[
		SetBlurXResource(m_hdrSRV, 0.7f);		// �������̃u���[���o�̏���
		m_ShaderBlur.SetGPU();					// �u���[���o�V�F�[�_�[
		m_Screen.Draw();						// �������̃u���[���o

		// �c�����̃u���[
		SetBlurYResource(m_BlurSRV[0], 0.7f);	// �������̃u���[���o�̏���
		m_Screen.Draw();						// �������̃u���[���o

		// ��ʊE�[�x�̓K�p
		SetDoFResource();						// ��ʊE�[�x�̏���
		m_ShaderDoF.SetGPU();					// ��ʊE�[�x�V�F�[�_�[
		m_Screen.Draw();						// �`��

	}

	// �u���[�� //
	if (isPP.isBloom)
	{
		SetLuminanceResource();					// �P�x���o�̏���
		m_ShaderLuminance.SetGPU();				// �P�x���o�̃V�F�[�_�[
		m_Screen.Draw();						// �`��

		ClearBlurRenderTarget();				// �u���[�p�����_�[�^�[�Q�b�g���N���A

		// �������̃u���[
		SetBlurXResource(m_LuminanceSRV, 3.0f);	// �������̃u���[���o�̏���
		m_ShaderBlur.SetGPU();					// �u���[���o�V�F�[�_�[
		m_Screen.Draw();						// �������̃u���[���o

		// �c�����̃u���[
		SetBlurYResource(m_BlurSRV[0], 3.0f);	// �������̃u���[���o�̏���
		m_Screen.Draw();						// �������̃u���[���o


		SetBloomResource();						// �u���[�����C�e�B���O�̏���
		m_ShaderBloom.SetGPU();					// �u���[���V�F�[�_�[
		m_Screen.Draw();						// �`��
	}

	// �r�l�b�g //
	if (isPP.isVignette)
	{
		ChangePingPongResource();				// �s���|���V�F�[�_�[�؂�ւ�
		m_ShaderVignette.SetGPU();				// �r�l�b�g�V�F�[�_�[
		m_Screen.Draw();						// �`��
	}


	// �ŏI�o�� //
	ChangePingPongResource();					// �s���|���V�F�[�_�[�؂�ւ�
	Renderer::SetDefaultRenderTarget();			// �o�b�N�o�b�t�@�̃����_�[�^�[�Q�b�g���Z�b�g
	m_ShaderUnlit.SetGPU();						// Unlit�V�F�[�_�[�i�G�t�F�N�g�Ȃ��j
	m_Screen.Draw();							// �`��

}
// �V�[���̏I��
void SceneDemo::SceneDispose()
{
	// ��� 
	
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

	// DLSS�[�x
	m_DLSSDepthTex->Release();
	m_DLSSDepthTex = nullptr;
	m_DLSSDepthDSV->Release();
	m_DLSSDepthDSV = nullptr;
	m_DLSSDepthSRV->Release();
	m_DLSSDepthSRV = nullptr;

	// DLSS�o��
	m_DLSSOutputTex->Release();
	m_DLSSOutputTex = nullptr;
	m_DLSSOutputUAV->Release();
	m_DLSSOutputUAV = nullptr;
	m_DLSSOutputSRV->Release();
	m_DLSSOutputSRV = nullptr;

	// �I�t�X�N���[��
	m_DLSSInputTex->Release();
	m_DLSSInputTex = nullptr;
	m_DLSSInputRTV->Release();
	m_DLSSInputRTV = nullptr;
	m_DLSSInputSRV->Release();
	m_DLSSInputSRV = nullptr;


	// �V���h�E�}�b�v
	m_ShadowMapTex->Release();
	m_ShadowMapTex = nullptr;
	m_ShadowMapDSV->Release();
	m_ShadowMapDSV = nullptr;
	m_ShadowMapSRV->Release();
	m_ShadowMapSRV = nullptr;

	// �V���h�E
	m_ShadowRTV->Release();
	m_ShadowRTV = nullptr;
	m_ShadowTex->Release();
	m_ShadowTex = nullptr;
	m_ShadowSRV->Release();
	m_ShadowSRV = nullptr;
	

	// HDR�i�|�X�g�v���Z�X�j
	m_hdrTex->Release();
	m_hdrTex = nullptr;
	m_hdrRTV->Release();
	m_hdrRTV = nullptr;
	m_hdrSRV->Release();
	m_hdrSRV = nullptr;


	// �|�X�g�v���Z�X
	for (int i = 0; i < PINGPONG_NUM; ++i)
	{
		m_PingPongTex[i]->Release();
		m_PingPongTex[i] = nullptr;
		m_PingPongRTV[i]->Release();
		m_PingPongRTV[i] = nullptr;
		m_PingPongSRV[i]->Release();
		m_PingPongSRV[i] = nullptr;
	}

	// �[�x�������݁iDoF�j
	m_WriteDepthTex->Release();
	m_WriteDepthTex = nullptr;
	m_WriteDepthDSV->Release();
	m_WriteDepthDSV = nullptr;
	m_WriteDepthSRV->Release();
	m_WriteDepthSRV = nullptr;

	// �P�x���o
	m_LuminanceTex->Release();
	m_LuminanceTex = nullptr;
	m_LuminanceRTV->Release();
	m_LuminanceRTV = nullptr;
	m_LuminanceSRV->Release();
	m_LuminanceSRV = nullptr;

	// �u���[���o
	for (int i = 0; i < BLUR_NUM; ++i)
	{
		m_BlurTex[i]->Release();
		m_BlurTex[i] = nullptr;
		m_BlurRTV[i]->Release();
		m_BlurRTV[i] = nullptr;
		m_BlurSRV[i]->Release();
		m_BlurSRV[i] = nullptr;
	}

	// ���C�g
	Lights.DisposeLight();
}


//-------------------------------------
// Update���\�b�h
//-------------------------------------
// DLSS�̍X�V����
void SceneDemo::UpdateDLSS()
{
	//-------------------------------
	// �����_�����O�T�C�Y�֘A
	//-------------------------------

	// �����_�����O�T�C�Y�i�A�b�v�X�P�[���O�j
	XMUINT2 inputRenderTargetSize = Renderer::GetInputRenderSize();
	// �ŏI�o�̓����_�����O�T�C�Y�i�A�b�v�X�P�[����j
	XMUINT2 outputRenderTargetSize = Renderer::GetOutputRenderSize();
	// ���O�i�O�t���[���j�̐��������_�����O�T�C�Y
	static XMUINT2 inputLastSize = { 0, 0 };	// ����
	static XMUINT2 outputLastSize = { 0, 0 };	// �o�� 
	// �T���v���[�X�e�[�g�ɓn��Lod�o�C�A�X
	float lodBias = 0.f;
	// DLSS�쐬���ɐݒ肵�������_�����O�T�C�Y
	XMUINT2 dlssCreationTimeRenderSize = inputRenderTargetSize;

	//-------------------------------
	// �N�I���e�B���[�h�̐ݒ�
	//-------------------------------

	// ���݂̃N�I���e�B���[�h�iUI����ݒ�\�ɂ��Ă���j
	PrevQuality = PerfQualityMode;
	PerfQualityMode = PERF_QUALITY_LIST[perfQualityIndex].PerfQuality;

	//------------------------------------------------
	// DLSS�̐����T�C�Y�ݒ�i�����_�����O�T�C�Y�j�̃N�G��
	// �ŏI�����_�����O�T�C�Y�ioutput�j����
	// ���������_�����O�T�C�Y�iinput�j�����߂�
	//------------------------------------------------

	// �A���`�G�C���A�V���O���[�h��DLSS�̏ꍇ
	if (g_AntiAliasingMode == AntiAliasingMode::DLSS)
	{
		// �ŏI�o�̓����_�����O�T�C�Y���O�t���[������X�V����Ă��Ȃ��ꍇ��
		// �œK�����_�����O�T�C�Y�̎擾�N�G���͏ȗ��o����
		if (outputLastSize.x != outputRenderTargetSize.x ||
			outputLastSize.y != outputRenderTargetSize.y)
		{
			// �e�p�t�H�[�}���X���[�h�ɂ��čœK�ݒ�̃N�G��
			for (PERF_QUALITY_ITEM& item : PERF_QUALITY_LIST)
			{
				// ���������_�����O�T�C�Y�̃N�G��
				DLSSManager::GetInstance().QueryOptimalSettings(
					outputRenderTargetSize,
					item.PerfQuality,
					&g_RecommendedSettingsMap[item.PerfQuality]);

				// �����ݒ肪�L�����m�F
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

				// �����T�C�Y�̃f�o�b�O�\��
				std::cout << "Recommended Optimal Render Size: ("
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxRecommendedOptimalRenderSize.x << ", "
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxRecommendedOptimalRenderSize.y << ")" << std::endl;
				std::cout << "Dynamic Maximum Render Size: ("
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMaximumRenderSize.x << ", "
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMaximumRenderSize.y << ")" << std::endl;
				std::cout << "Dynamic Minimum Render Size: ("
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMinimumRenderSize.x << ", "
					<< g_RecommendedSettingsMap[item.PerfQuality].m_ngxDynamicMinimumRenderSize.y << ")" << std::endl;

				// ���I�ݒ肪�\���m�F
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

			// �O�t���[���̃����_�����O�T�C�Y��
			// ���݂�DLSS�ŏI�����_�����O�T�C�Y��ۑ����Ă���
			outputLastSize = outputRenderTargetSize;

			std::cout << "DLSS�œK�ݒ芮��" << endl;
		}
	}


	//--------------------------------------------------------------------
	// �����_�����O�T�C�Y���O�t���[���Ɣ�r���ĕύX����Ă����ꍇ�̏���
	// �i��Ƀp�t�H�[�}���X���[�h�Ɉˑ����ăT�C�Y���ς��j
	//--------------------------------------------------------------------

	// �e�N�X�`����LOD�iLevel of Detail�j��X�������̃T�C�Y��������
	float texLodXDimension = { 0 };

	// ��������Ă���œK�ȃ����_�����O�T�C�Y��DLSS���������̃����_�����O�T�C�Y�ɐݒ�
	dlssCreationTimeRenderSize = g_RecommendedSettingsMap[PerfQualityMode].m_ngxRecommendedOptimalRenderSize;

	// DLSS�쐬���̃����_�����O�T�C�Y���A�b�v�X�P�[���O�̃����_�����O�T�C�Y�ɐݒ�
	inputRenderTargetSize = dlssCreationTimeRenderSize;

	// Lod�o�C�A�X�������_�����O�T�C�Y����v�Z
	texLodXDimension = inputRenderTargetSize.x;
	float ratio = (float)inputRenderTargetSize.x / (float)outputRenderTargetSize.x;
	lodBias = (std::log2f(ratio)) - 1.0f;

	// ���߂�Lod�o�C�A�X�ŃT���v���[���Z�b�g
	Renderer::SetSamplerState(lodBias);

	//-------------------------------
	// DLSS�̏�����
	//-------------------------------
	// �����_�����O�T�C�Y���O�t���[���ƈقȂ�ꍇ
	if (inputRenderTargetSize.x != inputLastSize.x ||
		inputRenderTargetSize.y != inputLastSize.y)
	{

		std::cout << "Lod Bias: " << lodBias << std::endl;

		// HDR��[�x���]�̐ݒ�i�K�v�ɉ����ĕύX�j
		bool isHDR = false;
		bool depthInverted = false;

		// DLSS�I�v�V�����i��: Quality�ݒ�j
		NVSDK_NGX_PerfQuality_Value qualitySetting = PerfQualityMode;

		// �����_�[�v���Z�b�g�i0�̓f�t�H���g�j
		unsigned int renderPreset = 0;

		// �����_�����O�T�C�Y�ɉ����ă��\�[�X���Đ���
		Renderer::SetInputRenderSize(inputRenderTargetSize);
		CreateDLSSResource(inputRenderTargetSize);


		// NVIDIA GPU���g�p�\�ȏꍇ
		if (Renderer::GetIsAbleNVIDIA())
		{
			isInitDLSS = true;

			// DLSS������
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
				std::cerr << "DLSS�̏������Ɏ��s���܂���" << std::endl;
				isInitDLSS = false;
			}

		}
		else
		{
			std::cerr << "NVIDIA_GPU���g�p���Ă��܂���" << std::endl;
		}

	}

	// �O�t���[���Ɍ��݂̃����_�����O�T�C�Y��ۑ�
	inputLastSize = inputRenderTargetSize;
}
// ���C�g�̍X�V����
void SceneDemo::UpdateLight()
{

	Directional DLight = Lights.GetDirectionalLight();

	//-------------------------------------
	// ���s����
	//-------------------------------------
	// ���C�g�p�����[�^�ɂ���ĕύX
	switch (LightType)
	{
	case DLIGHTTYPE::MORNING:	// ��
		DLight.Diffuse = { 0.8f, 0.7f, 0.9f, 1.0f };
		DLight.Ambient = { 0.5f, 0.5f, 0.8f, 1.0f };
		break;
	case DLIGHTTYPE::NOON:		// ��
		DLight.Diffuse = { 1.0f, 1.0f, 0.95f, 1.0f };
		DLight.Ambient = { 0.8f, 0.8f, 0.8f, 1.0f };
		break;
	case DLIGHTTYPE::EVENING:	// �[
		DLight.Diffuse = { 1.0f, 0.5f, 0.3f, 1.0f };
		DLight.Ambient = { 0.6f, 0.4f, 0.3f, 1.0f };
		break;
	case DLIGHTTYPE::NIGHT:		// ��
		DLight.Diffuse = { 0.3f, 0.3f, 0.5f, 1.0f };
		DLight.Ambient = { 0.3f, 0.3f, 0.4f, 1.0f };
		break;
	default:
		break;
	}

	Lights.SetDirectionalLight(DLight);

	//-------------------------------------
	// �_����
	//-------------------------------------
	for (auto i = 0; i < POINTLIGHT_NUM; i++)
	{
		Point PLight = Lights.GetPointLight(i);
		Directional DLight = Lights.GetDirectionalLight();

		// �_�����̍��W���X�V
		//Vector3 pos = PLight.Position;
		Vector3 pos = PLight.Position;
		m_MRLightObj[i].SetPosition(pos);

		Lights.SetPointLight(i, PLight);

	}


	// �V���h�E�C���O�̂��߂ɃJ�����̏����J�����ɓn��
	Lights.UpdateLightMatrix(&m_Camera);

}


//-------------------------------------
// Resource�������\�b�h
//-------------------------------------
// �����_�����O���\�[�X�̐���
void SceneDemo::CreateRenderResource()
{
	CreateDeferredResource();	// �f�B�t�@�[�h�����_�����O
	CreateDLSSResource();		// DLSS
	CreateShadowResource();		// �V���h�E
	CreateHDRResource();		// �I�t�X�N���[��
	CreatePostProcessResource();// �|�X�g�v���Z�X
}

// �f�B�t�@�[�h�����_�����O���\�[�X�̐���
void SceneDemo::CreateDeferredResource()
{
	auto device = Renderer::GetDevice();
	auto iRenderSize = Renderer::GetInputRenderSize();
	HRESULT hr = S_OK;

	// �eG�o�b�t�@�̃t�H�[�}�b�g
	DXGI_FORMAT formats[GBUFFER_NUM] = {
		DXGI_FORMAT_R8G8B8A8_UNORM,		// Albedo
		DXGI_FORMAT_R16G16B16A16_FLOAT, // Normal
		DXGI_FORMAT_R32G32B32A32_FLOAT, // WorldPos
		DXGI_FORMAT_R32G32B32A32_FLOAT,	// Depth�i�_�~�[�j
		DXGI_FORMAT_R32G32_FLOAT,		// MVector
	};

	// G�o�b�t�@�̐���
	for (int i = 0; i < GBUFFER_NUM; ++i) {
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = iRenderSize.x;
		textureDesc.Height = iRenderSize.y;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.SampleDesc.Count = 1;	// �}���`�T���v���͖���
		textureDesc.Format = formats[i];
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		// �e�N�X�`���쐬
		hr = device->CreateTexture2D(&textureDesc, nullptr, &m_DeferredGBufferTexs[i]);
		if (FAILED(hr)) { cout << "G�o�b�t�@�e�N�X�`���̍쐬�Ɏ��s���܂���" << endl; }

		// RTV�쐬
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = formats[i];
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		hr = device->CreateRenderTargetView(m_DeferredGBufferTexs[i], &rtvDesc, &m_DeferredGBufferRTVs[i]);
		if (FAILED(hr)) { cout << "G�o�b�t�@RTV�̍쐬�Ɏ��s���܂���" << endl; }

		// SRV�쐬
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = formats[i];
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		hr = device->CreateShaderResourceView(m_DeferredGBufferTexs[i], &srvDesc, &m_DeferredGBufferSRVs[i]);
		if (FAILED(hr)) { cout << "G�o�b�t�@SRV�̍쐬�Ɏ��s���܂���" << endl; }

	}

}
// DLSS���\�[�X�̐����i�y�эĐ����j
void SceneDemo::CreateDLSSResource(XMUINT2 renderSize)
{
	auto device = Renderer::GetDevice();
	HRESULT hr = S_OK;

	// �������\�[�X�̉��
	// G�o�b�t�@�̉��
	for (int i = 0; i < GBUFFER_NUM; ++i) {
		if (m_DLSSGBufferRTVs[i]) m_DLSSGBufferRTVs[i]->Release();
		if (m_DLSSGBufferSRVs[i]) m_DLSSGBufferSRVs[i]->Release();
		if (m_DLSSGBufferTexs[i]) m_DLSSGBufferTexs[i]->Release();
	}
	// DLSS���̓o�b�t�@�̉��
	if (m_DLSSInputRTV) m_DLSSInputRTV->Release();
	if (m_DLSSInputSRV) m_DLSSInputSRV->Release();
	if (m_DLSSInputTex) m_DLSSInputTex->Release();
	// DLSS�o�̓o�b�t�@�̉��
	if (m_DLSSOutputUAV) m_DLSSOutputUAV->Release();
	if (m_DLSSOutputSRV) m_DLSSOutputSRV->Release();
	if (m_DLSSOutputTex) m_DLSSOutputTex->Release();
	// �[�x�o�b�t�@�̉��
	if (m_DLSSDepthDSV) m_DLSSDepthDSV->Release();
	if (m_DLSSDepthSRV) m_DLSSDepthSRV->Release();
	if (m_DLSSDepthTex) m_DLSSDepthTex->Release();


	D3D11_TEXTURE2D_DESC textureDesc = {};
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

	// �eG�o�b�t�@�̃t�H�[�}�b�g
	DXGI_FORMAT formats[GBUFFER_NUM] = {
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,// Albedo�i����`��ԁj
		DXGI_FORMAT_R16G16B16A16_FLOAT, // Normal
		DXGI_FORMAT_R32G32B32A32_FLOAT, // WorldPos
		DXGI_FORMAT_R32_FLOAT,			// Depth
		DXGI_FORMAT_R32G32_FLOAT,		// MVector
	};

	//------------------------------------------------
	// G�o�b�t�@�̐���
	//------------------------------------------------
	for (int i = 0; i < GBUFFER_NUM; ++i) {

		textureDesc.Width = renderSize.x;
		textureDesc.Height = renderSize.y;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.SampleDesc.Count = 1;	// �}���`�T���v���͖���
		textureDesc.Format = formats[i];
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		// �e�N�X�`��
		hr = device->CreateTexture2D(&textureDesc, nullptr, &m_DLSSGBufferTexs[i]);
		if (FAILED(hr)) std::cout << "GBuffer�̃e�N�X�`�������Ɏ��s���܂����F�v�f�ԍ�[" << i << "]" << endl;
		// RTV
		hr = device->CreateRenderTargetView(m_DLSSGBufferTexs[i], nullptr, &m_DLSSGBufferRTVs[i]);
		if (FAILED(hr)) std::cout << "GBuffer��RTV�����Ɏ��s���܂����F�v�f�ԍ�[" << i << "]" << endl;
		// SRV
		hr = device->CreateShaderResourceView(m_DLSSGBufferTexs[i], nullptr, &m_DLSSGBufferSRVs[i]);
		if (FAILED(hr)) std::cout << "GBuffer��SRV�����Ɏ��s���܂����F�v�f�ԍ�[" << i << "]" << endl;

	}

	//------------------------------------------------
	// �[�x�o�b�t�@�̐���
	//------------------------------------------------
	textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	textureDesc.Width = renderSize.x;
	textureDesc.Height = renderSize.y;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; // SRV���o�C���h�\��
	// �e�N�X�`��
	hr = device->CreateTexture2D(&textureDesc, nullptr, &m_DLSSDepthTex);
	if (FAILED(hr)) std::cout << "�[�x�e�N�X�`���̐����Ɏ��s���܂���" << endl;
	// DSV
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	hr = device->CreateDepthStencilView(m_DLSSDepthTex, &dsvDesc, &m_DLSSDepthDSV);
	if (FAILED(hr)) std::cout << "�[�x�p��DSV�̐����Ɏ��s���܂���" << endl;
	// SRV
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; // �V�F�[�_�[�Ő[�x�݂̂��g�p
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(m_DLSSDepthTex, &srvDesc, &m_DLSSDepthSRV);
	if (FAILED(hr)) std::cout << "�[�x�p��SRV�̐����Ɏ��s���܂���" << endl;

	//------------------------------------------------
	// ���͐�o�b�t�@�̐���
	//------------------------------------------------
	textureDesc.Width = renderSize.x;
	textureDesc.Height = renderSize.y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;	// �}���`�T���v���͖���
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	// �e�N�X�`��
	device->CreateTexture2D(&textureDesc, nullptr, &m_DLSSInputTex);
	if (FAILED(hr)) std::cout << "�I�t�X�N���[���e�N�X�`���̐����Ɏ��s���܂���" << endl;
	// RTV
	device->CreateRenderTargetView(m_DLSSInputTex, nullptr, &m_DLSSInputRTV);
	if (FAILED(hr)) std::cout << "�I�t�X�N���[���pRTV�̐����Ɏ��s���܂���" << endl;
	// SRV
	device->CreateShaderResourceView(m_DLSSInputTex, nullptr, &m_DLSSInputSRV);
	if (FAILED(hr)) std::cout << "�I�t�X�N���[���pSRV�̐����Ɏ��s���܂���" << endl;


	//------------------------------------------------
	// �o�͐�o�b�t�@�̐���
	//------------------------------------------------
	textureDesc.Width = renderSize.x;
	textureDesc.Height = renderSize.y;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT,	 // �o�̓t�H�[�}�b�g
	textureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	// �e�N�X�`��
	device->CreateTexture2D(&textureDesc, nullptr, &m_DLSSOutputTex);
	if (FAILED(hr)) std::cout << "�������ݐ�e�N�X�`���̐����Ɏ��s���܂���" << endl;
	// UAV
	uavDesc.Format = textureDesc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(m_DLSSOutputTex, &uavDesc, &m_DLSSOutputUAV);
	if (FAILED(hr)) std::cout << "�������ݗp��UAV�̐����Ɏ��s���܂���" << endl;
	// SRV
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(m_DLSSOutputTex, &srvDesc, &m_DLSSOutputSRV);
	if (FAILED(hr)) std::cout << "�������ݗp��SRV�̐����Ɏ��s���܂���" << endl;

}
// �V���h�E���\�[�X�̐���
void SceneDemo::CreateShadowResource()
{
	auto device = Renderer::GetDevice();
	HRESULT hr = S_OK;

	//-------------------------------------
	// �V���h�E�}�b�v
	//-------------------------------------
	// �e�N�X�`��
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
	if (FAILED(hr)) std::cout << "�V���h�E�}�b�v�e�N�X�`���̐����Ɏ��s���܂���" << endl;

	// DSV
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Typeless �ɑΉ�����t�H�[�}�b�g
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = device->CreateDepthStencilView(m_ShadowMapTex, &dsvDesc, &m_ShadowMapDSV);
	if (FAILED(hr)) std::cout << "�V���h�E�}�b�v�pDSV�̐����Ɏ��s���܂���" << endl;

	// SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;  // SRV�p�ɕϊ�
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(m_ShadowMapTex, &srvDesc, &m_ShadowMapSRV);
	if (FAILED(hr)) std::cout << "�V���h�E�}�b�v�p��SRV�̐����Ɏ��s���܂���" << endl;

	//-------------------------------------
	// �V���h�E�C���O���s�����߂�RTV
	//-------------------------------------
	auto iRenderSize = Renderer::GetInputRenderSize();

	// �e�N�X�`��
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
	if (FAILED(hr)) std::cout << "�V���h�E�e�N�X�`���̍쐬�Ɏ��s���܂����B" << std::endl;

	// �����_�[�^�[�Q�b�g�r���[
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	hr = device->CreateRenderTargetView(m_ShadowTex, &rtvDesc, &m_ShadowRTV);
	if (FAILED(hr)) std::cout << "�V���h�ERTV�̍쐬�Ɏ��s���܂����B" << std::endl;

	// �V�F�[�_�[���\�[�X�r���[
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	hr = device->CreateShaderResourceView(m_ShadowTex, &srvDesc, &m_ShadowSRV);
	if (FAILED(hr)) std::cout << "�V���h�ESRV�̍쐬�Ɏ��s���܂����B" << std::endl;

}
// HDR���\�[�X�̍쐬�i��{���\�[�X�j
void SceneDemo::CreateHDRResource()
{
	auto device = Renderer::GetDevice();
	auto iRenderSize = Renderer::GetInputRenderSize();
	HRESULT hr = S_OK;

	//-------------------------------------
	// HDR���\�[�X
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

	// �e�N�X�`���̍쐬
	hr = device->CreateTexture2D(&hdrDesc, nullptr, &m_hdrTex);
	if (FAILED(hr)) std::cout << "HDR �e�N�X�`���̍쐬�Ɏ��s���܂����B" << std::endl;

	// HDR�p��RTV�쐬
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	hr = device->CreateRenderTargetView(m_hdrTex, &rtvDesc, &m_hdrRTV);
	if (FAILED(hr)) std::cout << "HDR RTV�̍쐬�Ɏ��s���܂����B" << std::endl;

	// HDR�p��SRV�쐬
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	hr = device->CreateShaderResourceView(m_hdrTex, &srvDesc, &m_hdrSRV);
	if (FAILED(hr)) std::cout << "HDR SRV �̍쐬�Ɏ��s���܂����B" << std::endl;


	//-------------------------------------
	// �[�x�������ݏ��
	//-------------------------------------
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = iRenderSize.x;
	depthDesc.Height = iRenderSize.y;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; // typeless��DSV/SRV���Ή�
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;
	// �e�N�X�`��
	hr = device->CreateTexture2D(&depthDesc, nullptr, &m_WriteDepthTex);
	if (FAILED(hr)) { cout << "�[�x�������݃e�N�X�`���̍쐬�Ɏ��s���܂���" << endl; }

	// DSV
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = device->CreateDepthStencilView(m_WriteDepthTex, &dsvDesc, &m_WriteDepthDSV);
	if (FAILED(hr)) { cout << "�[�x��������DSV�̍쐬�Ɏ��s���܂���" << endl; }

	// SRV
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(m_WriteDepthTex, &srvDesc, &m_WriteDepthSRV);
	if (FAILED(hr)) { cout << "�[�x��������SRV�̍쐬�Ɏ��s���܂���" << endl; }


}
// �|�X�g�v���Z�X���\�[�X�̍쐬
void SceneDemo::CreatePostProcessResource()
{
	auto device = Renderer::GetDevice();
	auto iRenderSize = Renderer::GetInputRenderSize();
	HRESULT hr = S_OK;

	// �e�N�X�`�����
	D3D11_TEXTURE2D_DESC hdrDesc = {};
	hdrDesc.Width = iRenderSize.x;
	hdrDesc.Height = iRenderSize.y;
	hdrDesc.MipLevels = 1;
	hdrDesc.ArraySize = 1;
	hdrDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	hdrDesc.SampleDesc.Count = 1;
	hdrDesc.Usage = D3D11_USAGE_DEFAULT;
	hdrDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	// RTV���
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	// SRV���
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
		if (FAILED(hr)) std::cout << "�|�X�g�v���Z�X�e�N�X�`���̍쐬�Ɏ��s���܂����B" << std::endl;

		hr = device->CreateRenderTargetView(m_PingPongTex[i], &rtvDesc, &m_PingPongRTV[i]);
		if (FAILED(hr)) std::cout << "�|�X�g�v���Z�XRTV�̍쐬�Ɏ��s���܂����B" << std::endl;

		hr = device->CreateShaderResourceView(m_PingPongTex[i], &srvDesc, &m_PingPongSRV[i]);
		if (FAILED(hr)) std::cout << "�|�X�g�v���Z�XSRV�̍쐬�Ɏ��s���܂����B" << std::endl;
	}

	//-------------------------------------
	// �u���[
	//-------------------------------------

	//// �u���[�����_�����O�T�C�Y
	//XMUINT2 blurSize[BLUR_NUM] = {
	//	{iRenderSize.x / 2,iRenderSize.y},		// �������u���[�iHorizontal�j
	//	{iRenderSize.x / 2,iRenderSize.y / 2}	// �c�����u���[�iVertical�j
	//};

	// �u���[�p�e�N�X�`��
	D3D11_TEXTURE2D_DESC blurDesc = {};
	blurDesc.Width = iRenderSize.x;
	blurDesc.Height = iRenderSize.y;
	blurDesc.MipLevels = 1;
	blurDesc.ArraySize = 1;
	blurDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;  // HDR�t�H�[�}�b�g
	blurDesc.SampleDesc.Count = 1;
	blurDesc.Usage = D3D11_USAGE_DEFAULT;
	blurDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	// �u���[�pRTV
	D3D11_RENDER_TARGET_VIEW_DESC blurRTVDesc = {};
	blurRTVDesc.Format = blurDesc.Format;
	blurRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	// �u���[�pSRV
	D3D11_SHADER_RESOURCE_VIEW_DESC blurSRVDesc = {};
	blurSRVDesc.Format = blurDesc.Format;
	blurSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	blurSRVDesc.Texture2D.MipLevels = 1;


	for (int i = 0; i < BLUR_NUM; ++i)
	{
		//// �u���[�����_�����O�T�C�Y���Z�b�g
		//blurDesc.Width = blurSize[i].x;
		//blurDesc.Height = blurSize[i].y;

		hr = device->CreateTexture2D(&blurDesc, nullptr, &m_BlurTex[i]);
		if (FAILED(hr)) {
			std::cout << "�u���[�p�e�N�X�`���̍쐬�Ɏ��s���܂����B" << std::endl;
		}

		hr = device->CreateRenderTargetView(m_BlurTex[i], &blurRTVDesc, &m_BlurRTV[i]);
		if (FAILED(hr)) {
			std::cout << "�u���[�pRTV�̍쐬�Ɏ��s���܂����B" << std::endl;
		}

		hr = device->CreateShaderResourceView(m_BlurTex[i], &blurSRVDesc, &m_BlurSRV[i]);
		if (FAILED(hr)) {
			std::cout << "�u���[�pSRV�̍쐬�Ɏ��s���܂����B" << std::endl;
		}
	}

	//-------------------------------------
	// �P�x�i�u���[���\���p�j
	//-------------------------------------

	hr = device->CreateTexture2D(&hdrDesc, nullptr, &m_LuminanceTex);
	if (FAILED(hr)) std::cout << "�P�x�������݃e�N�X�`���̍쐬�Ɏ��s���܂����B" << std::endl;

	hr = device->CreateRenderTargetView(m_LuminanceTex, &rtvDesc, &m_LuminanceRTV);
	if (FAILED(hr)) std::cout << "�P�x��������RTV�̍쐬�Ɏ��s���܂����B" << std::endl;

	hr = device->CreateShaderResourceView(m_LuminanceTex, &srvDesc, &m_LuminanceSRV);
	if (FAILED(hr)) std::cout << "�P�x��������SRV�̍쐬�Ɏ��s���܂����B" << std::endl;


	//-------------------------------------
	// �[�x�������݁iDoF�j
	//-------------------------------------
	// �[�x�o�b�t�@�쐬
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = iRenderSize.x;
	depthDesc.Height = iRenderSize.y;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; // �[�x + �X�e���V��
	depthDesc.SampleDesc.Count = 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	// �e�N�X�`��
	hr = device->CreateTexture2D(&depthDesc, nullptr, &m_WriteDepthTex);
	if (FAILED(hr)) { cout << "�[�x�������݃e�N�X�`���̍쐬�Ɏ��s���܂���" << endl; }
	// DSV
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	hr = device->CreateDepthStencilView(m_WriteDepthTex, &dsvDesc, &m_WriteDepthDSV);
	if (FAILED(hr)) { cout << "�[�x��������DSV�̍쐬�Ɏ��s���܂���" << endl; }
	// SRV
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(m_WriteDepthTex, &srvDesc, &m_WriteDepthSRV);
	if (FAILED(hr)) { cout << "�[�x��������SRV�̍쐬�Ɏ��s���܂���" << endl; }

}


//-------------------------------------
// Set���\�b�h
//-------------------------------------
// �����_�[�^�[�Q�b�g���Z�b�g�i�f�B�t�@�[�h�����_�����O�j
void SceneDemo::SetDeferredGBufferRenderTarget()
{
	auto context = Renderer::GetDeviceContext();

	// G�o�b�t�@�������_�[�^�[�Q�b�g�Ƃ��Đݒ�
	ID3D11RenderTargetView* rtv[] = {
		m_DeferredGBufferRTVs[0], // Albedo
		m_DeferredGBufferRTVs[1], // Normal
		m_DeferredGBufferRTVs[2], // WorldPos
		m_DeferredGBufferRTVs[3], // Depth
		m_DeferredGBufferRTVs[4], // MVector
	};
	
	context->OMSetRenderTargets(GBUFFER_NUM, rtv, m_WriteDepthDSV);

	// G�o�b�t�@���N���A
	float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	for (int i = 0; i < GBUFFER_NUM; ++i) {	
		context->ClearRenderTargetView(m_DeferredGBufferRTVs[i], clearColor);
	}
	context->ClearDepthStencilView(m_WriteDepthDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

}
// �f�B�t�@�[�h�����_�����O���\�[�X���Z�b�g
void SceneDemo::SetDeferredShaderResource()
{
	auto context = Renderer::GetDeviceContext();

	// SRV���s�N�Z���V�F�[�_�[�Ƀo�C���h
	context->PSSetShaderResources(0, GBUFFER_NUM, m_DeferredGBufferSRVs);

	context->PSSetShaderResources(6, 1, &m_WriteDepthSRV);	// �[�x���

}

// DLSS�p�̃����_�[�^�[�Q�b�g���w��
void SceneDemo::SetDLSSRenderTarget()
{
	auto context = Renderer::GetDeviceContext();

	// G�o�b�t�@�������_�[�^�[�Q�b�g�Ƃ��Đݒ�
	ID3D11RenderTargetView* rtv[] = {
		m_DLSSGBufferRTVs[0],	// Albedo
		m_DLSSGBufferRTVs[1],	// Normal
		m_DLSSGBufferRTVs[2],	// WorldPos
		m_DLSSGBufferRTVs[3],	// Depth�i�_�~�[�j
		m_DLSSGBufferRTVs[4],	// MVector
	};;

	// �r���[�̐�
	unsigned int viewNum = GBUFFER_NUM;

	context->OMSetRenderTargets(viewNum, rtv, m_DLSSDepthDSV);

	// G�o�b�t�@���N���A
	float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	for (int i = 0; i < viewNum; ++i) {
		context->ClearRenderTargetView(rtv[i], clearColor);
	}
	context->ClearDepthStencilView(m_DLSSDepthDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}
// DLSS�p�̃V�F�[�_�[���\�[�X���w��
void SceneDemo::SetDLSSShaderResource()
{
	auto context = Renderer::GetDeviceContext();

	// SRV���s�N�Z���V�F�[�_�[�Ƀo�C���h
	context->PSSetShaderResources(0, GBUFFER_NUM, m_DLSSGBufferSRVs);
	// DSV�ŏ������񂾐[�x�l���o�C���h
	context->PSSetShaderResources(3, 1, &m_DLSSDepthSRV);
}
// �A�b�v�X�P�[����̃��\�[�X���o�C���h
void SceneDemo::SetDLSSOutputResource()
{
	auto context = Renderer::GetDeviceContext();

	// SRV���s�N�Z���V�F�[�_�[�Ƀo�C���h
	context->PSSetShaderResources(0, 1, &m_DLSSOutputSRV);
}

// �V���h�E�}�b�v�̃Z�b�g
void SceneDemo::SetShadowMap()
{
	auto context = Renderer::GetDeviceContext();

	// �o�C���h
	context->PSSetShaderResources(5, 1, &m_ShadowMapSRV);
}
// �|�X�g�v���Z�X�p���\�[�X���Z�b�g
void SceneDemo::ChangePingPongResource()
{
	auto context = Renderer::GetDeviceContext();

	// RTV��ݒ�i�`���j
	context->OMSetRenderTargets(1, &m_PingPongRTV[currentIndex], nullptr);

	// �C���f�b�N�X��؂�ւ��i0 <-> 1�j
	currentIndex = 1 - currentIndex;

	// �O��̕`�挋�ʂ��o�C���h
	context->PSSetShaderResources(0, 1, &m_PingPongSRV[currentIndex]);


}
// �|�X�g�v���Z�X�p�����_�[�^�[�Q�b�g���N���A
void SceneDemo::ClearPingPongRenderTarget()
{
	auto context = Renderer::GetDeviceContext();

	// �F�ŃN���A
	float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };

	for (int i = 0; i < PINGPONG_NUM; ++i) {
		// RTV���N���A
		context->ClearRenderTargetView(m_PingPongRTV[i], clearColor);
	}

	// �Q�Ƃ���v�f�ԍ������Z�b�g
	currentIndex = 0;
}

// �u���[�o�͗p�����_�[�^�[�Q�b�g���N���A
void SceneDemo::ClearBlurRenderTarget()
{
	auto context = Renderer::GetDeviceContext();

	// �F�ŃN���A
	float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };

	for (int i = 0; i < BLUR_NUM; ++i) {
		// RTV���N���A
		context->ClearRenderTargetView(m_BlurRTV[i], clearColor);
	}


}
// �������̃u���[���o�̏���
void SceneDemo::SetBlurXResource(ID3D11ShaderResourceView* _srv, float _blurX)
{
	auto context = Renderer::GetDeviceContext();
	auto iRenderSize = Renderer::GetInputRenderSize();

	// RTV��ݒ�
	context->OMSetRenderTargets(1, &m_BlurRTV[0], nullptr);

	// �O��̕`�挋�ʂ��o�C���h
	context->PSSetShaderResources(0, 1, &_srv);

	// �������̃u���[
	Param.blurParam[0] = (float)iRenderSize.x * 0.5f;	// �u���[���s���e�N�X�`���̉���
	Param.blurParam[1] = (float)iRenderSize.y;			// �u���[���s���e�N�X�`���̏c��
	Param.blurParam[2] = _blurX;						// x�����̃u���[
	Param.blurParam[3] = 0.0f;							// y�����̃u���[
	Renderer::SetExtraParam(Param);						// �o�C���h

}
// �c�����̃u���[���o�̏���
void SceneDemo::SetBlurYResource(ID3D11ShaderResourceView* _srv, float _blurY)
{
	auto context = Renderer::GetDeviceContext();

	// RTV��ݒ�
	context->OMSetRenderTargets(1, &m_BlurRTV[1], nullptr);

	// �������̃u���[���ʂ��o�C���h
	context->PSSetShaderResources(0, 1, &_srv);

	// �c�����̃u���[
	Param.blurParam[1] *= 0.5f;		// �u���[���s���e�N�X�`���̏c��
	Param.blurParam[2]  = 0.0f;		// x�����̃u���[
	Param.blurParam[3]  = _blurY;	// y�����̃u���[
	Renderer::SetExtraParam(Param);	// �o�C���h

}
// ��ʊE�[�x�̏���
void SceneDemo::SetDoFResource()
{
	auto context = Renderer::GetDeviceContext();

	// RTV��ݒ�
	context->OMSetRenderTargets(1, &m_PingPongRTV[currentIndex], nullptr);

	// �C���f�b�N�X��؂�ւ��i0 <-> 1�j
	currentIndex = 1 - currentIndex;


	// ��ʊE�[�x�ɕK�v�ȃ��\�[�X��S�ăo�C���h
	context->PSSetShaderResources(0, 1, &m_hdrSRV);			// �`�挋��
	context->PSSetShaderResources(6, 1, &m_WriteDepthSRV);	// �[�x���
	context->PSSetShaderResources(7, 1, &m_BlurSRV[1]);		// �u���[����
}
// �P�x���o�̏���
void SceneDemo::SetLuminanceResource()
{
	auto context = Renderer::GetDeviceContext();

	// RTV��ݒ�
	context->OMSetRenderTargets(1, &m_LuminanceRTV, nullptr);

	// ���F�ŃN���A
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	// RTV���N���A
	context->ClearRenderTargetView(m_LuminanceRTV, clearColor);


	// �C���f�b�N�X��؂�ւ��i0 <-> 1�j
	currentIndex = 1 - currentIndex;

	// �`�挋�ʂ��Z�b�g
	context->PSSetShaderResources(0, 1, &m_PingPongSRV[currentIndex]);

	// �C���f�b�N�X��؂�ւ��i0 <-> 1�j
	currentIndex = 1 - currentIndex;
}
// �u���[�����C�e�B���O�̏���
void SceneDemo::SetBloomResource()
{
	auto context = Renderer::GetDeviceContext();

	// RTV��ݒ�
	context->OMSetRenderTargets(1, &m_PingPongRTV[currentIndex], nullptr);

	// �C���f�b�N�X��؂�ւ��i0 <-> 1�j
	currentIndex = 1 - currentIndex;

	// �`�挋�ʂƋP�x���o���ʂ��Z�b�g
	context->PSSetShaderResources(0, 1, &m_PingPongSRV[currentIndex]);
	context->PSSetShaderResources(8, 1, &m_BlurSRV[1]);

}


// �V���h�E�C���O��̃V�F�[�_�[���\�[�X���o�C���h
void SceneDemo::SetShadowResource()
{
	auto context = Renderer::GetDeviceContext();

	// SRV���s�N�Z���V�F�[�_�[�Ƀo�C���h
	context->PSSetShaderResources(0, 1, &m_ShadowSRV);

}
// �I�t�X�N���[����̃����_�[�^�[�Q�b�g���w��
void SceneDemo::SetHDRRenderTarget(bool _isClearRTV,ID3D11DepthStencilView* _dsv,bool _isClearDSV)
{
	auto context = Renderer::GetDeviceContext();


	// �I�t�X�N���[�������_�[�^�[�Q�b�g���Z�b�g
	context->OMSetRenderTargets(1, &m_hdrRTV, _dsv);


	// �����_�[�^�[�Q�b�g�̃N���A
	if (_isClearRTV)
	{
		// �F�ŃN���A
		float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
		// RTV���N���A
		context->ClearRenderTargetView(m_hdrRTV, clearColor);
	}
	// DSV�̃N���A
	if (_isClearDSV)
	{
		context->ClearDepthStencilView(_dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
}
// HDR�o�͂̃V�F�[�_�[���\�[�X���o�C���h
void SceneDemo::SetHDRResource()
{
	auto context = Renderer::GetDeviceContext();

	// SRV���s�N�Z���V�F�[�_�[�Ƀo�C���h
	context->PSSetShaderResources(0, 1, &m_hdrSRV);			// �`�挋��

	context->PSSetShaderResources(6, 1, &m_WriteDepthSRV);	// �[�x���

}


