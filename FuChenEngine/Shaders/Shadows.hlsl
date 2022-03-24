//***************************************************************************************
// Shadows.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************
#include <ShadowsHead.hlsli>

Texture2D gShadowMap : register(t0);

SamplerState gsamPointWrap        : register(s0);
/*SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);*/

cbuffer cbPerObject : register(b0)
{
	//float4x4 gWorldViewProj; 
	float4x4 gRotation;
	float4x4 gWorld;
	float4x4 gView;
	float4x4 gProj;
	float time;
};

float4 CameraLoc : register(b1);

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
};

[RootSignature(FuChenSample_RootSig)]
VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	float4x4 gWorldViewProj = mul(gWorld, mul(gView, gProj));
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.Color = vin.Color;
	vout.Normal = mul(vin.Normal, gRotation);
	vout.TexC = vin.TexC;

	return vout;
}

void PS(VertexOut pin)
{

}


