#pragma once
#include "FRenderScene.h"
#include "FActor.h"
#include "FScene.h"

class FRenderer
{
public:
	FRenderer();
	virtual ~FRenderer();

	virtual void Init() = 0;
	virtual void Destroy() = 0;
	virtual void Render() = 0;
	virtual void BuildDirtyPrimitive(FScene& fScene) = 0;
	virtual void Draw(FPrimitive& fPrimitive) = 0;
};