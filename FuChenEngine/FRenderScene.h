#pragma once
#include "FPrimitive.h"

class FRenderScene
{
public:
	FRenderScene();
	~FRenderScene();

	void AddPrimitive(std::shared_ptr<FPrimitive> primitive);
	FPrimitive& GetPrimitive(const int& index);
	int GetNumPrimitive();

private:
	std::vector<std::shared_ptr<FPrimitive>> fPrimitives;
};