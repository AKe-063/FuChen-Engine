#pragma once
#include "FRenderer.h"
#include "RHI.h"
#include "FRenderTarget.h"
#include "FShader.h"
#include "FBloomPP.h"
#include "FCyberpunkPP.h"
#include "FBlendBufferRT.h"

class ForwardRenderer : public FRenderer
{
public:
	ForwardRenderer();
	virtual ~ForwardRenderer();

	virtual void Init()override;
	virtual void Destroy()override;
	virtual void Render()override;
	virtual void BuildDirtyPrimitive(FScene& fScene)override;

protected:
	void ShadowPass();
	void SceneColorPass();
	void PostProcessPass(POST_PROCESS_TYPE ppType);
	void ToneMappsPass();
	void BlendPass(std::shared_ptr<FRenderTarget>& mRT, std::shared_ptr<FRenderTarget>& mBlendResourceRT, std::shared_ptr<FRenderTarget>& mBlendBaseRT, const std::string& passName);

	//PostProcessPass
	void BloomPass();
	void CyberpunkPass();

private:
	bool testInitTextureOnce = true;
	RHI* rhi;
	FRenderScene fRenderScene;
	std::shared_ptr<FRenderTarget> mShadowMap;
	std::shared_ptr<FRenderTarget> mSceneColorRT;
	std::shared_ptr<FBloomPP> mBloomPP;
	std::shared_ptr<FShaderManager> fShaderManager;
	std::shared_ptr<FRenderTarget> mBackBufferRT;
	std::shared_ptr<FCyberpunkPP> mCyberpunkRT;
	std::shared_ptr<FBlendBufferRT> mBlendBufferRT;
};