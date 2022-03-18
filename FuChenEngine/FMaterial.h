#pragma once
#include "FTexture.h"
#include "FMaterialDesc.h"

class FMaterial
{
public:
	FMaterial();
	~FMaterial();

private:
	FMaterialDesc materialDesc;
	
};