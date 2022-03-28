#pragma once
#include "MeshDescribe.h"

class FPrimitive
{
public:
	virtual ~FPrimitive();

	virtual int GetIndex() = 0;
	virtual void SetIndex(const int& index) = 0;
	virtual MeshGeometry& GetMeshGeometryInfo() = 0;
	virtual ObjectConstants& GetObjConstantInfo() = 0;
};