#pragma once
#include "FInputBase.h"

class FWin32Input : public FInputBase, public FSingleton<FWin32Input>
{
public:
	FWin32Input();
	~FWin32Input();

	void KeyDown(int KeyCode);
	void KeyUp(int KeyCode);
	bool IsKeyDown(int KeyCode);

	void MouseUp(WPARAM wParam, LPARAM lParam);
	void MouseDown(WPARAM wParam, LPARAM lParam);
	void MouseMove(WPARAM wParam, LPARAM lParam);

	bool* GetMouseInputFlag();
	WPARAM* GetMouseWparam();
	LPARAM* GetMouselParam();

protected:

private:
	//UP 0, DOWN 1, MOVE 2
	bool mouseInputFlag[3] = { false };
	WPARAM mouseWparam[3] = { 0 };
	LPARAM mouselParam[3] = { 0 };
	bool Keys[256] = { false };
};