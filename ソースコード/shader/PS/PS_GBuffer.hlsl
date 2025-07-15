#include "Common.hlsli"

// Pos:�N���b�v��ԍ��W
// prevPos:�O�t���[���̃N���b�v���
float2 GetMotionVector(float4 Pos, float4 prevPos)
{
    //�O�t���[���̃N���b�v���
    float4 clipPos = prevPos;
    // ��������i�N���b�v�O�́j�I�u�W�F�N�g�͓����𖳎�
    if (clipPos.w <= 0)
        return 0;

    // �������Z�iClip ��� �� ���K���f�o�C�X���W�iNDC�j[-1�`1]�j
    clipPos.xyz /= clipPos.w;
    
    // �O�t���[���̃E�B���h�E���W�֕ϊ�
    // �i�r���[�|�[�g���W�͌Œ�Ȃ̂ō���͒��ڐ錾�j
    float viewportWidth = 1280.0f;
    float viewportHeight = 720.0f;
    //float2 prevWindowPos = (clipPos.xy + 1) * 0.5f * float2(viewportWidth, viewportHeight);
    float2 clipToWindowScale = float2(viewportWidth * 0.5, viewportHeight * -0.5);  // �i640.0f, -360.0f�j
    float2 clipToWindowBias = float2(viewportWidth * 0.5, viewportHeight * 0.5);    // �i640.0f, 360.0f�j
    float2 prevWindowPos = clipPos.xy * clipToWindowScale + clipToWindowBias;

    // ���[�V�����x�N�g�����v�Z
    return saturate(prevWindowPos - Pos.xy);
}


PS_OUT_GBUFFER main(PS_IN input)
{
    PS_OUT_GBUFFER output;

    
    //// �A���t�@���������ꍇ�͖���
    //clip(alpha - 0.0001f);
    
    // �J���[
    output.Albedo = g_Texture.Sample(g_SamplerState, input.uv);
    
    // �@��
    if (UseNormalMap == 0){
        // �@���𐳋K������w������1��
        float4 normal = float4(normalize(input.normal), 1.0);
        // skybox�p�ɍ쐬�����@��
        output.Normal = normal;
    }
    else if (UseNormalMap == 1)
    {

        // �@���}�b�v���Q��
        float3 SampledNormal = g_Normal.Sample(g_SamplerState, input.uv).xyz;
        // �@���͈̔͂��O�`�P����[�P�`�{�P�ɂ���i�^���W�F���g�X�y�[�X�j
        SampledNormal = (SampledNormal - 0.5f) * 2.0f;
        
        // TBN�s������i���[���h��Ԃɕϊ����邽�߁j
        float3x3 TBN = float3x3(normalize(input.tangent),
                            normalize(input.bitangent),
                            normalize(input.normal));

        // �@�������[���h��ԂɓK�p����
        float3 normal = mul(SampledNormal, TBN);
        
        // ���K�����ďo��
        output.Normal = float4(normal.xyz, 1.0);
        
    }

    // ���[���h���W
    output.World = input.worldPos;
    
    // �[�x
    float zValue = input.pos.z / input.pos.w;
    output.Depth = saturate(zValue.r);
    
    // ���[�V�����x�N�g��
    float2 mVector = GetMotionVector(input.pos, input.prevPos);

    // UV��Ԃł̃��[�V�����x�N�g��
    output.MVector = float4(mVector, 0.0f, 1.0f);
    
    return output;
}