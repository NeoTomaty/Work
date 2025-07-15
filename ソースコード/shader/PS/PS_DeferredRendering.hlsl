#include "Common.hlsli"

// ���C�g���_�ł̐[�x�Ɣ�r���ĉe�̕����𔻒肷��֐�
float ShadowCalculation(float4 lightPos)
{
      // NDC �� UV���W�ɕϊ�
    float3 projCoords = lightPos.xyz / lightPos.w;
    projCoords = projCoords * 0.5f + 0.5f;

    // Y�����]���K�v�Ȃ炱����
    projCoords.y = 1.0f - projCoords.y;

    // UV���͈͊O�Ȃ�e�𗎂Ƃ��Ȃ�
    if (projCoords.x < 0 || projCoords.x > 1 ||
        projCoords.y < 0 || projCoords.y > 1 ||
        projCoords.z < 0 || projCoords.z > 1)
        return 1.0f;

    // �V���h�E�}�b�v�����r�p�[�x���擾
    float shadowMapDepth = g_ShadowMap.Sample(g_SamplerShadow, projCoords.xy).r;

    // �V���h�E�o�C�A�X�i�e�̃A�[�e�B�t�@�N�g��h�����߂̔������j
    float bias = 0.005f;

    // �[�x��r�F���C�g���_���猩�����s���Ǝ��ۂ̉��s��
    float shadow = projCoords.z - bias > shadowMapDepth ? 0.0f : 1.0f;

    return shadow;
}

float4 main(PS_IN input) : SV_TARGET
{
    float4 color = float4(0, 0, 0, 1);
    
    // DEFAULT�o��
    if (TextureType == 0)
    {
        // �@��
        float3 normal = g_Normal.Sample(g_SamplerState, input.uv);
    
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

        // �V���h�E�̌v�Z
            float shadow;
            if (isShadow)
            {
                shadow = ShadowCalculation(input.lightPos);
            }
            else
            {
                shadow = 1.0f;
            }

        
        // �ŏI�F�̍���
            color.rgb += texColor.rgb * diffuse * DLight.Diffuse.rgb * shadow;
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
            //float specular = pow(saturate(dot(R, V)), shininess);

                float3 lightColor = PLight[i].Diffuse * PLight[i].Intensity; // ���C�g�̐F
            
            // �_�����̉e�������Z
                color.rgb += texColor.rgb * (diffuse * lightColor * attenuation);

            }
        
        }
        
    }
    // Color�o��
    else if (TextureType == 1)
    {
        // �J���[�e�N�X�`�����T���v�����O
        color = g_Texture.Sample(g_SamplerState, input.uv);
        
    }
    // �@���o��
    else if (TextureType == 2)
    {
        color = g_Normal.Sample(g_SamplerState, input.uv);
        
    }
    // ���[���h���W�o��
    else if (TextureType == 3)
    {
        // �J���[�e�N�X�`�����T���v�����O
        color = g_World.Sample(g_SamplerState, input.uv);
    }
    // �[�x���o��
    else if (TextureType == 4)
    {
        // �[�x�e�N�X�`�����T���v�����O
        color = g_Depth.Sample(g_SamplerState, input.uv);
    }
    // ���x�o�b�t�@���o��
    else if (TextureType == 5)
    {
        // ���x�o�b�t�@�e�N�X�`�����T���v�����O
        color = g_MVector.Sample(g_SamplerState, input.uv);
    }
    
    return color;
}