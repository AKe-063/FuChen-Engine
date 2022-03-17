#pragma once
#include "MeshDescribe.h"
#include "UploadBuffer.h"
#include "d3dUtil.h"

struct DXPrimitiveDesc
{
	int objCBIndex = -1;
	std::unique_ptr<UploadBuffer<ObjectConstants>> objectCB = nullptr;
	MeshGeometry* Geo = nullptr;
};