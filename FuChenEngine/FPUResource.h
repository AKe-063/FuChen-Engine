#pragma once

class FPUResource
{
public:
#if _DX_PLATFORM
	ID3D12Resource* fPUResource;
#endif
};