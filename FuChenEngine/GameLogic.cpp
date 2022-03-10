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
	if (!(loadSystem.InitFAssetManager("../FuChenEngine/ExportFile/AllAssets.dat") && 
		loadSystem.LoadMap("../FuChenEngine/ExportFile/AllActor.dat")))
		return false;

	Engine::GetInstance().GetDxRender()->Build();
	return true;
}

void GameLogic::Run()
{
	cameraInputLogic.OnKeyboardInput();
	cameraInputLogic.OnMouseInput();
	if (loadSystem.OnAddActorKeyDown())
	{
		Engine::GetInstance().GetDxRender()->Build();
	}
	
}

void GameLogic::Destroy()
{

}
