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

	// Transform to homogeneous clip space.
	//vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	//vout.PosH = mul(float4(vin.PosL, abs(sin(time))*0.5+0.5), gWorldViewProj);
	float4x4 gWorldViewProj = mul(gWorld,mul(gView, gProj));
	//vout.PosH = mul(float4(vin.PosL, abs(sin(time)) * 0.5 + 0.5), gWorldViewProj);
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

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
	//float4 RetColor = pow(pin.Normal * 0.5f + 0.5f,1/2.2f);
	return tex;
	//return pin.Color;
}


