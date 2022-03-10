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
	return FAssetManager::GetInstance().LoadAssetsLib(assetsPath);
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
