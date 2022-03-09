#include "stdafx.h"
#include "GameInstance.h"

GameInstance::GameInstance()
{
	
}

GameInstance::~GameInstance()
{

}

bool GameInstance::Init()
{
	if (gameLocIns.Init() && engineIns.Initialize())
		return true;
	return false;
}

void GameInstance::Run()
{
	gameLocIns.Run();
	engineIns.Run();
}

void GameInstance::Destroy()
{
	gameLocIns.Destroy();
	engineIns.Destroy();
}
