#pragma once

class Window
{
public:
	Window();
	virtual ~Window();
	virtual bool Run();

	virtual int GetWidth();
	virtual int GetHeight();
	virtual void SetWidth(const int& newWidth);
	virtual void SetHeight(const int& newHeight);
	virtual bool CreateAWindow();
	float AspectRatio()const;
	virtual std::wstring GetMainWndCaption();

protected:
	int mwWidth = 800;
	int mwHeight = 600;
};