#pragma once
#include "Camera.h"
#include "Win32Window.h"
#include "FScene.h"
#include "FAssetManager.h"
#include "RHI.h"
#include "DXPrimitiveDesc.h"

class DX12RHI : public RHI
{
public:
	DX12RHI();
	virtual ~DX12RHI();

	void OnResize();
	virtual void Draw()override;
	virtual void Init()override;
	virtual void Destroy()override;

	//Get Instance
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice();
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> GetCommandAllocator();
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList();
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue();
	bool Getm4xMsaaState();

	//Render Build
	void BuildDescriptorHeaps();
	void BuildConstantBuffers();
	void BuildShaderResourceView();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildGeometry();
	void BuildPSO();
	virtual void BuildInitialMap()override;
	void BuildNewTexture(const std::string& name, const std::wstring& textureFilePath);
	void BuildAllTextures();
	void InitConstantBuffers();
	void DrawPrimitive();

	void AddConstantBuffer();
	void AddGeometry();
	void AddNewBuild();

protected:
	//DX Init
	bool InitDirect3D();
	virtual void CreateRtvAndDsvDescriptorHeaps();
	bool Get4xMsaaState()const;
	//void Set4xMsaaState(bool value);
	void CreateCommandObjects();
	void CreateSwapChain();
	void FlushCommandQueue();
	ID3D12Resource* CurrentBackBuffer()const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;
	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

private:
	std::vector<DXPrimitiveDesc> mPrimitives;
	ComPtr<ID3D12DescriptorHeap> mCbvHeap;
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;
	//std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB;
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3DBlob> mvsByteCode = nullptr;
	ComPtr<ID3DBlob> mpsByteCode = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	ComPtr<ID3D12PipelineState> mPSO = nullptr;
	//std::vector<MeshGeometry> mMeshes;

	mat4x4 mWorld = MathHelper::Identity4x4();

	// Set true to use 4X MSAA (?.1.8).  The default is false.
	bool      m4xMsaaState = false;    // 4X MSAA enabled
	UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA

	// Derived class should set these in derived constructor to customize starting values.
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//DX Init Members
	Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
	Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
	Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
	UINT64 mCurrentFence = 0;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;
	static const int SwapChainBufferCount = 2;
	int mCurrBackBuffer = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;
	std::unordered_map<std::string, Texture> mTextures;
	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;
	UINT mRtvDescriptorSize = 0;
	UINT mDsvDescriptorSize = 0;
	UINT mCbvSrvUavDescriptorSize = 0;

	Window* mWindow;
};
