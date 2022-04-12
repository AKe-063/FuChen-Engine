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
	fShaderManager = std::make_shared<FShaderManager>();
	fShaderManager->AddShader(L"..\\FuChenEngine\\Shaders\\color.hlsl");
	fShaderManager->AddShader(L"..\\FuChenEngine\\Shaders\\Shadows.hlsl");

	rhi = RHI::Get();
	rhi->Init(fShaderManager);
	rhi->CreateRenderTarget(mShadowMap, 2048.0f, 2048.0f);
	rhi->CreateRenderTarget(mBloomRT, 1440.0f, 900.0f);
}

void ForwardRenderer::Destroy()
{
	rhi->Destroy();
	rhi->ReleaseRHI();
}

void ForwardRenderer::Render()
{
	rhi->BeginRender("geo_pso");
	//Draw ShadowMap
	rhi->TransShadowMapResourBarrier(mShadowMap->DSResource().get(), 1, RESOURCE_STATE_GENERIC_READ, RESOURCE_STATE_DEPTH_WRITE);
	rhi->SetPipelineState("shadow_pso");
	rhi->SetRenderTargets(0, 0, false, mShadowMap->Dsv());
	rhi->DrawShadow(fRenderScene, mShadowMap);

	//TransBarrier
	rhi->TransShadowMapResourBarrier(mShadowMap->DSResource().get(), 1, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_GENERIC_READ);
	rhi->TransCurrentBackBufferResourBarrier(1, RESOURCE_STATE_PRESENT, RESOURCE_STATE_RENDER_TARGET);

	//Draw real object
	rhi->BeginBaseDraw();
	rhi->SetRenderTargets(1, rhi->GetCurrentBackBufferViewHandle(), false, rhi->GetDepthStencilViewHandle());
	rhi->DrawPrimitives(fRenderScene, mShadowMap);
	rhi->TransCurrentBackBufferResourBarrier(1, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_PRESENT);

	rhi->EndDraw();
}

void ForwardRenderer::BuildDirtyPrimitive(FScene& fScene)
{
	rhi->BeginTransSceneDataToRenderScene("geo_pso");
	rhi->BuildShadowRenderTex(mShadowMap);
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
	rhi->EndTransScene();

	//Before render, to upload MVP buffer data to GPU.
	for (auto primitiveMap : fRenderScene.GetAllPrimitives())
	{
		auto primitive = primitiveMap.second;
		rhi->UpdateM(*primitive);
	}
	rhi->UpdateVP();
	rhi->UploadMaterialData();
}
