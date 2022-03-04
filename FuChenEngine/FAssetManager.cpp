#include "stdafx.h"
#include "FAssetManager.h"

std::unique_ptr<FAssetManager> FAssetManager::fAssetManager(new FAssetManager());

FAssetManager::FAssetManager()
{

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
	std::unique_ptr<Serialize> sr = std::make_unique<Serialize>();
	assets.insert(std::make_pair(assetPath, sr->DeserializeAssetInfo(assetPath)));
}
