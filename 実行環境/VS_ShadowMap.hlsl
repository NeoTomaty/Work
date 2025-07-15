#include "Common.hlsli"


PS_IN_SHADOWMAP main(VS_IN input)
{
    PS_IN_SHADOWMAP output;
   
    float4 pos = input.pos;
    
    float4 worldPos = mul(pos, World);
    
    float4 viewPos = mul(worldPos, LightView);
    float4 clipPos = mul(viewPos, LightProj);
    
    output.position = clipPos; // SV_POSITION ‚ÉƒZƒbƒg
    
    output.uv = input.uv;
    
    return output;
}