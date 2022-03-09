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
	MSG msg = { 0 };

	Engine::GetInstance().GetTimer()->Reset();

	while (msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise, do animation/game stuff.
		else
		{
			Engine::GetInstance().Run();
			gameLogIns.Run();
		}
	}
}

void GameInstance::Destroy()
{
	Engine::GetInstance().Destroy();
	gameLogIns.Destroy();
}
