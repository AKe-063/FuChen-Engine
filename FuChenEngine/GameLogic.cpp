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

	Engine::GetInstance().GetDxRender()->BuildInitialMap();
	return true;
}

void GameLogic::Run()
{
	// 	while (!Engine::GetInstance().GetmEngineFinished())
	// 	{
	// 		cameraInputLogic.OnKeyboardInput();
	// 		cameraInputLogic.OnMouseInput();
	// 		if (loadSystem.OnAddActorKeyDown())
	// 		{
	// 			Engine::GetInstance().GetDxRender()->AddNewBuild();
	// 		}
	// 	}
	cameraInputLogic.OnKeyboardInput();
	cameraInputLogic.OnMouseInput();
	if (loadSystem.OnAddActorKeyDown())
	{
		Engine::GetInstance().GetDxRender()->AddNewBuild();
	}
}

void GameLogic::Destroy()
{

}
