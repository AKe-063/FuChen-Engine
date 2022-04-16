#include "stdafx.h"
#include "DX12RHI.h"
#include "Engine.h"
#include "FLight.h"
#include <pix.h>

using Microsoft::WRL::ComPtr;
using namespace std;

DX12RHI::DX12RHI()
{

}

DX12RHI::~DX12RHI()
{
	/*	OutputDebugStringA(std::to_string(mObjectCB[0].use_count()).c_str());*/

	// 	ComPtr<ID3D12DebugDevice> mDebugDevice;
	// 	md3dDevice->QueryInterface(mDebugDevice.GetAddressOf());
	// 	HRESULT hr = md3dDevice->QueryInterface(__uuidof(ID3D12DebugDevice), reinterpret_cast<void**>(mDebugDevice.GetAddressOf()));
	// 	mDebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
}

void DX12RHI::OnResize(BackBufferRT& backBufferRT)
{
	assert(md3dDevice);
	assert(mSwapChain);
	assert(mDirectCmdListAlloc);

	// Flush before changing any resources.
	FlushCommandQueue();

	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	// Release the previous resources we will be recreating.
	for (int i = 0; i < backBufferRT.SwapChainBufferCount; ++i)
		backBufferRT.mSwapChainBuffer[i]->fPUResource.Reset();
	backBufferRT.mDepthStencilBuffer->fPUResource.Reset();

		// Resize the swap chain.
	ThrowIfFailed(mSwapChain->ResizeBuffers(
		backBufferRT.SwapChainBufferCount,
		mWindow->GetWidth(), mWindow->GetHeight(),
		DXGI_FORMAT(backBufferRT.mBackBufferFormat),
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	backBufferRT.CurrentBackBufferRT = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mHeapManager->GetHeap(HeapType::RTV)->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < backBufferRT.SwapChainBufferCount; i++)
	{
		ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBufferRT.mSwapChainBuffer[i]->fPUResource)));
		md3dDevice->CreateRenderTargetView(backBufferRT.mSwapChainBuffer[i]->fPUResource.Get(), nullptr, rtvHeapHandle);
		backBufferRT.SetBackBufferHandle(rtvHeapHandle.ptr, i);
		rtvHeapHandle.Offset(1, mRtvDescriptorSize);
		mHeapManager->AddRtvHeapDescriptorIndex(1);
	}

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = mWindow->GetWidth();
	depthStencilDesc.Height = mWindow->GetHeight();
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT(backBufferRT.mDepthStencilFormat);
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(backBufferRT.mDepthStencilBuffer->fPUResource.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT(backBufferRT.mDepthStencilFormat);
	dsvDesc.Texture2D.MipSlice = 0;
	md3dDevice->CreateDepthStencilView(backBufferRT.mDepthStencilBuffer->fPUResource.Get(), &dsvDesc, DepthStencilView());
	backBufferRT.SetBackDepthStencilBufferHandle(DepthStencilView().ptr);
	mHeapManager->AddDsvHeapDescriptorIndex(1);

	// Transition the resource from its initial state to be used as a depth buffer.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(backBufferRT.mDepthStencilBuffer->fPUResource.Get(),
	D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	// Execute the resize commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until resize is complete.
	FlushCommandQueue();

	// Update the viewport transform to cover the client area.
	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(mWindow->GetWidth());
	mScreenViewport.Height = static_cast<float>(mWindow->GetHeight());
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	mScissorRect = { 0, 0, mWindow->GetWidth(), mWindow->GetHeight() };

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	FScene::GetInstance().GetCamera()->SetLens(0.25f * MathHelper::Pi, mWindow->AspectRatio(), 1.0f, 20000.0f);
}

void DX12RHI::Init(std::shared_ptr<FShaderManager> fShaderManager, BackBufferRT& backBufferRT)
{
	mWindow = Engine::GetInstance().GetWindow();
	mHeapManager = std::make_unique<FHeapManager>();
	mFPsoManage = std::make_unique<FPsoManager>();
	if (!InitDirect3D(backBufferRT))
	{
		throw("InitDX False!");
	}
	mObjectPass = std::make_unique<UploadBuffer<PassConstants>>(md3dDevice.Get(), 1, true);
	mObjectLight = std::make_unique<UploadBuffer<LightConstants>>(md3dDevice.Get(), 1, true);
	mObjectCamera = std::make_unique<UploadBuffer<CameraConstants>>(md3dDevice.Get(), 1, true);
	mObjectMat = std::make_unique<UploadBuffer<MaterialConstants>>(md3dDevice.Get(), 1, true);

	// Do the initial resize code.
	OnResize(backBufferRT);

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(GetCommandList()->Reset(GetCommandAllocator().Get(), nullptr));

	BuildDescriptorHeaps();
	BuildConstantBuffer();
	BuildShadersAndInputLayout(fShaderManager);
	BuildRootSignature(fShaderManager);
	BuildPSO(fShaderManager, PSO_TYPE::GLOBAL);
	BuildPSO(fShaderManager, PSO_TYPE::SHADOWMAP);
	BuildPSO(fShaderManager, PSO_TYPE::HDR_GLOBAL);
	BuildPSO(fShaderManager, PSO_TYPE::BLOOM_SET_UP);
	BuildPSO(fShaderManager, PSO_TYPE::BLOOM_DOWN);
	BuildPSO(fShaderManager, PSO_TYPE::BLOOM_UP);
	BuildPSO(fShaderManager, PSO_TYPE::BLOOM_SUNMERGEPS);
	BuildPSO(fShaderManager, PSO_TYPE::TONEMAPPS);

	// Execute the initialization commands.
	ThrowIfFailed(GetCommandList()->Close());
	ID3D12CommandList* cmdsLists[] = { GetCommandList().Get() };
	GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();
}

void DX12RHI::TransTextureToRenderResource(FActor& actor, FTexture* texture, FRenderScene& fRenderScene)
{
	std::shared_ptr<FRenderTexPrimitive> fRenderTex = std::make_shared<DXRenderTexPrimitive>();
	auto texDes = texture->GetDesc();
	auto tex = Texture();
	tex.Name = texDes.name;
	tex.Filename = texDes.textureFilePath;
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), tex.Filename.c_str(),
		tex.Resource, tex.UploadHeap));

	fRenderTex->SetTex(tex);
	fRenderScene.AddTextureResource(fRenderTex);
	BuildTextureResourceView(fRenderTex);
}

void DX12RHI::Destroy()
{
	if (md3dDevice != nullptr)
		FlushCommandQueue();
}

std::shared_ptr<FDevice> DX12RHI::GetDevice()
{
	std::shared_ptr<FDevice> fDevice = std::make_shared<FDevice>();
	fDevice->fDevice = md3dDevice.Get();
	return fDevice;
}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator> DX12RHI::GetCommandAllocator()
{
	return mDirectCmdListAlloc;
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> DX12RHI::GetCommandList()
{
	return mCommandList;
}

Microsoft::WRL::ComPtr<ID3D12CommandQueue> DX12RHI::GetCommandQueue()
{
	return mCommandQueue;
}

bool DX12RHI::Getm4xMsaaState()
{
	return m4xMsaaState;
}

bool DX12RHI::InitDirect3D(BackBufferRT& backBufferRT)
{
#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory)));

	// Try to create hardware device.
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&md3dDevice));

	// Fallback to WARP device.
	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&md3dDevice)));
	}

	ThrowIfFailed(md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&mFence)));

	mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDsvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mCbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = DXGI_FORMAT(backBufferRT.mBackBufferFormat);
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(md3dDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

	m4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

#ifdef _DEBUG
	LogAdapters();
#endif

	CreateCommandObjects();
	CreateSwapChain(backBufferRT);
	CreateRtvAndDsvDescriptorHeaps();

	return true;
}

void DX12RHI::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC managedHeapDesc;
	managedHeapDesc.NumDescriptors = 1024;
	managedHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	managedHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	managedHeapDesc.NodeMask = 0;
	mHeapManager->CreateDescriptorHeap(md3dDevice, managedHeapDesc, HeapType::CBV_SRV_UAV);
}

void DX12RHI::BuildTextureResourceView(std::shared_ptr<FRenderTexPrimitive> texPrimitive)
{
	Texture* texture = texPrimitive->GetTex();
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mHeapManager->GetCPUDescriptorHandleInHeapStart(HeapType::CBV_SRV_UAV));
	hDescriptor.Offset(mHeapManager->GetCurrentCSUDescriptorNum(), mCbvSrvUavDescriptorSize);
	mHeapManager->AddCSUHeapDescriptorIndex(1);
	auto tex = texture->Resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = tex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = tex->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	md3dDevice->CreateShaderResourceView(tex.Get(), &srvDesc, hDescriptor);

	texPrimitive->SetSrvIndex(mHeapManager->GetCurrentCSUDescriptorNum() - 1);
}

void DX12RHI::BuildRootSignature(std::shared_ptr<FShaderManager> fShaderManager)
{
	for (auto shaderPair : fShaderManager->GetShaderMap())
	{
		if (mRootSignatures.find(shaderPair.first) == mRootSignatures.end())
		{
			Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
			ThrowIfFailed(md3dDevice->CreateRootSignature(
				0,
				shaderPair.second.compileResult.mvsByteCode->GetBufferPointer(),
				shaderPair.second.compileResult.mvsByteCode->GetBufferSize(),
				IID_PPV_ARGS(&rootSignature)));
			mRootSignatures[shaderPair.first] = rootSignature;
		}
		fShaderManager->GetShaderMap()[shaderPair.first].RootSignature = shaderPair.first;
	}
}

void DX12RHI::BuildShadersAndInputLayout(std::shared_ptr<FShaderManager> fShaderManager)
{
	for (auto shaderPair : fShaderManager->GetShaderMap())
	{
		Microsoft::WRL::ComPtr<ID3DBlob> vsByteCode = d3dUtil::CompileShader(shaderPair.first, nullptr, "VS", "vs_5_0");
		Microsoft::WRL::ComPtr<ID3DBlob> psByteCode = d3dUtil::CompileShader(shaderPair.first, nullptr, "PS", "ps_5_0");
		ShaderCompileResult result;
		result.mvsByteCode = vsByteCode;
		result.mpsByteCode = psByteCode;
		std::vector<INPUT_ELEMENT_DESC> mInputLayout = shaderPair.second.GetLayout();
		fShaderManager->GetShaderMap()[shaderPair.first] = FShader(shaderPair.first, result, mInputLayout);
	}
}

void DX12RHI::BuildPSO(std::shared_ptr<FShaderManager> fShaderManager, PSO_TYPE psoType)
{
	switch (psoType)
	{
	case PSO_TYPE::GLOBAL:
	{
		mFPsoManage->CreatePso(fShaderManager->GetShaderMap()[L"..\\FuChenEngine\\Shaders\\color.hlsl"], psoType);
		mFPsoManage->psoMap["geo_pso"].psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
		mFPsoManage->psoMap["geo_pso"].psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
		mFPsoManage->psoMap["geo_pso"].psoDesc.pRootSignature = mRootSignatures[L"..\\FuChenEngine\\Shaders\\color.hlsl"].Get();
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&mFPsoManage->psoMap["geo_pso"].psoDesc, IID_PPV_ARGS(&mPSOs["geo_pso"])));
		break;
	}
	case PSO_TYPE::SHADOWMAP:
	{
		mFPsoManage->CreatePso(fShaderManager->GetShaderMap()[L"..\\FuChenEngine\\Shaders\\Shadows.hlsl"], psoType);
		mFPsoManage->psoMap["shadow_pso"].psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
		mFPsoManage->psoMap["shadow_pso"].psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
		mFPsoManage->psoMap["shadow_pso"].psoDesc.pRootSignature = mRootSignatures[L"..\\FuChenEngine\\Shaders\\Shadows.hlsl"].Get();
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&mFPsoManage->psoMap["shadow_pso"].psoDesc, IID_PPV_ARGS(&mPSOs["shadow_pso"])));
		break;
	}
	case PSO_TYPE::HDR_GLOBAL:
	{
		mFPsoManage->CreatePso(fShaderManager->GetShaderMap()[L"..\\FuChenEngine\\Shaders\\color.hlsl"], psoType);
		mFPsoManage->psoMap["hdr_geo_pso"].psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
		mFPsoManage->psoMap["hdr_geo_pso"].psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
		mFPsoManage->psoMap["hdr_geo_pso"].psoDesc.pRootSignature = mRootSignatures[L"..\\FuChenEngine\\Shaders\\color.hlsl"].Get();
		mFPsoManage->psoMap["hdr_geo_pso"].psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&mFPsoManage->psoMap["hdr_geo_pso"].psoDesc, IID_PPV_ARGS(&mPSOs["hdr_geo_pso"])));
		break;
	}
	case PSO_TYPE::BLOOM_SET_UP:
	{
		mFPsoManage->CreatePso(fShaderManager->GetShaderMap()[L"..\\FuChenEngine\\Shaders\\bloomsetup.hlsl"], psoType);
		mFPsoManage->psoMap["bloom_pso"].psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
		mFPsoManage->psoMap["bloom_pso"].psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
		mFPsoManage->psoMap["bloom_pso"].psoDesc.pRootSignature = mRootSignatures[L"..\\FuChenEngine\\Shaders\\bloomsetup.hlsl"].Get();
		mFPsoManage->psoMap["bloom_pso"].psoDesc.RTVFormats[0] = DXGI_FORMAT_R11G11B10_FLOAT;
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&mFPsoManage->psoMap["bloom_pso"].psoDesc, IID_PPV_ARGS(&mPSOs["bloom_pso"])));
		break;
	}
	case PSO_TYPE::BLOOM_DOWN:
	{
		mFPsoManage->CreatePso(fShaderManager->GetShaderMap()[L"..\\FuChenEngine\\Shaders\\bloomdown.hlsl"], psoType);
		mFPsoManage->psoMap["bloom_down_pso"].psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
		mFPsoManage->psoMap["bloom_down_pso"].psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
		mFPsoManage->psoMap["bloom_down_pso"].psoDesc.pRootSignature = mRootSignatures[L"..\\FuChenEngine\\Shaders\\bloomdown.hlsl"].Get();
		mFPsoManage->psoMap["bloom_down_pso"].psoDesc.RTVFormats[0] = DXGI_FORMAT_R11G11B10_FLOAT;
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&mFPsoManage->psoMap["bloom_down_pso"].psoDesc, IID_PPV_ARGS(&mPSOs["bloom_down_pso"])));
		break;
	}
	case PSO_TYPE::BLOOM_UP:
	{
		mFPsoManage->CreatePso(fShaderManager->GetShaderMap()[L"..\\FuChenEngine\\Shaders\\bloomup.hlsl"], psoType);
		mFPsoManage->psoMap["bloom_up_pso"].psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
		mFPsoManage->psoMap["bloom_up_pso"].psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
		mFPsoManage->psoMap["bloom_up_pso"].psoDesc.pRootSignature = mRootSignatures[L"..\\FuChenEngine\\Shaders\\bloomup.hlsl"].Get();
		mFPsoManage->psoMap["bloom_up_pso"].psoDesc.RTVFormats[0] = DXGI_FORMAT_R11G11B10_FLOAT;
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&mFPsoManage->psoMap["bloom_up_pso"].psoDesc, IID_PPV_ARGS(&mPSOs["bloom_up_pso"])));

		break;
	}
	case PSO_TYPE::BLOOM_SUNMERGEPS:
	{
		mFPsoManage->CreatePso(fShaderManager->GetShaderMap()[L"..\\FuChenEngine\\Shaders\\bloomsunmergeps.hlsl"], psoType);
		mFPsoManage->psoMap["bloom_sunmergeps_pso"].psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
		mFPsoManage->psoMap["bloom_sunmergeps_pso"].psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
		mFPsoManage->psoMap["bloom_sunmergeps_pso"].psoDesc.pRootSignature = mRootSignatures[L"..\\FuChenEngine\\Shaders\\bloomsunmergeps.hlsl"].Get();
		mFPsoManage->psoMap["bloom_sunmergeps_pso"].psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&mFPsoManage->psoMap["bloom_sunmergeps_pso"].psoDesc, IID_PPV_ARGS(&mPSOs["bloom_sunmergeps_pso"])));
		break;
	}
	case PSO_TYPE::TONEMAPPS:
	{
		mFPsoManage->CreatePso(fShaderManager->GetShaderMap()[L"..\\FuChenEngine\\Shaders\\tonemapps.hlsl"], psoType);
		mFPsoManage->psoMap["tonemapps_pso"].psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
		mFPsoManage->psoMap["tonemapps_pso"].psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
		mFPsoManage->psoMap["tonemapps_pso"].psoDesc.pRootSignature = mRootSignatures[L"..\\FuChenEngine\\Shaders\\tonemapps.hlsl"].Get();
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&mFPsoManage->psoMap["tonemapps_pso"].psoDesc, IID_PPV_ARGS(&mPSOs["tonemapps_pso"])));
		break;
	}
	default:
	{
		assert(0);
		break;
	}
	}
}

void DX12RHI::AddConstantBuffer(FPrimitive& fPrimitive)
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	std::shared_ptr<UploadBuffer<ObjectConstants>> objConstant = std::make_shared<UploadBuffer<ObjectConstants>>(md3dDevice.Get(), 1, true);
	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objConstant->Resource()->GetGPUVirtualAddress();

	auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mHeapManager->GetCPUDescriptorHandleInHeapStart(HeapType::CBV_SRV_UAV));
	handle.Offset(mHeapManager->GetCurrentCSUDescriptorNum(), mCbvSrvUavDescriptorSize);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	md3dDevice->CreateConstantBufferView(&cbvDesc, handle);
	mObjectCB.push_back(objConstant);
	fPrimitive.SetObjCBIndex((int)mObjectCB.size() - 1);
	mHeapManager->AddCSUHeapDescriptorIndex(1);
}

void DX12RHI::BuildConstantBuffer()
{
	//PassCB
	UINT objPCByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectPass->Resource()->GetGPUVirtualAddress();

	auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mHeapManager->GetCPUDescriptorHandleInHeapStart(HeapType::CBV_SRV_UAV));

	handle.Offset(mHeapManager->GetCurrentCSUDescriptorNum(), mCbvSrvUavDescriptorSize);
	mHeapManager->AddCSUHeapDescriptorIndex(1);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvPassDesc;
	cbvPassDesc.BufferLocation = cbAddress;
	cbvPassDesc.SizeInBytes = objPCByteSize;

	md3dDevice->CreateConstantBufferView(&cbvPassDesc, handle);

	//LightCB
	UINT objLCByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(LightConstants));

	cbAddress = mObjectLight->Resource()->GetGPUVirtualAddress();

	handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mHeapManager->GetCPUDescriptorHandleInHeapStart(HeapType::CBV_SRV_UAV));
	handle.Offset(mHeapManager->GetCurrentCSUDescriptorNum(), mCbvSrvUavDescriptorSize);
	mHeapManager->AddCSUHeapDescriptorIndex(1);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvLightDesc;
	cbvLightDesc.BufferLocation = cbAddress;
	cbvLightDesc.SizeInBytes = objLCByteSize;

	md3dDevice->CreateConstantBufferView(&cbvLightDesc, handle);

	//CameraCB
	UINT objCCByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));

	cbAddress = mObjectCamera->Resource()->GetGPUVirtualAddress();

	handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mHeapManager->GetCPUDescriptorHandleInHeapStart(HeapType::CBV_SRV_UAV));
	handle.Offset(mHeapManager->GetCurrentCSUDescriptorNum(), mCbvSrvUavDescriptorSize);
	mHeapManager->AddCSUHeapDescriptorIndex(1);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvCameraDesc;
	cbvCameraDesc.BufferLocation = cbAddress;
	cbvCameraDesc.SizeInBytes = objCCByteSize;

	md3dDevice->CreateConstantBufferView(&cbvCameraDesc, handle);
}

void DX12RHI::InitShadowRT(std::shared_ptr<FRenderTarget> mShadowMap)
{
	if (!mShadowMap->bInit)
	{
		auto dsvCpuStart = mHeapManager->GetHeap(HeapType::DSV)->GetCPUDescriptorHandleForHeapStart();
		mShadowMap->BuildRTBuffer(
			CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvCpuStart, mHeapManager->GetCurrentDsvDescriptorNum(), mDsvDescriptorSize).ptr,
			-1,
			RTDepthStencilBuffer);
		mHeapManager->AddDsvHeapDescriptorIndex(1);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		srvDesc.Texture2D.PlaneSlice = 0;

		auto srvCpuStart = mHeapManager->GetCPUDescriptorHandleInHeapStart(HeapType::CBV_SRV_UAV);
		auto srvGpuStart = mHeapManager->GetGPUDescriptorHandleInHeapStart();
		auto shadowCPUSrv = CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, (INT)mHeapManager->GetCurrentCSUDescriptorNum(), mCbvSrvUavDescriptorSize);
		mShadowMap->CreateRTTexture((UINT32)mHeapManager->GetCurrentCSUDescriptorNum(), RTDepthStencilBuffer);
		md3dDevice->CreateShaderResourceView(mShadowMap->DSResource()->fPUResource.Get(), &srvDesc, shadowCPUSrv);
		mHeapManager->AddCSUHeapDescriptorIndex(1);
		mShadowMap->bInit = true;
	}
}

void DX12RHI::InitPPRT(std::shared_ptr<FRenderTarget> mPostProcess, RESOURCE_FORMAT format)
{
	if (!mPostProcess->bInit)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mHeapManager->GetHeap(HeapType::RTV)->GetCPUDescriptorHandleForHeapStart());
		rtvHeapHandle.Offset(mHeapManager->GetCurrentRtvDescriptorNum(), mRtvDescriptorSize);
		mPostProcess->BuildRTBuffer(
			rtvHeapHandle.ptr,
			-1,
			RTType::RTColorBuffer,
			format);
		mHeapManager->AddRtvHeapDescriptorIndex(1);

		auto dsvCpuStart = mHeapManager->GetHeap(HeapType::DSV)->GetCPUDescriptorHandleForHeapStart();
		mPostProcess->BuildRTBuffer(
			CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvCpuStart, mHeapManager->GetCurrentDsvDescriptorNum(), mDsvDescriptorSize).ptr,
			-1,
			RTDepthStencilBuffer);
		mHeapManager->AddDsvHeapDescriptorIndex(1);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT(format); //DXGI_FORMAT_R16G16B16A16_FLOAT;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		srvDesc.Texture2D.PlaneSlice = 0;

		auto srvCpuStart = mHeapManager->GetCPUDescriptorHandleInHeapStart(HeapType::CBV_SRV_UAV);
		auto srvGpuStart = mHeapManager->GetGPUDescriptorHandleInHeapStart();
		auto pprtCPUSrv = CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, (INT)mHeapManager->GetCurrentCSUDescriptorNum(), mCbvSrvUavDescriptorSize);
		mPostProcess->CreateRTTexture((UINT32)mHeapManager->GetCurrentCSUDescriptorNum(), RTColorBuffer);
		md3dDevice->CreateShaderResourceView(mPostProcess->ColorResource(0)->fPUResource.Get(), &srvDesc, pprtCPUSrv);
		mHeapManager->AddCSUHeapDescriptorIndex(1);

		mPostProcess->bInit = true;
	}
}

void DX12RHI::BeginRender(std::string pso)
{
	ThrowIfFailed(mDirectCmdListAlloc->Reset());
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSOs[pso].Get()));
}

void DX12RHI::BeginDraw(std::shared_ptr<FRenderTarget> mRT, std::string EventName, bool bUseRTViewPort)
{
	PIXBeginEvent(mCommandList.Get(), 0, EventName.c_str());
	PIXBeginEvent(mCommandQueue.Get(), 0, EventName.c_str());
	ID3D12DescriptorHeap* descriptorHeaps[] = { mHeapManager->GetHeap(HeapType::CBV_SRV_UAV) };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	if (bUseRTViewPort)
	{
		RSSetViewPorts(1, &mRT->Viewport());
		RESetScissorRects(1, &mRT->ScissorRect());
	}
	else
	{
		mCommandList->RSSetViewports(1, &mScreenViewport);
		mCommandList->RSSetScissorRects(1, &mScissorRect);
	}
	float color[4] = { 1.0f,1.0f,1.0f,1.0f };
	if (mRT->ColorResource(0)->fPUResource != nullptr)
	{
		if (!mRT->bBackBuffer)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE handle;
			handle.ptr = mRT->Rtv(0);
			mCommandList->ClearRenderTargetView(handle, color, 0, nullptr);
		}
		else
		{
			D3D12_CPU_DESCRIPTOR_HANDLE handle;
			handle.ptr = mRT->mBackBufferRT->GetCurrentBackBufferHandle();
			mCommandList->ClearRenderTargetView(handle, color, 0, nullptr);
		}
	}
	if (!mRT->bBackBuffer)
	{
		ClearDepthBuffer(mRT->Dsv());
	}
	else
	{
		ClearDepthBuffer(mRT->mBackBufferRT->GetCurrentBackDepthStencilBufferHandle());
	}
}

void DX12RHI::SetPrimitive(const std::string& psoName, std::shared_ptr<FPrimitive>& fPrimitive)
{
	if (currentPso != psoName)
	{
		mCommandList->SetPipelineState(mPSOs[psoName].Get());
		mCommandList->SetGraphicsRootSignature(mFPsoManage->psoMap[psoName].psoDesc.pRootSignature);
		currentPso = psoName;
	}
	mCommandList->IASetVertexBuffers(0, 1, &fPrimitive->GetMeshGeometryInfo().VertexBufferView());
	mCommandList->IASetIndexBuffer(&fPrimitive->GetMeshGeometryInfo().IndexBufferView());
	mCommandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DX12RHI::EndDraw(BackBufferRT& backBufferRT)
{
	CloseCommandList();
	SwapChain(backBufferRT);
	FlushCommandQueue();
}

void DX12RHI::PrepareForRender(std::string pso)
{
	ThrowIfFailed(mDirectCmdListAlloc->Reset());
	if (pso == "")
	{
		ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));
	}
	else
	{
		ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSOs[pso].Get()));
	}
}

void DX12RHI::CreateRenderTarget(std::shared_ptr<FRenderTarget>& mRT, float width, float height, bool bBackBufferRT)
{
	if (bBackBufferRT)
	{
		mRT = std::make_shared<DXRenderTarget>();
		mRT->mBackBufferRT = std::make_shared<BackBufferRT>();
		mRT->bBackBuffer = true;
	}
	else
	{
		mRT = std::make_shared<DXRenderTarget>();
		mRT->Init(width, height);
	}
}

void DX12RHI::EndPass()
{
	currentPso = "";
	PIXEndEvent(mCommandList.Get());
	PIXEndEvent(mCommandQueue.Get());
}

void DX12RHI::EndPrepare()
{
	CloseCommandList();
	FlushCommandQueue();
}

void DX12RHI::ResetCmdListAlloc()
{
	ThrowIfFailed(mDirectCmdListAlloc->Reset());
}

void DX12RHI::ResetCommandList(std::string pso)
{
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSOs[pso].Get()));
}

void DX12RHI::RSSetViewPorts(unsigned int numViewports, const VIEWPORT* scrernViewport)
{
	D3D12_VIEWPORT viewport;
	viewport.Height = scrernViewport->Height;
	viewport.MaxDepth = scrernViewport->MaxDepth;
	viewport.MinDepth = scrernViewport->MinDepth;
	viewport.TopLeftX = scrernViewport->TopLeftX;
	viewport.TopLeftY = scrernViewport->TopLeftY;
	viewport.Width = scrernViewport->Width;
	mCommandList->RSSetViewports(numViewports, &viewport);
}

void DX12RHI::RESetScissorRects(unsigned int numRects, const TAGRECT* rect)
{
	D3D12_RECT tagRect;
	tagRect.bottom = rect->bottom;
	tagRect.right = rect->right;
	tagRect.left = rect->left;
	tagRect.top = rect->top;
	mCommandList->RSSetScissorRects(numRects, &tagRect);
}

void DX12RHI::TransResourBarrier(FPUResource* resource, unsigned int numBarriers, RESOURCE_STATES currentState, RESOURCE_STATES targetState)
{
	mCommandList->ResourceBarrier(numBarriers, &CD3DX12_RESOURCE_BARRIER::Transition(resource->fPUResource.Get(), D3D12_RESOURCE_STATES(currentState), D3D12_RESOURCE_STATES(targetState)));
}

unsigned __int64 DX12RHI::GetDepthStencilViewHandle()
{
	return mHeapManager->GetHeap(HeapType::DSV)->GetCPUDescriptorHandleForHeapStart().ptr;
}

void DX12RHI::ClearDepthBuffer(unsigned __int64 handle)
{
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	cpuHandle.ptr = handle;
	mCommandList->ClearDepthStencilView(cpuHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}

void DX12RHI::SetRenderTargets(unsigned int numRenderTarget, unsigned __int64 renderTargetDescriptor, bool RTsSingleHandleToDescriptorRange, unsigned __int64 DepthDescriptor)
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvDes;
	rtvDes.ptr = renderTargetDescriptor;
	D3D12_CPU_DESCRIPTOR_HANDLE dsvDes;
	dsvDes.ptr = DepthDescriptor;
	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(numRenderTarget, &rtvDes, RTsSingleHandleToDescriptorRange, &dsvDes);
}

void DX12RHI::DrawFPrimitive(FPrimitive& fPrimitive, std::shared_ptr<FRenderTarget> mShadowMap /*= nullptr*/, std::shared_ptr<FRenderTarget> mPPMap /*= nullptr*/)
{
	mCommandList->DrawIndexedInstanced(
		fPrimitive.GetMeshGeometryInfo().DrawArgs[fPrimitive.GetMeshGeometryInfo().Name].IndexCount,
		1, 0, 0, 0);
}

VIEWPORT DX12RHI::GetViewport()
{
	VIEWPORT viewport;
	viewport.Height = mScreenViewport.Height;
	viewport.MaxDepth = mScreenViewport.MaxDepth;
	viewport.MinDepth = mScreenViewport.MinDepth;
	viewport.TopLeftX = mScreenViewport.TopLeftX;
	viewport.TopLeftY = mScreenViewport.TopLeftY;
	viewport.Width = mScreenViewport.Width;
	return viewport;
}

TAGRECT DX12RHI::GetTagRect()
{
	TAGRECT tagRect;
	tagRect.bottom = mScissorRect.bottom;
	tagRect.right = mScissorRect.right;
	tagRect.left = mScissorRect.left;
	tagRect.top = mScissorRect.top;
	return tagRect;
}

void DX12RHI::UpdateVP()
{
	LightConstants lightConstant;
	lightConstant.lightOrthoVP = glm::transpose(Engine::GetInstance().GetFScene()->GetLight(0)->GetFlightDesc()->shadowTransform);
	lightConstant.lightProj = glm::transpose(Engine::GetInstance().GetFScene()->GetLight(0)->GetFlightDesc()->lightProj);
	lightConstant.lightVP = glm::transpose(Engine::GetInstance().GetFScene()->GetLight(0)->GetFlightDesc()->lightProj * Engine::GetInstance().GetFScene()->GetLight(0)->GetFlightDesc()->lightView);
	lightConstant.lightDir = Engine::GetInstance().GetFScene()->GetLight(0)->GetFlightDesc()->lightDir;
	lightConstant.density = Engine::GetInstance().GetFScene()->GetLight(0)->GetFlightDesc()->lightDensity - 1.75f;

	PassConstants passConstant;
	passConstant.InvViewProj = glm::transpose(Engine::GetInstance().GetFScene()->GetCamera()->GetProj() * Engine::GetInstance().GetFScene()->GetCamera()->GetView());

	CameraConstants cameraConstant;
	cameraConstant.cameraLoc = Engine::GetInstance().GetFScene()->GetCamera()->GetPosition();

	mObjectLight->CopyData(0, lightConstant);
	mObjectPass->CopyData(0, passConstant);
	mObjectCamera->CopyData(0, cameraConstant);
}

void DX12RHI::UpdateM(FPrimitive& fPrimitive)
{
	mObjectCB[fPrimitive.GetObjCBIndex()]->CopyData(0, fPrimitive.GetObjConstantInfo());
}

void DX12RHI::UploadMaterialData()
{
	if (!nowJustNeedOnce)
	{
		//Material
		UINT objMatCByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

		auto cbAddress = mObjectMat->Resource()->GetGPUVirtualAddress();

		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mHeapManager->GetCPUDescriptorHandleInHeapStart(HeapType::CBV_SRV_UAV));
		handle.Offset(mHeapManager->GetCurrentCSUDescriptorNum(), mCbvSrvUavDescriptorSize);
		mHeapManager->AddCSUHeapDescriptorIndex(1);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvMatDesc;
		cbvMatDesc.BufferLocation = cbAddress;
		cbvMatDesc.SizeInBytes = objMatCByteSize;

		md3dDevice->CreateConstantBufferView(&cbvMatDesc, handle);
		nowJustNeedOnce = true;
	}
	MaterialConstants matConstant;
	mObjectMat->CopyData(0, matConstant);
}

void DX12RHI::UploadResourceBuffer(UINT slot, const int index)
{
	mCommandList->SetGraphicsRootConstantBufferView(slot, mObjectCB[index]->Resource()->GetGPUVirtualAddress());
}

void DX12RHI::UploadResourceBuffer(INT32 slotLight, INT32 slotPass, INT32 slotCamera, INT32 slotMat)
{
	if (slotLight >= 0)
		mCommandList->SetGraphicsRootConstantBufferView(slotLight, mObjectLight->Resource()->GetGPUVirtualAddress());
	if (slotPass >= 0)
		mCommandList->SetGraphicsRootConstantBufferView(slotPass, mObjectPass->Resource()->GetGPUVirtualAddress());
	if (slotCamera >= 0)
		mCommandList->SetGraphicsRootConstantBufferView(slotCamera, mObjectCamera->Resource()->GetGPUVirtualAddress());
	if (slotMat >= 0)
		mCommandList->SetGraphicsRootConstantBufferView(slotMat, mObjectMat->Resource()->GetGPUVirtualAddress());
}

void DX12RHI::UploadResourceTable(UINT slot, int dataAddress)
{
	auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mHeapManager->GetGPUDescriptorHandleInHeapStart());
	handle.Offset(dataAddress, mCbvSrvUavDescriptorSize);
	mCommandList->SetGraphicsRootDescriptorTable(slot, handle);
}

void DX12RHI::UploadResourceConstants(UINT slot, UINT dataNum, const void* data, UINT offset)
{
	mCommandList->SetGraphicsRoot32BitConstants(slot, dataNum, data, offset);
}

void DX12RHI::TransActorToRenderPrimitive(FActor& actor, FRenderScene& fRenderScene)
{
	std::vector<Vertex> vertices;
	Vertex vertice;
	ActorInfo actorInfo = actor.GetActorInfo();

	for (FMeshInfoStruct fMeshInfo : actorInfo.staticMeshes)
	{
		vertices.clear();
		AssetInfo meshInfo;
		if (FAssetManager::GetInstance().AssetContrain(fMeshInfo.name))
		{
			meshInfo = FAssetManager::GetInstance().GetAssetByName(fMeshInfo.name);
		}
		else
		{
			throw(0);
		}

		std::shared_ptr<DXPrimitive> priDesc = std::make_shared<DXPrimitive>();
		actor.GetFMeshByName(fMeshInfo.name).GetMaterial().SetPSO("geo_pso");
		priDesc->SetMaterial(actor.GetFMeshByName(fMeshInfo.name).GetMaterial());
		priDesc->GetMeshGeometryInfo().Name = meshInfo.name;
		priDesc->GetMeshGeometryInfo().mMeshWorld = MathHelper::Identity4x4();

		priDesc->GetObjConstantInfo().World =
			priDesc->GetMeshGeometryInfo().mMeshWorld = glm::transpose(
				translate(priDesc->GetMeshGeometryInfo().mMeshWorld,
					vec3(fMeshInfo.transform.Translation.x,
						fMeshInfo.transform.Translation.y,
						fMeshInfo.transform.Translation.z
					)) * mat4_cast(qua<float>(
						fMeshInfo.transform.Rotation.w,
						fMeshInfo.transform.Rotation.x,
						fMeshInfo.transform.Rotation.y,
						fMeshInfo.transform.Rotation.z
						)) * scale(
							vec3(fMeshInfo.transform.Scale3D.x,
								fMeshInfo.transform.Scale3D.y,
								fMeshInfo.transform.Scale3D.z)));

		for (int i = 0; i < meshInfo.loDs[0].numVertices; i++)
		{
			vertice = meshInfo.loDs[0].vertices[i];
			vertice.SetNormal(priDesc->GetMeshGeometryInfo().mMeshWorld, meshInfo.loDs[0].normals[i]);
			vertice.SetTangentX(meshInfo.loDs[0].tangentX[i]);
			vertice.SetTangentY(meshInfo.loDs[0].tangentY[i]);
			vertice.mUVs = vec2(meshInfo.loDs[0].verticeUVs[i].x, meshInfo.loDs[0].verticeUVs[i].y);
			vertices.push_back(vertice);
		}

		std::vector<std::uint16_t> indices;
		for (int i = 0; i < meshInfo.loDs[0].numIndices; i++)
		{
			indices.push_back(meshInfo.loDs[0].indices[i]);
		}

		priDesc->GetObjConstantInfo().Roatation =
			priDesc->GetMeshGeometryInfo().Rotation =
			glm::transpose(
				glm::mat4_cast(qua<float>(
					fMeshInfo.transform.Rotation.w,
					fMeshInfo.transform.Rotation.x,
					fMeshInfo.transform.Rotation.y,
					fMeshInfo.transform.Rotation.z
					)));

		priDesc->GetObjConstantInfo().Scale =
			glm::transpose(
				scale(vec3(fMeshInfo.transform.Scale3D.x,
					fMeshInfo.transform.Scale3D.y,
					fMeshInfo.transform.Scale3D.z)));

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &priDesc->GetMeshGeometryInfo().VertexBufferCPU));
		CopyMemory(priDesc->GetMeshGeometryInfo().VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, &priDesc->GetMeshGeometryInfo().IndexBufferCPU));
		CopyMemory(priDesc->GetMeshGeometryInfo().IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		priDesc->GetMeshGeometryInfo().VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), vertices.data(), vbByteSize, priDesc->GetMeshGeometryInfo().VertexBufferUploader);

		priDesc->GetMeshGeometryInfo().IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), indices.data(), ibByteSize, priDesc->GetMeshGeometryInfo().IndexBufferUploader);

		priDesc->GetMeshGeometryInfo().VertexByteStride = sizeof(Vertex);
		priDesc->GetMeshGeometryInfo().VertexBufferByteSize = vbByteSize;
		priDesc->GetMeshGeometryInfo().IndexFormat = DXGI_FORMAT_R16_UINT;
		priDesc->GetMeshGeometryInfo().IndexBufferByteSize = ibByteSize;

		SubmeshGeometry submesh;
		submesh.IndexCount = (UINT)indices.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		priDesc->GetMeshGeometryInfo().DrawArgs[priDesc->GetMeshGeometryInfo().Name] = submesh;

		AddConstantBuffer(*priDesc);
		auto name = actorInfo.actorName.erase(actorInfo.actorName.size() - 1);
		fRenderScene.AddPrimitive(name, priDesc);

	}
}

void DX12RHI::CloseCommandList()
{
	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
}

void DX12RHI::SwapChain(BackBufferRT& backBufferRT)
{
	// swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	backBufferRT.CurrentBackBufferRT = (backBufferRT.CurrentBackBufferRT + 1) % backBufferRT.SwapChainBufferCount;
}

void DX12RHI::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = 1024;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	mHeapManager->CreateDescriptorHeap(md3dDevice, rtvHeapDesc, HeapType::RTV);


	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1024;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	mHeapManager->CreateDescriptorHeap(md3dDevice, dsvHeapDesc, HeapType::DSV);
}

bool DX12RHI::Get4xMsaaState() const
{
	return m4xMsaaState;
}

void DX12RHI::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(md3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));

	ThrowIfFailed(md3dDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(mDirectCmdListAlloc.GetAddressOf())));

	ThrowIfFailed(md3dDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		mDirectCmdListAlloc.Get(), // Associated command allocator
		nullptr,                   // Initial PipelineStateObject
		IID_PPV_ARGS(mCommandList.GetAddressOf())));

	// Start off in a closed state.  This is because the first time we refer 
	// to the command list we will Reset it, and it needs to be closed before
	// calling Reset.
	mCommandList->Close();
}

void DX12RHI::CreateSwapChain(BackBufferRT& backBufferRT)
{
	// Release the previous swapchain we will be recreating.
	mSwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = mWindow->GetWidth();
	sd.BufferDesc.Height = mWindow->GetHeight();
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT(backBufferRT.mBackBufferFormat);
	//sd.BufferDesc.Format = mBackBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = backBufferRT.SwapChainBufferCount;
	//sd.BufferCount = SwapChainBufferCount;
	sd.OutputWindow = GetActiveWindow();
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	ThrowIfFailed(mdxgiFactory->CreateSwapChain(
		mCommandQueue.Get(),
		&sd,
		mSwapChain.GetAddressOf()));
}

void DX12RHI::FlushCommandQueue()
{
	// Advance the fence value to mark commands up to this fence point.
	mCurrentFence++;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), mCurrentFence));

	// Wait until the GPU has completed commands up to this fence point.
	if (mFence->GetCompletedValue() < mCurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.  
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrentFence, eventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12RHI::DepthStencilView()const
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mHeapManager->GetCPUDescriptorHandleInHeapStart(HeapType::DSV), mHeapManager->GetCurrentDsvDescriptorNum());
	return handle;
}

std::shared_ptr<FPrimitive> DX12RHI::CreatePrimitiveByVerticesAndIndices(std::vector<Vertex> vertices, std::vector<std::uint16_t> indices)
{
	std::shared_ptr<DXPrimitive> primitive = std::make_shared<DXPrimitive>();

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &primitive->GetMeshGeometryInfo().VertexBufferCPU));
	CopyMemory(primitive->GetMeshGeometryInfo().VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &primitive->GetMeshGeometryInfo().IndexBufferCPU));
	CopyMemory(primitive->GetMeshGeometryInfo().IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	primitive->GetMeshGeometryInfo().VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, primitive->GetMeshGeometryInfo().VertexBufferUploader);

	primitive->GetMeshGeometryInfo().IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, primitive->GetMeshGeometryInfo().IndexBufferUploader);

	primitive->GetMeshGeometryInfo().VertexByteStride = sizeof(Vertex);
	primitive->GetMeshGeometryInfo().VertexBufferByteSize = vbByteSize;
	primitive->GetMeshGeometryInfo().IndexFormat = DXGI_FORMAT_R16_UINT;
	primitive->GetMeshGeometryInfo().IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;
	primitive->GetMeshGeometryInfo().Name = "primitive->GetMeshGeometryInfo()";
	primitive->GetMeshGeometryInfo().DrawArgs[primitive->GetMeshGeometryInfo().Name] = submesh;

	return primitive;
}

void DX12RHI::LogAdapters()
{
	UINT i = 0;
	IDXGIAdapter* adapter = nullptr;
	std::vector<IDXGIAdapter*> adapterList;
	while (mdxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);

		std::wstring text = L"***Adapter: ";
		text += desc.Description;
		text += L"\n";

		OutputDebugString(text.c_str());

		adapterList.push_back(adapter);

		++i;
	}

	for (size_t i = 0; i < adapterList.size(); ++i)
	{
		LogAdapterOutputs(adapterList[i]);
		ReleaseCom(adapterList[i]);
	}
}

void DX12RHI::LogAdapterOutputs(IDXGIAdapter* adapter)
{
	UINT i = 0;
	IDXGIOutput* output = nullptr;
	while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);

		std::wstring text = L"***Output: ";
		text += desc.DeviceName;
		text += L"\n";
		OutputDebugString(text.c_str());

		LogOutputDisplayModes(output, DXGI_FORMAT_R8G8B8A8_UNORM);

		ReleaseCom(output);

		++i;
	}
}

void DX12RHI::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
	UINT count = 0;
	UINT flags = 0;

	// Call with nullptr to get list count.
	output->GetDisplayModeList(format, flags, &count, nullptr);

	std::vector<DXGI_MODE_DESC> modeList(count);
	output->GetDisplayModeList(format, flags, &count, &modeList[0]);

	for (auto& x : modeList)
	{
		UINT n = x.RefreshRate.Numerator;
		UINT d = x.RefreshRate.Denominator;
		std::wstring text =
			L"Width = " + std::to_wstring(x.Width) + L" " +
			L"Height = " + std::to_wstring(x.Height) + L" " +
			L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
			L"\n";

		::OutputDebugString(text.c_str());
	}
}