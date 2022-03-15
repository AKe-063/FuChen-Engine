#pragma once
#include "MeshDescribe.h"
#include "FMesh.h"
#include "Serialize.h"

class FAssetManager : public FSingleton<FAssetManager>
{
public:
	FAssetManager();
	virtual ~FAssetManager();

	bool LoadAssetsLib(const std::string& assetsPath);
	void LoadAssets(const std::string& assetPath);
	AssetInfo GetAssetByName(const std::string& name);
	bool AssetContrain(const std::string& name);
	std::wstring GetTextureFilePathFromName(const std::string& name);
	std::unordered_map<std::string, std::wstring> GetTexturesFilePath();
	void LoadTexture();

protected:
	std::unordered_map<std::string, AssetInfo> assets;

private:
	std::unordered_map<std::string, std::wstring> textureFilePath;
};