#include <postprocessblend.hlsli>

Texture2D gSceneColor : register(t0);
Texture2D gBlend : register(t1);

SamplerState gSceneColorSampler : register(s0);

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
	int X = floor(pin.PosH.x);
	int Y = floor(pin.PosH.y);

	float2 Tex;
	Tex.x = 1.0f * X / RenderTargetSize[0];
	Tex.y = 1.0f * Y / RenderTargetSize[1];

	float4 SceneColor = gSceneColor.Sample(gSceneColorSampler, Tex);
	float4 BlendColor = gBlend.Sample(gSceneColorSampler, Tex);

	half3 LinearColor = SceneColor.rgb + BlendColor.rgb;

	float4 OutColor = float4(LinearColor, SceneColor.a);

	return OutColor;
}


