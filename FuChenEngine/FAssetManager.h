#pragma once
#include "MeshDescribe.h"
#include "FMesh.h"
#include "Serialize.h"

class FAssetManager : public FSingleton<FAssetManager>
{
public:
	FAssetManager();
	virtual ~FAssetManager();

	void LoadAssets(const std::string& assetPath);
	AssetInfo GetAssetByName(const std::string& name);
	bool AssetContrain(const std::string& name);

protected:
	std::unordered_map<std::string, AssetInfo> assets;

private:

};