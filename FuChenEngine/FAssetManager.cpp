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
	for (auto tex : textureFileLink)
	{
		auto& texDec = tex.GetDesc();
		if (texDec.name == name)
		{
			return texDec.textureFilePath;
		}
	}
	return L"NULL";
}

std::vector<FTexture> FAssetManager::GetTexturesFilePath()
{

	return textureFileLink;
}

void FAssetManager::LoadTexture()
{
	FTexture tex;
	auto & texDec = tex.GetDesc();
	texDec.name = "bricks";
	texDec.textureFilePath = L"../FuChenEngine/Textures/bricks.dds";
	textureFileLink.push_back(tex);

	texDec.name = "T_Chair_M";
	texDec.textureFilePath = L"../FuChenEngine/Textures/jacket_diff.dds";
	textureFileLink.push_back(tex);

	texDec.name = "T_Chair_N";
	texDec.textureFilePath = L"../FuChenEngine/Textures/T_Chair_N.dds";
	textureFileLink.push_back(tex);

	texDec.name = "T_RockMesh_M";
	texDec.textureFilePath = L"../FuChenEngine/Textures/T_RockMesh_M.dds";
	textureFileLink.push_back(tex);

	texDec.name = "T_RockMesh_N";
	texDec.textureFilePath = L"../FuChenEngine/Textures/T_RockMesh_N.dds";
	textureFileLink.push_back(tex);
}

FLight FAssetManager::LoadLight(const std::string& name)
{
	std::unique_ptr<Serialize> sr = std::make_unique<Serialize>();
	return sr->DeserialzeLightInfo(name);
}
