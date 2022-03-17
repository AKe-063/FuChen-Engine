#pragma once
#include "MeshDescribe.h"
#include "UploadBuffer.h"

struct DXPrimitiveDesc
{
	DXPrimitiveDesc(ID3D12Device* device, UINT elementCount, bool isConstantBuffer);
	std::shared_ptr<UploadBuffer<ObjectConstants>> objectCB = nullptr;
	MeshGeometry geo;
};