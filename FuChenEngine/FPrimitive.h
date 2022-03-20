#pragma once

class FPrimitive
{
public:
	virtual ~FPrimitive();

	virtual int GetIndex() = 0;
	virtual void SetIndex(const int& index) = 0;
	virtual MeshGeometry& GetMeshGeometryInfo() = 0;
};