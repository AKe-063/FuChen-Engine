#include "stdafx.h"
#include "FPSO.h"

FPSO::FPSO()
{

}

FPSO::~FPSO()
{

}

FPsoManager::FPsoManager()
{
	psoMap.insert({ "geo_pso", PipelineState() });
	psoMap.insert({ "shadow_pso", PipelineState() });
}

FPsoManager::~FPsoManager()
{
}

void FPsoManager::CreatePso(FShader& fShader, PSO_TYPE psoType)
{
	switch (psoType)
	{
	case PSO_TYPE::GLOBAL:
	{
		BuildGlobalRenderPso(fShader);
		break;
	}
	case PSO_TYPE::SHADOWMAP:
	{
		BuildShadowMapPso(fShader);
		break;
	}
	default:
	{
		assert(0);
		break;
	}
	}
}

void FPsoManager::BuildGlobalRenderPso(FShader& fShader)
{
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayoutVec;
	PipelineState pipelineState;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	for (auto inputLayout : fShader.GetLayout())
	{
		D3D12_INPUT_ELEMENT_DESC dxInputLayout;
		dxInputLayout.SemanticName = inputLayout.SemanticName;
		dxInputLayout.SemanticIndex = inputLayout.SemanticIndex;
		dxInputLayout.Format = DXGI_FORMAT(inputLayout.Format);
		dxInputLayout.InputSlot = inputLayout.InputSlot;
		dxInputLayout.AlignedByteOffset = inputLayout.AlignedByteOffset;
		dxInputLayout.InputSlotClass = D3D12_INPUT_CLASSIFICATION(inputLayout.InputSlotClass);
		dxInputLayout.InstanceDataStepRate = inputLayout.InstanceDataStepRate;
		inputLayoutVec.push_back(dxInputLayout);
	}
	mInputLayout.insert({ "geo_pso", inputLayoutVec });
	psoDesc.InputLayout = { mInputLayout["geo_pso"].data(), (UINT)mInputLayout["geo_pso"].size()};
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(fShader.compileResult.mvsByteCode->GetBufferPointer()),
		fShader.compileResult.mvsByteCode->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(fShader.compileResult.mpsByteCode->GetBufferPointer()),
		fShader.compileResult.mpsByteCode->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FrontCounterClockwise = TRUE;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	pipelineState.psoDesc = psoDesc;
	psoMap["geo_pso"] = pipelineState;
}

void FPsoManager::BuildShadowMapPso(FShader& fShader)
{
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayoutVec;
	PipelineState pipelineState;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	for (auto inputLayout : fShader.GetLayout())
	{
		D3D12_INPUT_ELEMENT_DESC dxInputLayout;
		dxInputLayout.SemanticName = inputLayout.SemanticName;
		dxInputLayout.SemanticIndex = inputLayout.SemanticIndex;
		dxInputLayout.Format = DXGI_FORMAT(inputLayout.Format);
		dxInputLayout.InputSlot = inputLayout.InputSlot;
		dxInputLayout.AlignedByteOffset = inputLayout.AlignedByteOffset;
		dxInputLayout.InputSlotClass = D3D12_INPUT_CLASSIFICATION(inputLayout.InputSlotClass);
		dxInputLayout.InstanceDataStepRate = inputLayout.InstanceDataStepRate;
		inputLayoutVec.push_back(dxInputLayout);
	}
	mInputLayout.insert({ "shadow_pso", inputLayoutVec });
	psoDesc.InputLayout = { mInputLayout["shadow_pso"].data(), (UINT)mInputLayout["shadow_pso"].size() };
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(fShader.compileResult.mvsByteCode->GetBufferPointer()),
		fShader.compileResult.mvsByteCode->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(fShader.compileResult.mpsByteCode->GetBufferPointer()),
		fShader.compileResult.mpsByteCode->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FrontCounterClockwise = TRUE;
	psoDesc.RasterizerState.DepthBias = 10000;
	psoDesc.RasterizerState.DepthBiasClamp = 0.0f;
	psoDesc.RasterizerState.SlopeScaledDepthBias = 0.50f;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 0;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	pipelineState.psoDesc = psoDesc;
	psoMap["shadow_pso"] = pipelineState;
}
