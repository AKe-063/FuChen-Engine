#pragma once
#include "FRenderTarget.h"
#include "FPrimitive.h"

class FBlendBufferRT
{
public:
	FBlendBufferRT();
	~FBlendBufferRT();

	void InitBlendBufferRTs();
	void SwapBlendRT();
	void BlendRTsClear();

	std::shared_ptr<FRenderTarget>& GetBlendBufferRT(const int index);

	std::vector<Vertex> vertices;
	std::vector<std::uint16_t> indices = { 0,1,2 };
	std::shared_ptr<FPrimitive> fPrimitive = nullptr;
	std::unordered_map<std::string, int> fBlendPPMap;

	static const int blendRTNum = 2;
	std::shared_ptr<FRenderTarget> mBlendRT[blendRTNum];
	int currentBlendRT = 0;
	int currentResourceRT = -1;
};