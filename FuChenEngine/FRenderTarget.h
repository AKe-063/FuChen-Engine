#pragma once
#include <basetsd.h>

enum RTBufferType
{
	RTColorBuffer = 0,
	DepthStencilBuffer = 1,
};

class FRenderTarget
{
private:
	UINT32 mWidth;
	UINT32 mHeight;

};