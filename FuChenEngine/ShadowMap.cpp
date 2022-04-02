#include "stdafx.h"
#include "ShadowMap.h"
#include "RHI.h"

DXShadowMap::DXShadowMap()
{

}

void DXShadowMap::Init(unsigned int width, unsigned int height)
{
	md3dDevice = RHI::Get()->GetDevice()->fDevice;

	mWidth = width;
	mHeight = height;

	mViewport = { 0.0f, 0.0f, 2048.0f, 2048.0f, 0.0f, 1.0f };
	mScissorRect = { 0, 0, 2048, 2048 };

	BuildResource();
}

unsigned int DXShadowMap::Width()const
{
	return mWidth;
}

unsigned int DXShadowMap::Height()const
{
	return mHeight;
}

std::shared_ptr<FPUResource> DXShadowMap::Resource()
{
	std::shared_ptr<FPUResource> fResource = std::make_shared<FPUResource>();
	fResource->fPUResource = mShadowMap.Get();
	return fResource;
}

SIZE_T DXShadowMap::Srv()const
{
	return mhGpuSrv.ptr;
}

SIZE_T DXShadowMap::Dsv()const
{
	return mhCpuDsv.ptr;
}

VIEWPORT DXShadowMap::Viewport()const
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

TAGRECT DXShadowMap::ScissorRect()const
{
	TAGRECT tagRect;
	tagRect.bottom = mScissorRect.bottom;
	tagRect.top = mScissorRect.top;
	tagRect.left = mScissorRect.left;
	tagRect.right = mScissorRect.right;
	return tagRect;
}

void DXShadowMap::BuildDescriptors(SIZE_T hCpuSrv,
	SIZE_T hGpuSrv,
	SIZE_T hCpuDsv)
{
	// Save references to the descriptors. 
	mhCpuSrv.ptr = hCpuSrv;
	mhGpuSrv.ptr = hGpuSrv;
	mhCpuDsv.ptr = hCpuDsv;

	//  Create the descriptors
	BuildDescriptors();
}

void DXShadowMap::BuildDescriptors()
{
	// Create SRV to resource so we can sample the shadow map in a shader program.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;
	md3dDevice->CreateShaderResourceView(mShadowMap.Get(), &srvDesc, mhCpuSrv);

	// Create DSV to resource so we can render to the shadow map.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	md3dDevice->CreateDepthStencilView(mShadowMap.Get(), &dsvDesc, mhCpuDsv);
}

void DXShadowMap::BuildResource()
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
		IID_PPV_ARGS(&mShadowMap)));
}