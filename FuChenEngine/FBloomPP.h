#pragma once
#include "FRenderTarget.h"
#include "FPrimitive.h"

class FBloomPP
{
public:
	FBloomPP(int downUp, std::shared_ptr<FRenderTarget>& sceneColorRT);
	~FBloomPP();

	void InitBloomRTs();

	std::shared_ptr<FRenderTarget>& GetBloomSetUpRT();
	std::shared_ptr<FRenderTarget>& GetBloomSunmergepsRT();
	std::vector<std::shared_ptr<FRenderTarget>>& GetBloomDownRTs();
	std::vector<std::shared_ptr<FRenderTarget>>& GetBloomUpRTs();

	int downUpNum = 0;
	std::vector<Vertex> vertices;
	std::vector<std::uint16_t> indices = { 0,1,2 };
	std::shared_ptr<FPrimitive> fPrimitive = nullptr;

private:
	std::shared_ptr<FRenderTarget> mBloomSetUpRT = nullptr;
	std::vector<std::shared_ptr<FRenderTarget>> mBloomDownRT;
	std::vector<std::shared_ptr<FRenderTarget>> mBloomUpRT;
	std::shared_ptr<FRenderTarget> mBloomSunmergepsRT = nullptr;
};