TextureCube g_TexCube : register(t0);
SamplerState g_Sam : register(s0);

struct PS_INPUT
{
	float4 PosH : SV_POSITION;
	float3 PosL : POSITION;
	float2 outTexCoord : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 sampleColor = g_TexCube.Sample(g_Sam, input.PosL);
	//sampleColor = pow(color, float3(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f));
	return sampleColor;

}