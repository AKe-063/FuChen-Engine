#pragma once
#include "Window.h"
#include "Win32App.h"

class Win32App;

class Win32Window : public Window
{
public:
	Win32Window();
	~Win32Window();
	virtual bool CreateAWindow()override;
	HINSTANCE GetAppInst();
	void SetAppInst(const HINSTANCE& hInstance);
	HWND GetMainWnd();
	std::wstring GetMainWndCaption();
protected:
	

private:
	HINSTANCE mhAppInst = nullptr; // application instance handle
	HWND      mhMainWnd = nullptr; // main window handle

	// Derived class should set these in derived constructor to customize starting values.
	std::wstring mMainWndCaption = L"FuChen Engine";
};