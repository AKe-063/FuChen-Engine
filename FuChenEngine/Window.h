#pragma once

class Window
{
public:
	Window();
	virtual ~Window();
	virtual int Run();

	virtual int GetWidth();
	virtual int GetHeight();
	virtual void SetWidth(const int& newWidth);
	virtual void SetHeight(const int& newHeight);
	virtual bool CreateAWindow();
	float AspectRatio()const;
	virtual std::wstring GetMainWndCaption();
	virtual void CalculateFrameStats();

protected:
	int mwWidth = 1440;
	int mwHeight = 900;
};