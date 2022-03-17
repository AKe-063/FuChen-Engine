#pragma once
#include "MeshDescribe.h"
#include "UploadBuffer.h"

struct DXPrimitiveDesc
{
public:
// 	int objCBIndex = -1;
 	UploadBuffer<ObjectConstants> objectCB;
	MeshGeometry geo;
};
