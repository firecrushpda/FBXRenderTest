//#pragma pack_matrix( row_major )

cbuffer perObjectBuffer : register(b0)
{
	float4x4 mat;
    float4x4 wvpMatrix;
}; 

struct VS_INPUT
{
	float3 inPos : POSITION;
	float3 inNormal : NORMAL;
	float3 inTangent : TANGENT;
	float2 inTexCoord : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 PosH : SV_POSITION;
	float3 PosL : POSITION;
	float2 outTexCoord : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input) 
{
	VS_OUTPUT vOut;

	float4 posH = mul(float4(input.inPos, 1.0f), mat);
	vOut.PosH = posH.xyww;
	vOut.PosL = input.inPos;
	vOut.outTexCoord = input.inTexCoord;
	return vOut;

}