#include "stdafx.h"
#include "FAssetManager.h"

FAssetManager::FAssetManager()
{
	
}

FAssetManager::~FAssetManager()
{

}

void FAssetManager::LoadAssets(const std::string& assetPath)
{
	std::string filePath = "../FuChenEngine/ExportFile/" + assetPath;
	std::unique_ptr<Serialize> sr = std::make_unique<Serialize>();
	assets.insert(std::make_pair(filePath, sr->DeserializeAssetInfo(assetPath)));
}

AssetInfo FAssetManager::GetAssetByName(const std::string& name)
{
	return assets.find("../FuChenEngine/ExportFile/" + name)->second;
}

bool FAssetManager::AssetContrain(const std::string& name)
{
	if (assets.find("../FuChenEngine/ExportFile/" + name) == assets.end())
	{
		return false;
	}

	return true;
}
