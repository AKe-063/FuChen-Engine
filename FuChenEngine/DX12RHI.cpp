#include "stdafx.h"
#include "DX12RHI.h"
#include "Engine.h"
#include "FLight.h"

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

void DX12RHI::OnResize()
{
	assert(md3dDevice);
	assert(mSwapChain);
	assert(mDirectCmdListAlloc);

	// Flush before changing any resources.
	FlushCommandQueue();

	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	// Release the previous resources we will be recreating.
	for (int i = 0; i < SwapChainBufferCount; ++i)
		mSwapChainBuffer[i].Reset();
	mDepthStencilBuffer.Reset();

	// Resize the swap chain.
	ThrowIfFailed(mSwapChain->ResizeBuffers(
		SwapChainBufferCount,
		mWindow->GetWidth(), mWindow->GetHeight(),
		mBackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	mCurrBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mHeapManager->GetHeap(HeapType::RTV)->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i])));
		md3dDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, mRtvDescriptorSize);
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
	optClear.Format = mDepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = mDepthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	md3dDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

	// Transition the resource from its initial state to be used as a depth buffer.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(),
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

void DX12RHI::Init()
{
	mWindow = Engine::GetInstance().GetWindow();
	mHeapManager = std::make_unique<FHeapManager>();
	if (!InitDirect3D())
	{
		throw("InitDX False!");
	}
	mObjectPass = std::make_unique<UploadBuffer<PassConstants>>(md3dDevice.Get(), 1, true);
	mObjectLight = std::make_unique<UploadBuffer<LightConstants>>(md3dDevice.Get(), 1, true);
	
	// Do the initial resize code.
	OnResize();

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(GetCommandList()->Reset(GetCommandAllocator().Get(), nullptr));

	BuildDescriptorHeaps();
	BuildConstantBuffer();
	BuildShadersAndInputLayout();
	BuildRootSignature();
	BuildPSO();

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

bool DX12RHI::InitDirect3D()
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
	msQualityLevels.Format = mBackBufferFormat;
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
	CreateSwapChain();
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
	hDescriptor.Offset(mHeapManager->GetCurrentDescriptorNum(), mCbvSrvUavDescriptorSize);
	mHeapManager->AddIndex(1);
	auto tex = texture->Resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = tex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = tex->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	md3dDevice->CreateShaderResourceView(tex.Get(), &srvDesc, hDescriptor);

	texPrimitive->SetSrvIndex(mHeapManager->GetCurrentDescriptorNum() - 1);
}

void DX12RHI::BuildRootSignature()
{
	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		mvsByteCode->GetBufferPointer(),
		mvsByteCode->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)));

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		mvsShadowShader->GetBufferPointer(),
		mvsShadowShader->GetBufferSize(),
		IID_PPV_ARGS(&mShadowSignature)));
}

void DX12RHI::BuildShadersAndInputLayout()
{
	mvsByteCode = d3dUtil::CompileShader(L"..\\FuChenEngine\\Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	mpsByteCode = d3dUtil::CompileShader(L"..\\FuChenEngine\\Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");
	mvsShadowShader = d3dUtil::CompileShader(L"..\\FuChenEngine\\Shaders\\Shadows.hlsl", nullptr, "VS", "vs_5_0");
	mpsShadowShader = d3dUtil::CompileShader(L"..\\FuChenEngine\\Shaders\\Shadows.hlsl", nullptr, "PS", "ps_5_0");

	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "Color", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "Normal", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void DX12RHI::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	psoDesc.pRootSignature = mRootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),
		mvsByteCode->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),
		mpsByteCode->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FrontCounterClockwise = TRUE;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = mBackBufferFormat;
	psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSOs["geo_pso"])));

	//
   // PSO for shadow map pass.
   //
	D3D12_GRAPHICS_PIPELINE_STATE_DESC smapPsoDesc = psoDesc;
	smapPsoDesc.RasterizerState.DepthBias = 10000;
	smapPsoDesc.RasterizerState.DepthBiasClamp = 0.0f;
	smapPsoDesc.RasterizerState.SlopeScaledDepthBias = 0.50f;
	smapPsoDesc.pRootSignature = mShadowSignature.Get();
	smapPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mvsShadowShader->GetBufferPointer()),
		mvsShadowShader->GetBufferSize()
	};
	smapPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mpsShadowShader->GetBufferPointer()),
		mpsShadowShader->GetBufferSize()
	};

	// Shadow map pass does not have a render target.
	smapPsoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	smapPsoDesc.NumRenderTargets = 0;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&smapPsoDesc, IID_PPV_ARGS(&mPSOs["shadow_pso"])));
}

void DX12RHI::AddConstantBuffer(FPrimitive& fPrimitive)
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	std::shared_ptr<UploadBuffer<ObjectConstants>> objConstant = std::make_shared<UploadBuffer<ObjectConstants>>(md3dDevice.Get(), 1, true);
	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objConstant->Resource()->GetGPUVirtualAddress();

	auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mHeapManager->GetCPUDescriptorHandleInHeapStart(HeapType::CBV_SRV_UAV));
	handle.Offset(mHeapManager->GetCurrentDescriptorNum(), mCbvSrvUavDescriptorSize);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	md3dDevice->CreateConstantBufferView(&cbvDesc, handle);
	mObjectCB.push_back(objConstant);
	fPrimitive.SetObjCBIndex((int)mObjectCB.size()-1);
	mHeapManager->AddIndex(1);
}

void DX12RHI::BuildConstantBuffer()
{
	//PassCB
	UINT objPCByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectPass->Resource()->GetGPUVirtualAddress();

	auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mHeapManager->GetCPUDescriptorHandleInHeapStart(HeapType::CBV_SRV_UAV));

	handle.Offset(mHeapManager->GetCurrentDescriptorNum(), mCbvSrvUavDescriptorSize);
	mHeapManager->AddIndex(1);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvPassDesc;
	cbvPassDesc.BufferLocation = cbAddress;
	cbvPassDesc.SizeInBytes = objPCByteSize;

	md3dDevice->CreateConstantBufferView(&cbvPassDesc, handle);

	//LightCB
	UINT objLCByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(LightConstants));

	cbAddress = mObjectPass->Resource()->GetGPUVirtualAddress();

	handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mHeapManager->GetCPUDescriptorHandleInHeapStart(HeapType::CBV_SRV_UAV));
	handle.Offset(mHeapManager->GetCurrentDescriptorNum(), mCbvSrvUavDescriptorSize);
	mHeapManager->AddIndex(1);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvLightDesc;
	cbvLightDesc.BufferLocation = cbAddress;
	cbvLightDesc.SizeInBytes = objLCByteSize;

	md3dDevice->CreateConstantBufferView(&cbvLightDesc, handle);
}

void DX12RHI::BuildShadowRenderTex(std::shared_ptr<FRenderTarget> mShadowMap)
{
	if (mShadowMap->DSResource() != nullptr)
	{
		mShadowMap->AddRTResource(RTDepthStencilBuffer, mHeapManager->GetHeap(HeapType::CBV_SRV_UAV));
		auto dsvCpuStart = mHeapManager->GetHeap(HeapType::DSV)->GetCPUDescriptorHandleForHeapStart();
		mShadowMap->BuildRTBuffer(
			CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvCpuStart, 1, mDsvDescriptorSize).ptr,
			-1,
			RTDepthStencilBuffer);

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
		auto shadowCPUSrv = CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, (INT)mHeapManager->GetCurrentDescriptorNum(), mCbvSrvUavDescriptorSize);
		mShadowMap->CreateRTTexture((UINT32)mHeapManager->GetCurrentDescriptorNum());
		md3dDevice->CreateShaderResourceView(mShadowMap->DSResource()->fPUResource, &srvDesc, shadowCPUSrv);
		mHeapManager->AddIndex(1);
	}
}

void DX12RHI::BeginRender(std::string pso)
{
	ThrowIfFailed(mDirectCmdListAlloc->Reset());
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSOs[pso].Get()));
}

void DX12RHI::DrawShadow(FRenderScene& fRenderScene, std::shared_ptr<FRenderTarget> mShadowMap)
{
	ID3D12DescriptorHeap* descriptorHeaps[] = { mHeapManager->GetHeap(HeapType::CBV_SRV_UAV) };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	RSSetViewPorts(1, &mShadowMap->Viewport());
	RESetScissorRects(1, &mShadowMap->ScissorRect());
	ClearDepthBuffer(mShadowMap->Dsv());
	SetShadowSignature();
	for (auto primitiveMap : fRenderScene.GetAllPrimitives())
	{
		auto primitive = primitiveMap.second;
		IASetVertexBF(*primitive);
		IASetIndexBF(*primitive);
		IASetPriTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		DrawFPrimitive(*primitive);
	}
}

void DX12RHI::BeginBaseDraw()
{
	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect); 
}

void DX12RHI::DrawPrimitives(FRenderScene& fRenderScene, std::shared_ptr<FRenderTarget> mShadowMap)
{
	ID3D12DescriptorHeap* descriptorHeaps[] = { mHeapManager->GetHeap(HeapType::CBV_SRV_UAV) };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	float color[4] = { 1.0f,1.0f,1.0f,1.0f };
	ClearBackBuffer(color);
	ClearDepthBuffer(GetDepthStencilViewHandle());
	SetGraphicsRootSignature();
	for (auto primitiveMap : fRenderScene.GetAllPrimitives())
	{
		auto primitive = primitiveMap.second;
		IASetVertexBF(*primitive);
		IASetIndexBF(*primitive);
		IASetPriTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		DrawFPrimitive(*primitive, mShadowMap);
	}
}

void DX12RHI::EndDraw()
{
	CloseCommandList();
	SwapChain();
	FlushCommandQueue();
}

void DX12RHI::BeginTransSceneDataToRenderScene(std::string pso)
{
	ThrowIfFailed(mDirectCmdListAlloc->Reset());
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSOs[pso].Get()));
}

void DX12RHI::CreateRenderTarget(std::shared_ptr<FRenderTarget>& mShadowMap)
{
	mShadowMap = std::make_shared<DXRenderTarget>();
	mShadowMap->Init(2048, 2048);
}

void DX12RHI::EndTransScene()
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

void DX12RHI::TransCurrentBackBufferResourBarrier(unsigned int numBarriers, RESOURCE_STATES currentState, RESOURCE_STATES targetState)
{
	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(numBarriers, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATES(currentState), D3D12_RESOURCE_STATES(targetState)));
}

void DX12RHI::TransShadowMapResourBarrier(FPUResource* resource, unsigned int numBarriers, RESOURCE_STATES currentState, RESOURCE_STATES targetState)
{
	mCommandList->ResourceBarrier(numBarriers, &CD3DX12_RESOURCE_BARRIER::Transition(resource->fPUResource, D3D12_RESOURCE_STATES(currentState), D3D12_RESOURCE_STATES(targetState)));
}

void DX12RHI::SetPipelineState(std::string pso)
{
	mCommandList->SetPipelineState(mPSOs[pso].Get());
}

unsigned __int64 DX12RHI::GetCurrentBackBufferViewHandle()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(
		mHeapManager->GetHeap(HeapType::RTV)->GetCPUDescriptorHandleForHeapStart(),
		mCurrBackBuffer,
		mRtvDescriptorSize);
	return handle.ptr;
}

unsigned __int64 DX12RHI::GetDepthStencilViewHandle()
{
	return mHeapManager->GetHeap(HeapType::DSV)->GetCPUDescriptorHandleForHeapStart().ptr;
}

void DX12RHI::SetShadowSignature()
{
	mCommandList->SetGraphicsRootSignature(mShadowSignature.Get());
	DCShadowMap = true;
}

void DX12RHI::ClearBackBuffer(const float* color)
{
	// Clear the back buffer and depth buffer.
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), color, 0, nullptr);
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

void DX12RHI::SetGraphicsRootSignature()
{
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
	DCShadowMap = false;
}

void DX12RHI::DrawFPrimitive(FPrimitive& fPrimitive, std::shared_ptr<FRenderTarget> mShadowMap /*= nullptr*/)
{
	mCommandList->SetGraphicsRootConstantBufferView(0, mObjectCB[fPrimitive.GetObjCBIndex()]->Resource()->GetGPUVirtualAddress());
	mCommandList->SetGraphicsRootConstantBufferView(1, mObjectLight->Resource()->GetGPUVirtualAddress());
	mCommandList->SetGraphicsRootConstantBufferView(2, mObjectPass->Resource()->GetGPUVirtualAddress());

	if (!DCShadowMap)
	{
		auto mainTex = CD3DX12_GPU_DESCRIPTOR_HANDLE(mHeapManager->GetGPUDescriptorHandleInHeapStart());
		mainTex.Offset(fPrimitive.GetMainRsvIndex(), mCbvSrvUavDescriptorSize);
		auto normalTex = CD3DX12_GPU_DESCRIPTOR_HANDLE(mHeapManager->GetGPUDescriptorHandleInHeapStart());
		normalTex.Offset(fPrimitive.GetNormalRsvIndex(), mCbvSrvUavDescriptorSize);
		auto shadowTex = CD3DX12_GPU_DESCRIPTOR_HANDLE(mHeapManager->GetGPUDescriptorHandleInHeapStart());
		shadowTex.Offset(mShadowMap->GetRTDesc(RTDepthStencilBuffer).rtTexture->GetSrvIndex(), mCbvSrvUavDescriptorSize);

		mCommandList->SetGraphicsRootDescriptorTable(3, mainTex);
		mCommandList->SetGraphicsRootDescriptorTable(4, normalTex);
		mCommandList->SetGraphicsRootDescriptorTable(5, shadowTex);
	}

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

	PassConstants passConstant;
	passConstant.InvViewProj = glm::transpose(Engine::GetInstance().GetFScene()->GetCamera()->GetProj() * Engine::GetInstance().GetFScene()->GetCamera()->GetView());

	mObjectLight->CopyData(0, lightConstant);
	mObjectPass->CopyData(0, passConstant);
}

void DX12RHI::UpdateM(FPrimitive& fPrimitive)
{
	mObjectCB[fPrimitive.GetObjCBIndex()]->CopyData(0, fPrimitive.GetObjConstantInfo());
}

void DX12RHI::IASetVertexBF(FPrimitive& fPrimitive)
{
	mCommandList->IASetVertexBuffers(0, 1, &fPrimitive.GetMeshGeometryInfo().VertexBufferView());
}

void DX12RHI::IASetIndexBF(FPrimitive& fPrimitive)
{
	mCommandList->IASetIndexBuffer(&fPrimitive.GetMeshGeometryInfo().IndexBufferView());
}

void DX12RHI::IASetPriTopology(PRIMITIVE_TOPOLOGY topology)
{
	mCommandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY(topology));
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
			vertice.SetNormal(meshInfo.loDs[0].normals[i]);
			vertice.SetColor(meshInfo.loDs[0].normals[i]);
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
			glm::mat4_cast(qua<float>(
				fMeshInfo.transform.Rotation.w,
				fMeshInfo.transform.Rotation.x,
				fMeshInfo.transform.Rotation.y,
				fMeshInfo.transform.Rotation.z
				));

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
 		auto name = actorInfo.actorName.erase(actorInfo.actorName.size()-1);
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

void DX12RHI::SwapChain()
{
	// swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
}

void DX12RHI::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	mHeapManager->CreateDescriptorHeap(md3dDevice, rtvHeapDesc, HeapType::RTV);


	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 2;
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

void DX12RHI::CreateSwapChain()
{
	// Release the previous swapchain we will be recreating.
	mSwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = mWindow->GetWidth();
	sd.BufferDesc.Height = mWindow->GetHeight();
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = mBackBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SwapChainBufferCount;
	//sd.OutputWindow = mhMainWnd;
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

ID3D12Resource* DX12RHI::CurrentBackBuffer()const
{
	return mSwapChainBuffer[mCurrBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12RHI::CurrentBackBufferView()const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		mHeapManager->GetHeap(HeapType::RTV)->GetCPUDescriptorHandleForHeapStart(),
		mCurrBackBuffer,
		mRtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12RHI::DepthStencilView()const
{
	return mHeapManager->GetCPUDescriptorHandleInHeapStart(HeapType::DSV);
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

		LogOutputDisplayModes(output, mBackBufferFormat);

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