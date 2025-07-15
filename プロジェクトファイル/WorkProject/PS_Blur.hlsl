#include "Common.hlsli"

float4 main(PS_IN input) : SV_TARGET
{
    float value = 1.0f / 5.0f; // 5x5�̃t�B���^�[�l
    float filter[5] =
    {
        value, value, value, value, value,
    };

	// �t�B���^�[�Ɋ�Â��ĐF���擾
    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);  // �o�͐F�̏�����
    float2 uvOffset = 1.0f / texSize;               // 1�s�N�Z����UV��ł̈ړ���
    uvOffset *= blurDir;                            // �w�肳�ꂽ�����݈̂ړ�������
    float2 uv = input.uv - uvOffset * 2;            // �����ʒu���炸�ꂽUV���W���v�Z
	
	// �t�B���^�[��K�p���ĐF���v�Z
    for (int i = 0; i < 5; ++i)
    {
		// �e�N�X�`������F���T���v�����O���ăt�B���^�[��������
        color += g_Texture.Sample(g_SamplerState, uv) * filter[i];
        uv += uvOffset;
    }
    
    
    return color;
}