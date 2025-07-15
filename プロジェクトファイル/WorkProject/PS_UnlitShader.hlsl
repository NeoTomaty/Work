#include "Common.hlsli"


float4 main(PS_IN input) : SV_TARGET
{
	
    //�J���[�̏o��
    float4 color = g_Texture.Sample(g_SamplerState, input.uv);
        
    // ���s�����̐F�����Z
    float3 light = DLight.Diffuse * 0.9f;

    // �e�N�X�`���J���[�ɉ��Z
    float3 result = saturate(color.rgb * light);

    // �_�����ɂ����Z��
    for (int i = 0; i < PLightNum; ++i)
    {
        PointLight light = PLight[i];
        if (light.Enable == 0)
            continue;

        // ���C�g�܂ł̃x�N�g��
        float3 toLight = light.Position - input.worldPos.xyz;
        float dist = length(toLight);

        // �����W��
        float attenuation = 1.0 / (light.Attenuation.x + light.Attenuation.y * dist + light.Attenuation.z * dist * dist);

        // �g�U�� �~ ���� �~ ����
        float3 diffuse = light.Diffuse.rgb * light.Intensity * attenuation;

        // �F�ɉ��Z
        result += color.rgb * diffuse * 0.1f;
    }
    
    return float4(result, color.a);
}