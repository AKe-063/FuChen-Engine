#include "stdafx.h"
#include "FBlendBufferRT.h"
#include "RHI.h"

FBlendBufferRT::FBlendBufferRT()
{
	for (size_t i = 0; i < 3; i++)
	{
		Vertex ver;
		vertices.push_back(ver);
	}
	vertices[0].Pos = vec3(-1.0f, 1.0f, 0.0f);
	vertices[1].Pos = vec3(-1.0f, -3.0f, 0.0f);
	vertices[2].Pos = vec3(3.0f, 1.0f, 0.0f);
}

FBlendBufferRT::~FBlendBufferRT()
{
}

void FBlendBufferRT::InitBlendBufferRTs()
{
	fPrimitive = RHI::Get()->CreatePrimitiveByVerticesAndIndices(vertices, indices);
	for (int i = 0; i < blendRTNum; i++)
	{
		RHI::Get()->CreateRenderTarget(mBlendRT[i], 1440.0f, 900.0f, false);
		RHI::Get()->InitPPRT(mBlendRT[i], RESOURCE_FORMAT::FORMAT_R16G16B16A16_FLOAT);
	}
}

void FBlendBufferRT::SwapBlendRT()
{
	currentBlendRT = (currentBlendRT + 1) % blendRTNum;
	currentResourceRT = (currentResourceRT + 1) % blendRTNum;
}

void FBlendBufferRT::BlendRTsClear()
{
	if (currentResourceRT != -1)
	{
		for (int i = 0; i < blendRTNum; i++)
		{
			RHI::Get()->ClearRT(mBlendRT[i]);
		}
	}	
	currentResourceRT = -1;
	currentBlendRT = 0;
}

std::shared_ptr<FRenderTarget>& FBlendBufferRT::GetBlendBufferRT(const int index)
{
	return mBlendRT[index];
}
