#pragma once
#include "Camera.h"
#include "Win32Window.h"
#include "FScene.h"
#include "FAssetManager.h"
#include "RHI.h"
#include "DXPrimitive.h"
#include "FHeapManager.h"

class DX12RHI : public RHI
{
public:
	DX12RHI();
	virtual ~DX12RHI();

	void OnResize();
	virtual void Init()override;
	virtual void Destroy()override;

	//Get Instance
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> GetCommandAllocator();
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList();
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue();
	bool Getm4xMsaaState();

	//Render Build	
	void AddConstantBuffer(FPrimitive& fPrimitive);
	void BuildDescriptorHeaps();
	void BuildTextureResourceView(std::shared_ptr<FRenderTexPrimitive> texPrimitive);
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildPSO();
	void BuildConstantBuffer();

	//Abstract RHI
	virtual void BeginRender(std::string pso)override;
	virtual void BeginBaseDraw()override;
	virtual void BeginTransSceneDataToRenderScene(std::string pso)override;
	virtual void BuildShadowRenderTex(std::shared_ptr<FRenderTarget> mShadowMap)override;
	virtual void CreateRenderTarget(std::shared_ptr<FRenderTarget>& mShadowMap)override;
	virtual void DrawShadow(FRenderScene& fRenderScene, std::shared_ptr<FRenderTarget> mShadowMap)override;
	virtual void DrawPrimitives(FRenderScene& fRenderScene, std::shared_ptr<FRenderTarget> mShadowMap)override;
	virtual void EndDraw()override;
	virtual void EndTransScene()override;
	virtual void SetPipelineState(std::string pso)override;
	virtual void SetRenderTargets(unsigned int numRenderTarget, unsigned __int64 renderTargetDescriptor, bool RTsSingleHandleToDescriptorRange, unsigned __int64 DepthDescriptor)override;
	virtual std::shared_ptr<FDevice> GetDevice()override;
	virtual VIEWPORT GetViewport()override;
	virtual TAGRECT GetTagRect()override;
	virtual unsigned __int64 GetCurrentBackBufferViewHandle()override;
	virtual unsigned __int64 GetDepthStencilViewHandle()override;
	virtual void TransActorToRenderPrimitive(FActor& actor, FRenderScene& fRenderScene)override;
	virtual void TransCurrentBackBufferResourBarrier(unsigned int numBarriers, RESOURCE_STATES currentState, RESOURCE_STATES targetState)override;
	virtual void TransShadowMapResourBarrier(FPUResource* resource, unsigned int numBarriers, RESOURCE_STATES currentState, RESOURCE_STATES targetState)override;
	virtual void TransTextureToRenderResource(FActor& actor, FTexture* texture, FRenderScene& fRenderScene)override;
	virtual void UpdateVP()override;
	virtual void UpdateM(FPrimitive& fPrimitive)override;

protected:
	void DrawFPrimitive(FPrimitive& fPrimitive, std::shared_ptr<FRenderTarget> mShadowMap = nullptr);
	void RSSetViewPorts(unsigned int numViewports, const VIEWPORT* scrernViewport);
	void RESetScissorRects(unsigned int numRects, const TAGRECT* rect);
	void ClearBackBuffer(const float* color);
	void ClearDepthBuffer(unsigned __int64 handle);
	void SetGraphicsRootSignature();
	void SetShadowSignature();
	void ResetCmdListAlloc();
	void ResetCommandList(std::string pso);
	void CloseCommandList();
	void SwapChain();
	void FlushCommandQueue();
	void IASetVertexBF(FPrimitive& fPrimitive);
	void IASetIndexBF(FPrimitive& fPrimitive);
	void IASetPriTopology(PRIMITIVE_TOPOLOGY topology);

	//DX Init
	bool InitDirect3D();
	void CreateRtvAndDsvDescriptorHeaps();
	bool Get4xMsaaState()const;
	void CreateCommandObjects();
	void CreateSwapChain();
	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
	ID3D12Resource* CurrentBackBuffer()const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;

private:
	std::unique_ptr<FHeapManager> mHeapManager;
	std::vector<std::shared_ptr<UploadBuffer<ObjectConstants>>> mObjectCB;
	std::unique_ptr<UploadBuffer<PassConstants>> mObjectPass;
	std::unique_ptr<UploadBuffer<LightConstants>> mObjectLight;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> mShadowSignature = nullptr;
	ComPtr<ID3DBlob> mvsByteCode = nullptr;
	ComPtr<ID3DBlob> mpsByteCode = nullptr;
	ComPtr<ID3DBlob> mvsShadowShader = nullptr;
	ComPtr<ID3DBlob> mpsShadowShader = nullptr;

	// Set true to use 4X MSAA (?.1.8).  The default is false.
	bool      m4xMsaaState = false;    // 4X MSAA enabled
	UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA

	// Derived class should set these in derived constructor to customize starting values.
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//DX Init Members
	static const int SwapChainBufferCount = 2;
	int mCurrBackBuffer = 0;
	int flag = 0;
	bool DCShadowMap = true;
	Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
	Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
	Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;
	Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;
	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;
	UINT mRtvDescriptorSize = 0;
	UINT mDsvDescriptorSize = 0;
	UINT mCbvSrvUavDescriptorSize = 0;
	UINT64 mCurrentFence = 0;
	Window* mWindow;
};
