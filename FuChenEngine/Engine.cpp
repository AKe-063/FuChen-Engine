#include "stdafx.h"
#include "Engine.h"

using namespace std;
using namespace glm;

std::unique_ptr<Engine> Engine::mEngine(new Engine(GetInstanceModule(0)));

Engine::Engine(HINSTANCE hInstance)
{
	mTimer = std::make_unique<GameTimer>();
	fScene = std::make_unique<FScene>();
	fAssetManager = std::make_unique<FAssetManager>();
	fWin32Input = std::make_unique<FWin32Input>();
	win32Window = std::make_unique<Win32Window>();
	win32Window->SetAppInst(GetInstanceModule(0));
}

Engine::~Engine()
{
	if (dxRender->GetDevice() != nullptr)
		dxRender->FlushCommandQueue();
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
	dxRender = std::make_unique<DxRender>();

	if (!(InitWindow() && dxRender->InitDirect3D(win32Window.get())))
		return false;

	// Do the initial resize code.
	dxRender->OnResize(fScene->GetCamera(), win32Window.get());

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(dxRender->GetCommandList()->Reset(dxRender->GetCommandAllocator().Get(), nullptr));
	
	dxRender->BuildBoxGeometry(fScene.get(), fAssetManager.get());
	dxRender->BuildDescriptorHeaps();
	dxRender->BuildConstantBuffers();
	dxRender->BuildRootSignature();
	dxRender->BuildShadersAndInputLayout();
	dxRender->BuildPSO();

	// Execute the initialization commands.
	ThrowIfFailed(dxRender->GetCommandList()->Close());
	ID3D12CommandList* cmdsLists[] = { dxRender->GetCommandList().Get() };
	dxRender->GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	dxRender->FlushCommandQueue();

	return true;
}

void Engine::Update(const GameTimer& gt)
{
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