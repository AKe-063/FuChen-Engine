#pragma once
#include "Window.h"

class Win32Window : public Window
{
public:
	Win32Window();
	~Win32Window();
	virtual int Run()override;

	virtual bool CreateAWindow()override;
	HINSTANCE GetAppInst();
	void SetAppInst(const HINSTANCE& hInstance);
	HWND GetMainWnd();
	virtual std::wstring GetMainWndCaption();

protected:

private:
	HINSTANCE mhAppInst = nullptr; // application instance handle
	HWND      mhMainWnd = nullptr; // main window handle
	MSG msg = { 0 };

	// Derived class should set these in derived constructor to customize starting values.
	std::wstring mMainWndCaption = L"FuChen Engine";
};