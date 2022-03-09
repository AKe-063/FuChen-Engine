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
	if (engineIns.Initialize() && gameLocIns.Init())
		return true;
	return false;
}

void GameInstance::Run()
{
	engineIns.Run();
	gameLocIns.Run();
}

void GameInstance::Destroy()
{
	engineIns.Destroy();
	gameLocIns.Destroy();
}
