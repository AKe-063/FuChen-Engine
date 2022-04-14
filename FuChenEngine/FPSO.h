#pragma once

#include "FShader.h"

enum class PSO_TYPE
{
	GLOBAL = 0,
	SHADOWMAP = 1,
	HDR_GLOBAL = 2,
	BLOOM_SET_UP = 3,
	BLOOM_DOWN = 4,
	BLOOM_UP = 5,
	BLOOM_SUNMERGEPS = 6,
	TONEMAPPS = 7
};

struct PipelineState
{
#if _DX_PLATFORM
	//Microsoft::WRL::ComPtr<ID3D12PipelineState> mPso = nullptr;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
#endif
};

class FPSO
{
public:
	FPSO();
	virtual ~FPSO();

private:
	std::string psoName;
	PipelineState pipelineState;
};

class FPsoManager
{
public:
	FPsoManager();
	~FPsoManager();

	void CreatePso(FShader& fShader, PSO_TYPE psoType);
	std::unordered_map<std::string, PipelineState> psoMap;

protected:
	void BuildGlobalRenderPso(FShader& fShader, const std::string& name);
	void BuildShadowMapPso(FShader& fShader);

private:
	std::unordered_map <std::string, std::vector<D3D12_INPUT_ELEMENT_DESC>> mInputLayout;
};