#include "stdafx.h"
#include "Win32Window.h"
#include "FWin32Input.h"

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	//return Win32App::GetApp()->MsgProc(hwnd, msg, wParam, lParam);
	return FWin32Input::GetFWin32Input()->MsgProc(hwnd, msg, wParam, lParam);
}

Win32Window::Win32Window()
	:Window()
{
	SetAppInst(GetInstanceModule(0));
}

Win32Window::~Win32Window()
{
	
}

bool Win32Window::CreateAWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mhAppInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"MainWnd";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, GetWidth(), GetHeight() };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	mhMainWnd = CreateWindow(L"MainWnd", mMainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mhAppInst, 0);
	if (!mhMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);

	return true;;
}

HINSTANCE Win32Window::GetAppInst()
{
	return mhAppInst;
}

void Win32Window::SetAppInst(const HINSTANCE& hInstance)
{
	mhAppInst = hInstance;
}

HWND Win32Window::GetMainWnd()
{
	return mhMainWnd;
}

std::wstring Win32Window::GetMainWndCaption()
{
	return mMainWndCaption;
}

