#pragma once
#include "MeshDescribe.h"
#include "FMaterial.h"

class FMesh
{
public:
	FMesh();
	virtual ~FMesh();

private:
	AssetInfo asset;
	FMaterial material;
};
