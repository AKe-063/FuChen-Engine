#include <colorhead.hlsli>

Texture2D gDiffuseMap : register(t0);
Texture2D gNormalMap : register(t1);
Texture2D gShadowMap : register(t2);

SamplerState gsamPointWrap        : register(s0);
/*SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);*/
SamplerComparisonState gsamShadow : register(s1);

cbuffer cbPerObject : register(b0)
{
	float4x4 gRotation;
	float4x4 gWorld;
	float time;
};

cbuffer lightConstant : register(b1)
{
	float4x4 glightPrpj;
	float4x4 glightVP;
	float4x4 glightOrthoVP;
	float gLightDensity;
	float3 gLightDir;
}

cbuffer passConstant : register(b2)
{
	float4x4 gViewProj;
}

cbuffer CameraConstant : register(b3)
{
	float4 gCameraLoc;
}

cbuffer cbMaterial : register(b4)
{
	float4 gDiffuseAlbedo;
	float3 gFresnelR0;
	float  gRoughness;
};

struct VertexIn
{
	float3 PosL  : POSITION;
	float4 Color : COLOR;
	float4 Normal : NORMAL;
	float2 TexC    : TEXCOORD;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
	float4 Color : COLOR;
	float4 Normal : NORMAL;
	float2 TexC    : TEXCOORD;
	float4 ShadowPosH : POSITION;
};

float CalcShadowFactor(float4 shadowPosH)
{
	shadowPosH.xyz /= shadowPosH.w;

	float depth = shadowPosH.z;

	uint width, height, numMips;
	gShadowMap.GetDimensions(0, width, height, numMips);

	float dx = 1.0f / (float)width;

	float percentLit = 0.0f;
	const float2 offsets[9] =
	{
		float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
	};

	[unroll]
	for (int i = 0; i < 9; ++i)
	{
		percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow, shadowPosH.xy + offsets[i], depth).r;
	}

	return percentLit / 9.0f;
}


float ShadowCalculation(float4 PosH)
{
	PosH.xyz /= PosH.w;
	float CD = PosH.z;
	uint width, height, nummip;
	gShadowMap.GetDimensions(0, width, height, nummip);
	float2 Ppos = PosH.xy * width;
	float Dep = gShadowMap.Load(int3(Ppos, 0)).r;

	return CD > Dep ? 0 : 1;
}

float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
	float cosIncidentAngle = saturate(dot(normal, lightVec));

	float f0 = 1.0f - cosIncidentAngle;
	float3 reflectPercent = R0 + (1.0f - R0) * (f0 * f0 * f0 * f0 * f0);

	return reflectPercent;
}

float3 BlinnPhong(
	float3 lightStrength, float3 lightVec,
	float3 normal, float3 toEye,
	float4 gDiffuseAlbedo, float3 gFresnelR0, float gRoughness)
{
	const float shininess = 1.0f - gRoughness;
	const float m = shininess * 256.0f;
	float3 halfVec = normalize(toEye + lightVec);

	float roughnessFactor = (m + 8.0f) * pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
	float3 fresnelFactor = SchlickFresnel(gFresnelR0, halfVec, lightVec);

	float3 specAlbedo = fresnelFactor * roughnessFactor;

	// Our spec formula goes outside [0,1] range, but we are 
	// doing LDR rendering.  So scale it down a bit.
	specAlbedo = specAlbedo / (specAlbedo + 1.0f);

	return (gDiffuseAlbedo.rgb + specAlbedo) * lightStrength;
}

float3 ComputeDirectionalLight(
	float3 LightDirection, float3 LightStrength,
	float4 gDiffuseAlbedo, float3 gFresnelR0, float gRoughness,
	float3 normal, float3 toEye)
{
	// The light vector aims opposite the direction the light rays travel.
	float3 lightVec = -LightDirection;

	// Scale light down by Lambert's cosine law.
	float ndotl = max(dot(lightVec, normal), 0.0f);
	float3 lightStrength = LightStrength * ndotl;

	return BlinnPhong(lightStrength, lightVec, normal, toEye, gDiffuseAlbedo, gFresnelR0, gRoughness);
}

[RootSignature(FuChenSample_RootSig)]
VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	// Transform to homogeneous clip space.
	float4x4 gWorldViewProj = mul(gWorld,gViewProj);
	float4 gWorldVertex = mul(float4(vin.PosL, 1.0f), gWorld);
	float4x4 gLightViewProj = mul(gWorld, glightOrthoVP);
	//vout.PosH = mul(float4(vin.PosL, abs(sin(time)) * 0.5 + 0.5), gWorldViewProj);


	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

	vout.ShadowPosH = mul(gWorldVertex, glightOrthoVP);

	// Just pass vertex color into the pixel shader.
	vout.Color = vin.Color;

	//vout.Normal = mul(vin.Normal, gRotation);
	vout.Normal = mul(vin.Normal, gWorld);

	// Output vertex attributes for interpolation across triangle.
	vout.TexC = vin.TexC;

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	/*pin.Normal = normalize(pin.Normal);
	float3 toEyes = normalize(gCameraLoc - pin.PosH);
	float4 ambient = { 0,0,0,1.0f };
	ambient = ambient * gDiffuseAlbedo;

	const float shininess = 1.0f - gRoughness;
	Material mat = { gDiffuseAlbedo, gFresnelR0, shininess };
	float3 shadowFactor = 1.0f;
	Light gLights[MaxLights];
	float4 directLight = ComputeLighting(gLights, mat, pin.PosH,
		pin.Normal, toEyes, shadowFactor);*/

	//Gamma Correction
	float4 diffuseAlbedo = gDiffuseMap.Sample(gsamPointWrap, pin.TexC);
	float4 normalMap = gNormalMap.Sample(gsamPointWrap, pin.TexC);
	float4 tex = diffuseAlbedo * normalMap;

	float4 shadow = CalcShadowFactor(pin.ShadowPosH);

	float4 directLight = float4(ComputeDirectionalLight(
		gLightDir, gLightDensity,
		diffuseAlbedo, gFresnelR0, gRoughness,
		pin.Normal, gCameraLoc), 1.0f);
	float4 DiffuseAlbedo = diffuseAlbedo * 0.01;
	float4 FinalColor = (shadow + 0.1) * (DiffuseAlbedo + directLight);
	return pow(FinalColor, 1 / 2.2f);
	/*float4 litColor = diffuseAlbedo + directLight;
	litColor.a = gDiffuseAlbedo.a;*/

	//return pow(diffuseAlbedo * (shadow + 0.1), 1 / 2.2f);
	//return litColor * shadow;
}


