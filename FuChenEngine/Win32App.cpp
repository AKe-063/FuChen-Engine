#include "stdafx.h"
#include "Win32App.h"

using Microsoft::WRL::ComPtr;
using namespace std;
using namespace glm;

std::shared_ptr<Win32App> Win32App::mApp(new Win32App(GetInstanceModule(0)));

Win32App::Win32App(HINSTANCE hInstance)
	:App()
{
	mTimer = std::make_unique<GameTimer>();
	fScene = std::make_unique<FScene>();
	fAssetManager = std::make_unique<FAssetManager>();
}

Win32App::~Win32App()
{
	if (dxRender->GetDevice() != nullptr)
		dxRender->FlushCommandQueue();
}

int Win32App::Run()
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

			if (!mAppPaused)
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

bool Win32App::Initialize()
{
	dxRender = std::make_unique<DxRender>();

	if (!(InitWindow() && dxRender->InitDirect3D()))
		return false;

	// Do the initial resize code.
	dxRender->OnResize();

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

void Win32App::Update(const GameTimer& gt)
{
	OnKeyboardInput(gt);
	dxRender->GetCamera()->SetView();
}

void Win32App::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(dxRender->GetWin32Window()->GetMainWnd());
}

void Win32App::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void Win32App::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = radians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = radians(0.25f * static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		dxRender->GetCamera()->Pitch(dy);
		dxRender->GetCamera()->Yaw(dx);

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);

	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.1f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.1f * static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 100.0f, 500.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void Win32App::OnKeyboardInput(const GameTimer& gt)
{
	const float dt = gt.DeltaTime();
	const float speed = 1000.0f;

	if (GetAsyncKeyState('W') & 0x8000)
		dxRender->GetCamera()->Walk(speed * dt);

	if (GetAsyncKeyState('S') & 0x8000)
		dxRender->GetCamera()->Walk(speed * -dt);

	if (GetAsyncKeyState('A') & 0x8000)
		dxRender->GetCamera()->Strafe(speed * -dt);

	if (GetAsyncKeyState('D') & 0x8000)
		dxRender->GetCamera()->Strafe(speed * dt);

	if (GetAsyncKeyState('Q') & 0x8000)
		dxRender->GetCamera()->Roll(0.05f);

	if (GetAsyncKeyState('E') & 0x8000)
		dxRender->GetCamera()->Roll(-0.05f);

	dxRender->GetCamera()->UpdateViewMatrix();
}

LRESULT Win32App::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		return WMActivate(wParam, lParam);

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		return WMSize(wParam, lParam);

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		return WMEnterSizeMove(wParam, lParam);

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		return WMExitSizeMove(wParam, lParam);

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		return WMDestroy(wParam, lParam);

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return WMMenuChar(wParam, lParam);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		return WMGetMinMaxiInfo(wParam, lParam);

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		return WMRButtonDown(wParam, lParam);
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		return WMRButtonUp(wParam, lParam);
	case WM_MOUSEMOVE:
		return WMMouseMove(wParam, lParam);
	case WM_KEYUP:
		return WMKeyUp(wParam, lParam);
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT Win32App::WMActivate(WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(wParam) == WA_INACTIVE)
	{
		mAppPaused = true;
		mTimer->Stop();
	}
	else
	{
		mAppPaused = false;
		mTimer->Start();
	}
	return 0;
}

LRESULT Win32App::WMSize(WPARAM wParam, LPARAM lParam)
{
	// Save the new client area dimensions.
	dxRender->GetWin32Window()->SetWidth(LOWORD(lParam));
	dxRender->GetWin32Window()->SetHeight(HIWORD(lParam));
	if (dxRender->GetDevice())
	{
		if (wParam == SIZE_MINIMIZED)
		{
			mAppPaused = true;
			mMinimized = true;
			mMaximized = false;
		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			mAppPaused = false;
			mMinimized = false;
			mMaximized = true;
			dxRender->OnResize();
		}
		else if (wParam == SIZE_RESTORED)
		{

			// Restoring from minimized state?
			if (mMinimized)
			{
				mAppPaused = false;
				mMinimized = false;
				dxRender->OnResize();
			}

			// Restoring from maximized state?
			else if (mMaximized)
			{
				mAppPaused = false;
				mMaximized = false;
				dxRender->OnResize();
			}
			else if (mResizing)
			{
				// If user is dragging the resize bars, we do not resize 
				// the buffers here because as the user continuously 
				// drags the resize bars, a stream of WM_SIZE messages are
				// sent to the window, and it would be pointless (and slow)
				// to resize for each WM_SIZE message received from dragging
				// the resize bars.  So instead, we reset after the user is 
				// done resizing the window and releases the resize bars, which 
				// sends a WM_EXITSIZEMOVE message.
			}
			else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
			{
				dxRender->OnResize();
			}
		}
	}
	return 0;
}

LRESULT Win32App::WMEnterSizeMove(WPARAM wParam, LPARAM lParam)
{
	mAppPaused = true;
	mResizing = true;
	mTimer->Stop();
	return 0;
}

LRESULT Win32App::WMExitSizeMove(WPARAM wParam, LPARAM lParam)
{
	mAppPaused = false;
	mResizing = false;
	mTimer->Start();
	dxRender->OnResize();
	return 0;
}

LRESULT Win32App::WMDestroy(WPARAM wParam, LPARAM lParam)
{
	PostQuitMessage(0);
	return 0;
}

LRESULT Win32App::WMMenuChar(WPARAM wParam, LPARAM lParam)
{
	// Don't beep when we alt-enter.
	return MAKELRESULT(0, MNC_CLOSE);
}

LRESULT Win32App::WMGetMinMaxiInfo(WPARAM wParam, LPARAM lParam)
{
	((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
	((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
	return 0;
}

LRESULT Win32App::WMRButtonDown(WPARAM wParam, LPARAM lParam)
{
	OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	return 0;
}

LRESULT Win32App::WMRButtonUp(WPARAM wParam, LPARAM lParam)
{
	OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	return 0;
}

LRESULT Win32App::WMMouseMove(WPARAM wParam, LPARAM lParam)
{
	OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	return 0;
}

LRESULT Win32App::WMKeyUp(WPARAM wParam, LPARAM lParam)
{
	if (wParam == VK_ESCAPE)
	{
		PostQuitMessage(0);
	}
	else if ((int)wParam == VK_F2)
		dxRender->Set4xMsaaState(!dxRender->Get4xMsaaState());

	return 0;
}

std::shared_ptr<Win32App> Win32App::GetApp()
{
	return mApp;
}

bool Win32App::InitWindow()
{
	if (!dxRender->InitWindow())
		return false;
	
	return true;
}