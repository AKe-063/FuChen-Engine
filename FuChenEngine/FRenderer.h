#pragma once
#include "FRenderScene.h"
#include "FActor.h"

class FRenderer
{
public:
	FRenderer();
	virtual ~FRenderer();

	virtual void Init() = 0;
	virtual void Destroy() = 0;
	virtual void Render() = 0;
	virtual void BuildPrimitive(FActor& actor) = 0;
	virtual void Draw() = 0;
};