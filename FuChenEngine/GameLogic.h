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
	bool LoadMap(const std::string& filePath);

private:
	LoadingSystem loadSystem;
	CameraInputLogic cameraInputLogic;
};