#include "Common.hlsli"

float4 main(PS_IN input) : SV_TARGET
{
    float4 color = (0.0f, 0.0f, 0.0f, 1.0f);
    
    // �ʏ�̕`�挋�ʂ��擾
    float4 sceneColor = g_Texture.Sample(g_SamplerState, input.uv);

    // �P�x�}�b�v���擾
    float4 bloomColor = g_LuminanceTex.Sample(g_SamplerState, input.uv);

    // ���Z����
    color = sceneColor + bloomColor * bloomStrength;

    return color;
    
}
