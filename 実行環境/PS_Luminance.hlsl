#include "Common.hlsli"

float4 main(PS_IN input) : SV_TARGET
{
    // �J���[�T���v�����O
    float4 color = g_Texture.Sample(g_SamplerState, input.uv);
	
	// �T���v�����O�����J���[���疾�邳���v�Z���A���o����
    float4 t = dot(color.xyz, float3(0.215f, 0.7154f, 0.0721f));
	
	// ���邳0.5f�ȉ��̏ꍇ�͏������݂��s��Ȃ�
    clip(t - 0.5f);
	
    return color;
    
}