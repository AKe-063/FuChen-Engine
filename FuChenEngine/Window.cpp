#include "stdafx.h"
#include "Window.h"
#include "Engine.h"

Window::Window()
{

}

Window::~Window()
{

}

int Window::Run()
{
	return 1;
}

int Window::GetWidth()
{
	return mwWidth;
}

int Window::GetHeight()
{
	return mwHeight;
}

void Window::SetWidth(const int& newWidth)
{
	mwWidth = newWidth;
}

void Window::SetHeight(const int& newHeight)
{
	mwHeight = newHeight;
}

bool Window::CreateAWindow()
{
	return true;
}

float Window::AspectRatio() const
{
	return static_cast<float>(mwWidth/mwHeight);
}

std::wstring Window::GetMainWndCaption()
{
	return L"this is WindowBase.";
}

void Window::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((Engine::GetInstance().GetTimer()->TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wstring fpsStr = std::to_wstring(fps);
		std::wstring mspfStr = std::to_wstring(mspf);

		std::wstring windowText = GetMainWndCaption() +
			L"    fps: " + fpsStr +
			L"   mspf: " + mspfStr;

		//SetWindowText(mhMainWnd, windowText.c_str());
		SetWindowText(GetActiveWindow(), windowText.c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

