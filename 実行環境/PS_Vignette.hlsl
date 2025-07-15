#include "Common.hlsli"

float4 main(PS_IN input) : SV_TARGET
{
    float2 uv = input.uv;

    // �r�l�b�g�̒��S
    float2 screenCenter = float2(vignetteCenter[0], vignetteCenter[1]); 
    
    // ��ʒ��S����̋������v�Z
    float2 distVec = uv - screenCenter;
    float dist = length(distVec);

    // �r�l�b�g�t�@�N�^�[�v�Z
    float vignette = smoothstep(vignetteRadius, 1.0, dist);
    vignette = pow(vignette, vignetteStrength);

    // ���̐F�擾
    float4 color = g_Texture.Sample(g_SamplerState, uv);

    // �r�l�b�g���ʂ�K�p
    color.rgb *= (1.0 - vignette);

    return color;
}