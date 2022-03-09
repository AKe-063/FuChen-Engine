#include "stdafx.h"
#include "GameLogic.h"

GameLogic::GameLogic()
	:loadSystem()
{

}

GameLogic::~GameLogic()
{

}

bool GameLogic::Init()
{
	if (!LoadMap("../FuChenEngine/ExportFile/AllActor.dat"))
		return false;

	return true;
}

void GameLogic::Run()
{

}

void GameLogic::Destroy()
{

}

bool GameLogic::LoadMap(const std::string& filePath)
{
	return loadSystem.Load(filePath);
}
