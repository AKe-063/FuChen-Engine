#include "stdafx.h"
#include "GameLogic.h"
#include "Engine.h"

GameLogic::GameLogic()
	:loadSystem(), cameraInputLogic()
{

}

GameLogic::~GameLogic()
{

}

bool GameLogic::Init()
{
	if (!LoadMap("../FuChenEngine/ExportFile/AllActor.dat"))
		return false;

	Engine::GetInstance().GetDxRender()->Build();
	return true;
}

void GameLogic::Run()
{
	cameraInputLogic.OnKeyboardInput();
	cameraInputLogic.OnMouseInput();
}

void GameLogic::Destroy()
{

}

bool GameLogic::LoadMap(const std::string& filePath)
{
	return loadSystem.Load(filePath);
}
