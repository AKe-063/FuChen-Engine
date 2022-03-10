#include "stdafx.h"
#include "LoadingSystem.h"
#include "FScene.h"
#include "FWin32Input.h"
#include "FAssetManager.h"
#include "Engine.h"

LoadingSystem::LoadingSystem()
{

}

LoadingSystem::~LoadingSystem()
{

}

bool LoadingSystem::InitFAssetManager(const std::string& assetsPath)
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
			FAssetManager::GetInstance().LoadAssets(name);
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool LoadingSystem::LoadMap(const std::string& filePath)
{
	if (!FScene::GetInstance().LoadAllActors(filePath))
		return false;

	return true;
}

bool LoadingSystem::OnAddActorKeyDown()
{
	if (FWin32Input::GetInstance().IsKeyDown('J'))
	{
		Engine::GetInstance().GetFScene()->AddNewActor("ThirdPersonExampleMap_NewBlueprint_C_1");
		return true;
	}
	
	return false;
}
