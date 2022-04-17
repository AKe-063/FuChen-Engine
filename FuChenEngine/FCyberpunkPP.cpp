#include "stdafx.h"
#include "FCyberpunkPP.h"
#include "RHI.h"

FCyberpunkPP::FCyberpunkPP(std::shared_ptr<FRenderTarget>& sceneColorRT)
{
	float width = (float)sceneColorRT->Width();
	float height = (float)sceneColorRT->Height();

	RHI::Get()->CreateRenderTarget(mCyberpunkRT, width, height, false);

	for (size_t i = 0; i < 3; i++)
	{
		Vertex ver;
		vertices.push_back(ver);
	}
	vertices[0].Pos = vec3(-1.0f, 1.0f, 0.0f);
	vertices[1].Pos = vec3(-1.0f, -3.0f, 0.0f);
	vertices[2].Pos = vec3(3.0f, 1.0f, 0.0f);
}

FCyberpunkPP::~FCyberpunkPP()
{
}

void FCyberpunkPP::InitCyberpunkRTs()
{
	RHI::Get()->InitPPRT(mCyberpunkRT, RESOURCE_FORMAT::FORMAT_R16G16B16A16_FLOAT);
	RHI::Get()->PrepareForRender("");
	fPrimitive = RHI::Get()->CreatePrimitiveByVerticesAndIndices(vertices, indices);
	RHI::Get()->EndPrepare();
}

std::shared_ptr<FRenderTarget>& FCyberpunkPP::GetCyberpunkRT()
{
	return mCyberpunkRT;
}
