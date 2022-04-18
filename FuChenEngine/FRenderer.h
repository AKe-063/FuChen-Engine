#pragma once
#include "FRenderScene.h"
#include "FActor.h"
#include "FScene.h"

enum class POST_PROCESS_TYPE
{
	Bloom = 0,
	Cyberpunk = 1
};

enum class CHOSE_POST_PROCESS_MODEL
{
	BLOOM = 1,
	CYBERPUNK = 2,

	BLEND = 121,
	NONE = 0,
};

class FRenderer
{
public:
	FRenderer();
	virtual ~FRenderer();

	virtual void Init() = 0;
	virtual void Destroy() = 0;
	virtual void Render() = 0;
	virtual void BuildDirtyPrimitive(FScene& fScene) = 0;

	virtual void ChosePostProcess(CHOSE_POST_PROCESS_MODEL modelType) = 0;
};