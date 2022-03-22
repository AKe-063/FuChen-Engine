#pragma once
#include "RHIParameterType.h"
#include "FPrimitive.h"
#include "FActor.h"
#include "FRenderScene.h"

class RHI
{
public:
	virtual ~RHI();

	static RHI* Get();
	static void CreateRHI();
	static void ReleaseRHI();

	virtual void Init() = 0;
	virtual void Destroy() = 0;
	virtual void Draw() = 0;

	virtual VIEWPORT GetViewport() = 0;
	virtual TAGRECT GetTagRect() = 0;

	virtual void BuildInitialMap() = 0;
	virtual void StartDraw() = 0;
	virtual void RSSetViewPorts(unsigned int numViewports, const VIEWPORT* scrernViewport) = 0;
	virtual void RESetScissorRects(unsigned int numRects, const TAGRECT* rect) = 0;
	virtual void ClearBackBufferAndDepthBuffer(const float* color, float depth, unsigned int stencil, unsigned int numRects) = 0;
	virtual void SetRenderTargets(unsigned int numRenderTarget) = 0;
	virtual void SetGraphicsRootSignature() = 0;
	virtual void DrawFRenderScene(FRenderScene& fRenderScene) = 0;
	virtual void EndDraw() = 0;
	virtual void CreatePrimitive(FActor& actor, FRenderScene& fRenderScene) = 0;

protected:
	static RHI* rhi;
};