#pragma once
#include "RHIParameterType.h"

class RHI
{
public:
	virtual ~RHI();
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
	virtual void DrawPrimitive() = 0;
	virtual void EndDraw() = 0;
// 
// protected:
// 	VIEWPORT viewport;
// 	TAGRECT tagRect;

};