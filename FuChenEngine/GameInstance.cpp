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
// 	std::thread engineThread(&Engine::Run, &Engine::GetInstance());
// 	std::thread gameLogThread(&GameLogic::Run, &gameLogIns);
// 
// 	engineThread.join();
// 	gameLogThread.join();

	while (Engine::GetInstance().Run())
	{
		gameLogIns.Run();
	}
}

void GameInstance::Destroy()
{
	Engine::GetInstance().Destroy();
	gameLogIns.Destroy();
}
