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

protected:
	std::unordered_map<std::string, AssetInfo> assets;
	static std::unique_ptr<FAssetManager> fAssetManager;

private:

};