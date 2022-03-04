#pragma once

class Window
{
public:
	Window();
	virtual ~Window();

	virtual int GetWidth();
	virtual int GetHeight();
	virtual void SetWidth(const int& newWidth);
	virtual void SetHeight(const int& newHeight);
	virtual bool CreateAWindow() = 0;
	float AspectRatio()const;

protected:
	int mwWidth = 800;
	int mwHeight = 600;
};