#include "stdafx.h"
#include "FWin32Input.h"
#include "Camera.h"

FWin32Input::FWin32Input()
{

}

FWin32Input::~FWin32Input()
{

}

void FWin32Input::KeyDown(int KeyCode)
{
	Keys[KeyCode] = true;
}

void FWin32Input::KeyUp(int KeyCode)
{
	Keys[KeyCode] = false;
}

bool FWin32Input::IsKeyDown(int KeyCode)
{
	return Keys[KeyCode];
}

void FWin32Input::MouseUp(WPARAM wParam, LPARAM lParam)
{
	mouseInputFlag[0] = true;
	mouseInputFlag[1] = false;
	mouseWparam[0] = wParam;
	mouselParam[0] = lParam;
}

void FWin32Input::MouseDown(WPARAM wParam, LPARAM lParam)
{
	mouseInputFlag[1] = true;
	mouseInputFlag[0] = false;
	mouseInputFlag[2] = false;
	mouseWparam[1] = wParam;
	mouselParam[1] = lParam;
}

void FWin32Input::MouseMove(WPARAM wParam, LPARAM lParam)
{
	mouseInputFlag[2] = true;
	mouseInputFlag[1] = false;
	mouseWparam[2] = wParam;
	mouselParam[2] = lParam;
}

bool* FWin32Input::GetMouseInputFlag()
{
	return mouseInputFlag;
}

WPARAM* FWin32Input::GetMouseWparam()
{
	return mouseWparam;
}

LPARAM* FWin32Input::GetMouselParam()
{
	return mouselParam;
}
