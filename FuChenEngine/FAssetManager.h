#pragma once
#include "MeshDescribe.h"
#include "FMesh.h"
#include "Serialize.h"
#include "FTexture.h"
#include "FLight.h"

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
	std::vector<FTexture> GetTexturesFilePath();
	void LoadTexture();
	FLight LoadLight(const std::string& name);

protected:
	std::unordered_map<std::string, AssetInfo> assets;

private:
	std::vector<FTexture> textureFileLink;
};