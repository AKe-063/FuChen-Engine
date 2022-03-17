#include "stdafx.h"
#include "ForwardRenderer.h"
#include "DX12RHI.h"

ForwardRenderer::ForwardRenderer()
{
#if _DX12_START_UP
	rhi = std::make_unique<DX12RHI>();
#endif
}

ForwardRenderer::~ForwardRenderer()
{

}

void ForwardRenderer::Init()
{
	rhi->Init();
}

void ForwardRenderer::Destroy()
{
	rhi->Destroy();
}

void ForwardRenderer::Draw()
{
	rhi->Draw();
}

void ForwardRenderer::BuildInitialMap()
{
	rhi->BuildInitialMap();
}
