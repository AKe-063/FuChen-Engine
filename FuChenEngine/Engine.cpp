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

int Engine::Run()
{
	MSG msg = { 0 };

	mTimer->Reset();

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
			mTimer->Tick();

			if (!mEnginePaused)
			{
				dxRender->CalculateFrameStats(mTimer.get());
				Update(*mTimer);
				dxRender->Draw(*mTimer);
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}

bool Engine::Initialize()
{
	mTimer = std::make_unique<GameTimer>();
	fScene.reset(&FScene::GetInstance());
	fAssetManager.reset(&FAssetManager::GetInstance());
	//fInput = std::make_unique<FInputBase>();
	fInput.reset(CreateInput());
	//mWindow = std::make_unique<Window>();
	mWindow.reset(CreateAWindow());

	if (!(InitWindow()))
		return false;
	//dxRender = std::make_unique<DxRender>(mWindow.get());
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

void Engine::Update(const GameTimer& gt)
{
	fInput->Update(gt);
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
