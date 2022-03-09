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
	if (Engine::GetInstance().Initialize() && gameLogIns.Init())
		return true;
	return false;
}

void GameInstance::Run()
{
	Engine::GetInstance().Run();
	gameLogIns.Run();
}

void GameInstance::Destroy()
{
	Engine::GetInstance().Destroy();
	gameLogIns.Destroy();
}
