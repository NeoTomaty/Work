#include "Common.hlsli"

PS_IN main( VS_IN input)
{
    PS_IN output;
    
    // ���[�J�����W���X�N���[�����W�֕ϊ�
    output.pos = float4(input.pos.xyz, 1.0f);
    
    // �֊s���Ƃ��ĕ\�����邽�߂ɁA�@�������֒��_���ړ�
    // �g�傪�傫���Ȃ肷����̂�h�����ߕ␳������
    output.pos.xyz += normalize(input.normal) * 0.01f;
    output.pos = mul(output.pos, World);    // ���[���h���W
    output.pos = mul(output.pos, View);     // �r���[���W
    output.pos = mul(output.pos, Proj);     // �v���W�F�N�V�������W
    
    // uv���W
    output.uv = input.uv;
    
    // �s�N�Z���V�F�[�_�[�ŗ��p����@����n��
    output.normal = mul(input.normal.xyz, (float3x3) World);
    
    return output;
}
