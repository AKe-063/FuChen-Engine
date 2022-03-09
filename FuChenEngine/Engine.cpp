#include "stdafx.h"
#include "Engine.h"

using namespace std;
using namespace glm;

std::unique_ptr<Engine> Engine::mEngine(new Engine());

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
				dxRender->CalculateFrameStats(mTimer.get(), mWindow.get());
				Update(*mTimer);
				dxRender->Draw(*mTimer, fScene->GetCamera());
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
	fScene = std::make_unique<FScene>();
	fAssetManager = std::make_unique<FAssetManager>();
	fInput = std::make_unique<FInputBase>();
	fInput.reset(CreateInput());
	mWindow = std::make_unique<Window>();
	mWindow.reset(CreateAWindow());

	if (!(InitWindow()))
		return false;
	dxRender = std::make_unique<DxRender>(fScene.get(), fAssetManager.get(), mWindow.get());

	return true;
}

void Engine::Destroy()
{
	dxRender->Destroy();
}

void Engine::Update(const GameTimer& gt)
{
	fInput->Update(gt);
	fScene->Update();
}

std::unique_ptr<Engine>& Engine::GetApp()
{
	return mEngine;
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
	return new FWin32Input();
#endif
}
