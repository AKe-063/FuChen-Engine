#pragma once
#include "FTextureStruct.h"

class FTexture
{
public:
	FTexture();
	virtual ~FTexture();

	FTextureDesc& GetDesc();

private:
	FTextureDesc fTexture;

};