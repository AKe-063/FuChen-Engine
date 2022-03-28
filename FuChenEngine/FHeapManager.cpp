#include "stdafx.h"
#include "FHeapManager.h"

FHeapManager::FHeapManager()
{
}

FHeapManager::~FHeapManager()
{
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> FHeapManager::GetComHeap()
{
	return descriptorHeap;
}

void FHeapManager::CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_DESC& managedHeapDesc)
{
	ThrowIfFailed(device->CreateDescriptorHeap(&managedHeapDesc,
		IID_PPV_ARGS(&descriptorHeap)));
}

D3D12_CPU_DESCRIPTOR_HANDLE FHeapManager::GetCPUDescriptorHandleInHeapStart()
{
	return descriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_GPU_DESCRIPTOR_HANDLE FHeapManager::GetGPUDescriptorHandleInHeapStart()
{
	return descriptorHeap->GetGPUDescriptorHandleForHeapStart();
}

int FHeapManager::GetIndex()
{
	return index;
}

void FHeapManager::AddIndex(const int count)
{
	index += count;
}

void FHeapManager::SubIndex(const int count)
{
	index -= count;
}

ID3D12DescriptorHeap* FHeapManager::GetHeap()
{
	return descriptorHeap.Get();
}
