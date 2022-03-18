#include "stdafx.h"
#include "DXPrimitiveDesc.h"

DXPrimitiveDesc::DXPrimitiveDesc(ID3D12Device* device, UINT elementCount, bool isConstantBuffer)
	//:objectCB(UploadBuffer<ObjectConstants>(device, elementCount, isConstantBuffer))
{
	objectCB = std::make_shared<UploadBuffer<ObjectConstants>>(device,elementCount,isConstantBuffer);
}
