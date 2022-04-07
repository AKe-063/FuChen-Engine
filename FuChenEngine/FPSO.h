#pragma once

#include "FShader.h"

enum class PSO_TYPE
{
	GLOBAL = 0,
	SHADOWMAP = 1
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
	void BuildGlobalRenderPso(FShader& fShader);
	void BuildShadowMapPso(FShader& fShader);

private:
	//std::unordered_map<std::string, PipelineState> psoMap;
	std::unordered_map <std::string, std::vector<D3D12_INPUT_ELEMENT_DESC>> mInputLayout;
};