#include "Common.hlsli"


PS_IN main( VS_IN input )
{
    PS_IN output;
	
    float4 pos = float4(input.pos.xyz, 1.0f); // w��1.0f�ɂ���
    
    // ���݃t���[���̒��_���W
    output.pos = mul(pos, World);                   // ���[���h�s��Ə�Z
    output.worldPos = float4(output.pos.xyz, 1.0f); // ���[���h��Ԃł̒��_�ʒu��ۑ�
    output.pos = mul(output.pos, View);             // �r���[�s��Ə�Z
    output.viewPos = output.pos;                    // �r���[��Ԃł̒��_�ʒu��ۑ�
    output.pos = mul(output.pos, Proj);             // �v���W�F�N�V�����s��Ə�Z�i�N���b�v��ԁj
    
    // �O�t���[���̒��_���W
    output.prevPos = mul(pos, PrevWorld);                   // �O�t���[���̃��[���h�s��Ə�Z
    output.prevWorldPos = float4(output.prevPos.xyz, 1.0f); // �O�t���[���̃��[���h��Ԃł̒��_�ʒu��ۑ�
    output.prevPos = mul(output.prevPos, PrevView);         // �O�t���[���̃r���[�s��Ə�Z
    output.prevViewPos = output.prevPos;                    // �O�t���[���̃r���[��Ԃł̒��_�ʒu��ۑ�
    output.prevPos = mul(output.prevPos, PrevProj);         // �O�t���[���̃v���W�F�N�V�����s��Ə�Z�i�N���b�v��ԁj

    // ���C�g�̃J�������W���v�Z
    //float4 Light = mul(pos, World);
    float4 Light = pos;
    Light = mul(Light, LightView);
    Light = mul(Light, LightProj);
    output.lightPos = Light;
    
	// �@���Ɛڐ������[���h�s��ŕϊ�
    output.normal = normalize(mul(input.normal.xyz, (float3x3) World));
    output.tangent = normalize(mul(input.tangent.xyz, (float3x3) World));

    // �]�@�������߂�
    output.bitangent = normalize(cross(output.normal, output.tangent)); 
    
	// ���̑��̃f�[�^�͂��̂܂ܓn��
    output.uv = input.uv;       // UV���W
    output.color = input.color; // �J���[
    
    return output;
}

    
