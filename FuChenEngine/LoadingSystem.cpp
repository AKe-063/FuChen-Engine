#include "stdafx.h"
#include "LoadingSystem.h"
#include "FScene.h"

LoadingSystem::LoadingSystem()
{

}

LoadingSystem::~LoadingSystem()
{

}

bool LoadingSystem::Load(const std::string& filePath)
{
	if (!FScene::GetInstance().LoadAllActors(filePath))
		return false;

	return true;
}
