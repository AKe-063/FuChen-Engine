#pragma once
#include "FInputBase.h"

class FWin32Input : public FInputBase, public FSingleton<FWin32Input>
{
public:
	FWin32Input();
	~FWin32Input();
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void OnKeyboardInput(const GameTimer& gt);
	virtual void Update(const GameTimer& gt)override;
	virtual bool Init()override;

protected:

private:
	virtual void OnMouseDown(WPARAM btnState, int x, int y);
	virtual void OnMouseUp(WPARAM btnState, int x, int y);
	virtual void OnMouseMove(WPARAM btnState, int x, int y);
	
	POINT mLastMousePos;
};