#include "stdafx.h"
#include "FAssetManager.h"

FAssetManager::FAssetManager()
{
	
}

FAssetManager::~FAssetManager()
{

}

bool FAssetManager::LoadAssetsLib(const std::string& assetsPath)
{
	std::ifstream fin(assetsPath, std::ios::binary);
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
		return true;
	}
	else
	{
		return false;
	}
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

std::wstring FAssetManager::GetTextureFilePathFromName(const std::string& name)
{
	return textureFilePath[name];
}

std::unordered_map<std::string, std::wstring> FAssetManager::GetTexturesFilePath()
{
	return textureFilePath;
}

void FAssetManager::LoadTexture()
{
	textureFilePath.insert({ "bricks",L"../FuChenEngine/Textures/bricks.dds" });
	textureFilePath.insert({ "T_Chair_M",L"../FuChenEngine/Textures/T_Chair_M.dds" });
	textureFilePath.insert({ "T_Chair_N",L"../FuChenEngine/Textures/T_Chair_N.dds" });
	textureFilePath.insert({ "T_RockMesh_M",L"../FuChenEngine/Textures/T_RockMesh_M.dds" });
	textureFilePath.insert({ "T_RockMesh_N",L"../FuChenEngine/Textures/T_RockMesh_N.dds" });
}
