#include "stdafx.h"
#include "Engine.h"

using namespace std;
using namespace glm;

std::unique_ptr<Engine> Engine::mEngine(new Engine(GetInstanceModule(0)));

Engine::Engine(HINSTANCE hInstance)
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
				dxRender->CalculateFrameStats(mTimer.get(), win32Window.get());
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
	fWin32Input = std::make_unique<FWin32Input>();
	win32Window = std::make_unique<Win32Window>();
	win32Window->SetAppInst(GetInstanceModule(0));
	dxRender = std::make_unique<DxRender>();

	if (!(InitWindow() && dxRender->InitDirect3D(win32Window.get())))
		return false;

	dxRender->Init(fScene.get(), fAssetManager.get(), win32Window.get());

	return true;
}

void Engine::Destroy()
{
	if (dxRender->GetDevice() != nullptr)
		dxRender->FlushCommandQueue();
}

void Engine::Update(const GameTimer& gt)
{
	fWin32Input->OnKeyboardInput(gt);
	fScene->GetCamera()->UpdateViewMatrix();
	fScene->GetCamera()->SetView();
}

std::unique_ptr<Engine>& Engine::GetApp()
{
	return mEngine;
}

bool Engine::InitWindow()
{
	if (!win32Window->CreateAWindow())
		return false;
	
	return true;
}