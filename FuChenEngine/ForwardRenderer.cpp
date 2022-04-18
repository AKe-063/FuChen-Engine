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

	rhi = RHI::Get();
	rhi->CreateRenderTarget(mBackBufferRT, 1440.0f, 900.0f, true);
	rhi->Init(fShaderManager, *mBackBufferRT->mBackBufferRT);
	rhi->CreateRenderTarget(mShadowMap, 2048.0f, 2048.0f, false);
	rhi->CreateRenderTarget(mSceneColorRT, 1440.0f, 900.0f, false);

	rhi->PrepareForRender("");
	mBloomPP = std::make_shared<FBloomPP>(4, mSceneColorRT);
	mBloomPP->InitBloomRTs();
	mCyberpunkRT = std::make_shared<FCyberpunkPP>(mSceneColorRT);
	mCyberpunkRT->InitCyberpunkRTs();
	mBlendBufferRT = std::make_shared<FBlendBufferRT>();
	mBlendBufferRT->InitBlendBufferRTs();

	rhi->InitShadowRT(mShadowMap);
 	rhi->InitPPRT(mSceneColorRT, RESOURCE_FORMAT::FORMAT_R16G16B16A16_FLOAT);
	rhi->EndPrepare();
}

void ForwardRenderer::Destroy()
{
	rhi->Destroy();
	rhi->ReleaseRHI();
}

void ForwardRenderer::Render()
{
	rhi->BeginRender("geo_pso");
	rhi->BeginPass("ClearBlendRT");
	mBlendBufferRT->BlendRTsClear();
	rhi->EndPass();
	//Draw ShadowMap
	ShadowPass();

	//Draw to SceneColor
	SceneColorPass();

	//PostProcess pass
	PostProcessPass(POST_PROCESS_TYPE::Bloom);
	PostProcessPass(POST_PROCESS_TYPE::Cyberpunk);
	
	ToneMappsPass();
	
	rhi->EndDraw(*mBackBufferRT->mBackBufferRT);
}

void ForwardRenderer::BuildDirtyPrimitive(FScene& fScene)
{
	rhi->PrepareForRender("geo_pso");

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
	rhi->EndPrepare();

	//Before render, to upload MVP buffer data to GPU.
	for (auto primitiveMap : fRenderScene.GetAllPrimitives())
	{
		auto primitive = primitiveMap.second;
		rhi->UpdateM(*primitive);
	}
	rhi->UpdateVP();
	rhi->UploadMaterialData();
}

void ForwardRenderer::ShadowPass()
{
	rhi->BeginPass("ShadowPass");
	rhi->TransResourBarrier(mShadowMap->DSResource().get(), 1, RESOURCE_STATE_GENERIC_READ, RESOURCE_STATE_DEPTH_WRITE);
	rhi->SetRenderTargets(0, 0, false, mShadowMap->Dsv());
	rhi->BasePrepare(mShadowMap, true);
	for (auto primitivePair : fRenderScene.GetAllPrimitives())
	{
		rhi->SetPrimitive("shadow_pso", primitivePair.second);
		rhi->UploadResourceBuffer(0, primitivePair.second->GetObjCBIndex());
		rhi->UploadResourceBuffer(1, 2, -1, -1);
		rhi->DrawFPrimitive(*primitivePair.second);
	}
	rhi->TransResourBarrier(mShadowMap->DSResource().get(), 1, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_GENERIC_READ);
	rhi->EndPass();
}

void ForwardRenderer::SceneColorPass()
{
	rhi->BeginPass("SceneColorPass");
	rhi->TransResourBarrier(mSceneColorRT->ColorResource(0).get(), 1, RESOURCE_STATE_COMMON, RESOURCE_STATE_RENDER_TARGET);
	rhi->TransResourBarrier(mSceneColorRT->DSResource().get(), 1, RESOURCE_STATE_GENERIC_READ, RESOURCE_STATE_DEPTH_WRITE);
	rhi->SetRenderTargets(1, mSceneColorRT->Rtv(0), false, mSceneColorRT->Dsv());
	rhi->BasePrepare(mSceneColorRT, false);
	for (auto primitivePair : fRenderScene.GetAllPrimitives())
	{
		rhi->SetPrimitive("hdr_geo_pso", primitivePair.second);
		rhi->UploadResourceBuffer(0, primitivePair.second->GetObjCBIndex());
		rhi->UploadResourceBuffer(1, 2, 3, 4);
		rhi->UploadResourceTable(5, primitivePair.second->GetMainRsvIndex());
		rhi->UploadResourceTable(6, primitivePair.second->GetNormalRsvIndex());
		rhi->UploadResourceTable(7, mShadowMap->GetRTDesc(RTDepthStencilBuffer).rtTexture->GetSrvIndex());
		rhi->DrawFPrimitive(*primitivePair.second);
	}
	rhi->TransResourBarrier(mSceneColorRT->ColorResource(0).get(), 1, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_COMMON);
	rhi->TransResourBarrier(mSceneColorRT->DSResource().get(), 1, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_GENERIC_READ);
	rhi->EndPass();
}

void ForwardRenderer::PostProcessPass(POST_PROCESS_TYPE ppType)
{
	switch (ppType)
	{
	case POST_PROCESS_TYPE::Bloom:
		BloomPass();
		break;
	case POST_PROCESS_TYPE::Cyberpunk:
		CyberpunkPass();
		break;
	default:
		assert(0);
		break;
	}
}

void ForwardRenderer::ToneMappsPass()
{
	//ToneMapps
	rhi->BeginPass("ToneMappsPass");
	std::shared_ptr<BackBufferRT> backBufferRT = mBackBufferRT->mBackBufferRT;
	rhi->TransResourBarrier(backBufferRT->mSwapChainBuffer[backBufferRT->CurrentBackBufferRT].get(), 1, RESOURCE_STATE_PRESENT, RESOURCE_STATE_RENDER_TARGET);
	rhi->SetRenderTargets(1, backBufferRT->GetCurrentBackBufferHandle(), false, backBufferRT->GetCurrentBackDepthStencilBufferHandle());
	rhi->BasePrepare(mBackBufferRT, false);
	rhi->SetPrimitive("tonemapps_pso", mBlendBufferRT->fPrimitive);
	rhi->UploadResourceTable(0, mBlendBufferRT->mBlendRT[mBlendBufferRT->currentResourceRT]->GetRTDesc(RTColorBuffer).rtTexture->GetSrvIndex());
	FVector2DInt rtSize;
	rtSize.x = (int)backBufferRT->mViewport.Width;
	rtSize.y = (int)backBufferRT->mViewport.Height;
	rhi->UploadResourceConstants(1, 2, &rtSize, 0);
	rhi->DrawFPrimitive(*mBlendBufferRT->fPrimitive);
	rhi->TransResourBarrier(backBufferRT->mSwapChainBuffer[backBufferRT->CurrentBackBufferRT].get(), 1, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_PRESENT);
	rhi->EndPass();
}

void ForwardRenderer::BlendPass(std::shared_ptr<FRenderTarget>& mRT, std::shared_ptr<FRenderTarget>& mBlendResourceRT, std::shared_ptr<FRenderTarget>& mBlendBaseRT, const std::string& passName)
{
	rhi->BeginPass(passName);
	rhi->TransResourBarrier(mRT->ColorResource(0).get(), 1, RESOURCE_STATE_COMMON, RESOURCE_STATE_RENDER_TARGET);
	rhi->TransResourBarrier(mRT->DSResource().get(), 1, RESOURCE_STATE_GENERIC_READ, RESOURCE_STATE_DEPTH_WRITE);
	rhi->SetRenderTargets(1, mRT->Rtv(0), false, mRT->Dsv());
	rhi->BasePrepare(mRT, false);
	rhi->SetPrimitive("postprocessblend_pso", mBlendBufferRT->fPrimitive);
	rhi->UploadResourceTable(0, mBlendBaseRT->GetRTDesc(RTColorBuffer).rtTexture->GetSrvIndex());
	rhi->UploadResourceTable(1, mBlendResourceRT->GetRTDesc(RTColorBuffer).rtTexture->GetSrvIndex());
	FVector2DInt rtSize;
	rtSize.x = (int)mRT->Width();
	rtSize.y = (int)mRT->Height();
	rhi->UploadResourceConstants(2, 2, &rtSize, 0);
	rhi->DrawFPrimitive(*mBlendBufferRT->fPrimitive);
	rhi->TransResourBarrier(mRT->ColorResource(0).get(), 1, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_COMMON);
	rhi->TransResourBarrier(mRT->DSResource().get(), 1, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_GENERIC_READ);
	rhi->EndPass();
}

void ForwardRenderer::BloomPass()
{
	//Bloom setup
	rhi->BeginPass("BloomSetUpPass");
 	rhi->TransResourBarrier(mBloomPP->GetBloomSetUpRT()->ColorResource(0).get(), 1, RESOURCE_STATE_COMMON, RESOURCE_STATE_RENDER_TARGET);
 	rhi->TransResourBarrier(mBloomPP->GetBloomSetUpRT()->DSResource().get(), 1, RESOURCE_STATE_GENERIC_READ, RESOURCE_STATE_DEPTH_WRITE);
 	rhi->SetRenderTargets(1, mBloomPP->GetBloomSetUpRT()->Rtv(0), false, mBloomPP->GetBloomSetUpRT()->Dsv());
	rhi->BasePrepare(mBloomPP->GetBloomSetUpRT(), false);
	rhi->SetPrimitive("bloom_pso", mBloomPP->fPrimitive);
	rhi->UploadResourceTable(0, mSceneColorRT->GetRTDesc(RTColorBuffer).rtTexture->GetSrvIndex());
	FVector2DInt rtSize;
	rtSize.x = mSceneColorRT->Width();
	rtSize.y = mSceneColorRT->Height();
	rhi->UploadResourceConstants(1, 2, &rtSize, 0);
	rhi->DrawFPrimitive(*mBloomPP->fPrimitive);
 	rhi->TransResourBarrier(mBloomPP->GetBloomSetUpRT()->ColorResource(0).get(), 1, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_COMMON);
 	rhi->TransResourBarrier(mBloomPP->GetBloomSetUpRT()->DSResource().get(), 1, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_GENERIC_READ);
	rhi->EndPass();

 	//Bloom down
 	for (size_t i = 0; i < mBloomPP->downUpNum; i++)
 	{
		rhi->BeginPass("BloomDownPass");
 		rhi->TransResourBarrier(mBloomPP->GetBloomDownRTs()[i]->ColorResource(0).get(), 1, RESOURCE_STATE_COMMON, RESOURCE_STATE_RENDER_TARGET);
 		rhi->TransResourBarrier(mBloomPP->GetBloomDownRTs()[i]->DSResource().get(), 1, RESOURCE_STATE_GENERIC_READ, RESOURCE_STATE_DEPTH_WRITE);
 		rhi->SetRenderTargets(1, mBloomPP->GetBloomDownRTs()[i]->Rtv(0), false, mBloomPP->GetBloomDownRTs()[i]->Dsv());
		rhi->BasePrepare(mBloomPP->GetBloomDownRTs()[i], false);
		rhi->SetPrimitive("bloom_down_pso", mBloomPP->fPrimitive);
 		if (i == 0)
 		{
			rhi->UploadResourceTable(0, mBloomPP->GetBloomSetUpRT()->GetRTDesc(RTColorBuffer).rtTexture->GetSrvIndex());
			FVector2DInt rtSize;
			rtSize.x = mBloomPP->GetBloomSetUpRT()->Width();
			rtSize.y = mBloomPP->GetBloomSetUpRT()->Height();
			rhi->UploadResourceConstants(1, 2, &rtSize, 0);
 		}
 		else
 		{
			rhi->UploadResourceTable(0, mBloomPP->GetBloomDownRTs()[i - 1]->GetRTDesc(RTColorBuffer).rtTexture->GetSrvIndex());
			FVector2DInt rtSize;
			rtSize.x = mBloomPP->GetBloomDownRTs()[i - 1]->Width();
			rtSize.y = mBloomPP->GetBloomDownRTs()[i - 1]->Height();
			rhi->UploadResourceConstants(1, 2, &rtSize, 0);
 		}
		rhi->DrawFPrimitive(*mBloomPP->fPrimitive);
 		rhi->TransResourBarrier(mBloomPP->GetBloomDownRTs()[i]->ColorResource(0).get(), 1, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_COMMON);
 		rhi->TransResourBarrier(mBloomPP->GetBloomDownRTs()[i]->DSResource().get(), 1, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_GENERIC_READ);
		rhi->EndPass();
	}
 
 	//Bloom up
 	for (size_t i = 0; i < mBloomPP->downUpNum - 1; i++)
 	{
		rhi->BeginPass("BloomUpPass");
 		rhi->TransResourBarrier(mBloomPP->GetBloomUpRTs()[i]->ColorResource(0).get(), 1, RESOURCE_STATE_COMMON, RESOURCE_STATE_RENDER_TARGET);
 		rhi->TransResourBarrier(mBloomPP->GetBloomUpRTs()[i]->DSResource().get(), 1, RESOURCE_STATE_GENERIC_READ, RESOURCE_STATE_DEPTH_WRITE);
 		rhi->SetRenderTargets(1, mBloomPP->GetBloomUpRTs()[i]->Rtv(0), false, mBloomPP->GetBloomUpRTs()[i]->Dsv());
		rhi->BasePrepare(mBloomPP->GetBloomUpRTs()[i], false);
		rhi->SetPrimitive("bloom_up_pso", mBloomPP->fPrimitive);
 		auto bloomDownRTNum = mBloomPP->downUpNum;
 		if (i == 0)
 		{
			rhi->UploadResourceTable(0, mBloomPP->GetBloomDownRTs()[bloomDownRTNum - 1]->GetRTDesc(RTColorBuffer).rtTexture->GetSrvIndex());
			rhi->UploadResourceTable(1, mBloomPP->GetBloomDownRTs()[bloomDownRTNum - 2]->GetRTDesc(RTColorBuffer).rtTexture->GetSrvIndex());
			FVector4Int rtSize;
			rtSize.x = mBloomPP->GetBloomUpRTs()[i]->Width();
			rtSize.y = mBloomPP->GetBloomUpRTs()[i]->Height();
			rtSize.z = 20;
			rtSize.w = 20;
			rhi->UploadResourceConstants(2, 4, &rtSize, 0);
 		}
 		else
 		{
			rhi->UploadResourceTable(0, mBloomPP->GetBloomUpRTs()[i - 1]->GetRTDesc(RTColorBuffer).rtTexture->GetSrvIndex());
			rhi->UploadResourceTable(1, mBloomPP->GetBloomDownRTs()[bloomDownRTNum - 2 - i]->GetRTDesc(RTColorBuffer).rtTexture->GetSrvIndex());
			FVector4Int rtSize;
			rtSize.x = mBloomPP->GetBloomUpRTs()[i]->Width();
			rtSize.y = mBloomPP->GetBloomUpRTs()[i]->Height();
			rtSize.z = 20;
			rtSize.w = 20;
			rhi->UploadResourceConstants(2, 4, &rtSize, 0);
 		}
		rhi->DrawFPrimitive(*mBloomPP->fPrimitive);
 		rhi->TransResourBarrier(mBloomPP->GetBloomUpRTs()[i]->ColorResource(0).get(), 1, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_COMMON);
 		rhi->TransResourBarrier(mBloomPP->GetBloomUpRTs()[i]->DSResource().get(), 1, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_GENERIC_READ);
		rhi->EndPass();
	}

  	//Bloom sunmergeps
	rhi->BeginPass("BloomSunmergepsPass");
 	rhi->TransResourBarrier(mBloomPP->GetBloomSunmergepsRT()->ColorResource(0).get(), 1, RESOURCE_STATE_COMMON, RESOURCE_STATE_RENDER_TARGET);
 	rhi->TransResourBarrier(mBloomPP->GetBloomSunmergepsRT()->DSResource().get(), 1, RESOURCE_STATE_GENERIC_READ, RESOURCE_STATE_DEPTH_WRITE);
 	rhi->SetRenderTargets(1, mBloomPP->GetBloomSunmergepsRT()->Rtv(0), false, mBloomPP->GetBloomSunmergepsRT()->Dsv());
	rhi->BasePrepare(mBloomPP->GetBloomSunmergepsRT(), false);
	rhi->SetPrimitive("bloom_sunmergeps_pso", mBloomPP->fPrimitive);
	auto bloomDownRTNum = mBloomPP->downUpNum;
	rhi->UploadResourceTable(0, mBloomPP->GetBloomUpRTs()[mBloomPP->downUpNum - 2]->GetRTDesc(RTColorBuffer).rtTexture->GetSrvIndex());
	rhi->UploadResourceTable(1, mBloomPP->GetBloomSetUpRT()->GetRTDesc(RTColorBuffer).rtTexture->GetSrvIndex());
	FVector4Int rtSizeVec4;
	rtSizeVec4.x = mBloomPP->GetBloomSunmergepsRT()->Width();
	rtSizeVec4.y = mBloomPP->GetBloomSunmergepsRT()->Height();
	rtSizeVec4.z = 20;
	rtSizeVec4.w = 20;
	rhi->UploadResourceConstants(2, 4, &rtSizeVec4, 0);
	rhi->DrawFPrimitive(*mBloomPP->fPrimitive);
 	rhi->TransResourBarrier(mBloomPP->GetBloomSunmergepsRT()->ColorResource(0).get(), 1, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_COMMON);
 	rhi->TransResourBarrier(mBloomPP->GetBloomSunmergepsRT()->DSResource().get(), 1, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_GENERIC_READ);
	rhi->EndPass();

	//Blend
	if (mBlendBufferRT->currentResourceRT == -1)
	{
		BlendPass(mBlendBufferRT->mBlendRT[mBlendBufferRT->currentBlendRT], mBloomPP->GetBloomSunmergepsRT(), mSceneColorRT, "BlendBloomPass");
		mBlendBufferRT->SwapBlendRT();
	}
	else
	{
		BlendPass(mBlendBufferRT->mBlendRT[mBlendBufferRT->currentBlendRT], mBloomPP->GetBloomSunmergepsRT(), mBlendBufferRT->mBlendRT[mBlendBufferRT->currentResourceRT], "BlendBloomPass");
		mBlendBufferRT->SwapBlendRT();
	}
}

void ForwardRenderer::CyberpunkPass()
{
	//Punk Pass
	//Blend
	rhi->BeginPass("BlendCyberpunkPass");
	if (mBlendBufferRT->currentResourceRT == -1)
	{
		rhi->TransResourBarrier(mBlendBufferRT->mBlendRT[mBlendBufferRT->currentBlendRT]->ColorResource(0).get(), 1, RESOURCE_STATE_COMMON, RESOURCE_STATE_RENDER_TARGET);
		rhi->TransResourBarrier(mBlendBufferRT->mBlendRT[mBlendBufferRT->currentBlendRT]->DSResource().get(), 1, RESOURCE_STATE_GENERIC_READ, RESOURCE_STATE_DEPTH_WRITE);
		rhi->SetRenderTargets(1, mBlendBufferRT->mBlendRT[mBlendBufferRT->currentBlendRT]->Rtv(0), false, mBlendBufferRT->mBlendRT[mBlendBufferRT->currentBlendRT]->Dsv());
		rhi->BasePrepare(mBlendBufferRT->mBlendRT[mBlendBufferRT->currentBlendRT], false);
		rhi->SetPrimitive("cyberpunk_pso", mCyberpunkRT->fPrimitive);
		rhi->UploadResourceTable(0, mSceneColorRT->GetRTDesc(RTColorBuffer).rtTexture->GetSrvIndex());
		FVector2DInt rtSize;
		rtSize.x = mSceneColorRT->Width();
		rtSize.y = mSceneColorRT->Height();
		rhi->UploadResourceConstants(1, 2, &rtSize, 0);
		rhi->DrawFPrimitive(*mCyberpunkRT->fPrimitive);
		rhi->TransResourBarrier(mBlendBufferRT->mBlendRT[mBlendBufferRT->currentBlendRT]->ColorResource(0).get(), 1, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_COMMON);
		rhi->TransResourBarrier(mBlendBufferRT->mBlendRT[mBlendBufferRT->currentBlendRT]->DSResource().get(), 1, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_GENERIC_READ);
		mBlendBufferRT->SwapBlendRT();
	}
	else
	{
		rhi->TransResourBarrier(mBlendBufferRT->mBlendRT[mBlendBufferRT->currentBlendRT]->ColorResource(0).get(), 1, RESOURCE_STATE_COMMON, RESOURCE_STATE_RENDER_TARGET);
		rhi->TransResourBarrier(mBlendBufferRT->mBlendRT[mBlendBufferRT->currentBlendRT]->DSResource().get(), 1, RESOURCE_STATE_GENERIC_READ, RESOURCE_STATE_DEPTH_WRITE);
		rhi->SetRenderTargets(1, mBlendBufferRT->mBlendRT[mBlendBufferRT->currentBlendRT]->Rtv(0), false, mBlendBufferRT->mBlendRT[mBlendBufferRT->currentBlendRT]->Dsv());
		rhi->BasePrepare(mBlendBufferRT->mBlendRT[mBlendBufferRT->currentBlendRT], false);
		rhi->SetPrimitive("cyberpunk_pso", mCyberpunkRT->fPrimitive);
		rhi->UploadResourceTable(0, mBlendBufferRT->mBlendRT[mBlendBufferRT->currentResourceRT]->GetRTDesc(RTColorBuffer).rtTexture->GetSrvIndex());
		FVector2DInt rtSize;
		rtSize.x = mBlendBufferRT->mBlendRT[mBlendBufferRT->currentResourceRT]->Width();
		rtSize.y = mBlendBufferRT->mBlendRT[mBlendBufferRT->currentResourceRT]->Height();
		rhi->UploadResourceConstants(1, 2, &rtSize, 0);
		rhi->DrawFPrimitive(*mCyberpunkRT->fPrimitive);
		rhi->TransResourBarrier(mBlendBufferRT->mBlendRT[mBlendBufferRT->currentBlendRT]->ColorResource(0).get(), 1, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_COMMON);
		rhi->TransResourBarrier(mBlendBufferRT->mBlendRT[mBlendBufferRT->currentBlendRT]->DSResource().get(), 1, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_GENERIC_READ);
		mBlendBufferRT->SwapBlendRT();
	}
	rhi->EndPass();
}
