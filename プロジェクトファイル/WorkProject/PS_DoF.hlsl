#include "Common.hlsli"

// �[�x�������j�A��
float LinearizeDepth(float z, float nearZ, float farZ)
{
    return (nearZ * farZ) / (farZ - z * (farZ - nearZ));
}

float4 main(PS_IN input) : SV_TARGET
{
    
    float4 color = (0.0f, 0.0f, 0.0f, 1.0f);
    
	// �e�����擾
    float4 albedo = g_Texture.Sample(g_SamplerState, input.uv);
    float depth = g_DepthMap.Sample(g_SamplerState, input.uv).r;
    float4 blur = g_BlurTex.Sample(g_SamplerState, input.uv);

    // z�����j�A��
    float linearDepth = LinearizeDepth(depth, Camera.nearClip, Camera.farClip);

    
    float focus = cameraFocus; // �J��������s���g�������ʒu�܂ł̋���
    float range = DoFRange;    // ��ʊE�[�x�łڂ����̉摜�ɕω����鋗��

    float centerDist = abs(linearDepth - focus);    // �t�H�[�J�X�ʒu����̋���
    float rate = saturate(centerDist / range);      // ���K��(0�`1)

    color = lerp(albedo, blur, rate);


    return color;
}