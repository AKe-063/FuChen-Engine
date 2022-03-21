#pragma once
#include "RHI.h"
#include "FRenderer.h"

class ForwardRenderer : public FRenderer
{
public:
	ForwardRenderer();
	virtual ~ForwardRenderer();

	virtual void Init()override;
	virtual void Destroy()override;
	virtual void Render()override;
	virtual void BuildPrimitive(FActor& actor)override;
	virtual void Draw()override;
	virtual void BuildInitialMap()override;

private:
	std::unique_ptr<RHI> rhi;
	FRenderScene fRenderScene;
};