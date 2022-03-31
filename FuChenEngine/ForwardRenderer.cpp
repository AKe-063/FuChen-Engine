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

	//Draw ShadowMap
	rhi->RSSetViewPorts(1, &rhi->GetShadowMapViewport());
	rhi->RESetScissorRects(1, &rhi->GetShadowMapTagRect());
	rhi->TransShadowMapResourBarrier(1, RESOURCE_STATE_GENERIC_READ, RESOURCE_STATE_DEPTH_WRITE);
	rhi->ClearDepthBuffer(rhi->GetShadowMapCUPHandle());
	rhi->SetPipelineState("shadow_pso");
	rhi->SetRenderTargets(0, 0, false, rhi->GetShadowMapCUPHandle());
	rhi->UpdateVP();
	rhi->SetShadowSignature(fRenderScene);
	for (auto primitiveMap : fRenderScene.GetAllPrimitives())
	{
		auto primitive = primitiveMap.second;
		rhi->UpdateM(*primitive);
		Draw(*primitive);
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
	for (auto primitiveMap : fRenderScene.GetAllPrimitives())
	{
		auto primitive = primitiveMap.second;
		Draw(*primitive);
	}
	rhi->TransCurrentBackBufferResourBarrier(1, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_PRESENT);
	rhi->CloseCommandList();
	rhi->SwapChain();
	rhi->FlushCommandQueue();
}

void ForwardRenderer::BuildDirtyPrimitive(FScene& fScene)
{
	rhi->ResetCmdListAlloc();
	rhi->ResetCommandList("geo_pso");
	std::unordered_map<std::string, FActor> allActorMap = fScene.GetAllActor();
	for (std::string renderActorName : fScene.GetDirtyActor())
	{
		rhi->TransActorToRenderPrimitive(allActorMap[renderActorName], fRenderScene);
		if (!fRenderScene.ContainTexture(allActorMap[renderActorName].GetMainTexName()))
		{
			rhi->TransTextureToRenderResource(
				allActorMap[renderActorName],
				FAssetManager::GetInstance().GetTextureFilePathFromName(allActorMap[renderActorName].GetMainTexName()),
				fRenderScene);
		}
		if (!fRenderScene.ContainTexture(allActorMap[renderActorName].GetNormalTexName()))
		{
			rhi->TransTextureToRenderResource(
				allActorMap[renderActorName],
				FAssetManager::GetInstance().GetTextureFilePathFromName(allActorMap[renderActorName].GetNormalTexName()),
				fRenderScene);
		}
		fRenderScene.GetPrimitive(renderActorName).SetMainRsvIndex(fRenderScene.GetTexByName(allActorMap[renderActorName].GetMainTexName())->GetSrvIndex());
		fRenderScene.GetPrimitive(renderActorName).SetNormalRsvIndex(fRenderScene.GetTexByName(allActorMap[renderActorName].GetNormalTexName())->GetSrvIndex());
		fScene.EraseDirtyActorByIndex(0);
	}
	rhi->CloseCommandList();
	rhi->FlushCommandQueue();
}

void ForwardRenderer::Draw(FPrimitive& fPrimitive)
{
	rhi->IASetVertexBF(fPrimitive);
	rhi->IASetIndexBF(fPrimitive);
	rhi->IASetPriTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	rhi->DrawFPrimitive(fPrimitive);
}
