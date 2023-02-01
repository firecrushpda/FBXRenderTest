#define PI 3.1415927f

cbuffer alphaBuffer : register(b0)
{
	float3 cameraPosition;
    float alpha;

	float3 albedo;
	float cbmetallic;
	float cbroughness;
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

cbuffer BSDFConstantBuffer : register(b2)
{
	float3 baseColor;
	float metallic;
	float subsurface;
	float specular;
	float roughness;
	float specularTint;
	float anisotropic;
	float sheen;
	float sheenTint;
	float clearcoat;
	float clearcoatGloss;
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


float sqr(float x) { return x * x; }

float SchlickFresnel(float u)
{
	float m = clamp(1 - u, 0, 1);
	float m2 = m * m;
	return m2 * m2*m; // pow(m,5)
}

float GTR1(float NdotH, float a)
{
	if (a >= 1) return 1 / PI;
	float a2 = a * a;
	float t = 1 + (a2 - 1)*NdotH*NdotH;
	return (a2 - 1) / (PI*log(a2)*t);
}

float GTR2(float NdotH, float a)
{
	float a2 = a * a;
	float t = 1 + (a2 - 1)*NdotH*NdotH;
	return a2 / (PI * t*t);
}

float GTR2_aniso(float NdotH, float HdotX, float HdotY, float ax, float ay)
{
	return 1 / (PI * ax*ay * sqr(sqr(HdotX / ax) + sqr(HdotY / ay) + NdotH * NdotH));
}

float smithG_GGX(float NdotV, float alphaG)
{
	float a = alphaG * alphaG;
	float b = NdotV * NdotV;
	return 1 / (NdotV + sqrt(a + b - a * b));
}

float smithG_GGX_aniso(float NdotV, float VdotX, float VdotY, float ax, float ay)
{
	return 1 / (NdotV + sqrt(sqr(VdotX*ax) + sqr(VdotY*ay) + sqr(NdotV)));
}

float3 mon2lin(float3 x)
{
	return float3(pow(x[0], 2.2), pow(x[1], 2.2), pow(x[2], 2.2));
}

float4 main(PS_INPUT input) : SV_TARGET
{
	float3 L = normalize(lightPos1 - input.inWorldPos);//normalize(lightPos1);
	float3 V = normalize(cameraPosition - input.inWorldPos);//normalize(cameraPosition);
	float3 N = normalize(input.inNormal);
	float3 X = normalize(input.inTangent - N * dot(input.inTangent, N));
	float3 Y = normalize(cross(X, N));

	float NdotL = dot(N,L);
	float NdotV = dot(N,V);

	float3 H = normalize(L + V);
	float NdotH = dot(N, H);
	float LdotH = dot(L, H);

	float3 Cdlin = pow(baseColor, 2.2f);
	float Cdlum = .3 * Cdlin[0] + .6 * Cdlin[1] + .1 * Cdlin[2];

	float3 Ctint = Cdlum > 0 ? Cdlin / Cdlum : float3(1,1,1);
	float3 Cspec0 = lerp(specular *.08 * lerp(float3(1, 1, 1), Ctint, specularTint), Cdlin, metallic);
	float3 Csheen = lerp(float3(1, 1, 1), Ctint, sheenTint);

	// Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
   // and mix in diffuse retro-reflection based on roughness
	float FL = SchlickFresnel(NdotL);
	float FV = SchlickFresnel(NdotV);
	float Fd90 = 0.5 + 2 * LdotH * LdotH * roughness;
	float Fd = lerp(1.0, Fd90, FL) * lerp(1.0, Fd90, FV);

	// Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
	// 1.25 scale is used to (roughly) preserve albedo
	// Fss90 used to "flatten" retroreflection based on roughness
	float Fss90 = LdotH * LdotH*roughness;
	float Fss = lerp(1.0, Fss90, FL) * lerp(1.0, Fss90, FV);
	float ss = 1.25 * (Fss * (1 / (NdotL + NdotV) - .5) + .5);

	// specular
	float aspect = sqrt(1 - anisotropic * .9);
	float ax = max(.001, sqr(roughness) / aspect);
	float ay = max(.001, sqr(roughness)*aspect);
	float Ds = GTR2_aniso(NdotH, dot(H, X), dot(H, Y), ax, ay);
	float FH = SchlickFresnel(LdotH);
	float3 Fs = lerp(Cspec0, float3(1,1,1), FH);
	float Gs;
	Gs = smithG_GGX_aniso(NdotL, dot(L, X), dot(L, Y), ax, ay);
	Gs *= smithG_GGX_aniso(NdotV, dot(V, X), dot(V, Y), ax, ay);

	// sheen
	float3 Fsheen = FH * sheen * Csheen;

	// clearcoat (ior = 1.5 -> F0 = 0.04)
	float Dr = GTR1(NdotH, lerp(.1, .001, clearcoatGloss));
	float Fr = lerp(.04, 1.0, FH);
	float Gr = smithG_GGX(NdotL, .25) * smithG_GGX(NdotV, .25);

	return float4(((1 / PI) * lerp(Fd, ss, subsurface) * Cdlin + Fsheen) * (1 - metallic) + Gs * Fs * Ds + .25 * clearcoat * Gr * Fr * Dr,1.0);
}