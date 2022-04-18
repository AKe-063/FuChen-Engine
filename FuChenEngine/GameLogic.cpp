#include "stdafx.h"
#include "GameLogic.h"
#include "Engine.h"

GameLogic::GameLogic()
	:loadSystem(), cameraInputLogic(), commonInputLogic()
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
	commonInputLogic.OnChoosePostProcessKeyDown();
// 	if (loadSystem.OnAddActorKeyDown())
// 	{
// 		Engine::GetInstance().GetRenderer()->AddNewBuild();
// 	}
}

void GameLogic::Destroy()
{

}
