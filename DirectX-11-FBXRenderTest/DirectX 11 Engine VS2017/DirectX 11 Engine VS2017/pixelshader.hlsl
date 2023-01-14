#define PI 3.1415927f

cbuffer alphaBuffer : register(b0)
{
	float3 cameraPosition;
    float alpha;

	float3 albedo;
	float metallic;
	float roughness;
	float ao;
}

cbuffer lightDataBuffer : register(b1)
{
	float3 lightPos1;
	float padding1;
	float3 lightPos2;
	float padding2;
	float3 lightPos3;
	float padding3;
	float3 lightPos4;
	float padding4;
	float3 lightCol;
	float padding5;
}

struct PS_INPUT
{
    float4 inPosition : SV_POSITION;
    float2 inTexCoord : TEXCOORD;

	float3 inNormal : NORMAL;
	float3 inTangent: TANGENT;
	float3 inWorldPos : WORLD_POSITION;
};

Texture2D objTexture : TEXTURE : register(t0);
SamplerState objSamplerState : SAMPLER : register(s0);

Texture2D brdfLUT : register(t1);
TextureCube skyIR : register(t2);
TextureCube skyPrefilter: register(t3);
Texture2D specularTexture : TEXTURE: register(t4);
Texture2D normalSRV : TEXTURE: register(t5);
Texture2D depthTexture : TEXTURE: register(t6);
Texture2D dissolveNoiseTexture : TEXTURE: register(t7);

float NormalDistributionGGXTR(float3 normalVec, float3 halfwayVec, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;   // a2 = a^2
	float NdotH = max(dot(normalVec, halfwayVec), 0.0);     // NdotH = normalVec.halfwayVec
	float NdotH2 = NdotH * NdotH;   // NdotH2 = NdotH^2
	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
	denom = PI * denom * denom;

	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)  // k is a remapping of roughness based on direct lighting or IBL lighting
{
	float r = roughness + 1.0f;
	float k = (r * r) / 8.0f;

	float nom = NdotV;
	float denom = NdotV * (1.0f - k) + k;

	return nom / denom;
}

float GeometrySmith(float3 normalVec, float3 viewDir, float3 lightDir, float k)
{
	float NdotV = max(dot(normalVec, viewDir), 0.0f);
	float NdotL = max(dot(normalVec, lightDir), 0.0f);
	float ggx1 = GeometrySchlickGGX(NdotV, k);
	float ggx2 = GeometrySchlickGGX(NdotL, k);

	return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)   // cosTheta is n.v and F0 is the base reflectivity
{
	return (F0 + (1.0f - F0) * pow(1.0 - cosTheta, 5.0f));
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)   // cosTheta is n.v and F0 is the base reflectivity
{
	return F0 + (max(float3(1.0f - roughness, 1.0f - roughness, 1.0f - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0f);
}

void CalcRadiance(PS_INPUT input, float3 viewDir, float3 normalVec, float3 lightPos, float3 lightCol, float3 F0, out float3 rad)
{
	//calculate light radiance
	float3 lightDir = normalize(lightPos - input.inWorldPos);
	float3 halfwayVec = normalize(viewDir + lightDir);
	float distance = length(lightPos - input.inWorldPos);
	float attenuation = 1.0f / dot(float3(1.0f, 0.0f, 1.0f), float3(1.0f, distance, distance*distance));
	float3 radiance = lightCol * attenuation;

	//Cook-Torrance BRDF
	float D = NormalDistributionGGXTR(normalVec, halfwayVec, roughness);
	float G = GeometrySmith(normalVec, viewDir, lightDir, roughness);
	float3 F = FresnelSchlick(max(dot(halfwayVec, viewDir), 0.0f), F0);

	float3 kS = F;
	float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
	kD *= (1.0 - metallic);

	float3 nom = D * G * F;
	float denom = 4 * max(dot(normalVec, viewDir), 0.0f) * max(dot(normalVec, lightDir), 0.0) + 0.001f; // 0.001f just in case product is 0
	float3 specular = nom / denom;

	//Add to outgoing radiance Lo
	float NdotL = max(dot(normalVec, lightDir), 0.0f);
	rad = (((kD * albedo / PI) + specular) * radiance * NdotL);
}


float4 main(PS_INPUT input) : SV_TARGET
{
    float3 pixelColor = objTexture.Sample(objSamplerState, input.inTexCoord);
    
	float3 viewDir = normalize(cameraPosition - input.inWorldPos);
	float3 normalVec = normalize(input.inNormal);
	float3 R = reflect(-viewDir, normalVec);

	float3 F0 = float3(0.04f, 0.04f, 0.04f);
	F0 = lerp(F0, albedo, metallic);

	float3 rad = float3(0.0f, 0.0f, 0.0f);
	//reflectance equation
	float3 Lo = float3(0.0f, 0.0f, 0.0f);

	CalcRadiance(input, viewDir, normalVec, lightPos1, lightCol, F0, rad);
	Lo += rad;

	CalcRadiance(input, viewDir, normalVec, lightPos2, lightCol, F0, rad);
	Lo += rad;

	CalcRadiance(input, viewDir, normalVec, lightPos3, lightCol, F0, rad);
	Lo += rad;

	CalcRadiance(input, viewDir, normalVec, lightPos4, lightCol, F0, rad);
	Lo += rad;

	float3 kS = FresnelSchlickRoughness(max(dot(normalVec, viewDir), 0.0f), F0, roughness);
	float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
	kD *= 1.0 - metallic;

	float3 irradiance = skyIR.Sample(objSamplerState, normalVec).rgb;
	float3 diffuse = albedo * irradiance;// *pixelColor;

	const float MAX_REF_LOD = 4.0f;
	float3 prefilteredColor = skyPrefilter.SampleLevel(objSamplerState, R, roughness * MAX_REF_LOD).rgb;
	float2 brdf = brdfLUT.Sample(objSamplerState, float2(max(dot(normalVec, viewDir), 0.0f), roughness)).rg;
	float3 specular = prefilteredColor * (kS * brdf.x + brdf.y);//

	float3 ambient = (kD * diffuse + specular) * ao;//
	float3 color = ambient + Lo;

	color = color / (color + float3(1.0f, 1.0f, 1.0f));
	color = pow(color, float3(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f));
	
	return float4(color, alpha);
}