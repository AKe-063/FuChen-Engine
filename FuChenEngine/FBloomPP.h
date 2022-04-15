#pragma once
#include "FRenderTarget.h"

class FBloomPP
{
public:
	FBloomPP(int32_t downUp, std::shared_ptr<FRenderTarget>& sceneColorRT);
	~FBloomPP();

	int32_t downUpNum = 0;

private:
	std::shared_ptr<FRenderTarget> mBloomSetUpRT = nullptr;
	std::vector<std::shared_ptr<FRenderTarget>> mBloomDownRT;
	std::vector<std::shared_ptr<FRenderTarget>> mBloomUpRT;
	std::shared_ptr<FRenderTarget> mBloomSunmergepsRT = nullptr;
};