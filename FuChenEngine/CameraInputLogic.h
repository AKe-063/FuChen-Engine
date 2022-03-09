#pragma once

class CameraInputLogic
{
public:
	CameraInputLogic();
	~CameraInputLogic();

	void OnKeyboardInput();
	void OnMouseInput();
	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	POINT mLastMousePos;
};