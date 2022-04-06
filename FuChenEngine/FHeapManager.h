#pragma once

enum class HeapType
{
	CBV_SRV_UAV = 0,
	RTV = 1,
	DSV = 2
};

class FHeapManager
{
public:
	FHeapManager();
	~FHeapManager();

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetComHeap(HeapType heapType);
	void CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_DESC& managedHeapDesc, HeapType heapType);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleInHeapStart(HeapType heapType);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleInHeapStart();
	int GetCurrentDescriptorNum();
	void AddIndex(const int count);
	void SubIndex(const int count);
	ID3D12DescriptorHeap* GetHeap(HeapType heapType);

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCbvSrvUavHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap = nullptr;
	int currentCSUDescriptorNum = 0;
};