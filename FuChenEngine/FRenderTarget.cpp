#include "stdafx.h"
#include "FRenderTarget.h"
#include "DXPrimitive.h"
#include "RHI.h"

DXRenderTarget::DXRenderTarget()
{

}

void DXRenderTarget::Init(unsigned int width, unsigned int height)
{
	md3dDevice = RHI::Get()->GetDevice()->fDevice;

	mWidth = width;
	mHeight = height;

	mViewport = { 0.0f, 0.0f, 2048.0f, 2048.0f, 0.0f, 1.0f };
	mScissorRect = { 0, 0, 2048, 2048 };

	//BuildResource();
}

unsigned int DXRenderTarget::Width()const
{
	return mWidth;
}

unsigned int DXRenderTarget::Height()const
{
	return mHeight;
}

std::shared_ptr<FPUResource> DXRenderTarget::ColorResource(const UINT index)
{
	std::shared_ptr<FPUResource> fResource = std::make_shared<FPUResource>();
	fResource->fPUResource = mRenderTargetColorBuffer[index].Get();
	return fResource;
}

std::shared_ptr<FPUResource> DXRenderTarget::DSResource()
{
	std::shared_ptr<FPUResource> fResource = std::make_shared<FPUResource>();
	fResource->fPUResource = mRenderTargetDepthStencil.Get();
	return fResource;
}

SIZE_T DXRenderTarget::Srv(const UINT index)const
{
	return rtDesc[index].CPUHandlePtr;
}

SIZE_T DXRenderTarget::Dsv()const
{
	return rtDSDesc.CPUHandlePtr;
}

VIEWPORT DXRenderTarget::Viewport()const
{
	VIEWPORT thisViewport;
	thisViewport.Height = mViewport.Height;
	thisViewport.MaxDepth = mViewport.MaxDepth;
	thisViewport.MinDepth = mViewport.MinDepth;
	thisViewport.TopLeftX = mViewport.TopLeftX;
	thisViewport.TopLeftY = mViewport.TopLeftY;
	thisViewport.Width = mViewport.Width;
	return thisViewport;
}

TAGRECT DXRenderTarget::ScissorRect()const
{
	TAGRECT tagRect;
	tagRect.bottom = mScissorRect.bottom;
	tagRect.top = mScissorRect.top;
	tagRect.left = mScissorRect.left;
	tagRect.right = mScissorRect.right;
	return tagRect;
}

void DXRenderTarget::CreateRTTexture(const UINT32 index)
{
#if _DX_PLATFORM
	rtDSDesc.rtTexture = std::make_shared<DXRenderTexPrimitive>();
	rtDSDesc.rtTexture->SetSrvIndex(index);
#endif
}

void DXRenderTarget::AddRTResource(RTType rtType, ID3D12DescriptorHeap* heap)
{
	switch (rtType)
	{
	case RTColorBuffer:
		// 		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		// 		md3dDevice->CreateRenderTargetView(resource.Get(),nullptr,heap)
		break;
	case RTDepthStencilBuffer:
		D3D12_RESOURCE_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Alignment = 0;
		texDesc.Width = mWidth;
		texDesc.Height = mHeight;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = mFormat;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;

		ThrowIfFailed(md3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			&optClear,
			IID_PPV_ARGS(&mRenderTargetDepthStencil)));
		break;
	default:
		assert(0);
		break;
	}
}

void DXRenderTarget::BuildRTBuffer(SIZE_T hCpuHandlePtr,
	SIZE_T hGpuHandlePtr, RTType rtType)
{
	RTDesc rtDesc;
	switch (rtType)
	{
	case RTColorBuffer:
		rtDesc.CPUHandlePtr = hCpuHandlePtr;
		rtDesc.GPUHandlePtr = hGpuHandlePtr;
		rtDesc.indexInColorBufferVec = (int32_t)mRenderTargetColorBuffer.size();
		rtDesc.rtType = rtType;
		this->rtDesc.push_back(rtDesc);
		BuildRTBuffer(this->rtDesc[rtDesc.indexInColorBufferVec]);
		break;
	case RTDepthStencilBuffer:
		rtDesc.CPUHandlePtr = hCpuHandlePtr;
		rtDesc.GPUHandlePtr = hGpuHandlePtr;
		rtDesc.indexInColorBufferVec = -1;
		rtDesc.rtType = rtType;
		rtDSDesc = rtDesc;
		BuildRTBuffer(rtDSDesc);
		break;
	default:
		assert(0);
		break;
	}
}

void DXRenderTarget::BuildRTBuffer(RTDesc& rtDesc)
{
	switch (rtDesc.rtType)
	{
	case RTColorBuffer:
	{
// 		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
// 		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
// 		srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
// 		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
// 		srvDesc.Texture2D.MostDetailedMip = 0;
// 		srvDesc.Texture2D.MipLevels = 1;
// 		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
// 		srvDesc.Texture2D.PlaneSlice = 0;
// 		CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle;
// 		srvHandle.ptr = rtDesc.CPUHandlePtr;
// 		md3dDevice->CreateShaderResourceView(mRenderTargetColorBuffer[rtDesc.indexInColorBufferVec].Get(), &srvDesc, srvHandle);
		break;
	}


	case RTDepthStencilBuffer:
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.Texture2D.MipSlice = 0;
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
		dsvHandle.ptr = rtDesc.CPUHandlePtr;
		md3dDevice->CreateDepthStencilView(mRenderTargetDepthStencil.Get(), &dsvDesc, dsvHandle);
		break;
	}

	default:
	{
		assert(0);
		break;
	}
	}
}

RTDesc DXRenderTarget::GetRTDesc(RTType rtType, int32_t index /*= -1*/)
{
	switch (rtType)
	{
	case RTColorBuffer:
	{
		return rtDesc[index];
	}
	case RTDepthStencilBuffer:
	{
		return rtDSDesc;
	}
	default:
	{
		assert(0);
		break;
	}
	}
	RTDesc mNULL;
	return mNULL;
}

void DXRenderTarget::BuildResource()
{
	// 	D3D12_RESOURCE_DESC texDesc;
	// 	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	// 	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	// 	texDesc.Alignment = 0;
	// 	texDesc.Width = mWidth;
	// 	texDesc.Height = mHeight;
	// 	texDesc.DepthOrArraySize = 1;
	// 	texDesc.MipLevels = 1;
	// 	texDesc.Format = mFormat;
	// 	texDesc.SampleDesc.Count = 1;
	// 	texDesc.SampleDesc.Quality = 0;
	// 	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	// 	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	// 
	// 	D3D12_CLEAR_VALUE optClear;
	// 	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 	optClear.DepthStencil.Depth = 1.0f;
	// 	optClear.DepthStencil.Stencil = 0;
	// 
	// 	ThrowIfFailed(md3dDevice->CreateCommittedResource(
	// 		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	// 		D3D12_HEAP_FLAG_NONE,
	// 		&texDesc,
	// 		D3D12_RESOURCE_STATE_GENERIC_READ,
	// 		&optClear,
	// 		IID_PPV_ARGS(&mShadowMap)));
}