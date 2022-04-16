#include "stdafx.h"
#include "FBloomPP.h"
#include "RHI.h"

FBloomPP::FBloomPP(int downUp, std::shared_ptr<FRenderTarget>& sceneColorRT)
	:downUpNum(downUp)
{
	float width = (float)sceneColorRT->Width() / 4;
	float height = (float)sceneColorRT->Height() / 4;

	RHI::Get()->CreateRenderTarget(mBloomSetUpRT, width, height, false);
	RHI::Get()->CreateRenderTarget(mBloomSunmergepsRT, width, height, false);

	for (size_t i = 0; i < downUp; i++)
	{
		std::shared_ptr<FRenderTarget> fRT;
		RHI::Get()->CreateRenderTarget(fRT, float(width / pow(2, i + 1)), float(height / pow(2, i + 1)), false);
		mBloomDownRT.push_back(fRT);
	}
	for (size_t i = 0; i < downUp - 1; i++)
	{
		std::shared_ptr<FRenderTarget> fRT;
		RHI::Get()->CreateRenderTarget(fRT, float(mBloomDownRT[mBloomDownRT.size() - 2 - i]->mWidth), float(mBloomDownRT[mBloomDownRT.size() - 2 - i]->mHeight), false);
		mBloomUpRT.push_back(fRT);
	}
	for (size_t i = 0; i < 3; i++)
	{
		Vertex ver;
		vertices.push_back(ver);
	}
	vertices[0].Pos = vec3(-1.0f, 1.0f, 0.0f);
	vertices[1].Pos = vec3(-1.0f, -3.0f, 0.0f);
	vertices[2].Pos = vec3(3.0f, 1.0f, 0.0f);
}
 
FBloomPP::~FBloomPP()
{

}

void FBloomPP::InitBloomRTs()
{
	RHI::Get()->InitPPRT(mBloomSunmergepsRT, RESOURCE_FORMAT::FORMAT_R16G16B16A16_FLOAT);
	RHI::Get()->InitPPRT(mBloomSetUpRT, RESOURCE_FORMAT::FORMAT_R11G11B10_FLOAT);
	for (size_t i = 0; i < mBloomDownRT.size(); i++)
	{
		RHI::Get()->InitPPRT(mBloomDownRT[i], RESOURCE_FORMAT::FORMAT_R11G11B10_FLOAT);
	}
	for (size_t i = 0; i < mBloomUpRT.size(); i++)
	{
		RHI::Get()->InitPPRT(mBloomUpRT[i], RESOURCE_FORMAT::FORMAT_R11G11B10_FLOAT);
	}
	RHI::Get()->PrepareForRender("");
	fPrimitive = RHI::Get()->CreatePrimitiveByVerticesAndIndices(vertices, indices);
	RHI::Get()->EndPrepare();
}

std::shared_ptr<FRenderTarget>& FBloomPP::GetBloomSetUpRT()
{
	return mBloomSetUpRT;
}

std::shared_ptr<FRenderTarget>& FBloomPP::GetBloomSunmergepsRT()
{
	return mBloomSunmergepsRT;
}

std::vector<std::shared_ptr<FRenderTarget>>& FBloomPP::GetBloomDownRTs()
{
	return mBloomDownRT;
}

std::vector<std::shared_ptr<FRenderTarget>>& FBloomPP::GetBloomUpRTs()
{
	return mBloomUpRT;
}
