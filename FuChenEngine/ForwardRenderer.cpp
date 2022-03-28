#include "stdafx.h"
#include "ForwardRenderer.h"
#include "Engine.h"

ForwardRenderer::ForwardRenderer()
	:rhi(nullptr)
{
	rhi->CreateRHI();
}

ForwardRenderer::~ForwardRenderer()
{

}

void ForwardRenderer::Init()
{
	rhi = RHI::Get();
	rhi->Init();
}

void ForwardRenderer::Destroy()
{
	rhi->Destroy();
	rhi->ReleaseRHI();
}

void ForwardRenderer::Render()
{
	rhi->ResetCmdListAlloc();
	rhi->ResetCommandList("geo_pso");
	if (!initMap)
	{
		rhi->BuildInitialMap();
		initMap = true;
	}
	//Rebuild dirty primitives
	std::unordered_map<std::string, FActor> allActorMap = Engine::GetInstance().GetFScene()->GetAllActor();
	for (std::string renderActorName : Engine::GetInstance().GetFScene()->GetDirtyActor())
	{
		BuildPrimitive(allActorMap[renderActorName]);
		Engine::GetInstance().GetFScene()->EraseDirtyActorByIndex(0);
	}

	//Draw ShadowMap
	rhi->RSSetViewPorts(1, &rhi->GetShadowMapViewport());
	rhi->RESetScissorRects(1, &rhi->GetShadowMapTagRect());
	rhi->TransShadowMapResourBarrier(1, RESOURCE_STATE_GENERIC_READ, RESOURCE_STATE_DEPTH_WRITE);
	rhi->ClearDepthBuffer(rhi->GetShadowMapCUPHandle());
	rhi->SetPipelineState("shadow_pso");
	rhi->SetRenderTargets(0, 0, false, rhi->GetShadowMapCUPHandle());
	rhi->UpdateVP();
	rhi->SetShadowSignature(fRenderScene);
	for (int i = 0; i < fRenderScene.GetNumPrimitive(); i++)
	{
		rhi->UpdateM(fRenderScene.GetPrimitive(i));
		Draw(fRenderScene.GetPrimitive(i));
	}
	rhi->TransShadowMapResourBarrier(1, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_GENERIC_READ);

	//Draw real objecet
	rhi->RSSetViewPorts(1, &rhi->GetViewport());
	rhi->RESetScissorRects(1, &rhi->GetTagRect());
	rhi->TransCurrentBackBufferResourBarrier(1, RESOURCE_STATE_PRESENT, RESOURCE_STATE_RENDER_TARGET);
	float color[4] = { 1.0f,1.0f,1.0f,1.0f };
	rhi->ClearBackBuffer(color);
	rhi->ClearDepthBuffer(rhi->GetDepthStencilViewHandle());
	rhi->SetRenderTargets(1, rhi->GetCurrentBackBufferViewHandle(), false, rhi->GetDepthStencilViewHandle());
	rhi->SetGraphicsRootSignature();
	rhi->SetPipelineState("geo_pso");
	for (int i = 0; i < fRenderScene.GetNumPrimitive(); i++)
	{
		Draw(fRenderScene.GetPrimitive(i));
	}
	rhi->TransCurrentBackBufferResourBarrier(1, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_PRESENT);
	rhi->CloseCommandList();
	rhi->SwapChain();
	rhi->FlushCommandQueue();
}

void ForwardRenderer::BuildPrimitive(FActor& actor)
{
	rhi->CreatePrimitive(actor, fRenderScene);
}

void ForwardRenderer::Draw(FPrimitive& fPrimitive)
{
	rhi->IASetVertexBF(fPrimitive);
	rhi->IASetIndexBF(fPrimitive);
	rhi->IASetPriTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	rhi->DrawFPrimitive(fPrimitive);
}
