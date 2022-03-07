#pragma once
#include "MeshDescribe.h"
#include "App.h"
#include "Camera.h"
#include "FAssetManager.h"
#include "FScene.h"
#include "DxRender.h"

using Microsoft::WRL::ComPtr;
using namespace glm;

class DxRender;

class Win32App: public App
{
public:
	Win32App(HINSTANCE hInstance);
	~Win32App();

	virtual int Run()override;
	virtual bool Initialize()override;
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT WMActivate(WPARAM wParam, LPARAM lParam);
	LRESULT WMSize(WPARAM wParam, LPARAM lParam);
	LRESULT WMEnterSizeMove(WPARAM wParam, LPARAM lParam);
	LRESULT WMExitSizeMove(WPARAM wParam, LPARAM lParam);
	LRESULT WMDestroy(WPARAM wParam, LPARAM lParam);
	LRESULT WMMenuChar(WPARAM wParam, LPARAM lParam);
	LRESULT WMGetMinMaxiInfo(WPARAM wParam, LPARAM lParam);
	LRESULT WMRButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT WMRButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT WMMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT WMKeyUp(WPARAM wParam, LPARAM lParam);

	static std::shared_ptr<Win32App> GetApp();

protected:
	static std::shared_ptr<Win32App> mApp;

private:
	virtual void Update(const GameTimer& gt)override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y);
	virtual void OnMouseUp(WPARAM btnState, int x, int y);
	virtual void OnMouseMove(WPARAM btnState, int x, int y);
	void OnKeyboardInput(const GameTimer& gt);

	bool InitWindow();

	std::unique_ptr<FAssetManager> fAssetManager;
	std::unique_ptr<FScene> fScene;
	std::unique_ptr<DxRender> dxRender;

	POINT mLastMousePos;
	// Used to keep track of the delta-time?and game time (?.4).
	std::unique_ptr<GameTimer> mTimer;
};
