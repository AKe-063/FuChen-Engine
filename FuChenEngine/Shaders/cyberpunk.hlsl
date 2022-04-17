#include <cyberpunk.hlsli>

Texture2D gSceneMap : register(t0);

SamplerState gSceneMapSample : register(s0);

int2 RenderTargetSize : register(b0);

struct VertexIn
{
	float3 PosL  : POSITION;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
};

[RootSignature(FuChenSample_BloomSig)]
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.PosH = float4(vin.PosL,1.0f);
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{

	const float BloomThreshold = 1.0f;

	int X = floor(pin.PosH.x);
	int Y = floor(pin.PosH.y);

	float Width = RenderTargetSize[0];
	float Height = RenderTargetSize[1];

	float DeltaU = 5.0f / RenderTargetSize[0];
	float DeltaV = 5.0f / RenderTargetSize[1];

	float2 Tex;
	Tex.x = 1.0f * X / Width;
	Tex.y = 1.0f * Y / Height;

	float4 Color0 = gSceneMap.Sample(gSceneMapSample, Tex + float2(-DeltaU, -DeltaV));
	float4 Color1 = gSceneMap.Sample(gSceneMapSample, Tex + float2(+DeltaU, +DeltaV));
	float4 Color2 = gSceneMap.Sample(gSceneMapSample, Tex + float2(0, -DeltaV));

	float4 OutColor = float4(Color0.r, Color1.g, Color2.b, 1.0f);

	return OutColor;
}


