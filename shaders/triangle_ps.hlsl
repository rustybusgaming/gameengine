// DirectX 11 HLSL Pixel Shader for Simple Triangle
// Nexus Engine - Triangle Demo

struct PSInput
{
    float4 position : SV_POSITION;
    float3 color    : COLOR;
};

float4 main(PSInput input) : SV_TARGET
{
    return float4(input.color, 1.0);
}
