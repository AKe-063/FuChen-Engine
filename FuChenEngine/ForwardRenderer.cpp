#include "stdafx.h"
#include "ForwardRenderer.h"
#include "DX12RHI.h"
#include "Engine.h"

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

void ForwardRenderer::Render()
{
	if (flagOfTest == 0)
	{
		for (auto actor : Engine::GetInstance().GetFScene()->GetAllActor())
		{
			BuildPrimitive(actor.second);
		}
		flagOfTest++;
	}
	Draw();
}

void ForwardRenderer::BuildPrimitive(FActor& actor)
{
		fRenderScene.AddPrimitive(rhi->CreatePrimitive(actor));
}

void ForwardRenderer::Draw()
{
	rhi->StartDraw();
	rhi->RSSetViewPorts(1, &rhi->GetViewport());
	rhi->RESetScissorRects(1, &rhi->GetTagRect());
	float color[4] = { 0.0f,0.2f,0.5f,0.25f };
	rhi->ClearBackBufferAndDepthBuffer(color, 1.0f, 0, 0);
	rhi->SetRenderTargets(1);
	rhi->SetGraphicsRootSignature();
	rhi->DrawFRenderScene(fRenderScene);
	rhi->EndDraw();
}

void ForwardRenderer::BuildInitialMap()
{
	rhi->BuildInitialMap();
}
