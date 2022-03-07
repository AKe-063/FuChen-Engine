#pragma once
#include "MeshDescribe.h"
#include "FMesh.h"
#include "Serialize.h"

class FAssetManager
{
public:
	FAssetManager();
	virtual ~FAssetManager();
	static std::unique_ptr<FAssetManager>& GetFAssetManager();

	void LoadAssets(const std::string& assetPath);
	AssetInfo GetAssetByName(const std::string& name);
	bool AssetContain(const std::string& name);

protected:
	std::unordered_map<std::string, AssetInfo> assets;
	static std::unique_ptr<FAssetManager> fAssetManager;

private:

};