#pragma once
#include "FPrimitive.h"

class DXPrimitive : public FPrimitive
{
public:
	DXPrimitive();
	virtual ~DXPrimitive();

	virtual int GetIndex()override;
	virtual void SetIndex(const int& index)override;
	virtual MeshGeometry& GetMeshGeometryInfo()override;
	virtual ObjectConstants& GetObjConstantInfo()override;

private:
	MeshGeometry geo;
	int objCBIndex = -1;
	ObjectConstants objConstant;
};