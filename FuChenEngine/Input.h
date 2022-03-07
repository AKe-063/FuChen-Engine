#pragma once
#include "Win32App.h"

class Input
{
public:
	Input();
	virtual ~Input();

protected:
	virtual void OnMouseDown(WPARAM btnState, int x, int y);
	virtual void OnMouseUp(WPARAM btnState, int x, int y);
	virtual void OnMouseMove(WPARAM btnState, int x, int y);
	void OnKeyboardInput(const GameTimer& gt);

private:

};