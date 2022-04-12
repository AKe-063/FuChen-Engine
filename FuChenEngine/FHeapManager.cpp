#include "stdafx.h"
#include "FHeapManager.h"

FHeapManager::FHeapManager()
{
}

FHeapManager::~FHeapManager()
{
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> FHeapManager::GetComHeap(HeapType heapType)
{
	switch (heapType)
	{
	case HeapType::CBV_SRV_UAV:
		return mCbvSrvUavHeap;
	case HeapType::RTV:
		return mRtvHeap;
	case HeapType::DSV:
		return mDsvHeap;
	default:
	{
		assert(0);
		break;
	}
	}
	return nullptr;
}

void FHeapManager::CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_DESC& managedHeapDesc, HeapType heapType)
{
	switch (heapType)
	{
	case HeapType::CBV_SRV_UAV:
	{
		ThrowIfFailed(device->CreateDescriptorHeap(&managedHeapDesc,
			IID_PPV_ARGS(&mCbvSrvUavHeap)));
		break;
	}
	case HeapType::RTV:
	{
		ThrowIfFailed(device->CreateDescriptorHeap(&managedHeapDesc,
			IID_PPV_ARGS(&mRtvHeap)));
		break;
	}
	case HeapType::DSV:
	{
		ThrowIfFailed(device->CreateDescriptorHeap(&managedHeapDesc,
			IID_PPV_ARGS(&mDsvHeap)));
		break;
	}
	{
		assert(0);
		break;
	}
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE FHeapManager::GetCPUDescriptorHandleInHeapStart(HeapType heapType)
{
	switch (heapType)
	{
	case HeapType::CBV_SRV_UAV:
	{
		return mCbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart();
		break;
	}
	case HeapType::RTV:
	{
		return mRtvHeap->GetCPUDescriptorHandleForHeapStart();
		break;
	}
	case HeapType::DSV:
	{
		return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
		break;
	}
	{
		assert(0);
		break;
	}
	}
	D3D12_CPU_DESCRIPTOR_HANDLE nullPtr;
	nullPtr.ptr = 0;
	return nullPtr;
}

D3D12_GPU_DESCRIPTOR_HANDLE FHeapManager::GetGPUDescriptorHandleInHeapStart()
{
	return mCbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart();
}

int FHeapManager::GetCurrentCSUDescriptorNum()
{
	return currentCSUDescriptorNum;
}

int FHeapManager::GetCurrentRtvDescriptorNum()
{
	return currentRtvDescriptorNum;
}

int FHeapManager::GetCurrentDsvDescriptorNum()
{
	return currentDsvDescriptorNum;
}

void FHeapManager::AddCSUHeapDescriptorIndex(const int count)
{
	currentCSUDescriptorNum += count;
}

void FHeapManager::AddRtvHeapDescriptorIndex(const int count)
{
	currentRtvDescriptorNum += count;
}

void FHeapManager::AddDsvHeapDescriptorIndex(const int count)
{
	currentDsvDescriptorNum += count;
}

void FHeapManager::SubIndex(const int count)
{
	currentCSUDescriptorNum -= count;
}

ID3D12DescriptorHeap* FHeapManager::GetHeap(HeapType heapType)
{
	switch (heapType)
	{
	case HeapType::CBV_SRV_UAV:
		return mCbvSrvUavHeap.Get();
	case HeapType::RTV:
		return mRtvHeap.Get();
	case HeapType::DSV:
		return mDsvHeap.Get();
	default:
	{
		assert(0);
		break;
	}
	}
	return nullptr;
}
