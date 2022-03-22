#include "stdafx.h"
#include "RHI.h"
#include "DX12RHI.h"

RHI* RHI::rhi = nullptr;

RHI::~RHI()
{

}

RHI* RHI::Get()
{
	return rhi;
}

void RHI::CreateRHI()
{
	assert(rhi == nullptr);
#if _DX12_START_UP
	rhi = new DX12RHI();
#endif
}

void RHI::ReleaseRHI()
{
	assert(rhi != nullptr);
	delete rhi;
}
