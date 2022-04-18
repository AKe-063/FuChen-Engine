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

	virtual void Init(std::shared_ptr<FShaderManager> fShaderManager, BackBufferRT& backBufferRT) = 0;
	virtual void Destroy() = 0;

	virtual VIEWPORT GetViewport() = 0;
	virtual TAGRECT GetTagRect() = 0;
	virtual std::shared_ptr<FDevice> GetDevice() = 0;
	virtual unsigned __int64 GetDepthStencilViewHandle() = 0;

	virtual void BeginRender(std::string pso) = 0;
	virtual void BeginPass(const std::string& passName) = 0;
	virtual void PrepareForRender(std::string pso) = 0;
	virtual void BasePrepare(std::shared_ptr<FRenderTarget> mRT, bool bUseRTViewPort) = 0;
	virtual void InitShadowRT(std::shared_ptr<FRenderTarget> mShadowMap) = 0;
	virtual void InitPPRT(std::shared_ptr<FRenderTarget> mPostProcess, RESOURCE_FORMAT format) = 0;
	virtual void CreateRenderTarget(std::shared_ptr<FRenderTarget>& mRT, float width, float height, bool bBackBufferRT) = 0;
	virtual void ClearRT(std::shared_ptr<FRenderTarget>& mRT) = 0;
	virtual void EndPass() = 0;
	virtual std::shared_ptr<FPrimitive> CreatePrimitiveByVerticesAndIndices(std::vector<Vertex> vertices, std::vector<std::uint16_t> indices) = 0;
	virtual void SetPrimitive(const std::string& psoName, std::shared_ptr<FPrimitive>& fPrimitive) = 0;
	virtual void DrawFPrimitive(FPrimitive& fPrimitive, std::shared_ptr<FRenderTarget> mShadowMap = nullptr, std::shared_ptr<FRenderTarget> mPPMap = nullptr) = 0;
	virtual void EndDraw(BackBufferRT& backBufferRT) = 0;
	virtual void EndPrepare() = 0;
	virtual void SetRenderTargets(unsigned int numRenderTarget, unsigned __int64 renderTargetDescriptor, bool RTsSingleHandleToDescriptorRange, unsigned __int64 DepthDescriptor) {};
	virtual void TransActorToRenderPrimitive(FActor& actor, FRenderScene& fRenderScene) = 0;
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