#include "stdafx.h"
#include "Window.h"

Window::Window()
{

}

Window::~Window()
{

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

