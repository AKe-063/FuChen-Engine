#pragma once
#include "FRenderTarget.h"
#include "FPrimitive.h"

class FCyberpunkPP
{
public:
	FCyberpunkPP(std::shared_ptr<FRenderTarget>& sceneColorRT);
	~FCyberpunkPP();

	void InitCyberpunkRTs();

	std::shared_ptr<FRenderTarget>& GetCyberpunkRT();

	int downUpNum = 0;
	std::vector<Vertex> vertices;
	std::vector<std::uint16_t> indices = { 0,1,2 };
	std::shared_ptr<FPrimitive> fPrimitive = nullptr;

private:
	std::shared_ptr<FRenderTarget> mCyberpunkRT = nullptr;
};