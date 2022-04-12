#pragma once
#include "RHIParameterType.h"
#include "FPrimitive.h"
#include "FActor.h"
#include "FRenderScene.h"
#include "FTexture.h"
#include "FRenderTarget.h"
#include "FShader.h"

class FDevice;

class RHI
{
public:
	virtual ~RHI();

	static RHI* Get();
	static void CreateRHI();
	static void ReleaseRHI();

	virtual void Init(std::shared_ptr<FShaderManager> fShaderManager) = 0;
	virtual void Destroy() = 0;

	virtual VIEWPORT GetViewport() = 0;
	virtual TAGRECT GetTagRect() = 0;
	virtual std::shared_ptr<FDevice> GetDevice() = 0;
	virtual unsigned __int64 GetCurrentBackBufferViewHandle() = 0;
	virtual unsigned __int64 GetDepthStencilViewHandle() = 0;

	virtual void BeginRender(std::string pso) = 0;
	virtual void BeginTransSceneDataToRenderScene(std::string pso) = 0;
	virtual void BeginBaseDraw() = 0;
	virtual void BuildShadowRenderTex(std::shared_ptr<FRenderTarget> mShadowMap) = 0;
	virtual void BuildPPRT(std::shared_ptr<FRenderTarget> mPostProcess) = 0;
	virtual void CreateRenderTarget(std::shared_ptr<FRenderTarget>& mShadowMap, float width, float height) = 0;
	virtual void DrawShadow(FRenderScene& fRenderScene, std::shared_ptr<FRenderTarget> mShadowMap) = 0;
	virtual void DrawPrimitives(FRenderScene& fRenderScene, std::shared_ptr<FRenderTarget> mShadowMap) = 0;
	virtual void EndDraw() = 0;
	virtual void EndTransScene() = 0;
	virtual void SetRenderTargets(unsigned int numRenderTarget, unsigned __int64 renderTargetDescriptor, bool RTsSingleHandleToDescriptorRange, unsigned __int64 DepthDescriptor) {};
	virtual void SetPipelineState(std::string pso) = 0;
	virtual void TransActorToRenderPrimitive(FActor& actor, FRenderScene& fRenderScene) = 0;
	virtual void TransCurrentBackBufferResourBarrier(unsigned int numBarriers, RESOURCE_STATES currentState, RESOURCE_STATES targetState) {};
	virtual void TransShadowMapResourBarrier(FPUResource* resource, unsigned int numBarriers, RESOURCE_STATES currentState, RESOURCE_STATES targetState) {};
	virtual void TransTextureToRenderResource(FActor& actor, FTexture* texture, FRenderScene& fRenderScene) = 0;
	virtual void UpdateVP() = 0;
	virtual void UpdateM(FPrimitive& fPrimitive) = 0;
	virtual void UploadMaterialData() = 0;
	

protected:
	static RHI* rhi;
};

class FDevice
{
public:
#if _DX_PLATFORM
	ID3D12Device* fDevice;
#endif
};