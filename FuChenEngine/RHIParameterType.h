#pragma once
typedef struct VIEWPORT
{
	float TopLeftX;
	float TopLeftY;
	float Width;
	float Height;
	float MinDepth;
	float MaxDepth;
};

typedef struct TAGRECT
{
	long    left;
	long    top;
	long    right;
	long    bottom;
};