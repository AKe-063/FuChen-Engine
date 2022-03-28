#pragma once

class FHeapManager
{
public:
	FHeapManager();
	~FHeapManager();

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetComHeap();
	void CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_DESC& managedHeapDesc);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleInHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleInHeapStart();
	int GetIndex();
	void AddIndex(const int count);
	void SubIndex(const int count);
	ID3D12DescriptorHeap* GetHeap();

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	int index = 0;
};