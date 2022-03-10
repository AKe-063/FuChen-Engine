#pragma once
#include "LoadingSystem.h"
#include "CameraInputLogic.h"

class GameLogic
{
public:
	GameLogic();
	~GameLogic();

	bool Init();
	void Run();
	void Destroy();

private:
	LoadingSystem loadSystem;
	CameraInputLogic cameraInputLogic;
};