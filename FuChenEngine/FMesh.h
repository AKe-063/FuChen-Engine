#pragma once
#include "MeshDescribe.h"
#include "FMaterial.h"
#include "FAssetManager.h"

class FMesh
{
public:
	FMesh();
	virtual ~FMesh();

	std::string GetName();
	FMaterial& GetMaterial();

	void operator=(FMeshInfoStruct& meshInfo)
	{
		name = meshInfo.name;
		asset = FAssetManager::GetInstance().GetAssetByName(meshInfo.name);
	}
private:
	std::string name;
	AssetInfo asset;
	FMaterial material;
};
