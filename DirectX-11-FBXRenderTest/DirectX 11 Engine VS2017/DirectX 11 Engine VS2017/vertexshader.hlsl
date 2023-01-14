
cbuffer mycBuffer : register(b0)
{
    float4x4 wvpMatrix;
	float4x4 worldMatrix;
}; 

struct VS_INPUT
{
    float3 inPos : POSITION;
	float3 inNormal : NORMAL;
	float3 inTangent : TANGENT;
	float2 inTexCoord : TEXCOORD;
	//
};

struct VS_OUTPUT
{
    float4 outPosition : SV_POSITION;
    float2 outTexCoord : TEXCOORD;

	float3 outNormal : NORMAL;
	float3 outTangent: TANGENT;
	float3 outWorldPos : WORLD_POSITION;
};

VS_OUTPUT main(VS_INPUT input) 
{
    VS_OUTPUT output;
    output.outPosition = mul(float4(input.inPos, 1.0f), wvpMatrix);
	output.outWorldPos = mul(float4(input.inPos, 1.0f), worldMatrix).rgb;
	output.outNormal = mul(input.inNormal, (float3x3)worldMatrix);
	output.outTangent = mul(input.inTangent, (float3x3)worldMatrix);
	output.outTexCoord = input.inTexCoord;
    return output;
}