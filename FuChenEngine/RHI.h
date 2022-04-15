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
	virtual void PrepareForRender(std::string pso) = 0;
	virtual void BeginDraw(std::shared_ptr<FRenderTarget> mRT, std::string EventName, bool bUseRTViewPort) = 0;
	virtual void InitShadowRT(std::shared_ptr<FRenderTarget> mShadowMap) = 0;
	virtual void InitPPRT(std::shared_ptr<FRenderTarget> mPostProcess, RESOURCE_FORMAT format) = 0;
	virtual void CreateRenderTarget(std::shared_ptr<FRenderTarget>& mShadowMap, float width, float height) = 0;
	virtual void EndPass() = 0;
	virtual std::shared_ptr<FPrimitive> CreatePrimitiveByVerticesAndIndices(std::vector<Vertex> vertices, std::vector<std::uint16_t> indices) = 0;
	virtual void DrawShadow(FRenderScene& fRenderScene, std::shared_ptr<FRenderTarget> mShadowMap) = 0;
	virtual void DrawPrimitives(FRenderScene& fRenderScene, std::shared_ptr<FRenderTarget> mShadowMap, std::shared_ptr<FRenderTarget> mPPMap) = 0;
	virtual void DrawToHDR(FRenderScene& fRenderScene, std::shared_ptr<FRenderTarget> mShadowMap, std::shared_ptr<FRenderTarget> mBloom) = 0;
	virtual void DrawBloomDown(const std::string& psoName, std::shared_ptr<FRenderTarget> mPPMap = nullptr, std::shared_ptr<FRenderTarget> mRT = nullptr) = 0;
	virtual void SetPrimitive(const std::string& psoName, std::shared_ptr<FPrimitive>& fPrimitive) = 0;
	virtual void DrawBloomUp(const std::string& psoName, std::shared_ptr<FRenderTarget> mResourceRTUp = nullptr, std::shared_ptr<FRenderTarget> mRmResourceRTDown = nullptr, std::shared_ptr<FRenderTarget> mRT = nullptr) = 0;
	virtual void DrawFPrimitive(FPrimitive& fPrimitive, std::shared_ptr<FRenderTarget> mShadowMap = nullptr, std::shared_ptr<FRenderTarget> mPPMap = nullptr) = 0;
	virtual void ToneMapps(const std::string& psoName, std::shared_ptr<FPrimitive> fPrimitive, std::shared_ptr<FRenderTarget> mSceneColor = nullptr, std::shared_ptr<FRenderTarget> mSunmergeps = nullptr) = 0;
	virtual void EndDraw() = 0;
	virtual void EndPrepare() = 0;
	virtual void SetRenderTargets(unsigned int numRenderTarget, unsigned __int64 renderTargetDescriptor, bool RTsSingleHandleToDescriptorRange, unsigned __int64 DepthDescriptor) {};
	virtual void SetPipelineState(std::string pso) = 0;
	virtual void TransActorToRenderPrimitive(FActor& actor, FRenderScene& fRenderScene) = 0;
	virtual void TransCurrentBackBufferResourBarrier(unsigned int numBarriers, RESOURCE_STATES currentState, RESOURCE_STATES targetState) {};
	virtual void TransResourBarrier(FPUResource* resource, unsigned int numBarriers, RESOURCE_STATES currentState, RESOURCE_STATES targetState) {};
	virtual void TransTextureToRenderResource(FActor& actor, FTexture* texture, FRenderScene& fRenderScene) = 0;
	virtual void UpdateVP() = 0;
	virtual void UpdateM(FPrimitive& fPrimitive) = 0;
	virtual void UploadMaterialData() = 0;
	virtual void UploadResourceBuffer(UINT slot, const int index) = 0;
	virtual void UploadResourceTable(UINT slot, int dataAddress) = 0;
	virtual void UploadResourceConstants(UINT slot, UINT dataNum, const void* data, UINT offset) = 0;
	virtual void UploadResourceBuffer(INT32 slotLight, INT32 slotPass, INT32 slotCamera, INT32 slotMat) = 0;

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