#pragma once

class FPUResource
{
public:
#if _DX_PLATFORM
	Microsoft::WRL::ComPtr<ID3D12Resource> fPUResource;
#endif
};