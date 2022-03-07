#include "stdafx.h"
#include "FAssetManager.h"

std::unique_ptr<FAssetManager> FAssetManager::fAssetManager(new FAssetManager());

FAssetManager::FAssetManager()
{
	std::ifstream fin("../FuChenEngine/ExportFile/AllAssets.dat", std::ios::binary);
	if (fin.is_open())
	{
		int32_t num = 0, len;
		fin.read((char*)&num, sizeof(int32_t));
		std::string name = "";
		for (size_t i = 0; i < num; i++)
		{
			fin.read((char*)&len, sizeof(int32_t));
			name.resize(len);
			fin.read((char*)name.data(), sizeof(char) * len);
			LoadAssets(name);
		}
	}
}

FAssetManager::~FAssetManager()
{

}

std::unique_ptr<FAssetManager>& FAssetManager::GetFAssetManager()
{
	return fAssetManager;
}

void FAssetManager::LoadAssets(const std::string& assetPath)
{
	std::string filePath = "../FuChenEngine/ExportFile/" + assetPath;
	/*std::string filePath = str.erase(str.length() - 1) + ".dat";*/
	std::unique_ptr<Serialize> sr = std::make_unique<Serialize>();
	assets.insert(std::make_pair(filePath, sr->DeserializeAssetInfo(assetPath)));
}

AssetInfo FAssetManager::GetAssetByName(const std::string& name)
{
	return assets.find("../FuChenEngine/ExportFile/" + name)->second;
}

bool FAssetManager::AssetContain(const std::string& name)
{
	if (assets.find("../FuChenEngine/ExportFile/" + name) == assets.end())
	{
		return false;
	}

	return true;
}
