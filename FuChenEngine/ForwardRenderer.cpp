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
	std::vector<INPUT_ELEMENT_DESC> mInputLayout =
	{
		{ "POSITION", 0, INPUT_FORMAT::INPUT_FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION::INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TangentY", 0, INPUT_FORMAT::INPUT_FORMAT_R32G32B32A32_FLOAT, 0, 12, INPUT_CLASSIFICATION::INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TangentX", 0, INPUT_FORMAT::INPUT_FORMAT_R32G32B32A32_FLOAT, 0, 28, INPUT_CLASSIFICATION::INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "Normal", 0, INPUT_FORMAT::INPUT_FORMAT_R32G32B32A32_FLOAT, 0, 44, INPUT_CLASSIFICATION::INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, INPUT_FORMAT::INPUT_FORMAT_R32G32_FLOAT, 0, 60, INPUT_CLASSIFICATION::INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	fShaderManager->AddShader(L"..\\FuChenEngine\\Shaders\\color.hlsl");
	fShaderManager->GetShaderMap()[L"..\\FuChenEngine\\Shaders\\color.hlsl"].SetShaderLayout(mInputLayout);
	fShaderManager->AddShader(L"..\\FuChenEngine\\Shaders\\Shadows.hlsl");
	fShaderManager->GetShaderMap()[L"..\\FuChenEngine\\Shaders\\Shadows.hlsl"].SetShaderLayout(mInputLayout);
	fShaderManager->AddShader(L"..\\FuChenEngine\\Shaders\\bloomsetup.hlsl");
	fShaderManager->AddShader(L"..\\FuChenEngine\\Shaders\\bloomdown.hlsl");
	fShaderManager->AddShader(L"..\\FuChenEngine\\Shaders\\bloomup.hlsl");
	fShaderManager->AddShader(L"..\\FuChenEngine\\Shaders\\bloomsunmergeps.hlsl");
	fShaderManager->AddShader(L"..\\FuChenEngine\\Shaders\\tonemapps.hlsl");
	mInputLayout.clear();
	mInputLayout =
	{
		{ "POSITION", 0, INPUT_FORMAT::INPUT_FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION::INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	fShaderManager->GetShaderMap()[L"..\\FuChenEngine\\Shaders\\bloomsetup.hlsl"].SetShaderLayout(mInputLayout);
	fShaderManager->GetShaderMap()[L"..\\FuChenEngine\\Shaders\\bloomdown.hlsl"].SetShaderLayout(mInputLayout);
	fShaderManager->GetShaderMap()[L"..\\FuChenEngine\\Shaders\\bloomup.hlsl"].SetShaderLayout(mInputLayout);
	fShaderManager->GetShaderMap()[L"..\\FuChenEngine\\Shaders\\bloomsunmergeps.hlsl"].SetShaderLayout(mInputLayout);
	fShaderManager->GetShaderMap()[L"..\\FuChenEngine\\Shaders\\tonemapps.hlsl"].SetShaderLayout(mInputLayout);

	rhi = RHI::Get();
	rhi->Init(fShaderManager);
	rhi->CreateRenderTarget(mShadowMap, 2048.0f, 2048.0f);
	rhi->CreateRenderTarget(mSceneColorRT, 1440.0f, 900.0f);

	mBloomPP = std::make_shared<FBloomPP>(4, mSceneColorRT);
	mBloomPP->InitBloomRTs();

	rhi->InitShadowRT(mShadowMap);
 	rhi->InitPPRT(mSceneColorRT, RESOURCE_FORMAT::FORMAT_R16G16B16A16_FLOAT);
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
	rhi->TransResourBarrier(mShadowMap->DSResource().get(), 1, RESOURCE_STATE_GENERIC_READ, RESOURCE_STATE_DEPTH_WRITE);
	rhi->SetPipelineState("shadow_pso");
	rhi->SetRenderTargets(0, 0, false, mShadowMap->Dsv());
	rhi->DrawShadow(fRenderScene, mShadowMap);

	//TransBarrier
	rhi->TransResourBarrier(mShadowMap->DSResource().get(), 1, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_GENERIC_READ);
	rhi->TransResourBarrier(mSceneColorRT->ColorResource(0).get(), 1, RESOURCE_STATE_PRESENT, RESOURCE_STATE_RENDER_TARGET);
	rhi->TransResourBarrier(mSceneColorRT->DSResource().get(), 1, RESOURCE_STATE_GENERIC_READ, RESOURCE_STATE_DEPTH_WRITE);

	//Draw to SceneColor
	rhi->BeginBaseDraw();
	rhi->SetRenderTargets(1, mSceneColorRT->Rtv(0), false, mSceneColorRT->Dsv());
	rhi->DrawToHDR(fRenderScene, mShadowMap, mSceneColorRT);
	rhi->TransResourBarrier(mSceneColorRT->ColorResource(0).get(), 1, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_PRESENT);
	rhi->TransResourBarrier(mSceneColorRT->DSResource().get(), 1, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_GENERIC_READ);

	//Bloom setup
	rhi->TransResourBarrier(mBloomPP->GetBloomSetUpRT()->ColorResource(0).get(), 1, RESOURCE_STATE_PRESENT, RESOURCE_STATE_RENDER_TARGET);
	rhi->TransResourBarrier(mBloomPP->GetBloomSetUpRT()->DSResource().get(), 1, RESOURCE_STATE_GENERIC_READ, RESOURCE_STATE_DEPTH_WRITE);
	rhi->SetRenderTargets(1, mBloomPP->GetBloomSetUpRT()->Rtv(0), false, mBloomPP->GetBloomSetUpRT()->Dsv());
	rhi->DrawBloomDown("bloom_pso", mSceneColorRT, mBloomPP->GetBloomSetUpRT());
	rhi->TransResourBarrier(mBloomPP->GetBloomSetUpRT()->ColorResource(0).get(), 1, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_PRESENT);
	rhi->TransResourBarrier(mBloomPP->GetBloomSetUpRT()->DSResource().get(), 1, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_GENERIC_READ);

	//Bloom down
	for (size_t i = 0; i < mBloomPP->downUpNum; i++)
	{
		rhi->TransResourBarrier(mBloomPP->GetBloomDownRTs()[i]->ColorResource(0).get(), 1, RESOURCE_STATE_PRESENT, RESOURCE_STATE_RENDER_TARGET);
		rhi->TransResourBarrier(mBloomPP->GetBloomDownRTs()[i]->DSResource().get(), 1, RESOURCE_STATE_GENERIC_READ, RESOURCE_STATE_DEPTH_WRITE);
		rhi->SetRenderTargets(1, mBloomPP->GetBloomDownRTs()[i]->Rtv(0), false, mBloomPP->GetBloomDownRTs()[i]->Dsv());
		if (i == 0)
		{
			rhi->DrawBloomDown("bloom_down_pso", mBloomPP->GetBloomSetUpRT(), mBloomPP->GetBloomDownRTs()[i]);
		}
		else
		{
			rhi->DrawBloomDown("bloom_down_pso", mBloomPP->GetBloomDownRTs()[i - 1], mBloomPP->GetBloomDownRTs()[i]);
		}
		rhi->TransResourBarrier(mBloomPP->GetBloomDownRTs()[i]->ColorResource(0).get(), 1, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_PRESENT);
		rhi->TransResourBarrier(mBloomPP->GetBloomDownRTs()[i]->DSResource().get(), 1, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_GENERIC_READ);
	}

	//Bloom up
	for (size_t i = 0; i < mBloomPP->downUpNum - 1; i++)
	{
		rhi->TransResourBarrier(mBloomPP->GetBloomUpRTs()[i]->ColorResource(0).get(), 1, RESOURCE_STATE_PRESENT, RESOURCE_STATE_RENDER_TARGET);
		rhi->TransResourBarrier(mBloomPP->GetBloomUpRTs()[i]->DSResource().get(), 1, RESOURCE_STATE_GENERIC_READ, RESOURCE_STATE_DEPTH_WRITE);
		rhi->SetRenderTargets(1, mBloomPP->GetBloomUpRTs()[i]->Rtv(0), false, mBloomPP->GetBloomUpRTs()[i]->Dsv());
		auto bloomDownRTNum = mBloomPP->downUpNum;
		if (i == 0)
		{
			rhi->DrawBloomUp("bloom_up_pso", mBloomPP->GetBloomDownRTs()[bloomDownRTNum - 1], mBloomPP->GetBloomDownRTs()[bloomDownRTNum - 2], mBloomPP->GetBloomUpRTs()[i]);
		}
		else
		{
			rhi->DrawBloomUp("bloom_up_pso", mBloomPP->GetBloomUpRTs()[i - 1], mBloomPP->GetBloomDownRTs()[bloomDownRTNum - 2 - i], mBloomPP->GetBloomUpRTs()[i]);
		}
		rhi->TransResourBarrier(mBloomPP->GetBloomUpRTs()[i]->ColorResource(0).get(), 1, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_PRESENT);
		rhi->TransResourBarrier(mBloomPP->GetBloomUpRTs()[i]->DSResource().get(), 1, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_GENERIC_READ);
	}

	//Bloom sunmergeps
	rhi->TransResourBarrier(mBloomPP->GetBloomSunmergepsRT()->ColorResource(0).get(), 1, RESOURCE_STATE_PRESENT, RESOURCE_STATE_RENDER_TARGET);
	rhi->TransResourBarrier(mBloomPP->GetBloomSunmergepsRT()->DSResource().get(), 1, RESOURCE_STATE_GENERIC_READ, RESOURCE_STATE_DEPTH_WRITE);
	rhi->SetRenderTargets(1, mBloomPP->GetBloomSunmergepsRT()->Rtv(0), false, mBloomPP->GetBloomSunmergepsRT()->Dsv());
	rhi->DrawBloomUp("bloom_sunmergeps_pso", mBloomPP->GetBloomUpRTs()[mBloomPP->downUpNum - 2], mBloomPP->GetBloomSetUpRT(), mBloomPP->GetBloomSunmergepsRT());
	rhi->TransResourBarrier(mBloomPP->GetBloomSunmergepsRT()->ColorResource(0).get(), 1, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_PRESENT);
	rhi->TransResourBarrier(mBloomPP->GetBloomSunmergepsRT()->DSResource().get(), 1, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_GENERIC_READ);

	//ToneMapps
	rhi->TransCurrentBackBufferResourBarrier(1, RESOURCE_STATE_PRESENT, RESOURCE_STATE_RENDER_TARGET);
	rhi->SetRenderTargets(1, rhi->GetCurrentBackBufferViewHandle(), false, rhi->GetDepthStencilViewHandle());
	rhi->ToneMapps("tonemapps_pso", mSceneColorRT, mBloomPP->GetBloomSunmergepsRT());
	rhi->TransCurrentBackBufferResourBarrier(1, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_PRESENT);

	//--------------------------------
// 	rhi->TransCurrentBackBufferResourBarrier(1, RESOURCE_STATE_PRESENT, RESOURCE_STATE_RENDER_TARGET);
// 	rhi->SetRenderTargets(1, rhi->GetCurrentBackBufferViewHandle(), false, rhi->GetDepthStencilViewHandle());
// 	rhi->DrawPrimitives(fRenderScene, mShadowMap, mSceneColorRT);
// 	rhi->TransCurrentBackBufferResourBarrier(1, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_PRESENT);
	
	rhi->EndDraw();
}

void ForwardRenderer::BuildDirtyPrimitive(FScene& fScene)
{
	rhi->BeginTransSceneDataToRenderScene("geo_pso");

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
