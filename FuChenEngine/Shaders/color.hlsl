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
}

cbuffer passConstant : register(b2)
{
	float4x4 gViewProj;
}

float4 CameraLoc : register(b3);

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

	//	float2 PixelPos = shadowPosH.xy*width;
	//	float depthInMap = gShadowMap.SampleLevel(gsamAnisotropicWrap, shadowPosH.xy, 0).r;
	//	return depth > depthInMap? 0:1;

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
		//percentLit += gShadowMap.SampleLevel(gsamAnisotropicWrap, shadowPosH.xy + offsets[i], 0).r - depth > 0 ? 1 : 0;
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

[RootSignature(FuChenSample_RootSig)]
VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	// Transform to homogeneous clip space.
	//vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	//vout.PosH = mul(float4(vin.PosL, abs(sin(time))*0.5+0.5), gWorldViewProj);
	//float4x4 gWorldViewProj = mul(gWorld,mul(gView, gProj));
	float4x4 gWorldViewProj = mul(gWorld,gViewProj);
	float4 gWorldVertex = mul(float4(vin.PosL, 1.0f), gWorld);
	float4x4 gLightViewProj = mul(gWorld, glightOrthoVP);
	//vout.PosH = mul(float4(vin.PosL, abs(sin(time)) * 0.5 + 0.5), gWorldViewProj);


	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

	vout.ShadowPosH = mul(gWorldVertex, glightOrthoVP);

//	float4 PosWorld = mul(gWorld, float4(vin.PosL, 1.0f));
//	vout.ShadowPosH = mul(gLightVP, PosWorld);

	// Just pass vertex color into the pixel shader.
	vout.Color = vin.Color;

	vout.Normal = mul(vin.Normal, gRotation);
	//vout.Normal = vin.Normal;

	// Output vertex attributes for interpolation across triangle.
	vout.TexC = vin.TexC;

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	//Gamma Correction
	float4 diffuseAlbedo = gDiffuseMap.Sample(gsamPointWrap, pin.TexC);
	float4 normalMap = gNormalMap.Sample(gsamPointWrap, pin.TexC);
	float4 tex = diffuseAlbedo * normalMap;

	float4 shadow = CalcShadowFactor(pin.ShadowPosH);
	//float4 shadow = ShadowCalculation(pin.ShadowPosH);

	//float4 RetColor = pow(pin.Normal * 0.5f + 0.5f,1/2.2f);
	//float4 resultColor = pow(diffuseAlbedo * ShadowCalculation(pin.ShadowPosH) + 0.1, 1 / 2.2f);
	//float4 resultColor = tex * ShadowCalculation(pin.ShadowPosH);

	return pow(diffuseAlbedo * (shadow + 0.1), 1 / 2.2f);
	//return pin.Color;
	//return diffuseAlbedo;
}


