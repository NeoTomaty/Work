#include "Common.hlsli"

float4 main(PS_IN_SHADOWMAP input) : SV_TARGET
{
    
    // ���f���̃��l���擾
    float alpha = g_Texture.Sample(g_SamplerState, input.uv).a;

    // �A���t�@���������ꍇ�͉e�����Ȃ�
    clip(alpha - 0.0001f);

    return float4(0.0, 0.0, 0.0, 1.0); // �F�͕Ԃ��Ȃ�
}