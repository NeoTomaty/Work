// �萔
static const float NightBright = 0.1f;             // ��̖��邳
static const float PLightNum = 5;      // �_�����̐�

// ���[���h�ϊ��s��
cbuffer WorldBuffer : register(b0)
{
    matrix World;
    matrix InvWorld;    // �t�s��
    matrix PrevWorld;   // �O�t���[���̍s��
}

// �r���[�ϊ��s��
cbuffer ViewBuffer : register(b1)
{
    matrix View;
    matrix InvView;     // �t�s��
    matrix PrevView;    // �O�t���[���̍s��
}

// �v���W�F�N�V�����ϊ��s��
cbuffer ProjectionBuffer : register(b2)
{
    matrix Proj;
    matrix InvProj;     // �t�s��
    matrix PrevProj;    // �O�t���[���̍s��
}

// �}�e���A�����
struct MATERIAL
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emission;
    float Shininess;
    float Alpha;
    float2 Dummy;
};

// �}�e���A���萔
cbuffer MaterialBuffer : register(b3)
{
    MATERIAL Material;
}

// �J�������
struct CAMERA
{
    float3 eyePos;  // �J�����̈ʒu
    float padding;  // �p�f�B���O
    
    float nearClip; // �j�A�N���b�v
    float farClip;  // �t�@�[�N���b�v
    float2 padding4;// �p�f�B���O
};

// �J�����萔
cbuffer CameraBuffer : register(b4)
{
    CAMERA Camera;
};


// ���̑��p�����[�^
cbuffer Param : register(b5)
{
    int UseNormalMap;   // �@���}�b�v���g�p���邩�ǂ���
    int TextureType;    // �f�B�t�@�[�h�����_�����O�Ŏg�p����e�N�X�`���^�C�v
    int isShadow;       // �V���h�E��ON/OFF
    int padding2;       // �p�f�B���O�p
    
    float2 texSize;     // �u���[�e�N�X�`���T�C�Y
    float2 blurDir;     // �u���[����
    
    float cameraFocus;  // �J�����̏œ_����
    float DoFRange;     // ��ʊE�[�x�͈�
    
    float vignetteStrength; // �r�l�b�g�̋���
    float vignetteRadius;   // �r�l�b�g�̊J�n���a
    float2 vignetteCenter;  // �r�l�b�g�̒��S
    
    float bloomStrength;    // �u���[�������̋���
    float padding3;         // �p�f�B���O
}

// ���s����
struct DirectionalLight
{
    int Enable;         // ���C�g�̉�
    float3 padding0;    // �p�f�B���O

    float3 Position;    // ���C�g�̈ʒu
    float padding2;     // �p�f�B���O
    
    float4 Direction;   // ���C�g�̕���

    float4 Diffuse;     // �g�U���ˌ�
    float4 Ambient;     // ����
    
};

// �_����
struct PointLight
{
    int Enable;         // ���C�g�̉�
    float3 padding1;    // �p�f�B���O

    float3 Position;    // ���C�g�̈ʒu
    float Intensity;    // ���邳

    float4 Diffuse;     // �g�U���ˌ�
    float4 Ambient;     // ����

    float3 Attenuation; // �����W��
    float padding2;     // �p�f�B���O
};

// ���C�g�萔
cbuffer LightBuffer : register(b6)
{
    DirectionalLight DLight;        // ���s����
    PointLight PLight[PLightNum];   // �_����
    
    matrix LightView;               // ���C�g�̃r���[
    matrix LightProj;               // ���C�g�̃v���W�F�N�V����
};


// �f�B�t�@�[�h�����_�����O
Texture2D g_Texture : register(t0);     // �A���x�h
Texture2D g_Normal : register(t1);      // �@��
Texture2D g_World : register(t2);       // ���[���h���W
Texture2D g_Depth : register(t3);       // �[�x
Texture2D g_MVector : register(t4);     // ���[�V�����x�N�g��

// �V���h�E�}�b�v
Texture2D g_ShadowMap : register(t5);           // �V���h�E�}�b�v

// �[�x���
Texture2D<float> g_DepthMap : register(t6);     // �[�x�������݂Ŏ擾�����[�x���

// �u���[
Texture2D g_BlurTex : register(t7);             // �u���[�e�N�X�`��

// �u���[���i�P�x�j
Texture2D g_LuminanceTex : register(t8);        // �u���[���e�N�X�`��


SamplerState g_SamplerState : register(s0);  // �e�N�X�`���T���v���[
SamplerState g_SamplerShadow : register(s1); // �V���h�E�}�b�v�T���v���[

// ���_����
struct VS_IN
{
    float4 pos : POSITION0;
    float4 normal : NORMAL0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float3 tangent : TANGENT0;
};

// �s�N�Z������
struct PS_IN
{
    float4 pos : SV_POSITION0;          // �v���W�F�N�V�����ϊ�����W
    float4 prevPos : POSITION0;         // �O�t���[���̃v���W�F�N�V�����ϊ�����W
    float4 worldPos : POSITION1;        // ���[���h�ϊ�����W
    float4 prevWorldPos : POSITION2;    // �O�t���[���̃��[���h�ϊ�����W
    float4 viewPos : POSITION3;         // �r���[�ϊ�����W
    float4 prevViewPos : POSITION4;     // �O�t���[���̃r���[�ϊ�����W

    float4 lightPos : POSITION5;        // ���C�g�̍��W
    
    float3 normal : NORMAL0;            // �@��
    float4 color : COLOR0;              // �J���[
    float2 uv : TEXCOORD0;              // UV
    float3 tangent : TANGENT0;          // �ڐ�
    float3 bitangent : BITANGENT0;      // �]�@��
};

// �V���h�E�}�b�v�p
struct PS_IN_SHADOWMAP
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD0;
};


// �ŏI�o�́i�[�x�����i�[�j
struct PS_OUT
{
    float4 BeseColor    : SV_TARGET0;  // ��1�̃����_�[�^�[�Q�b�g�iRTV 0�j�ւ̏o��
};


// �V���h�E�}�b�v�pOutput
struct PS_OUT_SHADOWMAP
{
    float4 position : SV_TARGET0;
};


// G�o�b�t�@�p�o�͌���
struct PS_OUT_GBUFFER
{
    float4 Albedo : SV_TARGET0;     // ��1�̃����_�[�^�[�Q�b�g�iRTV 0�j�ւ̏o��
    float4 Normal : SV_TARGET1;     // ��2�̃����_�[�^�[�Q�b�g�iRTV 1�j�ւ̏o��
    float4 World : SV_TARGET2;      // ��3�̃����_�[�^�[�Q�b�g�iRTV 2�j�ւ̏o��
    float4 Depth : SV_TARGET3;      // ��4�̃����_�[�^�[�Q�b�g�iRTV 3�j�ւ̏o��
    float4 MVector : SV_TARGET4;    // ��5�̃����_�[�^�[�Q�b�g�iRTV 4�j�ւ̏o��
};


