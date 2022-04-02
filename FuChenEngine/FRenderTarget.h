#pragma once
#include "RHIParameterType.h"
#include "FPUResource.h"
#include "FPrimitive.h"

enum RTType
{
	RTColorBuffer = 0,
	RTDepthStencilBuffer = 1,
};

struct RTDesc
{
	RTType rtType;
	SIZE_T CPUHandlePtr;
	SIZE_T GPUHandlePtr;
	int32_t indexInColorBufferVec;
	std::shared_ptr<FRenderTexPrimitive> rtTexture = nullptr;
};

class FRenderTarget
{
public:
	virtual ~FRenderTarget() {};
	virtual void Init(unsigned int width = 2048, unsigned int height = 2048) = 0;
	virtual VIEWPORT Viewport()const = 0;
	virtual TAGRECT ScissorRect()const = 0;
	virtual SIZE_T Srv(const UINT index)const = 0;
	virtual SIZE_T Dsv()const = 0;
	virtual std::shared_ptr<FPUResource> ColorResource(const UINT index) = 0;
	virtual std::shared_ptr<FPUResource> DSResource() = 0;
	virtual void CreateRTTexture(const UINT32 index) = 0;
	virtual void AddRTResource(RTType rtType, ID3D12DescriptorHeap* heap) = 0;
	virtual void BuildRTBuffer(
		SIZE_T hCpuSrv,
		SIZE_T hGpuSrv,
		RTType rtType) = 0;
	virtual RTDesc GetRTDesc(RTType rtType, int32_t index = -1) = 0;
};

class DXRenderTarget : public FRenderTarget
{
public:
	DXRenderTarget();

	DXRenderTarget(const DXRenderTarget& rhs) = delete;
	DXRenderTarget& operator=(const DXRenderTarget& rhs) = delete;
	virtual ~DXRenderTarget() {};

	virtual void Init(unsigned int width = 2048, unsigned int height = 2048)override;
	unsigned int Width()const;
	unsigned int Height()const;
	virtual std::shared_ptr<FPUResource> ColorResource(const UINT index)override;
	virtual std::shared_ptr<FPUResource> DSResource()override;
	virtual SIZE_T Srv(const UINT index)const override;
	virtual SIZE_T Dsv()const override;

	virtual VIEWPORT Viewport()const override;
	virtual TAGRECT ScissorRect()const override;
	virtual void CreateRTTexture(const UINT32 index)override;
	virtual void AddRTResource(RTType rtType, ID3D12DescriptorHeap* heap)override;
	virtual void BuildRTBuffer(
		SIZE_T hCpuSrv,
		SIZE_T hGpuSrv,
		RTType rtType)override;
	virtual RTDesc GetRTDesc(RTType rtType, int32_t index = -1)override;

protected:
	void BuildRTBuffer(RTDesc& rtDesc);
	void BuildResource();

private:

	ID3D12Device* md3dDevice = nullptr;

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;

	UINT mWidth = 0;
	UINT mHeight = 0;
	DXGI_FORMAT mFormat = DXGI_FORMAT_R24G8_TYPELESS;

	std::vector<RTDesc> rtDesc;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> mRenderTargetColorBuffer;

	RTDesc rtDSDesc;
	Microsoft::WRL::ComPtr<ID3D12Resource> mRenderTargetDepthStencil = nullptr;
	//bool bResourceValid = false;
};
