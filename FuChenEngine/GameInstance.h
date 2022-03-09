#pragma once
#include "Engine.h"
#include "GameLogic.h"

class GameInstance
{
public:
	GameInstance();
	~GameInstance();

	bool Init();
	void Run();
	void Destroy();

private:
	GameLogic gameLogIns;
};