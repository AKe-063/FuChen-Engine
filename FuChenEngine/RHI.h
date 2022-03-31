#pragma once
#include "RHIParameterType.h"
#include "FPrimitive.h"
#include "FActor.h"
#include "FRenderScene.h"
#include "FTexture.h"

class RHI
{
public:
	virtual ~RHI();

	static RHI* Get();
	static void CreateRHI();
	static void ReleaseRHI();

	virtual void Init() = 0;
	virtual void Destroy() = 0;

	virtual VIEWPORT GetViewport() = 0;
	virtual TAGRECT GetTagRect() = 0;

	virtual void RSSetViewPorts(unsigned int numViewports, const VIEWPORT* scrernViewport) = 0;
	virtual void RESetScissorRects(unsigned int numRects, const TAGRECT* rect) = 0;
	virtual void ClearBackBuffer(const float* color) = 0;
	virtual void ClearDepthBuffer(unsigned __int64 handle) = 0;
	virtual void SetRenderTargets(unsigned int numRenderTarget, unsigned __int64 renderTargetDescriptor, bool RTsSingleHandleToDescriptorRange, unsigned __int64 DepthDescriptor) {};
	virtual void SetGraphicsRootSignature() = 0;
	virtual void DrawFPrimitive(FPrimitive& fPrimitive) = 0;
	virtual void TransActorToRenderPrimitive(FActor& actor, FRenderScene& fRenderScene) = 0;
	virtual void SetShadowSignature(FRenderScene& fRenderScene) = 0;
	virtual void ResetCmdListAlloc() = 0;
	virtual void ResetCommandList(std::string pso) = 0;
	virtual void CloseCommandList() = 0;
	virtual void SwapChain() = 0;
	virtual void TransCurrentBackBufferResourBarrier(unsigned int numBarriers, RESOURCE_STATES currentState, RESOURCE_STATES targetState) {};
	virtual void TransShadowMapResourBarrier(unsigned int numBarriers, RESOURCE_STATES currentState, RESOURCE_STATES targetState) {};
	virtual unsigned __int64 GetShadowMapCUPHandle() = 0;
	virtual void SetPipelineState(std::string pso) = 0;
	virtual unsigned __int64 GetCurrentBackBufferViewHandle() = 0;
	virtual unsigned __int64 GetDepthStencilViewHandle() = 0;
	virtual void FlushCommandQueue() {};
	virtual VIEWPORT GetShadowMapViewport() = 0;
	virtual TAGRECT GetShadowMapTagRect() = 0;
	virtual void UpdateVP() = 0;
	virtual void UpdateM(FPrimitive& fPrimitive) = 0;
	virtual void IASetVertexBF(FPrimitive& fPrimitive) = 0;
	virtual void IASetIndexBF(FPrimitive& fPrimitive) = 0;
	virtual void IASetPriTopology(PRIMITIVE_TOPOLOGY topology) = 0;
	virtual void TransTextureToRenderResource(FActor& actor, FTexture* texture, FRenderScene& fRenderScene) = 0;

protected:
	static RHI* rhi;
};