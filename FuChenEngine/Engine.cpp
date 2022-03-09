#include "stdafx.h"
#include "Engine.h"

using namespace std;
using namespace glm;

Engine::Engine()
{
	
}

Engine::~Engine()
{
	
}

void Engine::Run()
{
	mTimer->Tick();

	if (!mEnginePaused)
	{
		dxRender->CalculateFrameStats(mTimer.get());
		Update();
		dxRender->Draw(*mTimer);
	}
	else
	{
		Sleep(100);
	}
}

bool Engine::Initialize()
{
	mTimer = std::make_unique<GameTimer>();
	fScene.reset(&FScene::GetInstance());
	fAssetManager.reset(&FAssetManager::GetInstance());
	fInput.reset(CreateInput());
	mWindow.reset(CreateAWindow());
	mCameraInputLogic = std::make_unique<CameraInputLogic>();

	if (!(InitWindow()))
		return false;
	dxRender = std::make_unique<DxRender>();

	return true;
}

void Engine::Destroy()
{
	dxRender->Destroy();
	fInput.release();
	fScene.release();
	fAssetManager.release();
}

DxRender* Engine::GetDxRender()
{
	return dxRender.get();
}

FScene* Engine::GetFScene()
{
	return fScene.get();
}

FAssetManager* Engine::GetFAssetManager()
{
	return fAssetManager.get();
}

Window* Engine::GetWindow()
{
	return mWindow.get();
}

GameTimer* Engine::GetTimer()
{
	return mTimer.get();
}

bool Engine::GetmEnginePaused()
{
	return mEnginePaused;
}

void Engine::Update()
{
	fScene->Update();
}

bool Engine::InitWindow()
{
	if (!mWindow->CreateAWindow())
		return false;
	
	return true;
}

Window* Engine::CreateAWindow()
{
#if _PLATFORM_WIN32
	return new Win32Window();
#endif
}

FInputBase* Engine::CreateInput()
{
#if _PLATFORM_WIN32
	return &FWin32Input::GetInstance();
#endif
}
