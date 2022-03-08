#pragma once
#include "FInputBase.h"
#include "FTaskManager.h"

class FWin32Input
{
public:
	FWin32Input();
	~FWin32Input();
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static FWin32Input* GetFWin32Input();

protected:
	static std::unique_ptr<FWin32Input> fWin32Input;

private:
	virtual void OnMouseDown(WPARAM btnState, int x, int y);
	virtual void OnMouseUp(WPARAM btnState, int x, int y);
	virtual void OnMouseMove(WPARAM btnState, int x, int y);
	void OnKeyboardInput(const GameTimer& gt);

	POINT mLastMousePos;
	std::unique_ptr<FTaskManager> fTaskManager;
};