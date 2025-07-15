#include "Common.hlsli"

static const float PI = 3.1415926f;


float4 main(PS_IN input) : SV_TARGET
{

    float4 color = (0, 0, 0, 1);
    
    // �e�N�X�`������A���x�h(�F)�����T���v�����O
    color = g_Texture.Sample(g_SamplerState, input.uv);

	// �e�N�X�`������@���x�N�g�����T���v�����O�����K��
    float3 N = normalize(g_Normal.Sample(g_SamplerState, input.uv).rgb);
	
	// �e�N�X�`�����烏�[���h���W���T���v�����O
    float3 worldPos = g_World.Sample(g_SamplerState, input.uv).rgb;
	
	// �s�N�Z����1�_�ɑ΂��āA�S�Ă̓_�����Ƃ̖��邳���v�Z����
    float3 L = normalize(-DLight.Direction.xyz); // ���C�g�̕����𐳋K��
    float diffuse = saturate(dot(N, L)); // �g�U���ˌ��̌v�Z(�@���x�N�g���ƃ��C�g�̕����̓���)
    
    color *= diffuse * DLight.Diffuse / PI + DLight.Ambient; //�J���[�Ɋg�U���ˌ��Ɗ�����K�p
	
	
	return color;
}