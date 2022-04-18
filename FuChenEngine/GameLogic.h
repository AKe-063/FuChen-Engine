#pragma once
#include "LoadingSystem.h"
#include "CameraInputLogic.h"
#include "CommonInputLogic.h"

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
	CommonInputLogic commonInputLogic;
};