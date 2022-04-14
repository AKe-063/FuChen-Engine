#include "stdafx.h"
#include "FRenderTarget.h"
#include "DXPrimitive.h"
#include "RHI.h"

DXRenderTarget::DXRenderTarget()
{

}

void DXRenderTarget::Init(float width /*= 2048.0f*/, float height /*= 2048.0f*/)
{
	md3dDevice = RHI::Get()->GetDevice()->fDevice;

	mWidth = (UINT)width;
	mHeight = (UINT)height;

	mViewport = { 0.0f, 0.0f, width, width, 0.0f, 1.0f };
	mScissorRect = { 0, 0, (int)width, (int)width };

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
	fResource->fPUResource = mRenderTargetColorBuffer.Get();
	return fResource;
}

std::shared_ptr<FPUResource> DXRenderTarget::DSResource()
{
	std::shared_ptr<FPUResource> fResource = std::make_shared<FPUResource>();
	fResource->fPUResource = mRenderTargetDepthStencil.Get();
	return fResource;
}

SIZE_T DXRenderTarget::Rtv(const UINT index) const
{
	return rtDesc.CPUHandlePtr;
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

void DXRenderTarget::CreateRTTexture(const UINT32 index, RTType rtType)
{
#if _DX_PLATFORM
	switch (rtType)
	{
	case RTColorBuffer:
	{
		rtDesc.rtTexture = std::make_shared<DXRenderTexPrimitive>();
		rtDesc.rtTexture->SetSrvIndex(index);
		break;
	}
	case RTDepthStencilBuffer:
	{
		rtDSDesc.rtTexture = std::make_shared<DXRenderTexPrimitive>();
		rtDSDesc.rtTexture->SetSrvIndex(index);
		break;
	}
	default:
		assert(0);
		break;
	}
#endif
}

void DXRenderTarget::BuildRTBuffer(SIZE_T hCpuHandlePtr, SIZE_T hGpuHandlePtr, RTType rtType ,RESOURCE_FORMAT format)
{
	RTDesc mRTDesc;
	switch (rtType)
	{
	case RTColorBuffer:
		mRTDesc.CPUHandlePtr = hCpuHandlePtr;
		mRTDesc.GPUHandlePtr = hGpuHandlePtr;
		mRTDesc.indexInColorBufferVec = -1;
		mRTDesc.rtType = rtType;
		rtDesc = mRTDesc;
		BuildRTBuffer(rtDesc, format);
		break;
	case RTDepthStencilBuffer:
		mRTDesc.CPUHandlePtr = hCpuHandlePtr;
		mRTDesc.GPUHandlePtr = hGpuHandlePtr;
		mRTDesc.indexInColorBufferVec = -1;
		mRTDesc.rtType = rtType;
		rtDSDesc = mRTDesc;
		BuildRTBuffer(rtDSDesc, format);
		break;
	default:
		assert(0);
		break;
	}
}

void DXRenderTarget::BuildRTBuffer(RTDesc& rtDesc, RESOURCE_FORMAT format)
{
	switch (rtDesc.rtType)
	{
	case RTColorBuffer:
	{
		D3D12_RESOURCE_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Alignment = 0;
		texDesc.Width = mWidth;
		texDesc.Height = mHeight;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = DXGI_FORMAT(format);//DXGI_FORMAT_R16G16B16A16_FLOAT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		float normalClearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		CD3DX12_CLEAR_VALUE optClear(DXGI_FORMAT(format), normalClearColor);
		ThrowIfFailed(md3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&optClear,
			IID_PPV_ARGS(&mRenderTargetColorBuffer)));

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
		rtvDesc.Format = DXGI_FORMAT(format);//DXGI_FORMAT_R16G16B16A16_FLOAT;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Texture2D.PlaneSlice = 0;
		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		handle.ptr = rtDesc.CPUHandlePtr;
		md3dDevice->CreateRenderTargetView(mRenderTargetColorBuffer.Get(), &rtvDesc, handle);
		break;
	}
	case RTDepthStencilBuffer:
	{
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
		return rtDesc;
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