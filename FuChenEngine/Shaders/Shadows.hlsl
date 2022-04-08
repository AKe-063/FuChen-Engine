//***************************************************************************************
// Shadows.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************
#include <ShadowsHead.hlsli>

SamplerState gsamPointWrap        : register(s0);
/*SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);*/

cbuffer cbPerObject : register(b0)
{ 
	float4x4 gScale;
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

struct VertexIn
{
	float3 PosL  : POSITION;
	float4 TangentY : TANGENTY;
	float4 TangentX : TANGENTX;
	float4 Normal : NORMAL;
	float2 TexC    : TEXCOORD;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
	float4 Color : COLOR;
	float4 Normal : NORMAL;
	float2 TexC    : TEXCOORD;
};

[RootSignature(FuChenSample_ShadowSig)]
VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	float4x4 gWorldViewProj = mul(gWorld, glightVP);
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.Color = vin.TangentX;
	vout.Normal = mul(vin.Normal, gRotation);
	vout.TexC = vin.TexC;

	return vout;
}

void PS(VertexOut pin)
{

}


