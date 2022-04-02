#pragma once
#include "RHIParameterType.h"
#include "FPUResource.h"

class ShadowMap
{
public:
	virtual ~ShadowMap() {};
	virtual void Init(unsigned int width, unsigned int height) = 0;
	virtual VIEWPORT Viewport()const = 0;
	virtual TAGRECT ScissorRect()const = 0;
	virtual SIZE_T Srv()const = 0;
	virtual SIZE_T Dsv()const = 0;
	virtual std::shared_ptr<FPUResource> Resource() = 0;
	virtual void BuildDescriptors(
		SIZE_T hCpuSrv,
		SIZE_T hGpuSrv,
		SIZE_T hCpuDsv) = 0;
};

class DXShadowMap : public ShadowMap
{
public:
	DXShadowMap();

	DXShadowMap(const DXShadowMap& rhs) = delete;
	DXShadowMap& operator=(const DXShadowMap& rhs) = delete;
	virtual ~DXShadowMap() {};

	virtual void Init(unsigned int width, unsigned int height)override;

	unsigned int Width()const;
	unsigned int Height()const;
	virtual std::shared_ptr<FPUResource> Resource()override;
	virtual SIZE_T Srv()const override;
	virtual SIZE_T Dsv()const override;

	virtual VIEWPORT Viewport()const override;
	virtual TAGRECT ScissorRect()const override;

	virtual void BuildDescriptors(
		SIZE_T hCpuSrv,
		SIZE_T hGpuSrv,
		SIZE_T hCpuDsv)override;

protected:
	void BuildDescriptors();
	void BuildResource();

private:

	ID3D12Device* md3dDevice = nullptr;

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;

	UINT mWidth = 0;
	UINT mHeight = 0;
	DXGI_FORMAT mFormat = DXGI_FORMAT_R24G8_TYPELESS;

	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mhGpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuDsv;

	Microsoft::WRL::ComPtr<ID3D12Resource> mShadowMap = nullptr;
};
