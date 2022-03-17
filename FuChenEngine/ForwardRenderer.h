#pragma once
#include "RHI.h"

class ForwardRenderer
{
public:
	ForwardRenderer();
	~ForwardRenderer();

	void Init();
	void Destroy();
	void Draw();
	void BuildInitialMap();

private:
	std::unique_ptr<RHI> rhi;
};