#include "stdafx.h"
#include "DX12RHI.h"
#include "Engine.h"

using Microsoft::WRL::ComPtr;
using namespace std;

DX12RHI::DX12RHI()
{
	
}

DX12RHI::~DX12RHI()
{

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

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
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

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
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

void DX12RHI::Draw()
{
	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(mDirectCmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSO.Get()));

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	FLOAT color[4] = { 0.0f,0.2f,0.5f,0.25f };
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), color, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	//test!!!!!!!!!!!!!!!!!!!!!
	ID3D12DescriptorHeap* descriptorHeaps1[] = { mSrvDescriptorHeap.Get() };

	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
	
	for (int i = 0; i < mMeshes.size(); i++)
	{
		mCommandList->IASetVertexBuffers(0, 1, &mMeshes[i].VertexBufferView());
		mCommandList->IASetIndexBuffer(&mMeshes[i].IndexBufferView());
		mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//mat4 worldViewProj = Engine::GetInstance().GetFScene()->GetCamera()->GetProj() * Engine::GetInstance().GetFScene()->GetCamera()->GetView() * mMeshes[i].mMeshWorld;

		// Update the constant buffer with the latest worldViewProj matrix.
		ObjectConstants objConstants;
		objConstants.Roatation = transpose(mMeshes[i].Rotation);
		objConstants.Proj = transpose(Engine::GetInstance().GetFScene()->GetCamera()->GetProj());
		objConstants.View = transpose(Engine::GetInstance().GetFScene()->GetCamera()->GetView());
		objConstants.World = transpose(mMeshes[i].mMeshWorld);
		objConstants.time = Engine::GetInstance().GetTimer()->TotalTime();
		mObjectCB->CopyData(i, objConstants);

		mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps1), descriptorHeaps1);
		auto handle1 = CD3DX12_GPU_DESCRIPTOR_HANDLE(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		handle1.Offset(2, mCbvSrvUavDescriptorSize);
		mCommandList->SetGraphicsRootDescriptorTable(1, handle1);
		handle1.Offset(2, mCbvSrvUavDescriptorSize);
		mCommandList->SetGraphicsRootDescriptorTable(2, handle1);

		mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
		handle.Offset(i, mCbvSrvUavDescriptorSize);
		mCommandList->SetGraphicsRootDescriptorTable(0, handle);
		//mCommandList->SetGraphicsRootConstantBufferView(0, );

		mCommandList->DrawIndexedInstanced(
			mMeshes[i].DrawArgs[mMeshes[i].Name].IndexCount,
			1, 0, 0, 0);
	}
	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	FlushCommandQueue();
}

void DX12RHI::Init()
{
	mWindow = Engine::GetInstance().GetWindow();
	if (!InitDirect3D())
	{
		throw("InitDX False!");
	}
	// Do the initial resize code.
	OnResize();

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(GetCommandList()->Reset(GetCommandAllocator().Get(), nullptr));

	BuildDescriptorHeaps();
	InitConstantBuffers();
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

void DX12RHI::BuildInitialMap()
{
	ThrowIfFailed(GetCommandList()->Reset(GetCommandAllocator().Get(), nullptr));

	BuildAllTextures();
	BuildShaderResourceView();
	BuildGeometry();

	ThrowIfFailed(GetCommandList()->Close());
	ID3D12CommandList* cmdsLists[] = { GetCommandList().Get() };
	GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();
}

void DX12RHI::BuildNewTexture(const std::string& name, const std::wstring& textureFilePath)
{
	auto tex = Texture();
	tex.Name = name;
	tex.Filename = textureFilePath;
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), tex.Filename.c_str(),
		tex.Resource, tex.UploadHeap));

	mTextures[tex.Name] = tex;
}

void DX12RHI::BuildAllTextures()
{
	for (auto texture : FAssetManager::GetInstance().GetTexturesFilePath())
	{
		auto tex = Texture();
		tex.Name = texture.first;
		tex.Filename = texture.second;
		ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
			mCommandList.Get(), tex.Filename.c_str(),
			tex.Resource, tex.UploadHeap));

		mTextures[tex.Name] = tex;
	}
}

void DX12RHI::Destroy()
{
	if (md3dDevice != nullptr)
		FlushCommandQueue();
}

ComPtr<ID3D12Device> DX12RHI::GetDevice()
{
	return md3dDevice;
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
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = mMeshes.size() + 1024;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&mCbvHeap)));

	//Create the SRV heap
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = mTextures.size() + 1024;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));
}

void DX12RHI::BuildConstantBuffers()
{
	 	for (int i = 0; i < mMeshes.size(); i++)
	 	{
	 		UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	 
	 		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();
	 		// Offset to the ith object constant buffer in the buffer.
	 		cbAddress += i * objCBByteSize;
	 
	 		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
	 		handle.Offset(i, mCbvSrvUavDescriptorSize);
	 
	 		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	 		cbvDesc.BufferLocation = cbAddress;
	 		cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	 
	 		md3dDevice->CreateConstantBufferView(&cbvDesc, handle);
	 	}
}

void DX12RHI::BuildShaderResourceView()
{
	int index = 0;
	for (auto texture : mTextures)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		hDescriptor.Offset(index++, mCbvSrvUavDescriptorSize);
		auto tex = mTextures[texture.first].Resource;

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = tex->GetDesc().Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = tex->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		md3dDevice->CreateShaderResourceView(tex.Get(), &srvDesc, hDescriptor);
	}
}

void DX12RHI::BuildRootSignature()
{
// 	// Root parameter can be a table, root descriptor or root constants.
// 	CD3DX12_ROOT_PARAMETER slotRootParameter[2];
// 
// 	// Create a single descriptor table of CBVs.
// 	CD3DX12_DESCRIPTOR_RANGE cbvTable;
// 	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
// 	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);
// 
// 	CD3DX12_DESCRIPTOR_RANGE texTable;
// 	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
// 	slotRootParameter[1].InitAsDescriptorTable(1, &texTable);
// 
// 	auto staticSamplers = GetStaticSamplers();
// 
// 	// A root signature is an array of root parameters.
// 	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(),
// 		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
// // 	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 0, nullptr,
// // 		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
// 
	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
// 	ComPtr<ID3DBlob> serializedRootSig = nullptr;
// 	ComPtr<ID3DBlob> errorBlob = nullptr;
// 	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
// 		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
// 
// 	if (errorBlob != nullptr)
// 	{
// 		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
// 	}
// 	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		mvsByteCode->GetBufferPointer(),
		mvsByteCode->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)));
}

void DX12RHI::BuildShadersAndInputLayout()
{
	HRESULT hr = S_OK;

	mvsByteCode = d3dUtil::CompileShader(L"..\\FuChenEngine\\Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	mpsByteCode = d3dUtil::CompileShader(L"..\\FuChenEngine\\Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "Color", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "Normal", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void DX12RHI::BuildGeometry()
{
	mMeshes.clear();
	std::vector<Vertex> vertices;
	Vertex vertice;

	for (std::pair<std::string, FActor> actor : FScene::GetInstance().GetAllActor())
	{
		for (FMeshInfoStruct fMeshInfo : actor.second.GetActorInfo().staticMeshes)
		{
			vertices.clear();
			AssetInfo meshInfo;
			if (FAssetManager::GetInstance().AssetContrain(fMeshInfo.name))
			{
				meshInfo = FAssetManager::GetInstance().GetAssetByName(fMeshInfo.name);
			}
			else
			{
				assert(0);
			}

			std::unique_ptr<MeshGeometry> mMesh = std::make_unique<MeshGeometry>();
			mMesh->Name = meshInfo.name;
			mMesh->mMeshWorld = mWorld;

			mMesh->Rotation = mat4_cast(qua<float>(
				fMeshInfo.transform.Rotation.w,
				fMeshInfo.transform.Rotation.x,
				fMeshInfo.transform.Rotation.y,
				fMeshInfo.transform.Rotation.z
				));

			mMesh->mMeshWorld = translate(
				mMesh->mMeshWorld,
				vec3(fMeshInfo.transform.Translation.x,
					fMeshInfo.transform.Translation.y,
					fMeshInfo.transform.Translation.z
				)) * mMesh->Rotation * scale(
						vec3(fMeshInfo.transform.Scale3D.x,
							fMeshInfo.transform.Scale3D.y,
							fMeshInfo.transform.Scale3D.z));

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

			const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
			const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

			ThrowIfFailed(D3DCreateBlob(vbByteSize, &mMesh->VertexBufferCPU));
			CopyMemory(mMesh->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

			ThrowIfFailed(D3DCreateBlob(ibByteSize, &mMesh->IndexBufferCPU));
			CopyMemory(mMesh->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

			mMesh->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
				mCommandList.Get(), vertices.data(), vbByteSize, mMesh->VertexBufferUploader);

			mMesh->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
				mCommandList.Get(), indices.data(), ibByteSize, mMesh->IndexBufferUploader);

			mMesh->VertexByteStride = sizeof(Vertex);
			mMesh->VertexBufferByteSize = vbByteSize;
			mMesh->IndexFormat = DXGI_FORMAT_R16_UINT;
			mMesh->IndexBufferByteSize = ibByteSize;

			SubmeshGeometry submesh;
			submesh.IndexCount = (UINT)indices.size();
			submesh.StartIndexLocation = 0;
			submesh.BaseVertexLocation = 0;

			mMesh->DrawArgs[mMesh->Name] = submesh;
			AddConstantBuffer();
			mMeshes.push_back(*mMesh.get());
		}
	}
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
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));
}

void DX12RHI::AddConstantBuffer()
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();
	// Offset to the ith object constant buffer in the buffer.
	cbAddress += mMeshes.size() * objCBByteSize;

	auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
	handle.Offset(mMeshes.size(), mCbvSrvUavDescriptorSize);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	md3dDevice->CreateConstantBufferView(&cbvDesc, handle);
}

void DX12RHI::AddGeometry()
{
	std::vector<Vertex> vertices;
	Vertex vertice;
	ActorInfo actor = (--Engine::GetInstance().GetFScene()->GetAllActor().end())->second.GetActorInfo();

	for (FMeshInfoStruct fMeshInfo : actor.staticMeshes)
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

		std::unique_ptr<MeshGeometry> mMesh = std::make_unique<MeshGeometry>();
		mMesh->Name = meshInfo.name;
		mMesh->mMeshWorld = mWorld;

		mMesh->mMeshWorld = translate(
			mMesh->mMeshWorld,
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
						fMeshInfo.transform.Scale3D.z));

		for (int i = 0; i < meshInfo.loDs[0].numVertices; i++)
		{
			vertice = meshInfo.loDs[0].vertices[i];
			vertice.SetNormal(meshInfo.loDs[0].normals[i]);
			vertice.SetColor(meshInfo.loDs[0].normals[i]);
			vertices.push_back(vertice);
		}

		std::vector<std::uint16_t> indices;
		for (int i = 0; i < meshInfo.loDs[0].numIndices; i++)
		{
			indices.push_back(meshInfo.loDs[0].indices[i]);
		}

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &mMesh->VertexBufferCPU));
		CopyMemory(mMesh->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, &mMesh->IndexBufferCPU));
		CopyMemory(mMesh->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		mMesh->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), vertices.data(), vbByteSize, mMesh->VertexBufferUploader);

		mMesh->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), indices.data(), ibByteSize, mMesh->IndexBufferUploader);

		mMesh->VertexByteStride = sizeof(Vertex);
		mMesh->VertexBufferByteSize = vbByteSize;
		mMesh->IndexFormat = DXGI_FORMAT_R16_UINT;
		mMesh->IndexBufferByteSize = ibByteSize;

		SubmeshGeometry submesh;
		submesh.IndexCount = (UINT)indices.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		mMesh->DrawArgs[mMesh->Name] = submesh;
		mMeshes.push_back(*mMesh.get());

		AddConstantBuffer();
	}
}

void DX12RHI::AddNewBuild()
{
	ThrowIfFailed(GetCommandList()->Reset(GetCommandAllocator().Get(), nullptr));

	AddGeometry();

	ThrowIfFailed(GetCommandList()->Close());
	ID3D12CommandList* cmdsLists[] = { GetCommandList().Get() };
	GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();
}

void DX12RHI::InitConstantBuffers()
{
	mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(md3dDevice.Get(), mMeshes.size() + 1024, true);
}

void DX12RHI::DrawPrimitive()
{

}

void DX12RHI::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));


	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));
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
		mRtvHeap->GetCPUDescriptorHandleForHeapStart(),
		mCurrBackBuffer,
		mRtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12RHI::DepthStencilView()const
{
	return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
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