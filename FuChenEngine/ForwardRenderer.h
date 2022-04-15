#pragma once
#include "FRenderer.h"
#include "RHI.h"
#include "FRenderTarget.h"
#include "FShader.h"
#include "FBloomPP.h"

class ForwardRenderer : public FRenderer
{
public:
	ForwardRenderer();
	virtual ~ForwardRenderer();

	virtual void Init()override;
	virtual void Destroy()override;
	virtual void Render()override;
	virtual void BuildDirtyPrimitive(FScene& fScene)override;

private:
	bool testInitTextureOnce = true;
	RHI* rhi;
	FRenderScene fRenderScene;
	std::shared_ptr<FRenderTarget> mShadowMap;
	std::shared_ptr<FRenderTarget> mSceneColorRT;
	std::shared_ptr<FBloomPP> mBloomPP;
	std::shared_ptr<FShaderManager> fShaderManager;
};