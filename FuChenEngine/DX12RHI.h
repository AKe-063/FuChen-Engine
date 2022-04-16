#pragma once
#include "Camera.h"
#include "Win32Window.h"
#include "FScene.h"
#include "FAssetManager.h"
#include "RHI.h"
#include "DXPrimitive.h"
#include "FHeapManager.h"
#include "FPSO.h"

class DX12RHI : public RHI
{
public:
	DX12RHI();
	virtual ~DX12RHI();

	void OnResize(BackBufferRT& backBufferRT);
	virtual void Init(std::shared_ptr<FShaderManager> fShaderManager, BackBufferRT& backBufferRT)override;
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
	void BuildRootSignature(std::shared_ptr<FShaderManager> fShaderManager);
	void BuildShadersAndInputLayout(std::shared_ptr<FShaderManager> fShaderManager);
	void BuildPSO(std::shared_ptr<FShaderManager> fShaderManager, PSO_TYPE psoType);
	void BuildConstantBuffer();

	//Abstract RHI
	virtual void BeginRender(std::string pso)override;
	virtual void BeginDraw(std::shared_ptr<FRenderTarget> mRT, std::string EventName, bool bUseRTViewPort)override;
	virtual void PrepareForRender(std::string pso)override;
	virtual void InitShadowRT(std::shared_ptr<FRenderTarget> mShadowMap)override;
	virtual void InitPPRT(std::shared_ptr<FRenderTarget> mPostProcess, RESOURCE_FORMAT format)override;
	virtual void CreateRenderTarget(std::shared_ptr<FRenderTarget>& mRT, float width, float height, bool bBackBufferRT)override;
	virtual void EndPass()override;
	virtual std::shared_ptr<FPrimitive> CreatePrimitiveByVerticesAndIndices(std::vector<Vertex> vertices, std::vector<std::uint16_t> indices)override;
	virtual void SetPrimitive(const std::string& psoName, std::shared_ptr<FPrimitive>& fPrimitive)override;
	virtual void DrawFPrimitive(FPrimitive& fPrimitive, std::shared_ptr<FRenderTarget> mShadowMap = nullptr, std::shared_ptr<FRenderTarget> mPPMap = nullptr)override;
	virtual void ToneMapps(const std::string& psoName, std::shared_ptr<FPrimitive> fPrimitive, std::shared_ptr<FRenderTarget> mSceneColor = nullptr, std::shared_ptr<FRenderTarget> mSunmergeps = nullptr)override;
	virtual void EndDraw(BackBufferRT& backBufferRT)override;
	virtual void EndPrepare()override;
	virtual void SetRenderTargets(unsigned int numRenderTarget, unsigned __int64 renderTargetDescriptor, bool RTsSingleHandleToDescriptorRange, unsigned __int64 DepthDescriptor)override;
	virtual std::shared_ptr<FDevice> GetDevice()override;
	virtual VIEWPORT GetViewport()override;
	virtual TAGRECT GetTagRect()override;
	virtual unsigned __int64 GetDepthStencilViewHandle()override;
	virtual void TransActorToRenderPrimitive(FActor& actor, FRenderScene& fRenderScene)override;
	virtual void TransResourBarrier(FPUResource* resource, unsigned int numBarriers, RESOURCE_STATES currentState, RESOURCE_STATES targetState)override;
	virtual void TransTextureToRenderResource(FActor& actor, FTexture* texture, FRenderScene& fRenderScene)override;
	virtual void UpdateVP()override;
	virtual void UpdateM(FPrimitive& fPrimitive)override;
	virtual void UploadMaterialData()override;
	virtual void UploadResourceBuffer(UINT slot, const int index)override;
	virtual void UploadResourceTable(UINT slot, int dataAddress)override;
	virtual void UploadResourceConstants(UINT slot, UINT dataNum, const void* data, UINT offset)override;
	virtual void UploadResourceBuffer(INT32 slotLight, INT32 slotPass, INT32 slotCamera, INT32 slotMat)override;

protected:
	void RSSetViewPorts(unsigned int numViewports, const VIEWPORT* scrernViewport);
	void RESetScissorRects(unsigned int numRects, const TAGRECT* rect);
	void ClearDepthBuffer(unsigned __int64 handle);
	void ResetCmdListAlloc();
	void ResetCommandList(std::string pso);
	void CloseCommandList();
	void SwapChain(BackBufferRT& backBufferRT);
	void FlushCommandQueue();

	//DX Init
	bool InitDirect3D(BackBufferRT& backBufferRT);
	void CreateRtvAndDsvDescriptorHeaps();
	bool Get4xMsaaState()const;
	void CreateCommandObjects();
	void CreateSwapChain(BackBufferRT & backBufferRT);
	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;

private:
	std::string currentPso;
	std::unique_ptr<FHeapManager> mHeapManager;
	std::vector<std::shared_ptr<UploadBuffer<ObjectConstants>>> mObjectCB;
	std::unique_ptr<UploadBuffer<PassConstants>> mObjectPass;
	std::unique_ptr<UploadBuffer<LightConstants>> mObjectLight;
	std::unique_ptr<UploadBuffer<CameraConstants>> mObjectCamera;
	std::unique_ptr<UploadBuffer<MaterialConstants>> mObjectMat;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;
	std::unique_ptr<FPsoManager> mFPsoManage;
	std::unordered_map<std::wstring, ComPtr<ID3D12RootSignature>> mRootSignatures;

	// Set true to use 4X MSAA (?.1.8).  The default is false.
	bool      m4xMsaaState = false;    // 4X MSAA enabled
	UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA

	// Derived class should set these in derived constructor to customize starting values.
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;

	//DX Init Members
	bool DCShadowMap = true;
	bool nowJustNeedOnce = false;
	Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
	Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
	Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;
	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;
	UINT mRtvDescriptorSize = 0;
	UINT mDsvDescriptorSize = 0;
	UINT mCbvSrvUavDescriptorSize = 0;
	UINT64 mCurrentFence = 0;
	Window* mWindow;

	MeshGeometry triangle;
};
