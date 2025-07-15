#include "Common.hlsli"

float4 main(PS_IN input) : SV_TARGET
{
    float4 color = float4(0, 0, 0, 0);

    float shininess = 1.0f; // �P�x�i�傫���قǗ}�������ʔ��˂Ɂj
    float3 specularColor = float3(0.01f, 0.01f, 0.01f); // ���˂̂�������}����
    
    // �@��
    float3 normal = input.normal;
    
    // �@���}�b�v���g�p���Ă��邩�H
    if (UseNormalMap)
    {
        // �@���}�b�v���Q��
        float3 normal = g_Normal.Sample(g_SamplerState, input.uv).xyz;
        // �@���͈̔͂��O�`�P����[�P�`�{�P�ɂ���i�^���W�F���g�X�y�[�X�j
        normal = (normal - 0.5f) * 2.0f;
        
        // TBN�s������i���[���h��Ԃɕϊ����邽�߁j
        float3x3 TBN = float3x3(normalize(input.tangent),
                            normalize(input.bitangent),
                            normalize(input.normal));

        // �@�������[���h��ԂɓK�p����
        normal = mul(normal, TBN);
    }
    
	// �e�N�X�`������F���擾
    float4 texColor = g_Texture.Sample(g_SamplerState, input.uv);

    // ���s�����̗L���Ŋ������ω�
    float3 ambient = DLight.Enable ? DLight.Ambient : NightBright;
    color.rgb += texColor.rgb * ambient;
    
    float3 N = normalize(normal); // �@���x�N�g���𐳋K��
    float3 V = normalize(Camera.eyePos - input.worldPos.xyz); // �����x�N�g���̐��K��
    
    // ���s�������������ꍇ�i���\���j
    if (DLight.Enable)
    {
        float3 L = normalize(-DLight.Direction.xyz); // ���C�g�̕����𐳋K��
        float3 R = reflect(-L, N); // ���˃x�N�g��

         // �g�U���ˌ��̌v�Z
        float diffuse = saturate(dot(N, L));

        // ���ʔ��ˁi�X�y�L�����j�̌v�Z
        float specular = pow(saturate(dot(R, V)), shininess);

        // �ŏI�F�̍���
        color.rgb += texColor.rgb * diffuse * DLight.Diffuse.rgb;
        color.rgb += specular * specularColor;
    }
    
    // �_����
    for (int i = 0; i < PLightNum; i++)
    {
        // �_�������������ꍇ
        if (PLight[i].Enable)
        {
            float3 lightPos = PLight[i].Position.xyz; // ���C�g�̍��W
            float3 L = normalize(lightPos - input.worldPos.xyz); // �����x�N�g��
            float3 R = reflect(-L, N); // ���˃x�N�g��

            float distance = length(lightPos - input.worldPos.xyz); // �����Ƃ̋���
            
            // �����ɂ�錸���v�Z
            // ��ʎ��u1 / (a + b*d + c*d^2)�v�ɂ�錻���̌���
            float a = PLight[i].Attenuation.x;
            float b = PLight[i].Attenuation.y;
            float c = PLight[i].Attenuation.z;
            float attenuation = 1.0f / (a + b * distance + c * distance * distance);

            // �g�U���ˁi�f�B�t���[�Y�j�Ƌ��ʔ��ˁi�X�y�L�����j�̌v�Z
            float diffuse = saturate(dot(N, L));
            float specular = pow(saturate(dot(R, V)), shininess);

            float3 lightColor = PLight[i].Diffuse * PLight[i].Intensity; // ���C�g�̐F
            
            // �_�����̉e�������Z
            color.rgb += texColor.rgb * (diffuse * lightColor * attenuation);   // �X�y�L�����͂Ȃ�������������

        }
        
    }

    
    return color;
}
